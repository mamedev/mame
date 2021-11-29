// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Электроника ИМ-01 (Elektronika IM-01)

Soviet chess computer, produced by Svetana from 1986-1992.
IM-01T is the same hardware, the program has more difficulty levels.

Hardware notes:
- К1801ВМ1 CPU (PDP-11 derived)
- 16KB ROM (2*К1809РЕ1), 4KB RAM(К1809РУ1)
- K1809BB1 (I/O, counter)
- 4-digit 7seg panel, beeper

TODO:
- emulate К1801ВМ1, using T11 for now and I hope it works ok
- emulate K1809BB1
- inputs, 7segs, sound
- cpu frequency, irq frequency
- dump/add im01 (rom serial 106/107)

******************************************************************************/

#include "emu.h"

#include "cpu/t11/t11.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"


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

protected:
	virtual void machine_start() override;

private:
	required_device<t11_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<6> m_inputs;

	void main_map(address_map &map);

	u8 irq_callback(offs_t offset);
	INTERRUPT_GEN_MEMBER(interrupt);

	void update_display();
	u16 mux_r(offs_t offset, u16 mem_mask);
	void mux_w(offs_t offset, u16 data, u16 mem_mask);
	u16 digit_r(offs_t offset, u16 mem_mask);
	void digit_w(offs_t offset, u16 data, u16 mem_mask);
	u16 input_r(offs_t offset, u16 mem_mask);

	u16 m_inp_mux = 0;
	u16 m_digit_data = 0;
};

void im01_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_digit_data));
}



/******************************************************************************
    Interrupts
******************************************************************************/

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



/******************************************************************************
    I/O
******************************************************************************/

void im01_state::update_display()
{
	m_display->matrix(m_inp_mux, bitswap<8>(m_digit_data,0,1,2,3,4,5,6,7));
}

u16 im01_state::mux_r(offs_t offset, u16 mem_mask)
{
	return m_inp_mux;
}

void im01_state::mux_w(offs_t offset, u16 data, u16 mem_mask)
{
	// d0-d5: input mux
	COMBINE_DATA(&m_inp_mux);
	update_display();

	// d7: speaker out
	m_dac->write(BIT(m_inp_mux, 7));
}

u16 im01_state::digit_r(offs_t offset, u16 mem_mask)
{
	return m_digit_data;
}

void im01_state::digit_w(offs_t offset, u16 data, u16 mem_mask)
{
	// d0-d7: digit segment data
	COMBINE_DATA(&m_digit_data);
	update_display();
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



/******************************************************************************
    Address Maps
******************************************************************************/

void im01_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x2000, 0x5fff).rom();
	map(0xe830, 0xe831).rw(FUNC(im01_state::mux_r), FUNC(im01_state::mux_w));
	map(0xe83c, 0xe83d).rw(FUNC(im01_state::digit_r), FUNC(im01_state::digit_w));
	map(0xe83e, 0xe83f).r(FUNC(im01_state::input_r));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( im01 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) // d4  1311r
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) // c3
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) // b2
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) // a1  4342r

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) // verify?
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // enter

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // h8
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) // g7
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) // f6
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) // e5

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F)

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) // lv
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void im01_state::im01(machine_config &config)
{
	// basic machine hardware
	T11(config, m_maincpu, 5'000'000);
	m_maincpu->set_initial_mode(3 << 13);
	m_maincpu->set_addrmap(AS_PROGRAM, &im01_state::main_map);
	m_maincpu->in_iack().set(FUNC(im01_state::irq_callback));
	m_maincpu->set_periodic_int(FUNC(im01_state::interrupt), attotime::from_hz(200));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(5, 8);
	m_display->set_segmask(0x1f, 0x7f);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( im01t )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("0000148", 0x2000, 0x2000, CRC(327c6055) SHA1(b90b3b1261d677eb93014ea9e809e45b3b25152a) )
	ROM_LOAD("0000149", 0x4000, 0x2000, CRC(43b14589) SHA1(b083b631f38a26a335226bc474669ef7f332f541) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME   PARENT CMP MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
CONS( 1986, im01t, 0,      0, im01,    im01,  im01_state, empty_init, "Svetlana", "Elektronika IM-01T", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
