#include "sound/samples.h"

class gotya_state : public driver_device
{
public:
	gotya_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_scroll(*this, "scroll"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int      m_scroll_bit_8;

	/* sound-related */
	int      m_theme_playing;

	/* devices */
	samples_device *m_samples;
	DECLARE_WRITE8_MEMBER(gotya_videoram_w);
	DECLARE_WRITE8_MEMBER(gotya_colorram_w);
	DECLARE_WRITE8_MEMBER(gotya_video_control_w);
	DECLARE_WRITE8_MEMBER(gotya_soundlatch_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows_thehand);
};


/*----------- defined in audio/gotya.c -----------*/



/*----------- defined in video/gotya.c -----------*/


PALETTE_INIT( gotya );
VIDEO_START( gotya );
SCREEN_UPDATE_IND16( gotya );
