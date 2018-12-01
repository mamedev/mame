// license:BSD-3-Clause
// copyright-holders:David Haywood, Paul Priest
#ifndef MAME_INCLUDES_PSIKYOSH_H
#define MAME_INCLUDES_PSIKYOSH_H

#pragma once

#include "video/bufsprite.h"
#include "machine/eepromser.h"
#include "cpu/sh/sh2.h"
#include "emupal.h"
#include "screen.h"


#define MASTER_CLOCK 57272700   // main oscillator frequency

/* Psikyo PS6406B */
#define FLIPSCREEN (((m_vidregs[3] & 0x0000c000) == 0x0000c000) ? 1:0)
#define DISPLAY_DISABLE (((m_vidregs[2] & 0x0000000f) == 0x00000006) ? 1:0)
#define BG_LARGE(n) (((m_vidregs[7] << (4*n)) & 0x00001000 ) ? 1:0)
#define BG_DEPTH_8BPP(n) (((m_vidregs[7] << (4*n)) & 0x00004000 ) ? 1:0)
#define BG_LAYER_ENABLE(n) (((m_vidregs[7] << (4*n)) & 0x00008000 ) ? 1:0)

#define BG_TYPE(n) (((m_vidregs[6] << (8*n)) & 0x7f000000 ) >> 24)
#define BG_LINE(n) (((m_vidregs[6] << (8*n)) & 0x80000000 ) ? 1:0)

#define BG_TRANSPEN rgb_t(0x00,0xff,0x00,0xff) // used for representing transparency in temporary bitmaps

#define SPRITE_PRI(n) (((m_vidregs[2] << (4*n)) & 0xf0000000 ) >> 28)


class psikyosh_state : public driver_device
{
public:
	psikyosh_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_zoomram(*this, "zoomram"),
		m_vidregs(*this, "vidregs"),
		m_ram(*this, "ram"),
		m_gfxrombank(*this, "gfxbank"),
		m_controller_io(*this, "CONTROLLER"),
		m_inputs(*this, "INPUTS"),
		m_mahjong_io(*this, "MAHJONG"),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void psikyo3v1(machine_config &config);
	void psikyo5(machine_config &config);
	void psikyo5_mahjong(machine_config &config);
	void psikyo5_240(machine_config &config);

	void init_ps3();
	void init_ps5();

private:
	/* memory pointers */
	required_device<buffered_spriteram32_device> m_spriteram;
	required_shared_ptr<uint32_t> m_zoomram;
	required_shared_ptr<uint32_t> m_vidregs;
	required_shared_ptr<uint32_t> m_ram;

	required_memory_bank m_gfxrombank;

	optional_ioport m_controller_io;
	optional_ioport m_inputs;
	optional_ioport m_mahjong_io;

	/* video-related */
	bitmap_ind8                 m_zoom_bitmap;
	bitmap_ind16                m_z_bitmap;
	bitmap_rgb32                m_bg_bitmap;
	std::unique_ptr<uint16_t[]> m_bg_zoom;
	std::unique_ptr<uint8_t[]>  m_alphatable;

	/* devices */
	required_device<sh2_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE32_MEMBER(psikyosh_irqctrl_w);
	DECLARE_WRITE32_MEMBER(vidregs_w);
	DECLARE_READ32_MEMBER(mjgtaste_input_r);
	DECLARE_WRITE8_MEMBER(eeprom_w);
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void draw_scanline32_alpha(bitmap_rgb32 &bitmap, int32_t destx, int32_t desty, int32_t length, const uint32_t *srcptr, int alpha);
	void draw_scanline32_argb(bitmap_rgb32 &bitmap, int32_t destx, int32_t desty, int32_t length, const uint32_t *srcptr);
	void draw_scanline32_transpen(bitmap_rgb32 &bitmap, int32_t destx, int32_t desty, int32_t length, const uint32_t *srcptr);
	void draw_bglayer(uint8_t const layer, bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t const req_pri);
	void cache_bitmap(int16_t const scanline, gfx_element *gfx, uint8_t const size, uint8_t const tilebank, int16_t const alpha, uint8_t *last_bank);
	void draw_bglayerscroll(uint8_t const layer, bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t const req_pri);
	void draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t const req_pri);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t const req_pri);
	void psikyosh_prelineblend(bitmap_rgb32 &bitmap, const rectangle &cliprect );
	void psikyosh_postlineblend(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t const req_pri);
	void psikyosh_drawgfxzoom(bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx,
	uint32_t const code, uint16_t const color, uint8_t const flipx, uint8_t const flipy, int16_t const offsx, int16_t const offsy,
	int16_t const alpha, uint32_t const zoomx, uint32_t const zoomy, uint8_t const wide, uint8_t const high, uint16_t const z);
	void ps3v1_map(address_map &map);
	void ps5_map(address_map &map);
	void ps5_mahjong_map(address_map &map);
};

#endif // MAME_INCLUDES_PSIKYOSH_H
