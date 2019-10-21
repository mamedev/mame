// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/********************************************************************

 F-E1-32 driver


 Supported Games      PCB-ID
 ----------------------------------
 Mosaic               F-E1-32-009
 Royal Poker 2        F-E1-32N-COM9e

 driver by Pierpaolo Prazzoli

*********************************************************************/

/*
royalpk2 : to get 'secret. OK.'

0002D92C: MOV L18, L29
0002D92E: CMPI L18, $1
0002D930: BNE $2d94c

go 2d92c
do l29 = 1
f5

*/

#include "emu.h"
#include "cpu/e132xs/e132xs.h"
#include "machine/eepromser.h"
#include "sound/ym2151.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class mosaicf2_state : public driver_device
{
public:
	mosaicf2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") ,
		m_videoram(*this, "videoram")
	{ }

	void mosaicf2(machine_config &config);
	void royalpk2(machine_config &config);

private:
	/* devices */
	required_device<hyperstone_device>  m_maincpu;

	/* memory pointers */
	required_shared_ptr<uint32_t> m_videoram;

	DECLARE_READ32_MEMBER(f32_input_port_1_r);
	uint32_t screen_update_mosaicf2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void common_map(address_map &map);
	void mosaicf2_io(address_map &map);
	void royalpk2_io(address_map &map);
	void royalpk2_map(address_map &map);
};


uint32_t mosaicf2_state::screen_update_mosaicf2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	offs_t offs;

	for (offs = 0; offs < 0x10000; offs++)
	{
		int y = offs >> 8;
		int x = offs & 0xff;

		if ((x < 0xa0) && (y < 0xe0))
		{
			bitmap.pix16(y, (x * 2) + 0) = (m_videoram[offs] >> 16) & 0x7fff;
			bitmap.pix16(y, (x * 2) + 1) = (m_videoram[offs] >>  0) & 0x7fff;
		}
	}

	return 0;
}



void mosaicf2_state::common_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram();
	map(0x40000000, 0x4003ffff).ram().share("videoram");
	map(0x80000000, 0x80ffffff).rom().region("user2", 0);
	map(0xfff00000, 0xffffffff).rom().region("user1", 0);
}

READ32_MEMBER(mosaicf2_state::f32_input_port_1_r)
{
	/* burn a bunch of cycles because this is polled frequently during busy loops */

	offs_t pc = m_maincpu->pc();
	if ((pc == 0x000379de) || (pc == 0x000379cc) )
		m_maincpu->eat_cycles(100);
	//else printf("PC %08x\n", pc );
	return ioport("SYSTEM_P2")->read();
}


void mosaicf2_state::mosaicf2_io(address_map &map)
{
	map(0x4003, 0x4003).r("oki", FUNC(okim6295_device::read));
	map(0x4813, 0x4813).r("ymsnd", FUNC(ym2151_device::status_r));
	map(0x5000, 0x5003).portr("P1");
	map(0x5200, 0x5203).r(FUNC(mosaicf2_state::f32_input_port_1_r));
	map(0x5400, 0x5403).portr("EEPROMIN");
	map(0x6003, 0x6003).w("oki", FUNC(okim6295_device::write));
	map(0x6803, 0x6803).w("ymsnd", FUNC(ym2151_device::data_w));
	map(0x6813, 0x6813).w("ymsnd", FUNC(ym2151_device::register_w));
	map(0x7000, 0x7003).portw("EEPROMCLK");
	map(0x7200, 0x7203).portw("EEPROMCS");
	map(0x7400, 0x7403).portw("EEPROMOUT");
}


static INPUT_PORTS_START( mosaicf2 )
	PORT_START("P1")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM_P2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x00000400, IP_ACTIVE_LOW )
	PORT_BIT( 0x00007800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)

	PORT_START( "EEPROMCLK" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)

	PORT_START( "EEPROMCS" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END




void mosaicf2_state::mosaicf2(machine_config &config)
{
	/* basic machine hardware */
	E132XN(config, m_maincpu, XTAL(20'000'000)*4); /* 4x internal multiplier */
	m_maincpu->set_addrmap(AS_PROGRAM, &mosaicf2_state::common_map);
	m_maincpu->set_addrmap(AS_IO, &mosaicf2_state::mosaicf2_io);
	m_maincpu->set_vblank_int("screen", FUNC(mosaicf2_state::irq0_line_hold));

	EEPROM_93C46_16BIT(config, "eeprom")
		.erase_time(attotime::from_usec(1))
		.write_time(attotime::from_usec(1));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(512, 512);
	screen.set_visarea(0, 319, 0, 223);
	screen.set_screen_update(FUNC(mosaicf2_state::screen_update_mosaicf2));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_555);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181)/4)); /* 3.579545 MHz */
	ymsnd.add_route(0, "lspeaker", 1.0);
	ymsnd.add_route(1, "rspeaker", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(14'318'181)/8, okim6295_device::PIN7_HIGH)); /* 1.7897725 MHz */
	oki.add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}



static INPUT_PORTS_START( royalpk2 )
	PORT_START("P1")

	PORT_START("SYSTEM_P2")
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xff7fffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)

	PORT_START( "EEPROMCLK" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)

	PORT_START( "EEPROMCS" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END



void mosaicf2_state::royalpk2_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram();
	map(0x40000000, 0x4003ffff).ram().share("videoram");
	map(0x80000000, 0x807fffff).rom().region("user2", 0);
	map(0xfff00000, 0xffffffff).rom().region("user1", 0);
}

void mosaicf2_state::royalpk2_io(address_map &map)
{
	map(0x4900, 0x4903).portr("SYSTEM_P2");

	map(0x4a00, 0x4a03).portr("EEPROMIN");

	map(0x6800, 0x6803).portw("EEPROMCLK");
	map(0x6900, 0x6903).portw("EEPROMCS");
	map(0x6a00, 0x6a03).portw("EEPROMOUT");
}

void mosaicf2_state::royalpk2(machine_config &config)
{
	/* basic machine hardware */
	GMS30C2132(config, m_maincpu, XTAL(50'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &mosaicf2_state::royalpk2_map);
	m_maincpu->set_addrmap(AS_IO, &mosaicf2_state::royalpk2_io);
	m_maincpu->set_vblank_int("screen", FUNC(mosaicf2_state::irq1_line_hold));

	EEPROM_93C46_16BIT(config, "eeprom")
		.erase_time(attotime::from_usec(1))
		.write_time(attotime::from_usec(1));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(512, 512);
	screen.set_visarea(0, 319, 0, 223);
	screen.set_screen_update(FUNC(mosaicf2_state::screen_update_mosaicf2));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_555);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

//  ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181)/4)); /* 3.579545 MHz */
//  ymsnd.add_route(0, "lspeaker", 1.0);
//  ymsnd.add_route(1, "rspeaker", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(14'318'181)/8, okim6295_device::PIN7_HIGH)); /* 1.7897725 MHz */
	oki.add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	// there is a 16c550 for communication
}


/*

Mosaic (c) 1999 F2 System

   CPU: Hyperstone E1-32XN
 Video: QuickLogic QL2003-XPL84C FPGA
 Sound: OKI 6295, BS901 (YM2151) & BS902 (YM3012)
   OSC: 20MHz & 14.31818MHz
EEPROM: 93C46 (controlled through FPGA)

F-E1-32-009
+------------------------------------------------------------------+
|            VOL                               +---------+         |
+-+                              YM3812        |   SND   |         |
  |                                            +---------+         |
+-+                              YM2151            OKI6295         |
|                                                                  |
|                                   +---------------+              |
|                                   |               |              |
|J                   +-------+      |               |              |
|A                   | VRAML |      | QuickLogic    |  14.31818MHz |
|M                   +-------+      | QL2003-XPL84C |              |
|M                   +-------+      | 9819 BA       |   +-----+    |
|A                   | VRAMU |      |               |   |93C46|    |
|                    +-------+      +---------------+   +-----+    |
|C                                                                 |
|O                                      +---------+   +---------+  |
|N                                      |   L00   |   |   U00   |  |
|N                                      |         |   |         |  |
|E                                      +---------+   +---------+  |
|C                   +------------+     +---------+   +---------+  |
|T                   |            |     |   L01   |   |   U01   |  |
|O                   |            |     |         |   |         |  |
|R                   | HyperStone |     +---------+   +---------+  |
|                    |  E1-32XN   |     +---------+   +---------+  |
|                    |            |     |   L02   |   |   U02   |  |
|          +-----+   |            |     |         |   |         |  |
|          |DRAML|   +------------+     +---------+   +---------+  |
+-+        +-----+                      +---------+   +---------+  |
  |        +-----+               20MHz  |   L03   |   |   U03   |  |
+-+        |DRAMU|                      |         |   |         |  |
|          +-----+    +----------+      +---------+   +---------+  |
|  +--+ +--+          |   ROM1   |                                 |
|  |S3| |S1|          +----------+                                 |
+------------------------------------------------------------------+

S3 is a reset button
S1 is the setup button

VRAML & VRAMU are KM6161002CJ-12
DRAML & DRAMU are GM71C18163CJ6

ROM1 & SND are stardard 27C040 and/or 27C020 eproms
L00-L03 & U00-U03 are 29F1610ML Flash roms


todo: royalpk2 layout (it's very different)
*/

ROM_START( mosaicf2 ) /* Released October 1999 */
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "rom1.bin",            0x80000, 0x080000, CRC(fceb6f83) SHA1(b98afb477627c3b2d584c0f0fb26c4dd5b1a31e2) )

	ROM_REGION32_BE( 0x1000000, "user2", 0 )  /* gfx data */
	ROM_LOAD32_WORD_SWAP( "u00.bin", 0x000000, 0x200000, CRC(a2329675) SHA1(bff8974fab9120274821c9c9646744317f47c79c) )
	ROM_LOAD32_WORD_SWAP( "l00.bin", 0x000002, 0x200000, CRC(d96fe93b) SHA1(005d9889077825fc0e308d2981f6fca5e6b51fe8) )
	ROM_LOAD32_WORD_SWAP( "u01.bin", 0x400000, 0x200000, CRC(6379e73f) SHA1(fe5abafbcbd828795cb06a08763fae1bbe2a75ad) )
	ROM_LOAD32_WORD_SWAP( "l01.bin", 0x400002, 0x200000, CRC(a269ea82) SHA1(d962a8b3293c6f46dbefa49859b2b3e594e7a386) )
	ROM_LOAD32_WORD_SWAP( "u02.bin", 0x800000, 0x200000, CRC(c17f95cd) SHA1(1c701185be138b615d2851866288647f40809c28) )
	ROM_LOAD32_WORD_SWAP( "l02.bin", 0x800002, 0x200000, CRC(69cd9c5c) SHA1(6b4d204a6ab5f36dfba9053bb3be2d094fcfdd00) )
	ROM_LOAD32_WORD_SWAP( "u03.bin", 0xc00000, 0x200000, CRC(0e47df20) SHA1(6f6c3e7fc8c99db7ddc73d8d10a661373bb72a1a) )
	ROM_LOAD32_WORD_SWAP( "l03.bin", 0xc00002, 0x200000, CRC(d79f6ca8) SHA1(4735dda9269aa05ba1251d335dc73914f5cb43b0) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "snd.bin",             0x000000, 0x040000, CRC(4584589c) SHA1(5f9824724f840767c3dc1dc04b203ddf3d78b84c) )
ROM_END

ROM_START( royalpk2 )
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prog1",            0x80000, 0x080000, CRC(e1546304) SHA1(b628b347ba7fbbae948e98e72aa5ea190c5d0f2b) )

	ROM_REGION32_BE( 0x800000, "user2", 0 )  /* gfx data */
	ROM_LOAD32_WORD_SWAP( "1.u00", 0x000000, 0x200000, CRC(b397a805) SHA1(3fafa8533c793f41d0567b76667d3f3478eb9c1d) )
	ROM_LOAD32_WORD_SWAP( "2.l00", 0x000002, 0x200000, CRC(83a67d20) SHA1(9bf4c3da0cd1aab2488f260f694493d8ee25883e) )
	ROM_LOAD32_WORD_SWAP( "3.u01", 0x400000, 0x200000, CRC(f7b9d508) SHA1(5d98687c6cf158df8134d88d3726778d3762b411) )
	ROM_LOAD32_WORD_SWAP( "4.l01", 0x400002, 0x200000, CRC(dcff4960) SHA1(f742c7a3b62262c4b0210db9df03f51b3f600bf2) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "snd2",             0x000000, 0x080000, CRC(f25e3315) SHA1(ce5350ecba6769b17bb01d82b55f26ded4d51773) )
ROM_END


GAME( 1999, mosaicf2, 0, mosaicf2, mosaicf2, mosaicf2_state, empty_init, ROT0, "F2 System", "Mosaic (F2 System)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, royalpk2, 0, royalpk2, royalpk2, mosaicf2_state, empty_init, ROT0, "F2 System", "Royal Poker 2 (Network version 3.12)", MACHINE_NOT_WORKING )
