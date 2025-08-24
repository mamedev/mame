// license:BSD-3-Clause
// copyright-holders:

/*
    Gryzor (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 7 main boards and 2 sub-boards.
    More information, dip switches and PCB pictures: https://www.recreativas.org/modular-system-gryzor-4344-gaelco-sa

    MOD 3/2   - CPU board: 2 ROMs, RAM, 22.1184 MHz XTAL, Small sub-board with 2 x Motorola SC84014P a PROM and a PAL.
    MOD 1/3   - Sound board: Motorola SC84014P, 2 x YM2203C, 2 x Y3014, one ROM, one PROM, two 8 dip switches Banks.
    MOD 4/3   - Tilemap board: logic + 2 tilemap ROMs, long thin sub-board (C0431A) with no chips, just routing along one edge.
    MOD 4/3-A - Tilemap board: logic + 4 tilemap ROMs, long thin sub-board (C0435A) with no chips, just routing along one edge.
    MOD 4/3-B - Tilemap board: logic + 4 tilemap ROMs, long thin sub-board (C0436A) with no chips, just routing along one edge.
    MOD 51    - Sprite board: logic, a PROM, and 8 ROMs on a small sub-board.
    MOD 21/1  - Logic, 24 MHz XTAL, two PROMs.

    TODO:
    - everything,
*/


#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class gryzor_ms_state : public driver_device
{
public:
	gryzor_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_soundcpu(*this, "soundcpu"),
		m_screen(*this, "screen")
	{ }

	void gryzorm(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_soundcpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
};


uint32_t gryzor_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void gryzor_ms_state::machine_start()
{
}


void gryzor_ms_state::main_map(address_map &map)
{
}


static INPUT_PORTS_START( gryzorm )
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


void gryzor_ms_state::gryzorm(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, 22.1184_MHz_XTAL / 3); // Motorola SC84014P, divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &gryzor_ms_state::main_map);

	MC6809(config, m_subcpu, 22.1184_MHz_XTAL / 3); // Motorola SC84014P, divisor unknown

	MC6809(config, m_soundcpu, 22.1184_MHz_XTAL / 5).set_disable(); // Motorola SC84014P, divisor unknown, no xtal on the PCB

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(gryzor_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 256).set_membits(8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ym1", 22.1184_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	YM2203(config, "ym2", 22.1184_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown
}

ROM_START( gryzorm )
	ROM_REGION( 0x20000, "maincpu", 0 ) // on MOD 3 board
	ROM_LOAD( "mod_3-2_gz301_27512.ic14", 0x00000, 0x10000, CRC(b25cfc4a) SHA1(c1334b45a38fba8d304a4f96e241dd46222037df) )
	ROM_LOAD( "mod_3-2_gz302_27512.ic13", 0x10000, 0x10000, CRC(207e5fd0) SHA1(c2846a1d8eb3ad22b841807ececdb511ac1d4d1a) )

	ROM_REGION( 0x8000, "audiocpu", 0 ) // on MOD 1/3 board
	ROM_LOAD( "mod_1-3_gz0101_27256.ic12", 0x0000, 0x8000, CRC(b6106777) SHA1(891a622924f93c58ef2aca4be2cc5d7f3dee815b) )

	ROM_REGION( 0x10000, "tiles1", 0 ) // on MOD 4/3 board
	ROM_LOAD( "mod_4-3_gz401_27256.ic18", 0x00000, 0x08000, CRC(7dedabc6) SHA1(ce4c6b17844dc54012ce981a3a518e32f87606f5) ) //  0111xxxxxxxxxxx = 0xFF
	ROM_LOAD( "mod_4-3_gz402_27256.ic17", 0x08000, 0x08000, CRC(02131f92) SHA1(51a77da46ed5b6bcdca94ad0a96f9ba2d43dfdde) ) //  0111xxxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "tiles2", 0 ) // on MOD 4/3-A board
	ROM_LOAD( "mod_4-3_a_gz4a01_27512.ic17", 0x00000, 0x10000, CRC(bfd1780a) SHA1(059276c9b9b138d43645dd59415467a7b6bdd2eb) )
	ROM_LOAD( "mod_4-3_a_gz4a02_27512.ic16", 0x10000, 0x10000, CRC(7c115140) SHA1(5f26fab8ca0e8df6fe79824b4985302b1e0dfd08) )
	ROM_LOAD( "mod_4-3_a_gz4a03_27512.ic15", 0x20000, 0x10000, CRC(de42af04) SHA1(1d79162889e965166ac34d462c20d6db15f36db8) )
	ROM_LOAD( "mod_4-3_a_gz4a04_27512.ic14", 0x30000, 0x10000, CRC(2a536bd5) SHA1(b523a7da90d69eefa0c4b528f89dc1fbc4e5252a) )

	ROM_REGION( 0x40000, "tiles3", 0 ) // on MOD 4/3-B board
	ROM_LOAD( "mod_4-3_b_gz4b01_27512.ic17", 0x00000, 0x10000, CRC(51e247d1) SHA1(2176a39625ab21b88e25d2f3f5e03d7093041b52) )
	ROM_LOAD( "mod_4-3_b_gz4b02_27512.ic16", 0x10000, 0x10000, CRC(f1fba529) SHA1(8f6baaf8360bdb6c805e379b0dbb2079baa51364) )
	ROM_LOAD( "mod_4-3_b_gz4b03_27512.ic15", 0x20000, 0x10000, CRC(4aa2b6b9) SHA1(547653b220d5981160d1b31a68f0001a0d406c03) )
	ROM_LOAD( "mod_4-3_b_gz4b04_27512.ic14", 0x30000, 0x10000, CRC(8f77c11b) SHA1(32ba5870576ce419a4991091607de99d4f98583a) )

	ROM_REGION( 0x80000, "sprites", 0 ) // on MOD 51 board
	ROM_LOAD( "mod_51_sub_gz501_27512.bin",  0x00000, 0x10000, CRC(a42793fd) SHA1(89856a72301c9fa5e59b8b3304df953cf358cefb) )
	ROM_LOAD( "mod_51_sub_gz502_27512.bin",  0x10000, 0x10000, CRC(6ccfb733) SHA1(657161e725cd48cbc807c75ecd0644de8f7d0270) )
	ROM_LOAD( "mod_51_sub_gz503_27512.bin",  0x20000, 0x10000, CRC(0a17cc17) SHA1(9bed123b6754d9b8f7cfba2176536f3e453d94f0) )
	ROM_LOAD( "mod_51_sub_gz504_27512.bin",  0x30000, 0x10000, CRC(7afc287e) SHA1(bc2168194845ff2fde35df450bc8fa755b29df3f) )
	ROM_LOAD( "mod_51_sub_gz5a01_27512.bin", 0x40000, 0x10000, CRC(54cf2e96) SHA1(28ad483fdf645bf5e19e1e71ed1da9d32dfe95ca) )
	ROM_LOAD( "mod_51_sub_gz5a02_27512.bin", 0x50000, 0x10000, CRC(4c32e827) SHA1(7670803d46de7333df021b232759ea346f748e37) )
	ROM_LOAD( "mod_51_sub_gz5a03_27512.bin", 0x60000, 0x10000, CRC(abd7b5b7) SHA1(94326d7ff8bba1299a5be193d06c1e019d38ce5c) )
	ROM_LOAD( "mod_51_sub_gz5a04_27512.bin", 0x70000, 0x10000, CRC(e0b11c30) SHA1(a35c94a2dd48f10eec623356655f25bd9e533eaf) )

	ROM_REGION( 0x200, "proms", 0 ) // PROMs (function unknown)
	ROM_LOAD( "mod_1-3_p0109_mmi6331.ic20",      0x000, 0x020, CRC(0c58dcfc) SHA1(a89170754745659d3fad6d25b9f54a4a48d4f26f) )
	ROM_LOAD( "mod_21-1_p0202_82s129.ic12",      0x000, 0x100, CRC(e434128a) SHA1(ef0f6d8daef8b25211095577a182cdf120a272c1) )
	ROM_LOAD( "mod_21-1_p0206_82s129.ic4",       0x000, 0x100, CRC(a47680d5) SHA1(6475cad53b91f6ef1043e7330b79f58695fd7ec9) )
	ROM_LOAD( "mod_3-4_sub_p0318_mmi63s481.bin", 0x000, 0x200, CRC(67a4bbc7) SHA1(441be3a810225813bf2667ddf9b2fdc7de35196e) )
	ROM_LOAD( "mod_51_p0502_mmi63s141.ic10",     0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "mod_3-4_sub_p36_pal20x4.bin",  0x000, 0x0cc, CRC(896e285d) SHA1(5a49890c396fb9f3625d421ff4a8823d3b76ccb9) )
	ROM_LOAD( "mod_4-3_p0403_pal16r8.ic29",   0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) ) // Same on all MOD 4/3 PCBs
	ROM_LOAD( "mod_4-3_a_p0403_pal16r8.ic29", 0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) ) // Same on all MOD 4/3 PCBs
	ROM_LOAD( "mod_4-3_b_p0403_pal16r8.ic29", 0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) ) // Same on all MOD 4/3 PCBs
	ROM_LOAD( "mod_51_p0503_pal16r6a.ic46",   0x000, 0x104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) )
ROM_END

} // anonymous namespace


GAME( 1987, gryzorm, contra, gryzorm, gryzorm, gryzor_ms_state, empty_init, ROT90, "bootleg (Gaelco / Ervisa)", "Gryzor (Modular System)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
