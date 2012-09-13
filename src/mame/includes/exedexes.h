/*************************************************************************

    Exed Exes

*************************************************************************/

#include "video/bufsprite.h"

class exedexes_state : public driver_device
{
public:
	exedexes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_nbg_yscroll(*this, "nbg_yscroll"),
		m_nbg_xscroll(*this, "nbg_xscroll"),
		m_bg_scroll(*this, "bg_scroll"){ }

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
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
};



/*----------- defined in video/exedexes.c -----------*/


extern PALETTE_INIT( exedexes );
extern VIDEO_START( exedexes );
extern SCREEN_UPDATE_IND16( exedexes );
