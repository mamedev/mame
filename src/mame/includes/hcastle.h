/*************************************************************************

    Haunted Castle

*************************************************************************/

#include "video/bufsprite.h"

class hcastle_state : public driver_device
{
public:
	hcastle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram"),
		  m_spriteram2(*this, "spriteram2") ,
		m_paletteram(*this, "paletteram"),
		m_pf1_videoram(*this, "pf1_videoram"),
		m_pf2_videoram(*this, "pf2_videoram"){ }

	required_device<buffered_spriteram8_device> m_spriteram;
	required_device<buffered_spriteram8_device> m_spriteram2;
	/* memory pointers */
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_pf1_videoram;
	required_shared_ptr<UINT8> m_pf2_videoram;

	/* video-related */
	tilemap_t    *m_fg_tilemap;
	tilemap_t    *m_bg_tilemap;
	int        m_pf2_bankbase;
	int        m_pf1_bankbase;
	int        m_old_pf1;
	int        m_old_pf2;
	int        m_gfx_bank;

	/* devices */
	cpu_device *m_audiocpu;
	device_t *m_k007121_1;
	device_t *m_k007121_2;

	DECLARE_WRITE8_MEMBER(hcastle_bankswitch_w);
	DECLARE_WRITE8_MEMBER(hcastle_soundirq_w);
	DECLARE_WRITE8_MEMBER(hcastle_coin_w);
	DECLARE_WRITE8_MEMBER(hcastle_pf1_video_w);
	DECLARE_WRITE8_MEMBER(hcastle_pf2_video_w);
	DECLARE_WRITE8_MEMBER(hcastle_gfxbank_w);
	DECLARE_READ8_MEMBER(hcastle_gfxbank_r);
	DECLARE_WRITE8_MEMBER(hcastle_pf1_control_w);
	DECLARE_WRITE8_MEMBER(hcastle_pf2_control_w);
	DECLARE_WRITE8_MEMBER(sound_bank_w);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_hcastle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/hcastle.c -----------*/





