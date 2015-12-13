// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Brad Oliver
#include "machine/rp5h01.h"
#include "video/ppu2c0x.h"

struct chr_bank
{
	int writable;   // 1 for RAM, 0 for ROM
	UINT8* chr;     // direct access to the memory
};

class playch10_state : public driver_device
{
public:
	playch10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppu(*this, "ppu"),
		m_rp5h01(*this, "rp5h01"),
		m_ram_8w(*this, "ram_8w"),
		m_videoram(*this, "videoram"),
		m_timedata(*this, "timedata"),
		m_work_ram(*this, "work_ram"),
		m_gfxdecode(*this, "gfxdecode")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;
	optional_device<rp5h01_device> m_rp5h01;

	required_shared_ptr<UINT8> m_ram_8w;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_timedata;
	required_shared_ptr<UINT8> m_work_ram;
	required_device<gfxdecode_device> m_gfxdecode;

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
	UINT8* m_vrom;
	UINT8* m_vram;
	UINT8* m_nametable[4];
	UINT8* m_nt_ram;
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
	UINT8 m_IRQ_count_latch;
	int m_IRQ_enable;
	int m_pc10_bios;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(up8w_w);
	DECLARE_READ8_MEMBER(ram_8w_r);
	DECLARE_WRITE8_MEMBER(ram_8w_w);
	DECLARE_WRITE8_MEMBER(sprite_dma_w);
	DECLARE_WRITE8_MEMBER(time_w);
	DECLARE_WRITE8_MEMBER(pc10_SDCS_w);
	DECLARE_WRITE8_MEMBER(pc10_CNTRLMASK_w);
	DECLARE_WRITE8_MEMBER(pc10_DISPMASK_w);
	DECLARE_WRITE8_MEMBER(pc10_SOUNDMASK_w);
	DECLARE_WRITE8_MEMBER(pc10_NMIENABLE_w);
	DECLARE_WRITE8_MEMBER(pc10_DOGDI_w);
	DECLARE_WRITE8_MEMBER(pc10_GAMERES_w);
	DECLARE_WRITE8_MEMBER(pc10_GAMESTOP_w);
	DECLARE_WRITE8_MEMBER(pc10_PPURES_w);
	DECLARE_READ8_MEMBER(pc10_detectclr_r);
	DECLARE_WRITE8_MEMBER(pc10_CARTSEL_w);
	DECLARE_READ8_MEMBER(pc10_prot_r);
	DECLARE_WRITE8_MEMBER(pc10_prot_w);
	DECLARE_WRITE8_MEMBER(pc10_in0_w);
	DECLARE_READ8_MEMBER(pc10_in0_r);
	DECLARE_READ8_MEMBER(pc10_in1_r);
	DECLARE_WRITE8_MEMBER(pc10_nt_w);
	DECLARE_READ8_MEMBER(pc10_nt_r);
	DECLARE_WRITE8_MEMBER(pc10_chr_w);
	DECLARE_READ8_MEMBER(pc10_chr_r);
	DECLARE_WRITE8_MEMBER(mmc1_rom_switch_w);
	DECLARE_WRITE8_MEMBER(aboard_vrom_switch_w);
	DECLARE_WRITE8_MEMBER(bboard_rom_switch_w);
	DECLARE_WRITE8_MEMBER(cboard_vrom_switch_w);
	DECLARE_WRITE8_MEMBER(eboard_rom_switch_w);
	DECLARE_WRITE8_MEMBER(gboard_rom_switch_w);
	DECLARE_WRITE8_MEMBER(iboard_rom_switch_w);
	DECLARE_WRITE8_MEMBER(hboard_rom_switch_w);
	void pc10_set_mirroring(int mirroring);
	DECLARE_WRITE8_MEMBER(playch10_videoram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(pc10_int_detect_r);
	DECLARE_DRIVER_INIT(playch10);
	DECLARE_DRIVER_INIT(pc_gun);
	DECLARE_DRIVER_INIT(pcaboard);
	DECLARE_DRIVER_INIT(pcbboard);
	DECLARE_DRIVER_INIT(pccboard);
	DECLARE_DRIVER_INIT(pcdboard);
	DECLARE_DRIVER_INIT(pcdboard_2);
	DECLARE_DRIVER_INIT(pceboard);
	DECLARE_DRIVER_INIT(pcfboard);
	DECLARE_DRIVER_INIT(pcfboard_2);
	DECLARE_DRIVER_INIT(virus);
	DECLARE_DRIVER_INIT(pcgboard);
	DECLARE_DRIVER_INIT(pcgboard_type2);
	DECLARE_DRIVER_INIT(pchboard);
	DECLARE_DRIVER_INIT(pciboard);
	DECLARE_DRIVER_INIT(pckboard);
	DECLARE_DRIVER_INIT(pc_hrz);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(playch10);
	DECLARE_MACHINE_START(playch10_hboard);
	DECLARE_VIDEO_START(playch10_hboard);
	UINT32 screen_update_playch10_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_playch10_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_playch10_single(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(playch10_interrupt);
	void pc10_set_videorom_bank( int first, int count, int bank, int size );
	void set_videoram_bank( int first, int count, int bank, int size );
	void gboard_scanline_cb( int scanline, int vblank, int blanked );
	void ppu_irq(int *ppu_regs);
	void mapper9_latch(offs_t offset);
};
