#include "sound/samples.h"

class gotya_state : public driver_device
{
public:
	gotya_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_videoram2;
	UINT8 *  m_colorram;
	UINT8 *  m_spriteram;
	UINT8 *  m_scroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int      m_scroll_bit_8;

	/* sound-related */
	int      m_theme_playing;

	/* devices */
	samples_device *m_samples;
};


/*----------- defined in audio/gotya.c -----------*/

WRITE8_HANDLER( gotya_soundlatch_w );


/*----------- defined in video/gotya.c -----------*/

WRITE8_HANDLER( gotya_videoram_w );
WRITE8_HANDLER( gotya_colorram_w );
WRITE8_HANDLER( gotya_video_control_w );

PALETTE_INIT( gotya );
VIDEO_START( gotya );
SCREEN_UPDATE_IND16( gotya );
