// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Play Mechanix / Right Hand Tech "VP100" and "VP101" platforms
    (PCBs are also marked "Raw Thrills" but all RT games appear to be on PC hardware)

    Skeleton driver by R. Belmont

    MIPS VR5500 at 300 to 400 MHz
    Xilinx Virtex-II FPGA with custom 3D hardware and 1 or 2 PowerPC 405 CPU cores
    AC97 audio with custom DMA frontend which streams 8 stereo channels
    PIC18c442 protection chip (not readable) on VP101 only (VP100 is unprotected?)

****************************************************************************/


#include "emu.h"
#include "cpu/mips/mips3.h"
#include "machine/ataintf.h"


class vp10x_state : public driver_device
{
public:
	vp10x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ32_MEMBER(tty_ready_r);
	DECLARE_WRITE32_MEMBER(tty_w);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:

	// devices
	required_device<mips3_device> m_maincpu;

	// driver_device overrides
	virtual void video_start();
};


void vp10x_state::video_start()
{
}

UINT32 vp10x_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ32_MEMBER(vp10x_state::tty_ready_r)
{
	return 0x60;    // must return &0x20 for output at tty_w to continue
}

WRITE32_MEMBER(vp10x_state::tty_w)  // set breakpoint at bfc01430 to catch when it's printing things
{
// uncomment to see startup messages - it says "RAM OK" and "EPI RSS Ver 4.5.1" followed by "<RSS active>" and then lots of dots
// Special Forces also says "<inited tv_cap> = 00000032"
//  printf("%c", data);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, vp10x_state )
	AM_RANGE(0x00000000, 0x07ffffff) AM_RAM             // this is a sufficient amount to get "RAM OK"
	AM_RANGE(0x1c000000, 0x1c000003) AM_WRITE(tty_w)        // RSS OS code uses this one
	AM_RANGE(0x1c000014, 0x1c000017) AM_READ(tty_ready_r)
	AM_RANGE(0x1c400000, 0x1c400003) AM_WRITE(tty_w)        // boot ROM code uses this one
	AM_RANGE(0x1c400014, 0x1c400017) AM_READ(tty_ready_r)
	AM_RANGE(0x1fc00000, 0x1fcfffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( vp101 )
INPUT_PORTS_END


static MACHINE_CONFIG_START( vp101, vp10x_state )
	MCFG_CPU_ADD("maincpu", R5000LE, 300000000) /* actually VR5500 with added NEC VR-series custom instructions */
	MCFG_MIPS3_ICACHE_SIZE(32768)
	MCFG_MIPS3_DCACHE_SIZE(32768)
	MCFG_MIPS3_SYSTEM_CLOCK(100000000)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_DRIVER(vp10x_state, screen_update)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 32768)
MACHINE_CONFIG_END


ROM_START(jnero)
	ROM_REGION(0x100000, "maincpu", 0)  /* Boot ROM */
	ROM_LOAD( "d710.05523.bin", 0x000000, 0x100000, CRC(6054a066) SHA1(58e68b7d86e6f24c79b99c8406e86e3c14387726) )

	ROM_REGION(0x80000, "pic", 0)       /* PIC18c422 program - read-protected, need dumped */
	ROM_LOAD( "8722a-1206.bin", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY("jn010108", 0, SHA1(4f3e9c6349c9be59213df1236dba7d79e7cd704e) )
ROM_END

ROM_START(specfrce)
	ROM_REGION(0x100000, "maincpu", 0)  /* Boot ROM */
	ROM_LOAD( "special_forces_boot_v3.4.u4", 0x000000, 0x100000, CRC(db4862ac) SHA1(a1e886d424cf7d26605e29d972d48e8d44ae2d58) )
	ROM_LOAD( "special_forces_boot_v3.5.u4", 0x000000, 0x100000, CRC(ae8dfdf0) SHA1(d64130e710d0c70095ad8ebd4e2194b8c461be4a) ) /* Newer, but keep both in driver */

	ROM_REGION(0x80000, "pic", 0)       /* PIC18c422 I/P program - read-protected, need dumped */
	ROM_LOAD( "special_forces_et_u7_rev1.2.u7", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY("sf010101", 0, SHA1(59b5e3d8e1d5537204233598830be2066aad0556) )
ROM_END


GAME( 2002,  specfrce,  0,  vp101,  vp101, driver_device,  0,  ROT0,  "ICE/Play Mechanix",    "Special Forces Elite Training",   MACHINE_IS_SKELETON )
GAME( 2004,  jnero,     0,  vp101,  vp101, driver_device,  0,  ROT0,  "ICE/Play Mechanix",    "Johnny Nero Action Hero",         MACHINE_IS_SKELETON )
