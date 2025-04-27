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
	void init_siddr();

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

ROM_START( siddr )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x800000, "maincpu:spidirect", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "ddr-toy.bin", 0x0000, 0x400000, CRC(873cbcc8) SHA1(bdd3d12adb1284991a3f8aaa8e451e3a55931267) )
ROM_END

void generalplus_gpspi_direct_game_state::init_fif()
{
	uint16_t* spirom16 = (uint16_t*)memregion("maincpu:spidirect")->base();
	for (int i = 0; i < 0x800000 / 2; i++)
	{
		spirom16[i] = bitswap<16>(spirom16[i] ^ 0xdd0d,
			3, 1, 11, 9, 6, 14, 0, 2, 8, 7, 13, 15, 4, 5, 12, 10);
	}
}

void generalplus_gpspi_direct_game_state::init_siddr()
{
	uint8_t* spirom8 = (uint8_t*)memregion("maincpu:spidirect")->base();
	for (int i = 0x3000; i < 0x400000; i++)
	{
		spirom8[i] = bitswap<8>(spirom8[i] ^ 0x68,
			3, 5, 0, 7,    1, 4, 2, 6);
	}
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

// probably not identical hardware, encryption is different, but it does seem to still be a 'direct access' SPI ROM case
CONS(201?, siddr,    0, 0, generalplus_gpspi_direct, bfspyhnt, generalplus_gpspi_direct_game_state, init_siddr, "Super Impulse", "Dance Dance Revolution - Broadwalk Arcade", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
