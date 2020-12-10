// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
	These seem to be GPL16250 related based on video register use
	however the SPI ROM maps directly into CPU space, where you'd
	expect internal ROM to be?!
*/

#include "emu.h"
#include "includes/generalplus_gpl16250.h"

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
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
};

void generalplus_gpspi_direct_game_state::machine_start()
{
}

void generalplus_gpspi_direct_game_state::machine_reset()
{
	m_maincpu->reset(); // reset CPU so vector gets read etc.
}

static INPUT_PORTS_START( gcm394 )
	PORT_START("IN0")
	PORT_START("IN1")
	PORT_START("IN2")
INPUT_PORTS_END


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
	m_maincpu->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_maincpu->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
	m_maincpu->set_bootmode(0);
	m_maincpu->set_cs_config_callback(FUNC(gcm394_game_state::cs_callback));

	FULL_MEMORY(config, m_memory).set_map(&generalplus_gpspi_direct_game_state::cs_map_base);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, (320*2)-1, 0, (240*2)-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

// Is there an internal ROM that gets mapped out or can this type really execute directly from scrambled SPI?

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

void generalplus_gpspi_direct_game_state::init_fif()
{
	uint16_t* spirom16 = (uint16_t*)memregion("maincpu:spidirect")->base();
	for (int i = 0; i < 0x800000 / 2; i++)
	{
		spirom16[i] = bitswap<16>(spirom16[i] ^ 0xdd0d,
			3, 1, 11, 9, 6, 14, 0, 2, 8, 7, 13, 15, 4, 5, 12, 10);
	}
}

CONS(2017, fixitflx, 0, 0, generalplus_gpspi_direct, gcm394, generalplus_gpspi_direct_game_state , init_fif, "Basic Fun", "Fix It Felix Jr. (Mini Arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2018, wiwcs,    0, 0, generalplus_gpspi_direct, gcm394, generalplus_gpspi_direct_game_state , init_fif, "Basic Fun", "Where in the World is Carmen Sandiego? (Handheld)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
