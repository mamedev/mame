// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  ** subclass of hh_ucom4_state (includes/hh_ucom4.h, drivers/hh_ucom4.cpp) **

  Roland TB-303 Bass Line, 1982, designed by Tadao Kikumoto
  * NEC uCOM-43 MCU, labeled D650C 133
  * 3*uPD444C 1024x4 Static CMOS SRAM
  * board is packed with discrete components

  TODO:
  - still too much to list here

***************************************************************************/

#include "includes/hh_ucom4.h"

#include "tb303.lh"


class tb303_state : public hh_ucom4_state
{
public:
	tb303_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag),
		m_t3_off_timer(*this, "t3_off")
	{ }

	required_device<timer_device> m_t3_off_timer;

	UINT8 m_ram[0xc00];
	UINT16 m_ram_address;
	bool m_ram_ce;
	bool m_ram_we;

	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(strobe_w);
	void refresh_ram();

	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ8_MEMBER(input_r);
	void update_leds();

	TIMER_DEVICE_CALLBACK_MEMBER(t3_clock);
	TIMER_DEVICE_CALLBACK_MEMBER(t3_off);

	virtual void machine_start() override;
};


/***************************************************************************

  Timer/Interrupt

***************************************************************************/

// T2 to MCU CLK: LC circuit, stable sine wave, 2.2us interval
#define TB303_T2_CLOCK_HZ   454545 /* in hz */

// T3 to MCU _INT: square wave, 1.8ms interval, short duty cycle
#define TB303_T3_CLOCK      attotime::from_usec(1800)
#define TB303_T3_OFF        (TB303_T3_CLOCK / 8)

TIMER_DEVICE_CALLBACK_MEMBER(tb303_state::t3_off)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(tb303_state::t3_clock)
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
	m_t3_off_timer->adjust(TB303_T3_OFF);
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
	UINT8 hi = 0;
	switch (m_port[NEC_UCOM4_PORTE] >> 2 & 3)
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
			m_ram_address = hi << 10 | (m_port[NEC_UCOM4_PORTE] << 8 & 0x300) | m_port[NEC_UCOM4_PORTF] << 4 | m_port[NEC_UCOM4_PORTD];
		else
			m_ram[m_ram_address] = m_port[NEC_UCOM4_PORTC];
	}
}

WRITE8_MEMBER(tb303_state::ram_w)
{
	// MCU C: RAM data
	// MCU D,F,E: RAM address
	m_port[offset] = data;
	refresh_ram();

	// MCU D,F01: pitch data
	//..
}

READ8_MEMBER(tb303_state::ram_r)
{
	// MCU C: RAM data
	if (m_ram_ce && !m_ram_we)
		return m_ram[m_ram_address];
	else
		return 0;
}

WRITE8_MEMBER(tb303_state::strobe_w)
{
	// MCU I0: RAM _WE
	m_ram_we = (data & 1) ? false : true;
	refresh_ram();

	// MCU I1: pitch data latch strobe
	// MCU I2: gate signal

	m_port[offset] = data;
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
	display_matrix(4, 4, m_port[NEC_UCOM4_PORTG], m_port[NEC_UCOM4_PORTH]);

	// todo: battery led
	// todo: 4 more leds(see top-left part)
}

WRITE8_MEMBER(tb303_state::switch_w)
{
	// MCU G: leds state
	// MCU H: input/led mux
	if (offset == NEC_UCOM4_PORTH)
		m_inp_mux = data = data ^ 0xf;

	m_port[offset] = data;
	update_leds();
}

READ8_MEMBER(tb303_state::input_r)
{
	// MCU A,B: multiplexed inputs
	// if input mux(port H) is 0, port A status buffer & gate is selected (via Q5 NAND)
	if (offset == NEC_UCOM4_PORTA && m_inp_mux == 0)
	{
		// todo..
		return m_inp_matrix[4]->read();
	}
	else
		return read_inputs(4) >> (offset*4) & 0xf;
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

  Machine Config

***************************************************************************/

void tb303_state::machine_start()
{
	hh_ucom4_state::machine_start();

	// zerofill
	memset(m_ram, 0, sizeof(m_ram));
	m_ram_address = 0;
	m_ram_ce = false;
	m_ram_we = false;

	// register for savestates
	save_item(NAME(m_ram));
	save_item(NAME(m_ram_address));
	save_item(NAME(m_ram_ce));
	save_item(NAME(m_ram_we));
}

static MACHINE_CONFIG_START( tb303, tb303_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D650, TB303_T2_CLOCK_HZ)
	MCFG_UCOM4_READ_A_CB(READ8(tb303_state, input_r))
	MCFG_UCOM4_READ_B_CB(READ8(tb303_state, input_r))
	MCFG_UCOM4_READ_C_CB(READ8(tb303_state, ram_r))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(tb303_state, ram_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(tb303_state, ram_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(tb303_state, ram_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(tb303_state, ram_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(tb303_state, switch_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(tb303_state, switch_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(tb303_state, strobe_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("t3_clock", tb303_state, t3_clock, TB303_T3_CLOCK)
	MCFG_TIMER_START_DELAY(TB303_T3_CLOCK)
	MCFG_TIMER_DRIVER_ADD("t3_off", tb303_state, t3_off)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_tb303)

	/* no video! */

	/* sound hardware */
	// discrete...
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tb303 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d650c-133.ic8", 0x0000, 0x0800, CRC(268a8d8b) SHA1(7a4236b2bc9a5cd4c80c63ca1a193e03becfcb4c) )
ROM_END


CONS( 1982, tb303, 0, 0, tb303, tb303, driver_device, 0, "Roland", "TB-303", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
