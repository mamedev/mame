/*************************************************************************

    Yun Sung 16 Bit Games

*************************************************************************/

class yunsun16_state : public driver_device
{
public:
	yunsun16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    vram_0;
	UINT16 *    vram_1;
	UINT16 *    scrollram_0;
	UINT16 *    scrollram_1;
	UINT16 *    priorityram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	UINT16 *    spriteram;
	size_t      spriteram_size;

	/* other video-related elements */
	tilemap_t     *tilemap_0, *tilemap_1;
	int         sprites_scrolldx, sprites_scrolldy;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/yunsun16.c -----------*/

WRITE16_HANDLER( yunsun16_vram_0_w );
WRITE16_HANDLER( yunsun16_vram_1_w );

VIDEO_START( yunsun16 );
VIDEO_UPDATE( yunsun16 );
