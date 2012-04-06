/***************************************************************************

    Bionic Commando

***************************************************************************/

#include "video/bufsprite.h"

class bionicc_state : public driver_device
{
public:
	bionicc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT16 *  m_bgvideoram;
	UINT16 *  m_fgvideoram;
	UINT16 *  m_txvideoram;
	UINT16 *  m_paletteram;

	/* video-related */
	tilemap_t   *m_tx_tilemap;
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	UINT16    m_scroll[4];

	UINT16    m_inp[3];
	UINT16    m_soundcommand;

	required_device<buffered_spriteram16_device> m_spriteram;
	DECLARE_WRITE16_MEMBER(hacked_controls_w);
	DECLARE_READ16_MEMBER(hacked_controls_r);
	DECLARE_WRITE16_MEMBER(bionicc_mpu_trigger_w);
	DECLARE_WRITE16_MEMBER(hacked_soundcommand_w);
	DECLARE_READ16_MEMBER(hacked_soundcommand_r);
	DECLARE_WRITE16_MEMBER(bionicc_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(bionicc_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(bionicc_txvideoram_w);
	DECLARE_WRITE16_MEMBER(bionicc_paletteram_w);
	DECLARE_WRITE16_MEMBER(bionicc_scroll_w);
	DECLARE_WRITE16_MEMBER(bionicc_gfxctrl_w);
};


/*----------- defined in video/bionicc.c -----------*/


VIDEO_START( bionicc );
SCREEN_UPDATE_IND16( bionicc );
