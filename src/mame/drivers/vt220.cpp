// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

    DEC VT220

    30/06/2009 Skeleton driver.

    The VT220 exists in two major hardware revisions. The Pocket Service
    Guide has separate lists of components for earlier and later models,
    with the following being those that differ:

                                    A, B, and C     D, E, and F
        Terminal controller         70-20814-02     70-23363-01
        Power supply/monitor board  70-20624-02     70-23361-01
        Transformer                 70-20772-02     70-23362-02
        CRT/bezel/yoke (white)      70-20662-04     70-23360-01
                       (green)      70-20662-05     70-23360-02
                       (amber)      70-20662-06     70-23360-03
        Danish keyboard             LK201-AD        LK201-ED
        British keyboard            LK201-AE        LK201-EE
        Norwegian keyboard          LK201-AN        LK201-EN

    The original VT220 Technical Manual (EK-VT220-TM) covers Models A, B,
    and C only. Though it was later reprinted with an addendum for Models
    D, E, and F, this has not been found. The available schematics are
    inapplicable to later models, which have an altogether different memory
    map. It might even be possible that later models substitute some generic
    battery-backed CMOS SRAM for the X2212 and/or replace the CRT9007 with
    a custom video gate array.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/mc68681.h"
#include "machine/ram.h"
//#include "machine/x2212.h"
//#include "video/crt9007.h"
#include "screen.h"


class vt220_state : public driver_device
{
public:
	vt220_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG) { }

	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_vt220(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	void vt220(machine_config &config);
	void vt220a(machine_config &config);
	void vt220_io(address_map &map);
	void vt220_mem(address_map &map);
	void vt220a_io(address_map &map);
	void vt220a_mem(address_map &map);
};


ADDRESS_MAP_START(vt220_state::vt220_mem)
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START(vt220_state::vt220a_mem)
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START(vt220_state::vt220_io)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x2000, 0x2fff) AM_MIRROR(0xc000) AM_RAM
	AM_RANGE(0x3800, 0x380f) AM_MIRROR(0xc7f0) AM_DEVREADWRITE("duart", scn2681_device, read, write)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READNOP
ADDRESS_MAP_END

ADDRESS_MAP_START(vt220_state::vt220a_io)
	ADDRESS_MAP_UNMAP_HIGH
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

uint32_t vt220_state::screen_update_vt220(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


MACHINE_CONFIG_START(vt220_state::vt220)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8051, XTAL(11'059'200)) // from schematic for earlier version
	MCFG_CPU_PROGRAM_MAP(vt220_mem)
	MCFG_CPU_IO_MAP(vt220_io)

	MCFG_DEVICE_ADD("duart", SCN2681, XTAL(3'686'400))
	MCFG_MC68681_IRQ_CALLBACK(INPUTLINE("maincpu", MCS51_INT1_LINE))

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

MACHINE_CONFIG_START(vt220_state::vt220a)
	vt220(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(vt220a_mem)
	MCFG_CPU_IO_MAP(vt220a_io)
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START(vt220)
	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("23-178e6.bin", 0x0000, 0x8000, CRC(cce5088c) SHA1(4638304729d1213658a96bb22c5211322b74d8fc) )

	ROM_REGION(0x4000, "chargen", ROMREGION_ERASEFF)
	ROM_LOAD("23-348e4.e13", 0x0000, 0x2000, CRC(994f3e37) SHA1(fe72a9fe9adb3a24743a6288d88ae07570cfea9a) )
ROM_END

ROM_START(vt220a)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
#if 0
	// these ROMs are listed in the schematics; it is unknown whether or not they were actually shipped
	ROM_LOAD("23-012e5.e3", 0x0000, 0x4000, NO_DUMP) // location e3 with jumper w7 present and w6 cut
	ROM_RELOAD(0x4000, 0x4000) // the rom also maps to 4000-7fff as a14 is unconnected
	ROM_LOAD("23-248e4.e4", 0x8000, 0x2000, NO_DUMP)
	// a000-bfff may be open bus or may be a mirror of above depending on whether the rom uses PGM/A13 as a secondary enable or not
	ROM_RELOAD(0xc000, 0x2000)
	// e000-ffff may be open bus or may be a mirror of above depending on whether the rom uses PGM/A13 as a secondary enable or not
#endif
	ROM_LOAD("23-183e5.e3", 0x8000, 0x4000, CRC(2848db40) SHA1(b3b029ef964c86ede68ad1b2854cfad766f20af0))
	ROM_RELOAD(0xc000, 0x4000)
	ROM_LOAD("23-182e5.e4", 0x0000, 0x4000, CRC(c759bf9f) SHA1(6fe21e8eb9576fbcda76d7909b07859db0793c4e))
	ROM_RELOAD(0x4000, 0x4000)
	ROM_LOAD("23-011m1.e1", 0x0000, 0x1000, CRC(6c4930a9) SHA1(1200a5dbf431017a11a51b5c4c9b4e7952b0a2bb)) // 8051 internal code

	ROM_REGION(0x4000, "chargen", ROMREGION_ERASEFF)
#if 0
	// this ROM is listed in the schematics
	ROM_LOAD("23-247e4.e13", 0x0000, 0x2000, NO_DUMP)
#endif
	ROM_LOAD("23-348e4.e13", 0x0000, 0x2000, CRC(994f3e37) SHA1(fe72a9fe9adb3a24743a6288d88ae07570cfea9a)) // this can maybe be read as well as a read/writable ram for custom characters which lives ?above? it in chargen address space by setting a bit in a config register. I haven't figured out where in 8051 address space it appears when readable nor where the ram appears.
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  STATE         INIT  COMPANY                          FULLNAME  FLAGS */
COMP( 1983, vt220,  0,      0,       vt220,     vt220, vt220_state,  0,    "Digital Equipment Corporation", "VT220 (Version 2.3)",  MACHINE_IS_SKELETON )
COMP( 1983, vt220a, vt220,  0,       vt220a,    vt220, vt220_state,  0,    "Digital Equipment Corporation", "VT220 (Version 2.1)",  MACHINE_IS_SKELETON )
