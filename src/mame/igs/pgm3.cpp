// license:BSD-3-Clause
// copyright-holders:Xing Xing

/* PGM 3 hardware.

  Games on this platform

  Knights of Valour 3 HD

  according to Xing Xing
  "The main cpu of PGM3 whiched coded as 'SOC38' is an ARM1176@800M designed by SOCLE(http://www.socle-tech.com/). Not much infomation is available on this asic"

  there is likely a 512KBytes encrypted bootloader(u-boot?) inside the cpu which load the kernel&initrd from the external SD card.

  however, according to
  http://www.arcadebelgium.net/t4958-knights-of-valour-3-hd-sangoku-senki-3-hd
  the CPU is an Intel Atom D525 CPU with 2GB of RAM (but based on hardware images this is incorrect, they clearly show the CPU Xing Xing states is used)


  the card images seem to have encrypted data up to the C2000000 mark, then
  some text string about a non-bootable disk followed by mostly blank data

  Offset(h)  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F

0C2000000  EB 3C 90 6D 6B 64 6F 73 66 73 00 00 02 04 04 00  ë<.mkdosfs......
0C2000010  02 00 02 00 00 F8 00 01 3F 00 FF 00 00 00 00 00  .....ø..?.ÿ.....
0C2000020  00 00 04 00 00 00 29 4C 88 BA 7C 20 20 20 20 20  ......)L.º|
0C2000030  20 20 20 20 20 20 46 41 54 31 36 20 20 20 0E 1F        FAT16   ..
0C2000040  BE 5B 7C AC 22 C0 74 0B 56 B4 0E BB 07 00 CD 10  ¾[|¬"Àt.V´.»..Í.
0C2000050  5E EB F0 32 E4 CD 16 CD 19 EB FE 54 68 69 73 20  ^ëð2äÍ.Í.ëþThis
0C2000060  69 73 20 6E 6F 74 20 61 20 62 6F 6F 74 61 62 6C  is not a bootabl
0C2000070  65 20 64 69 73 6B 2E 20 20 50 6C 65 61 73 65 20  e disk.  Please
0C2000080  69 6E 73 65 72 74 20 61 20 62 6F 6F 74 61 62 6C  insert a bootabl
0C2000090  65 20 66 6C 6F 70 70 79 20 61 6E 64 0D 0A 70 72  e floppy and..pr
0C20000A0  65 73 73 20 61 6E 79 20 6B 65 79 20 74 6F 20 74  ess any key to t
0C20000B0  72 79 20 61 67 61 69 6E 20 2E 2E 2E 20 0D 0A 00  ry again ... ...


  DSW:
    1: OFF = Game mode / ON = Test mode
    2: OFF = JAMMA / ON = JVS
    3: OFF = 16/9 (1280x720) / ON = 4/3 (800x600)
    4: NO USE

  todo: add other hardware details?

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "emupal.h"
#include "screen.h"


namespace {

class pgm3_state : public driver_device
{
public:
	pgm3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	void pgm3(machine_config &config);

	void init_kov3hd();

private:

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_pgm3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_pgm3(int state);
	required_device<cpu_device> m_maincpu;
	void pgm3_map(address_map &map) ATTR_COLD;
};

void pgm3_state::pgm3_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom();
}

static INPUT_PORTS_START( pgm3 )
INPUT_PORTS_END

uint32_t pgm3_state::screen_update_pgm3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void pgm3_state::screen_vblank_pgm3(int state)
{
}

void pgm3_state::video_start()
{
}

void pgm3_state::machine_start()
{
}

void pgm3_state::machine_reset()
{
}

void pgm3_state::pgm3(machine_config &config)
{
	/* basic machine hardware */
	ARM9(config, m_maincpu, 800000000); // wrong, see notes at top of driver
	m_maincpu->set_addrmap(AS_PROGRAM, &pgm3_state::pgm3_map);
	m_maincpu->set_disable();

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(1280, 720);
	screen.set_visarea(0, 1280-1, 0, 720-1);
	screen.set_screen_update(FUNC(pgm3_state::screen_update_pgm3));
	screen.screen_vblank().set(FUNC(pgm3_state::screen_vblank_pgm3));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x1000);
}

ROM_START( kov3hd )
	ROM_REGION( 0x04000, "maincpu", ROMREGION_ERASE00 )
	// does it boot from the card, or is there an internal rom?

	DISK_REGION( "card" )
	DISK_IMAGE( "kov3hd_m105", 0, SHA1(81af30aa6e1a34b2a8fab8c5c23a313a7164767c) )
	//DISK_IMAGE( "kov3hd_v105", 0, SHA1(c185888c59880805bb76b5c0a42b05c614dcff37) ) bad dump, doesn't work on real hardware
ROM_END

ROM_START( kov3hd104 )
	ROM_REGION( 0x04000, "maincpu", ROMREGION_ERASE00 )
	// does it boot from the card, or is there an internal rom?

	DISK_REGION( "card" )
	DISK_IMAGE( "kov3hd_m104", 0, SHA1(899b3b81825e6f23ae8f39aa67ad5b019f387cf9) )
ROM_END

ROM_START( kov3hd103 )
	ROM_REGION( 0x04000, "maincpu", ROMREGION_ERASE00 )
	// does it boot from the card, or is there an internal rom?

	DISK_REGION( "card" )
	DISK_IMAGE( "kov3hd_m103", 0, SHA1(0d4fd981f477cd5ed62609b875f4ddec939a2bb0) )
ROM_END

ROM_START( kov3hd102 )
	ROM_REGION( 0x04000, "maincpu", ROMREGION_ERASE00 )
	// does it boot from the card, or is there an internal rom?

	DISK_REGION( "card" )
	DISK_IMAGE( "kov3hd_m102", 0, SHA1(a5a872f9add5527b94019ec77ff1cd0f167f040f) )
ROM_END

ROM_START( kov3hd101 )
	ROM_REGION( 0x04000, "maincpu", ROMREGION_ERASE00 )
	// does it boot from the card, or is there an internal rom?

	DISK_REGION( "card" )
	DISK_IMAGE( "kov3hd_m101", 0, SHA1(086d6f1b8b2c01a8670fd6480da44b9c507f6e08) )
ROM_END


void pgm3_state::init_kov3hd()
{
}

} // anonymous namespace


// all dumped sets might be China region, unless region info comes from elsewhere
GAME( 2011, kov3hd,     0,      pgm3,    pgm3, pgm3_state, init_kov3hd, ROT0, "IGS", "Knights of Valour 3 HD (M-105CN 13-07-04 18:54:01)", MACHINE_IS_SKELETON )
GAME( 2011, kov3hd104,  kov3hd, pgm3,    pgm3, pgm3_state, init_kov3hd, ROT0, "IGS", "Knights of Valour 3 HD (V104)", MACHINE_IS_SKELETON )
GAME( 2011, kov3hd103,  kov3hd, pgm3,    pgm3, pgm3_state, init_kov3hd, ROT0, "IGS", "Knights of Valour 3 HD (V103)", MACHINE_IS_SKELETON )
GAME( 2011, kov3hd102,  kov3hd, pgm3,    pgm3, pgm3_state, init_kov3hd, ROT0, "IGS", "Knights of Valour 3 HD (V102)", MACHINE_IS_SKELETON )
GAME( 2011, kov3hd101,  kov3hd, pgm3,    pgm3, pgm3_state, init_kov3hd, ROT0, "IGS", "Knights of Valour 3 HD (V101)", MACHINE_IS_SKELETON )
