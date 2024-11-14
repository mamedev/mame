// license:BSD-3-Clause
// copyright-holders:

/*
    Twin Cobra (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 7 boards:

    MOD-6/1   - MC68000P10, 3 ROMs, RAMs, 20 MHz Xtal.
    MOD 1/3   - Sound board (Z840006PSC, 2 x YM2203C, 2 x Y3014B, 1 ROM). Two 8 dip switches banks.
    MOD 21/1  - Logic, 2 PROMs, 24 MHz Xtal.
    MOD 4/3   - Tilemap board, has logic + 3 tilemap ROMs, long thin sub-board (C0477 SOLD) with no chips, just routing along one edge.
    MOD 4/3-A - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0477 SOLD) with no chips, just routing along one edge.
    MOD 4/3-B - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0477 SOLD) with no chips, just routing along one edge.
    MOD 51/3  - Sprite board, has logic + 4 ROMs and 1 PROM.

    PCB pics and more info: https://www.recreativas.org/modular-system-twin-cobra-4353-gaelco-sa

    TODO:
    - everything,
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class twincobr_ms_state : public driver_device
{
public:
	twincobr_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void twincobrm(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
};


uint32_t twincobr_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void twincobr_ms_state::machine_start()
{
}


void twincobr_ms_state::main_map(address_map &map)
{
}


static INPUT_PORTS_START( twincobrm )
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

void twincobr_ms_state::twincobrm(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &twincobr_ms_state::main_map);

	Z80(config, "audiocpu", 20_MHz_XTAL / 5).set_disable(); // divisor unknown

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(twincobr_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 256).set_membits(8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2203(config, "ym1", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	YM2203(config, "ym2", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown
}

ROM_START( twincobrm )
	ROM_REGION( 0x40000, "maincpu", 0 ) // on MOD 6/1 board
	ROM_LOAD( "mod_6-1_tw_603.ic17",    0x00000, 0x10000, CRC(c50a31ae) SHA1(06fd0f1710de88c72cfece5e8202c9a3c726caaf) )
	ROM_LOAD( "mod_6-1_tw_606.ic8",     0x10000, 0x10000, CRC(fa8df9ed) SHA1(7b713edf6e5a9d2a0a930501b1f66be7eed95074) )
	ROM_LOAD( "mod_6-1_tw_601.ic26",    0x20000, 0x10000, CRC(07f64d13) SHA1(864ce0f9369c40c3ae792fc4ab2444a168214749) )
	ROM_LOAD( "mod_6-1_tw_604.ic25",    0x30000, 0x10000, CRC(41be6978) SHA1(4784804b738a332c7f24a43bcbb7a1e607365735) )

	ROM_REGION( 0x08000, "audiocpu", 0 ) // on MOD 1/3 board
	ROM_LOAD( "mod_1-3_tw_101.ic12",           0x00000, 0x08000, CRC(e1aafdf5) SHA1(3441fc32c4eba46fa84010a437d5627fe7cdb2d0) )

	ROM_REGION( 0x14000, "tiles1", 0 ) // on MOD 4/3 board
	ROM_LOAD( "mod_4-3_tw_401.ic17",    0x00000, 0x08000, CRC(da1efca2) SHA1(f8685872b266ff7b9bd657bcc538217159938823) )
	ROM_LOAD( "mod_4-3_tw_402.ic16",    0x08000, 0x04000, CRC(e9e2d4b1) SHA1(e0a19dd46a9ba85d95bba7fbf81d8dc36dbfeabd) )
	ROM_LOAD( "mod_4-3_tw_403.ic15",    0x0C000, 0x08000, CRC(bf4ed7aa) SHA1(b4c479b153c6f146d0d846cec04f0a52cbccef3e) )

	ROM_REGION( 0x40000, "tiles2", 0 ) // on MOD 4/3-A board
	ROM_LOAD( "mod_4-3_a_tw_4a01.ic17", 0x00000, 0x10000, CRC(8cc79357) SHA1(31064df2b796ca85ad3caccf626b684dff1104a1) )
	ROM_LOAD( "mod_4-3_a_tw_4a02.ic16", 0x10000, 0x10000, CRC(13daeac8) SHA1(1cb103f434e2ecf193fa936ca7ea9194064c5b39) )
	ROM_LOAD( "mod_4-3_a_tw_4a03.ic15", 0x20000, 0x10000, CRC(d9e2e55d) SHA1(0409e6df836d1d5198b64b21b42192631aa6d096) )
	ROM_LOAD( "mod_4-3_a_tw_4a04.ic14", 0x30000, 0x10000, CRC(15b3991d) SHA1(f5e7ed7a7721ed7e6dfd440634160390b7a294e4) )

	ROM_REGION( 0x20000, "tiles3", 0 ) // on MOD 4/3-B board
	ROM_LOAD( "mod_4-3_b_tw_4b01.ic17", 0x00000, 0x08000, CRC(44f5accd) SHA1(2f9bdebe71c8be195332356df68992fd38d86994) )
	ROM_LOAD( "mod_4-3_b_tw_4b02.ic16", 0x08000, 0x08000, CRC(170c01db) SHA1(f4c5a1600f6cbb48abbace66c6f7514f79138e8b) )
	ROM_LOAD( "mod_4-3_b_tw_4b03.ic15", 0x10000, 0x08000, CRC(97f20fdc) SHA1(7cb3cd0637b0db889a3d552fd7c1a916eee5ca27) )
	ROM_LOAD( "mod_4-3_b_tw_4b04.ic14", 0x18000, 0x08000, CRC(b5d48389) SHA1(a00c5b9c231d3d580fa20c7ad3f8b6fd990e6594) )

	ROM_REGION( 0x40000, "sprites", 0 ) // on MOD 51/3 board
	ROM_LOAD( "mod_51-3_tw_501.ic43",   0x00000, 0x10000, CRC(7cab3a5a) SHA1(2ab994fac033046d85fe9e31dbae80b9530f9a72) )
	ROM_LOAD( "mod_51-3_tw_502.ic42",   0x10000, 0x10000, CRC(0c7e2b95) SHA1(401168061a1c05f8f436d0be5f32b81c527c3837) )
	ROM_LOAD( "mod_51-3_tw_503.ic41",   0x20000, 0x10000, CRC(41de100a) SHA1(cceba1083e89d667cd67a71e992913f8ab314e62) )
	ROM_LOAD( "mod_51-3_tw_504.ic40",   0x30000, 0x10000, CRC(f318d76e) SHA1(c124aec667e98489cc7adcfa4b48b5a147101cb7) )

	ROM_REGION( 0x00100, "proms", 0 )    // PROMs (function unknown)
	ROM_LOAD( "mod_1-3_112_82s123.ic20",       0x00000, 0x00020, CRC(e37acb44) SHA1(c3bd11da0a13658edff6e795e112356307e16290) )
	ROM_LOAD( "mod_21-1_205_tbp24s10.ic12",    0x00000, 0x00100, CRC(204a7aee) SHA1(322164134aa65c37a9389024f921364a81d13e88) )
	ROM_LOAD( "mod_21-1_207_tbp24s10.ic4",     0x00000, 0x00100, CRC(7a63d740) SHA1(5b6d73153d0c058e02c55cbcaa40c4f5e5cb5ce4) )
	ROM_LOAD( "mod_51-3_502_82s129.ic10",      0x00000, 0x00100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0x00117, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "mod_4-3_403_pal16r8.ic29",      0x00000, 0x00104, CRC(16379b0d) SHA1(5379560b0ec7c67cbe131a581a347b86395f34ac) )
	ROM_LOAD( "mod_4-3_a_405_gal16v8.ic29",    0x00000, 0x00117, CRC(9e360ea4) SHA1(095932874ce385e1079194ebdb20cd4933ca6395) ) // Same as on MOD 4/3-B
	ROM_LOAD( "mod_4-3_b_404_gal16v8.ic29",    0x00000, 0x00117, CRC(9e360ea4) SHA1(095932874ce385e1079194ebdb20cd4933ca6395) ) // Same as on MOD 4/3-A
	ROM_LOAD( "mod_51-3_p0503_pal16r6.ic46",   0x00000, 0x00104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) )
	ROM_LOAD( "mod_6-1_604_gal16v8.ic13",      0x00000, 0x00117, CRC(45b8e164) SHA1(2e227ac5171ebf14e1d1f7214626ef4df0d5ae99) )
	ROM_LOAD( "mod_6-1_624_gal16v8.ic7",       0x00000, 0x00117, CRC(e044b4dc) SHA1(e83211758303a1e1fc50b78f6cfacc9c4b7c71c4) )
ROM_END

} // anonymous namespace


GAME( 199?, twincobrm, twincobr, twincobrm, twincobrm, twincobr_ms_state, empty_init, ROT270, "bootleg (Gaelco / Ervisa)", "Twin Cobra (Modular System)", MACHINE_IS_SKELETON )
