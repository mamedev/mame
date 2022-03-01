// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Brad Oliver
#ifndef MAME_INCLUDES_PLAYCH10_H
#define MAME_INCLUDES_PLAYCH10_H

#pragma once

#include "cpu/m6502/n2a03.h"
#include "machine/rp5h01.h"
#include "video/ppu2c0x.h"
#include "emupal.h"
#include "tilemap.h"

class playch10_state : public driver_device
{
public:
	playch10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cartcpu(*this, "cart")
		, m_ppu(*this, "ppu")
		, m_rp5h01(*this, "rp5h01")
		, m_ram_8w(*this, "ram_8w")
		, m_videoram(*this, "videoram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_prg_banks(*this, "prg%u", 0U)
		, m_prg_view(*this, "prg_view")
		, m_vrom_region(*this, "gfx2")
		, m_timedigits(*this, "digit_%u", 0U)
	{
	}

	void playch10(machine_config &config);
	void playch10_a(machine_config &config);
	void playch10_b(machine_config &config);
	void playch10_c(machine_config &config);
	void playch10_d(machine_config &config);
	void playch10_d2(machine_config &config);
	void playch10_e(machine_config &config);
	void playch10_f(machine_config &config);
	void playch10_f2(machine_config &config);
	void playch10_g(machine_config &config);
	void playch10_h(machine_config &config);
	void playch10_i(machine_config &config);
	void playch10_k(machine_config &config);

	void init_playch10();
	void init_pc_gun();
	void init_pcaboard();
	void init_pcbboard();
	void init_pccboard();
	void init_pcdboard();
	void init_pceboard();
	void init_pcfboard();
	void init_rp5h01_fix();
	void init_virus();
	void init_ttoon();
	void init_pcgboard();
	void init_pcgboard_type2();
	void init_pchboard();
	void init_pciboard();
	void init_pckboard();
	void init_pc_hrz();

	DECLARE_READ_LINE_MEMBER(int_detect_r);

private:
	DECLARE_WRITE_LINE_MEMBER(up8w_w);
	uint8_t ram_8w_r(offs_t offset);
	void ram_8w_w(offs_t offset, uint8_t data);
	void sprite_dma_w(address_space &space, uint8_t data);
	void time_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(sdcs_w);
	DECLARE_WRITE_LINE_MEMBER(cntrl_mask_w);
	DECLARE_WRITE_LINE_MEMBER(disp_mask_w);
	DECLARE_WRITE_LINE_MEMBER(sound_mask_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_enable_w);
	DECLARE_WRITE_LINE_MEMBER(dog_di_w);
	DECLARE_WRITE_LINE_MEMBER(ppu_reset_w);
	uint8_t pc10_detectclr_r();
	void cart_sel_w(uint8_t data);
	uint8_t pc10_prot_r();
	void pc10_prot_w(uint8_t data);
	void pc10_in0_w(uint8_t data);
	uint8_t pc10_in0_r();
	uint8_t pc10_in1_r();
	void pc10_nt_w(offs_t offset, uint8_t data);
	uint8_t pc10_nt_r(offs_t offset);
	void pc10_chr_w(offs_t offset, uint8_t data);
	uint8_t pc10_chr_r(offs_t offset);
	void mmc1_rom_switch_w(offs_t offset, uint8_t data);
	void aboard_vrom_switch_w(uint8_t data);
	void bboard_rom_switch_w(uint8_t data);
	void cboard_vrom_switch_w(uint8_t data);
	void eboard_rom_switch_w(offs_t offset, uint8_t data);
	void gboard_rom_switch_w(offs_t offset, uint8_t data);
	void hboard_rom_switch_w(offs_t offset, uint8_t data);
	void iboard_rom_switch_w(uint8_t data);
	void playch10_videoram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void bios_io_map(address_map &map);
	void bios_map(address_map &map);
	void ppu_map(address_map &map);
	void cart_map(address_map &map);
	void cart_a_map(address_map &map);
	void cart_b_map(address_map &map);
	void cart_c_map(address_map &map);
	void cart_d_map(address_map &map);
	void cart_d2_map(address_map &map);
	void cart_e_map(address_map &map);
	void cart_f_map(address_map &map);
	void cart_f2_map(address_map &map);
	void cart_g_map(address_map &map);
	void cart_h_map(address_map &map);
	void cart_i_map(address_map &map);
	void cart_k_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	struct chr_bank
	{
		int writable;   // 1 for RAM, 0 for ROM
		uint8_t* chr;     // direct access to the memory
	};

	void playch10_palette(palette_device &palette) const;
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	void pc10_set_videorom_bank( int first, int count, int bank, int size );
	void set_videoram_bank( int first, int count, int bank, int size );
	void gboard_scanline_cb( int scanline, int vblank, int blanked );
	DECLARE_WRITE_LINE_MEMBER(int_detect_w);
	void mapper9_latch(offs_t offset);
	void pc10_set_mirroring(int mirroring);

	uint32_t screen_update_playch10_top(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_playch10_bottom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_playch10_single(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<n2a03_device> m_cartcpu;
	required_device<ppu2c0x_device> m_ppu;
	optional_device<rp5h01_device> m_rp5h01;

	required_shared_ptr<uint8_t> m_ram_8w;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;

	void init_prg_banking();
	void prg32(int bank);
	void prg16(int slot, int bank);
	void prg8(int slot, int bank);
	memory_bank_array_creator<4> m_prg_banks;
	memory_view m_prg_view;
	int m_prg_chunks;

	optional_memory_region m_vrom_region;

	output_finder<4> m_timedigits;

	int m_up_8w;
	int m_pc10_nmi_enable;
	int m_pc10_dog_di;
	int m_pc10_sdcs;
	int m_pc10_dispmask;
	int m_pc10_int_detect;
	int m_pc10_game_mode;
	int m_pc10_dispmask_old;
	int m_pc10_gun_controller;
	int m_cart_sel;
	int m_cntrl_mask;
	int m_input_latch[2];
	int m_mirroring;
	int m_MMC2_bank[4];
	int m_MMC2_bank_latch[2];
	uint8_t* m_vrom;
	std::unique_ptr<uint8_t[]> m_vram;
	uint8_t* m_nametable[4];
	std::unique_ptr<uint8_t[]> m_nt_ram;
	std::unique_ptr<uint8_t[]> m_cart_nt_ram;
	chr_bank m_chr_page[8];
	int m_mmc1_shiftreg;
	int m_mmc1_shiftcount;
	int m_gboard_banks[2];
	int m_gboard_command;
	int m_IRQ_count;
	uint8_t m_IRQ_count_latch;
	int m_IRQ_enable;
	int m_pc10_bios;
	tilemap_t *m_bg_tilemap;
};

#endif // MAME_INCLUDES_PLAYCH10_H
