// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    (unSP 2.0 based System on a Chip)

    JAKKS call this GPAC800, other sources (including Pac-Man Connect and Play test mode) call it GPL16250

    die markings GCM394
     - Smart Fit Park
     - Spongebob Bikini Bottom 500
     - Mobigo2 (sunplus_unsp20soc_mobigo.cpp)

    some of the systems here might use newer dies but the video etc. appears the same.

    Compared to vii.cpp this is clearly newer, has extra opcodes, different internal map etc. also scaling and higher resolutions based on Spongebob

    note, these SoC types always have a 128Kwords internal ROM, which the JAKKS games appear to use for basic bootstrap purposes.

    GPAC800 / GCM394 (SpongeBob Bikini Bottom 500 Test Mode also calls this GPAC800, even if the mappings appear different to the NAND version below - different CS base, maybe just depends on boot mode?)
        Smart Fit Park
        SpongeBob SquarePants Bikini Bottom 500
        Spiderman - The Masked Menace 'Spider Sense' (pad type with Spiderman model)
        (Wireless Hunting? - maybe, register map looks the same even if it sets stack to 2fff not 6fff)

    GPAC800 (with NAND support)
        Wireless Air 60
        Golden Tee Golf
        Cars 2
        Toy Story Mania
        V.Baby
        Playskool Heroes Transformers Rescue Bots Beam Box

    GPAC500 (based on test modes, unknown hardware, might be GPAC800 but without the higher resolution support?)
        The Price is Right
        Bejeweled? (might be GPAC800)

    Notes
        smartfp: hold button Circle, Star and Home on startup for Test Menu

    these are all unsp 2.0 type, as they use the extended ocpodes


    NAND types:

    Toy Story Mania H27U518S2C dumped as HY27US08121A (512+16) x 32 x 4096
    Beam Box GPR27P512A dumped as HY27US08121A (512+16) x 32 x 4096
    Golden Tee GPR27P512A dumped as HY27US08121A (512+16) x 32 x 4096
    Cars 2 GPR27P512A dumped as HY27US08121A (512+16) x 32 x 4096

    V.Baby HY27UF081G2A (2048+64) x 64 x 1024



    Non-emulation bugs (happen on real hardware):
        paccon:  Pac-Man - Bottom set of Power Pills are squashed.
                 Galaga - Incorrect sprite used for left shot in 'Double Ship' mode


    JAKKS Pacific Test modes:

    jak_hmhsm : uses the standard JAKKS code (on first screen - Hold Up, Hold A, Release Up, Down)
                the High School Musical part has its own test mode which tests a different part of the ROM, use the same code but after selecting the game from menu


    --- the individual CS spaces could (and probably should?) be done with a bunch of extra memory spaces rather than the cs0_r / cs0_w etc. but that would mean yet
        another trip through the memory system for almost everything and at ~100Mhz that is slow.

*/

#include "emu.h"
#include "generalplus_gpl16250.h"


uint16_t gcm394_game_state::cs0_r(offs_t offset)
{
	return m_romregion[offset & 0x3fffff];
}

void gcm394_game_state::cs0_w(offs_t offset, uint16_t data)
{
	logerror("cs0_w %04x %04x (to ROM!)\n", offset, data);
}

uint16_t gcm394_game_state::cs1_r(offs_t offset) { logerror("cs1_r %06n", offset); return 0x0000; }
void gcm394_game_state::cs1_w(offs_t offset, uint16_t data) { logerror("cs1_w %06x %04x\n", offset, data); }
uint16_t gcm394_game_state::cs2_r(offs_t offset) { logerror("cs2_r %06n", offset); return 0x0000; }
void gcm394_game_state::cs2_w(offs_t offset, uint16_t data) { logerror("cs2_w %06x %04x\n", offset, data); }
uint16_t gcm394_game_state::cs3_r(offs_t offset) { logerror("cs3_r %06n", offset); return 0x0000; }
void gcm394_game_state::cs3_w(offs_t offset, uint16_t data) { logerror("cs3_w %06x %04x\n", offset, data); }
uint16_t gcm394_game_state::cs4_r(offs_t offset) { logerror("cs4_r %06n", offset); return 0x0000; }
void gcm394_game_state::cs4_w(offs_t offset, uint16_t data) { logerror("cs4_w %06x %04x\n", offset, data); }

void gcm394_game_state::cs_map_base(address_map& map)
{
}

uint16_t gcm394_game_state::read_external_space(offs_t offset)
{
	return m_memory->get_program()->read_word(offset);
}

void gcm394_game_state::write_external_space(offs_t offset, uint16_t data)
{
	m_memory->get_program()->write_word(offset, data);
}

uint16_t gcm394_game_state::porta_r()
{
	uint16_t data = m_io[0]->read();
	logerror("Port A Read: %04x\n", data);
	return data;
}

uint16_t gcm394_game_state::portb_r()
{
	uint16_t data = m_io[1]->read();
	logerror("Port B Read: %04x\n", data);
	return data;
}

uint16_t gcm394_game_state::portc_r()
{
	uint16_t data = m_io[2]->read();
	logerror("Port C Read: %04x\n", data);
	return data;
}

void gcm394_game_state::porta_w(uint16_t data)
{
	logerror("%s: Port A:WRITE %04x\n", machine().describe_context(), data);
}

// some sources indicate these later SoC types run at 96Mhz, others indicate 48Mhz.
// unSP 2.0 CPUs have a lower average CPI too (2 instead of 6 on unSP 1.0 or 5 on unSP 1.1 / 1.2 / unSP 2.0) so using regular unSP timings might result in things being too slow
// as with the older SunPlus chips this appears to be an fully internally generated frequency, external XTALs again are typically 6MHz or simply not present.

void gcm394_game_state::base(machine_config &config)
{
	GCM394(config, m_maincpu, 96000000, m_screen);
	m_maincpu->porta_in().set(FUNC(gcm394_game_state::porta_r));
	m_maincpu->portb_in().set(FUNC(gcm394_game_state::portb_r));
	m_maincpu->portc_in().set(FUNC(gcm394_game_state::portc_r));
	m_maincpu->porta_out().set(FUNC(gcm394_game_state::porta_w));
	m_maincpu->space_read_callback().set(FUNC(gcm394_game_state::read_external_space));
	m_maincpu->space_write_callback().set(FUNC(gcm394_game_state::write_external_space));
	m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(sunplus_gcm394_base_device::irq_vector_cb));
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
	m_maincpu->set_bootmode(1); // boot from external ROM / CS mirror
	m_maincpu->set_cs_config_callback(FUNC(gcm394_game_state::cs_callback));

	FULL_MEMORY(config, m_memory).set_map(&gcm394_game_state::cs_map_base);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, (320*2)-1, 0, (240*2)-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "speaker", 2).front();
}




void gcm394_game_state::machine_start()
{
}

void gcm394_game_state::machine_reset()
{
	cs_callback(0x00, 0x00, 0x00, 0x00, 0x00);
	m_maincpu->set_cs_space(m_memory->get_program());

	m_maincpu->reset(); // reset CPU so vector gets read etc.

	//m_maincpu->set_paldisplaybank_high_hack(1);
	m_maincpu->set_alt_tile_addressing_hack(0);
}

void gcm394_game_state::cs_callback(uint16_t cs0, uint16_t cs1, uint16_t cs2, uint16_t cs3, uint16_t cs4)
{
	// wipe existing mappings;
	m_memory->get_program()->unmap_readwrite(0, (0x8000000*5)-1);
	m_memory->get_program()->nop_readwrite(0, (0x8000000*5)-1); // stop logging spam if video params are invalid

	int start_address = 0;
	int end_address;

	int size; // cs region sizes in kwords

	size = (((cs0 & 0xff00) >> 8) + 1) * 0x10000;
	end_address = start_address + (size - 1);
	logerror("installing cs0 handler start_address %08x end_address %08x\n", start_address, end_address);
	m_memory->get_program()->install_readwrite_handler( start_address, end_address, read16sm_delegate(*this, FUNC(gcm394_game_state::cs0_r)), write16sm_delegate(*this, FUNC(gcm394_game_state::cs0_w)));
	start_address += size;

	size = (((cs1 & 0xff00) >> 8) + 1) * 0x10000;
	end_address = start_address + (size - 1);
	logerror("installing cs1 handler start_address %08x end_address %08x\n", start_address, end_address);
	m_memory->get_program()->install_readwrite_handler( start_address, end_address, read16sm_delegate(*this, FUNC(gcm394_game_state::cs1_r)), write16sm_delegate(*this, FUNC(gcm394_game_state::cs1_w)));
	start_address += size;

	size = (((cs2 & 0xff00) >> 8) + 1) * 0x10000;
	end_address = start_address + (size - 1);
	logerror("installing cs2 handler start_address %08x end_address %08x\n", start_address, end_address);
	m_memory->get_program()->install_readwrite_handler( start_address, end_address, read16sm_delegate(*this, FUNC(gcm394_game_state::cs2_r)), write16sm_delegate(*this, FUNC(gcm394_game_state::cs2_w)));
	start_address += size;

	size = (((cs3 & 0xff00) >> 8) + 1) * 0x10000;
	end_address = start_address + (size - 1);
	logerror("installing cs3 handler start_address %08x end_address %08x\n", start_address, end_address);
	m_memory->get_program()->install_readwrite_handler( start_address, end_address, read16sm_delegate(*this, FUNC(gcm394_game_state::cs3_r)), write16sm_delegate(*this, FUNC(gcm394_game_state::cs3_w)));
	start_address += size;

	size = (((cs4 & 0xff00) >> 8) + 1) * 0x10000;
	end_address = start_address + (size - 1);
	logerror("installing cs4 handler start_address %08x end_address %08x\n", start_address, end_address);
	m_memory->get_program()->install_readwrite_handler( start_address, end_address, read16sm_delegate(*this, FUNC(gcm394_game_state::cs4_r)), write16sm_delegate(*this, FUNC(gcm394_game_state::cs4_w)));
	//start_address += size;
}

