/***************************************************************************

    Time Pilot

***************************************************************************/

#include "sound/tc8830f.h"

class timeplt_state : public driver_device
{
public:
	timeplt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tc8830f(*this, "tc8830f"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<tc8830f_device> m_tc8830f;

	/* memory pointers */
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_spriteram2;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	UINT8    m_nmi_enable;

	DECLARE_WRITE8_MEMBER(timeplt_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(timeplt_coin_counter_w);
	DECLARE_READ8_MEMBER(psurge_protection_r);
	DECLARE_WRITE8_MEMBER(timeplt_videoram_w);
	DECLARE_WRITE8_MEMBER(timeplt_colorram_w);
	DECLARE_WRITE8_MEMBER(timeplt_flipscreen_w);
	DECLARE_READ8_MEMBER(timeplt_scanline_r);
	DECLARE_CUSTOM_INPUT_MEMBER(chkun_hopper_status_r);
	DECLARE_WRITE8_MEMBER(chkun_sound_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_chkun_tile_info);
};


/*----------- defined in video/timeplt.c -----------*/


PALETTE_INIT( timeplt );
VIDEO_START( timeplt );
VIDEO_START( chkun );
SCREEN_UPDATE_IND16( timeplt );
