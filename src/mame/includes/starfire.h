// license:BSD-3-Clause
// copyright-holders:Dan Boris, Olivier Galibert, Aaron Giles
/***************************************************************************

    Star Fire/Fire One system

***************************************************************************/

#include "sound/samples.h"


#define STARFIRE_MASTER_CLOCK   (XTAL_20MHz)
#define STARFIRE_CPU_CLOCK      (STARFIRE_MASTER_CLOCK / 8)
#define STARFIRE_PIXEL_CLOCK    (STARFIRE_MASTER_CLOCK / 4)
#define STARFIRE_HTOTAL         (0x13f)  /* could be 0x140, but I think this is right */
#define STARFIRE_HBEND          (0x000)
#define STARFIRE_HBSTART        (0x100)
#define STARFIRE_VTOTAL         (0x106)
#define STARFIRE_VBEND          (0x020)
#define STARFIRE_VBSTART        (0x100)
#define STARFIRE_NUM_PENS       (0x40)


class starfire_state : public driver_device
{
public:
	starfire_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_starfire_colorram(*this, "colorram"),
		m_starfire_videoram(*this, "videoram"),
		m_samples(*this, "samples"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen") { }

	required_shared_ptr<UINT8> m_starfire_colorram;
	required_shared_ptr<UINT8> m_starfire_videoram;
	optional_device<samples_device> m_samples;

	UINT8 m_prev_sound;
	UINT8 m_fireone_select;

	UINT8 m_starfire_vidctrl;
	UINT8 m_starfire_vidctrl1;
	UINT8 m_starfire_color;
	UINT16 m_starfire_colors[STARFIRE_NUM_PENS];

	read8_delegate m_input_read;
	write8_delegate m_io2_write;

	emu_timer* m_scanline_timer;
	bitmap_rgb32 m_starfire_screen;
	DECLARE_WRITE8_MEMBER(starfire_scratch_w);
	DECLARE_READ8_MEMBER(starfire_scratch_r);
	DECLARE_READ8_MEMBER(starfire_input_r);
	DECLARE_READ8_MEMBER(fireone_input_r);
	DECLARE_WRITE8_MEMBER(starfire_sound_w);
	DECLARE_WRITE8_MEMBER(fireone_sound_w);
	DECLARE_WRITE8_MEMBER(starfire_colorram_w);
	DECLARE_READ8_MEMBER(starfire_colorram_r);
	DECLARE_WRITE8_MEMBER(starfire_videoram_w);
	DECLARE_READ8_MEMBER(starfire_videoram_r);
	DECLARE_DRIVER_INIT(starfire);
	DECLARE_DRIVER_INIT(fireone);
	virtual void video_start() override;
	UINT32 screen_update_starfire(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(starfire_scanline_callback);
	INTERRUPT_GEN_MEMBER(vblank_int);
	void get_pens(pen_t *pens);
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};
