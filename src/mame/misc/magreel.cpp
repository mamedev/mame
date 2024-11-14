// license:BSD-3-Clause
// copyright-holders: Scott Stone

/***************************************************************************

Magic Reels - Casino Slots
Play System

1x XILINX XCS30 PQ240CKN9825 A2015802A
1x MC68HC000FN8
1x XILINX XC9572 TQ100AEM9917
1x XILINX XC95108 TQ100AEM9909
1x XILINX XC9536 VQ44ASJ9917
2x GM76C8128CLLFW70
4x GM76C256CLLFW70
2x TD62083AP
1x TL7705ACP
1x HIN232CP

1x XTAL: 58.00000Mhz

UNDUMPED:
1x PIC16C621A  ==> PROTECTED

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/pic16c62x/pic16c62x.h"

#include "screen.h"
#include "speaker.h"


namespace {

class magreel_state : public driver_device
{
public:
	magreel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void magreel(machine_config &config);

	void init_magreel();

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}
};


void magreel_state::mem_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
}


static INPUT_PORTS_START( magreel )
	// dummy active high structure
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	// dummy active low structure
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void magreel_state::machine_start()
{
}


void magreel_state::machine_reset()
{
}


void magreel_state::magreel(machine_config &config)
{
	m68000_device &maincpu(M68000(config, "maincpu", 58_MHz_XTAL / 12));
	maincpu.set_addrmap(AS_PROGRAM, &magreel_state::mem_map);

	PIC16C621A(config, "pic", 58_MHz_XTAL / 12).set_disable(); // clock unverified

	/* video hardware */
//  screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_refresh_hz(60);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
//  screen.set_screen_update(FUNC(magreel_state::screen_update));
//  screen.set_size(32*8, 32*8);
//  screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
//  screen.set_raw(58_MHz_XTAL / 2, 442, 0, 320, 264, 0, 240);          // generic NTSC video timing at 320x240
//  screen.set_palette("palette");

//  GFXDECODE(config, "gfxdecode", "palette", gfx_magreel);

//  PALETTE(config, "palette", FUNC(magreel_state::magreel), 8)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( magreel )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "m27c800.ic18",  0x000000, 0x100000, CRC(2af3d8e7) SHA1(729cd2c1011d8018cf8d77c2d118d1815e30f475) ) // TODO: figure out the line swapping

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF )
	ROM_LOAD( "pic16c621", 0x0000, 0x4000, NO_DUMP ) // read protected

	ROM_REGION( 0x800000, "reels", 0 )
	ROM_LOAD( "m27c160.ic3",  0x000000, 0x200000, CRC(707a835a) SHA1(4edbb2279298f330514512147166b9382c79861d) )
	ROM_LOAD( "m27c160.ic4",  0x200000, 0x200000, CRC(d5590a3c) SHA1(69dacb370b630fd7fee3ddd4beeb34a336dd2d16) )
	ROM_LOAD( "m27c160.ic5",  0x400000, 0x200000, CRC(72da9809) SHA1(19516432c4cfc33c3db20aab0c64fafb72ed1a19) )
	ROM_LOAD( "m27c160.ic6",  0x600000, 0x200000, CRC(0f3274d0) SHA1(1abb45ebc74a09f1832cf80775a35966e8d5cd84) )

	ROM_REGION( 0x200000, "flash", 0 )
	ROM_LOAD( "mx29f161.ic24",0x000000, 0x200000, CRC(b479c2db) SHA1(cac4c38ef26d307bfd3ffff77859ea90cf554418) )

	ROM_REGION( 0x4000, "eeproms", 0 )
	ROM_LOAD( "m28c64.ic19",  0x000000, 0x002000, CRC(d0238e5c) SHA1(513bb97487d33c3b844877104bb2af3220851583) )
	ROM_LOAD( "m28c64.ic20",  0x002000, 0x002000, CRC(4e6abd42) SHA1(5b1741b755f0fddd94e16d41d5d39a03f37fb23b) )
ROM_END


void magreel_state::init_magreel()
{
	uint8_t *rom = memregion("maincpu")->base();

	std::vector<uint8_t> buffer(0x100000);

	memcpy(&buffer[0], rom, 0x100000);

	// descramble address. TODO: gives M68000-looking code structure, but probably isn't correct
	for (int i = 0; i < 0x100000; i++)
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 16, 17, 18, 1, 3, 2, 4, 6, 5, 7, 8, 9, 10, 11, 14, 12, 13, 15, 0)];

	uint16_t *rom16 = (uint16_t *)memregion("maincpu")->base();

	// descramble data. TODO: everything
	for (int i = 0; i < 0x100000 / 2; i++)
		rom16[i] = bitswap<16>(rom16[i] ^ 0x0000, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
}

} // anonymous namespace

GAME( 199?, magreel, 0, magreel, magreel, magreel_state, init_magreel, ROT0, "Play System", "Magic Reels", MACHINE_IS_SKELETON )
