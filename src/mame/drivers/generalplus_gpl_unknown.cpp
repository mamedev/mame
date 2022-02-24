// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
   NOTES:

   Exact size of internal ROM is unknown, but it appears to occupy at least 0x4000 - 0xffff (word addresses)
   in unSP space and is different for each game.  Data is pulled for jump tables, where the addresses are in
   internal ROM and the code in external

   For correctly offset disassembly use
   unidasm xxx.bin -arch unsp12 -xchbytes -basepc 200000 >palace.txt

   Multiple different units appear to share the same ROM with a jumper to select game.
   It should be verified in each case that the external ROM was not changed

   It is confirmed that in some cases the external ROMs contain both versions for Micro Arcade and Tiny Arcade
   units, with different sound, but in some cases the expected game behavior differs too, so the code revisions
   could be different.  (for example the Micro Arcade Pac-Man doesn't display points when you eat a ghost)
*/


#include "emu.h"

#include "cpu/unsp/unsp.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "logmacro.h"


class generalplus_gpl_unknown_state : public driver_device
{
public:
	generalplus_gpl_unknown_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		//m_mainrom(*this, "maincpu"),
		//m_mainram(*this, "mainram"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_spirom(*this, "spi")
	{ }

	void generalplus_gpl_unknown(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<unsp_12_device> m_maincpu;
	//required_region_ptr<uint16_t> m_mainrom;
	//required_shared_ptr<uint16_t> m_mainram;

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_region_ptr<uint16_t> m_spirom;

	void map(address_map &map);
};

uint32_t generalplus_gpl_unknown_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static INPUT_PORTS_START( generalplus_gpl_unknown )
INPUT_PORTS_END


void generalplus_gpl_unknown_state::map(address_map &map)
{
	map(0x000000, 0x000fff).ram(); // RAM
	//map(0x001000, 0x0017ff).ram(); // acts like open bus?

	//map(0x003000, 0x003fff).ram(); // system regs

	map(0x004000, 0x00bfff).rom().region("maincpu", 0x0000);
	map(0x00c000, 0x00ffff).rom().region("maincpu", 0x0000);

	map(0x200000, 0x3fffff).rom().region("spi", 0x0000); // has direct access to SPI ROM
}


void generalplus_gpl_unknown_state::machine_start()
{
}

void generalplus_gpl_unknown_state::machine_reset()
{
}

void generalplus_gpl_unknown_state::generalplus_gpl_unknown(machine_config &config)
{

	UNSP_20(config, m_maincpu, 96000000); // internal ROM uses unsp2.0 opcodes, unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &generalplus_gpl_unknown_state::map);
	m_maincpu->set_vectorbase(0x4010); // there is also a set of vectors for what looks to be a burn-in test at 4000, maybe external pin selects?

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(128, 128);
	m_screen->set_visarea(0, 128-1, 0, 128-1); // not the correct resolution
	m_screen->set_screen_update(FUNC(generalplus_gpl_unknown_state::screen_update));
	//m_screen->screen_vblank().set(FUNC(generalplus_gpl_unknown_state::screen_vblank));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x8000);
}


ROM_START( mapacman ) // this is the single game (no games hidden behind solder pads) release
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "mapacman_internal.rom", 0x000000, 0x10000, CRC(9ea69d2a) SHA1(17f5001794f4454bf5856cfa170834509d68bed0) )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "fm25q16a.bin", 0x000000, 0x200000, CRC(aeb472ac) SHA1(500c24b725f6d3308ef8cbdf4259f5be556c7c92) )
ROM_END

ROM_START( taspinv )
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "tinyarcade_spaceinvaders.bin", 0x000000, 0x200000, CRC(11ac4c77) SHA1(398d5eff83a4e94487ed810819085a0e44582908) )
ROM_END

ROM_START( tagalaga )
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "tinyarcadegalaga_fm25q16a_a14015.bin", 0x000000, 0x200000, CRC(2a91460c) SHA1(ce297642d2d51ce568e93c0c57432446633b2077) )
ROM_END

ROM_START( parcade )
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "palacearcade_gpr25l3203_c22016.bin", 0x000000, 0x400000, CRC(98fbd2a1) SHA1(ffc19aadd53ead1f9f3472475606941055ca09f9) )
ROM_END

ROM_START( taturtf )
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "tinyarcadeturtlefighter_25q32bst16_684016.bin", 0x000000, 0x400000, CRC(8e046f2d) SHA1(e48492cf953f22a47fa2b88a8f96a1e459b8c487) )
ROM_END


// The 'Micro Arcade' units are credit card sized handheld devices
CONS( 2017, mapacman,      0,       0,      generalplus_gpl_unknown,   generalplus_gpl_unknown, generalplus_gpl_unknown_state, empty_init, "Super Impulse", "Pac-Man (Micro Arcade)", MACHINE_IS_SKELETON )

// The 'Tiny Arcade' units are arcade cabinets on a keyring.
CONS( 2017, taspinv,       0,       0,      generalplus_gpl_unknown,   generalplus_gpl_unknown, generalplus_gpl_unknown_state, empty_init, "Super Impulse", "Space Invaders (Tiny Arcade)", MACHINE_IS_SKELETON )

CONS( 2017, tagalaga,      0,       0,      generalplus_gpl_unknown,   generalplus_gpl_unknown, generalplus_gpl_unknown_state, empty_init, "Super Impulse", "Galaga (Tiny Arcade)", MACHINE_IS_SKELETON )

CONS( 2017, parcade,       0,       0,      generalplus_gpl_unknown,   generalplus_gpl_unknown, generalplus_gpl_unknown_state, empty_init, "Hasbro", "Palace Arcade (Tiny Arcade)", MACHINE_IS_SKELETON )

CONS( 2019, taturtf,       0,       0,      generalplus_gpl_unknown,   generalplus_gpl_unknown, generalplus_gpl_unknown_state, empty_init, "Super Impulse", "Teenage Mutant Ninja Turtles - Turtle Fighter (Tiny Arcade)", MACHINE_IS_SKELETON )


