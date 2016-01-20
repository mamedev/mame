// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, David Haywood
/*************************************************************************

    Cross Pang

*************************************************************************/

#include "video/decospr.h"

class crospang_state : public driver_device
{
public:
	crospang_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_sprgen(*this, "spritegen"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_fg_videoram;
	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_spriteram;
	optional_device<decospr_device> m_sprgen;

	/* video-related */
	tilemap_t   *m_bg_layer;
	tilemap_t   *m_fg_layer;
	int       m_bestri_tilebank;

	/* devices */
	DECLARE_WRITE16_MEMBER(crospang_soundlatch_w);
	DECLARE_WRITE16_MEMBER(bestri_tilebank_w);
	DECLARE_WRITE16_MEMBER(bestri_bg_scrolly_w);
	DECLARE_WRITE16_MEMBER(bestri_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(bestri_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(bestri_bg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_scrolly_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_videoram_w);
	DECLARE_DRIVER_INIT(crospang);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_crospang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tumblepb_gfx1_rearrange();
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
