// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Дебют / Дебют-М (Debut) Chess Computer

Released in 1994 in Russian Federation by ЭНЕРГОПРИБОР (Energopribor), Moscow.
It's running the Mirage chess engine by Vladimir Rybinkin, originally made for MS-DOS.

TODO:
- where is lcd segments common active state? (see m_lcd_data), current implementation
  is wrong, it will occasionally display wrong (inverted) digits
- where does the interrupt come from?
- Debut-M is an updated version? Or is it the same program as Debut with a redesigned case?

*******************************************************************************

Hardware notes:
- КР1810ВМ86 (i8086 clone), 16200K XTAL
- КР1810ГФ84 (i8284 clock divider /3)
- 2*КР537РУ10 (2KB*8 RAM), 2*8KB ROM
- lcd panel (4 7seg digits), 64 chessboard buttons, 16 leds

A bit more detailed, list of other Soviet standard TTL chips used and their equivalents:
- 2*КР580ИР82 = Intel 8282, aka 74573
- 2*К555ЛЛ1 = 7432
- К555ИЛ7 = 74138
- К555ИД10В = 74145
- КМ555КЛ15 = 74251
- К561ЛЕ5А = CD4001
- PC74HC259P = the odd one out

keypad legend:

АН  - анализ (analysis, switches view info)
ХОД - ходи (force move)
ИНТ - интерактивность (interactivity, switches 1P vs CPU, or 2P)
ПОЗ - позиция (position mode)
ВФ  - выбор фигуры (select piece)
ВП  - возврат (take back 2 plies)
УР  - уровень игры (level)
ВВ  - ввод позиции (enter position)
СБ  - сброс / новая игра (reset / new game)

******************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "debutm.lh" // clickable


namespace {

class debut_state : public driver_device
{
public:
	debut_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_out_digit(*this, "digit%u", 0U),
		m_inputs(*this, "IN.0")
	{ }

	// assume that RESET button is tied to CPU RESET pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

	// machine drivers
	void debutm(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	output_finder<4> m_out_digit;
	required_ioport m_inputs;

	// address maps
	void main_map(address_map &map);
	void main_io(address_map &map);

	// I/O handlers
	INTERRUPT_GEN_MEMBER(interrupt);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(control_w);

	u8 m_select;
	u8 m_dac_data;
	u8 m_lcd_data;
	u8 m_digit_data[4];
};

void debut_state::machine_start()
{
	// resolve handlers
	m_out_digit.resolve();

	// zerofill
	m_select = 0;
	m_dac_data = 0;
	m_lcd_data = 0;
	memset(m_digit_data, 0, sizeof(m_digit_data));

	// register for savestates
	save_item(NAME(m_select));
	save_item(NAME(m_dac_data));
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_digit_data));
}



/******************************************************************************
    I/O
******************************************************************************/

INTERRUPT_GEN_MEMBER(debut_state::interrupt)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // I8086
}

READ8_MEMBER(debut_state::input_r)
{
	u8 data = 0;
	u8 sel = m_select & 0xf;

	// a1-a3,d0: multiplexed inputs
	// read keypad
	if (sel == 0)
		data = BIT(m_inputs->read(), offset);

	// read chessboard sensors
	else if (sel < 9)
		data = BIT(m_board->read_rank(sel - 1), offset);

	return ~data;
}

WRITE8_MEMBER(debut_state::control_w)
{
	u8 mask = 1 << offset;
	u8 prev = m_select;

	// a1-a3,d0: 74259
	m_select = (m_select & ~mask) | ((data & 1) ? mask : 0);

	// 74259 q0-q3: input mux/led select
	// 74259 q4,q5: led data
	m_display->matrix(~m_select >> 4 & 3, 1 << (m_select & 0xf));

	// 74259 q7: toggle speaker
	if (~m_select & prev & 0x80)
	{
		m_dac_data ^= 1;
		m_dac->write(m_dac_data);
	}

	// a1-a3,d1-d4 go to lcd panel
	// clock lcd digit segments
	for (int i = 0; i < 4; i++)
	{
		if (BIT(~data, i + 1))
			m_digit_data[i] = (m_digit_data[i] & ~mask) | (m_lcd_data << offset);

		m_out_digit[i] = bitswap<8>(m_digit_data[i],0,7,4,3,2,1,6,5);
	}

	// where is lcd common state?
	if (offset == 0)
		m_lcd_data = BIT(~data, 4);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void debut_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x0fff).mirror(0x3000).ram();
	map(0x4000, 0x7fff).rom();
}

void debut_state::main_io(address_map &map)
{
	map(0x00, 0x0f).rw(FUNC(debut_state::input_r), FUNC(debut_state::control_w)).umask16(0x00ff);
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( debutm )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("АН (Analysis)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("ХОД (Force Move)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("ИНТ (Switch 1P/2P)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("ПОЗ (Position Mode)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("ВФ (Select Piece)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("ВП (Take Back)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("УР (Level)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("ВВ (Enter Position)")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, debut_state, reset_button, nullptr) PORT_NAME("СБ (Reset)")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void debut_state::debutm(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, 16.2_MHz_XTAL / 3);
	m_maincpu->set_periodic_int(FUNC(debut_state::interrupt), attotime::from_hz(400));
	m_maincpu->set_addrmap(AS_PROGRAM, &debut_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &debut_state::main_io);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(2, 9);
	m_display->set_bri_maximum(0.5);
	config.set_default_layout(layout_debutm);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( debutm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("dd12", 0x4000, 0x2000, CRC(0f64ebab) SHA1(e069c387ec01e8786e4fb720630196ac27fac449) ) // no ROM labels, PCB ICs location = "DDxx"
	ROM_LOAD16_BYTE("dd13", 0x4001, 0x2000, CRC(c171f5ac) SHA1(62dc030e3d60172f31f0d14944437b0fd4b53a2a) ) // "
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME    PARENT CMP MACHINE INPUT   CLASS        INIT        COMPANY, FULLNAME, FLAGS
CONS( 1994, debutm, 0,      0, debutm, debutm, debut_state, empty_init, "Energopribor", "Debut-M", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK )
