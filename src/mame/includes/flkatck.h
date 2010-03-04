/*************************************************************************

    Flak Attack / MX5000

*************************************************************************/

class flkatck_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, flkatck_state(machine)); }

	flkatck_state(running_machine &machine) { }

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
