// license:???
// copyright-holders:Richard Davies
/*************************************************************************

    Exed Exes

*************************************************************************/

#include "video/bufsprite.h"

class exedexes_state : public driver_device
{
public:
	exedexes_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram") ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_nbg_yscroll(*this, "nbg_yscroll"),
		m_nbg_xscroll(*this, "nbg_xscroll"),
		m_bg_scroll(*this, "bg_scroll"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_nbg_yscroll;
	required_shared_ptr<UINT8> m_nbg_xscroll;
	required_shared_ptr<UINT8> m_bg_scroll;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	tilemap_t        *m_tx_tilemap;
	int            m_chon;
	int            m_objon;
	int            m_sc1on;
	int            m_sc2on;

	DECLARE_WRITE8_MEMBER(exedexes_videoram_w);
	DECLARE_WRITE8_MEMBER(exedexes_colorram_w);
	DECLARE_WRITE8_MEMBER(exedexes_c804_w);
	DECLARE_WRITE8_MEMBER(exedexes_gfxctrl_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILEMAP_MAPPER_MEMBER(exedexes_bg_tilemap_scan);
	TILEMAP_MAPPER_MEMBER(exedexes_fg_tilemap_scan);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(exedexes);
	UINT32 screen_update_exedexes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(exedexes_scanline);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
