// license:BSD-3-Clause
// copyright-holders:Ville Linde, hap, Nicola Salmoria
/******************************************************************************

    Seibu SPI hardware

******************************************************************************/
#ifndef MAME_SEIBU_SEIBUSPI_H
#define MAME_SEIBU_SEIBUSPI_H

#pragma once

#include "sei25x_rise1x_spr.h"

#include "machine/7200fifo.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"

#include "sound/okim6295.h"

#include "emupal.h"
#include "tilemap.h"

// base state class (sprite only)
class seibuspi_base_state : public driver_device
{
protected:
	seibuspi_base_state(const machine_config &mconfig, device_type type, const char *tag, size_t paletteram_size, size_t spriteram_size, u32 sprite_bpp)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainram(*this, "mainram")
		, m_eeprom(*this, "eeprom")
		, m_palette(*this, "palette")
		, m_spritegen(*this, "spritegen")
		, m_palette_ram(*this, "palette_ram", paletteram_size, ENDIANNESS_LITTLE)
		, m_sprite_ram(*this, "sprite_ram", spriteram_size, ENDIANNESS_LITTLE)
		, m_key(*this, "KEY%u", 0)
		, m_special(*this, "SPECIAL")
		, m_sprite_bpp(sprite_bpp)
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u32> m_mainram;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<palette_device> m_palette;
	required_device<sei25x_rise1x_device> m_spritegen;

	memory_share_creator<u32> m_palette_ram;
	memory_share_creator<u16> m_sprite_ram;

	optional_ioport_array<5> m_key;
	optional_ioport m_special;

	const u32 m_sprite_bpp;

	u32 m_video_dma_length = 0;
	u32 m_video_dma_address = 0;
	u16 m_layer_enable = 0;
	u8 m_alpha_table[0x2000]{};

	virtual void video_start() override ATTR_COLD;

	void palette_dma_start_w(u32 data);
	void sprite_dma_start_w(u16 data);
	void video_dma_length_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void video_dma_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u8 spi_status_r();
	void eeprom_w(u8 data);

	u32 screen_update_sys386f(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	IRQ_CALLBACK_MEMBER(irq_callback);
	INTERRUPT_GEN_MEMBER(interrupt);

	void rise_map(address_map &map) ATTR_COLD;
};

// with mahjong input, YMZ280B sound
class sys386f_state : public seibuspi_base_state
{
public:
	sys386f_state(const machine_config &mconfig, device_type type, const char *tag)
		: seibuspi_base_state(mconfig, type, tag, 0x4000, 0x2000, 8)
	{ }

	void sys386f(machine_config &config) ATTR_COLD;

	void init_sys386f() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	u8 m_input_select = 0;

	u32 input_mux_r();
	void input_select_w(u32 data);

	void sys386f_map(address_map &map) ATTR_COLD;
};

// with tilemap
class seibuspi_tilemap_state : public seibuspi_base_state
{
public:
	seibuspi_tilemap_state(const machine_config &mconfig, device_type type, const char *tag)
		: seibuspi_base_state(mconfig, type, tag, 0x3000, 0x1000, 6)
		, m_gfxdecode(*this, "gfxdecode")
		, m_tilemap_ram(*this, "tilemap_ram", 0x4000, ENDIANNESS_LITTLE)
	{ }

	void init_rdft22kc() ATTR_COLD;
	void init_rfjet2kc() ATTR_COLD;

protected:
	required_device<gfxdecode_device> m_gfxdecode;
	memory_share_creator<u32> m_tilemap_ram;
	bitmap_ind16 m_sprite_bitmap;

	tilemap_t *m_text_layer = nullptr;
	tilemap_t *m_back_layer = nullptr;
	tilemap_t *m_midl_layer = nullptr;
	tilemap_t *m_fore_layer = nullptr;
	u16 m_layer_bank = 0;
	u8 m_rf2_layer_bank = 0;
	u16 m_scrollram[6]{};
	bool m_rowscroll_enable = false;
	u32 m_midl_layer_offset = 0;
	u32 m_fore_layer_offset = 0;
	u32 m_text_layer_offset = 0;
	u32 m_fore_layer_d13 = 0;
	u32 m_back_layer_d14 = 0;
	u32 m_midl_layer_d14 = 0;
	u32 m_fore_layer_d14 = 0;
	u32 m_bg_fore_layer_position = 0;

	virtual void video_start() override ATTR_COLD;

	void tile_decrypt_key_w(u16 data);
	void layer_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void layer_enable_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void rf2_layer_bank_w(u8 data);
	void scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tilemap_dma_start_w(u32 data);
	void spi_layerbanks_eeprom_w(u8 data);

	u32 rf2_speedup_r();
	u32 rfjet_speedup_r();

	void set_layer_offsets();
	void blend_pixel(u32 &dest, u16 pen);
	void blend_sprite(bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri);
	void combine_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tile, int sx, int sy, int opaque, s16 *rowscroll);
	u32 gfxbank_callback(u32 code, u8 ext);

	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_midl_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	u32 screen_update_spi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void rdft2_text_decrypt(u8 *rom) ATTR_COLD;
	void rdft2_bg_decrypt(u8 *rom, int size) ATTR_COLD;

	void rfjet_text_decrypt(u8 *rom) ATTR_COLD;
	void rfjet_bg_decrypt(u8 *rom, int size) ATTR_COLD;

	void base_video(machine_config &config) ATTR_COLD;

	void base_map(address_map &map) ATTR_COLD;
};

// single board with OKI MSM6295 sound subsystem
class sys386i_state : public seibuspi_tilemap_state
{
public:
	sys386i_state(const machine_config &mconfig, device_type type, const char *tag)
		: seibuspi_tilemap_state(mconfig, type, tag)
		, m_oki(*this, "oki%u", 1)
	{ }

	void sys386i(machine_config &config) ATTR_COLD;

private:
	required_device_array<okim6295_device, 2> m_oki;

	void oki_bank_w(u8 data);

	void sys386i_map(address_map &map) ATTR_COLD;
};

// with Z80 + YMF271 based sound subsystem
class seibuspi_z80_state : public seibuspi_tilemap_state
{
public:
	seibuspi_z80_state(const machine_config &mconfig, device_type type, const char *tag)
		: seibuspi_tilemap_state(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_soundfifo(*this, "soundfifo%u", 1)
		, m_z80_rom(*this, "audiocpu")
		, m_z80_bank(*this, "z80_bank")
	{ }

	void sxx2e(machine_config &config) ATTR_COLD;
	void sxx2f(machine_config &config) ATTR_COLD;
	void sxx2g(machine_config &config) ATTR_COLD;

	void init_rdft() ATTR_COLD;
	void init_rdft2() ATTR_COLD;
	void init_rfjet() ATTR_COLD;
	void init_sei252() ATTR_COLD;

protected:
	required_device<cpu_device> m_audiocpu;
	optional_device_array<fifo7200_device, 2> m_soundfifo;

	required_memory_region m_z80_rom;
	required_memory_bank m_z80_bank;

	s32 m_z80_lastbank = 0;
	u8 m_sb_coin_latch = 0;

	u8 spi_ds2404_unknown_r();
	u8 sb_coin_r();
	void spi_coin_w(u8 data);
	u8 sound_fifo_status_r();
	u8 z80_soundfifo_status_r();
	void z80_bank_w(u8 data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u32 rdft_speedup_r();

	void ymf_irqhandler(int state);

	void init_spi_common() ATTR_COLD;

	void text_decrypt(u8 *rom) ATTR_COLD;
	void bg_decrypt(u8 *rom, int size) ATTR_COLD;

	void sei252_map(address_map &map) ATTR_COLD;
	void sxx2e_map(address_map &map) ATTR_COLD;
	void sxx2e_soundmap(address_map &map) ATTR_COLD;
	void sxx2f_map(address_map &map) ATTR_COLD;
};

// with interchangeable cartridge, flash ROM for sound samples
class seibuspi_state : public seibuspi_z80_state
{
public:
	seibuspi_state(const machine_config &mconfig, device_type type, const char *tag)
		: seibuspi_z80_state(mconfig, type, tag)
		, m_soundflash(*this, "soundflash%u", 1U)
		, m_soundflash1_region(*this, "soundflash1")
	{ }

	void ejanhs(machine_config &config) ATTR_COLD;
	void rdft2(machine_config &config) ATTR_COLD;
	void spi(machine_config &config) ATTR_COLD;

	void init_batlball() ATTR_COLD;
	void init_ejanhs() ATTR_COLD;
	void init_senkyu() ATTR_COLD;
	void init_senkyua() ATTR_COLD;
	void init_viprp1() ATTR_COLD;
	void init_viprp1o() ATTR_COLD;

	template <int N> ioport_value ejanhs_encode();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device_array<intel_e28f008sa_device, 2> m_soundflash;

	optional_region_ptr<u8> m_soundflash1_region;

	u32 m_z80_prg_transfer_pos = 0;

	void z80_prg_transfer_w(u8 data);
	void z80_enable_w(u8 data);

	u32 senkyu_speedup_r();
	u32 senkyua_speedup_r();
	u32 batlball_speedup_r();
	u32 viprp1_speedup_r();
	u32 viprp1o_speedup_r();
	u32 ejanhs_speedup_r();

	DECLARE_VIDEO_START(ejanhs);

	void rdft2_map(address_map &map) ATTR_COLD;
	void spi_map(address_map &map) ATTR_COLD;
	void spi_soundmap(address_map &map) ATTR_COLD;
	void spi_ymf271_map(address_map &map) ATTR_COLD;
};

#endif // MAME_SEIBU_SEIBUSPI_H
