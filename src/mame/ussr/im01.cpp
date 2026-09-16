// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Radon17, RCgoff, Berger
/*******************************************************************************

Электроника ИМ-01 (Elektronika IM-01)

Soviet chess computer, produced by Svetana from 1986-1992.
ИМ-01Т is the same hardware, improved program and has 1 more difficulty level.
ИМ-05 is also on similar hardware, it was released after the Soviet dissolution.

TODO:
- emulate К1801ВМ1/2, using T11 for now and I hope it works ok
- emulate K1809BB1, IRQ is from here too (measured 177.4Hz)
- emulate К1573ХМ1
- It's running a bit too fast?: CPU clock was measured 4.61MHz, beeper
  frequency 3.73kHz and beeper duration 34.2ms. In MAME, beeper frequency is
  4.15kHz and duration is 31ms, meaning it's around 1.1 times faster, maybe
  К1801ВМ1 internal timing differs from T11, and/or MAME's T11 core timing
  itself is not 100% accurate.
- Is im01t/im05 extra RAM chip used at all? Even when trying to solve mate
  problems (level 7, and overclocked CPU), there are no writes to it.
- What is im01 0x6000-0x7fff for? it will test for both RAM and ROM in this
  area if no bus error was triggered, and will fail to boot up.
- verify im05 XTAL and IRQ
- verify im05 button functions

================================================================================

Hardware notes:

ИМ-01:
- К1801ВМ1 CPU (PDP-11 derived) @ ~4.61MHz (9216 кгц XTAL)
- 16KB ROM (2*К1809РЕ1), 2KB RAM(К1809РУ1)
- K1809BB1 (I/O, counter)
- 4-digit VFD 7seg panel(cyan, green window overlay), beeper

ИМ-01Т:
- 4KB RAM (2*К1809РУ1), but also seen with 1 К1809РУ1 like ИМ-01
- rest is same as ИМ-01

ИМ-05:
- КМ1801ВМ2 CPU (similar to К1801ВМ1)
- 24KB ROM (3*К1809РЕ1), 4KB RAM (2*КР573РУ10)
- К1573ХМ1 (I/O controller)
- rest is same as ИМ-01

================================================================================

ИМ-01 keypad legend (excluding A1-H8 and black/white):

Фиг: префиксная клавиша для ввода кода шахматной фигуры, - Prefix Key (hold)
     а также для установки фигур в начальную позицию,
     сброса фигур с доски и изменения очередности хода

НП:  установка фигур в начальную позицию                 - Reset Board*
СД:  сброс всех фигур с доски                            - Clear Board*
↓:   ввод в компьютер Вашего хода,                       - Enter
     а также фигуры при установке позиции

ПХ:  передача хода компьютера                            - Move
≡:   индикация уровня сложности                          - Set Level (hold)
↑:   индикация текущего уровня                           - Show Depth
CИ:  сброс информации на индикаторе                      - Clear Entry
?:   проверка позиции                                    - Verify Position
Вар: ввод варианта                                       - Enter Variant
     (несколько ходов подряд за белых и черных)

↺:   ход назад                                           - Take Back
И:   индикация анализируемого хода                       - Show Analyzed Move
N:   чксло ходов                                         - Show Moves
РЗ:  установка режима решения шахматных задач            - Solve Mate / Set Up*

* note: hold Фиг

ИМ-05 keypad legend (differences shown, preliminary):

Реж: режима                                              - Mode (no hold)
НП:  -                                                   - Reset
СД:  -                                                   - Cancel?
↓:   -                                                   - Confirm (level/piece)
→:   -                                                   - Forward
CТА: -                                                   - Enter Move
ПХ:  -                                                   - Hint
Р:   -                                                   - Promotion (hold)

*******************************************************************************/

#include "emu.h"

#include "cpu/t11/t11.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "im01.lh"
#include "im05.lh"


namespace {

class im01_state : public driver_device
{
public:
	im01_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void im01(machine_config &config);
	void im01t(machine_config &config);
	void im05(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<t11_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<6> m_inputs;

	u16 m_inp_mux = 0;
	u16 m_digit_data = 0;

	void im01_map(address_map &map) ATTR_COLD;
	void im01t_map(address_map &map) ATTR_COLD;
	void im05_map(address_map &map) ATTR_COLD;

	u8 irq_callback(offs_t offset);
	INTERRUPT_GEN_MEMBER(interrupt);

	// I/O handlers
	u16 mux_r(offs_t offset, u16 mem_mask);
	void mux_w(offs_t offset, u16 data, u16 mem_mask);
	u16 digit_r(offs_t offset, u16 mem_mask);
	void digit_w(offs_t offset, u16 data, u16 mem_mask);
	void digita_w(offs_t offset, u16 data, u16 mem_mask);
	u16 input_r(offs_t offset, u16 mem_mask);
	u16 inputa_r(offs_t offset, u16 mem_mask);
	void error_w(offs_t offset, u16 data, u16 mem_mask);
};

void im01_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_digit_data));
}



/*******************************************************************************
    Interrupts
*******************************************************************************/

u8 im01_state::irq_callback(offs_t offset)
{
	m_maincpu->set_input_line(t11_device::CP0_LINE, CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP1_LINE, CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP3_LINE, CLEAR_LINE);
	return 0;
}

INTERRUPT_GEN_MEMBER(im01_state::interrupt)
{
	// indirect interrupt vector at 0100
	m_maincpu->set_input_line(t11_device::CP0_LINE, ASSERT_LINE);
	m_maincpu->set_input_line(t11_device::CP1_LINE, ASSERT_LINE);
	m_maincpu->set_input_line(t11_device::CP3_LINE, ASSERT_LINE);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// ИМ-01 / shared

u16 im01_state::mux_r(offs_t offset, u16 mem_mask)
{
	return m_inp_mux;
}

void im01_state::mux_w(offs_t offset, u16 data, u16 mem_mask)
{
	// d0-d5: input mux, digit select
	COMBINE_DATA(&m_inp_mux);
	m_display->write_my(m_inp_mux);

	// d7: speaker out
	m_dac->write(BIT(m_inp_mux, 7));
}

u16 im01_state::digit_r(offs_t offset, u16 mem_mask)
{
	return m_digit_data;
}

void im01_state::digit_w(offs_t offset, u16 data, u16 mem_mask)
{
	// d1-d7: digit segment data
	COMBINE_DATA(&m_digit_data);
	m_display->write_mx(bitswap<8>(m_digit_data,0,1,2,3,4,5,6,7));
}

u16 im01_state::input_r(offs_t offset, u16 mem_mask)
{
	u16 data = 0;

	// d8-d11: multiplexed inputs
	for (int i = 0; i < 6; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return data << 8;
}

void im01_state::error_w(offs_t offset, u16 data, u16 mem_mask)
{
	// unmapped port, it expects a bus error
	m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero);
}


// ИМ-05 specific

void im01_state::digita_w(offs_t offset, u16 data, u16 mem_mask)
{
	// digit segment data is in a different order
	digit_w(offset, data, mem_mask);
	m_display->write_mx(bitswap<8>(m_digit_data,0,1,6,4,2,3,5,7));
}

u16 im01_state::inputa_r(offs_t offset, u16 mem_mask)
{
	// inputs are reversed
	u16 data = input_r(offset, mem_mask);
	return (data & 0xf0ff) | (bitswap<4>(data,8,9,10,11) << 8);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void im01_state::im01_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x5fff).rom();
	map(0xe030, 0xe031).mirror(0x1800).rw(FUNC(im01_state::mux_r), FUNC(im01_state::mux_w));
	map(0xe03c, 0xe03d).mirror(0x1800).rw(FUNC(im01_state::digit_r), FUNC(im01_state::digit_w));
	map(0xe03e, 0xe03f).mirror(0x1800).r(FUNC(im01_state::input_r));
	map(0xffe8, 0xffe9).w(FUNC(im01_state::error_w)).nopr();
}

void im01_state::im01t_map(address_map &map)
{
	im01_map(map);
	map(0x0800, 0x0fff).ram();
}

void im01_state::im05_map(address_map &map)
{
	im01t_map(map);
	map(0x6000, 0x7fff).rom();
	map(0xe03c, 0xe03d).mirror(0x1800).w(FUNC(im01_state::digita_w));
	map(0xe03e, 0xe03f).mirror(0x1800).r(FUNC(im01_state::inputa_r));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( im01 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D) PORT_NAME("D 4 / Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C) PORT_NAME("C 3 / Bishop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B) PORT_NAME("B 2 / Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A) PORT_NAME("A 1 / Pawn")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME(u8"N (Show Moves)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME(u8"Вар (Enter Variant)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("? (Verify Position)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Show Depth")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME(u8"ПХ (Move)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME(u8"CИ (Clear Entry)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H) PORT_NAME("H 8")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G) PORT_NAME("G 7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F) PORT_NAME("F 6 / King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E) PORT_NAME("E 5 / Queen")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME(u8"Фиг (Prefix Key)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME(u8"И (Show Analyzed Move)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("White")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME(u8"РЗ (Solve Mate / Set Up)")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME(u8"НП (Reset Board)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME(u8"СД (Clear Board)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Black")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Set Level")
INPUT_PORTS_END

static INPUT_PORTS_START( im05 )
	PORT_INCLUDE( im01 )

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Forward")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME(u8"ПХ (Hint)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME(u8"Р (Promotion)")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Confirm")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME(u8"CТА (Enter Move)")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME(u8"Реж (Mode)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_Z) PORT_NAME("White / 9")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME(u8"N (Show Moves)")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME(u8"НП (Reset)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME(u8"СД (Cancel)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_X) PORT_NAME("Black / 0")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void im01_state::im01(machine_config &config)
{
	// basic machine hardware
	T11(config, m_maincpu, 9.216_MHz_XTAL / 2); // actually К1801ВМ1
	m_maincpu->set_initial_mode(3 << 13);
	m_maincpu->set_addrmap(AS_PROGRAM, &im01_state::im01_map);
	m_maincpu->in_iack().set(FUNC(im01_state::irq_callback));
	m_maincpu->set_periodic_int(FUNC(im01_state::interrupt), attotime::from_hz(177)); // measured

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(5, 7);
	m_display->set_segmask(0x1f, 0x7f);
	config.set_default_layout(layout_im01);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void im01_state::im01t(machine_config &config)
{
	im01(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &im01_state::im01t_map);
}

void im01_state::im05(machine_config &config)
{
	im01(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &im01_state::im05_map);
	m_maincpu->set_periodic_int(FUNC(im01_state::interrupt), attotime::from_hz(512)); // approximation

	// video hardware
	config.set_default_layout(layout_im05);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( im01 )
	ROM_REGION16_LE( 0x10000, "maincpu", 0 )
	ROM_LOAD("0000107", 0x2000, 0x2000, CRC(2318e85b) SHA1(6a92f6464af69ad0175f323f01dd067b91d345d3) )
	ROM_LOAD("0000106", 0x4000, 0x2000, CRC(b0ab8808) SHA1(b682e9e8a5e52cd8fee5ae45277ae658bf5c32f1) )
ROM_END

ROM_START( im01t )
	ROM_REGION16_LE( 0x10000, "maincpu", 0 )
	ROM_LOAD("0000148", 0x2000, 0x2000, CRC(327c6055) SHA1(b90b3b1261d677eb93014ea9e809e45b3b25152a) )
	ROM_LOAD("0000149", 0x4000, 0x2000, CRC(43b14589) SHA1(b083b631f38a26a335226bc474669ef7f332f541) )
ROM_END

ROM_START( im05 )
	ROM_REGION16_LE( 0x10000, "maincpu", 0 )
	ROM_LOAD("0000205", 0x2000, 0x2000, CRC(8aa51bcb) SHA1(211a3fa22627e828841121a735b4625a4bbff4da) )
	ROM_LOAD("0000206", 0x4000, 0x2000, CRC(3d3ecea6) SHA1(045abe1c935a8748b5a20a11f10f34c835c54156) )
	ROM_LOAD("0000207", 0x6000, 0x2000, CRC(3323378a) SHA1(94a890bff34699c8b8c516fb446872089778cf97) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
SYST( 1986, im01,  0,      0,      im01,    im01,  im01_state, empty_init, "Svetlana", "Elektronika IM-01", MACHINE_SUPPORTS_SAVE )
SYST( 1991, im01t, im01,   0,      im01t,   im01,  im01_state, empty_init, "Svetlana", "Elektronika IM-01T", MACHINE_SUPPORTS_SAVE )

SYST( 1993, im05,  0,      0,      im05,    im05,  im01_state, empty_init, "Svetlana", "Elektronika IM-05", MACHINE_SUPPORTS_SAVE )
