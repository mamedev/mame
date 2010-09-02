

class fuuki16_state : public driver_device
{
public:
	fuuki16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    vram_0;
	UINT16 *    vram_1;
	UINT16 *    vram_2;
	UINT16 *    vram_3;
	UINT16 *    vregs;
	UINT16 *    priority;
	UINT16 *    unknown;
	UINT16 *    spriteram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *tilemap_0, *tilemap_1, *tilemap_2, *tilemap_3;

	/* misc */
	emu_timer   *raster_interrupt_timer;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/fuukifg2.c -----------*/

WRITE16_HANDLER( fuuki16_vram_0_w );
WRITE16_HANDLER( fuuki16_vram_1_w );
WRITE16_HANDLER( fuuki16_vram_2_w );
WRITE16_HANDLER( fuuki16_vram_3_w );

VIDEO_START( fuuki16 );
VIDEO_UPDATE( fuuki16 );
