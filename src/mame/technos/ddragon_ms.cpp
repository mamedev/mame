// license:BSD-3-Clause
// copyright-holders:

/*
    Double Dragon (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 6 main boards and 3 sub-boards.
    More information, dip switches and PCB pictures: https://www.recreativas.org/modular-system-double-dragon-4350-gaelco-sa

    MOD 3/4   - CPU board: 3 ROMs, RAM, 22.1184 MHz XTAL, Small sub-board with 2 x MC68B09EP a PROM and a PAL.
    MOD 1/3   - Sound board: Motorola SC84014P, 2 x YM2203C, 2 x Y3014, one ROM, one PROM, two 8 dip switches Banks, and a small sub-board with an OKI M5205.
    MOD 4/3   - Tilemap board: logic + 4 tilemap ROMs, long thin sub-board (C0437) with no chips, just routing along one edge.
    MOD 4/3-A - Tilemap board: logic + 4 tilemap ROMs, long thin sub-board (C0438) with no chips, just routing along one edge.
    MOD 51    - Sprite board: logic, a PROM, and 8 ROMs on a small sub-board.
    MOD 21/1  - Logic, 20 MHz XTAL, two PROMs.

    TODO:
    - everything,
*/


#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ddragon_ms_state : public driver_device
{
public:
	ddragon_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_soundcpu(*this, "soundcpu"),
		m_screen(*this, "screen")
	{ }

	void ddragonm(machine_config &config);

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


uint32_t ddragon_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void ddragon_ms_state::machine_start()
{
}


void ddragon_ms_state::main_map(address_map &map)
{
}


static INPUT_PORTS_START( ddragonm )
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


void ddragon_ms_state::ddragonm(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 22.1184_MHz_XTAL / 3); // MC68B09EP, divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &ddragon_ms_state::main_map);

	MC6809E(config, m_subcpu, 22.1184_MHz_XTAL / 3); // MC68B09EP, divisor unknown

	MC6809(config, m_soundcpu, 22.1184_MHz_XTAL / 5).set_disable(); // Motorola SC84014P, divisor unknown, no xtal on the PCB

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(ddragon_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 256).set_membits(8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ym1", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	YM2203(config, "ym2", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	MSM5205(config, "msm", 20_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown
}

ROM_START( ddragonm )
	ROM_REGION( 0x30000, "maincpu", 0 ) // on MOD 3/4 board
	ROM_LOAD( "mod_3-4_dr303_27c512.ic12", 0x00000, 0x10000, CRC(3aa464b0) SHA1(cee14d02eca3a8dd8d8fd29fadd77a44bf60b72f) )
	ROM_LOAD( "mod_3-4_dr302_27c512.ic13", 0x10000, 0x10000, CRC(67321538) SHA1(9dca2ea8bdcf6c0d3657d0d81c8aa02bb34ce4bd) )
	ROM_LOAD( "mod_3-4_dr301_27c512.ic14", 0x20000, 0x10000, CRC(de8f6f06) SHA1(68de0f757b4a8652ed13ad9802b022a36339d6e1) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/3 board
	ROM_LOAD( "mod_1-3_dra101_27c512.ic12", 0x00000, 0x10000, CRC(e75de5a1) SHA1(e59a67b05d8dc723edfd88c52fe75301d345c38c) )

	ROM_REGION( 0x20000, "tiles1", 0 ) // on MOD 4/3 board
	ROM_LOAD( "mod_4-3_dr401_27256.ic17", 0x00000, 0x08000, CRC(86dc72eb) SHA1(dc23e884a03e21353ec25e24bc7073dd756408f6) ) //  01xxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "mod_4-3_dr402_27256.ic16", 0x08000, 0x08000, CRC(88abf9a3) SHA1(8ca97e2c0ecb8cf438a718bb197785d0c848613e) ) //  01xxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "mod_4-3_dr403_27256.ic15", 0x10000, 0x08000, CRC(b1d94294) SHA1(42e85f02eb2c1cf4b78e13be1c082ca4057aee57) ) //  01xxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "mod_4-3_dr404_27256.ic14", 0x18000, 0x08000, CRC(667682b2) SHA1(09108673d0f04a9249be04c6fa41a2cacff61e72) ) //  1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x40000, "tiles2", 0 ) // on MOD 4/3-A board
	ROM_LOAD( "mod_4-3_a_dr4a01_27512.ic17", 0x00000, 0x10000, CRC(5f6a6d6f) SHA1(7d546a226cda81c28e7ccfb4c5daebc65072198d) )
	ROM_LOAD( "mod_4-3_a_dr4a02_27512.ic16", 0x10000, 0x10000, CRC(4171b70d) SHA1(dc300c9bca6481417e97ad03c973e47389f261c1) )
	ROM_LOAD( "mod_4-3_a_dr4a03_27512.ic15", 0x20000, 0x10000, CRC(a670d088) SHA1(27e7b49645753dd039f104c3e0a7e6513a98710d) )
	ROM_LOAD( "mod_4-3_a_dr4a04_27512.ic14", 0x30000, 0x10000, CRC(736eff0f) SHA1(ae2ec2d5c8ab1db579a08256d874426dc5d889c6) )

	ROM_REGION( 0x80000, "sprites", 0 ) // on MOD 51 board
	ROM_LOAD( "mod_51_sub_dr501_27512.bin",  0x00000, 0x10000, CRC(bfa4da27) SHA1(68a649aec43e18dc79b4690c1dff2e2a6fc0065a) )
	ROM_LOAD( "mod_51_sub_dr502_27512.bin",  0x10000, 0x10000, CRC(a73bc8f5) SHA1(286cd04dfc8b6bfc26faf8ec246ffe9e7bfe87a4) )
	ROM_LOAD( "mod_51_sub_dr503_27512.bin",  0x20000, 0x10000, CRC(4b10defd) SHA1(fb43eba7c8a7f77f0fdd6253d51b40b0e64598f5) )
	ROM_LOAD( "mod_51_sub_dr504_27512.bin",  0x30000, 0x10000, CRC(7300b785) SHA1(6d3b72bd7208e2bd790517a753c9d5192c88d20f) )
	ROM_LOAD( "mod_51_sub_dr5a01_27512.bin", 0x40000, 0x10000, CRC(f80e81a6) SHA1(e74768d47985f59b9eb45013127f26ea8e0ddc28) )
	ROM_LOAD( "mod_51_sub_dr5a02_27512.bin", 0x50000, 0x10000, CRC(15b86326) SHA1(5da97dc3ac500a4ee38ee625a14e9efc1ca6bb5b) )
	ROM_LOAD( "mod_51_sub_dr5a03_27512.bin", 0x60000, 0x10000, CRC(6248bcc4) SHA1(27d160ad489ea32f4bda652f7f8f5e32257f34a1) )
	ROM_LOAD( "mod_51_sub_dr5a04-27512.bin", 0x70000, 0x10000, CRC(a4baa5ba) SHA1(25a7434a0a3ea33faa0881e6a372b1ee50b19cdd) )

	ROM_REGION( 0x200, "proms", 0 ) // PROMs (function unknown)
	ROM_LOAD( "mod_3-4_sub_p0320_82s147.bin", 0x000, 0x200, CRC(9bc0d387) SHA1(bac95901cacf883fcde0662b444c443364227def) )
	ROM_LOAD( "mod_1-3_p0111_82s123.ic20",    0x000, 0x020, CRC(b3baf409) SHA1(74e378bdb1f265a7067d59ff9514b50263231fa2) )
	ROM_LOAD( "mod_21-1_p0205_82s129.ic12",   0x000, 0x100, CRC(204a7aee) SHA1(322164134aa65c37a9389024f921364a81d13e88) )
	ROM_LOAD( "mod_21-1_p0201_82s129.ic4",    0x000, 0x100, CRC(2697da58) SHA1(e62516b886ff6e204b718e5f0c6ce2712e4b7fc5) )
	ROM_LOAD( "mod_51_p0502_82s129.ic10",     0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "mod_3-4_sub_p36_pal20x4.bin",   0x000, 0x0cc, CRC(896e285d) SHA1(5a49890c396fb9f3625d421ff4a8823d3b76ccb9) )
	ROM_LOAD( "mod_51_p0503-pal16r6a.ic46",    0x000, 0x104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) )
	ROM_LOAD( "mod_4-3_403_gal16v8.ic29",      0x000, 0x117, CRC(c136de93) SHA1(116f6d3b456d20621ab07a005c1421f57569915c) )
	ROM_LOAD( "mod_4-3_a_p0403_pal16r8a.ic29", 0x000, 0x104, CRC(16379b0d) SHA1(5379560b0ec7c67cbe131a581a347b86395f34ac) )
ROM_END

} // anonymous namespace


GAME( 1987, ddragonm, ddragon, ddragonm, ddragonm, ddragon_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Double Dragon (Modular System)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
