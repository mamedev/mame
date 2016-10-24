// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Brad Oliver
#include "machine/rp5h01.h"
#include "video/ppu2c0x.h"

struct chr_bank
{
	int writable;   // 1 for RAM, 0 for ROM
	uint8_t* chr;     // direct access to the memory
};

class playch10_state : public driver_device
{
public:
	playch10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppu(*this, "ppu")
		, m_rp5h01(*this, "rp5h01")
		, m_ram_8w(*this, "ram_8w")
		, m_videoram(*this, "videoram")
		, m_timedata(*this, "timedata")
		, m_work_ram(*this, "work_ram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_vrom_region(*this, "gfx2")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;
	optional_device<rp5h01_device> m_rp5h01;

	required_shared_ptr<uint8_t> m_ram_8w;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_timedata;
	required_shared_ptr<uint8_t> m_work_ram;
	required_device<gfxdecode_device> m_gfxdecode;

	optional_memory_region m_vrom_region;

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
	chr_bank m_chr_page[8];
	int m_mmc1_shiftreg;
	int m_mmc1_shiftcount;
	int m_mmc1_rom_mask;
	int m_gboard_scanline_counter;
	int m_gboard_scanline_latch;
	int m_gboard_banks[2];
	int m_gboard_4screen;
	int m_gboard_last_bank;
	int m_gboard_command;
	int m_IRQ_count;
	uint8_t m_IRQ_count_latch;
	int m_IRQ_enable;
	int m_pc10_bios;
	tilemap_t *m_bg_tilemap;
	void up8w_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ram_8w_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ram_8w_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sprite_dma_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void time_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc10_SDCS_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc10_CNTRLMASK_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc10_DISPMASK_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc10_SOUNDMASK_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc10_NMIENABLE_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc10_DOGDI_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc10_GAMERES_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc10_GAMESTOP_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc10_PPURES_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pc10_detectclr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pc10_CARTSEL_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pc10_prot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pc10_prot_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc10_in0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pc10_in0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pc10_in1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pc10_nt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pc10_nt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pc10_chr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pc10_chr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mmc1_rom_switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aboard_vrom_switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bboard_rom_switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cboard_vrom_switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void eboard_rom_switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gboard_rom_switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void iboard_rom_switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hboard_rom_switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc10_set_mirroring(int mirroring);
	void playch10_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value pc10_int_detect_r(ioport_field &field, void *param);
	void init_playch10();
	void init_pc_gun();
	void init_pcaboard();
	void init_pcbboard();
	void init_pccboard();
	void init_pcdboard();
	void init_pcdboard_2();
	void init_pceboard();
	void init_pcfboard();
	void init_pcfboard_2();
	void init_virus();
	void init_ttoon();
	void init_pcgboard();
	void init_pcgboard_type2();
	void init_pchboard();
	void init_pciboard();
	void init_pckboard();
	void init_pc_hrz();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_playch10(palette_device &palette);
	void machine_start_playch10_hboard();
	void video_start_playch10_hboard();
	uint32_t screen_update_playch10_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_playch10_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_playch10_single(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void playch10_interrupt(device_t &device);
	void pc10_set_videorom_bank( int first, int count, int bank, int size );
	void set_videoram_bank( int first, int count, int bank, int size );
	void gboard_scanline_cb( int scanline, int vblank, int blanked );
	void ppu_irq(int *ppu_regs);
	void mapper9_latch(offs_t offset);
};
