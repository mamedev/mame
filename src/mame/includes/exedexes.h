/*************************************************************************

    Exed Exes

*************************************************************************/

#include "video/bufsprite.h"

class exedexes_state : public driver_device
{
public:
	exedexes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT8 *        m_videoram;
	UINT8 *        m_colorram;
	UINT8 *        m_bg_scroll;
	UINT8 *        m_nbg_yscroll;
	UINT8 *        m_nbg_xscroll;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	tilemap_t        *m_tx_tilemap;
	int            m_chon;
	int            m_objon;
	int            m_sc1on;
	int            m_sc2on;

	required_device<buffered_spriteram8_device> m_spriteram;
	DECLARE_WRITE8_MEMBER(exedexes_videoram_w);
	DECLARE_WRITE8_MEMBER(exedexes_colorram_w);
	DECLARE_WRITE8_MEMBER(exedexes_c804_w);
	DECLARE_WRITE8_MEMBER(exedexes_gfxctrl_w);
};



/*----------- defined in video/exedexes.c -----------*/


extern PALETTE_INIT( exedexes );
extern VIDEO_START( exedexes );
extern SCREEN_UPDATE_IND16( exedexes );
