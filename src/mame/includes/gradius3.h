/*************************************************************************

    Gradius 3

*************************************************************************/

class gradius3_state : public driver_device
{
public:
	gradius3_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    gfxram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         layer_colorbase[3], sprite_colorbase;

	/* misc */
	int         priority;
	int         irqAen, irqBmask;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *subcpu;
	device_t *k007232;
	device_t *k052109;
	device_t *k051960;
};

/*----------- defined in video/gradius3.c -----------*/

extern void gradius3_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);
extern void gradius3_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);

READ16_HANDLER( gradius3_gfxrom_r );
WRITE16_HANDLER( gradius3_gfxram_w );

VIDEO_START( gradius3 );
VIDEO_UPDATE( gradius3 );
