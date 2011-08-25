/***************************************************************************

    Blockout

***************************************************************************/

class blockout_state : public driver_device
{
public:
	blockout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 * m_videoram;
	UINT16 * m_frontvideoram;
	UINT16 * m_paletteram;

	/* video-related */
	bitmap_t *m_tmpbitmap;
	UINT16   m_color;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};


/*----------- defined in video/blockout.c -----------*/

WRITE16_HANDLER( blockout_videoram_w );
WRITE16_HANDLER( blockout_paletteram_w );
WRITE16_HANDLER( blockout_frontcolor_w );

VIDEO_START( blockout );
SCREEN_UPDATE( blockout );
