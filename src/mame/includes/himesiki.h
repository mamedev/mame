/*************************************************************************

    Himeshikibu

*************************************************************************/

class himesiki_state : public driver_device
{
public:
	himesiki_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_bg_ram;
	UINT8 *    m_spriteram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int 	     m_scrollx[2];
	int        m_flipscreen;

	/* devices */
	device_t *m_subcpu;
	DECLARE_WRITE8_MEMBER(himesiki_rombank_w);
	DECLARE_WRITE8_MEMBER(himesiki_sound_w);
	DECLARE_WRITE8_MEMBER(himesiki_bg_ram_w);
	DECLARE_WRITE8_MEMBER(himesiki_scrollx_w);
	DECLARE_WRITE8_MEMBER(himesiki_flip_w);
};


/*----------- defined in video/himesiki.c -----------*/

VIDEO_START( himesiki );
SCREEN_UPDATE_IND16( himesiki );

