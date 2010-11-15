class tutankhm_state : public driver_device
{
public:
	tutankhm_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  paletteram;
	UINT8 *  scroll;

	/* video-related */
	tilemap_t  *bg_tilemap;
	UINT8     flip_x, flip_y;

	/* misc */
	UINT8    irq_toggle, irq_enable;

	/* devices */
	cpu_device *maincpu;
};


/*----------- defined in video/tutankhm.c -----------*/

WRITE8_HANDLER( tutankhm_flip_screen_x_w );
WRITE8_HANDLER( tutankhm_flip_screen_y_w );

VIDEO_UPDATE( tutankhm );
