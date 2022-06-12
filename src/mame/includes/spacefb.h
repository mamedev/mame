// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

    Space Firebird hardware

****************************************************************************/

#ifndef MAME_INCLUDES_SPACEFB
#define MAME_INCLUDES_SPACEFB

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/samples.h"
#include "screen.h"
/*
 *  SPACEFB_PIXEL_CLOCK clocks the star generator circuit.  The rest of
 *  the graphics use a clock half of SPACEFB_PIXEL_CLOCK, thus creating
 *  double width pixels.
 */

#define SPACEFB_MASTER_CLOCK            (20160000)
#define SPACEFB_MAIN_CPU_CLOCK          (6000000 / 2)
#define SPACEFB_AUDIO_CPU_CLOCK         (6000000)   /* this goes to X2, pixel clock goes to X1 */
#define SPACEFB_PIXEL_CLOCK             (SPACEFB_MASTER_CLOCK / 2)
#define SPACEFB_HTOTAL                  (0x280)
#define SPACEFB_HBEND                   (0x000)
#define SPACEFB_HBSTART                 (0x200)
#define SPACEFB_VTOTAL                  (0x100)
#define SPACEFB_VBEND                   (0x010)
#define SPACEFB_VBSTART                 (0x0f0)
#define SPACEFB_INT_TRIGGER_COUNT_1     (0x080)
#define SPACEFB_INT_TRIGGER_COUNT_2     (0x0f0)


class spacefb_state : public driver_device
{
public:
	spacefb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_samples(*this, "samples"),
		m_screen(*this, "screen"),
		m_videoram(*this, "videoram") { }

	void spacefb(machine_config &config);
	void spacefb_audio(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void spacefb_main_map(address_map &map);
	void spacefb_main_io_map(address_map &map);
	void spacefb_audio_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<i8035_device> m_audiocpu;
	required_device<samples_device> m_samples;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint8_t> m_videoram;

	uint8_t m_sound_latch = 0;
	emu_timer *m_interrupt_timer = nullptr;
	std::unique_ptr<uint8_t[]> m_object_present_map;
	uint8_t m_port_0 = 0;
	uint8_t m_port_2 = 0;
	uint32_t m_star_shift_reg = 0;
	double m_color_weights_rg[3]{};
	double m_color_weights_b[2]{};

	void port_0_w(uint8_t data);
	void port_1_w(uint8_t data);
	void port_2_w(uint8_t data);
	uint8_t audio_p2_r();
	DECLARE_READ_LINE_MEMBER(audio_t0_r);
	DECLARE_READ_LINE_MEMBER(audio_t1_r);

	TIMER_CALLBACK_MEMBER(interrupt_callback);
	void start_interrupt_timer();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline void shift_star_generator();
	void get_starfield_pens(pen_t *pens);
	void draw_starfield(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void get_sprite_pens(pen_t *pens);
	void draw_bullet(offs_t offs, pen_t pen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flip);
	void draw_sprite(offs_t offs, pen_t *pens, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flip);
	void draw_objects(bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

#endif // MAME_INCLUDES_SPACEFB
