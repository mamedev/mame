// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
	unclear if there is enough here to make a working driver (or if the resources match the program ROMS)

	uses a T11, but doesn't seem to be System 2 hardware

	code ends up doing this (why?)

	100170: MOV   #050000,R3
	100174: MOV   #054000,R4
	100200: .WORD 000017
	000000: HALT

	no text/character ROM despite text in ROM (is data uploaded, or is it missing?)
	no bg gfx either?

*/

#include "emu.h"

#include "cpu/t11/t11.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class pakrat11_state : public driver_device
{
public:
	pakrat11_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void pakrat11(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<t11_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
};

void pakrat11_state::video_start()
{
}

uint32_t pakrat11_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void pakrat11_state::prg_map(address_map &map)
{
	map(0000000, 0007777).ram();
	map(0010000, 0017777).ram();
	map(0020000, 0027777).ram();
	map(0030000, 0037777).ram();

	map(0100000, 0177777).rom();
}

static INPUT_PORTS_START( pakrat11 )
INPUT_PORTS_END

static const gfx_layout tile_layout16 =
{
	8,16,
	RGN_FRAC(1,1),
	4,
	{ 0,4,8,12 },
	{ 0,1,2,3,16,17,18,19 },
	{ STEP16(0,32) },
	16*32
};

static const gfx_layout tile_layout32 =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0,4,8,12 },
	{ 0,1,2,3,16,17,18,19 },
	{ STEP32(0,32) },
	32*32
};

static GFXDECODE_START( gfx_pakrat11 )
	GFXDECODE_ENTRY( "tiles", 0, tile_layout16, 0, 1 )
	GFXDECODE_ENTRY( "tiles", 0, tile_layout32, 0, 1 )
GFXDECODE_END

void pakrat11_state::machine_start()
{
}

void pakrat11_state::machine_reset()
{
}

void pakrat11_state::pakrat11(machine_config &config)
{
	T11(config, m_maincpu, 10'000'000); // ? MHz
	m_maincpu->set_initial_mode(0x36ff); // ?
	m_maincpu->set_addrmap(AS_PROGRAM, &pakrat11_state::prg_map);

	// wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(pakrat11_state::screen_update));
	screen.set_palette("palette");

	// wrong
	GFXDECODE(config, m_gfxdecode, "palette", gfx_pakrat11);
	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 0x100).set_endianness(ENDIANNESS_BIG);
}

ROM_START( pakrat11 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pakprogram.l1", 0x008000, 0x004000, CRC(5271a63f) SHA1(3fff2fab0ed70579844004f309f60761f479252e) )
	ROM_LOAD( "pakprogram.h1", 0x00c000, 0x004000, CRC(f92a94c0) SHA1(9f8849e5ea34be734c524a633e60167828941915) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pakaudio-4k.bin", 0x004000, 0x004000, CRC(61b5ecaa) SHA1(22eedb837a09d8adb54310cc1479d38ccac0d36d) )
	ROM_LOAD( "pakaudio-8k.bin", 0x008000, 0x004000, CRC(aeabb55a) SHA1(3e3558ac3b580b9dff0c21c525797a963cf32eaa) )
	ROM_LOAD( "pakaudio-ck.bin", 0x00c000, 0x004000, CRC(34a0442b) SHA1(8e4a7e899209a2030b94986bb247942f516716a0) )

	ROM_REGION( 0x50000, "tiles", 0 ) // are these even for the prototype?
	ROM_LOAD16_BYTE( "actn0.low",   0x00000, 0x4000, CRC(4efe7e31) SHA1(ee5bf301df2b2cae08197d6636ceb5a0efe0ec78) )
	ROM_LOAD16_BYTE( "actn0.hig",   0x00001, 0x4000, CRC(91dbf695) SHA1(e88aa92c19a720536a7e10a4217fafa6ad51a952) )
	ROM_LOAD16_BYTE( "actn1.low",   0x08000, 0x4000, CRC(f0265530) SHA1(807b6f026ccf28e142fc9d2d0dec4052474965d4) )
	ROM_LOAD16_BYTE( "actn1.hig",   0x08001, 0x4000, CRC(a7863c21) SHA1(cb04aaf7c70fa7f5e1cd015f7a16c544ec863358) )
	ROM_LOAD16_BYTE( "buldog.low",  0x10000, 0x4000, CRC(660733fb) SHA1(2cbf521ec1c92a6c967681c081cd99c516ca74b9) )
	ROM_LOAD16_BYTE( "buldog.hig",  0x10001, 0x4000, CRC(16546ef0) SHA1(2bc9e8d18c0794bf19f92327da2e820056aa5f4d) )
	ROM_LOAD16_BYTE( "obj.low",     0x18000, 0x4000, CRC(2fc468e6) SHA1(a250228ee0e718148627abff22b39764e5e63cf2) )
	ROM_LOAD16_BYTE( "obj.hig",     0x18001, 0x4000, CRC(83544432) SHA1(575c318357afffbf4bbfe64ecf6470d9bb88b47a) )
	ROM_LOAD16_BYTE( "packrat.low", 0x20000, 0x4000, CRC(2c9163e5) SHA1(5c921483ffb6c6597970a0569115e039b2cba662) )
	ROM_LOAD16_BYTE( "packrat.hig", 0x20001, 0x4000, CRC(fb302123) SHA1(7a3991df73d94160a139c182d07be47652489dcc) )
	ROM_LOAD16_BYTE( "pakall.low",  0x28000, 0x4000, CRC(8ad92683) SHA1(7c751122bf7d0c37d105dfbfb248484156fbda44) )
	ROM_LOAD16_BYTE( "pakall.hig",  0x28001, 0x4000, CRC(e97c800a) SHA1(b4f7c286a9a13841e60b5137650db72c6476fb09) )
	ROM_LOAD16_BYTE( "pakspi.low",  0x30000, 0x4000, CRC(1ddc4441) SHA1(62ce32aaa6559992fd48761b319963f89d62971b) )
	ROM_LOAD16_BYTE( "pakspi.hig",  0x30001, 0x4000, CRC(2a48381b) SHA1(1f0fb3645917113d73b0f82020943ba8de17c2ff) )
	ROM_LOAD16_BYTE( "stooj.low",   0x38000, 0x4000, CRC(262c0f46) SHA1(4715b283967923f00b275339db59d4cdc1ed1627) )
	ROM_LOAD16_BYTE( "stooj.hig",   0x38001, 0x4000, CRC(c0296f76) SHA1(9acec6fe873352f17a53846796f8d7057aceac78) )
	ROM_LOAD16_BYTE( "tough.low",   0x40000, 0x4000, CRC(bd4d4421) SHA1(8b1d24b0a4654ac95bd8c7aa34570d3cfcdbef42) )
	ROM_LOAD16_BYTE( "tough.hig",   0x40001, 0x4000, CRC(7252b9a2) SHA1(f83d8bf1eb164954fc3d0c9b46302c196b96e6a2) )
	ROM_LOAD16_BYTE( "walk.low",    0x48000, 0x4000, CRC(357863e4) SHA1(3f82a98a8c12c2a306455a0926de500a54b06b3d) )
	ROM_LOAD16_BYTE( "walk.hig",    0x48001, 0x4000, CRC(5ddac992) SHA1(29a51eefd5025ae302ea7b58956fdcbc6364febc) )

	// where are the text/background ROMs?
ROM_END

ROM_START( pakrat11a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pakprogram.l2", 0x008000, 0x004000, CRC(fdb3910b) SHA1(f7d50e1bbaf9370e3172f6679ddf061d084acd87) ) // only this ROM differs
	ROM_LOAD( "pakprogram.h2", 0x00c000, 0x004000, CRC(f92a94c0) SHA1(9f8849e5ea34be734c524a633e60167828941915) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pakaudio-4k.bin", 0x004000, 0x004000, CRC(61b5ecaa) SHA1(22eedb837a09d8adb54310cc1479d38ccac0d36d) )
	ROM_LOAD( "pakaudio-8k.bin", 0x008000, 0x004000, CRC(aeabb55a) SHA1(3e3558ac3b580b9dff0c21c525797a963cf32eaa) )
	ROM_LOAD( "pakaudio-ck.bin", 0x00c000, 0x004000, CRC(34a0442b) SHA1(8e4a7e899209a2030b94986bb247942f516716a0) )

	ROM_REGION( 0x50000, "tiles", 0 ) // are these even for the prototype?
	ROM_LOAD16_BYTE( "actn0.low",   0x00000, 0x4000, CRC(4efe7e31) SHA1(ee5bf301df2b2cae08197d6636ceb5a0efe0ec78) )
	ROM_LOAD16_BYTE( "actn0.hig",   0x00001, 0x4000, CRC(91dbf695) SHA1(e88aa92c19a720536a7e10a4217fafa6ad51a952) )
	ROM_LOAD16_BYTE( "actn1.low",   0x08000, 0x4000, CRC(f0265530) SHA1(807b6f026ccf28e142fc9d2d0dec4052474965d4) )
	ROM_LOAD16_BYTE( "actn1.hig",   0x08001, 0x4000, CRC(a7863c21) SHA1(cb04aaf7c70fa7f5e1cd015f7a16c544ec863358) )
	ROM_LOAD16_BYTE( "buldog.low",  0x10000, 0x4000, CRC(660733fb) SHA1(2cbf521ec1c92a6c967681c081cd99c516ca74b9) )
	ROM_LOAD16_BYTE( "buldog.hig",  0x10001, 0x4000, CRC(16546ef0) SHA1(2bc9e8d18c0794bf19f92327da2e820056aa5f4d) )
	ROM_LOAD16_BYTE( "obj.low",     0x18000, 0x4000, CRC(2fc468e6) SHA1(a250228ee0e718148627abff22b39764e5e63cf2) )
	ROM_LOAD16_BYTE( "obj.hig",     0x18001, 0x4000, CRC(83544432) SHA1(575c318357afffbf4bbfe64ecf6470d9bb88b47a) )
	ROM_LOAD16_BYTE( "packrat.low", 0x20000, 0x4000, CRC(2c9163e5) SHA1(5c921483ffb6c6597970a0569115e039b2cba662) )
	ROM_LOAD16_BYTE( "packrat.hig", 0x20001, 0x4000, CRC(fb302123) SHA1(7a3991df73d94160a139c182d07be47652489dcc) )
	ROM_LOAD16_BYTE( "pakall.low",  0x28000, 0x4000, CRC(8ad92683) SHA1(7c751122bf7d0c37d105dfbfb248484156fbda44) )
	ROM_LOAD16_BYTE( "pakall.hig",  0x28001, 0x4000, CRC(e97c800a) SHA1(b4f7c286a9a13841e60b5137650db72c6476fb09) )
	ROM_LOAD16_BYTE( "pakspi.low",  0x30000, 0x4000, CRC(1ddc4441) SHA1(62ce32aaa6559992fd48761b319963f89d62971b) )
	ROM_LOAD16_BYTE( "pakspi.hig",  0x30001, 0x4000, CRC(2a48381b) SHA1(1f0fb3645917113d73b0f82020943ba8de17c2ff) )
	ROM_LOAD16_BYTE( "stooj.low",   0x38000, 0x4000, CRC(262c0f46) SHA1(4715b283967923f00b275339db59d4cdc1ed1627) )
	ROM_LOAD16_BYTE( "stooj.hig",   0x38001, 0x4000, CRC(c0296f76) SHA1(9acec6fe873352f17a53846796f8d7057aceac78) )
	ROM_LOAD16_BYTE( "tough.low",   0x40000, 0x4000, CRC(bd4d4421) SHA1(8b1d24b0a4654ac95bd8c7aa34570d3cfcdbef42) )
	ROM_LOAD16_BYTE( "tough.hig",   0x40001, 0x4000, CRC(7252b9a2) SHA1(f83d8bf1eb164954fc3d0c9b46302c196b96e6a2) )
	ROM_LOAD16_BYTE( "walk.low",    0x48000, 0x4000, CRC(357863e4) SHA1(3f82a98a8c12c2a306455a0926de500a54b06b3d) )
	ROM_LOAD16_BYTE( "walk.hig",    0x48001, 0x4000, CRC(5ddac992) SHA1(29a51eefd5025ae302ea7b58956fdcbc6364febc) )

	// where are the text/background ROMs?
ROM_END


} // anonymous namespace

// internal strings just call this Packrat! not Peter Pack Rat
GAME( 198?, pakrat11,   peterpak, pakrat11, pakrat11,  pakrat11_state, empty_init, ROT0, "Atari Games", "Peter Pack Rat (T11 hardware, prototype, set 1)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 198?, pakrat11a,  peterpak, pakrat11, pakrat11,  pakrat11_state, empty_init, ROT0, "Atari Games", "Peter Pack Rat (T11 hardware, prototype, set 2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
