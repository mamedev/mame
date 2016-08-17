// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        MITS Altair 680b

        03/12/2009 Skeleton driver.

        08/June/2011 connected to a terminal

Monitor Commands:
J
L switch to terminal mode
M
N modify memory in a limited way
P this does a rti and causes a momentary crash. Weird.


ToDo:
- Hook ACIA back up, when there is some way to use it.


****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/mos6551.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class mits680b_state : public driver_device
{
public:
	mits680b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(terminal_status_r);
	DECLARE_READ8_MEMBER(terminal_r);
	DECLARE_READ8_MEMBER(status_check_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;
	virtual void machine_reset() override;
};

READ8_MEMBER( mits680b_state::status_check_r )
{
	return 0; // crashes at start if bit 7 high
}

READ8_MEMBER( mits680b_state::terminal_status_r )
{
	return (m_term_data) ? 3 : 2;
}

READ8_MEMBER( mits680b_state::terminal_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}


static ADDRESS_MAP_START(mits680b_mem, AS_PROGRAM, 8, mits680b_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x03ff ) AM_RAM // 1024 bytes RAM
	//AM_RANGE( 0xf000, 0xf003 ) AM_DEVREADWRITE("acia",  mos6551_device, read, write )
	AM_RANGE( 0xf000, 0xf000 ) AM_READ(terminal_status_r)
	AM_RANGE( 0xf001, 0xf001 ) AM_READ(terminal_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE( 0xf002, 0xf002 ) AM_READ(status_check_r)
	AM_RANGE( 0xff00, 0xffff ) AM_ROM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mits680b )
INPUT_PORTS_END


void mits680b_state::machine_reset()
{
	m_term_data = 0;
}

WRITE8_MEMBER( mits680b_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( mits680b, mits680b_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6800, XTAL_1MHz / 2)
	MCFG_CPU_PROGRAM_MAP(mits680b_mem)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(mits680b_state, kbd_put))

	/* acia */
	//MCFG_ACIA6551_ADD("acia")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mits680b )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mits680b.bin", 0xff00, 0x0100, CRC(397e717f) SHA1(257d3eb1343b8611dc05455aeed33615d581f29c))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1976, mits680b,  0,     0,     mits680b,  mits680b, driver_device,  0,  "MITS", "Altair 680b", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
