// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Blades of Steel

*************************************************************************/
#include "sound/flt_rc.h"
#include "sound/upd7759.h"
#include "video/k007342.h"
#include "video/k007420.h"
#include "video/k051733.h"

class bladestl_state : public driver_device
{
public:
	bladestl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),

		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007342(*this, "k007342"),
		m_k007420(*this, "k007420"),
		m_upd7759(*this, "upd"),
		m_filter1(*this, "filter1"),
		m_filter2(*this, "filter2"),
		m_filter3(*this, "filter3"),
		m_gfxdecode(*this, "gfxdecode"),
		m_trackball(*this, "TRACKBALL"),
		m_rombank(*this, "rombank") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007342_device> m_k007342;
	required_device<k007420_device> m_k007420;
	required_device<upd7759_device> m_upd7759;
	required_device<filter_rc_device> m_filter1;
	required_device<filter_rc_device> m_filter2;
	required_device<filter_rc_device> m_filter3;
	required_device<gfxdecode_device> m_gfxdecode;
	required_ioport_array<4> m_trackball;

	/* memory pointers */
	required_memory_bank m_rombank;

	/* video-related */
	int        m_spritebank;

	/* misc */
	int        m_last_track[4];

	/* devices */
	DECLARE_READ8_MEMBER(trackball_r);
	DECLARE_WRITE8_MEMBER(bladestl_bankswitch_w);
	DECLARE_WRITE8_MEMBER(bladestl_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(bladestl_port_B_w);
	DECLARE_READ8_MEMBER(bladestl_speech_busy_r);
	DECLARE_WRITE8_MEMBER(bladestl_speech_ctrl_w);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_PALETTE_INIT(bladestl);
	UINT32 screen_update_bladestl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(bladestl_scanline);
	K007342_CALLBACK_MEMBER(bladestl_tile_callback);
	K007420_CALLBACK_MEMBER(bladestl_sprite_callback);
};
