// license:BSD-3-Clause
// copyright-holders:

/*
    Blood Bros. (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 8 main boards and 1 sub board.

    MOD-6/1 - MC68000P10, 6 ROMs, RAMs, 22.1184 MHz XTAL.
    MOD 21/1(?) - 24 MHz XTAL.
    MOD 1/2 - Sound board (Z80, 2xYM2203C). 2 8-dips banks + small sub board with OKI M5205.
    MOD 51/1 - Sprite board, has logic + 4 empty ROM sockets. Sprite ROMs are actually on the below board.
    MODULAR SYSTEM 2 - red sprite ROM board, 16 sprite ROMs populated (maximum 24 ROMs)
    MOD 4/3 - Tilemap board, has logic + NO ROMs populated, long thin sub-board (CAR-0484/1 SOLD) with no chips, just routing along one edge.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (CAR-0484/1 SOLD) with no chips, just routing along one edge.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (CAR-0484/1 SOLD) with no chips, just routing along one edge.

    PCBs pictures and dip listing are available at: http://www.recreativas.org/modular-system-blood-bros-4316-gaelco-sa
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class bloodbro_ms_state : public driver_device
{
public:
	bloodbro_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void bloodbrom(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bloodbrom_map(address_map &map);
};


void bloodbro_ms_state::bloodbrom_map(address_map &map)
{
	map(0x000000, 0x7ffff).rom();
}


void bloodbro_ms_state::machine_start()
{
}


uint32_t bloodbro_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( bloodbrom )
INPUT_PORTS_END

static GFXDECODE_START( gfx_bloodbro_ms )
GFXDECODE_END

void bloodbro_ms_state::bloodbrom(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 22.1184_MHz_XTAL / 2); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &bloodbro_ms_state::bloodbrom_map);

	Z80(config, "audiocpu", 24_MHz_XTAL / 8).set_disable(); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(bloodbro_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBRG_444, 1024);

	GFXDECODE(config, "gfxdecode", "palette", gfx_bloodbro_ms);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ym1", 24_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one

	YM2203(config, "ym2", 24_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one

	MSM5205(config, "msm", 24_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one
}

ROM_START( bloodbrom )
	ROM_REGION( 0x100000, "maincpu", 0 ) // on MOD 6/1 board
	ROM_LOAD16_BYTE( "6-1_bb606.ic8",   0x00001, 0x10000, CRC(3c069061) SHA1(537a10376ad24537367fb221817789bdc31787fa) )
	ROM_LOAD16_BYTE( "6-1_bb605.ic11",  0x20001, 0x10000, CRC(2dc3fb8c) SHA1(44e8e4136979464101385531f97cce27abe1de34) )
	ROM_LOAD16_BYTE( "6-1_bb603.ic17",  0x00000, 0x10000, CRC(10f4c8e9) SHA1(e5c078395b70b73d21c100c6b60cff89e4668473) )
	ROM_LOAD16_BYTE( "6-1_bb602.ic20",  0x10000, 0x10000, CRC(8e507cce) SHA1(93bef8838cf8f73eb158dfe276f53c29f364fd45) )
	ROM_LOAD16_BYTE( "6-1_bb604.ic25",  0x40001, 0x10000, CRC(cc069a40) SHA1(314b27cde5427b285272840f41da097326b39ee9) )
	ROM_LOAD16_BYTE( "6-1_bb601.ic26",  0x40000, 0x10000, CRC(d06bf68d) SHA1(7df7a99805aa7dd2ad91fb3d641e369c058cc6ae) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/2 board
	ROM_LOAD( "1-2_bb101.ic12",  0x00000, 0x10000, CRC(3e184e74) SHA1(031cd37fe6d09daf8c9e88562da99fde03f52109) )

	// dumper's note: ROMs [bb4b1, bb4b2, bb4b3, bb4b4] and [bb4a1, bb4a2, bb4a3, bb4a4] have a strange setup
	// with pins 32, 31 and 31 soldered together and pin 2 connected between all four chips,
	// while the sockets are for 28 pin chips (with 27C512 silkscreened on the PCB behind the chips)
	ROM_REGION( 0x80000, "gfx1", 0 ) // on one of the MOD 4/3 boards
	ROM_LOAD( "4-3-a_bb4a1.ic17",  0x00000, 0x20000, CRC(499c91db) SHA1(bd7142a311a4f3e606f8a31aafc0b504f3d5a2e4) )
	ROM_LOAD( "4-3-a_bb4a2.ic16",  0x20000, 0x20000, CRC(e8f87153) SHA1(f4147c971d1c66e7c6133c6318357ced7e30e217) )
	ROM_LOAD( "4-3-a_bb4a3.ic15",  0x40000, 0x20000, CRC(13b888f2) SHA1(7a53f78f22a09fe4db45c36bf3912ad379deca64) )
	ROM_LOAD( "4-3-a_bb4a4.ic14",  0x60000, 0x20000, CRC(19bc0508) SHA1(01c4eb570dc7ba9401085012d23bdb865df78029) )

	ROM_REGION( 0x80000, "gfx2", 0 ) // on another MOD 4/3 board
	ROM_LOAD( "4-3-b_bb4b1.ic17",  0x00000, 0x20000, CRC(aa86ae59) SHA1(c15a78eaaca36bebd3261cb2c4a2c232b967a135) )
	ROM_LOAD( "4-3-b_bb4b2.ic16",  0x20000, 0x20000, CRC(f25dd182) SHA1(eff29970c7b898744b08a151f9e17b68ce77e78d) )
	ROM_LOAD( "4-3-b_bb4b3.ic15",  0x40000, 0x20000, CRC(3efcb6aa) SHA1(0a162285d08e171e946147e0725db879643ae113) )
	ROM_LOAD( "4-3-b_bb4b4.ic14",  0x60000, 0x20000, CRC(6b5254fa) SHA1(1e9e3096e5f29554fb8f8cb0df0e5157f940f8c9) )

	ROM_REGION( 0x100000, "gfx3", 0 ) // on MOD 51/1 board
	ROM_LOAD( "51-1-b_bb503.ic3",   0x00000, 0x10000, CRC(9d2a382d) SHA1(734b495ace73f07c622f64b305dafe43099395c1) )
	ROM_LOAD( "51-1-b_bb504.ic4",   0x10000, 0x10000, CRC(1fc7f229) SHA1(37120c85a170f31bc4fbf287b1ba80bc319522ec) )
	ROM_LOAD( "51-1-b_bb505.ic5",   0x20000, 0x10000, CRC(3ec650ce) SHA1(28091f535fcd580f2d3a941251a9c4f662fcf2e4) )
	ROM_LOAD( "51-1-b_bb506.ic6",   0x30000, 0x10000, CRC(10dba663) SHA1(ea0e4115ebb1c9f894c044a1eb11f135fcf5aba8) )
	ROM_LOAD( "51-1-b_bb512.ic12",  0x40000, 0x10000, CRC(83bbb220) SHA1(8f43354c7cea89938d1115d7a0f27ede8f7d3e96) )
	ROM_LOAD( "51-1-b_bb513.ic13",  0x50000, 0x10000, CRC(3767456b) SHA1(3680807282079862cdfb5ec055e7d771e708545b) )
	ROM_LOAD( "51-1-b_bb514.ic14",  0x60000, 0x10000, CRC(a29a2f44) SHA1(4e039d9a9b225179e84590d450eca3bed05bd3b8) )
	ROM_LOAD( "51-1-b_bb515.ic15",  0x70000, 0x10000, CRC(30110411) SHA1(fe9f418070c224d3a9acf6913bd4597b55afcc94) )
	ROM_LOAD( "51-1-b_bb518.ic18",  0x80000, 0x10000, CRC(efcf5b1d) SHA1(515b27f8e6df7ac7ed172cbd1ac64b14791de99f) )
	ROM_LOAD( "51-1-b_bb519.ic19",  0x90000, 0x10000, CRC(77670244) SHA1(27a5572d86ae6e9a5ef076572a4b3a04a22c86e9) )
	ROM_LOAD( "51-1-b_bb520.ic20",  0xa0000, 0x10000, CRC(d7f3b09a) SHA1(339206a7c3389d4eac63e8314ba7fdda9de73be7) )
	ROM_LOAD( "51-1-b_bb521.ic21",  0xb0000, 0x10000, CRC(fb8cff4c) SHA1(5fa0b52140959e029911a28928b3efad4aa9f1db) )
	ROM_LOAD( "51-1-b_bb524.ic24",  0xc0000, 0x10000, CRC(c4ccf38d) SHA1(be93ce6ed87c79fbd13838c0fe80526ce7e7e870) )
	ROM_LOAD( "51-1-b_bb525.ic25",  0xd0000, 0x10000, CRC(25b4e119) SHA1(7e7d95aefee2b8d4dddf105c16d347ec65cd76a5) )
	ROM_LOAD( "51-1-b_bb526.ic26",  0xe0000, 0x10000, CRC(1c2d70b0) SHA1(703f1acbcdaa7ff539f58829890d25b51a2e269e) )
	ROM_LOAD( "51-1-b_bb527.ic27",  0xf0000, 0x10000, CRC(a73cd7a5) SHA1(9106565d1c8a8e0efa8f5035106f3cdac2189107) )

	ROM_REGION( 0x0400, "proms", 0 )    // PROMs (function unknown)
	ROM_LOAD( "1-2_110_tbp18s030.ic20",  0x000, 0x020, NO_DUMP )
	ROM_LOAD( "2_211_82s129.ic4",        0x100, 0x100, CRC(4f8c3e63) SHA1(0aa68fa1de6ca945027366a06752e834bbbc8d09) )
	ROM_LOAD( "2_202_82s129.ic12",       0x200, 0x100, CRC(e434128a) SHA1(ef0f6d8daef8b25211095577a182cdf120a272c1) )
	ROM_LOAD( "51-1_p0502_82s129n.ic10", 0x300, 0x100, NO_DUMP )

	ROM_REGION( 0x1000, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "6-1_606_gal16v8-20hb1.ic13",    0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "6-1_646_gal16v8-20hb1.ic7",     0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "4-3_403_gal16v8-25hb1.ic29",    0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "4-3-a_p0403_pal16r8acn.ic29",   0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "4-3-b_403_gal16v8-25hb1.ic19",  0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "51-1_503_gal16v8-25lp.ic48",    0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "51-1-b_5146_gal16v8-20hb1.ic9", 0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "51-1-b_5246_gal16v8-20hb1.ic8", 0x000, 0x117, NO_DUMP ) // Protected
ROM_END

GAME( 199?, bloodbrom,  bloodbro,  bloodbrom,  bloodbrom,  bloodbro_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Blood Bros. (Modular System)", MACHINE_IS_SKELETON )
