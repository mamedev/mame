// license:BSD-3-Clause
// copyright-holders:Hpman
#include "emu.h"
#include "rom.h"
#include "paprium.h"
#include "md_slot.h"
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MD_ROM_PAPRIUM, md_rom_paprium_device, "md_rom_paprium", "MD Paprium")

/*-------------------------------------------------
 Paprium
 Cortex-M4 CPU code isn't dumped, we have to make
 up for the missing features with best guesses of
 internal workings.

 Current issues:
 - Still WIP/incomplete, no sound, reset not working, hacks, etc...
 - some objects will display with wrong palette attribute, extra data
seems to be in animation data, but wasn't figured yet.
 - Implementation is a bit too much pointer happy
 for save state atm.
 -------------------------------------------------*/

md_rom_paprium_device::md_rom_paprium_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : md_rom_sram_device(mconfig, MD_ROM_PAPRIUM, tag, owner, clock)
{
}

void md_rom_paprium_device::device_start()
{
	logerror("sizeof(ppm_interface_struct): 0x%x\n", (unsigned int)sizeof(ppm_interface_struct));
	ppm_interface = (ppm_interface_struct *)ppm_dual_port_ram;

	save_item(NAME(ppm_dual_port_ram));
	save_item(NAME(ppm_sdram));

	//	save_item(NAME(ppm_interface));

	save_item(NAME(ppm_scale_stamp));

	//	save_item(NAME(ppm_sdram_pointer));
	save_item(NAME(ppm_sdram_window_enabled));

	//	save_item(NAME(ppm_object_handles));
	save_pointer((uint16_t *)&ppm_object_handles[0], "ppm_object_handles", 0);
	save_item(NAME(ppm_drawList));
	//	save_item(NAME(ppm_drawListPtr));

	//	save_item(NAME(ppm_vram_slots));
	save_pointer((uint16_t *)&ppm_vram_slots[0], "ppm_vram_slots", 0);
	save_item(NAME(ppm_vram_max_slot));
	save_item(NAME(ppm_block_unpack_addr));

	save_item(NAME(ppm_anim_data_base_addr));
	//	save_item(NAME(ppm_anim_data));
	save_item(NAME(ppm_anim_max_index));

	save_item(NAME(ppm_unk_data_addr));
	save_item(NAME(ppm_unk_data2_addr));

	//	save_item(NAME(ppm_bgm_tracks_offsets));
	save_item(NAME(ppm_bgm_tracks_base_addr));
	save_item(NAME(ppm_bgm_unpack_addr));

	//	save_item(NAME(ppm_sfx_data));
	save_item(NAME(ppm_sfx_base_addr));

	//	save_item(NAME(ppm_gfx_blocks_offsets));
	save_item(NAME(ppm_gfx_blocks_base_addr));
	save_item(NAME(ppm_max_gfx_block));

	save_item(NAME(ppm_audio_bgm_volume));
	save_item(NAME(ppm_audio_sfx_volume));
	save_item(NAME(ppm_audio_config));
}

void md_rom_paprium_device::device_reset()
{
	// currently loading MAX10 data over rom area then copies it to ram block,
	// this should be changed later on to add proper dataarea for this to load into

	// did we decode yet?
	if (m_rom[0x8000 / 2])
	{
		uint16_t key1 = m_rom[0x8000 / 2];  // 0x0000 reset stack ptr
		uint16_t key2 = m_rom[0xbd000 / 2]; // blank area

		// top 8K overlayed with MAX10 dual port ram module
		// for (uint32_t addr = 0; addr < 0x2000 / 2; addr++)
		//	m_rom[addr] ^= (key2 | bitswap<16>(addr & 0xff, 15, 1, 14, 6, 13, 2, 12, 0, 11, 3, 10, 4, 9, 7, 8, 5));
		for (uint32_t addr = 0x2000 / 2; addr < 0x10000 / 2; addr++)
			m_rom[addr] ^= (key1 | bitswap<16>(addr & 0xff, 15, 1, 14, 6, 13, 2, 12, 0, 11, 3, 10, 4, 9, 7, 8, 5));
		for (uint32_t addr = 0x10000 / 2; addr < 0x800000 / 2; addr++)
			m_rom[addr] ^= (key2 | bitswap<16>(addr & 0xff, 15, 1, 14, 6, 13, 2, 12, 0, 11, 3, 10, 4, 9, 7, 8, 5));

		// copy data to dual port ram
		for (uint16_t x = 0; x < 0x2000 / 2; x++)
			ppm_dual_port_ram[x] = m_rom[x];

		// check ROM version & patch a few things
		// (hold B on startup to display version ingame)
		switch (m_rom[0x1000a / 2])
		{
		case 0x2e7f:
			// boot check
			ppm_dual_port_ram[0x1d1c / 2] = 0x0004;	 // 0xc00004
			ppm_dual_port_ram[0x1d2c / 2] |= 0x0100; // switch to beq.s (0x670e)

			// post splash jump location
			ppm_dual_port_ram[0x1562 / 2] = 0x0001;
			ppm_dual_port_ram[0x1564 / 2] = 0x0100;

			// emulator check
			m_rom[0x81104 / 2] = 0x4e71;
			break;
		default:
			printf("UNKNOWN PAPRIUM VERSION 0x%04X\n", m_rom[0x1000a / 2]);
			break;
		}
	}

	// clear/setup registers
	ppm_interface->reg_command = 0;
	ppm_interface->reg_status_1 = 0;
	ppm_interface->reg_status_2 = 7; // let's pretend MW is plugged & connected (last 3 bits)

	ppm_sdram_pointer = ppm_sdram;
	ppm_sdram_window_enabled = false;

	ppm_vram_max_slot = 0;
}

uint16_t md_rom_paprium_device::read(offs_t offset)
{
	if (offset < sizeof(ppm_dual_port_ram) / 2)
		return ppm_dual_port_ram[offset];

	if ((offset >= 0xc000 / 2) && (offset < 0x10000 / 2) && ppm_sdram_window_enabled)
		return !machine().side_effects_disabled() ? *ppm_sdram_pointer++ : *ppm_sdram_pointer;

	if (offset < 0x400000 / 2)
		return m_rom[offset];
	else
		return 0xffff;
}

void md_rom_paprium_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// can only write to dual port ram
	if (offset < sizeof(ppm_dual_port_ram) / 2)
	{
		ppm_dual_port_ram[offset] = (ppm_dual_port_ram[offset] & ~mem_mask) | (data & mem_mask);
		switch (offset)
		{
		case offsetof(ppm_interface_struct, reg_status_1) / 2:
			logerror("Status register 1 write? (0x%04x)\n", data);
			break;
		case offsetof(ppm_interface_struct, reg_status_2) / 2:
			logerror("Status register 2 write? (0x%04x)\n", data);
			break;
		case offsetof(ppm_interface_struct, reg_command) / 2:
			ppm_process_command();
			break;
		case offsetof(ppm_interface_struct, reg_watchdog) / 2:
			if (data != 0x9494)
				logerror("Watchdog? / unknown value: 0x%04x\n", data);
			break;
		case offsetof(ppm_interface_struct, reg_unk0) / 2:
		case offsetof(ppm_interface_struct, reg_unk1) / 2:
		case offsetof(ppm_interface_struct, reg_unk4) / 2:
		case offsetof(ppm_interface_struct, reg_unk7) / 2:
		case offsetof(ppm_interface_struct, reg_unk8) / 2:
		case offsetof(ppm_interface_struct, reg_unk9) / 2:
		case offsetof(ppm_interface_struct, reg_unkA) / 2:
		case offsetof(ppm_interface_struct, reg_unkB) / 2:
		case offsetof(ppm_interface_struct, reg_unkC) / 2:
		case offsetof(ppm_interface_struct, reg_unkD) / 2:
		case offsetof(ppm_interface_struct, reg_unkE) / 2:
		case offsetof(ppm_interface_struct, reg_unkF) / 2:
			logerror("Unknown register write? (0x%04x, 0x%04x)\n", offset * 2, data);
			break;
		}
	}
}

void md_rom_paprium_device::ppm_process_command()
{
	uint8_t command_id = ppm_interface->reg_command >> 8;
	uint8_t command_arg = ppm_interface->reg_command; // & 0xff

	switch (command_id)
	{
	// special challenge command used on startup.
	// command 0x0055 takes quite some time to execute
	case 0x00:
		switch (command_arg)
		{
		case 0xaa:
			ppm_interface->reg_command = 0x00ff;
			return;
		case 0x55:
			ppm_interface->reg_command = 0x0000; // explicit 0 return
			return;
		default:
			LOG("CMD 0x00, unknown challenge 0x%02x\n", command_arg);
			break;
		}
		break;
	// initial startup/reset? - 3 0w8100 writes on startup
	// finding arcade mode sends 0x810f
	case 0x81:
		LOG("CMD 0x%04x\n", ppm_interface->reg_command);
		ppm_sdram_window_enabled = true;
		//	removed, as command 0xb0 was added
		//	ppm_vram_set_budget(0);
		//	ppm_obj_reset(); // spr system reset
		break;
	// unk startup thing
	// 8300/8302 toggle *12 (24 total writes) on startup
	// couple toggles between stages
	case 0x83:
		LOG("CMD 0x%04x\n", ppm_interface->reg_command);
		break;
	// disable sdram window
	case 0x84:
		ppm_sdram_window_enabled = false;
		break;
	// set audio config
	case 0x88:
		ppm_audio_config = command_arg;
		if (command_arg & ~0x0b) // setting unknown bits ?
			logerror("audio config: unknown bits (0x%02x)\n", command_arg);
		break;
	// bgm play, check & 0x80
	case 0x8c:
		logerror("BGM play 0x%02x (code %02x)\n", command_arg & 0x7f, command_arg);
		if (command_arg & 0x80)
		{
			// ?
		}
		ppm_unpack(ppm_bgm_addr(command_arg & 0x7f), ppm_bgm_unpack_addr);
		break;

		//	case 0x8d:
		//	// sound related

		//	case 0x8e:
		//		// bgm stop/pause?
		//		break;

	// (bgm related)
	case 0x95:
		break;
	// (bgm related)
	case 0x96:
		break;

	// megawire settings
	case 0xa4:
		switch (command_arg)
		{
		default:
			logerror("Megawire unk. option (0x%02x)\n", command_arg);
		case 0x00: // reset conn.
		case 0x40: // ?delete credentials?
		case 0x80: // power profile off
		case 0x81: // power profile auto
		case 0x82: // power profile forced
			break;
		}
		break;
	// a858 MW reboot (update screen)
	// a900 MW interrupt/reset? sub_9B738

	// add object to draw list
	case 0xad:
		ppm_obj_add(command_arg);
		break;
	// frame begin
	case 0xae:
		ppm_obj_frame_start();
		break;
	// frame finish
	case 0xaf:
		// af01: no scaling ongoing
		// af02: must reserve bandwith for scaling buffer?
		ppm_obj_frame_end();
		break;
		// ?unk?
		//	case 0xb0:
		//		break;
		// sent by waitvbl under condition, instead of 0xaf
		// end frame without obj rendering ?
	case 0xb0:
		// reset SPR engine? (used at startup & test screen)
		ppm_obj_reset();
		break;
	case 0xb1:
		// seems used to keep previous SAT data in VRAM, do nothing?
		break;
	// post scale command
	case 0xb6:
		// likely instructs to restore boot code to allow back HW reset
		break;
	// load/setup base data
	case 0xc6:
		ppm_setup_data(
		    swapshorts(ppm_interface->command_args_long[0]),
		    swapshorts(ppm_interface->command_args_long[1]),
		    swapshorts(ppm_interface->command_args_long[2]),
		    swapshorts(ppm_interface->command_args_long[3]),
		    swapshorts(ppm_interface->command_args_long[4]),
		    swapshorts(ppm_interface->command_args_long[5]),
		    swapshorts(ppm_interface->command_args_long[6]));
		break;
	// set BGM volume
	case 0xc9:
		ppm_audio_bgm_volume = command_arg;
		break;
	// set SFX volume
	case 0xca:
		ppm_audio_sfx_volume = command_arg;
		break;

	// d0

	// SFX play, 4 additional args
	case 0xd1:
		LOG("SFX 0x%02x - 0x%04x / 0x%04x / 0x%04x / 0x%04x\n", command_arg, ppm_interface->command_args[0], ppm_interface->command_args[1], ppm_interface->command_args[2], ppm_interface->command_args[3]);
		break;
		// ?unk?
		//	case 0xd2:
		// 0xd240 0xd280
		//		break;

	// d6
	case 0xd6:
		LOG("CMD 0x%04x - 0x%04x\n", ppm_interface->reg_command, ppm_interface->command_args[0]);
		break;

	// unpack request
	case 0xda:
		logerror("Unpacking 0x%06x @0x%04x (0x%02x)\n", (ppm_interface->command_args[1] << 16) + ppm_interface->command_args[2], ppm_interface->command_args[0], command_arg);
		ppm_unpack((ppm_interface->command_args[1] << 16) + ppm_interface->command_args[2], ppm_interface->command_args[0]);
		ppm_sdram_pointer = &ppm_sdram[ppm_interface->command_args[0] / 2]; // optional ?
		ppm_interface->reg_status_1 &= ~0x0004;
		ppm_interface->reg_status_2 &= ~STATUS2_BUSY; // clear busy bit
		break;
	// setup sdram pointer for read
	case 0xdb:
		logerror("Setup sdram read @0x%06x, size 0x%04x (0x%02x)\n", swapshorts(ppm_interface->command_args_long[0]), ppm_interface->command_args[2], command_arg);
		ppm_sdram_pointer = &ppm_sdram[swapshorts(ppm_interface->command_args_long[0]) / 2];
		// read size = ppm_interface->command_args[2];
		break;
	// eeprom load
	case 0xdf:
		switch (command_arg)
		{
		case 1:
		case 2:
		case 3:
			memcpy(&ppm_dual_port_ram[ppm_interface->command_args[0] / 2], &m_nvram[(0x200 + command_arg * 0x200) / 2], 0x100);
			break;
		case 4:
			memcpy(&ppm_dual_port_ram[ppm_interface->command_args[0] / 2], &m_nvram[0], 0x200);
			break;
		}
		break;
	// eeprom save (command_args[0]=0xbeef)
	case 0xe0:
		switch (command_arg)
		{
		case 1:
		case 2:
		case 3:
			memcpy(&m_nvram[(0x200 + command_arg * 0x200) / 2], &ppm_dual_port_ram[ppm_interface->command_args[1] / 2], 0x100);
			break;
		case 4:
			memcpy(&m_nvram[0], &ppm_dual_port_ram[ppm_interface->command_args[1] / 2], 0x200);
			break;
		}
		ppm_interface->reg_status_2 &= ~STATUS2_EEPROM_ERROR1;
		ppm_interface->reg_status_2 &= ~STATUS2_EEPROM_ERROR2;
		break;

	case 0xe7: // send some data over network
		LOG("CMD 0x%04x (0x%04x / 0x%04x):", ppm_interface->reg_command, ppm_interface->command_args[0], ppm_interface->command_args[1]);
		for (int i = 0; i < (ppm_interface->command_args[1] + 1) / 2; i++)
			LOG(" 0x%04x", ppm_interface->command_args[2 + i]);
		LOG("\n");
		ppm_interface->reg_status_2 |= STATUS2_MW_DATA_IN;			     // pretend data is in
		ppm_interface->network_data[0x10 / 2] = ppm_interface->command_args[0] + 16; // pretend 16 bytes in?
		// 0x24: ranking fetch
		// 0x82: list access points
		// 0x84: set account/password (16 bytes each, null terminated if less than 16)
		break;

	// set vram block budget
	case 0xec:
		ppm_vram_set_budget(ppm_interface->command_args[1]);
		if (ppm_interface->command_args[0])
			logerror("PPM command 0xec: unk arg 0 (0x%04x)", ppm_interface->command_args[0]);
		break;
	// load single block (used in obj viewer)
	case 0xf2:
		// todo: check block range
		ppm_unpack(ppm_block_addr(ppm_interface->command_args[0]), 0x9000);
		ppm_unpack(ppm_block_addr(ppm_interface->command_args[0]), 0x9200);
		ppm_sdram_pointer = &ppm_sdram[0x9000 / 2];
		break;
	// load a stamp for scaling
	case 0xf4:
		ppm_unpack(swapshorts(ppm_interface->command_args_long[0]), 0, true);
		break;
	// scale current stamp to desired size
	case 0xf5:
		ppm_stamp_rescale(ppm_interface->command_args[0], ppm_interface->command_args[1], ppm_interface->command_args[2], ppm_interface->command_args[3]);
		break;
	default:
		LOG("Unprocessed command 0x%04x\n", ppm_interface->reg_command);
		break;
	}
	// ack command
	// cart returns 0 tho only MSB is checked by the game
	ppm_interface->reg_command = 0;
}

uint32_t md_rom_paprium_device::ppm_sdram_read_u32(uint32_t addr)
{
	// fetching non aligned U32
	return ((ppm_sdram[addr / 2 + 1] << 16) + ppm_sdram[addr / 2]);
}

uint32_t md_rom_paprium_device::ppm_unpack(uint32_t source_addr, uint32_t dest_addr, bool is_scaling_stamp)
{
	// packed data is byte width, ^1 on all addresses to fix endian mumbo jumbo
	uint8_t code, count, data_byte;
	uint32_t copy_addr;
	uint32_t initial_dest_addr = dest_addr;
	uint8_t *packed_data = (uint8_t *)m_rom;
	uint8_t *unpacked_data = is_scaling_stamp ? (uint8_t *)ppm_scale_stamp : (uint8_t *)ppm_sdram;

	switch (packed_data[source_addr++ ^ 1])
	{
	case 0x80:
		while ((code = packed_data[source_addr++ ^ 1]))
		{
			switch (count = code & 0x3f, code >> 6)
			{
			case 0:
				while (count--)
					unpacked_data[dest_addr++ ^ 1] = packed_data[source_addr++ ^ 1];
				break;
			case 1:
				data_byte = packed_data[source_addr++ ^ 1];
				while (count--)
					unpacked_data[dest_addr++ ^ 1] = data_byte;
				break;
			case 2:
				copy_addr = dest_addr - packed_data[source_addr++ ^ 1];
				while (count--)
					unpacked_data[dest_addr++ ^ 1] = unpacked_data[copy_addr++ ^ 1];
				break;
			case 3:
				while (count--)
					unpacked_data[dest_addr++ ^ 1] = 0;
				break;
			}
		}
		break;
	case 0x81:
		uint16_t copy_size, literal_size;

		while ((code = packed_data[source_addr++ ^ 1]) != 0x11) // unconfirmed end code
		{
			switch (code >> 4)
			{
			case 0:
				copy_size = 0;
				literal_size = code ? (3 + (code & 0x1f)) : (0x12 + packed_data[source_addr++ ^ 1]);
				break;
			case 1:
				if ((copy_size = 2 + (code & 0x7)) == 2)
					copy_size = 9 + packed_data[source_addr++ ^ 1];
				literal_size = packed_data[source_addr ^ 1] & 0x3;
				copy_addr = dest_addr - 0x4000 - (((packed_data[(source_addr + 1) ^ 1] << 8) + packed_data[source_addr ^ 1]) >> 2);
				source_addr += 2;
				break;
			case 2:
			case 3:
				if ((copy_size = (code & 0x1f)))
					copy_size += 2;
				else
				{
					copy_size = 0x21;
					while (!packed_data[source_addr++ ^ 1])
						copy_size += 0xff;
					copy_size += packed_data[(source_addr - 1) ^ 1];
				}
				literal_size = packed_data[source_addr ^ 1] & 0x3;
				copy_addr = dest_addr - 1 - (((packed_data[(source_addr + 1) ^ 1] << 8) + packed_data[source_addr ^ 1]) >> 2);
				source_addr += 2;
				break;
			default:
				copy_size = (code >> 5) + 1;
				literal_size = code & 0x3;
				copy_addr = dest_addr - 1 - (((code >> 2) & 0x7) + (packed_data[source_addr ^ 1] << 3));
				source_addr++;
				break;
			}
			while (copy_size--)
				unpacked_data[dest_addr++ ^ 1] = unpacked_data[copy_addr++ ^ 1];
			while (literal_size--)
				unpacked_data[dest_addr++ ^ 1] = packed_data[source_addr++ ^ 1];
		}
		break;
	default:
		logerror("unknown packer format, wrong address? (0x%06x)\n", source_addr - 1);
		break;
	}
	return dest_addr - initial_dest_addr;
}

void md_rom_paprium_device::ppm_setup_data(uint32_t bgm_file, uint32_t unk1_file, uint32_t smp_file, uint32_t unk2_file, uint32_t sfx_file, uint32_t anm_file, uint32_t blk_file)
{
	uint32_t unpack_addr = 0x10000;	// lower area reserved for ingame unpacking

	logerror("=== data setup ===\n");
	// setup misc data (unpack some of thoses to sdram):
	// bgm data
	logerror("BGM data: 0x%06x\n", bgm_file);
	ppm_bgm_tracks_base_addr = bgm_file;
	ppm_bgm_tracks_offsets = (uint32_t *)&m_rom[bgm_file / 2];
	// ?? data
	logerror("uk1 data: 0x%06x > 0x%06x\n", unk1_file, unpack_addr);
	ppm_unk_data_addr = unpack_addr;
	unpack_addr += ppm_unpack(unk1_file, unpack_addr);
	++unpack_addr &= 0xfffffffeu; // word align
	// samples data (WavPack file)
	logerror("SMP data: 0x%06x >  0x%06x\n", smp_file, unpack_addr);
	// unpacked data is about 1.4MB
	// ?? data
	logerror("uk2 data: 0x%06x > 0x%06x\n", unk2_file, unpack_addr);
	ppm_unk_data2_addr = unpack_addr;
	unpack_addr += ppm_unpack(unk2_file, unpack_addr);
	++unpack_addr &= 0xfffffffeu; // word align
	// sfx data
	logerror("SFX data: 0x%06x\n", sfx_file);
	ppm_sfx_base_addr = sfx_file;
	ppm_sfx_data = (ppm_sfx_struct *)&m_rom[sfx_file / 2];
	ppm_sfx_struct *fx = &ppm_sfx_data[1];
	for (int x = 1; x <= ppm_sfx_data[0].length; fx++, x++)
		logerror("\tSFX 0x%02x: 0x%06x / 0x%04x / 0x%04x\n", x, swapshorts(fx->offset), fx->attributes, fx->length);
	// animation data
	logerror("ANM data: 0x%06x > 0x%06x\n", anm_file, unpack_addr);
	ppm_anim_data_base_addr = unpack_addr;
	unpack_addr += ppm_unpack(anm_file, unpack_addr);
	++unpack_addr &= 0xfffffffeu; // word align
	ppm_anim_data = (uint32_t *)&ppm_sdram[ppm_anim_data_base_addr / 2];
	logerror("anim data for 0x%x objs\n", ppm_anim_data[0]);
	for (uint16_t x = 1; x <= ppm_anim_data[0]; x++)
	{
		uint32_t anim_offset = ppm_anim_data[x];
		uint16_t anim_count = 0;
		while (ppm_anim_data[anim_count + (anim_offset / 4)] != 0xffffffffu)
			anim_count++;
		ppm_anim_max_index[x - 1] = anim_count - 1;
		logerror("\tobj 0x%02x (0x%06x): %d anims\n", x, anim_offset, anim_count);
		if (anim_offset & 0x3)
			printf("(unaligned pointers)\n");
	}
	// blocks data
	logerror("BLK data: 0x%06x\n", blk_file);
	ppm_gfx_blocks_base_addr = blk_file;
	ppm_gfx_blocks_offsets = (uint32_t *)&m_rom[blk_file / 2];
	ppm_max_gfx_block = swapshorts(ppm_gfx_blocks_offsets[0]) - 1;
	logerror("blocks addr: %06x, count: %d\n", ppm_gfx_blocks_base_addr, swapshorts(ppm_gfx_blocks_offsets[0]));

	logerror("unpack addr finish: 0x%06x / free: 0x%06x\n", unpack_addr, 0x200000 - unpack_addr);
	ppm_bgm_unpack_addr = unpack_addr;
}

void md_rom_paprium_device::ppm_vram_set_budget(uint16_t blocks)
{
	// temp failsafe
	if (blocks > 0x35)
	{
		logerror("Allocation error (0x%04x blocks)\n", blocks);
		blocks = 0x35;
	}
	ppm_vram_reset_blocks((ppm_vram_max_slot = blocks));
}

void md_rom_paprium_device::ppm_vram_reset_blocks(uint16_t last_block)
{
	ppm_vram_slot_struct *slot = &ppm_vram_slots[last_block];
	for (uint16_t i = last_block; i < PPM_MAX_VRAM_SLOTS; slot++, i++)
	{
		slot->block_num = 0;
		slot->usage = 0;
		slot->age = 0;
	}
}

uint16_t md_rom_paprium_device::ppm_vram_find_block(uint16_t num)
{ // return tile index
	ppm_vram_slot_struct *slot = ppm_vram_slots;
	for (uint16_t x = 0; x < ppm_vram_max_slot; slot++, x++)
		if (slot->block_num == num)
			return ((x + (x <= 0x30 ? 1 : 0x4b)) << 4);
	return 0;
}

uint16_t md_rom_paprium_device::ppm_vram_load_block(uint16_t num)
{
	if (!num)
		return 0;

	// is block already in VRAM?
	ppm_vram_slot_struct *slot = ppm_vram_slots;
	for (uint16_t x = 0; x < ppm_vram_max_slot; slot++, x++)
		if (slot->block_num == num)
		{
			slot->usage++;
			slot->age = 0;
			return ((x + (x <= 0x30 ? 1 : 0x4b)) << 4);
		}

	// enough DMA budget?
	if (ppm_interface->dma_remaining < 0x110)
		return 0;

	// find oldest slot to load into
	uint32_t max_age = 0;
	uint16_t block_index = 0xffff;
	slot = ppm_vram_slots;

	for (uint16_t x = 0; x < ppm_vram_max_slot; slot++, x++)
		if ((!slot->usage) && (slot->age > max_age))
		{
			max_age = slot->age;
			block_index = x;
		}

	// no slot available?
	if (block_index == 0xffff)
		return 0;

	// found slot & have budget, unpack and DMA the block
	slot = &ppm_vram_slots[block_index];
	slot->block_num = num;
	slot->usage++;
	slot->age = 0;

	ppm_unpack(ppm_block_addr(num), ppm_block_unpack_addr);
	ppm_block_unpack_addr += 0x200;

	ppm_dma_command_struct *dma_entry = &ppm_interface->dma_commands[ppm_interface->dma_commands_count++];
	ppm_interface->dma_remaining -= 0x110;
	dma_entry->autoinc = 0x8f02;
	dma_entry->lenH = 0x9401;
	dma_entry->lenL = 0x9300; // 0x200 words size
	dma_entry->srcH = 0x9700;
	dma_entry->srcM = 0x9660;
	dma_entry->srcL = 0x9500; // 0xc000 source

	block_index += (block_index <= 0x30 ? 1 : 0x4b); // translate index
	uint32_t command = (((block_index << 25) | (block_index >> 5)) & 0x3fff0003) | 0x40000080;
	dma_entry->cmdH = command >> 16;
	dma_entry->cmdL = command;

	return (block_index << 4);
}

void md_rom_paprium_device::ppm_obj_reset()
{
	ppm_vram_reset_blocks(0);

	memset(ppm_interface->obj_data, 0, sizeof(ppm_interface->obj_data));
	ppm_drawListPtr = ppm_drawList;
}

void md_rom_paprium_device::ppm_obj_add(uint8_t num)
{
	*ppm_drawListPtr++ = num;
}

void md_rom_paprium_device::ppm_obj_frame_start()
{
	ppm_drawListPtr = ppm_drawList;

	// all blocks unused
	ppm_vram_slot_struct *slot = ppm_vram_slots;
	for (uint16_t x = 0; x < PPM_MAX_VRAM_SLOTS; slot++, x++)
		slot->usage = 0;
}

void md_rom_paprium_device::ppm_obj_frame_end()
{
	ppm_block_unpack_addr = 0x9000;
	ppm_interface->dma_remaining = ppm_interface->dma_budget - ppm_interface->dma_total;

	uint8_t *ptr = ppm_drawList;
	while (ptr != ppm_drawListPtr)
		ppm_obj_render(*ptr++);

	ppm_vram_slot_struct *slot = ppm_vram_slots;
	for (uint16_t x = 0; x < PPM_MAX_VRAM_SLOTS; slot++, x++)
		if (!slot->usage)
			slot->age++;

	ppm_close_sprite_table();
	ppm_sdram_pointer = &ppm_sdram[0x9000 / 2];
}

void md_rom_paprium_device::ppm_close_sprite_table()
{
	ppm_sat_item_struct *sat_entry = &ppm_interface->sat_data[ppm_interface->sat_count];

	if (!ppm_interface->sat_count)
	{
		sat_entry->posY = 0x10;
		sat_entry->sizeNext = 0;
		sat_entry->attrs = 0;
		sat_entry->posX = 0x10;
		ppm_interface->sat_count++;
	}
	else
		(--sat_entry)->sizeNext &= 0xff00;

	ppm_dma_command_struct *dma_entry = &ppm_interface->dma_commands[ppm_interface->dma_commands_count++];
	dma_entry->autoinc = 0x8f02;

	uint16_t word_size = ppm_interface->sat_count * (sizeof(ppm_sat_item_struct) / 2);
	dma_entry->lenH = 0x9400 + (word_size >> 8);
	dma_entry->lenL = 0x9300 + (word_size & 0xff);

	uint32_t sat_addr = offsetof(ppm_interface_struct, sat_data) / 2;
	dma_entry->srcH = 0x9700 + ((sat_addr >> 16) & 0xff);
	dma_entry->srcM = 0x9600 + ((sat_addr >> 8) & 0xff);
	dma_entry->srcL = 0x9500 + (sat_addr & 0xff);

	// xfer to SAT location in VRAM (0xf000)
	dma_entry->cmdH = 0x7000;
	dma_entry->cmdL = 0x0083;
}

void md_rom_paprium_device::ppm_obj_render(uint16_t obj_slot)
{
	ppm_intf_object_struct *intf_obj = &ppm_interface->obj_data[obj_slot]; // interface obj
	ppm_obj_struct *handle = &ppm_object_handles[obj_slot];		       // internal handle

	if ((intf_obj->anim & 0xff) > ppm_anim_max_index[intf_obj->objID & 0xff])
	{
		logerror("anim over for ID %02x: 0x%04x/0x%04x\n", intf_obj->objID & 0xff, intf_obj->anim & 0xff, ppm_anim_max_index[intf_obj->objID & 0xff]);
		return;
	}

	uint32_t offset, data_offset;
	uint32_t previous_offset = handle->anim_offset;
	uint16_t previous_counter = handle->counter;

	//	printf("Slot 0x%02x (ID 0x%04x): ", obj_slot, intf_obj->objID);

	// set / update animation
	if ((intf_obj->objID & 0x8000) || (intf_obj->anim != handle->crtAnim) || (intf_obj->animCounter != handle->counter))
	{
		// fresh obj?
		if (intf_obj->objID & 0x8000)
			previous_offset = 0, previous_counter = 1;

		offset = ppm_anim_data[(intf_obj->objID & 0xff) + 1];		 // obj offset
		offset = ppm_anim_data[(offset >> 2) + (intf_obj->anim & 0xff)]; // anim offset
		data_offset = ppm_anim_data[offset >> 2] & 0xffffffu;		 // data offset

		handle->anim_offset = offset;
		handle->crtAnim = intf_obj->anim;
		handle->counter = intf_obj->animCounter;
	}
	else
	{
		// move to next frame

		// get current offset (previous frame)
		if (!(offset = handle->anim_offset))
			return;
		// read data pointer
		data_offset = ppm_anim_data[offset >> 2];

		if (data_offset & 0x80000000u)
		{
			// previous frame was not last, just move to next
			offset += 4;
		}
		else
		{
			// anim is over, do we have fallback anim?
			if (intf_obj->nextAnim != 0xffff)
			{
				// yes, switch anims
				intf_obj->anim = intf_obj->nextAnim;
				intf_obj->nextAnim = 0xffff; // unverified
				ppm_obj_render(obj_slot);
				return;
			}
			else
			{
				// no, take anim loop
				offset = ppm_anim_data[(offset + 4) >> 2] & 0xffffffu;
			}
		}

		if (!(handle->anim_offset = offset)) // store offset
			return;
		data_offset = ppm_anim_data[offset >> 2] & 0xffffffu;
		intf_obj->animCounter++;
		handle->counter++;
	}

	// render sprite to SAT
	// > data might not be 4 bytes aligned, access via ppm_sdram <
	ppm_spr_data_header_struct *spr_info = (ppm_spr_data_header_struct *)&ppm_sdram[(ppm_anim_data_base_addr + data_offset) >> 1];
	ppm_sat_item_struct *satEntry = &ppm_interface->sat_data[ppm_interface->sat_count];
	int16_t posX = intf_obj->posX;
	int16_t posY = intf_obj->posY;

	bool blocks_available = true;
	// load new spr data
	ppm_spr_data_struct *spr_data = &spr_info->sprites[0];
	for (uint16_t x = 0; x < spr_info->count; spr_data++, x++)
	{
		if (!spr_data->blockNum)
			continue;
		if (!ppm_vram_load_block(spr_data->blockNum))
		{
			blocks_available = false;
			// break; need to iterate all to keep resevations?
		}
	}

	if (!blocks_available)
	{
		LOG(">>> block budget out / ");
		if (previous_offset)
		{
			// restore offset/counter
			handle->anim_offset = previous_offset;
			handle->counter = previous_counter;
			intf_obj->animCounter = previous_counter;
			data_offset = ppm_anim_data[previous_offset >> 2] & 0xffffff;
			spr_info = (ppm_spr_data_header_struct *)&ppm_sdram[(ppm_anim_data_base_addr + data_offset) >> 1];
			LOG("Obj #%02x: keep previous frame\n", obj_slot);
		}
		else
		{
			LOG("Obj #%02x: no render\n", obj_slot);
			return;
		}
	}

	spr_data = &spr_info->sprites[0];
	for (uint16_t x = 0; x < spr_info->count; spr_data++, x++)
	{
		posX += (intf_obj->attrs & 0x0800) ? spr_data->flipPosX : spr_data->posX;
		posY += spr_data->posY;
		if (!spr_data->blockNum)
			continue;

		// clipping
		if ((posX >= 320 + 128) ||
		    (posY >= 240 + 128) ||
		    (posX < 128 - ((((spr_data->size >> 2) & 0x3) + 1) * 8)) ||
		    (posY < 128 - (((spr_data->size & 0x3) + 1) * 8)))
			continue;

		ppm_interface->sat_count++;
		satEntry->posX = posX & 0x1ff;
		satEntry->posY = posY & 0x3ff;
		satEntry->sizeNext = ((spr_data->size & 0xf) << 8) + (ppm_interface->sat_count & 0xff);
		// attributes aren't quite right yet, some objects have wrong palette
		//satEntry->attrs = ((spr_data->attrs & 0x98) << 8) ^ intf_obj->attrs ^ (ppm_vram_find_block(spr_data->blockNum) + spr_data->offset); // whole attr word?
		satEntry->attrs = ((spr_data->attrs & 0xf8) << 8) ^ intf_obj->attrs ^ (ppm_vram_find_block(spr_data->blockNum) + spr_data->offset); // whole attr word?
		satEntry++;
	}
	intf_obj->objID &= 0x7fff;
}

void md_rom_paprium_device::ppm_stamp_rescale(uint16_t window_start, uint16_t window_end, uint16_t factor, uint16_t stamp_offset)
{
	// prepare temp buffer
	uint8_t scaled_stamp[128][32];
	memset(scaled_stamp, 0, sizeof(scaled_stamp));

	// scale the stamp in temp buffer
	float offset = (float)stamp_offset;
	float adder = (float)factor / 64;
	for (uint16_t y = window_start; y < window_end; offset += adder, y++)
		memcpy(&scaled_stamp[y][0], &ppm_scale_stamp[(int)offset][0], 32);

	// translate data: 128*32px block, as 4*32 px strips
	// layout is weird because... reasons?
	for (uint16_t s = 0; s < 32; s++) // 4px strips
	{
		uint16_t clmn = ((s & 0xfe) << 4) + ((s & 1) << 9);
		for (uint16_t y = 0; y < 32; y++)
		{
			// 1 word contains 4 pixels
			ppm_interface->scaling_buffer[clmn + y] =
			    (((scaled_stamp[(s << 2) + 0][y ^ 1] & 0xf0) |
			      (scaled_stamp[(s << 2) + 1][y ^ 1] & 0x0f))
			     << 8) +
			    (((scaled_stamp[(s << 2) + 2][y ^ 1] & 0xf0) |
			      (scaled_stamp[(s << 2) + 3][y ^ 1] & 0x0f)));
		}
	}
}
