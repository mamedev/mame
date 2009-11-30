	/***************************************************************************

    ESD 16 Bit Games

***************************************************************************/

typedef struct _esd16_state esd16_state;
struct _esd16_state
{
	/* memory pointers */
	UINT16 *       vram_0;
	UINT16 *       vram_1;
	UINT16 *       scroll_0;
	UINT16 *       scroll_1;
	UINT16 *       spriteram;
	UINT16 *       head_layersize;
	UINT16 *       headpanic_platform_x;
	UINT16 *       headpanic_platform_y;
//  UINT16 *       paletteram;  // currently this uses generic palette handling
	size_t         spriteram_size;

	/* video-related */
	tilemap       *tilemap_0_16x16, *tilemap_1_16x16;
	tilemap       *tilemap_0, *tilemap_1;
	int           tilemap0_color;

	/* devices */
	const device_config *audio_cpu;
	const device_config *eeprom;
};


/*----------- defined in video/esd16.c -----------*/

WRITE16_HANDLER( esd16_vram_0_w );
WRITE16_HANDLER( esd16_vram_1_w );
WRITE16_HANDLER( esd16_tilemap0_color_w );

VIDEO_START( esd16 );
VIDEO_UPDATE( esd16 );
VIDEO_UPDATE( hedpanic );
VIDEO_UPDATE( hedpanio );
