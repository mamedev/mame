// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/arcadia.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_ARCADIA_H
#define MAME_INCLUDES_ARCADIA_H

#pragma once

#include "cpu/s2650/s2650.h"
#include "audio/arcadia.h"

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
	DECLARE_READ_LINE_MEMBER(vsync_r);
	DECLARE_READ8_MEMBER(video_r);
	DECLARE_WRITE8_MEMBER(video_w);

	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init(palette_device &palette) const;
	uint32_t screen_update_arcadia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(video_line);
	void arcadia_mem(address_map &map);

	void draw_char(uint8_t *ch, int charcode, int y, int x);
	void vh_draw_line(int y, uint8_t chars1[16]);
	int sprite_collision(int n1, int n2);
	void draw_sprites();

private:
	int m_line;
	int m_charline;
	int m_shift;
	int m_ad_delay;
	int m_ad_select;
	int m_ypos;
	int m_graphics;
	int m_doublescan;
	int m_lines26;
	int m_multicolor;
	struct { int x, y; } m_pos[4];
	uint8_t m_bg[262][16+2*XPOS/8];
	uint8_t m_rectangle[0x40][8];
	uint8_t m_chars[0x40][8];
	int m_breaker;
	union
	{
		uint8_t data[0x400];
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
};

#endif // MAME_INCLUDES_ARCADIA_H
