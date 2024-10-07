// license:BSD-3-Clause
// copyright-holders:

/*
    Last Duel (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 7 main boards.
    More information, dip switches and PCB pictures: https://www.recreativas.org/modular-system-last-duel-4358-gaelco-sa

    MOD-6/1   - MC68000P10, 6 ROMs, RAMs, 20 MHz XTAL.
    MOD 1     - Sound board (Z8400BB1, 2 x YM2203C, 2 x Y3014), one EPROM, one PROM, two 8 dip switches banks.
    MOD 4/2   - Tilemap board, has logic + 2 tilemap ROMs, long thin sub-board (C0459/I) with no chips, just routing along one edge.
    MOD 4/3-A - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0460) with no chips, just routing along one edge.
    MOD 4/3-B - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0461) with no chips, just routing along one edge.
    MOD 51/3  - Sprite board, has logic, a PROM, and 4 ROMs.
    MOD 21/1 - Logic, 28.000 MHz XTAL.

    TODO:
    - everything,
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class lastduel_ms_state : public driver_device
{
public:
	lastduel_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void lastduelm(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
};


uint32_t lastduel_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void lastduel_ms_state::machine_start()
{
}


void lastduel_ms_state::main_map(address_map &map)
{
}


static INPUT_PORTS_START( lastduelm )
	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x0001, 0x0001, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0002, 0x0002, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0004, 0x0004, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0008, 0x0008, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x0010, 0x0010, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x0020, 0x0020, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x0040, 0x0040, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x0080, 0x0080, "SW1:8")
	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0100, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0200, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0400, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0800, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x1000, 0x1000, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x2000, 0x2000, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x4000, 0x4000, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x8000, 0x8000, "SW2:8")
INPUT_PORTS_END


void lastduel_ms_state::lastduelm(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &lastduel_ms_state::main_map);

	Z80(config, "audiocpu", 20_MHz_XTAL / 5).set_disable(); // divisor unknown

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(lastduel_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 256).set_membits(8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2203(config, "ym1", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	YM2203(config, "ym2", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown
}

ROM_START( lastduelm )
	ROM_REGION( 0x100000, "maincpu", 0 ) // on MOD 6/1 board
	ROM_LOAD16_BYTE( "mod_6-1_ld_603.ic17",   0x00000, 0x10000, CRC(c3403a02) SHA1(fbcaec5db6c7a06df2244f951b1943d9e40f539e) )
	ROM_LOAD16_BYTE( "mod_6-1_ld_606.ic8",    0x00001, 0x10000, CRC(290a3dc5) SHA1(9a4ca3b525d08505d96ad867dbd75b350a98fdfa) )
	ROM_LOAD16_BYTE( "mod_6-1_ld_601.ic26",   0x20000, 0x10000, CRC(429fb964) SHA1(78769b05e62c190d846dd08214427d1abbbe2bba) )
	ROM_LOAD16_BYTE( "mod_6-1_ld_604.ic25",   0x20001, 0x10000, CRC(6697615f) SHA1(c977d78f1d89034c0cb7f2f7e922a4d8f334fc70) )
	ROM_LOAD16_BYTE( "mod_6-1_ld_602.ic20",   0x40000, 0x10000, CRC(86bb19f9) SHA1(4c549a7cfaad60833a3e3bdd8aee5205cd81d1ff) ) // 1xxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "mod_6-1_ld_605.ic11",   0x40001, 0x10000, CRC(039740fa) SHA1(4ac50ffd4d6c080302204f83ace05c8b19a78452) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1 board
	ROM_LOAD( "mod_1_ld_101.ic12",            0x00000, 0x10000, CRC(6bc487c9) SHA1(23e757356894ec2ba5308c9fc9e27bb1e2fd8cad) )

	ROM_REGION( 0x10000, "tiles1", 0 ) // on MOD 4/2 board
	ROM_LOAD( "mod_4-2_ld_401.ic17",          0x00000, 0x08000, CRC(f8c8e903) SHA1(f5b390eaf48245e0a1b0c2c54df9777da97ac310) ) //  1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "mod_4-2_ld_402.ic16",          0x08000, 0x08000, CRC(c1553eb6) SHA1(c92b234c4a748fb34eaa79ad6b60626652ba9877) ) //  1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x80000, "tiles2", 0 ) // on MOD 4/3-A
	ROM_LOAD( "mod_4-3_a_ld_4a01.ic17",       0x00000, 0x20000, CRC(719e9d72) SHA1(9026126f6a6ad0b694de05873f75bd61180cdce1) )
	ROM_LOAD( "mod_4-3_a_ld_4a02.ic16",       0x20000, 0x20000, CRC(4668dfd9) SHA1(5822efd049deede08dcc96d2ace2afa378ab6d56) )
	ROM_LOAD( "mod_4-3_a_ld_4a03.ic15",       0x40000, 0x20000, CRC(372459ad) SHA1(5b5ee17d103748622dcfc57a4b58f40b703a03bf) )
	ROM_LOAD( "mod_4-3_a_ld_4a04.ic14",       0x60000, 0x20000, CRC(81762541) SHA1(a7d8bf6478d239ea8a5d9d0459481d9423c81109) )

	ROM_REGION( 0x40000, "tiles3", 0 ) // on MOD 4/3-B
	ROM_LOAD( "mod_4-3_b_ld_4b01.ic17",       0x00000, 0x10000, CRC(f631aa30) SHA1(a4517749fc15cd2ad6d58aa3da8983f3c07a1d87) )
	ROM_LOAD( "mod_4-3_b_ld_4b02.ic16",       0x10000, 0x10000, CRC(1b0e9136) SHA1(749fb028183025b0d5c622e67e58243f843aa2b8) )
	ROM_LOAD( "mod_4-3_b_ld_4b03.ic15",       0x20000, 0x10000, CRC(31a73db0) SHA1(8339537aeba25740ff5025d288c7a83fac862071) )
	ROM_LOAD( "mod_4-3_b_ld_4b04.ic14",       0x30000, 0x10000, CRC(9f420227) SHA1(39dc101fd9ed9fc26ee0b959adc20268a2a207d0) )

	ROM_REGION( 0x80000, "sprites", 0 ) // on MOD 51/3
	ROM_LOAD( "mod_51-3_ld_501.ic43",         0x00000, 0x20000, CRC(d3737ce2) SHA1(62ee62d29eefab51503be1acc796b4233a938ff9) )
	ROM_LOAD( "mod_51-3_ld_502.ic42",         0x20000, 0x20000, CRC(5a9e9d39) SHA1(5e4e521fc9322b1b95baa3ed36d47f2d62ae1383) )
	ROM_LOAD( "mod_51-3_ld_503.ic41",         0x40000, 0x20000, CRC(055a1575) SHA1(fbcf5fa79eaf3b1e363b9ce8b20c17d5fe3f7bf2) )
	ROM_LOAD( "mod_51-3_ld_504.ic40",         0x60000, 0x20000, CRC(227415bc) SHA1(b7bdcfc45a67074d77fa8d5ce8372b2834ce81ce) )

	ROM_REGION( 0x320, "proms", 0 ) // PROMs (function unknown)
	ROM_LOAD( "mod_1_116_82s123a.ic20",       0x00000, 0x00020, CRC(ebcedd2e) SHA1(7810d6f8a1e7474b2eb755cceb6a51ade4567020) )
	ROM_LOAD( "mod_51-3_502_82s129.ic10",     0x00020, 0x00100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )
	ROM_LOAD( "mod_21-1_203_82s129.ic12",     0x00120, 0x00100, CRC(d56e29b4) SHA1(d94157a9cd75d1f6d305f3d48291d0ae4c41e006) )
	ROM_LOAD( "mod_21-1_204_82s129.ic4",      0x00220, 0x00100, CRC(74470450) SHA1(40b0e0991090733f8190ad7efcb500bd109c2a7e) )

	ROM_REGION( 0x0200, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "mod_6-1_604_gal16v8.ic13",     0x00000, 0x00117, CRC(8b725626) SHA1(69caeaf3681452e6743605aa05cc5651eabaca3b) )
	ROM_LOAD( "mod_6-1_629_gal16v8.ic7",      0x00000, 0x00117, CRC(54b87ae5) SHA1(846bcf25d3e327f96120bd7346dd5dcae0d9a07d) )
	ROM_LOAD( "mod_4-2_403_gal16v8.ic29",     0x00000, 0x00104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) ) // Same as IC29 on MOD 4/3-A
	ROM_LOAD( "mod_4-3_a_p0403_pal16r8.ic29", 0x00000, 0x00104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) ) // Same as IC29 on MOD 4/2
	ROM_LOAD( "mod_4-3_b_403_gal16v8.ic29",   0x00000, 0x00117, CRC(c136de93) SHA1(116f6d3b456d20621ab07a005c1421f57569915c) )
	ROM_LOAD( "mod_51-3_503_gal16v8.ic46",    0x00000, 0x00117, CRC(11470ea1) SHA1(cfcafbcc7e55be717348f895df61e144fdd0cc9b) )
ROM_END

} // anonymous namespace


GAME( 199?, lastduelm, lastduel, lastduelm, lastduelm, lastduel_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Last Duel (Modular System)", MACHINE_IS_SKELETON )
