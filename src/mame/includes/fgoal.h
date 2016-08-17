// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
#include "machine/mb14241.h"

class fgoal_state : public driver_device
{
public:
	enum
	{
		TIMER_INTERRUPT
	};

	fgoal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb14241(*this, "mb14241"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video_ram(*this, "video_ram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<mb14241_device> m_mb14241;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<UINT8> m_video_ram;

	/* video-related */
	bitmap_ind16   m_bgbitmap;
	bitmap_ind16   m_fgbitmap;
	UINT8      m_xpos;
	UINT8      m_ypos;
	int        m_current_color;

	/* misc */
	int        m_player;
	UINT8      m_row;
	UINT8      m_col;
	int        m_prev_coin;
	emu_timer  *m_interrupt_timer;

	DECLARE_READ8_MEMBER(analog_r);
	DECLARE_READ8_MEMBER(nmi_reset_r);
	DECLARE_READ8_MEMBER(irq_reset_r);
	DECLARE_READ8_MEMBER(row_r);
	DECLARE_WRITE8_MEMBER(row_w);
	DECLARE_WRITE8_MEMBER(col_w);
	DECLARE_READ8_MEMBER(address_hi_r);
	DECLARE_READ8_MEMBER(address_lo_r);
	DECLARE_READ8_MEMBER(shifter_r);
	DECLARE_READ8_MEMBER(shifter_reverse_r);
	DECLARE_WRITE8_MEMBER(sound1_w);
	DECLARE_WRITE8_MEMBER(sound2_w);
	DECLARE_WRITE8_MEMBER(color_w);
	DECLARE_WRITE8_MEMBER(ypos_w);
	DECLARE_WRITE8_MEMBER(xpos_w);

	DECLARE_CUSTOM_INPUT_MEMBER(_80_r);

	TIMER_CALLBACK_MEMBER(interrupt_callback);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(fgoal);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int intensity(int bits);
	unsigned video_ram_address( );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
