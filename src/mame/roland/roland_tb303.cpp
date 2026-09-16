// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

Roland TB-303 Bass Line, 1982, designed by Tadao Kikumoto

Hardware notes:
- NEC uCOM-43 MCU, labeled D650C 133
- 3*uPD444C 1024x4 Static CMOS SRAM
- board is packed with discrete components

TODO:
- still too much to list here

***************************************************************************/

#include "emu.h"

#include "cpu/ucom4/ucom4.h"
#include "machine/clock.h"
#include "video/pwm.h"

#include "tb303.lh"


namespace {

class tb303_state : public driver_device
{
public:
	tb303_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void tb303(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<ucom4_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<5> m_inputs;

	u8 m_ram[0xc00];
	u8 m_ram_addrset[3];
	u16 m_ram_address = 0;
	u8 m_ram_data = 0;
	bool m_ram_ce = false;
	bool m_ram_we = false;
	u8 m_led_data = 0;
	u8 m_inp_mux = 0;

	void refresh_ram();
	template<int N> void ram_address_w(u8 data);
	void ram_data_w(u8 data);
	u8 ram_data_r();
	void strobe_w(u8 data);

	void update_leds();
	void led_w(u8 data);
	void input_w(u8 data);
	u8 input_r(offs_t offset);
};

void tb303_state::machine_start()
{
	// zerofill
	memset(m_ram, 0, sizeof(m_ram));
	memset(m_ram_addrset, 0, sizeof(m_ram_addrset));

	// register for savestates
	save_item(NAME(m_ram));
	save_item(NAME(m_ram_addrset));
	save_item(NAME(m_ram_address));
	save_item(NAME(m_ram_data));
	save_item(NAME(m_ram_ce));
	save_item(NAME(m_ram_we));
	save_item(NAME(m_led_data));
	save_item(NAME(m_inp_mux));
}



/***************************************************************************
    I/O
***************************************************************************/

// external ram

void tb303_state::refresh_ram()
{
	// MCU E2,E3 goes through a 4556 IC(pin 14,13) to one of uPD444 _CE:
	// _Q0: N/C, _Q1: IC-5, _Q2: IC-3, _Q3: IC-4
	m_ram_ce = true;
	u8 hi = 0;
	switch (m_ram_addrset[2] >> 2 & 3)
	{
		case 0: m_ram_ce = false; break;
		case 1: hi = 0; break;
		case 2: hi = 1; break;
		case 3: hi = 2; break;
	}

	if (m_ram_ce)
	{
		// _WE must be high(read mode) for address transitions
		if (!m_ram_we)
			m_ram_address = hi << 10 | (m_ram_addrset[2] << 8 & 0x300) | m_ram_addrset[1] << 4 | m_ram_addrset[0];
		else
			m_ram[m_ram_address] = m_ram_data;
	}
}

template<int N>
void tb303_state::ram_address_w(u8 data)
{
	// MCU D,F,E: RAM address
	m_ram_addrset[N] = data;
	refresh_ram();

	// MCU D,F01: pitch data
	//..
}

void tb303_state::ram_data_w(u8 data)
{
	// MCU C: RAM data
	m_ram_data = data;
	refresh_ram();
}

u8 tb303_state::ram_data_r()
{
	// MCU C: RAM data
	if (m_ram_ce && !m_ram_we)
		return m_ram[m_ram_address];
	else
		return 0;
}

void tb303_state::strobe_w(u8 data)
{
	// MCU I0: RAM _WE
	m_ram_we = (data & 1) ? false : true;
	refresh_ram();

	// MCU I1: pitch data latch strobe
	// MCU I2: gate signal
}


// switch board

void tb303_state::update_leds()
{
	// 4*4 LED matrix from port G/H:
	/*
	    0.0 D204    1.0 D211    2.0 D217    3.0 D205
	    0.1 D206    1.1 D213    2.1 D218    3.1 D207
	    0.2 D208    1.2 D215    2.2 D220    3.2 D210
	    0.3 D209    1.3 D216    2.3 D221    3.3 D212
	*/
	m_display->matrix(m_inp_mux, m_led_data);

	// todo: battery led
	// todo: 4 more leds(see top-left part)
}

void tb303_state::led_w(u8 data)
{
	// MCU G: leds state
	m_led_data = data;
	update_leds();
}

void tb303_state::input_w(u8 data)
{
	// MCU H: input/led mux
	m_inp_mux = data ^ 0xf;
	update_leds();
}

u8 tb303_state::input_r(offs_t offset)
{
	u8 data = 0;

	// MCU A,B: multiplexed inputs
	// if input mux(port H) is 0, port A status buffer & gate is selected (via Q5 NAND)
	if (offset == 0 && m_inp_mux == 0)
	{
		// todo..
		data = m_inputs[4]->read();
	}
	else
	{
		for (int i = 0; i < 4; i++)
			if (BIT(m_inp_mux, i))
				data |=  m_inputs[i]->read();

		data >>= (offset*4) & 0xf;
	}

	return data;
}



/***************************************************************************
    Inputs
***************************************************************************/

static INPUT_PORTS_START( tb303 )
	PORT_START("IN.0") // H0 port A/B
	PORT_CONFNAME( 0x03, 0x03, "Mode" )
	PORT_CONFSETTING(    0x03, "Track Write" )
	PORT_CONFSETTING(    0x02, "Track Play" )
	PORT_CONFSETTING(    0x00, "Pattern Play" )
	PORT_CONFSETTING(    0x01, "Pattern Write" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("DEL  C#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INS  D#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("1  C")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("2  D")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("3  E")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("4  F")

	PORT_START("IN.1") // H1 port A/B
	PORT_CONFNAME( 0x07, 0x00, "Track / Patt.Group" )
	PORT_CONFSETTING(    0x00, "1 / I" )
	PORT_CONFSETTING(    0x01, "2 / I" )
	PORT_CONFSETTING(    0x02, "3 / II" )
	PORT_CONFSETTING(    0x03, "4 / II" )
	PORT_CONFSETTING(    0x04, "5 / III" )
	PORT_CONFSETTING(    0x05, "6 / III" )
	PORT_CONFSETTING(    0x06, "7 / IV" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("5  G")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("6  A")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("7  B")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("8  C")

	PORT_START("IN.2") // H2 port A/B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Pattern Clear")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Function")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Pitch Mode")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Time Mode")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("9  Step")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("0  3n")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("100  A")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("200  B")

	PORT_START("IN.3") // H3 port B
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("F#")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("G#")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("A#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Back")

	PORT_START("IN.4") // H=0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Run/Stop")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Tap")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************
    Machine Configs
***************************************************************************/

void tb303_state::tb303(machine_config &config)
{
	// basic machine hardware
	NEC_D650(config, m_maincpu, 454545); // LC circuit(TI S74230), 2.2us
	m_maincpu->read_a().set(FUNC(tb303_state::input_r));
	m_maincpu->read_b().set(FUNC(tb303_state::input_r));
	m_maincpu->read_c().set(FUNC(tb303_state::ram_data_r));
	m_maincpu->write_c().set(FUNC(tb303_state::ram_data_w));
	m_maincpu->write_d().set(FUNC(tb303_state::ram_address_w<0>));
	m_maincpu->write_e().set(FUNC(tb303_state::ram_address_w<2>));
	m_maincpu->write_f().set(FUNC(tb303_state::ram_address_w<1>));
	m_maincpu->write_g().set(FUNC(tb303_state::led_w));
	m_maincpu->write_h().set(FUNC(tb303_state::input_w));
	m_maincpu->write_i().set(FUNC(tb303_state::strobe_w));

	auto &irq_clock(CLOCK(config, "irq_clock"));
	irq_clock.set_period(attotime::from_usec(1800)); // clock rate 1.8ms
	irq_clock.set_duty_cycle(0.1); // short duty cycle
	irq_clock.signal_handler().set_inputline(m_maincpu, 0);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 4);
	config.set_default_layout(layout_tb303);

	// sound hardware
	// discrete...
}



/***************************************************************************
    ROM Definitions
***************************************************************************/

ROM_START( tb303 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d650c-133.ic8", 0x0000, 0x0800, CRC(268a8d8b) SHA1(7a4236b2bc9a5cd4c80c63ca1a193e03becfcb4c) )
ROM_END

} // anonymous namespace



/***************************************************************************
    Drivers
***************************************************************************/

SYST( 1982, tb303, 0, 0, tb303, tb303, tb303_state, empty_init, "Roland", "TB-303 Bass Line", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
