// license:GPL-2.0+
// copyright-holders:Peter Trauner, Robbbert
// thanks-to:Manfred Schneider
/*****************************************************************************
 *
 * includes/vc4000.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_VC4000_H
#define MAME_INCLUDES_VC4000_H

#pragma once

#include "audio/vc4000.h"
#include "cpu/s2650/s2650.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"

#include "bus/vc4000/slot.h"
#include "bus/vc4000/rom.h"

#include "emupal.h"
#include "screen.h"

// define this to use digital inputs instead of the slow
// autocentering analog mame joys
#define ANALOG_HACK


class vc4000_state : public driver_device
{
public:
	vc4000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
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

	void cx3000tc(machine_config &config);
	void mpu1000(machine_config &config);
	void vc4000(machine_config &config);
	void database(machine_config &config);
	void h21(machine_config &config);
	void rwtrntcs(machine_config &config);
	void elektor(machine_config &config);

private:
	struct SPRITE_HELPER
	{
		uint8_t bitmap[10],x1,x2,y1,y2, res1, res2;
	};

	struct SPRITE
	{
		const SPRITE_HELPER *data;
		int mask;
		int state;
		int delay;
		int size;
		int y;
		uint8_t scolor;
		int finished;
		int finished_now;
	};

	struct vc4000_video_t
	{
		SPRITE sprites[4];
		int line;
		uint8_t sprite_collision;
		uint8_t background_collision;
		union
		{
			uint8_t data[0x100];
			struct
			{
				SPRITE_HELPER sprites[3];
				uint8_t res[0x10];
				SPRITE_HELPER sprite4;
				uint8_t res2[0x30];
				uint8_t grid[20][2];
				uint8_t grid_control[5];
				uint8_t res3[0x13];
				uint8_t sprite_sizes;
				uint8_t sprite_colors[2];
				uint8_t score_control;
				uint8_t res4[2];
				uint8_t background;
				uint8_t sound;
				uint8_t bcd[2];
				uint8_t background_collision;
				uint8_t sprite_collision;
			} d;
		} reg;
	} ;

	DECLARE_WRITE8_MEMBER(vc4000_sound_ctl);
	DECLARE_READ8_MEMBER(vc4000_key_r);
	DECLARE_READ8_MEMBER(vc4000_video_r);
	DECLARE_WRITE8_MEMBER(vc4000_video_w);
	DECLARE_READ_LINE_MEMBER(vc4000_vsync_r);
	DECLARE_READ8_MEMBER(elektor_cass_r);
	DECLARE_WRITE8_MEMBER(elektor_cass_w);
	vc4000_video_t m_video;
	uint8_t m_sprite_collision[0x20];
	uint8_t m_background_collision[0x20];
	uint8_t m_joy1_x;
	uint8_t m_joy1_y;
	uint8_t m_joy2_x;
	uint8_t m_joy2_y;
	uint8_t m_objects[512];
	uint8_t m_irq_pause;
	std::unique_ptr<bitmap_ind16> m_bitmap;
	virtual void machine_start() override;
	virtual void video_start() override;
	void vc4000_palette(palette_device &palette) const;
	uint32_t screen_update_vc4000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vc4000_video_line);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void elektor_mem(address_map &map);
	void vc4000_mem(address_map &map);

	required_device<s2650_device> m_maincpu;
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
	inline uint8_t vc4000_joystick_return_to_centre(uint8_t joy);
	void vc4000_draw_digit(bitmap_ind16 &bitmap, int x, int y, int d, int line);
	inline void vc4000_collision_plot(uint8_t *collision, uint8_t data, uint8_t color, int scale);
	void vc4000_sprite_update(bitmap_ind16 &bitmap, uint8_t *collision, SPRITE *This);
	inline void vc4000_draw_grid(uint8_t *collision);
};

#endif // MAME_INCLUDES_VC4000_H
