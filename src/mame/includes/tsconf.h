// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/*****************************************************************************
 *
 * includes/tsconfig.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_TSCONF_H
#define MAME_INCLUDES_TSCONF_H

#pragma once

#include "spectrum.h"

#include "machine/beta.h"

#include "machine/glukrs.h"
#include "machine/pckeybrd.h"
#include "machine/spi_sdcard.h"
#include "machine/tsconfdma.h"

#include "tilemap.h"


class tsconf_state : public spectrum_128_state
{
public:
	tsconf_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag),
		  m_p_rom(*this, "maincpu"),
		  m_bank1(*this, "bank1"), m_bank2(*this, "bank2"), m_bank3(*this, "bank3"), m_bank4(*this, "bank4"),
		  m_keyboard(*this, "pc_keyboard"),
		  m_beta(*this, BETA_DISK_TAG),
		  m_dma(*this, "dma"),
		  m_sdcard(*this, "sdcard"),
		  m_glukrs(*this, "glukrs"),
		  m_palette(*this, "palette"),
		  m_gfxdecode(*this, "gfxdecode"),
		  m_cram(*this, "cram")
	{
	}

	void tsconf(machine_config &config);

protected:
	virtual void video_start() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	enum gluk_ext : u8
	{
		CONF_VERSION = 0x00,
		BOOTLOADER_VERSION = 0x01,
		PS2KEYBOARDS_LOG = 0x02,
		RDCFG = 0x03,
		CONFIG = 0x0e,
		SPIFL = 0x10,

		DISABLED = 0xff
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
		INT_MASK = 0x2a,
		DMA_NUM_H = 0x2c,

		T0_X_OFFSER_L = 0x40,
		T0_X_OFFSER_H = 0x41,
		T0_Y_OFFSER_L = 0x42,
		T0_Y_OFFSER_H = 0x43,
		T1_X_OFFSER_L = 0x44,
		T1_X_OFFSER_H = 0x45,
		T1_Y_OFFSER_L = 0x46,
		T1_Y_OFFSER_H = 0x47
	};

	DECLARE_VIDEO_START(tsconf);
	TILE_GET_INFO_MEMBER(get_tile_info_txt);
	template <u8 Layer>
	TILE_GET_INFO_MEMBER(get_tile_info_16c);
	TILE_GET_INFO_MEMBER(get_sprite_info_16c);

	// Changing this consider to revert 'virtual' in spectrum.h
	u32 screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	void spectrum_UpdateScreenBitmap(bool eof = false) override;
	void tsconf_palette(palette_device &palette) const;
	u16 get_border_color() override;
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tsconf_update_video_mode();
	rectangle get_resolution_info();

	void tsconf_port_7ffd_w(u8 data);
	void tsconf_port_fe_w(offs_t offset, u8 data);
	u8 tsconf_port_xxaf_r(offs_t reg);
	void tsconf_port_xxaf_w(offs_t reg, u8 data);
	u8 tsconf_port_77_zctr_r(offs_t reg);
	void tsconf_port_77_zctr_w(offs_t reg, u8 data);
	u8 tsconf_port_57_zctr_r(offs_t reg);
	void tsconf_port_57_zctr_w(offs_t reg, u8 data);
	void tsconf_spi_miso_w(u8 data);
	u8 tsconf_port_f7_r(offs_t offset);
	void tsconf_port_f7_w(offs_t offset, u8 data);

	void tsconf_update_bank1();
	u8 beta_neutral_r(offs_t offset);
	u8 beta_enable_r(offs_t offset);
	u8 beta_disable_r(offs_t offset);

	void tsconf_io(address_map &map);
	void tsconf_mem(address_map &map);
	void tsconf_switch(address_map &map);

	template <unsigned Bank>
	void tsconf_bank_w(offs_t offset, u8 data);
	void ram_bank_write(u8 bank, offs_t offset, u8 data);
	void ram_page_write(u8 page, offs_t offset, u8 data);
	void cram_write(u16 offset, u8 data);
	void cram_write16(offs_t offset, u16 data);
	void ram_write16(offs_t offset, u16 data);
	u16 ram_read16(offs_t offset);
	u16 spi_read16();

	u8 m_regs[0x100];

	address_space *m_program;
	required_region_ptr<u8> m_p_rom;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;

	required_device<at_keyboard_device> m_keyboard;

	required_device<beta_disk_device> m_beta;
	required_device<tsconfdma_device> m_dma;
	required_device<spi_sdcard_sdhc_device> m_sdcard;
	u8 m_zctl_di;
	u8 m_zctl_cs;

	required_device<glukrs_device> m_glukrs;
	gluk_ext m_port_f7_ext;
	u8 m_port_f7_gluk_reg;

	optional_device<device_palette_interface> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	tilemap_t *m_ts_tilemap[4];
	required_device<ram_device> m_cram;
};

/*----------- defined in drivers/tsconf.c -----------*/

INPUT_PORTS_EXTERN(tsconf);

#endif // MAME_INCLUDES_TSCONF_H
