
#define PIXMAP_COLOR_BASE  (16 + 32)
#define BITMAPRAM_SIZE      0x6000


class dogfgt_state : public driver_device
{
public:
	dogfgt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    bgvideoram;
	UINT8 *    spriteram;
	UINT8 *    sharedram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling
	size_t     spriteram_size;

	/* video-related */
	bitmap_t  *pixbitmap;
	tilemap_t   *bg_tilemap;
	UINT8     *bitmapram;
	int       bm_plane, pixcolor;
	int       scroll[4];
	int       lastflip, lastpixcolor;

	/* sound-related */
	int       soundlatch, last_snd_ctrl;

	/* devices */
	device_t *subcpu;
};


/*----------- defined in video/dogfgt.c -----------*/

WRITE8_HANDLER( dogfgt_plane_select_w );
READ8_HANDLER( dogfgt_bitmapram_r );
WRITE8_HANDLER( dogfgt_bitmapram_w );
WRITE8_HANDLER( dogfgt_bgvideoram_w );
WRITE8_HANDLER( dogfgt_scroll_w );
WRITE8_HANDLER( dogfgt_1800_w );

PALETTE_INIT( dogfgt );
VIDEO_START( dogfgt );
VIDEO_UPDATE( dogfgt );
