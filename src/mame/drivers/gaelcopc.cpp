// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Gaelco PC based hardware

TODO:
- tokyocop can't be emulated without proper mb bios

Custom motherboard with
82815
82801
82562 (LAN)
RTM 560-25R (Audio)
TI4200 128Mb AGP
256 Mb PC133
Pentium 4 (??? XXXXMhz)

I/O Board with Altera Flex EPF15K50EQC240-3

The graphics cards are swappable between nVidia cards from
the era. There is no protection on the games, you can just swap out
hard drives to change games, though they do seem to have their own
motherboard bioses.

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "emupal.h"
#include "screen.h"


class gaelcopc_state : public driver_device
{
public:
	gaelcopc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void gaelcopc(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void gaelcopc_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void video_start() override;
};


void gaelcopc_state::video_start()
{
}

uint32_t gaelcopc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void gaelcopc_state::gaelcopc_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom();
}

static INPUT_PORTS_START( gaelcopc )
INPUT_PORTS_END


MACHINE_CONFIG_START(gaelcopc_state::gaelcopc)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", PENTIUM, 2000000000) /* Pentium4? */
	MCFG_DEVICE_PROGRAM_MAP(gaelcopc_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(gaelcopc_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100)
MACHINE_CONFIG_END


ROM_START(tokyocop)
	ROM_REGION32_LE(0x80000, "maincpu", 0)  /* motherboard bios */
	ROM_LOAD("al1.u10", 0x000000, 0x80000, CRC(e426e030) SHA1(52bdb6d46c12150077169ac3add8c450326ad4af) )

/* Dumper's note: The drive was ordered from Gaelco and they used a 250 GB drive that apparently used to have something
else on it because when I ripped the entire drive and compressed it, the compressed image was 30 GB which is too much for me
to upload. So I just ripped the partitions (it had 3) and the size was reasonable. This rip was burned into another drive and
tested working on the real hardware. It uses the same hardware and bios as the kit version.*/

	DISK_REGION( "disks" )
	DISK_IMAGE( "tokyocop", 0, SHA1(f3b60046da7094743822191473e05ee9cbc1af86) )
ROM_END

ROM_START(tokyocopk)
	ROM_REGION32_LE(0x80000, "maincpu", 0)  /* motherboard bios */
	ROM_LOAD("al1.u10", 0x000000, 0x80000, CRC(e426e030) SHA1(52bdb6d46c12150077169ac3add8c450326ad4af) )

	DISK_REGION( "disks" ) // Maxtor 2F040J0310613 VAM051JJ0
	DISK_IMAGE( "tokyocopk", 0, SHA1(3805e41903719d8ed163f9879db65e71aba2e3e7) )
ROM_END

ROM_START(tokyocopi)
	ROM_REGION32_LE(0x80000, "maincpu", 0)  /* motherboard bios */
	ROM_LOAD("tokyocopi.pcbios", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "tokyocopi", 0, SHA1(a3cf011c8ef8ec80724c28e1534191b40ae8515d) )
ROM_END

ROM_START(rriders)
	ROM_REGION32_LE(0x80000, "maincpu", 0)  /* motherboard bios */
	ROM_LOAD("22-03.u10", 0x000000, 0x80000, CRC(0ccae12f) SHA1(a8878fa73d5a4f5e9b6e3f35994fddea08cd3c2d) )

	DISK_REGION( "disks" ) // 250 MB compact flash card
	DISK_IMAGE( "rriders", 0, SHA1(46e10517ee1b383e03c88cac67a54318c227e3e1) )
ROM_END

ROM_START(tuningrc)
	ROM_REGION32_LE(0x80000, "maincpu", 0)  /* motherboard bios */
	ROM_LOAD("310j.u10", 0x000000, 0x80000, CRC(70b0797a) SHA1(696f602e83359d5d36798d4d2962ee85171e3622) )

	DISK_REGION( "disks" ) // Hitachi HDS728080PLAT20 ATA/IDE
	DISK_IMAGE( "tuningrc", 0, SHA1(4055cdc0b0c0e99252b90fbfafc48b693b144d67) )
ROM_END


GAME( 2003, tokyocop,  0,        gaelcopc, gaelcopc, gaelcopc_state, empty_init, ROT0, "Gaelco", "Tokyo Cop (US, dedicated version)",   MACHINE_IS_SKELETON )
GAME( 2003, tokyocopk, tokyocop, gaelcopc, gaelcopc, gaelcopc_state, empty_init, ROT0, "Gaelco", "Tokyo Cop (US, kit version)",         MACHINE_IS_SKELETON )
GAME( 2003, tokyocopi, tokyocop, gaelcopc, gaelcopc, gaelcopc_state, empty_init, ROT0, "Gaelco", "Tokyo Cop (Italy)",                   MACHINE_IS_SKELETON )
GAME( 2004, rriders,   0,        gaelcopc, gaelcopc, gaelcopc_state, empty_init, ROT0, "Gaelco", "Ring Riders (Software version v2.2)", MACHINE_IS_SKELETON )
GAME( 2005, tuningrc,  0,        gaelcopc, gaelcopc, gaelcopc_state, empty_init, ROT0, "Gaelco", "Gaelco Championship Tuning Race",     MACHINE_IS_SKELETON )
