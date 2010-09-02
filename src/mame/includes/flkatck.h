/*************************************************************************

    Flak Attack / MX5000

*************************************************************************/

class flkatck_state : public driver_device
{
public:
	flkatck_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    k007121_ram;
//  UINT8 *    paletteram;  // this currently uses generic palette handling

	/* video-related */
	tilemap_t    *k007121_tilemap[2];
	int        flipscreen;

	/* misc */
	int        irq_enabled;
	int        multiply_reg[2];

	/* devices */
	running_device *audiocpu;
	running_device *k007121;
};


//static rectangle k007121_clip[2];


/*----------- defined in video/flkatck.c -----------*/

WRITE8_HANDLER( flkatck_k007121_w );
WRITE8_HANDLER( flkatck_k007121_regs_w );

VIDEO_START( flkatck );
VIDEO_UPDATE( flkatck );
