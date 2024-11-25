// license:BSD-3-Clause
// copyright-holders:

/*
Unknown game running on a PCB with the following components:
Z80 (a second one is probably inside the box on the daughter board)
3 x D8255AC-2
HD63310P20 S-DPRAM
12.00 crystal
3.579545 crystal (near YM2413)
10 x 6116
5816
YM2413
3 x 8-dip banks
1 reset push button

The daughter card has a big box on it labelled as follows:
  Cointek logo
  Cointek, Technique
  Mask Microcomputer
  CODE NUM

The daughter board is connected to the main board via 40 pin socket.

TODO:
- Decryption may be incomplete, though code flow looks sane both for the main CPU and the audio CPU.
- The game puts some strings at 0xc000 and at 0xf810. At 0xf810 it puts 'MICRO' in DPRAM and then it expects to read 'DRAGON' (put in DPRAM by the audio CPU),
  if it doesn't it loops endlessly. For reasons to be investigated the audio CPU puts it at a slightly wrong offset.
- HD63310 seems to have more features than the DPRAM chips emulated in machine/mb8421.cpp. Maybe reason for the above problem?
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/mb8421.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class cointek_state : public driver_device
{
public:
	cointek_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{
	}

	void cointek(machine_config &config);

	void init_unkct();

private:
	required_shared_ptr<uint8_t> m_decrypted_opcodes;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void audio_io_map(address_map &map) ATTR_COLD;
	void audio_opcodes_map(address_map &map) ATTR_COLD;
	void audio_prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
};

uint32_t cointek_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void cointek_state::prg_map(address_map &map)
{
	map(0x0000, 0xbfff).rom(); // banking?
	map(0xc000, 0xc7ff).ram();
	map(0xf800, 0xfbff).rw("dpram", FUNC(idt7130_device::right_r), FUNC(idt7130_device::right_w));
	map(0xfc00, 0xffff).ram();
}

void cointek_state::io_map(address_map &map)
{
}

void cointek_state::audio_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
}

void cointek_state::audio_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_decrypted_opcodes);
}

void cointek_state::audio_io_map(address_map &map)
{
	//map(0x0040, 0x0043).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
	//map(0x8000, 0x80ff).rw("dpram", FUNC(idt7130_device::left_r), FUNC(idt7130_device::left_w)); // TODO: seems to write the required DRAGON string here but in the wrong place. And why only 0x100?
}


static INPUT_PORTS_START( unkct )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWA:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSWA:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSWA:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSWA:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWA:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWA:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWA:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSWA:8")

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWB:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSWB:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSWB:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSWB:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWB:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWB:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWB:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSWB:8")

	PORT_START("DSWC")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWC:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSWC:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSWC:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSWC:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWC:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWC:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWC:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSWC:8")
INPUT_PORTS_END

const gfx_layout gfx_8x8x4 = // TODO: not correct but it allows to see something in the GFX viewer for now
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	8*8
};

static GFXDECODE_START( gfx_unkct )
	GFXDECODE_ENTRY("unsorted_gfx", 0, gfx_8x8x4, 0, 32)
GFXDECODE_END


void cointek_state::cointek(machine_config &config)
{
	// basic machine hardware
	z80_device &maincpu(Z80(config, "maincpu", 12_MHz_XTAL / 3)); // presumably under black box, divisor guessed
	maincpu.set_addrmap(AS_PROGRAM, &cointek_state::prg_map);
	maincpu.set_addrmap(AS_IO, &cointek_state::io_map);

	z80_device &audiocpu(Z80(config, "audiocpu", 12_MHz_XTAL / 3)); // divisor guessed
	audiocpu.set_addrmap(AS_PROGRAM, &cointek_state::audio_prg_map);
	audiocpu.set_addrmap(AS_OPCODES, &cointek_state::audio_opcodes_map);
	audiocpu.set_addrmap(AS_IO, &cointek_state::audio_io_map);

	I8255(config, "ppi1");
	I8255(config, "ppi2");
	I8255(config, "ppi3");

	IDT7130(config, "dpram"); // actually HD63310P20
	//dpram.intr_callback().set_inputline("audiocpu", 0);
	//dpram.intl_callback().set_inputline("maincpu", 0);

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(cointek_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_unkct);
	PALETTE(config, "palette").set_entries(0x100);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	YM2413(config, "ym", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.30);
}


ROM_START( unkct ) // all labels handwritten but with Cointek logo
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "glb26.v5", 0x0000, 0x8000, CRC(fed184c5) SHA1(ae52487236b190edb6b49f90a2cc02d53f1ff922) ) // at 0x60 it contains MICRO DRAGON string, at 0x4900 Software by Micro Dragon (both scrambled)
	ROM_LOAD( "glb36.x5", 0x8000, 0x8000, CRC(32c68da2) SHA1(bfc30e28cc4d9a8dc4050922de9cb95081e27583) ) // at 0xbe00 it contains COINTEK ENTERPRISE CORP string (not scrambled)

	ROM_REGION(0x8000, "audiocpu", 0)
	ROM_LOAD( "glb14.s9", 0x0000, 0x8000, CRC(836f4b43) SHA1(893f5acf9af5353eb5047d4bd35ef6ee6205fe14) ) // at 0x2f90 it contains MICRODRAGON string (backwards, not scrambled)

	ROM_REGION(0x80000, "unsorted_gfx", 0)
	ROM_LOAD( "gl1.b1",  0x00000, 0x8000, CRC(e9f21aa1) SHA1(b2e4095195cde696fb1f5ad98f605e9e5ae06277) )
	ROM_LOAD( "gl5.b3",  0x08000, 0x8000, CRC(1ea3170e) SHA1(42c84aac2ebc129b5646f904bd08ec1e5d76a1b1) )
	ROM_LOAD( "gl9.b4",  0x10000, 0x8000, CRC(0c19e70e) SHA1(e7def24449fab7076eb240bd433481a76cfebc0e) )
	ROM_LOAD( "gl13.b6", 0x18000, 0x8000, CRC(8b4ba340) SHA1(2455f93c2b44b6c7ece9efb08c5f68f42bdfecba) )
	ROM_LOAD( "gl2.e1",  0x20000, 0x8000, CRC(b06395c5) SHA1(d05dd9fc1e45d1c03e59f9fef96feb223d56c7c6) )
	ROM_LOAD( "gl6.e3",  0x28000, 0x8000, CRC(57b93b09) SHA1(d4b0d20683dd9719f803a677cfbf4fcd3bd5f24f) )
	ROM_LOAD( "gl10.e4", 0x30000, 0x8000, CRC(a0bc38a0) SHA1(6ca6276e83d0c2502d7db300b4c62ea73327ebee) )
	ROM_LOAD( "gl14.e6", 0x38000, 0x8000, CRC(e4aca360) SHA1(dd2424d01de0bcb79d820f0a341931255d9edb88) )
	ROM_LOAD( "gl3.h1",  0x40000, 0x8000, CRC(a3f57d30) SHA1(feb596247d7d13a790f185e6927c621343cd8289) )
	ROM_LOAD( "gl7.h3",  0x48000, 0x8000, CRC(37e2f9cc) SHA1(d76ae005f162e9cf925526bdb542e6b6a9e581b5) )
	ROM_LOAD( "gl11.h4", 0x50000, 0x8000, CRC(8e049dd8) SHA1(9f37594108ffa7ca9042182c58fa044e4dd09e69) )
	ROM_LOAD( "gl15.h6", 0x68000, 0x8000, CRC(2b920af9) SHA1(64fb754f5898e646955a907c8daac3171c9e0c25) )
	ROM_LOAD( "gl4.k1",  0x60000, 0x8000, CRC(15aaa4a4) SHA1(fe7cf2685ed3ed6fababcd38eca4de53fd98f729) )
	ROM_LOAD( "gl8.k3",  0x68000, 0x8000, CRC(8b75e743) SHA1(883546a819caa88e159868c34818e55a4c372960) )
	ROM_LOAD( "gl12.k4", 0x70000, 0x8000, CRC(b0f18634) SHA1(39c5c16f6948a921440b35e5a1114dc9b1de9904) )
	ROM_LOAD( "gl16.k6", 0x78000, 0x8000, CRC(512fcbcc) SHA1(60daf0fe90fbda81f62ec40f5fb437f7bcfa06a6) )

	ROM_REGION(0x600, "unsorted_proms", 0)
	ROM_LOAD( "sn82s129n.a9",  0x000, 0x100, CRC(1fb14640) SHA1(a0eb126f813fc39710e044f253e37a70cfa9afb5) )
	ROM_LOAD( "sn82s129n.a10", 0x100, 0x100, CRC(458ca6e2) SHA1(e195444db4541c7a99cb68c51483f67837e36ae7) )
	ROM_LOAD( "sn82s129n.b9",  0x200, 0x100, CRC(13dfde1e) SHA1(09d9261c9a0b4f947d9cca54d0137dd15df3ee47) )
	ROM_LOAD( "sn82s129n.b10", 0x300, 0x100, CRC(bf3847e6) SHA1(18a02d38c54199aa40d65b236012ef181371e7c4) )
	ROM_LOAD( "sn82s129n.c9",  0x400, 0x100, CRC(dd44dec3) SHA1(b756526ae1bc65beacec4c6ab6ee7e81003e9ab8) )
	ROM_LOAD( "sn82s129n.c10", 0x500, 0x100, CRC(ac542251) SHA1(03012e968464693840cfb3c4e259d5545da3b549) )

	ROM_REGION(0x1500, "plds", 0)
	ROM_LOAD( "pal16l8acn.n7",   0x0000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8acn.p7",   0x0200, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8acn.s7",   0x0400, 0x104, NO_DUMP )
	ROM_LOAD( "tibpal16l8.s8",   0x0600, 0x104, NO_DUMP )
	ROM_LOAD( "pal20l10acns.t3", 0x0800, 0x0cc, NO_DUMP )
	ROM_LOAD( "pal20l10acns.u3", 0x0900, 0x0cc, NO_DUMP )
	ROM_LOAD( "pal20l8acns.u1",  0x0a00, 0x144, NO_DUMP )
	ROM_LOAD( "gal20v8.v1",      0x0c00, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8.r4",      0x0e00, 0x157, NO_DUMP )
	ROM_LOAD( "pal20r8acns.x4",  0x1000, 0x144, NO_DUMP )
	ROM_LOAD( "pal20l8acns.y4",  0x1200, 0x144, NO_DUMP )
	ROM_LOAD( "pal20l10acns.y8", 0x1400, 0x0cc, NO_DUMP )
ROM_END

void cointek_state::init_unkct()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x10000; i++) // TODO: seems good but needs verifying
		if (!(i & 0x1000))
			rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 5, 2, 1, 0);

	uint8_t *audiorom = memregion("audiocpu")->base();

	for (int i = 0; i < 0x8000; i++) // TODO: seems good but needs verifying
		m_decrypted_opcodes[i] = bitswap<8>(audiorom[i], 0, 6, 5, 4, 3, 2, 1, 7);
}

} // anonymous namespace


GAME( 1989, unkct, 0, cointek, unkct, cointek_state, init_unkct, ROT0, "Cointek Enterprise Corp", "unknown Cointek game", MACHINE_IS_SKELETON ) // string in ROM at 0x7839: Ver 4.00 1989-08-01
