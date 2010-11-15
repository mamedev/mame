class pooyan_state : public driver_device
{
public:
	pooyan_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;
	UINT8 *  spriteram;
	UINT8 *  spriteram2;

	/* video-related */
	tilemap_t  *bg_tilemap;

	/* misc */
	UINT8    irq_toggle, irq_enable;

	/* devices */
	cpu_device *maincpu;
};


/*----------- defined in video/pooyan.c -----------*/

WRITE8_HANDLER( pooyan_videoram_w );
WRITE8_HANDLER( pooyan_colorram_w );
WRITE8_HANDLER( pooyan_flipscreen_w );

PALETTE_INIT( pooyan );
VIDEO_START( pooyan );
VIDEO_UPDATE( pooyan );
