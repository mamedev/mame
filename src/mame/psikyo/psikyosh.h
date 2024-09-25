// license:BSD-3-Clause
// copyright-holders:David Haywood, Paul Priest
#ifndef MAME_PSIKYO_PSIKYOSH_H
#define MAME_PSIKYO_PSIKYOSH_H

#pragma once

#include "machine/eepromser.h"
#include "cpu/sh/sh7604.h"
#include "emupal.h"
#include "screen.h"


#define MASTER_CLOCK 57272700   // main oscillator frequency

/* Psikyo PS6406B */



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
	void s1945iiibl(machine_config &config);
	void s1945iiibla(machine_config &config);

	void init_ps3();
	void init_ps5();
	void init_s1945iiibl();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<u32> m_spriteram;
	required_shared_ptr<u32> m_zoomram;
	required_shared_ptr<u32> m_vidregs;
	required_shared_ptr<u32> m_ram;

	required_memory_bank m_gfxrombank;

	optional_ioport m_controller_io;
	optional_ioport m_inputs;
	optional_ioport m_mahjong_io;

	/* video-related */
	struct sprite_t
	{
		s32 ypos = 0, xpos = 0;
		u8 high = 0, wide = 0;
		u8 flpy = 0, flpx = 0;
		u8 spr_pri = 0, bg_pri = 0;
		u8 zoomy = 0, zoomx = 0;
		u32 tnum = 0;
		u16 colr = 0;
		u8 dpth = 0;
		s16 alpha = 0;
	};

	bitmap_ind8                 m_zoom_bitmap;
	bitmap_ind16                m_z_bitmap;
	bitmap_rgb32                m_bg_bitmap;
	std::unique_ptr<u16[]>      m_bg_zoom;
	std::unique_ptr<u8[]>       m_alphatable;
	std::unique_ptr<struct sprite_t []> m_spritelist;
	const struct sprite_t *m_sprite_end;

	/* devices */
	required_device<sh7604_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	bool const FLIPSCREEN() { return ((m_vidregs[3] & 0x0000c000) == 0x0000c000); } // currently ignored

	bool const BG_LARGE(u8 const n)        { return ((m_vidregs[7] << (4 * n)) & 0x00001000); }
	bool const BG_DEPTH_8BPP(u8 const n)   { return ((m_vidregs[7] << (4 * n)) & 0x00004000); }
	bool const BG_LAYER_ENABLE(u8 const n) { return ((m_vidregs[7] << (4 * n)) & 0x00008000); }

	u8 const BG_TYPE(u8 const n) { return ((m_vidregs[6] << (8 * n)) & 0x7f000000) >> 24; }
	bool const BG_LINE(u8 const n)    { return ((m_vidregs[6] << (8 * n)) & 0x80000000); }

	u8 const SPRITE_PRI(u8 const n) { return ((m_vidregs[2] << (4 * n)) & 0xf0000000) >> 28; }

	void irqctrl_w(u32 data);
	void vidregs_w(offs_t offset, u32 data, u32 mem_mask);
	u32 mjgtaste_input_r();
	void eeprom_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	INTERRUPT_GEN_MEMBER(interrupt);
	void draw_scanline32_alpha(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, int alpha);
	void draw_scanline32_argb(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr);
	void draw_scanline32_transpen(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr);
	void draw_bglayer(u8 const layer, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 const req_pri);
	void cache_bitmap(s16 const scanline, gfx_element *gfx, u8 const size, u8 const tilebank, s16 const alpha, u8 *last_bank);
	void draw_bglayerscroll(u8 const layer, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 const req_pri);
	void draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 const req_pri);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 const req_pri);
	void get_sprites();
	void prelineblend(bitmap_rgb32 &bitmap, const rectangle &cliprect );
	void postlineblend(bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 const req_pri);
	void psikyosh_drawgfxzoom(bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx,
	u32 const code, u16 const color, u8 const flipx, u8 const flipy, s32 const offsx, s32 const offsy,
	s16 const alpha, u32 const zoomx, u32 const zoomy, u8 const wide, u8 const high, u16 const z);
	void ps3v1_map(address_map &map) ATTR_COLD;
	void ps5_map(address_map &map) ATTR_COLD;
	void ps5_mahjong_map(address_map &map) ATTR_COLD;
	void s1945iiibl_map(address_map &map) ATTR_COLD;
	void s1945iiibla_map(address_map &map) ATTR_COLD;
};

#endif // MAME_PSIKYO_PSIKYOSH_H
