/***************************************************************************

Ampro Little Z80 Board

2013-09-02 Skeleton driver.

Chips: Z80A @4MHz, Z80CTC, Z80DART, WD1770/2, NCR5380. Crystal: 16 MHz

This is a complete CP/M single-board computer that could be mounted
on top of a standard 13cm floppy drive. You needed to supply your own
power supply and serial terminal.

The later versions included a SCSI chip (NCR5380) enabling the use
of a hard drive of up to 88MB.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/terminal.h"
//#include "machine/serial.h"
#include "machine/wd_fdc.h"


class ampro_state : public driver_device
{
public:
	ampro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, TERMINAL_TAG)
	{ }

	DECLARE_DRIVER_INIT(ampro);
	DECLARE_MACHINE_RESET(ampro);
	DECLARE_READ8_MEMBER( ampro_20_r );
	DECLARE_READ8_MEMBER( ampro_25_r );
	DECLARE_READ8_MEMBER( ampro_26_r );
	DECLARE_WRITE8_MEMBER( ampro_20_w );
	DECLARE_WRITE8_MEMBER(kbd_put);
private:
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

READ8_MEMBER( ampro_state::ampro_20_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( ampro_state::ampro_25_r )
{
	return (m_term_data) ? 0x21 : 0x20;
}

READ8_MEMBER( ampro_state::ampro_26_r )
{
	return 0;
}

WRITE8_MEMBER( ampro_state::ampro_20_w )
{
	m_terminal->write(space, 0, data & 0x7f);
}

static ADDRESS_MAP_START(ampro_mem, AS_PROGRAM, 8, ampro_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ampro_io, AS_IO, 8, ampro_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "z80ctc" },
	{ "z80dart" },
	{ NULL }
};

/* Input ports */
static INPUT_PORTS_START( ampro )
INPUT_PORTS_END

WRITE8_MEMBER( ampro_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(ampro_state, kbd_put)
};

MACHINE_RESET_MEMBER( ampro_state, ampro )
{
	membank("bankr0")->set_entry(1); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
}

DRIVER_INIT_MEMBER( ampro_state, ampro )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(0, &main[0x0000]);
	membank("bankr0")->configure_entry(1, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

static MACHINE_CONFIG_START( ampro, ampro_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(ampro_mem)
	MCFG_CPU_IO_MAP(ampro_io)
	MCFG_CPU_CONFIG(daisy_chain_intf)
	MCFG_MACHINE_RESET_OVERRIDE(ampro_state, ampro)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ampro )
	ROM_REGION( 0x11000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "boot", "Boot")
	ROMX_LOAD( "boot", 0x10000, 0x1000, CRC(b3524046) SHA1(5466f7d28c1a04cfbf328095cb35ad1525e91f44), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "mntr", "Monitor")
	ROMX_LOAD( "mntr", 0x10000, 0x1000, CRC(d59d0909) SHA1(936410f414b1e71445253840eea0045545e4ff0b), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "scsi", "SCSI Boot")
	ROMX_LOAD( "scsi", 0x10000, 0x1000, CRC(8eb20e5d) SHA1(0ab1ff65cf6d3c1a713a8ac5c1ee4c662ac3da0c), ROM_BIOS(3))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS          INIT     COMPANY       FULLNAME       FLAGS */
COMP( 1980, ampro,  0,      0,       ampro,     ampro,   ampro_state,   ampro,  "Ampro", "Little Z80 Board", GAME_NOT_WORKING | GAME_NO_SOUND_HW)
