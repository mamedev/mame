// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Victory system

****************************************************************************/
#ifndef MAME_INCLUDES_VICTORY_H
#define MAME_INCLUDES_VICTORY_H

#pragma once

#include "emupal.h"
#include "screen.h"


#define VICTORY_MAIN_CPU_CLOCK      (XTAL(8'000'000) / 2)

#define VICTORY_PIXEL_CLOCK             (XTAL(11'289'000) / 2)
#define VICTORY_HTOTAL                  (0x150)
#define VICTORY_HBEND                       (0x000)
#define VICTORY_HBSTART                 (0x100)
#define VICTORY_VTOTAL                  (0x118)
#define VICTORY_VBEND                       (0x000)
#define VICTORY_VBSTART                 (0x100)


class victory_state : public driver_device
{
public:
	victory_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_charram(*this, "charram"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void victory(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(lamp_control_w);
	DECLARE_WRITE8_MEMBER(paletteram_w);
	DECLARE_READ8_MEMBER(video_control_r);
	DECLARE_WRITE8_MEMBER(video_control_w);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_interrupt);
	TIMER_CALLBACK_MEMBER(bgcoll_irq_callback);

	void update_irq();
	void set_palette();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	void update_background();
	void update_foreground();

	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override;
	void main_io_map(address_map &map);
	void main_map(address_map &map);

	/* microcode state */
	struct micro_t
	{
		uint16_t    i;
		uint16_t    pc;
		uint8_t     r,g,b;
		uint8_t     xp,yp;
		uint8_t     cmd,cmdlo;
		emu_timer * timer;
		uint8_t     timer_active;
		attotime    endtime;

		void count_states(int states);
	};

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_charram;
	output_finder<4> m_lamps;

	uint16_t m_paletteram[0x40];
	std::unique_ptr<uint8_t[]> m_bgbitmap;
	std::unique_ptr<uint8_t[]> m_fgbitmap;
	std::unique_ptr<uint8_t[]> m_rram;
	std::unique_ptr<uint8_t[]> m_gram;
	std::unique_ptr<uint8_t[]> m_bram;
	uint8_t m_vblank_irq;
	uint8_t m_fgcoll;
	uint8_t m_fgcollx;
	uint8_t m_fgcolly;
	uint8_t m_bgcoll;
	uint8_t m_bgcollx;
	uint8_t m_bgcolly;
	uint8_t m_scrollx;
	uint8_t m_scrolly;
	uint8_t m_video_control;
	micro_t m_micro;
	emu_timer *m_bgcoll_irq_timer;
};

#endif // MAME_INCLUDES_VICTORY_H
