/*************************************************************************

    Ojanko High School & other Video System mahjong series

*************************************************************************/

class ojankohs_state : public driver_device
{
public:
	ojankohs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *   m_videoram;
	UINT8 *   m_colorram;
	UINT8 *   m_paletteram;

	/* video-related */
	tilemap_t  *m_tilemap;
	bitmap_t   *m_tmpbitmap;
	int       m_gfxreg;
	int       m_flipscreen;
	int       m_flipscreen_old;
	int       m_scrollx;
	int       m_scrolly;
	int       m_screen_refresh;

	/* misc */
	int       m_portselect;
	int       m_adpcm_reset;
	int       m_adpcm_data;
	int       m_vclk_left;

	/* devices */
	device_t *m_maincpu;
	device_t *m_msm;
};


/*----------- defined in video/ojankohs.c -----------*/

WRITE8_HANDLER( ojankohs_palette_w );
WRITE8_HANDLER( ccasino_palette_w );
WRITE8_HANDLER( ojankohs_videoram_w );
WRITE8_HANDLER( ojankohs_colorram_w );
WRITE8_HANDLER( ojankohs_gfxreg_w );
WRITE8_HANDLER( ojankohs_flipscreen_w );
WRITE8_HANDLER( ojankoc_palette_w );
WRITE8_HANDLER( ojankoc_videoram_w );

PALETTE_INIT( ojankoy );

VIDEO_START( ojankohs );
VIDEO_START( ojankoy );
VIDEO_START( ojankoc );

SCREEN_UPDATE( ojankohs );
SCREEN_UPDATE( ojankoc );

void ojankoc_flipscreen(address_space *space, int data);

