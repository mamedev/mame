// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood
#ifndef MAME_INCLUDES_TUNHUNT_H
#define MAME_INCLUDES_TUNHUNT_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class tunhunt_state : public driver_device
{
public:
	tunhunt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_workram(*this, "workram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_generic_paletteram_8(*this, "paletteram"),
		m_led(*this, "led0")
	{ }

	void tunhunt(machine_config &config);

private:
	void control_w(uint8_t data);
	uint8_t button_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	uint8_t dsw2_0r();
	uint8_t dsw2_1r();
	uint8_t dsw2_2r();
	uint8_t dsw2_3r();
	uint8_t dsw2_4r();

	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	virtual void machine_start() override { m_led.resolve(); }
	virtual void video_start() override;
	void tunhunt_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_pens();
	void draw_motion_object(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_shell(bitmap_ind16 &bitmap, const rectangle &cliprect, int picture_code,
	int hposition,int vstart,int vstop,int vstretch,int hstretch);
	void main_map(address_map &map);
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_workram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;
	output_finder<> m_led;

	uint8_t m_control;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_tmpbitmap;

	uint8_t m_mobsc0;
	uint8_t m_mobsc1;
	uint8_t m_lineh[13];
	uint8_t m_shl0st;
	uint8_t m_shl1st;
	uint8_t m_vstrlo;
	uint8_t m_linesh;
	uint8_t m_shl0pc;
	uint8_t m_shl1pc;
	uint8_t m_linec[13];
	uint8_t m_shl0v;
	uint8_t m_shl1v;
	uint8_t m_mobjh;
	uint8_t m_linev[13];
	uint8_t m_shl0vs;
	uint8_t m_shl1vs;
	uint8_t m_mobvs;
	uint8_t m_linevs[13];
	uint8_t m_shel0h;
	uint8_t m_mobst;
	uint8_t m_shel1h;
	uint8_t m_mobjv;
};

#endif // MAME_INCLUDES_TUNHUNT_H
