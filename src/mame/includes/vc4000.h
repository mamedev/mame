// license:GPL-2.0+
// copyright-holders:Peter Trauner, Robbbert
// thanks-to:Manfred Schneider
/*****************************************************************************
 *
 * includes/vc4000.h
 *
 ****************************************************************************/

#ifndef VC4000_H_
#define VC4000_H_

#include "emu.h"
#include "audio/vc4000snd.h"
#include "cpu/s2650/s2650.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"

#include "bus/vc4000/slot.h"
#include "bus/vc4000/rom.h"

// define this to use digital inputs instead of the slow
// autocentering analog mame joys
#define ANALOG_HACK


struct SPRITE_HELPER
{
	UINT8 bitmap[10],x1,x2,y1,y2, res1, res2;
};

struct SPRITE
{
	const SPRITE_HELPER *data;
	int mask;
	int state;
	int delay;
	int size;
	int y;
	UINT8 scolor;
	int finished;
	int finished_now;
};

struct vc4000_video_t
{
	SPRITE sprites[4];
	int line;
	UINT8 sprite_collision;
	UINT8 background_collision;
	union
	{
		UINT8 data[0x100];
		struct
		{
			SPRITE_HELPER sprites[3];
			UINT8 res[0x10];
			SPRITE_HELPER sprite4;
			UINT8 res2[0x30];
			UINT8 grid[20][2];
			UINT8 grid_control[5];
			UINT8 res3[0x13];
			UINT8 sprite_sizes;
			UINT8 sprite_colors[2];
			UINT8 score_control;
			UINT8 res4[2];
			UINT8 background;
			UINT8 sound;
			UINT8 bcd[2];
			UINT8 background_collision;
			UINT8 sprite_collision;
		} d;
	} reg;
} ;

class vc4000_state : public driver_device
{
public:
	vc4000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_cassette(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_custom(*this, "custom"),
		m_keypad1_1(*this, "KEYPAD1_1"),
		m_keypad1_2(*this, "KEYPAD1_2"),
		m_keypad1_3(*this, "KEYPAD1_3"),
		m_panel(*this, "PANEL"),
		m_keypad2_1(*this, "KEYPAD2_1"),
		m_keypad2_2(*this, "KEYPAD2_2"),
		m_keypad2_3(*this, "KEYPAD2_3"),
#ifndef ANALOG_HACK
		m_io_joy1_x(*this, "JOY1_X"),
		m_io_joy1_y(*this, "JOY1_Y"),
		m_io_joy2_x(*this, "JOY2_X"),
		m_io_joy2_y(*this, "JOY2_Y")
#else
		m_joys(*this, "JOYS"),
		m_config(*this, "CONFIG")
#endif
	{ }

	DECLARE_WRITE8_MEMBER(vc4000_sound_ctl);
	DECLARE_READ8_MEMBER(vc4000_key_r);
	DECLARE_READ8_MEMBER(vc4000_video_r);
	DECLARE_WRITE8_MEMBER(vc4000_video_w);
	DECLARE_READ8_MEMBER(vc4000_vsync_r);
	DECLARE_READ8_MEMBER(elektor_cass_r);
	DECLARE_WRITE8_MEMBER(elektor_cass_w);
	vc4000_video_t m_video;
	UINT8 m_sprite_collision[0x20];
	UINT8 m_background_collision[0x20];
	UINT8 m_joy1_x;
	UINT8 m_joy1_y;
	UINT8 m_joy2_x;
	UINT8 m_joy2_y;
	UINT8 m_objects[512];
	UINT8 m_irq_pause;
	bitmap_ind16 *m_bitmap;
	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(vc4000);
	UINT32 screen_update_vc4000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vc4000_video_line);
	DECLARE_QUICKLOAD_LOAD_MEMBER(vc4000);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<cassette_image_device> m_cassette;
	required_device<vc4000_cart_slot_device> m_cart;
	required_device<vc4000_sound_device> m_custom;
	required_ioport m_keypad1_1;
	required_ioport m_keypad1_2;
	required_ioport m_keypad1_3;
	required_ioport m_panel;
	required_ioport m_keypad2_1;
	required_ioport m_keypad2_2;
	required_ioport m_keypad2_3;
#ifndef ANALOG_HACK
	required_ioport m_io_joy1_x;
	required_ioport m_io_joy1_y;
	required_ioport m_io_joy2_x;
	required_ioport m_io_joy2_y;
#else
	required_ioport m_joys;
	required_ioport m_config;
#endif
	inline UINT8 vc4000_joystick_return_to_centre(UINT8 joy);
	void vc4000_draw_digit(bitmap_ind16 &bitmap, int x, int y, int d, int line);
	inline void vc4000_collision_plot(UINT8 *collision, UINT8 data, UINT8 color, int scale);
	void vc4000_sprite_update(bitmap_ind16 &bitmap, UINT8 *collision, SPRITE *This);
	inline void vc4000_draw_grid(UINT8 *collision);
};

#endif /* VC4000_H_ */
