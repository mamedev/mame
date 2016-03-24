// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

    Space Firebird hardware

****************************************************************************/
#include "sound/samples.h"
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

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<samples_device> m_samples;
	required_device<screen_device> m_screen;

	required_shared_ptr<UINT8> m_videoram;

	UINT8 m_sound_latch;
	emu_timer *m_interrupt_timer;
	std::unique_ptr<UINT8[]> m_object_present_map;
	UINT8 m_port_0;
	UINT8 m_port_2;
	UINT32 m_star_shift_reg;
	double m_color_weights_rg[3];
	double m_color_weights_b[2];

	DECLARE_WRITE8_MEMBER(port_0_w);
	DECLARE_WRITE8_MEMBER(port_1_w);
	DECLARE_WRITE8_MEMBER(port_2_w);
	DECLARE_READ8_MEMBER(audio_p2_r);
	DECLARE_READ8_MEMBER(audio_t0_r);
	DECLARE_READ8_MEMBER(audio_t1_r);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	TIMER_CALLBACK_MEMBER(interrupt_callback);
	void start_interrupt_timer();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline void shift_star_generator();
	void get_starfield_pens(pen_t *pens);
	void draw_starfield(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void get_sprite_pens(pen_t *pens);
	void draw_bullet(offs_t offs, pen_t pen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flip);
	void draw_sprite(offs_t offs, pen_t *pens, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flip);
	void draw_objects(bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:

	enum
	{
		TIMER_INTERRUPT
	};

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*----------- defined in audio/spacefb.c -----------*/
MACHINE_CONFIG_EXTERN( spacefb_audio );
