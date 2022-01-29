// license:BSD-3-Clause
// copyright-holders:Hau
#ifndef MAME_INCLUDES_GALASTRM_H
#define MAME_INCLUDES_GALASTRM_H

#pragma once

#include "machine/eepromser.h"

#include "video/poly.h"
#include "video/tc0100scn.h"
#include "video/tc0110pcr.h"
#include "video/tc0480scp.h"

#include "emupal.h"
#include "screen.h"


class galastrm_state;

struct gs_poly_data
{
	bitmap_ind16* texbase;
};

class galastrm_renderer : public poly_manager<float, gs_poly_data, 2>
{
public:
	galastrm_renderer(galastrm_state &state);

	void tc0610_draw_scanline(s32 scanline, const extent_t& extent, const gs_poly_data& object, int threadid);
	void tc0610_rotate_draw(bitmap_ind16 &srcbitmap, const rectangle &clip);

	bitmap_ind16 &screenbits() { return m_screenbits; }

private:
	galastrm_state& m_state;
	bitmap_ind16 m_screenbits;
};


class galastrm_state : public driver_device
{
	friend class galastrm_renderer;
public:
	galastrm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this,"spriteram"),
		m_spritemap_rom(*this, "sprmaprom"),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0110pcr(*this, "tc0110pcr"),
		m_tc0480scp(*this, "tc0480scp"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen")
	{ }

	void galastrm(machine_config &config);
	DECLARE_READ_LINE_MEMBER(frame_counter_r);

protected:
	virtual void video_start() override;

private:
	required_shared_ptr<u32> m_spriteram;

	required_region_ptr<u16> m_spritemap_rom;

	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0110pcr_device> m_tc0110pcr;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;

	struct gs_tempsprite
	{
		u8 gfx;
		u32 code,color;
		bool flipx,flipy;
		int x,y;
		int zoomx,zoomy;
		u32 primask;
	};

	u16 m_frame_counter;
	int m_tc0610_addr[2];
	s16 m_tc0610_ctrl_reg[2][8];
	std::unique_ptr<gs_tempsprite[]> m_spritelist;
	struct gs_tempsprite *m_sprite_ptr_pre;
	bitmap_ind16 m_tmpbitmaps;
	std::unique_ptr<galastrm_renderer> m_poly;

	int m_rsxb;
	int m_rsyb;
	int m_rsxoffs;
	int m_rsyoffs;

	template<int Chip> void tc0610_w(offs_t offset, u16 data);
	void coin_word_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void draw_sprites_pre(int x_offs, int y_offs);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const u32 *primasks, int priority);

	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_GALASTRM_H
