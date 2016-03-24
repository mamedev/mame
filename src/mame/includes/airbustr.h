// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    Air Buster

*************************************************************************/

#include "video/kan_pand.h"

class airbustr_state : public driver_device
{
public:
	airbustr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_devram(*this, "devram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_master(*this, "master"),
		m_slave(*this, "slave"),
		m_audiocpu(*this, "audiocpu"),
		m_pandora(*this, "pandora"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
		{ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_devram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	bitmap_ind16 m_sprites_bitmap;
	int        m_bg_scrollx;
	int        m_bg_scrolly;
	int        m_fg_scrollx;
	int        m_fg_scrolly;
	int        m_highbits;

	/* misc */
	int        m_soundlatch_status;
	int        m_soundlatch2_status;

	/* devices */
	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	required_device<cpu_device> m_audiocpu;
	required_device<kaneko_pandora_device> m_pandora;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_READ8_MEMBER(devram_r);
	DECLARE_WRITE8_MEMBER(master_nmi_trigger_w);
	DECLARE_WRITE8_MEMBER(master_bankswitch_w);
	DECLARE_WRITE8_MEMBER(slave_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_READ8_MEMBER(soundcommand_status_r);
	DECLARE_READ8_MEMBER(soundcommand_r);
	DECLARE_READ8_MEMBER(soundcommand2_r);
	DECLARE_WRITE8_MEMBER(soundcommand_w);
	DECLARE_WRITE8_MEMBER(soundcommand2_w);
	DECLARE_WRITE8_MEMBER(airbustr_coin_counter_w);
	DECLARE_WRITE8_MEMBER(airbustr_videoram_w);
	DECLARE_WRITE8_MEMBER(airbustr_colorram_w);
	DECLARE_WRITE8_MEMBER(airbustr_videoram2_w);
	DECLARE_WRITE8_MEMBER(airbustr_colorram2_w);
	DECLARE_WRITE8_MEMBER(airbustr_scrollregs_w);
	DECLARE_DRIVER_INIT(airbustr);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_airbustr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_airbustr(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(slave_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(airbustr_scanline);
};
