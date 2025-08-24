// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    These seem to be GPL16250 related based on video register use
    however the SPI ROM maps directly into CPU space, where you'd
    expect internal ROM to be?!

    It has been confirmed that Pacman/Fix It Felix/Ms Pacman can
    be swapped onto the same PCB, so either there is no internal
    area (runs direct from SPI?) or it's the same between games.
*/

#include "emu.h"
#include "generalplus_gpl16250.h"


namespace {

class generalplus_gpspi_direct_game_state : public gcm394_game_state
{
public:
	generalplus_gpspi_direct_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		gcm394_game_state(mconfig, type, tag)
	{
	}

	void init_fif();

	void generalplus_gpspi_direct(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint16_t cs0_r(offs_t offset) override;

private:
};

void generalplus_gpspi_direct_game_state::machine_start()
{
}

void generalplus_gpspi_direct_game_state::machine_reset()
{
	cs_callback(0x00, 0x00, 0x00, 0x00, 0x00);
	m_maincpu->set_cs_space(m_memory->get_program());

	m_maincpu->reset(); // reset CPU so vector gets read etc.

	m_maincpu->set_alt_tile_addressing_hack(1);
}

static INPUT_PORTS_START( bfmpac )
	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0000, "0" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Demo Play" ) // the units have a strip of paper that you must rip out to disable display / demo mode, this is probably related to that, have to press start to make it run a demo cycle?
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_START("IN0")
INPUT_PORTS_END

static INPUT_PORTS_START( bfspyhnt )
	PORT_INCLUDE( bfmpac )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 )
INPUT_PORTS_END


uint16_t generalplus_gpspi_direct_game_state::cs0_r(offs_t offset)
{
	// TODO: is cs_space even used by this type?
	return 0x00;
}

void generalplus_gpspi_direct_game_state::generalplus_gpspi_direct(machine_config &config)
{
	GP_SPI_DIRECT(config, m_maincpu, 96000000/2, m_screen);
	m_maincpu->porta_in().set(FUNC(generalplus_gpspi_direct_game_state::porta_r));
	m_maincpu->portb_in().set(FUNC(generalplus_gpspi_direct_game_state::portb_r));
	m_maincpu->portc_in().set(FUNC(generalplus_gpspi_direct_game_state::portc_r));
	m_maincpu->porta_out().set(FUNC(generalplus_gpspi_direct_game_state::porta_w));
	m_maincpu->space_read_callback().set(FUNC(generalplus_gpspi_direct_game_state::read_external_space));
	m_maincpu->space_write_callback().set(FUNC(generalplus_gpspi_direct_game_state::write_external_space));
	m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(sunplus_gcm394_base_device::irq_vector_cb));
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
	m_maincpu->set_bootmode(0);
	m_maincpu->set_cs_config_callback(FUNC(gcm394_game_state::cs_callback));

	FULL_MEMORY(config, m_memory).set_map(&generalplus_gpspi_direct_game_state::cs_map_base);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	//m_screen->set_refresh_hz(20); // 20hz update gives more correct speed (and working inputs) in fixitflx and bfdigdug, but speed should probably be limited in some other way
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, (320*2)-1, 0, (240*2)-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "speaker", 2).front();
}

// Is there an internal ROM that gets mapped out or can this type really execute directly from scrambled SPI?
// there doesn't appear to be anywhere to map an internal ROM, nor access to it, so I think they just execute
// from SPI

ROM_START( fixitflx )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "fixitfelix_md25q16csig_c84015.bin", 0x0000, 0x200000, CRC(605c6863) SHA1(4f6cc2e8388e20eb90c6b05265273650eeea56eb) )
ROM_END

ROM_START( wiwcs )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "wwcs.img", 0x0000, 0x800000, CRC(9b86bc45) SHA1(17721c662642a257d3e0f56e351a9a80d75d9110) )

	ROM_REGION16_BE(0x400, "i2c", ROMREGION_ERASE00)
	ROM_LOAD( "witw_eeprom.bin", 0x0000, 0x400, CRC(9426350b) SHA1(c481ded8b1f61fbf2403532dabb9c0a5c2a33fa2) )
ROM_END

ROM_START( bfpacman )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "basicfunpacman_25q80_c84014.bin", 0x0000, 0x100000, CRC(dd39fc64) SHA1(48c0e1eb729f61b7359e1fd52b7faab56817dfe8) )
ROM_END

ROM_START( bfmpac )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "mspacman_25q80_c84014.bin", 0x0000, 0x100000, CRC(c0c3f8ce) SHA1(30da9b14f1a2c966167c97da9b8329f2f7f73291) )
ROM_END

ROM_START( bfdigdug )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "arcadeclassicsdigdug_25q80csig_c84014.bin", 0x0000, 0x100000, CRC(4030bc46) SHA1(8c086c96b9822e95c1862012786d6d6e59e0387e) )
ROM_END

ROM_START( bfgalaga )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "arcadeclassicsgalaga_25q80csig_c84014.bin", 0x0000, 0x100000, CRC(69982c9d) SHA1(0f8f403fefa7d8a9fdfcc04dca5a67919b662c7e) )
ROM_END

ROM_START( bfspyhnt )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "arcadeclassicsspyhunter_md25q16csig_c84015.bin", 0x0000, 0x200000, CRC(1f1eaabd) SHA1(1c484e0b0749123cfa1ac6d1959aefa6ed09ab20) )

	// also has a 24C04 (to store high scores?)
ROM_END

ROM_START( bftetris )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "arcadeclassicstetris_25q16ct_c84015.bin", 0x0000, 0x200000, CRC(a97e1bab) SHA1(400944d310d5d5fccb2c6d048d7bf0cb00da09de) )
ROM_END



ROM_START( punirune )
	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "25l64.ic103", 0x0000, 0x800000, CRC(0737edc0) SHA1(fce19d91a0522a75e676197fb18645b8c6a273b8) )
ROM_END

ROM_START( punij1m )
	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "japan_v1pcb_mint_25l6433f.ic103", 0x0000, 0x800000, CRC(76f28b5b) SHA1(be04d60c88df52951dd51eab2f5bf5f1dc2405e8) )
ROM_END

ROM_START( punij1pk ) // this might be the same software revision as punij1m with different save (or default save) data
	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "japan_v1pcb_pink_25l6433f.ic103", 0x0000, 0x800000, CRC(9268c881) SHA1(10bacfa48b3d02956d804396b652829ff868d947) )
ROM_END

ROM_START( punij1pu ) // different software revision to punij1m / punij1pk but same case style
	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "japan_v1pcb_purple_25l6433f.ic103", 0x0000, 0x800000, CRC(5b73bcb6) SHA1(109b6fa29693e7622c528d95d2a995d37a1cd8ca) )
ROM_END

ROM_START( punij2pk )
	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "japan_v2pcb_pink_gpr25l64.ic103", 0x0000, 0x800000, CRC(7ae9f009) SHA1(d762634a0442ff231837f9481a1203933c070df0) )
ROM_END

ROM_START( punifrnd )
	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "25oh64.ic3", 0x0000, 0x800000, CRC(622ca9b3) SHA1(4206393a4458ffcdb63352e743481865532fe8b5) )
ROM_END

ROM_START( pokgoget )
	ROM_REGION16_BE(0x2000000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "mx25l25645g.u1", 0x0000, 0x2000000, CRC(a76ae22f) SHA1(3fa5eeedb3fe343a7707d76710298377b22b0681) )
ROM_END

ROM_START( smkcatch )
	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "gpr25l64.u2", 0x0000, 0x800000,  CRC(e2f52c4a) SHA1(f79862d27152cff8f96151c672d9762a3897a593) )
ROM_END

ROM_START( dsgnpal )
	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "gpr25l64.ic2", 0x0000, 0x800000, CRC(a1017ea8) SHA1(bd4b553ff71e763cd3fd726c49f5408eac3b7984) )
ROM_END


void generalplus_gpspi_direct_game_state::init_fif()
{
	uint16_t* spirom16 = (uint16_t*)memregion("maincpu:spidirect")->base();
	for (int i = 0; i < 0x800000 / 2; i++)
	{
		spirom16[i] = bitswap<16>(spirom16[i] ^ 0xdd0d,
			3, 1, 11, 9, 6, 14, 0, 2, 8, 7, 13, 15, 4, 5, 12, 10);
	}

	// the games upload some self-check code to 0x100 in RAM, it's unclear what it is checking, skip it for now
	// goto mr -> nop
	if (spirom16[0x00d8] == 0xf161) spirom16[0x00d8] = 0xf165; // fixitflx, bfpacman, bfmpac
	if (spirom16[0x00ac] == 0xf161) spirom16[0x00ac] = 0xf165; // wiwcs, bfgalaga, bfdigdug, bfspyhnt
	if (spirom16[0x00a2] == 0xf161) spirom16[0x00a2] = 0xf165; // bftetris
}

} // anonymous namespace

CONS(2017, fixitflx, 0, 0, generalplus_gpspi_direct, bfmpac, generalplus_gpspi_direct_game_state, init_fif, "Basic Fun", "Fix It Felix Jr. (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2018, wiwcs,    0, 0, generalplus_gpspi_direct, bfmpac, generalplus_gpspi_direct_game_state, init_fif, "Basic Fun", "Where in the World Is Carmen Sandiego? (handheld)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2018, bfpacman, 0, 0, generalplus_gpspi_direct, bfmpac, generalplus_gpspi_direct_game_state, init_fif, "Basic Fun", "Pac-Man (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2017, bfmpac,   0, 0, generalplus_gpspi_direct, bfmpac, generalplus_gpspi_direct_game_state, init_fif, "Basic Fun", "Ms. Pac-Man (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2017, bfgalaga, 0, 0, generalplus_gpspi_direct, bfmpac, generalplus_gpspi_direct_game_state, init_fif, "Basic Fun", "Galaga (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2018, bfdigdug, 0, 0, generalplus_gpspi_direct, bfmpac, generalplus_gpspi_direct_game_state, init_fif, "Basic Fun", "Dig Dug (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2019, bfspyhnt, 0, 0, generalplus_gpspi_direct, bfspyhnt, generalplus_gpspi_direct_game_state, init_fif, "Basic Fun", "Spy Hunter (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2019, bftetris, 0, 0, generalplus_gpspi_direct, bfspyhnt, generalplus_gpspi_direct_game_state, init_fif, "Basic Fun", "Tetris (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// games below use GPL95101 series chips, which might be different but are definitely unSP2.0 chips that run from SPI directly

// unclear if colour matches, but there are multiple generations of these at least
// uses PUNIRUNZU_MAIN_V3 pcb
CONS(2021, punirune, 0, 0, generalplus_gpspi_direct, bfspyhnt, generalplus_gpspi_direct_game_state, empty_init, "Takara Tomy", "Punirunes (PUNIRUNZU_MAIN_V3, pastel blue, Europe)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// the case on these looks like the European release, including English title logo.  CPU is a glob, PUNIRUNZU_MAIN_DICE_V1 on PCB
CONS(2021, punij1m,  punirune, 0, generalplus_gpspi_direct, bfspyhnt, generalplus_gpspi_direct_game_state, empty_init, "Takara Tomy", "Punirunes (PUNIRUNZU_MAIN_DICE_V1, mint, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2021, punij1pk, punirune, 0, generalplus_gpspi_direct, bfspyhnt, generalplus_gpspi_direct_game_state, empty_init, "Takara Tomy", "Punirunes (PUNIRUNZU_MAIN_DICE_V1, pink, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2021, punij1pu, punirune, 0, generalplus_gpspi_direct, bfspyhnt, generalplus_gpspi_direct_game_state, empty_init, "Takara Tomy", "Punirunes (PUNIRUNZU_MAIN_DICE_V1, purple, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// the case on these is similar to the above, but the text is in Japanese, uses PUNIRUNZU_MAIN_V2 on pcb
CONS(2021, punij2pk, punirune, 0, generalplus_gpspi_direct, bfspyhnt, generalplus_gpspi_direct_game_state, empty_init, "Takara Tomy", "Punirunes (PUNIRUNZU_MAIN_V2, pink, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// has a link feature
CONS(2021, punifrnd, 0,        0, generalplus_gpspi_direct, bfspyhnt, generalplus_gpspi_direct_game_state, empty_init, "Takara Tomy", u8"Punirunes Punitomo Tsūshin (hot pink, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// Pocket Monsters ガチッとゲットだぜ! モンスターボールゴー! - Pocket Monsters is printed on the inner shell, but not the box?
CONS(2021, pokgoget, 0,        0, generalplus_gpspi_direct, bfspyhnt, generalplus_gpspi_direct_game_state, empty_init, "Takara Tomy", "Gachitto Get da ze! Monster Ball Go! (210406, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)


// 2020 (device) / 2021 (box) version of Sumikko Gurashi a cloud shaped device
// Sumikko Gurashi - Sumikko Catch (すみっコぐらし すみっコキャッチ)
CONS( 2021, smkcatch, 0, 0, generalplus_gpspi_direct, bfmpac, generalplus_gpspi_direct_game_state, empty_init,  "San-X / Tomy", "Sumikko Gurashi - Sumikko Catch (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
// or Sumikko Gurashi - Sumikko Catch DX (すみっコぐらし すみっコキャッチDX) = Sumikko Catch with pouch and strap

// there seem to be different versions of this available, is the software the same?
CONS( 201?, dsgnpal, 0, 0, generalplus_gpspi_direct, bfmpac, generalplus_gpspi_direct_game_state, empty_init,  "Tomy", "Kiratto Pri-Chan Design Palette (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
