#include "video/bufsprite.h"

class lwings_state : public driver_device
{
public:
	lwings_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT8 *  m_fgvideoram;
	UINT8 *  m_bg1videoram;
	UINT8 *  m_soundlatch2;
//      UINT8 *  m_paletteram;    // currently this uses generic palette handling
//      UINT8 *  m_paletteram2;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *m_fg_tilemap;
	tilemap_t  *m_bg1_tilemap;
	tilemap_t  *m_bg2_tilemap;
	UINT8    m_bg2_image;
	int      m_bg2_avenger_hw;
	UINT8    m_scroll_x[2];
	UINT8    m_scroll_y[2];

	/* misc */
	UINT8    m_param[4];
	int      m_palette_pen;
	UINT8    m_soundstate;
	UINT8    m_adpcm;
	UINT8    m_nmi_mask;

	required_device<buffered_spriteram8_device> m_spriteram;
	DECLARE_WRITE8_MEMBER(avengers_adpcm_w);
	DECLARE_READ8_MEMBER(avengers_adpcm_r);
	DECLARE_WRITE8_MEMBER(lwings_bankswitch_w);
	DECLARE_WRITE8_MEMBER(avengers_protection_w);
	DECLARE_WRITE8_MEMBER(avengers_prot_bank_w);
	DECLARE_READ8_MEMBER(avengers_protection_r);
	DECLARE_READ8_MEMBER(avengers_soundlatch2_r);
	DECLARE_WRITE8_MEMBER(lwings_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(lwings_bg1videoram_w);
	DECLARE_WRITE8_MEMBER(lwings_bg1_scrollx_w);
	DECLARE_WRITE8_MEMBER(lwings_bg1_scrolly_w);
	DECLARE_WRITE8_MEMBER(trojan_bg2_scrollx_w);
	DECLARE_WRITE8_MEMBER(trojan_bg2_image_w);
};


/*----------- defined in video/lwings.c -----------*/


VIDEO_START( lwings );
VIDEO_START( trojan );
VIDEO_START( avengers );
SCREEN_UPDATE_IND16( lwings );
SCREEN_UPDATE_IND16( trojan );
