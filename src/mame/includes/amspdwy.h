/*************************************************************************

    American Speedway

*************************************************************************/

class amspdwy_state : public driver_device
{
public:
	amspdwy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_colorram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_flipscreen;

	/* misc */
	UINT8      m_wheel_old[2];
	UINT8      m_wheel_return[2];

	/* devices */
	device_t *m_audiocpu;
	DECLARE_READ8_MEMBER(amspdwy_wheel_0_r);
	DECLARE_READ8_MEMBER(amspdwy_wheel_1_r);
	DECLARE_WRITE8_MEMBER(amspdwy_sound_w);
	DECLARE_READ8_MEMBER(amspdwy_port_r);
	DECLARE_WRITE8_MEMBER(amspdwy_paletteram_w);
	DECLARE_WRITE8_MEMBER(amspdwy_flipscreen_w);
	DECLARE_WRITE8_MEMBER(amspdwy_videoram_w);
	DECLARE_WRITE8_MEMBER(amspdwy_colorram_w);
};


/*----------- defined in video/amspdwy.c -----------*/


VIDEO_START( amspdwy );
SCREEN_UPDATE_IND16( amspdwy );
