// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, David Haywood
/*************************************************************************

    Goal! '92

*************************************************************************/
#include "sound/msm5205.h"
class goal92_state : public driver_device
{
public:
	goal92_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg_data(*this, "bg_data"),
		m_fg_data(*this, "fg_data"),
		m_tx_data(*this, "tx_data"),
		m_spriteram(*this, "spriteram"),
		m_scrollram(*this, "scrollram"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_bg_data;
	required_shared_ptr<UINT16> m_fg_data;
	required_shared_ptr<UINT16> m_tx_data;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_scrollram;
	UINT16 *    m_buffered_spriteram;

	/* video-related */
	tilemap_t     *m_bg_layer;
	tilemap_t     *m_fg_layer;
	tilemap_t     *m_tx_layer;
	UINT16      m_fg_bank;

	/* misc */
	int         m_msm5205next;
	int         m_adpcm_toggle;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	DECLARE_WRITE16_MEMBER(goal92_sound_command_w);
	DECLARE_READ16_MEMBER(goal92_inputs_r);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_READ16_MEMBER(goal92_fg_bank_r);
	DECLARE_WRITE16_MEMBER(goal92_fg_bank_w);
	DECLARE_WRITE16_MEMBER(goal92_text_w);
	DECLARE_WRITE16_MEMBER(goal92_background_w);
	DECLARE_WRITE16_MEMBER(goal92_foreground_w);
	DECLARE_WRITE8_MEMBER(adpcm_control_w);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_goal92(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_goal92(screen_device &screen, bool state);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	DECLARE_WRITE_LINE_MEMBER(goal92_adpcm_int);
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
