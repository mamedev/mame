// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Gotcha

*************************************************************************/
#include "sound/okim6295.h"
#include "video/decospr.h"

class gotcha_state : public driver_device
{
public:
	gotcha_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_sprgen(*this, "spritegen"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_fgvideoram;
	required_shared_ptr<UINT16> m_bgvideoram;
	required_shared_ptr<UINT16> m_spriteram;
	optional_device<decospr_device> m_sprgen;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	int         m_banksel;
	int         m_gfxbank[4];
	UINT16      m_scroll[4];

	/* devices */
	required_device<cpu_device> m_audiocpu;
	DECLARE_WRITE16_MEMBER(gotcha_lamps_w);
	DECLARE_WRITE16_MEMBER(gotcha_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(gotcha_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(gotcha_gfxbank_select_w);
	DECLARE_WRITE16_MEMBER(gotcha_gfxbank_w);
	DECLARE_WRITE16_MEMBER(gotcha_scroll_w);
	DECLARE_WRITE16_MEMBER(gotcha_oki_bank_w);
	TILEMAP_MAPPER_MEMBER(gotcha_tilemap_scan);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_gotcha(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void get_tile_info( tile_data &tileinfo, int tile_index ,UINT16 *vram, int color_offs);
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
};
