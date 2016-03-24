// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

        DEC VT220

        30/06/2009 Skeleton driver.

        Schematics at www.bitsavers.org/pdf/dec/terminal/vt220/VT220_Schematic_Aug83.pdf
        There is an X2212 NVRAM at E17 which needs hookup

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/ram.h"


class vt220_state : public driver_device
{
public:
	vt220_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG) { }

	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_vt220(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
};


static ADDRESS_MAP_START(vt220_mem, AS_PROGRAM, 8, vt220_state)
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(vt220_io, AS_IO, 8, vt220_state)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( vt220 )
INPUT_PORTS_END

void vt220_state::machine_reset()
{
	memset(m_ram->pointer(),0,16*1024);
}

void vt220_state::video_start()
{
}

UINT32 vt220_state::screen_update_vt220(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static MACHINE_CONFIG_START( vt220, vt220_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8051, XTAL_11_0592MHz) // from schematic
	MCFG_CPU_PROGRAM_MAP(vt220_mem)
	MCFG_CPU_IO_MAP(vt220_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(vt220_state, screen_update_vt220)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( vt220 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "vt220" )
	ROM_SYSTEM_BIOS( 0, "vt220o", "VT220 from schematics" )
	ROMX_LOAD( "23-012e5.e3", 0x0000, 0x4000, NO_DUMP, ROM_BIOS(1)) // location e3 with jumper w7 present and w6 cut; the 8051 maps on top(?) of the first 0x1000 bytes of this rom, though this rom does seem to contain code there.
	ROMX_LOAD( "23-012e5.e3", 0x4000, 0x4000, NO_DUMP, ROM_BIOS(1)) // the rom also maps to 4000-7fff as a14 is unconnected (do we have a ROMX_RELOAD I could use here instead of this?)
	ROMX_LOAD( "23-248e4.e4", 0x8000, 0x2000, NO_DUMP, ROM_BIOS(1))
	// a000-bfff may be open bus or may be a mirror of above depending on whether the rom uses PGM/A13 as a secondary enable or not
	ROMX_LOAD( "23-248e4.e4", 0xc000, 0x2000, NO_DUMP, ROM_BIOS(1))
	// e000-ffff may be open bus or may be a mirror of above depending on whether the rom uses PGM/A13 as a secondary enable or not
	ROMX_LOAD( "23-011m1.e1", 0x0000, 0x1000, CRC(6c4930a9) SHA1(1200a5dbf431017a11a51b5c4c9b4e7952b0a2bb), ROM_BIOS(1)) // 8051 internal code, maps on top of the e3 rom at 0000-1000
	ROM_SYSTEM_BIOS( 1, "vt220v21", "VT220 Version 2.1" )
	ROMX_LOAD( "23-183e5.e3", 0x0000, 0x4000, CRC(2848db40) SHA1(b3b029ef964c86ede68ad1b2854cfad766f20af0), ROM_BIOS(2)) // location e3 with jumper w7 present and w6 cut; the 8051 maps on top(?) of the first 0x1000 bytes of this rom, though this rom does seem to contain code there.
	ROMX_LOAD( "23-183e5.e3", 0x4000, 0x4000, CRC(2848db40) SHA1(b3b029ef964c86ede68ad1b2854cfad766f20af0), ROM_BIOS(2)) // the rom also maps to 4000-7fff as a14 is unconnected (do we have a ROMX_RELOAD I could use here instead of this?)
	ROMX_LOAD( "23-182e5.e4", 0x8000, 0x4000, CRC(c759bf9f) SHA1(6fe21e8eb9576fbcda76d7909b07859db0793c4e), ROM_BIOS(2))
	ROMX_LOAD( "23-182e5.e4", 0xc000, 0x4000, CRC(c759bf9f) SHA1(6fe21e8eb9576fbcda76d7909b07859db0793c4e), ROM_BIOS(2)) // again a14 is unconnected here, so it maps at c000-ffff as well (do we have a ROMX_RELOAD I could use here instead of this?)
	ROMX_LOAD( "23-011m1.e1", 0x0000, 0x1000, CRC(6c4930a9) SHA1(1200a5dbf431017a11a51b5c4c9b4e7952b0a2bb), ROM_BIOS(2)) // 8051 internal code, maps on top of the e3 rom at 0000-1000
	ROM_SYSTEM_BIOS( 2, "vt220", "VT220 Version 2.3" )
	ROMX_LOAD( "23-178e6.bin", 0x0000, 0x8000, CRC(cce5088c) SHA1(4638304729d1213658a96bb22c5211322b74d8fc), ROM_BIOS(3)) // probably location e3 with jumper w7 cut and w6 present, hence a14 connects to this rom. socket e4 would be empty so 8000-ffff is open bus

	ROM_REGION( 0x4000, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "23-247e4.e13", 0x0000, 0x2000, NO_DUMP) // this is listed in the schematics
	ROM_LOAD( "23-348e4.e13", 0x0000, 0x2000, CRC(994f3e37) SHA1(fe72a9fe9adb3a24743a6288d88ae07570cfea9a)) // this can maybe be read as well as a read/writable ram for custom characters which lives ?above? it in chargen address space by setting a bit in a config register. I haven't figured out where in 8051 address space it appears when readable nor where the ram appears.
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                      FULLNAME       FLAGS */
COMP( 1983, vt220,  0,      0,       vt220,     vt220, driver_device,   0,  "Digital Equipment Corporation", "VT220", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
