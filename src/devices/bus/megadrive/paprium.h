// license:BSD-3-Clause
// copyright-holders:Hpman
#ifndef MAME_BUS_MEGADRIVE_PAPRIUM_H
#define MAME_BUS_MEGADRIVE_PAPRIUM_H

#pragma once

#include "md_slot.h"

// ======================> md_rom_paprium_device

// helpers
constexpr uint32_t swapshorts(uint32_t val) { return (((val) & 0xffff0000u) >> 16) | (((val) & 0xffffu) << 16); }
#define ppm_block_addr(num) (ppm_gfx_blocks_base_addr + swapshorts(ppm_gfx_blocks_offsets[(num)]))
#define ppm_bgm_addr(num) (ppm_bgm_tracks_base_addr + swapshorts(ppm_bgm_tracks_offsets[(num)]))

// dual port ram data
typedef struct ppm_sat_item_struct
{ // VDP specs
	uint16_t posY;
	uint16_t sizeNext;
	uint16_t attrs;
	uint16_t posX;
} ppm_sat_item_struct;

typedef struct ppm_intf_object_struct
{
	uint16_t anim;
	uint16_t nextAnim;
	uint16_t objID; // b15 = fresh assign ?
	uint16_t field_6;
	uint16_t attrs;
	uint16_t animCounter;
	int16_t posX;
	int16_t posY;
} ppm_intf_object_struct;

typedef struct ppm_dma_command_struct
{
	uint16_t autoinc;
	uint16_t lenH;
	uint16_t lenL;
	uint16_t srcH;
	uint16_t srcM;
	uint16_t srcL;
	uint16_t cmdH;
	uint16_t cmdL;
} ppm_dma_command_struct;

// defines for status register 2 (0x1fe6)
#define STATUS2_BUSY 0x4000	     // bit 14
#define STATUS2_EEPROM_ERROR1 0x0100 // bit 8
#define STATUS2_EEPROM_ERROR2 0x0200 // bit 9
#define STATUS2_MW_DATA_IN 0x0020    // bit 5

typedef struct ppm_interface_struct
{
	uint16_t vectors[0x80];	   // 68k vectors table
	uint16_t rom_header[0x80]; // megadrive header data
	uint16_t scaling_buffer[0x300];
	uint16_t save_buffer[0x100]; // also part of scaling data
	uint16_t _padding[0x80];
	ppm_sat_item_struct sat_data[144];   // sprite allocation table sits here
	ppm_intf_object_struct obj_data[64]; // objects table
	uint16_t _padding2[0x40];
	// 0x1400
	ppm_dma_command_struct dma_commands[121];
	uint16_t audio_data[0x70 / 2];
	uint16_t network_data[0x108];
	union
	{
		uint16_t command_args[128];
		uint32_t command_args_long[64];
	};
	uint16_t dma_total;	// total size in words
	uint16_t dma_budget;	// per frame budget (depends on system)
	uint16_t dma_remaining; // remaining budget for current frame
	uint16_t dma_commands_count;
	uint16_t sat_count; // # of items present in SAT table
	uint16_t unknown_1f1a;
	uint16_t unknown_1f1c;
	uint16_t unknown_1f1e;
	uint16_t buffer[0x60];
	// main registers
	uint16_t reg_unk0;     // 1fe0
	uint16_t reg_unk1;     // 1fe2
	uint16_t reg_status_1; // 1fe4
	uint16_t reg_status_2; // 1fe6
	uint16_t reg_unk4;     // 1fe8
	uint16_t reg_command;
	uint16_t reg_watchdog; // unconfirmed
	uint16_t reg_unk7;     // 1fee
	uint16_t reg_unk8;     // 1ff0
	uint16_t reg_unk9;     // 1ff2
	uint16_t reg_unkA;     // 1ff4
	uint16_t reg_unkB;     // 1ff6
	uint16_t reg_unkC;     // 1ff8
	uint16_t reg_unkD;     // 1ffa
	uint16_t reg_unkE;     // 1ffc
	uint16_t reg_unkF;     // 1ffe
} ppm_interface_struct;

// sdram data
#define PPM_MAX_VRAM_SLOTS 64
#define PPM_OBJECTS_COUNT 64
#define PPM_MAX_OBJ_BLOCKS 32

typedef struct ppm_sfx_struct
{
	uint32_t offset;
	uint16_t attributes;
	uint16_t length;
} ppm_sfx_struct;

typedef struct ppm_vram_slot_struct
{
	uint16_t block_num;
	uint16_t usage;
	uint16_t age;
} ppm_vram_slot_struct;

typedef struct ppm_obj_struct
{
	uint32_t anim_offset;	      //
	uint32_t lastDisplayedOffset; //
	uint16_t crtAnim;
	uint16_t counter;
} ppm_obj_struct;

typedef struct ppm_spr_data_struct
{ // byteswapped (endianess fix)
	int8_t posY;
	int8_t posX;
	int8_t flipPosX;
	uint8_t size; // lower nibble
	uint16_t blockNum;
	uint8_t offset; // block offset
	uint8_t attrs;	// flip posY?
} ppm_spr_data_struct;

typedef struct ppm_spr_data_header_struct
{ // byteswapped (endianess fix)
	uint8_t flags;
	uint8_t count;
	ppm_spr_data_struct sprites[];
} ppm_spr_data_header_struct;

// class md_rom_paprium_device : public md_std_rom_device
class md_rom_paprium_device : public md_rom_sram_device
{
public:
	// construction/destruction
	md_rom_paprium_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// memory interfaces
	uint16_t ppm_dual_port_ram[0x2000 / 2];
	ppm_interface_struct *ppm_interface;
	uint16_t ppm_sdram[0x200000 / 2];
	virtual uint32_t ppm_sdram_read_u32(uint32_t addr);

	// commands processing
	virtual void ppm_process_command();

	// scaling items
	uint8_t ppm_scale_stamp[64][32];
	virtual void ppm_stamp_rescale(uint16_t window_start, uint16_t window_end, uint16_t factor, uint16_t stamp_offset);

	// unpack & related data
	virtual uint32_t ppm_unpack(uint32_t from, uint32_t to, bool is_scaling_stamp = false);
	virtual void ppm_setup_data(uint32_t bgm_file, uint32_t unk1_file, uint32_t smp_file, uint32_t unk2_file, uint32_t sfx_file, uint32_t anm_file, uint32_t blk_file);
	uint16_t *ppm_sdram_pointer;
	bool ppm_sdram_window_enabled;

	// objects setup/render
	virtual void ppm_obj_reset();
	virtual void ppm_obj_add(uint8_t num);
	virtual void ppm_obj_frame_start();
	virtual void ppm_obj_frame_end();
	virtual void ppm_close_sprite_table();
	virtual void ppm_obj_render(uint16_t obj_slot);
	ppm_obj_struct ppm_object_handles[PPM_OBJECTS_COUNT]; // matching intf objs
	uint8_t ppm_drawList[PPM_OBJECTS_COUNT];	      // contains slot #
	uint8_t *ppm_drawListPtr;

	// block data
	virtual void ppm_vram_set_budget(uint16_t blocks);
	virtual void ppm_vram_reset_blocks(uint16_t last_block);
	virtual uint16_t ppm_vram_find_block(uint16_t num);
	virtual uint16_t ppm_vram_load_block(uint16_t num);
	ppm_vram_slot_struct ppm_vram_slots[PPM_MAX_VRAM_SLOTS];
	uint16_t ppm_vram_max_slot;
	uint16_t ppm_block_unpack_addr;

	// keeping track of various data locations
	uint32_t ppm_anim_data_base_addr;
	uint32_t *ppm_anim_data;
	uint16_t ppm_anim_max_index[256];

	uint32_t ppm_unk_data_addr;
	uint32_t ppm_unk_data2_addr;

	uint32_t *ppm_bgm_tracks_offsets;
	uint32_t ppm_bgm_tracks_base_addr;
	uint32_t ppm_bgm_unpack_addr;

	ppm_sfx_struct *ppm_sfx_data;
	uint32_t ppm_sfx_base_addr;

	uint32_t *ppm_gfx_blocks_offsets;
	uint32_t ppm_gfx_blocks_base_addr;
	uint32_t ppm_max_gfx_block;

	// audio related (unsupported)
	uint8_t ppm_audio_bgm_volume;
	uint8_t ppm_audio_sfx_volume;
	uint8_t ppm_audio_config;
};

DECLARE_DEVICE_TYPE(MD_ROM_PAPRIUM, md_rom_paprium_device)

#endif // MAME_BUS_MEGADRIVE_PAPRIUM_H
