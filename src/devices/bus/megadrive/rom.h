// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_MEGADRIVE_ROM_H
#define MAME_BUS_MEGADRIVE_ROM_H

#pragma once

#include "md_slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> md_std_rom_device

class md_std_rom_device : public device_t,
						public device_md_cart_interface
{
public:
	// construction/destruction
	md_std_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override { if (offset < 0x400000/2) return m_rom[MD_ADDR(offset)]; else return 0xffff; }
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { }

protected:
	md_std_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
};

// ======================> md_rom_sram_device

class md_rom_sram_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	md_rom_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

public:
	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;
};

// ======================> md_rom_sram_arg96_device

class md_rom_sram_arg96_device : public md_rom_sram_device
{
public:
	md_rom_sram_arg96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

// ======================> md_rom_fram_device

class md_rom_fram_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_fram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual uint16_t read_a13(offs_t offset) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;
};

// ======================> md_rom_ssf2_device

class md_rom_ssf2_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_ssf2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_bank[16];
	int m_lastoff, m_lastdata;
};

// ======================> md_rom_cm2in1_device

class md_rom_cm2in1_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_cm2in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int m_base;
};


// ======================> md_rom_mcpirate_device

class md_rom_mcpirate_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_mcpirate_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_bank;
};


// ======================> md_rom_bugslife_device

class md_rom_bugslife_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_bugslife_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_chinf3_device

class md_rom_chinf3_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_chinf3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int m_bank;
};

// ======================> md_rom_16mj2_device

class md_rom_16mj2_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_16mj2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_elfwor_device

class md_rom_elfwor_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_elfwor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_yasech_device

class md_rom_yasech_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_yasech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_kof98_device

class md_rom_kof98_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_kof98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_kof99_device

class md_rom_kof99_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_kof99_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_lion2_device

class md_rom_lion2_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_lion2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_prot1_data, m_prot2_data;
};

// ======================> md_rom_lion3_device

class md_rom_lion3_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_lion3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_reg[3];
	uint16_t m_bank;
};

// ======================> md_rom_mjlov_device

class md_rom_mjlov_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_mjlov_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_cjmjclub_device

class md_rom_cjmjclub_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_cjmjclub_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_pokea_device

class md_rom_pokea_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_pokea_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_pokestad_device

class md_rom_pokestad_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_pokestad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_bank;
};

// ======================> md_rom_realtec_device

class md_rom_realtec_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_realtec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_bank_addr, m_bank_size, m_old_bank_addr;
};

// ======================> md_rom_redcl_device

class md_rom_redcl_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_redcl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_rx3_device

class md_rom_rx3_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_rx3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_sbubl_device

class md_rom_sbubl_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_sbubl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_smb_device

class md_rom_smb_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_smb2_device

class md_rom_smb2_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smb2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_a13(offs_t offset) override;
};

// ======================> md_rom_smw64_device

class md_rom_smw64_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smw64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint32_t m_latch0, m_latch1;
	uint16_t m_reg[6];
	uint16_t m_ctrl[3];
};

// ======================> md_rom_smouse_device

class md_rom_smouse_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};


// ======================> md_rom_soulb_device

class md_rom_soulb_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_soulb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
};

// ======================> md_rom_squir_device

class md_rom_squir_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_squir_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_latch;
};

// ======================> md_rom_tc2000_device

class md_rom_tc2000_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_tc2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_retvalue;
};


// ======================> md_rom_tekkensp_device

class md_rom_tekkensp_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_tekkensp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_reg;
};

// ======================> md_rom_topf_device

class md_rom_topf_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_topf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_latch;
	uint8_t m_bank[3];
};

// ======================> md_rom_radica_device

class md_rom_radica_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_radica_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual uint16_t read_a13(offs_t offset) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_bank;
};

// ======================> md_rom_beggarp_device

class md_rom_beggarp_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_beggarp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_mode, m_lock;
};

// ======================> md_rom_wukong_device

class md_rom_wukong_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_wukong_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_mode;
};

// ======================> md_rom_starodys_device

class md_rom_starodys_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_starodys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual uint16_t read_a13(offs_t offset) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_mode, m_lock, m_ram_enable, m_base;
};


// ======================> md_rom_paprium_device

// helpers
constexpr uint32_t swapshorts(uint32_t val) { return (((val) & 0xffff0000u) >> 16) | (((val) & 0xffffu) << 16); }
#define ppm_block_addr(num) (ppm_gfx_blocks_base_addr+swapshorts(ppm_gfx_blocks_offsets[(num)]))
#define ppm_bgm_addr(num) (ppm_bgm_tracks_base_addr+swapshorts(ppm_bgm_tracks_offsets[(num)]))

// dual port ram data
typedef struct ppm_sat_item_struct
{	//VDP specs
	uint16_t posY;
	uint16_t sizeNext;
	uint16_t attrs;
	uint16_t posX;
} ppm_sat_item_struct;

typedef struct ppm_intf_object_struct
{
	uint16_t anim;
	uint16_t nextAnim;
	uint16_t objID;	//b15 = fresh assign ?
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

typedef struct ppm_interface_struct
{
	uint16_t vectors[0x80]; // 68k vectors table
	uint16_t rom_header[0x80]; // megadrive header data
	uint16_t scaling_buffer[0x300];
	uint16_t save_buffer[0x100];	// also part of scaling data
	uint16_t _padding[0x80];
	ppm_sat_item_struct sat_data[144]; // sprite allocation table sits here
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
	uint16_t dma_total; // total size in words
	uint16_t dma_budget; // per frame budget (depends on system)
	uint16_t dma_remaining; // remaining budget for current frame
	uint16_t dma_commands_count;
	uint16_t sat_count;	// # of items present in SAT table
	uint16_t unknown_1f1a;
	uint16_t unknown_1f1c;
	uint16_t unknown_1f1e;
	uint16_t buffer[0x60];
	// main registers
	uint16_t reg_unk0; // 1fe0
	uint16_t reg_unk1; // 1fe2
	uint16_t reg_status_1; // 1fe4
	struct
	{
		union
		{
			uint16_t word;
			struct
			{
				uint16_t megawire_status: 3;
				uint16_t bit_3: 1;
							// sub_9B738
				uint16_t bit_4: 1;	// MW? send a900 if set
				uint16_t megawire_data_in: 1;	// MW ack ?
				uint16_t bit_6: 1;
				uint16_t bit_7: 1;

				uint16_t eeprom_error1: 1; // b8 sum error?
				uint16_t eeprom_error2: 1; // b9 eepr error?
				uint16_t bit_10: 1;
				uint16_t bit_11: 1;

				uint16_t bit_12: 1;
				uint16_t bit_13: 1;
				uint16_t busy: 1; // b14 task busy
				uint16_t bit_15: 1;
			} bits;
		};
	} reg_status_2; // 1fe6
	uint16_t reg_unk4; // 1fe8
	uint16_t reg_command;
	uint16_t reg_watchdog; // unconfirmed
	uint16_t reg_unk7; // 1fee
	uint16_t reg_unk8; // 1ff0
	uint16_t reg_unk9; // 1ff2
	uint16_t reg_unkA; // 1ff4
	uint16_t reg_unkB; // 1ff6
	uint16_t reg_unkC; // 1ff8
	uint16_t reg_unkD; // 1ffa
	uint16_t reg_unkE; // 1ffc
	uint16_t reg_unkF; // 1ffe
} ppm_interface_struct;

// sdram data
#define PPM_DATA_START_ADDR 0x10000	// base sdram unpack address
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
	uint32_t anim_offset;	//
	uint32_t lastDisplayedOffset;	//
	uint16_t crtAnim;
	uint16_t counter;
} ppm_obj_struct;

typedef struct ppm_spr_data_struct
{	//byteswapped (endianess fix)
	int8_t posY;
	int8_t posX;
	int8_t flipPosX;
	uint8_t size;	//lower nibble
	uint16_t blockNum;
	uint8_t offset; //block offset
	uint8_t attrs;	//flip posY?
} ppm_spr_data_struct;

typedef struct ppm_spr_data_header_struct
{	//byteswapped (endianess fix)
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
	uint8_t ppm_drawList[PPM_OBJECTS_COUNT]; // contains slot #
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


// device type definition
DECLARE_DEVICE_TYPE(MD_STD_ROM,      md_std_rom_device)
DECLARE_DEVICE_TYPE(MD_ROM_SRAM,     md_rom_sram_device)
DECLARE_DEVICE_TYPE(MD_ROM_FRAM,     md_rom_fram_device)
DECLARE_DEVICE_TYPE(MD_ROM_CM2IN1,   md_rom_cm2in1_device)
DECLARE_DEVICE_TYPE(MD_ROM_SSF2,     md_rom_ssf2_device)
DECLARE_DEVICE_TYPE(MD_ROM_BUGSLIFE, md_rom_bugslife_device)
DECLARE_DEVICE_TYPE(MD_ROM_SMOUSE,   md_rom_smouse_device)
DECLARE_DEVICE_TYPE(MD_ROM_SMW64,    md_rom_smw64_device)
DECLARE_DEVICE_TYPE(MD_ROM_SMB,      md_rom_smb_device)
DECLARE_DEVICE_TYPE(MD_ROM_SMB2,     md_rom_smb2_device)
DECLARE_DEVICE_TYPE(MD_ROM_SBUBL,    md_rom_sbubl_device)
DECLARE_DEVICE_TYPE(MD_ROM_RX3,      md_rom_rx3_device)
DECLARE_DEVICE_TYPE(MD_ROM_MJLOV,    md_rom_mjlov_device)
DECLARE_DEVICE_TYPE(MD_ROM_CJMJCLUB, md_rom_cjmjclub_device)
DECLARE_DEVICE_TYPE(MD_ROM_KOF98,    md_rom_kof98_device)
DECLARE_DEVICE_TYPE(MD_ROM_KOF99,    md_rom_kof99_device)
DECLARE_DEVICE_TYPE(MD_ROM_SOULB,    md_rom_soulb_device)
DECLARE_DEVICE_TYPE(MD_ROM_CHINF3,   md_rom_chinf3_device)
DECLARE_DEVICE_TYPE(MD_ROM_16MJ2,    md_rom_16mj2_device)
DECLARE_DEVICE_TYPE(MD_ROM_ELFWOR,   md_rom_elfwor_device)
DECLARE_DEVICE_TYPE(MD_ROM_YASECH,   md_rom_yasech_device)
DECLARE_DEVICE_TYPE(MD_ROM_LION2,    md_rom_lion2_device)
DECLARE_DEVICE_TYPE(MD_ROM_LION3,    md_rom_lion3_device)
DECLARE_DEVICE_TYPE(MD_ROM_MCPIR,    md_rom_mcpirate_device)
DECLARE_DEVICE_TYPE(MD_ROM_POKEA,    md_rom_pokea_device)
DECLARE_DEVICE_TYPE(MD_ROM_POKESTAD, md_rom_pokestad_device)
DECLARE_DEVICE_TYPE(MD_ROM_REALTEC,  md_rom_realtec_device)
DECLARE_DEVICE_TYPE(MD_ROM_REDCL,    md_rom_redcl_device)
DECLARE_DEVICE_TYPE(MD_ROM_SQUIR,    md_rom_squir_device)
DECLARE_DEVICE_TYPE(MD_ROM_SRAM_ARG96, md_rom_sram_arg96_device)
DECLARE_DEVICE_TYPE(MD_ROM_TC2000,   md_rom_tc2000_device)
DECLARE_DEVICE_TYPE(MD_ROM_TEKKENSP, md_rom_tekkensp_device)
DECLARE_DEVICE_TYPE(MD_ROM_TOPF,     md_rom_topf_device)
DECLARE_DEVICE_TYPE(MD_ROM_RADICA,   md_rom_radica_device)
DECLARE_DEVICE_TYPE(MD_ROM_BEGGARP,  md_rom_beggarp_device)
DECLARE_DEVICE_TYPE(MD_ROM_WUKONG,   md_rom_wukong_device)
DECLARE_DEVICE_TYPE(MD_ROM_STARODYS, md_rom_starodys_device)
DECLARE_DEVICE_TYPE(MD_ROM_PAPRIUM,  md_rom_paprium_device)

#endif // MAME_BUS_MEGADRIVE_ROM_H
