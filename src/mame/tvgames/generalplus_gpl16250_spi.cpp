// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
     GPL16250* games using SPI Flash + RAM configuration

     *part number could be different for these, they've only
      been seen as globtops
*/

#include "emu.h"
#include "generalplus_gpl16250.h"
#include "softlist_dev.h"


namespace {

class generalplus_gpspispi_game_state : public gcm394_game_state
{
public:
	generalplus_gpspispi_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		gcm394_game_state(mconfig, type, tag)
	{
	}

	void generalplus_gpspispi(machine_config &config);

	void init_spi();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
};



class generalplus_gpspispi_bkrankp_game_state : public generalplus_gpspispi_game_state
{
public:
	generalplus_gpspispi_bkrankp_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		generalplus_gpspispi_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{
	}

	void generalplus_gpspispi_bkrankp(machine_config &config);

protected:
	required_device<generic_slot_device> m_cart;
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

private:
};


void generalplus_gpspispi_game_state::machine_start()
{
}

void generalplus_gpspispi_game_state::machine_reset()
{
	m_maincpu->reset(); // reset CPU so vector gets read etc.

	//m_maincpu->set_paldisplaybank_high_hack(0);
}

static INPUT_PORTS_START( gcm394 )
	PORT_START("IN0")
	PORT_START("IN1")
	PORT_START("IN2")
INPUT_PORTS_END


void generalplus_gpspispi_game_state::generalplus_gpspispi(machine_config &config)
{
	GP_SPISPI(config, m_maincpu, 96000000/2, m_screen);
	m_maincpu->porta_in().set(FUNC(generalplus_gpspispi_game_state::porta_r));
	m_maincpu->portb_in().set(FUNC(generalplus_gpspispi_game_state::portb_r));
	m_maincpu->portc_in().set(FUNC(generalplus_gpspispi_game_state::portc_r));
	m_maincpu->porta_out().set(FUNC(generalplus_gpspispi_game_state::porta_w));
	m_maincpu->space_read_callback().set(FUNC(generalplus_gpspispi_game_state::read_external_space));
	m_maincpu->space_write_callback().set(FUNC(generalplus_gpspispi_game_state::write_external_space));
	m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(sunplus_gcm394_base_device::irq_vector_cb));
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
	m_maincpu->set_bootmode(0); // boot from internal ROM (SPI bootstrap)
	m_maincpu->set_cs_config_callback(FUNC(gcm394_game_state::cs_callback));

	FULL_MEMORY(config, m_memory).set_map(&generalplus_gpspispi_game_state::cs_map_base);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, (320*2)-1, 0, (240*2)-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "speaker", 2).front();
}

DEVICE_IMAGE_LOAD_MEMBER(generalplus_gpspispi_bkrankp_game_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void generalplus_gpspispi_bkrankp_game_state::generalplus_gpspispi_bkrankp(machine_config &config)
{
	generalplus_gpspispi_game_state::generalplus_gpspispi(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "bkrankp_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(generalplus_gpspispi_bkrankp_game_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("bkrankp_cart");
}



ROM_START( bkrankp )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION(0x400000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "unit_mx25l3206e_c22016.bin", 0x0000, 0x400000, CRC(7efad116) SHA1(427d707e97586ae6ab5fe08f29ca450ddc7ad36e) )
ROM_END

ROM_START( prailpls )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x2000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "mx25l25635f.u9", 0x0000, 0x2000000, CRC(17faefb0) SHA1(1d31c5aa1a37882f74c08414f69c4285149352b7) )
ROM_END

ROM_START( anpanbd )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "mx25l12835f.u2", 0x0000, 0x1000000, CRC(c4be09d7) SHA1(c9098d0c1c9db649a010f67469f500b69407372f) )
ROM_END

ROM_START( anpanm15 )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "mx25l12835f.ic3", 0x0000, 0x1000000, CRC(47c36cbd) SHA1(f1cae506e21c1795401004d79f6bb1b1d982d657) )
ROM_END

ROM_START( anpaneng )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "w25q128fv.u11", 0x0000, 0x1000000, CRC(d204d646) SHA1(0b2f9f2d91a078b5fba687d73079b2b5665b33d4) )
ROM_END

ROM_START( pokegach )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "mx25l12835f.u4", 0x0000, 0x1000000, CRC(85bc9716) SHA1(3de7f0fd92e8f6084eb0b82ec293be3166c800ac) )
ROM_END

ROM_START( pokegac2 )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "red_mx25l6445e.u4", 0x0000, 0x800000, CRC(f20bb213) SHA1(787ae27e36352525e6ffebe25da4329cb156b219) )

	ROM_REGION(0x800, "i2cmem", ROMREGION_ERASE00) // probably just progress / settings
	ROM_LOAD16_WORD_SWAP( "red_ft24c16a.u9", 0x000, 0x800, CRC(2abcf4d4) SHA1(5227e868f93205a069bc49c30792a9b95c8f0efc) )
ROM_END

ROM_START( pokegac2y )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "yellow_mx25l6445e.u4", 0x0000, 0x800000, CRC(587310fa) SHA1(4334b91b7f9f599bc21b354b267b132fd470f53c) )

	ROM_REGION(0x800, "i2cmem", ROMREGION_ERASE00) // probably just progress / settings
	ROM_LOAD16_WORD_SWAP( "yellow_ft24c16a.u9", 0x000, 0x800, CRC(b7662106) SHA1(a75366cbf3f3954a4136c89cc1db0ffb6f7d8c13) )
ROM_END


ROM_START( bk139in1 )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x4000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "25q512.bin", 0x0000, 0x4000000, CRC(0cd111a4) SHA1(70553a44c3d946e5d23c09f04e0627a5dbaa3e4d) )
ROM_END

ROM_START( lxcyrace )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "25q128.u2", 0x0000, 0x1000000, CRC(4489c99d) SHA1(792d6d224584fe1f3349c64a59aa79a587dd8c17) )
ROM_END

ROM_START( lxcymsm )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x2000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "25q256.bin", 0x0000, 0x2000000, CRC(96b3ee5c) SHA1(f1e6bf46a4503877074310506d1acc4607dac331) )
ROM_END

ROM_START( lxcympp )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x2000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "25q256.bin", 0x0000, 0x2000000, CRC(570b669c) SHA1(e7fcae662c8c8cae18cf1151d6caefacfe1e9fda) )
ROM_END

ROM_START( lxcymls )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only (if it exists at all)

	ROM_REGION(0x2000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "25q256.bin", 0x0000, 0x2000000, CRC(76c89fe5) SHA1(99668cbce2ace6ec972ee4e72fec8b93862a0ef4) )
ROM_END


void generalplus_gpspispi_game_state::init_spi()
{
	int vectorbase = 0x2fe0;
	uint8_t* spirom = memregion("maincpu")->base();

	address_space& mem = m_maincpu->space(AS_PROGRAM);

	/*  Offset(h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F

	    00000000  50 47 70 73 73 69 69 70 70 73 44 00 44 3F 44 00  PGpssiipps
	    00000010  -- -- -- -- -- bb -- -- -- -- -- -- -- -- -- --
	                             ^^ copy dest, just like with nand type

	    bb = where to copy first block

	    The header is GPspispi (byteswapped) then some params
	    one of the params appears to be for the initial code copy operation done
	    by the bootstrap
	*/

	// probably more bytes are used
	int dest = spirom[0x15] << 8;

	// copy a block of code from the NAND to RAM
	for (int i = 0; i < 0x2000; i++)
	{
		uint16_t word = spirom[(i * 2) + 0] | (spirom[(i * 2) + 1] << 8);

		mem.write_word(dest + i, word);
	}

	// these vectors must either directly point to RAM, or at least redirect there after some code
	uint16_t* internal = (uint16_t*)memregion("maincpu:internal")->base();
	internal[0x7ff5] = vectorbase + 0x0a;
	internal[0x7ff6] = vectorbase + 0x0c;
	internal[0x7ff7] = dest + 0x20; // point boot vector at code in RAM (probably in reality points to internal code that copies the first block)
	internal[0x7ff8] = vectorbase + 0x10;
	internal[0x7ff9] = vectorbase + 0x12;
	internal[0x7ffa] = vectorbase + 0x14;
	internal[0x7ffb] = vectorbase + 0x16;
	internal[0x7ffc] = vectorbase + 0x18;
	internal[0x7ffd] = vectorbase + 0x1a;
	internal[0x7ffe] = vectorbase + 0x1c;
	internal[0x7fff] = vectorbase + 0x1e;
}

} // anonymous namespace

// ぼくはプラレール運転士 新幹線で行こう！プラス  (I am a Plarail driver Let's go by Shinkansen! Plus)
CONS(2015, prailpls, 0, 0, generalplus_gpspispi,         gcm394, generalplus_gpspispi_game_state,         init_spi, "Takara Tomy", "Boku wa Plarail Untenshi Shinkansen de Ikou! Plus (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // has built-in screen, but can be connected to a TV

// this is a half-head shaped unit, SHP13017-R1 main PCB  -  アンパンマン レッツゴー！育脳ドライブ きみものれるよ！アンパンマンごう「それいけ！アンパンマン」
CONS(2014, anpanbd,  0, 0, generalplus_gpspispi,         gcm394, generalplus_gpspispi_game_state,         init_spi, "JoyPalette", "Anpanman: Let's Go! Ikunou Drive (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

//  それいけ！アンパンマン」みんなで！育脳マット
CONS(2015, anpanm15, 0, 0, generalplus_gpspispi,         gcm394, generalplus_gpspispi_game_state,         init_spi, "JoyPalette", "Anpanman: Minnade! Ikunou Mat (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2013, anpaneng, 0, 0, generalplus_gpspispi,         gcm394, generalplus_gpspispi_game_state,         init_spi, "Sega Toys", "Anpanman: Touch de English (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2015, bkrankp,  0, 0, generalplus_gpspispi_bkrankp, gcm394, generalplus_gpspispi_bkrankp_game_state, init_spi, "Bandai", "Karaoke Ranking Party (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2015, pokegach,  0,        0, generalplus_gpspispi_bkrankp, gcm394, generalplus_gpspispi_bkrankp_game_state, init_spi, "Tomy", "Pokegacha (20150902, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// the 2nd release comes in 2 colours and they can communicate?
CONS(2015, pokegac2,  0,        0, generalplus_gpspispi_bkrankp, gcm394, generalplus_gpspispi_bkrankp_game_state, init_spi, "Tomy", "Pokegacha V2 Red (20151230, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2015, pokegac2y, pokegach, 0, generalplus_gpspispi_bkrankp, gcm394, generalplus_gpspispi_bkrankp_game_state, init_spi, "Tomy", "Pokegacha V2 Yellow (20151230, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// This can be found listed as a ZHISHAN / Aojiao / Bornkid 32 Bit Preloaded 139-in-1 Handheld Game Console
// but these just seem to be brands, manufacturer is unknown.
// Various case styles are available, the unit here was styled after a Nintendo Switch
//
// Architecture is unknown, it contains many of the games in beijuehh / bornkidh (generalplus_gpl16250_rom.cpp)
// but is running from SPI flash and has 'Loading' screens between menus and after selecting a game.
//
// While those are GeneralPlus based platforms, it's possible the games were ported to something else, the SPI
// appears to contain a filesystem, but data looks to be compressed / encrypted with no obvious code.
// There is no GPspi header in the SPI ROM.
CONS(202?, bk139in1,  0, 0, generalplus_gpspispi, gcm394, generalplus_gpspispi_game_state, empty_init, "<unknown>", "BornKid 32 Bit Preloaded 139-in-1 Handheld Game Console", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
// same unknown hardware as above, fewer games
CONS(2021, lxcyrace,  0, 0, generalplus_gpspispi, gcm394, generalplus_gpspispi_game_state, empty_init, "Lexibook", "Cyber Arcade Racing (JL3150)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2021, lxcymsm,   0, 0, generalplus_gpspispi, gcm394, generalplus_gpspispi_game_state, empty_init, "Lexibook", "Cyber Arcade Motion - Superman (JL3180SU)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2021, lxcympp,   0, 0, generalplus_gpspispi, gcm394, generalplus_gpspispi_game_state, empty_init, "Lexibook", "Cyber Arcade Motion - Paw Patrol (JL3180PA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2021, lxcymls,   0, 0, generalplus_gpspispi, gcm394, generalplus_gpspispi_game_state, empty_init, "Lexibook", "Cyber Arcade Motion - Lilo & Stitch (JL3180D_01)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
