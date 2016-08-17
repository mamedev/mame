// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Aaron Giles
/*************************************************************************

    Atari Tetris hardware

*************************************************************************/

#include "includes/slapstic.h"

class atetris_state : public driver_device
{
public:
	atetris_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_slapstic_device(*this, "slapstic"),
		m_nvram(*this, "nvram") ,
		m_videoram(*this, "videoram")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	optional_device<atari_slapstic_device> m_slapstic_device;

	required_shared_ptr<UINT8>  m_nvram;
	required_shared_ptr<UINT8> m_videoram;

	UINT8 *m_slapstic_source;
	UINT8 *m_slapstic_base;
	UINT8 m_current_bank;
	UINT8 m_nvram_write_enable;
	emu_timer *m_interrupt_timer;
	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(slapstic_r);
	DECLARE_WRITE8_MEMBER(coincount_w);
	DECLARE_WRITE8_MEMBER(nvram_w);
	DECLARE_WRITE8_MEMBER(nvram_enable_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_DRIVER_INIT(atetris);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_gen);
	void reset_bank();
};
