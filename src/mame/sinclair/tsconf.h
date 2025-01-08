// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/*****************************************************************************
 *
 * includes/tsconfig.h
 *
 ****************************************************************************/
#ifndef MAME_SINCLAIR_TSCONF_H
#define MAME_SINCLAIR_TSCONF_H

#pragma once

#include "spec128.h"

#include "glukrs.h"
#include "tsconfdma.h"

#include "beta_m.h"
#include "machine/pckeybrd.h"
#include "machine/spi_sdcard.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "tilemap.h"


class tsconf_state : public spectrum_128_state
{
public:
	tsconf_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag)
		, m_bank0_rom(*this, "bank0_rom")
		, m_keyboard(*this, "pc_keyboard")
		, m_io_mouse(*this, "mouse_input%u", 1U)
		, m_beta(*this, BETA_DISK_TAG)
		, m_dma(*this, "dma")
		, m_sdcard(*this, "sdcard")
		, m_glukrs(*this, "glukrs")
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_cram(*this, "cram")
		, m_sfile(*this, "sfile")
		, m_dac(*this, "dac")
		, m_ay(*this, "ay%u", 0U)
		, m_mod_ay(*this, "MOD_AY")
	{
	}

	void tsconf(machine_config &config);

	static constexpr u16 with_hblank(u16 pixclocks) { return 88 + pixclocks; }
	static constexpr u16 with_vblank(u16 pixclocks) { return 32 + pixclocks; }

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	virtual TIMER_CALLBACK_MEMBER(irq_off) override;
	TIMER_CALLBACK_MEMBER(irq_frame);
	TIMER_CALLBACK_MEMBER(irq_scanline);

private:
	enum gluk_ext : u8
	{
		CONF_VERSION = 0x00,
		BOOTLOADER_VERSION = 0x01,
		PS2KEYBOARDS_LOG = 0x02,
		RDCFG = 0x03,
		CONFIG = 0x0e,
		SPIFL = 0x10,
	};

	enum tilemaps : u8
	{
		TM_TS_CHAR = 0x00,
		TM_TILES0,
		TM_TILES1,
		TM_SPRITES,
		TM_ZX_CHAR,
	};

	enum tsconf_regs : u8
	{
		V_CONFIG = 0x00,
		V_PAGE = 0x01,

		G_X_OFFS_L = 0x02,
		G_X_OFFS_H = 0x03,
		G_Y_OFFS_L = 0x04,
		G_Y_OFFS_H = 0x05,

		TS_CONFIG = 0x06,
		PAL_SEL = 0x07,

		BORDER = 0x0f,
		PAGE0 = 0x10,
		PAGE1 = 0x11,
		PAGE2 = 0x12,
		PAGE3 = 0x13,

		FMAPS = 0x15,
		T_MAP_PAGE = 0x16,
		T0_G_PAGE = 0x17,
		T1_G_PAGE = 0x18,
		SG_PAGE = 0x19,

		DMAS_ADDRESS_L = 0x1a,
		DMAS_ADDRESS_H = 0x1b,
		DMAS_ADDRESS_X = 0x1c,
		DMAD_ADDRESS_L = 0x1d,
		DMAD_ADDRESS_H = 0x1e,
		DMAD_ADDRESS_X = 0x1f,

		SYS_CONFIG = 0x20,
		MEM_CONFIG = 0x21,
		HS_INT = 0x22,
		VS_INT_L = 0x23,
		VS_INT_H = 0x24,

		DMA_WAIT_PORT_DEV = 0x25,
		DMA_LEN = 0x26,
		DMA_CTRL = 0x27,
		DMA_NUM_L = 0x28,
		FDD_VIRT = 0x29,
		INT_MASK = 0x2a,
		CACHE_CONFIG = 0x2b,
		DMA_NUM_H = 0x2c,

		T0_X_OFFSET_L = 0x40,
		T0_X_OFFSET_H = 0x41,
		T0_Y_OFFSET_L = 0x42,
		T0_Y_OFFSET_H = 0x43,
		T1_X_OFFSET_L = 0x44,
		T1_X_OFFSET_H = 0x45,
		T1_Y_OFFSET_L = 0x46,
		T1_Y_OFFSET_H = 0x47
	};

	struct sprite_data
	{
		u32 code;
		u32 color;
		int flipx;
		int flipy;
		s32 destx;
		s32 desty;
		u32 pmask;
	};

	void update_frame_timer();
	emu_timer *m_frame_irq_timer = nullptr;
	emu_timer *m_scanline_irq_timer = nullptr;

	INTERRUPT_GEN_MEMBER(tsconf_vblank_interrupt);
	IRQ_CALLBACK_MEMBER(irq_vector);
	u8 m_int_mask;

	DECLARE_VIDEO_START(tsconf);
	TILE_GET_INFO_MEMBER(get_tile_info_txt);
	template <u8 Layer>
	TILE_GET_INFO_MEMBER(get_tile_info_16c);

	virtual u8 get_border_color(u16 hpos = ~0, u16 vpos = ~0) override;
	u32 get_vpage_offset();
	virtual rectangle get_screen_area() override;
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void tsconf_update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void tsconf_draw_zx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void tsconf_draw_txt(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void tsconf_draw_gfx(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void tsconf_palette(palette_device &palette) const;
	void tsconf_update_video_mode();

	u8 tsconf_port_xx1f_r(offs_t offset);
	void tsconf_port_7ffd_w(u8 data);
	void tsconf_ula_w(offs_t offset, u8 data);
	u8 tsconf_port_xxaf_r(offs_t reg);
	void tsconf_port_xxaf_w(offs_t reg, u8 data);
	u8 tsconf_port_77_zctr_r();
	void tsconf_port_77_zctr_w(u8 data);
	u8 tsconf_port_57_zctr_r();
	void tsconf_port_57_zctr_w(u8 data);
	void tsconf_spi_miso_w(u8 data);
	u8 tsconf_port_f7_r(offs_t offset);
	void tsconf_port_f7_w(offs_t offset, u8 data);
	void tsconf_ay_address_w(u8 data);

	void tsconf_update_bank0();
	u8 beta_neutral_r(offs_t offset);
	u8 beta_enable_r(offs_t offset);
	u8 beta_disable_r(offs_t offset);

	void tsconf_io(address_map &map) ATTR_COLD;
	void tsconf_mem(address_map &map) ATTR_COLD;
	void tsconf_switch(address_map &map) ATTR_COLD;

	u8 mem_bank_read(u8 bank, offs_t offset);
	template <u8 Bank>
	void tsconf_bank_w(offs_t offset, u8 data);
	void ram_bank_write(u8 bank, offs_t offset, u8 data);
	void ram_page_write(u8 page, offs_t offset, u8 data);
	void cram_write(u16 offset, u8 data);
	void cram_write16(offs_t offset, u16 data);
	void sfile_write16(offs_t offset, u16 data);
	void ram_write16(offs_t offset, u16 data);
	u16 ram_read16(offs_t offset);
	u16 spi_read16();
	void dma_ready(int line);

	std::map<tsconf_regs, u8> m_scanline_delayed_regs_update;
	u8 m_regs[0x100];

	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_view m_bank0_rom;

	required_device<at_keyboard_device> m_keyboard;
	required_ioport_array<3> m_io_mouse;

	required_device<beta_disk_device> m_beta;
	required_device<tsconfdma_device> m_dma;
	required_device<spi_sdcard_device> m_sdcard;
	u8 m_zctl_di = 0;
	u8 m_zctl_cs = 0;

	required_device<glukrs_device> m_glukrs;
	gluk_ext m_port_f7_ext{};

	s16 m_gfx_y_frame_offset = 0;
	required_device<device_palette_interface> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	tilemap_t *m_ts_tilemap[3]{};
	required_device<ram_device> m_cram;
	required_device<ram_device> m_sfile;
	std::vector<sprite_data> m_sprites_cache;

	required_device<dac_byte_interface> m_dac;
	required_device_array<ym2149_device, 2> m_ay;
	u8 m_ay_selected;
	required_ioport m_mod_ay;
};

/*----------- defined in drivers/tsconf.c -----------*/

INPUT_PORTS_EXTERN(tsconf);

#endif // MAME_SINCLAIR_TSCONF_H
