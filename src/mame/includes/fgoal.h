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
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_mb14241(*this, "mb14241"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
		{ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_video_ram;

	/* video-related */
	bitmap_ind16   m_bgbitmap;
	bitmap_ind16   m_fgbitmap;
	UINT8      m_xpos;
	UINT8      m_ypos;
	int        m_current_color;

	/* misc */
	int        m_fgoal_player;
	UINT8      m_row;
	UINT8      m_col;
	int        m_prev_coin;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<mb14241_device> m_mb14241;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_READ8_MEMBER(fgoal_analog_r);
	DECLARE_READ8_MEMBER(fgoal_nmi_reset_r);
	DECLARE_READ8_MEMBER(fgoal_irq_reset_r);
	DECLARE_READ8_MEMBER(fgoal_row_r);
	DECLARE_WRITE8_MEMBER(fgoal_row_w);
	DECLARE_WRITE8_MEMBER(fgoal_col_w);
	DECLARE_READ8_MEMBER(fgoal_address_hi_r);
	DECLARE_READ8_MEMBER(fgoal_address_lo_r);
	DECLARE_READ8_MEMBER(fgoal_shifter_r);
	DECLARE_READ8_MEMBER(fgoal_shifter_reverse_r);
	DECLARE_WRITE8_MEMBER(fgoal_sound1_w);
	DECLARE_WRITE8_MEMBER(fgoal_sound2_w);
	DECLARE_WRITE8_MEMBER(fgoal_color_w);
	DECLARE_WRITE8_MEMBER(fgoal_ypos_w);
	DECLARE_WRITE8_MEMBER(fgoal_xpos_w);
	DECLARE_CUSTOM_INPUT_MEMBER(fgoal_80_r);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(fgoal);
	UINT32 screen_update_fgoal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_callback);
	int intensity(int bits);
	unsigned video_ram_address(  );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};
