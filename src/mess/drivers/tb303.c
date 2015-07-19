// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Roland TB-303 Bass Line, 1982, designed by Tadao Kikumoto
  * NEC uCOM-43 MCU, labeled D650C 133
  * 3*uPD444C 1024x4 Static CMOS SRAM
  * board is packed with discrete components

  x

***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"

#include "tb303.lh"


class tb303_state : public driver_device
{
public:
	tb303_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_t3_off_timer(*this, "t3_off")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<timer_device> m_t3_off_timer;
	
	UINT8 m_mcu_port[9]; // MCU port A-I write data
	
	UINT8 m_ram[0xc00];
	UINT16 m_ram_address;
	bool m_ram_ce;
	bool m_ram_we;

	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(strobe_w);
	void refresh_ram();

	TIMER_DEVICE_CALLBACK_MEMBER(t3_clock);
	TIMER_DEVICE_CALLBACK_MEMBER(t3_off);

	virtual void machine_start();
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

void tb303_state::refresh_ram()
{
	// MCU E2,E3 goes through a 4556 IC(pin 14,13) to one of uPD444 _CE:
	// _Q0: N/C, _Q1: IC-5, _Q2: IC-3, _Q3: IC-4
	m_ram_ce = true;
	UINT8 hi = 0;
	switch (m_mcu_port[NEC_UCOM4_PORTE] >> 2 & 3)
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
			m_ram_address = hi << 10 | (m_mcu_port[NEC_UCOM4_PORTE] << 8 & 0x300) | m_mcu_port[NEC_UCOM4_PORTF] << 4 | m_mcu_port[NEC_UCOM4_PORTD];
		else
			m_ram[m_ram_address] = m_mcu_port[NEC_UCOM4_PORTC];
	}

	// to switchboard pin 19-22
	//..
}

WRITE8_MEMBER(tb303_state::ram_w)
{
	// MCU C: RAM data
	// MCU D,F,E: RAM address
	m_mcu_port[offset] = data;
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
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( tb303 )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tb303_state::machine_start()
{
	// zerofill
	memset(m_mcu_port, 0, sizeof(m_mcu_port));
	memset(m_ram, 0, sizeof(m_ram));
	m_ram_address = 0;
	m_ram_ce = false;
	m_ram_we = false;
	
	// register for savestates
	save_item(NAME(m_mcu_port));
	save_item(NAME(m_ram));
	save_item(NAME(m_ram_address));
	save_item(NAME(m_ram_ce));
	save_item(NAME(m_ram_we));
}

static MACHINE_CONFIG_START( tb303, tb303_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D650, TB303_T2_CLOCK_HZ)
	//MCFG_UCOM4_READ_A_CB
	//MCFG_UCOM4_READ_B_CB
	MCFG_UCOM4_READ_C_CB(READ8(tb303_state, ram_r))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(tb303_state, ram_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(tb303_state, ram_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(tb303_state, ram_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(tb303_state, ram_w))
	//MCFG_UCOM4_WRITE_G_CB
	//MCFG_UCOM4_WRITE_H_CB
	MCFG_UCOM4_WRITE_I_CB(WRITE8(tb303_state, strobe_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("t3_clock", tb303_state, t3_clock, TB303_T3_CLOCK)
	MCFG_TIMER_START_DELAY(TB303_T3_CLOCK)
	MCFG_TIMER_DRIVER_ADD("t3_off", tb303_state, t3_off)

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
	ROM_LOAD( "d650c-133.ic8", 0x0000, 0x0800, CRC(dd2f26ae) SHA1(7f5e37f38d970219dc9e5d49a20dc5335a5c0b30) )
ROM_END


CONS( 1982, tb303, 0, 0, tb303, tb303, driver_device, 0, "Roland", "TB-303", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
