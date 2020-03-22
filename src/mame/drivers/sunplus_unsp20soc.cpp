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

*/

#include "emu.h"
#include "includes/sunplus_unsp20soc.h"


// device type definition
DEFINE_DEVICE_TYPE(FULL_MEMORY, full_memory_device, "full_memory", "SunPlus Full CS Memory Map")

full_memory_device::full_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, FULL_MEMORY, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_program(nullptr)
{
}

device_memory_interface::space_config_vector full_memory_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

/*
':maincpu' (00F87F):possible DMA operation (7abf) (trigger 0001) with params mode:4009 source:00040000 (word offset) dest:00830000 (word offset) length:00007800 (words)
':maincpu' (002384):possible DMA operation (7abf) (trigger 0001) with params mode:0009 source:00180000 (word offset) dest:00840000 (word offset) length:00160000 (words)

':maincpu' (05048D):possible DMA operation (7abf) (trigger 0001) with params mode:0089 source:00006fa3 (word offset) dest:000025bc (word offset) length:000001e0 (words)
':maincpu' (05048D):possible DMA operation (7abf) (trigger 0001) with params mode:0089 source:00006fa3 (word offset) dest:000024cc (word offset) length:000000f0 (words)
':maincpu' (05048D):possible DMA operation (7abf) (trigger 0001) with params mode:0089 source:00006fa3 (word offset) dest:00000002 (word offset) length:00000400 (words)
':maincpu' (05048D):possible DMA operation (7abf) (trigger 0001) with params mode:0089 source:00006fa3 (word offset) dest:00000402 (word offset) length:00000400 (words)
':maincpu' (05048D):possible DMA operation (7abf) (trigger 0001) with params mode:0089 source:00006fa3 (word offset) dest:00000802 (word offset) length:00000400 (words)

gtg
':maincpu' (005ACE):possible DMA operation (7abf) (trigger 0001) with params mode:1089 source:30007854 (word offset) dest:00030000 (word offset) length:00000200 (words)
':maincpu' (005ACE):possible DMA operation (7abf) (trigger 0001) with params mode:1089 source:30007854 (word offset) dest:00030100 (word offset) length:00000200 (words)
':maincpu' (005ACE):possible DMA operation (7abf) (trigger 0001) with params mode:1089 source:30007854 (word offset) dest:00030200 (word offset) length:00000200 (words)
':maincpu' (005ACE):possible DMA operation (7abf) (trigger 0001) with params mode:1089 source:30007854 (word offset) dest:00030300 (word offset) length:00000200 (words)
':maincpu' (005ACE):possible DMA operation (7abf) (trigger 0001) with params mode:1089 source:30007854 (word offset) dest:00030400 (word offset) length:00000200 (words)

*/

void full_memory_device::device_config_complete()
{
	m_program_config = address_space_config( "program", ENDIANNESS_BIG, 16, 32, -1 );
}

void full_memory_device::device_start()
{
	m_program = &space(AS_PROGRAM);
}



READ16_MEMBER(gcm394_game_state::cs0_r)
{
	return m_romregion[offset & 0x3fffff];
}

WRITE16_MEMBER(gcm394_game_state::cs0_w)
{
	logerror("cs0_w %04x %04x (to ROM!)\n", offset, data);
}

READ16_MEMBER(gcm394_game_state::cs1_r) { logerror("cs1_r %06n", offset); return 0x0000; }
WRITE16_MEMBER(gcm394_game_state::cs1_w) { logerror("cs1_w %06x %04x\n", offset, data); }
READ16_MEMBER(gcm394_game_state::cs2_r) { logerror("cs2_r %06n", offset); return 0x0000; }
WRITE16_MEMBER(gcm394_game_state::cs2_w) { logerror("cs2_w %06x %04x\n", offset, data); }
READ16_MEMBER(gcm394_game_state::cs3_r) { logerror("cs3_r %06n", offset); return 0x0000; }
WRITE16_MEMBER(gcm394_game_state::cs3_w) { logerror("cs3_w %06x %04x\n", offset, data); }
READ16_MEMBER(gcm394_game_state::cs4_r) { logerror("cs4_r %06n", offset); return 0x0000; }
WRITE16_MEMBER(gcm394_game_state::cs4_w) { logerror("cs4_w %06x %04x\n", offset, data); }


/*
    map info (NAND type)

    map(0x000000, 0x006fff) internal RAM
    map(0x007000, 0x007fff) internal peripherals
    map(0x008000, 0x00ffff) internal ROM (lower 32kwords) - can also be configured to mirror CS0 308000 area with external pin for boot from external ROM
    map(0x010000, 0x027fff) internal ROM (upper 96kwords) - can't be switched
    map(0x028000, 0x02ffff) reserved

    map(0x030000, 0x0.....) view into external spaces (CS0 area starts here. followed by CS1 area, CS2 area etc.)

    map(0x200000, 0x3fffff) continued view into external spaces, but this area is banked with m_membankswitch_7810 (valid bank values 0x00-0x3f)
*/



void gcm394_game_state::cs_map_base(address_map& map)
{
}





READ16_MEMBER(wrlshunt_game_state::cs0_r)
{
	return m_romregion[offset & m_romwords_mask];
}

WRITE16_MEMBER(wrlshunt_game_state::cs0_w)
{
	logerror("cs0_w write to ROM?\n");
	//m_romregion[offset & 0x3ffffff] = data;
}

READ16_MEMBER(wrlshunt_game_state::cs1_r)
{
	return m_sdram[offset & 0x3fffff];
}

WRITE16_MEMBER(wrlshunt_game_state::cs1_w)
{
	m_sdram[offset & 0x3fffff] = data;
}


void wrlshunt_game_state::machine_start()
{
	m_romwords_mask = (memregion("maincpu")->bytes()/2)-1;
	save_item(NAME(m_sdram));
}

void wrlshunt_game_state::machine_reset()
{
	cs_callback(0x00, 0x00, 0x00, 0x00, 0x00);
	m_maincpu->set_cs_space(m_memory->get_program());
	m_maincpu->reset(); // reset CPU so vector gets read etc.

	m_maincpu->set_paldisplaybank_high_hack(1);
	m_maincpu->set_alt_tile_addressing_hack(1);
}

READ16_MEMBER(jak_s500_game_state::porta_r)
{
	uint16_t data = m_io[0]->read();
	logerror("%s: Port A Read: %04x\n", machine().describe_context(), data);

	//address_space& mem = m_maincpu->space(AS_PROGRAM);

	//if (mem.read_word(0x22b408) == 0x4846)
	//  mem.write_word(0x22b408, 0x4840);    // jak_s500 force service mode

	//if (mem.read_word(0x236271) == 0x4846)
	//  mem.write_word(0x236271, 0x4840);    // jak_totm force service mode

	//if (mem.read_word(0x22D6F7) == 0x4846)
	//  mem.write_word(0x22D6F7, 0x4840);    // jak_pf force service mode

	//if (mem.read_word(0x23E295) == 0x4846)
	//  mem.write_word(0x23E295, 0x4840);    // jak_smwm force service mode

	return data;
}

READ16_MEMBER(jak_s500_game_state::portb_r)
{
	uint16_t data = m_io[1]->read();
	logerror("%s: Port B Read: %04x\n", machine().describe_context(), data);
	return data;
}


void jak_s500_game_state::machine_reset()
{
	cs_callback(0x00, 0x00, 0x00, 0x00, 0x00);
	m_maincpu->set_cs_space(m_memory->get_program());
	m_maincpu->reset(); // reset CPU so vector gets read etc.

	m_maincpu->set_paldisplaybank_high_hack(0);
	m_maincpu->set_alt_tile_addressing_hack(1);
}


void generalplus_gpspispi_game_state::machine_start()
{
}

void generalplus_gpspispi_game_state::machine_reset()
{
	m_maincpu->reset(); // reset CPU so vector gets read etc.

	m_maincpu->set_paldisplaybank_high_hack(0);
	m_maincpu->set_alt_tile_addressing_hack(1);
}



void wrlshunt_game_state::init_wrlshunt()
{
	m_sdram.resize(0x400000); // 0x400000 words, 0x800000 bytes
}

void wrlshunt_game_state::init_ths()
{
	m_sdram.resize(0x400000); // 0x400000 words, 0x800000 bytes (verify)
}



READ16_MEMBER(generalplus_gpac800_game_state::cs0_r)
{
	return m_sdram2[offset & 0xffff];
}

WRITE16_MEMBER(generalplus_gpac800_game_state::cs0_w)
{
	m_sdram2[offset & 0xffff] = data;
}

READ16_MEMBER(generalplus_gpac800_game_state::cs1_r)
{
	return m_sdram[offset & (m_sdram_kwords-1)];
}

WRITE16_MEMBER(generalplus_gpac800_game_state::cs1_w)
{
	m_sdram[offset & (m_sdram_kwords-1)] = data;
}

READ8_MEMBER(generalplus_gpac800_game_state::read_nand)
{
	if (!m_nandregion)
		return 0x0000;

	return (offset < m_size) ? m_nandregion[offset] : 0xFF;
	//return m_strippedrom[offset & (m_strippedsize - 1)];
}

READ16_MEMBER(gcm394_game_state::read_external_space)
{
	return m_memory->get_program()->read_word(offset);
}

WRITE16_MEMBER(gcm394_game_state::write_external_space)
{
	m_memory->get_program()->write_word(offset, data);
}

READ16_MEMBER(gcm394_game_state::porta_r)
{
	uint16_t data = m_io[0]->read();
	logerror("Port A Read: %04x\n", data);
	return data;
}

READ16_MEMBER(gcm394_game_state::portb_r)
{
	uint16_t data = m_io[1]->read();
	logerror("Port B Read: %04x\n", data);
	return data;
}

READ16_MEMBER(gcm394_game_state::portc_r)
{
	uint16_t data = m_io[2]->read();
	logerror("Port C Read: %04x\n", data);
	return data;
}

WRITE16_MEMBER(gcm394_game_state::porta_w)
{
	logerror("%s: Port A:WRITE %04x\n", machine().describe_context(), data);
}

// some sources indicate these later SoC types run at 96Mhz, others indicate 48Mhz.
// unSP 2.0 CPUs have a lower average CPI too (2 instead of 6 on unSP 1.0 or 5 on unSP 1.1 / 1.2 / unSP 2.0) so using regular unSP timings might result in things being too slow
// as with the older SunPlus chips this appears to be an fully internally generated frequency, external XTALs again are typically 6MHz or simply not present.

void gcm394_game_state::base(machine_config &config)
{
	GCM394(config, m_maincpu, 96000000/2, m_screen);
	m_maincpu->porta_in().set(FUNC(gcm394_game_state::porta_r));
	m_maincpu->portb_in().set(FUNC(gcm394_game_state::portb_r));
	m_maincpu->portc_in().set(FUNC(gcm394_game_state::portc_r));
	m_maincpu->porta_out().set(FUNC(gcm394_game_state::porta_w));
	m_maincpu->space_read_callback().set(FUNC(gcm394_game_state::read_external_space));
	m_maincpu->space_write_callback().set(FUNC(gcm394_game_state::write_external_space));
	m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(sunplus_gcm394_base_device::irq_vector_cb));
	m_maincpu->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_maincpu->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
	m_maincpu->set_bootmode(1); // boot from external ROM / CS mirror
	m_maincpu->set_cs_config_callback(FUNC(gcm394_game_state::cs_callback));

	FULL_MEMORY(config, m_memory).set_map(&gcm394_game_state::cs_map_base);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, (320*2)-1, 0, (240*2)-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

void wrlshunt_game_state::wrlshunt(machine_config &config)
{
	gcm394_game_state::base(config);
}

void wrlshunt_game_state::paccon(machine_config &config)
{
	gcm394_game_state::base(config);

	m_maincpu->set_clock(96000000); // needs to be somewhere in this region or the emulator running on the SunPlus is too slow
}


void tkmag220_game_state::tkmag220(machine_config &config)
{
	gcm394_game_state::base(config);

	m_maincpu->porta_in().set_ioport("IN0");
	m_maincpu->portb_in().set_ioport("IN1");
	m_maincpu->portc_in().set_ioport("IN2");
}


READ16_MEMBER(wrlshunt_game_state::porta_r)
{
	uint16_t data = m_io[0]->read();
	logerror("%s: Port A Read: %04x\n",  machine().describe_context(), data);
	return data;
}

WRITE16_MEMBER(wrlshunt_game_state::porta_w)
{
	logerror("%s: Port A:WRITE %04x\n", machine().describe_context(), data);

	// HACK
	address_space& mem = m_maincpu->space(AS_PROGRAM);
	if (mem.read_word(0x5b354) == 0xafd0)   // wrlshubt - skip check (EEPROM?)
		mem.write_word(0x5b354, 0xB403);
}


void generalplus_gpac800_game_state::generalplus_gpac800(machine_config &config)
{
	GPAC800(config, m_maincpu, 96000000/2, m_screen);
	m_maincpu->porta_in().set(FUNC(generalplus_gpac800_game_state::porta_r));
	m_maincpu->portb_in().set(FUNC(generalplus_gpac800_game_state::portb_r));
	m_maincpu->portc_in().set(FUNC(generalplus_gpac800_game_state::portc_r));
	m_maincpu->porta_out().set(FUNC(generalplus_gpac800_game_state::porta_w));
	m_maincpu->space_read_callback().set(FUNC(generalplus_gpac800_game_state::read_external_space));
	m_maincpu->space_write_callback().set(FUNC(generalplus_gpac800_game_state::write_external_space));
	m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(sunplus_gcm394_base_device::irq_vector_cb));
	m_maincpu->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_maincpu->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
	m_maincpu->set_bootmode(0); // boot from internal ROM (NAND bootstrap)
	m_maincpu->set_cs_config_callback(FUNC(gcm394_game_state::cs_callback));

	m_maincpu->nand_read_callback().set(FUNC(generalplus_gpac800_game_state::read_nand));

	FULL_MEMORY(config, m_memory).set_map(&generalplus_gpac800_game_state::cs_map_base);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, (320*2)-1, 0, (240*2)-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

DEVICE_IMAGE_LOAD_MEMBER(generalplus_gpac800_vbaby_game_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

void generalplus_gpac800_vbaby_game_state::generalplus_gpac800_vbaby(machine_config &config)
{
	generalplus_gpac800_game_state::generalplus_gpac800(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "vbaby_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(generalplus_gpac800_vbaby_game_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("vbaby_cart");
}


void gcm394_game_state::machine_start()
{
}

void gcm394_game_state::machine_reset()
{
	cs_callback(0x00, 0x00, 0x00, 0x00, 0x00);
	m_maincpu->set_cs_space(m_memory->get_program());

	m_maincpu->reset(); // reset CPU so vector gets read etc.

	m_maincpu->set_paldisplaybank_high_hack(1);
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
	m_memory->get_program()->install_readwrite_handler( start_address, end_address, read16_delegate(*this, FUNC(gcm394_game_state::cs0_r)), write16_delegate(*this, FUNC(gcm394_game_state::cs0_w)));
	start_address += size;

	size = (((cs1 & 0xff00) >> 8) + 1) * 0x10000;
	end_address = start_address + (size - 1);
	logerror("installing cs1 handler start_address %08x end_address %08x\n", start_address, end_address);
	m_memory->get_program()->install_readwrite_handler( start_address, end_address, read16_delegate(*this, FUNC(gcm394_game_state::cs1_r)), write16_delegate(*this, FUNC(gcm394_game_state::cs1_w)));
	start_address += size;

	size = (((cs2 & 0xff00) >> 8) + 1) * 0x10000;
	end_address = start_address + (size - 1);
	logerror("installing cs2 handler start_address %08x end_address %08x\n", start_address, end_address);
	m_memory->get_program()->install_readwrite_handler( start_address, end_address, read16_delegate(*this, FUNC(gcm394_game_state::cs2_r)), write16_delegate(*this, FUNC(gcm394_game_state::cs2_w)));
	start_address += size;

	size = (((cs3 & 0xff00) >> 8) + 1) * 0x10000;
	end_address = start_address + (size - 1);
	logerror("installing cs3 handler start_address %08x end_address %08x\n", start_address, end_address);
	m_memory->get_program()->install_readwrite_handler( start_address, end_address, read16_delegate(*this, FUNC(gcm394_game_state::cs3_r)), write16_delegate(*this, FUNC(gcm394_game_state::cs3_w)));
	start_address += size;

	size = (((cs4 & 0xff00) >> 8) + 1) * 0x10000;
	end_address = start_address + (size - 1);
	logerror("installing cs4 handler start_address %08x end_address %08x\n", start_address, end_address);
	m_memory->get_program()->install_readwrite_handler( start_address, end_address, read16_delegate(*this, FUNC(gcm394_game_state::cs4_r)), write16_delegate(*this, FUNC(gcm394_game_state::cs4_w)));
	//start_address += size;
}


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
	m_maincpu->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_maincpu->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
	m_maincpu->set_bootmode(0); // boot from internal ROM (SPI bootstrap)
	m_maincpu->set_cs_config_callback(FUNC(gcm394_game_state::cs_callback));

	FULL_MEMORY(config, m_memory).set_map(&generalplus_gpspispi_game_state::cs_map_base);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, (320*2)-1, 0, (240*2)-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

DEVICE_IMAGE_LOAD_MEMBER(generalplus_gpspispi_bkrankp_game_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

void generalplus_gpspispi_bkrankp_game_state::generalplus_gpspispi_bkrankp(machine_config &config)
{
	generalplus_gpspispi_game_state::generalplus_gpspispi(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "bkrankp_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(generalplus_gpspispi_bkrankp_game_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("bkrankp_cart");
}

static INPUT_PORTS_START( gcm394 )
	PORT_START("IN0")
	PORT_START("IN1")
	PORT_START("IN2")
INPUT_PORTS_END

static INPUT_PORTS_START( smartfp )
	PORT_START("IN0")
	// entirely non-standard mat based controller (0-11 are where your feet are placed normally, row of selection places to step above those)
	// no sensible default mapping unless forced
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_Q) PORT_NAME("0")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_W) PORT_NAME("1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_E) PORT_NAME("2")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_R) PORT_NAME("3")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_CODE(KEYCODE_T) PORT_NAME("4")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_Y) PORT_NAME("5")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_U) PORT_NAME("6")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_I) PORT_NAME("7")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_O) PORT_NAME("8")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_P) PORT_NAME("9")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("10")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("11")

	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_A) PORT_NAME("Circle / Red")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_S) PORT_NAME("Square / Orange")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_D) PORT_NAME("Triangle / Yellow")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_F) PORT_NAME("Star / Blue")

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("HOME")
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gormiti ) // DOWN with A+B+C for test mode?
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 ) // causes long delay in test mode?
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wrlshunt )
	PORT_START("IN0")
	PORT_START("IN1")
	PORT_START("IN2")
INPUT_PORTS_END

static INPUT_PORTS_START( tkmag220 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Important" ) // gets stuck in inf loop if this is wrong
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_car2 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) // unused
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_gtg )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( jak_s500 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( paccon ) // for Test Mode hold buttons 1+2 until the screen starts changing colours (happens after the copyright display)
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED ) // PAL/NTSC flag
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) // 'A'
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) // '*C*  (doesn't exist?) (cheat)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) // 'B'
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) // '*MENU*' (doesn't exist?)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



ROM_START( smartfp )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("smartfitparkuk.bin", 0x000000, 0x800000, CRC(2072d7d0) SHA1(eaa4f254d6dee3a7eac64ae2204dd6291e4d27cc) )
ROM_END

ROM_START( smartfps )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("smartfitpark.bin", 0x000000, 0x800000, CRC(ada84507) SHA1(a3a80bf71fae62ebcbf939166a51d29c24504428) )
ROM_END

ROM_START( gormiti )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("gormiti.bin", 0x000000, 0x800000, CRC(71b82d41) SHA1(169b35dc7bdd05b7b32176ddf901ace27736cb86) )
ROM_END



ROM_START( paccon )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("pacmanconnect.bin", 0x000000, 0x400000, CRC(8567cdc7) SHA1(cef4e003142e479169e4438ab33558436ee9ee68) )
ROM_END


ROM_START( tkmag220 )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "u1g-2a.u2", 0x0000000, 0x8000000, CRC(0fd769a1) SHA1(df19402bcd20075483d63fb98fb3fa42bd33ccfd) )
ROM_END

ROM_START(lazertag)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("lazertag.bin", 0x000000, 0x1000000, CRC(8bf16a28) SHA1(90d05e1876332324b074e4845e28b90fcb007122) )
ROM_END

ROM_START(jak_s500)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("spbwheel.bin", 0x000000, 0x800000, CRC(6ba1d335) SHA1(1bb3e4d02c7b35dd4d336971c6a9f82071cc6ce1) )
ROM_END

ROM_START(jak_smwm)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("spidersense.bin", 0x000000, 0x800000, CRC(e0676d0e) SHA1(01c01852fe4aea799c09ebbb6870b2f6e92085c4) )
ROM_END


ROM_START(jak_pf)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("phineas.bin", 0x000000, 0x800000, CRC(bb18f70d) SHA1(4e3c204e44efe9186809404521ebeac348c45fac))
ROM_END

ROM_START(jak_prft)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("powerrangerssword.bin", 0x000000, 0x800000, CRC(77bc8aea) SHA1(a2efaa718d8ecece46cebb9f0f13a8fa10fc2826) )
ROM_END

ROM_START(jak_tink)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("disneyfairies.bin", 0x000000, 0x800000, CRC(566dae87) SHA1(3abe1b7d578ed9255101bfec0e4bb4d6dc0aa0b7) )
ROM_END





ROM_START(jak_totm)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("toysonthemove.bin", 0x000000, 0x800000, CRC(d08fb72a) SHA1(1fea98542ef7c65eef31afb70fd50952b4cef1c1) )
ROM_END

ROM_START(jak_ths)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("tripleheadersports.bin", 0x000000, 0x800000, CRC(2b5f8734) SHA1(57bccaa70f0efbf3da3259b74f3082d1a14c9908) )
ROM_END


ROM_START(wrlshunt)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x8000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("wireless.bin", 0x0000, 0x8000000, CRC(a6ecc20e) SHA1(3645f23ba2bb218e92d4560a8ae29dddbaabf796))
ROM_END

/*
Wireless Hunting Video Game System
(info provided with dump)

System: Wireless Hunting Video Game System
Publisher: Hamy / Kids Station Toys Inc
Year: 2011
ROM: FDI MSP55LV100G
RAM: Micron Technology 48LC8M16A2

Games:

Secret Mission
Predator
Delta Force
Toy Land
Dream Forest
Trophy Season
Freedom Force
Be Careful
Net Power
Open Training
Super Archer
Ultimate Frisbee
UFO Shooting
Happy Darts
Balloon Shoot
Avatair
Angry Pirate
Penguin War
Ghost Shooter
Duck Hunt


ROM Board:

Package: SO44
Spacing: 1.27 mm
Width: 16.14 mm
Length: 27.78 mm
Voltage: 3V
Pinout:

          A25  A24
            |  |
      +--------------------------+
A21 --|==   #  # `.__.'        ==|-- A20
A18 --|==                      ==|-- A19
A17 --|==                      ==|-- A8
 A7 --|==                      ==|-- A9
 A6 --|==                  o   ==|-- A10
 A5 --|==  +----------------+  ==|-- A11
 A4 --|==  |                |  ==|-- A12
 A3 --|==  |  MSP55LV100G   |  ==|-- A13
 A2 --|==  |  0834 M02H     |  ==|-- A14
 A1 --|==  |  JAPAN         |  ==|-- A15
 A0 --|==  |                |  ==|-- A16
#CE --|==  |                |  ==|-- A23
GND --|==  |                |  ==|-- A22
#OE --|==  |                |  ==|-- Q15
 Q0 --|==  |                |  ==|-- Q7
 Q8 --|==  |                |  ==|-- Q14
 Q1 --|==  +----------------+  ==|-- Q6
 Q9 --|==                      ==|-- Q13
 Q2 --|==       M55L100G       ==|-- Q5
Q10 --|==                      ==|-- Q12
 Q3 --|==                      ==|-- Q4
Q11 --|==                      ==|-- VCC
      +--------------------------+


The only interesting string in this ROM is SPF2ALP,
which is also found in the Wireless Air 60 ROM.

*/


/*
Wireless Air 60
(info provided with dump)

System: Wireless Air 60
ROM: Toshiba TC58NVG0S3ETA00
RAM: ESMT M12L128168A

This is a raw NAND flash dump

Interesting Strings:

GPnandnand; (GP is General Plus, which is Sunplus by another name)
GLB_GP-F_5B_USBD_1.0.0
SP_ToneMaker
GLB_GP-FS1_0405L_SPU_1.0.2.3
SPF2ALP

"GPnandnand" as a required signature appears to be referenced right here, in page 19 of a GeneralPlus document;
https://web.archive.org/web/20180106005235/http://www.lcis.com.tw/paper_store/paper_store/GPL162004A-507A_162005A-707AV10_code_reference-20147131205102.pdf

*/

ROM_START( wlsair60 )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x8400000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "wlsair60.nand", 0x0000, 0x8400000, CRC(eec23b97) SHA1(1bb88290cf54579a5bb51c08a02d793cd4d79f7a) )
ROM_END

ROM_START( jak_gtg )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "goldentee.bin", 0x0000, 0x4200000, CRC(87d5e815) SHA1(5dc46cd753b791449cc41d5eff4928c0dcaf35c0) )
ROM_END

ROM_START( jak_car2 )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "cars2.bin", 0x0000, 0x4200000, CRC(4d610e09) SHA1(bc59f5f7f676a8f2a78dfda7fb62c804bbf850b6) )
ROM_END

ROM_START( jak_tsm )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "toystorymania.bin", 0x0000, 0x4200000, CRC(183b20a5) SHA1(eb4fa5ee9dfac58f5244d00d4e833b1e461cc52c) )
ROM_END


ROM_START( jak_duck )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "duckcommander_gpr27p512a_c276_as_hy27us08121a.bin", 0x0000, 0x4200000, CRC(d9356d5b) SHA1(aca05525b4a504f7ad264ae9bbc2f1f8f399c4ca) )
ROM_END

ROM_START( jak_swc )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "jakksstarwarspistol_gpr27p512a_c276_as_hy27us08121a.bin", 0x0000, 0x4200000, CRC(024d49b8) SHA1(9694f4c7cd083c976ffbbcfa6f626fc6b4bc8d91) )
ROM_END

ROM_START( jak_wdzh )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "walkingdeadrifle_gpr27p512a_c276_as_hy27us08121a.bin", 0x0000, 0x4200000, CRC(b2c762f0) SHA1(7e10df517cc24924e0ec55e2a263563023d945f8) )
ROM_END

ROM_START( jak_wdbg )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "amcwalkingdeadcrossbow_gpr27p512a_c276_as_hy27us08121a.bin", 0x0000, 0x4200000, CRC(66510fd4) SHA1(3ad6347c5a7758c035654cb3e96858320875b97a) )
ROM_END


ROM_START( vbaby )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x8400000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "vbaby.bin", 0x0000, 0x8400000, CRC(d904441b) SHA1(3742bc4e1e403f061ce2813ecfafc6f30a44d287) )
ROM_END

ROM_START( mgtfit )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x8400000, "nandrom", ROMREGION_ERASE00 ) // Samsung 937 K9F1G08U0D  Ident: 0xEC 0xF1 Full Ident: 0xECF1001540
	ROM_LOAD( "k9f1g08u0d.bin", 0x0000, 0x8400000, CRC(1ca5ac09) SHA1(c2e123085d2198999c2c0edb1df4895361c00a99) )
ROM_END

ROM_START( beambox )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "beambox.bin", 0x0000, 0x4200000, CRC(a486f04e) SHA1(73c7d99d8922eba58d94e955e254b9c3baa4443e) )
ROM_END

// the JAKKS ones of these seem to be known as 'Generalplus GPAC500' hardware?
CONS(2009, smartfp,   0,       0, base, smartfp,  gcm394_game_state, empty_init, "Fisher-Price", "Fun 2 Learn Smart Fit Park (UK)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2009, smartfps,  smartfp, 0, base, smartfp,  gcm394_game_state, empty_init, "Fisher-Price", "Fun 2 Learn Smart Fit Park (Spain)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(200?, tkmag220,  0,       0, tkmag220, tkmag220, tkmag220_game_state,  empty_init,      "TaiKee",         "Mini Arcade Games Console (Family Sport 220-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// die on this one is 'GCM420'
CONS(2013, gormiti,   0, 0, base, gormiti,  gcm394_game_state, empty_init, "Giochi Preziosi", "Gormiti Game Arena (Spain)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)


// Fun 2 Learn 3-in-1 SMART SPORTS  ?

// also sold as "Pac-Man Connect & Play 35th Anniversary" (same ROM?)
CONS(2012, paccon, 0, 0, paccon, paccon, jak_s500_game_state, init_wrlshunt, "Bandai", "Pac-Man Connect & Play (Feb 14 2012 10:46:23)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2008, lazertag, 0, 0, wrlshunt, jak_s500, jak_s500_game_state, init_wrlshunt, "Tiger Electronics", "Lazer Tag Video Game Module", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2009, jak_s500, 0, 0, wrlshunt, jak_s500, jak_s500_game_state, init_wrlshunt, "JAKKS Pacific Inc", "SpongeBob SquarePants Bikini Bottom 500 (JAKKS Pacific TV Motion Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2009, jak_smwm, 0, 0, wrlshunt, jak_s500, jak_s500_game_state, init_wrlshunt, "JAKKS Pacific Inc", "Spider-Man Web Master (JAKKS Pacific TV Motion Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2010, jak_pf,   0, 0, wrlshunt, jak_s500, jak_s500_game_state, init_wrlshunt, "JAKKS Pacific Inc", "Phineas and Ferb: Best Game Ever! (JAKKS Pacific TV Motion Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // build date is 2009, but onscreen display is 2010
CONS(2009, jak_prft, 0, 0, wrlshunt, jak_s500, jak_s500_game_state, init_wrlshunt, "JAKKS Pacific Inc", "Power Rangers Force In Time (JAKKS Pacific TV Motion Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2009, jak_tink, 0, 0, wrlshunt, jak_s500, jak_s500_game_state, init_wrlshunt, "JAKKS Pacific Inc", "Tinker Bell and the Lost Treasure (JAKKS Pacific TV Motion Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(200?, jak_totm, 0, 0, wrlshunt, jak_s500, jak_s500_game_state, init_wrlshunt, "JAKKS Pacific Inc", "Toy Story - Toys on the Move (JAKKS Pacific TV Motion Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // Toys on the Move has ISSI 404A
CONS(2009, jak_ths,  0, 0, wrlshunt, jak_s500, jak_s500_game_state, init_ths,      "JAKKS Pacific Inc", "Triple Header Sports (JAKKS Pacific TV Motion Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2011, wrlshunt, 0, 0, wrlshunt, wrlshunt, wrlshunt_game_state, init_wrlshunt, "Hamy / Kids Station Toys Inc", "Wireless Hunting Video Game System", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

void generalplus_gpac800_game_state::machine_start()
{
	save_item(NAME(m_sdram));
}

void generalplus_gpac800_game_state::nand_create_stripped_region()
{
	uint8_t* rom = m_nandregion;
	int size = memregion("nandrom")->bytes();
	m_size = size;

	int numblocks = size / m_nandblocksize;
	m_strippedsize = numblocks * m_nandblocksize_stripped;
	m_strippedrom.resize(m_strippedsize);

	for (int i = 0; i < numblocks; i++)
	{
		const int base = i * m_nandblocksize;
		const int basestripped = i * m_nandblocksize_stripped;

		for (int j = 0; j < m_nandblocksize_stripped; j++)
		{
			m_strippedrom[basestripped + j] = rom[(base + j)];
		}
	}

	// debug to allow for easy use of unidasm.exe
	if (0)
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"stripped_%s", machine().system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(&m_strippedrom[0], m_nandblocksize_stripped * numblocks, 1, fp);
			fclose(fp);
		}
	}
}

void generalplus_gpac800_game_state::machine_reset()
{
	// configure CS defaults
	address_space& mem = m_maincpu->space(AS_PROGRAM);
	mem.write_word(0x007820, 0x0047);
	mem.write_word(0x007821, 0xff47);
	mem.write_word(0x007822, 0x00c7);
	mem.write_word(0x007823, 0x0047);
	mem.write_word(0x007824, 0x0047);

	m_maincpu->set_cs_space(m_memory->get_program());

	if (m_nandregion)
	{
		nand_create_stripped_region();

		// up to 256 pages (16384kw) for each space

		// (size of cs0 + cs1 + cs2 + cs3 + cs4) <= 81920kwords

		// simulate bootstrap / internal ROM

		address_space& mem = m_maincpu->space(AS_PROGRAM);

		/* Offset(h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
		   00000000 (50 47 61 6E 64 6E 61 6E 64 6E)-- -- -- -- -- --  PGandnandn------
		   00000010  -- -- -- -- -- bb -- -- -- -- -- -- -- -- -- --  ----------------

		   bb = where to copy first block

		   The header is GPnandnand (byteswapped) then some params
		   one of the params appears to be for the initial code copy operation done
		   by the bootstrap
		*/

		// probably more bytes are used
		int dest = m_strippedrom[0x15] << 8;

		// copy a block of code from the NAND to RAM
		for (int i = 0; i < m_initial_copy_words; i++)
		{
			uint16_t word = m_strippedrom[(i * 2) + 0] | (m_strippedrom[(i * 2) + 1] << 8);

			mem.write_word(dest + i, word);
		}

		// these vectors must either directly point to RAM, or at least redirect there after some code
		uint16_t* internal = (uint16_t*)memregion("maincpu:internal")->base();
		internal[0x7ff5] = m_vectorbase + 0x0a;
		internal[0x7ff6] = m_vectorbase + 0x0c;
		internal[0x7ff7] = dest + 0x20; // point boot vector at code in RAM (probably in reality points to internal code that copies the first block)
		internal[0x7ff8] = m_vectorbase + 0x10;
		internal[0x7ff9] = m_vectorbase + 0x12;
		internal[0x7ffa] = m_vectorbase + 0x14;
		internal[0x7ffb] = m_vectorbase + 0x16;
		internal[0x7ffc] = m_vectorbase + 0x18;
		internal[0x7ffd] = m_vectorbase + 0x1a;
		internal[0x7ffe] = m_vectorbase + 0x1c;
		internal[0x7fff] = m_vectorbase + 0x1e;
	}

	m_maincpu->reset(); // reset CPU so vector gets read etc.

	m_maincpu->set_paldisplaybank_high_hack(0);
	m_maincpu->set_alt_tile_addressing_hack(1);
}


void generalplus_gpac800_game_state::nand_init210()
{
	m_sdram.resize(m_sdram_kwords);
	m_sdram2.resize(0x10000);

	m_nandblocksize = 0x210;
	m_nandblocksize_stripped = 0x200;

	m_vectorbase = 0x6fe0;
}

void generalplus_gpac800_game_state::nand_init210_32mb()
{
	m_sdram_kwords = 0x400000 * 4;
	nand_init210();
}

void generalplus_gpac800_game_state::nand_init840()
{
	m_sdram.resize(m_sdram_kwords);
	m_sdram2.resize(0x10000);

	m_nandblocksize = 0x840;
	m_nandblocksize_stripped = 0x800;

	m_vectorbase = 0x6fe0;
}

void generalplus_gpac800_game_state::nand_wlsair60()
{
	nand_init840();
	m_initial_copy_words = 0x2800;
}

void generalplus_gpac800_game_state::nand_vbaby()
{
	nand_init840();
	m_initial_copy_words = 0x1000;
	m_maincpu->set_romtype(2);
}

void generalplus_gpac800_game_state::nand_tsm()
{

	// something odd must be going on with the bootloader?
	// structure has the first 0x4000 block repeated 3 times (must appear in RAM on startup?)
	// then it has a 0x10000 block repeated 4 times (must get copied to 0x30000 by code)
	// then it has the larger, main payload, just the once.

	// the addresses written to the NAND device don't compensate for these data repeats, however dump seems ok as no other data is being repeated?
	// reads after startup still need checking
	nand_init210();
	m_maincpu->set_romtype(1);
}

void generalplus_gpac800_game_state::nand_beambox()
{
	nand_init210();
	m_vectorbase = 0x2fe0;
}



// NAND dumps w/ internal bootstrap (and u'nSP 2.0 extended opcodes)  (have gpnandnand strings)
// the JAKKS ones seem to be known as 'Generalplus GPAC800' hardware
CONS(2010, wlsair60, 0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_wlsair60, "Jungle Soft / Kids Station Toys Inc", "Wireless Air 60",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, jak_gtg,  0, 0, generalplus_gpac800,       jak_gtg,  generalplus_gpac800_game_state,       nand_init210,  "JAKKS Pacific Inc",                   "Golden Tee Golf (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, jak_car2, 0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_init210,  "JAKKS Pacific Inc",                   "Cars 2 (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, jak_tsm , 0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_tsm,      "JAKKS Pacific Inc",                   "Toy Story Mania (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, beambox,  0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_beambox,  "Hasbro",                              "Playskool Heroes Transformers Rescue Bots Beam Box (Spain)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, mgtfit,   0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_wlsair60, "MGT",                                 "Fitness Konsole (NC1470)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // probably has other names in English too? menus don't appear to be in German
CONS(200?, vbaby,    0, 0, generalplus_gpac800_vbaby, jak_car2, generalplus_gpac800_vbaby_game_state, nand_vbaby,    "VTech",                               "V.Baby", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// There were 1 player and 2 player versions for several of the JAKKS guns.  The 2nd gun appears to be simply a controller (no AV connectors) but as they were separate products with the 2 player verisons being released up to a year after the original, the code could differ.
// If they differ, it is currently uncertain which versions these ROMs are from
CONS(2012, jak_wdzh, 0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_init210,       "JAKKS Pacific Inc",                   "The Walking Dead: Zombie Hunter (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // gun games all had Atmel 16CM (24C16).
CONS(2013, jak_swc,  0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc",                   "Star Wars Clone Trooper (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(2013, jak_duck, 0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc",                   "Duck Commander (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // no 2 Player version was released
CONS(2014, jak_wdbg, 0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc",                   "The Walking Dead: Battleground (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)


ROM_START( bkrankp )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION(0x400000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "unit_mx25l3206e_c22016.bin", 0x0000, 0x400000, CRC(7efad116) SHA1(427d707e97586ae6ab5fe08f29ca450ddc7ad36e) )
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


CONS(200?, bkrankp, 0, 0, generalplus_gpspispi_bkrankp, gcm394, generalplus_gpspispi_bkrankp_game_state , init_spi, "Bandai", "Karaoke Ranking Party (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)


