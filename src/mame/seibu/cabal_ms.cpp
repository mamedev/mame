// license:BSD-3-Clause
// copyright-holders:

/*
    Cabal (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 6 main boards and 1 sub board.
    More information and PCB pictures: https://www.recreativas.org/modular-system-cabal-553-ervisa

    MOD-6/1  - MC68000P10, 6 ROMs, RAMs, 20 MHz XTAL.
    MOD 1/4  - Sound board (Z8400BB1, 2 x YM2203C, 2 x Y3014B), one EPROM, two 8 dip switches banks, a small sub board with OKI M5205.
    MOD 4/2a - Tilemap board, has logic + 4 ROMs, long thin sub-board (C0466-1) with no chips, just routing along one edge.
    MOD 4/3  - Tilemap board, has logic + 2 ROMs, long thin sub-board (C0465-1) with no chips, just routing along one edge.
    MOD 21/1 - Logic, 2 PROMs, 28.000 MHz XTAL.
    MOD 51/3 - Sprite board, has logic + 4 ROMs and 1 PROM.

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

class cabal_ms_state : public driver_device
{
public:
	cabal_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void cabalm(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
};


uint32_t cabal_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void cabal_ms_state::machine_start()
{
}


void cabal_ms_state::main_map(address_map &map)
{
}


static INPUT_PORTS_START( cabalm )
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


void cabal_ms_state::cabalm(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &cabal_ms_state::main_map);

	Z80(config, "audiocpu", 20_MHz_XTAL / 5).set_disable(); // divisor unknown

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(cabal_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 256).set_membits(8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2203(config, "ym1", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	YM2203(config, "ym2", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	MSM5205(config, "msm", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown
}

ROM_START( cabalm )
	ROM_REGION( 0x100000, "maincpu", 0 ) // on MOD 6/1 board
	ROM_LOAD16_BYTE( "mod_6-1_ca_303.ic17",   0x00000, 0x08000, CRC(8ee66b0b) SHA1(78124848c8ec76c108a219a4e9a087f39489230a) )
	ROM_LOAD16_BYTE( "mod_6-1_ca_306.ic8",    0x00001, 0x08000, CRC(9de579d9) SHA1(dcab2f615235069788044fafb36deed4422b72a1) )
	ROM_LOAD16_BYTE( "mod_6-1_ca_302.ic20",   0x10000, 0x08000, CRC(04431f63) SHA1(a8e2980ccdb174c8c84b983d73d7c595b0babdb8) )
	ROM_LOAD16_BYTE( "mod_6-1_ca_305.ic11",   0x10001, 0x08000, CRC(8eb01277) SHA1(567e66d7a2781737fc81ae74cd8fa6e6571a0e25) )
	ROM_LOAD16_BYTE( "mod_6-1_ca_301.ic26",   0x20000, 0x08000, CRC(6c5bd9dd) SHA1(fa69a567990d9dfb6a79e2b16fde1ce1626250ea) )
	ROM_LOAD16_BYTE( "mod_6-1_ca_304.ic25",   0x20001, 0x08000, CRC(f9d7a8a2) SHA1(c425fa5e69fef4eefeaf597f95acc3a249705863) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/4 board
	ROM_LOAD( "mod_1-4_ca_101.ic12",          0x00000, 0x10000, CRC(1189d0ce) SHA1(a992006377c3b0c952337a2331d437458027bba4) )

	ROM_REGION( 0x80000, "tiles1", ROMREGION_INVERT ) // on MOD 4/2a board
	ROM_LOAD32_BYTE( "mod_4-2a_ca_4a01.ic17", 0x00003, 0x20000, CRC(f84786bb) SHA1(16f035eea5ff1978877951661a8e8fe8991e340c) )
	ROM_LOAD32_BYTE( "mod_4-2a_ca_4a02.ic16", 0x00002, 0x20000, CRC(8de8c6b2) SHA1(da06ceaa01158821205ba74419be012e94597db8) )
	ROM_LOAD32_BYTE( "mod_4-2a_ca_4a03.ic15", 0x00001, 0x20000, CRC(3b5d606b) SHA1(b09bf470fcda3e8bc8f5f3a893266abad4b56f26) )
	ROM_LOAD32_BYTE( "mod_4-2a_ca_4a04.ic14", 0x00000, 0x20000, CRC(9df7e3d9) SHA1(8ce55381d9087ca14cec3924222f0c277580e6c9) )

	ROM_REGION( 0x40000, "tiles2", ROMREGION_INVERT ) // on MOD 4/3 board
	ROM_LOAD32_BYTE( "mod_4-3_ca_401.ic17",   0x00001, 0x10000, CRC(fad717b2) SHA1(0654abf09c473f3b6c868f1198edd37e358fdd13) )
	ROM_LOAD32_BYTE( "mod_4-3_ca_402.ic16",   0x00000, 0x10000, CRC(c203912c) SHA1(702a0f2841aa18cb4573128ca19cc4a8d8fa3f3e) )

	ROM_REGION( 0x80000, "sprites", 0 ) // on MOD 51/3
	ROM_LOAD( "mod_51-3_ca_501.ic43",         0x00000, 0x20000, CRC(9786bffd) SHA1(31341475f2c0dea6e4bc3e2f14888e52de03a6c7) )
	ROM_LOAD( "mod_51-3_ca_502.ic42",         0x20000, 0x20000, CRC(8dc0ff51) SHA1(b4d4755ed31574cd0206d4d5f4f6ea23bc8bf27e) )
	ROM_LOAD( "mod_51-3_ca_503.ic41",         0x40000, 0x20000, CRC(14d8720b) SHA1(f8438345482d244593d61d848e991cd28ff9997b) )
	ROM_LOAD( "mod_51-3_ca_504.ic40",         0x60000, 0x20000, CRC(3048eb6d) SHA1(7af78233aeef1fcd4edab10bba562502b57a747d) )

	ROM_REGION( 0x320, "proms", 0 ) // PROMs (function unknown)
	ROM_LOAD( "mod_1-4_112_82s123.ic20",      0x00000, 0x00020, NO_DUMP )
	ROM_LOAD( "mod_51-3_502_82s129.ic10",     0x00020, 0x00100, NO_DUMP )
	ROM_LOAD( "mod_21-1_p0202_82s129.ic12",   0x00120, 0x00100, NO_DUMP )
	ROM_LOAD( "mod_21-1_201_82s129.ic4",      0x00220, 0x00100, NO_DUMP )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "mod_6-1_605_gal16v8.ic13",     0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "mod_6-1_632_gal16v8.ic7",      0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "mod_4-2a_p0403_pal16r8a.ic19", 0x00000, 0x00104, NO_DUMP )
	ROM_LOAD( "mod_4-3_403_gal16v8.ic29",     0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "mod_51-3_502_gal16v8.ic46",    0x00000, 0x00104, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1988, cabalm, cabal, cabalm, cabalm, cabal_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Cabal (Modular System)", MACHINE_IS_SKELETON )
