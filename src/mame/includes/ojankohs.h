/*************************************************************************

    Ojanko High School & other Video System mahjong series

*************************************************************************/

class ojankohs_state : public driver_device
{
public:
	ojankohs_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *   videoram;
	UINT8 *   colorram;
	UINT8 *   paletteram;

	/* video-related */
	tilemap_t  *tilemap;
	bitmap_t   *tmpbitmap;
	int       gfxreg;
	int       flipscreen, flipscreen_old;
	int       scrollx, scrolly;
	int       screen_refresh;

	/* misc */
	int       portselect;
	int       adpcm_reset;
	int       adpcm_data;
	int       vclk_left;

	/* devices */
	device_t *maincpu;
	device_t *msm;
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

VIDEO_UPDATE( ojankohs );
VIDEO_UPDATE( ojankoc );

void ojankoc_flipscreen(address_space *space, int data);

