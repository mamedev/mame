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

#include "machine/beta.h"
#include "machine/pckeybrd.h"
#include "machine/spi_sdcard.h"
#include "machine/tsconfdma.h"
#include "machine/nvram.h"
#include "spectrum.h"
#include "tilemap.h"

#define PAGE(_r) ((_r) << 14)
#define RAM_PAGE_OFFST(_page, _offset) (PAGE(_page) + _offset)

#define REG(_p) m_regs[_p]
#define REGNUM(_r) (&(_r) - &(V_CONFIG))

#define V_CONFIG REG(0x00)
#define V_PAGE REG(0x01)

#define G_X_OFFS_L REG(0x02)
#define G_X_OFFS_H REG(0x03)
#define G_Y_OFFS_L REG(0x04)
#define G_Y_OFFS_H REG(0x05)

#define TS_CONFIG REG(0x06)
#define PAL_SEL REG(0x07)
#define BORDER REG(0x0f)

#define PAGE0 REG(0x10)
#define PAGE1 REG(0x11)
#define PAGE2 REG(0x12)
#define PAGE3 REG(0x13)
#define FMAPS REG(0x15)

#define T_MAP_PAGE REG(0x16)
#define T0_G_PAGE REG(0x17)
#define T1_G_PAGE REG(0x18)
#define SG_PAGE REG(0x19)

#define DMAS_ADDRESS_L REG(0x1a)
#define DMAS_ADDRESS_H REG(0x1b)
#define DMAS_ADDRESS_X REG(0x1c)
#define DMAD_ADDRESS_L REG(0x1d)
#define DMAD_ADDRESS_H REG(0x1e)
#define DMAD_ADDRESS_X REG(0x1f)
#define DMA_WAIT_PORT_DEV REG(0x25)
#define DMA_LEN REG(0x26)
#define DMA_CTRL REG(0x27)
#define DMA_NUM_L REG(0x28)
#define DMA_NUM_H REG(0x2c)

#define SYS_CONFIG REG(0x20)
#define MEM_CONFIG REG(0x21)
#define HS_INT REG(0x22)
#define VS_INT_L REG(0x23)
#define VS_INT_H REG(0x24)

#define INT_MASK REG(0x2a)

#define T0_X_OFFSER_L REG(0x40)
#define T0_X_OFFSER_H REG(0x41)
#define T0_Y_OFFSER_L REG(0x42)
#define T0_Y_OFFSER_H REG(0x43)
#define T1_X_OFFSER_L REG(0x44)
#define T1_X_OFFSER_H REG(0x45)
#define T1_Y_OFFSER_L REG(0x46)
#define T1_Y_OFFSER_H REG(0x47)

class tsconf_state : public spectrum_128_state
{
public:
	tsconf_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag),
		  m_bank1(*this, "bank1"), m_bank2(*this, "bank2"), m_bank3(*this, "bank3"), m_bank4(*this, "bank4"),
		  m_keyboard(*this, "pc_keyboard"),
		  m_beta(*this, BETA_DISK_TAG),
		  m_sdcard(*this, "sdcard"),
		  m_dma(*this, "dma"), m_cmos(*this, "cmos"), m_glukrs(*this, "glukrs"),
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
	DECLARE_VIDEO_START(tsconf);

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

	void tsconf_port_7ffd_w(u8 data);
	void tsconf_port_fe_w(offs_t offset, u8 data);

	u8 tsconf_port_xxaf_r(offs_t reg);
	void tsconf_port_xxaf_w(offs_t reg, u8 data);

	u8 tsconf_port_77_zctr_r(offs_t reg);
	void tsconf_port_77_zctr_w(offs_t reg, u8 data);
	u8 tsconf_port_57_zctr_r(offs_t reg);
	void tsconf_port_57_zctr_w(offs_t reg, u8 data);
	void tsconf_spi_miso_w(u8 data);

	void tsconf_update_bank1();
	u8 beta_neutral_r(offs_t offset);
	u8 beta_enable_r(offs_t offset);
	u8 beta_disable_r(offs_t offset);
	template <unsigned Bank> void tsconf_bank_w(offs_t offset, u8 data);
	void ram_bank_write(u8 bank, offs_t offset, u8 data);
	void ram_page_write(u8 page, offs_t offset, u8 data);

	void tsconf_io(address_map &map);
	void tsconf_mem(address_map &map);
	void tsconf_switch(address_map &map);

	u16 ram_read16(offs_t offset);
	void ram_write16(offs_t offset, u16 data);
	u16 spi_read16();

	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;

	required_device<at_keyboard_device> m_keyboard;

	required_device<beta_disk_device> m_beta;
	required_device<spi_sdcard_sdhc_device> m_sdcard;
	required_device<tsconfdma_device> m_dma;

	required_device<ram_device> m_cmos;
	required_device<nvram_device> m_glukrs;
	u8 tsconf_port_f7_cmos_r(offs_t offset);
	void tsconf_port_f7_cmos_w(offs_t offset, u8 data);
	gluk_ext m_gluk_ext;
	u8 m_gluk_reg;

	optional_device<device_palette_interface> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	tilemap_t *m_ts_tilemap[3];
	required_device<ram_device> m_cram;
	void cram_write(u16 offset, u8 data);
	void cram_write16(offs_t offset, u16 data);

	// Changing this consider to revert 'virtual' in spectrum.h
	u32 screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	void spectrum_UpdateBorderBitmap() override;
	void spectrum_UpdateScreenBitmap(bool eof = false) override;

	TILE_GET_INFO_MEMBER(get_tile_info_txt);
	template <u8 Layer> TILE_GET_INFO_MEMBER(get_tile_info_16c);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	address_space *m_program;
	u8 *m_p_rom;
	void tsconf_update_video_mode();
	u8 m_regs[0x100];

	u8 m_zctl_di;
	u8 m_zctl_cs;
};

/*----------- defined in drivers/tsconf.c -----------*/

INPUT_PORTS_EXTERN(tsconf);

#endif // MAME_INCLUDES_TSCONF_H
