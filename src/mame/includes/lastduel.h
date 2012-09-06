/*************************************************************************

    Last Duel

*************************************************************************/

#include "video/bufsprite.h"

class lastduel_state : public driver_device
{
public:
	lastduel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spriteram(*this, "spriteram"),
		m_vram(*this, "vram"),
		m_scroll1(*this, "scroll1"),
		m_scroll2(*this, "scroll2"),
		m_paletteram(*this, "paletteram"){ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	/* memory pointers */
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_vram;
	required_shared_ptr<UINT16> m_scroll1;
	required_shared_ptr<UINT16> m_scroll2;
	required_shared_ptr<UINT16> m_paletteram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_tx_tilemap;
	UINT16      m_scroll[8];
	int         m_sprite_flipy_mask;
	int         m_sprite_pri_mask;
	int         m_tilemap_priority;

	DECLARE_WRITE16_MEMBER(lastduel_sound_w);
	DECLARE_WRITE8_MEMBER(mg_bankswitch_w);
	DECLARE_WRITE16_MEMBER(lastduel_flip_w);
	DECLARE_WRITE16_MEMBER(lastduel_scroll_w);
	DECLARE_WRITE16_MEMBER(lastduel_scroll1_w);
	DECLARE_WRITE16_MEMBER(lastduel_scroll2_w);
	DECLARE_WRITE16_MEMBER(lastduel_vram_w);
	DECLARE_WRITE16_MEMBER(madgear_scroll1_w);
	DECLARE_WRITE16_MEMBER(madgear_scroll2_w);
	DECLARE_WRITE16_MEMBER(lastduel_palette_word_w);
	TILE_GET_INFO_MEMBER(ld_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(ld_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_info);
};

/*----------- defined in video/lastduel.c -----------*/


VIDEO_START( lastduel );
VIDEO_START( madgear );
SCREEN_UPDATE_IND16( lastduel );
SCREEN_UPDATE_IND16( madgear );
