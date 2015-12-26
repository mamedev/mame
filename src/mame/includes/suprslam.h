// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Super Slams

*************************************************************************/

#include "video/k053936.h"

class suprslam_state : public driver_device
{
public:
	suprslam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_screen_videoram(*this, "screen_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_sp_videoram(*this, "sp_videoram"),
		m_spriteram(*this, "spriteram"),
		m_spr_ctrl(*this, "spr_ctrl"),
		m_screen_vregs(*this, "screen_vregs"),
		m_spr(*this, "vsystem_spr"),
		m_audiocpu(*this, "audiocpu"),
		m_k053936(*this, "k053936"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_screen_videoram;
	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_sp_videoram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_spr_ctrl;
	required_shared_ptr<UINT16> m_screen_vregs;
	required_device<vsystem_spr_device> m_spr;

	/* video-related */
	tilemap_t     *m_screen_tilemap;
	tilemap_t     *m_bg_tilemap;
	UINT16      m_screen_bank;
	UINT16      m_bg_bank;
	UINT32  suprslam_tile_callback( UINT32 code );

	/* misc */
	int         m_pending_command;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<k053936_device> m_k053936;
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
	DECLARE_WRITE8_MEMBER(suprslam_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(suprslam_screen_videoram_w);
	DECLARE_WRITE16_MEMBER(suprslam_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(suprslam_bank_w);
	TILE_GET_INFO_MEMBER(get_suprslam_tile_info);
	TILE_GET_INFO_MEMBER(get_suprslam_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_suprslam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
