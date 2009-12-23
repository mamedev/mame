/*************************************************************************

    Flak Attack / MX5000

*************************************************************************/

typedef struct _flkatck_state flkatck_state;
struct _flkatck_state
{
	/* memory pointers */
	UINT8 *    k007121_ram;
//  UINT8 *    paletteram;  // this currently uses generic palette handling

	/* video-related */
	tilemap    *k007121_tilemap[2];
	int        flipscreen;

	/* misc */
	int        irq_enabled;
	int        multiply_reg[2];

	/* devices */
	const device_config *audiocpu;
	const device_config *k007121;
};


//static rectangle k007121_clip[2];


/*----------- defined in video/flkatck.c -----------*/

WRITE8_HANDLER( flkatck_k007121_w );
WRITE8_HANDLER( flkatck_k007121_regs_w );

VIDEO_START( flkatck );
VIDEO_UPDATE( flkatck );
