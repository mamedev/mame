// license:BSD-3-Clause
// copyright-holders:fluff_, David Haywood

/*
    Furby Connect uses GPL162004A SoC, which seems like a GPL16250's (GPAC1800's) brother
    Has NAND + SDRAM configuration, also has an SPI flash
    NAND has also FAT32 filesystem, and has GPspispisp header in its GameCode.bin file (why not GPnandnand?)
	The bootstrap is quite needed in this situation. I think it finds GameCode.bin file from filesystem, and then copies all of it (to 0x50000)
	The bootstrap is on SPI flash (https://github.com/swarley7/furbhax), but data in dump (which is provided on git repo) seems to not be unSP bytecode. (TODO: further investigation required!)
	Currently, the task is to implement/get normal dump/investigate current dump, so it boots as it is and will be able to run 
	In this state it seems like there is a problem with DMA or NAND. Most likely, the problem is with incorrect NAND formating, because current NAND is reformatted so GameCode will be from 0x0, which is incorrect.

    --GeneralPlus GPL162004A-- --EtronTech EM639165TS-7G-- --Toshiba TC58BVG0S3HTA00-- --GeneralPlus GPR25L081B--
*/

#include "emu.h"
#include "generalplus_gpl16250_nand.h"
#include "generalplus_gpl16250_mobigo.cpp"
#include "generalplus_gpl16250_spi.cpp"
#include "screen.h"

namespace {
	class furby_connect_state : public generalplus_gpac800_game_state
	{
	public:
		furby_connect_state(const machine_config &mconfig, device_type type, const char *tag) :
			generalplus_gpac800_game_state(mconfig, type, tag)
		{}
	
		void furby_connect(machine_config &config);
	protected:
		virtual void machine_reset() override ATTR_COLD;
	private:
		//void map(address_map &map) ATTR_COLD;
	};

	void furby_connect_state::machine_reset()
	{
		// configure CS defaults
		[[maybe_unused]] address_space& mem = m_maincpu->space(AS_PROGRAM);
		mem.write_word(0x007820, 0x0247);
		mem.write_word(0x007821, 0xff47);
		mem.write_word(0x007822, 0x0047);
		mem.write_word(0x007823, 0xfec7);
		mem.write_word(0x007824, 0x0047);

		m_maincpu->set_cs_space(m_memory->get_program());

		if (m_nandregion)
		{
			popmessage("123");
			nand_create_stripped_region();
		}
		
		m_maincpu->reset(); // reset CPU so vector gets read etc.

		//m_maincpu->set_paldisplaybank_high_hack(0);
		m_maincpu->set_alt_tile_addressing_hack(1);
	}

	void furby_connect_state::furby_connect(machine_config &config)
	{
		GPAC800(config, m_maincpu, 96000000, m_screen);
		m_maincpu->porta_in().set(FUNC(furby_connect_state::porta_r));
		m_maincpu->portb_in().set(FUNC(furby_connect_state::portb_r));
		m_maincpu->portc_in().set(FUNC(furby_connect_state::portc_r));
		m_maincpu->porta_out().set(FUNC(furby_connect_state::porta_w));
		m_maincpu->space_read_callback().set(FUNC(furby_connect_state::read_external_space));
		m_maincpu->space_write_callback().set(FUNC(furby_connect_state::write_external_space));
		m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(sunplus_gcm394_base_device::irq_vector_cb));
		m_maincpu->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
		m_maincpu->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
		m_maincpu->set_bootmode(0);
		m_maincpu->set_cs_config_callback(FUNC(furby_connect_state::cs_callback));
	
		m_maincpu->nand_read_callback().set(FUNC(furby_connect_state::read_nand));
	
		//m_maincpu->set_addrmap(AS_PROGRAM, &furby_connect_state::map);
		FULL_MEMORY(config, m_memory).set_map(&generalplus_gpac800_game_state::cs_map_base);
	
		SCREEN(config, m_screen, SCREEN_TYPE_LCD);
		m_screen->set_refresh_hz(60);
		m_screen->set_size(256, 128);
		m_screen->set_visarea(0, (256)-1, 0, (128)-1);
		m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
		m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));
		
		SPEAKER(config, "lspeaker").front_left();
		SPEAKER(config, "rspeaker").front_right();

	}

	// void furby_connect_state::map(address_map &map)
	// {
	// 	map(0x00000 * 0x2, 0x6FFFF * 0x2).ram();
	// 	map(0x07800 * 0x2, 0x277FF + 0x1).rom().region("maincpu:internal", 0x0);
	// }
	
	//void furby_connect_state::init_furb()
	//{
	//}
	
	ROM_START( furby_connect )
		ROM_REGION16_BE( 0x42000, "maincpu:internal", ROMREGION_ERASE00 )
		ROM_LOAD16_WORD_SWAP( "uboot.bin", 0x0000, 0x42000, CRC(82739ac0))

		ROM_REGION16_BE( 0x100000, "maincpu", ROMREGION_ERASE00 )
		ROM_LOAD16_WORD_SWAP( "spi.bin", 0x0000, 0x100000, CRC(cb819c04))
	
	 	ROM_REGION( 0x8400000, "nandrom", ROMREGION_ERASE00 )
		ROM_LOAD( "nand.bin", 0x0000, 0x6d00000, CRC(b64b6db3)) // Toshiba TC58BVG0S3HTA00
	ROM_END
		
	} // anonymous namespace

 CONS( 2016, furby_connect, 0,      0, furby_connect, mobigo, furby_connect_state, nand_fc2016, "Hasbro", "Furby Connect", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
