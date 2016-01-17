// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    2013-12-01 Driver for Cromemco MCB-216 SCC (Single Card Computer),
    and also the earlier CB-308.

    The driver is working, just missing some hardware info (see ToDo).

    TODO:
    - Find out cpu clock speed
    - Find out what UART type is used

    Memory allocation
    - 0000 to 0FFF - standard roms
    - 1000 to 1FFF - optional roms or ram (expect roms)
    - 2000 to 23FF - standard ram
    - 2400 to FFFF - optional whatever the user wants (expect ram)

    MCB-216:
    Press Enter twice. You will see the Basic OK prompt. To get into the
    monitor, use the QUIT command, and to return use the B command.

    The mcb216 can use an optional floppy-disk-drive unit. The only other
    storage is paper-tape, which is expected to be attached to the terminal.

    CB-308:
    Press Enter twice. You will see the Monitor logo. To get into the BASIC,
    enter GE400. To return to the monitor, use the QUIT command followed by
    pressing Enter twice. All monitor commands must be in uppercase. The
    only storage is paper-tape.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class mcb216_state : public driver_device
{
public:
	mcb216_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_MACHINE_RESET(mcb216);
	DECLARE_MACHINE_RESET(cb308);

private:
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START(mcb216_mem, AS_PROGRAM, 8, mcb216_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x2400, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(mcb216_io, AS_IO, 8, mcb216_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(status_r)
	AM_RANGE(0x01, 0x01) AM_READ(keyin_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START(cb308_mem, AS_PROGRAM, 8, mcb216_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( mcb216 )
INPUT_PORTS_END


READ8_MEMBER( mcb216_state::keyin_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

// 0x40 - a keystroke is available
// 0x80 - ok to send to terminal
READ8_MEMBER( mcb216_state::status_r )
{
	return (m_term_data) ? 0xc0 : 0x80;
}

WRITE8_MEMBER( mcb216_state::kbd_put )
{
	m_term_data = data;
}

MACHINE_RESET_MEMBER( mcb216_state, mcb216 )
{
	m_term_data = 0;
}

MACHINE_RESET_MEMBER( mcb216_state, cb308 )
{
	m_term_data = 0;
	m_maincpu->set_state_int(Z80_PC, 0xe000);
}

static MACHINE_CONFIG_START( mcb216, mcb216_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(mcb216_mem)
	MCFG_CPU_IO_MAP(mcb216_io)
	MCFG_MACHINE_RESET_OVERRIDE(mcb216_state, mcb216)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(mcb216_state, kbd_put))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( cb308, mcb216_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(cb308_mem)
	MCFG_CPU_IO_MAP(mcb216_io)
	MCFG_MACHINE_RESET_OVERRIDE(mcb216_state, cb308)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(mcb216_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mcb216 )
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "mcb216r0", 0x0000, 0x0800, CRC(86d20cea) SHA1(9fb8fdbcb8d31bd3304a0b3339c7f423188e9d37) )
	ROM_LOAD( "mcb216r1", 0x0800, 0x0800, CRC(68a25b2c) SHA1(3eadd4a5d65726f767742deb4b51a97df813f37d) )
ROM_END

ROM_START( cb308 )
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "cb308r0",  0x0000, 0x0400, CRC(62f50531) SHA1(3071e2ab7fc6b2ca889e4fb5cf7cc9ee8fbe53d3) )
	ROM_LOAD( "cb308r1",  0x0400, 0x0400, CRC(03191ac1) SHA1(84665dfc797c9f51bb659291b18399986ed846fb) )
	ROM_LOAD( "cb308r2",  0x0800, 0x0400, CRC(695ea521) SHA1(efe36a712e2a038ee804e556c5ebe05443cf798e) )
	ROM_LOAD( "cb308r3",  0x0c00, 0x0400, CRC(e3e4a778) SHA1(a7c14458f8636d860ae25b10387fa6f7f2ef6ef9) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   CLASS         INIT    COMPANY    FULLNAME       FLAGS */
COMP( 1979, mcb216, 0,      0,       mcb216,    mcb216, driver_device,  0,  "Cromemco", "MCB-216", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1977, cb308,  mcb216, 0,       cb308,     mcb216, driver_device,  0,  "Cromemco", "CB-308",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
