// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/arcadia.h
 *
 ****************************************************************************/

#ifndef ARCADIA_H_
#define ARCADIA_H_

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "audio/arcadia.h"

#include "bus/arcadia/slot.h"
#include "bus/arcadia/rom.h"

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
	arcadia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
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
		m_screen(*this, "screen")  { }

	DECLARE_READ8_MEMBER(vsync_r);
	DECLARE_READ8_MEMBER(video_r);
	DECLARE_WRITE8_MEMBER(video_w);
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
	UINT8 m_bg[262][16+2*XPOS/8];
	UINT8 m_rectangle[0x40][8];
	UINT8 m_chars[0x40][8];
	int m_breaker;
	union
	{
		UINT8 data[0x400];
		struct
		{
			// 0x1800
			UINT8 chars1[13][16];
			UINT8 ram1[2][16];
			struct  { UINT8 y,x; } pos[4];
			UINT8 ram2[4];
			UINT8 vpos;
			UINT8 sound1, sound2;
			UINT8 char_line;
			// 0x1900
			UINT8 pad1a, pad1b, pad1c, pad1d;
			UINT8 pad2a, pad2b, pad2c, pad2d;
			UINT8 keys, unmapped3[0x80-9];
			UINT8 chars[8][8];
			UINT8 unknown[0x38];
			UINT8 pal[4];
			UINT8 collision_bg,
			collision_sprite;
			UINT8 ad[2];
			// 0x1a00
			UINT8 chars2[13][16];
			UINT8 ram3[3][16];
		} d;
	} m_reg;
	std::unique_ptr<bitmap_ind16> m_bitmap;
	DECLARE_DRIVER_INIT(arcadia);
	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(arcadia);
	UINT32 screen_update_arcadia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(video_line);

protected:
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

	void draw_char(UINT8 *ch, int charcode, int y, int x);
	void vh_draw_line(int y, UINT8 chars1[16]);
	int sprite_collision(int n1, int n2);
	void draw_sprites();
	required_device<cpu_device> m_maincpu;
	required_device<arcadia_cart_slot_device> m_cart;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};
#endif /* ARCADIA_H_ */
