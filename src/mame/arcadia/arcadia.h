// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/arcadia.h
 *
 ****************************************************************************/
#ifndef MAME_ARCADIA_ARCADIA_H
#define MAME_ARCADIA_ARCADIA_H

#pragma once

#include "cpu/s2650/s2650.h"
#include "arcadia_a.h"

#include "bus/arcadia/slot.h"
#include "bus/arcadia/rom.h"

#include "emupal.h"
#include "screen.h"

// space vultures sprites above
// combat below and invisible
#define YPOS 0
//#define YBOTTOM_SIZE 24
// grand slam sprites left and right
// space vultures left
// space attack left
#define XPOS 32


class arcadia_state : public driver_device
{
public:
	arcadia_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_custom(*this, "custom"),
		m_panel(*this, "panel"),
		m_controller1_col1(*this, "controller1_col1"),
		m_controller1_col2(*this, "controller1_col2"),
		m_controller1_col3(*this, "controller1_col3"),
		m_controller1_extra(*this, "controller1_extra"),
		m_controller2_col1(*this, "controller2_col1"),
		m_controller2_col2(*this, "controller2_col2"),
		m_controller2_col3(*this, "controller2_col3"),
		m_controller2_extra(*this, "controller2_extra"),
		m_joysticks(*this, "joysticks") ,
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	void init_arcadia();
	void arcadia(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	int m_line = 0;
	int m_charline = 0;
	int m_shift = 0;
	int m_ad_delay = 0;
	int m_ad_select = 0;
	int m_ypos = 0;
	int m_graphics = 0;
	int m_doublescan = 0;
	int m_lines26 = 0;
	int m_multicolor = 0;
	struct { int x = 0, y = 0; } m_pos[4];
	uint8_t m_bg[262][16+2*XPOS/8]{};
	uint8_t m_rectangle[0x40][8]{};
	uint8_t m_chars[0x40][8]{};
	int m_breaker = 0;
	union
	{
		uint8_t data[0x400]{};
		struct
		{
			// 0x1800
			uint8_t chars1[13][16];
			uint8_t ram1[2][16];
			struct  { uint8_t y,x; } pos[4];
			uint8_t ram2[4];
			uint8_t vpos;
			uint8_t sound1, sound2;
			uint8_t char_line;
			// 0x1900
			uint8_t pad1a, pad1b, pad1c, pad1d;
			uint8_t pad2a, pad2b, pad2c, pad2d;
			uint8_t keys, unmapped3[0x80-9];
			uint8_t chars[8][8];
			uint8_t unknown[0x38];
			uint8_t pal[4];
			uint8_t collision_bg,
			collision_sprite;
			uint8_t ad[2];
			// 0x1a00
			uint8_t chars2[13][16];
			uint8_t ram3[3][16];
		} d;
	} m_reg;
	std::unique_ptr<bitmap_ind16> m_bitmap;

	required_device<arcadia_sound_device> m_custom;
	required_ioport m_panel;
	required_ioport m_controller1_col1;
	required_ioport m_controller1_col2;
	required_ioport m_controller1_col3;
	required_ioport m_controller1_extra;
	required_ioport m_controller2_col1;
	required_ioport m_controller2_col2;
	required_ioport m_controller2_col3;
	required_ioport m_controller2_extra;
	required_ioport m_joysticks;

	required_device<s2650_device> m_maincpu;
	required_device<arcadia_cart_slot_device> m_cart;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	int vsync_r();
	uint8_t video_r(offs_t offset);
	void video_w(offs_t offset, uint8_t data);
	void palette_init(palette_device &palette) const;
	uint32_t screen_update_arcadia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(video_line);
	void arcadia_mem(address_map &map) ATTR_COLD;

	void draw_char(uint8_t *ch, int charcode, int y, int x);
	void vh_draw_line(int y, uint8_t chars1[16]);
	int sprite_collision(int n1, int n2);
	void draw_sprites();
};

#endif // MAME_ARCADIA_ARCADIA_H
