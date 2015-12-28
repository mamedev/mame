// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Intel iPB and iPC

        17/12/2009 Skeleton driver.

        22/04/2011 Connected to a terminal, it responds. Modernised.

        --> When started, you must press Space, then it will start to work.

        Monitor commands:
        A
        Dn n - dump memory
        E
        Fn n n - fill memory
        G
        Hn n - hex arithmetic
        Mn n n - move (copy) memory block
        N
        Q
        R
        Sn - modify a byte of memory
        W - display memory in Intel? format
        X - show and modify registers


        Preliminary Memory Map
        E800-F7FF BIOS ROM area
        F800-FFFF Monitor ROM (or other user interface)

        I/O F4/F5 main console input and output
        I/O F6/F7 alternate console input

        ToDo:
        - Everything!
        - iPC - Find missing rom F800-FFFF

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class ipc_state : public driver_device
{
public:
	ipc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER( ipc_f4_r );
	DECLARE_READ8_MEMBER( ipc_f5_r );
	DECLARE_WRITE8_MEMBER( kbd_put );
	UINT8 *m_ram;
	UINT8 m_term_data;
	virtual void machine_reset() override;
};

READ8_MEMBER( ipc_state::ipc_f4_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

// bit 0 high = ok to send to terminal; bit 1 high = key is pressed
READ8_MEMBER( ipc_state::ipc_f5_r )
{
	return (m_term_data) ? 3 : 1;
}


static ADDRESS_MAP_START(ipc_mem, AS_PROGRAM, 8, ipc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xdfff) AM_RAM
	AM_RANGE(0xe800, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ipc_io, AS_IO, 8, ipc_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf4, 0xf4) AM_READ(ipc_f4_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0xf5, 0xf5) AM_READ(ipc_f5_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ipc )
INPUT_PORTS_END


void ipc_state::machine_reset()
{
	m_maincpu->set_state_int(I8085_PC, 0xE800);
}

WRITE8_MEMBER( ipc_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( ipc, ipc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A, XTAL_19_6608MHz / 4)
	MCFG_CPU_PROGRAM_MAP(ipc_mem)
	MCFG_CPU_IO_MAP(ipc_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(ipc_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ipb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ipb_e8_v1.3.bin", 0xe800, 0x0800, CRC(fc9d4703) SHA1(2ce078e1bcd8b24217830c54bcf04c5d146d1b76) )
	ROM_LOAD( "ipb_f8_v1.3.bin", 0xf800, 0x0800, CRC(966ba421) SHA1(d6a904c7d992a05ed0f451d7d34c1fc8de9547ee) )
ROM_END

ROM_START( ipc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ipc_u82_v1.3_104584-001.bin", 0xe800, 0x1000, CRC(0889394f) SHA1(b7525baf1884a7d67402dea4b5566016a9861ef2) )

	// required rom is missing. Using this one from 'ipb' for now.
	ROM_LOAD( "ipb_f8_v1.3.bin", 0xf800, 0x0800, BAD_DUMP CRC(966ba421) SHA1(d6a904c7d992a05ed0f451d7d34c1fc8de9547ee) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 19??, ipb,      0,      0,      ipc,      ipc, driver_device,     0,     "Intel",   "iPB", MACHINE_NO_SOUND)
COMP( 19??, ipc,      ipb,    0,      ipc,      ipc, driver_device,     0,     "Intel",   "iPC", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
