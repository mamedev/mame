/*************************************************************************

    Combat School

*************************************************************************/

typedef struct _combatsc_state combatsc_state;
struct _combatsc_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    scrollram;
	UINT8 *    io_ram;
	UINT8 *    paletteram;
	UINT8 *    spriteram[2];

	/* video-related */
	tilemap_t *bg_tilemap[2], *textlayer;
	UINT8 scrollram0[0x40];
	UINT8 scrollram1[0x40];
	int priority;

	int  vreg;
	int  bank_select; /* 0x00..0x1f */
	int  video_circuit; /* 0 or 1 */
	UINT8 *page[2];

	/* misc */
	UINT8 pos[4],sign[4];
	int prot[2];
	int boost;
	emu_timer *interleave_timer;


	/* devices */
	running_device *audiocpu;
	running_device *k007121_1;
	running_device *k007121_2;
};


/*----------- defined in video/combatsc.c -----------*/

READ8_HANDLER( combatsc_video_r );
WRITE8_HANDLER( combatsc_video_w );

WRITE8_HANDLER( combatsc_pf_control_w );
READ8_HANDLER( combatsc_scrollram_r );
WRITE8_HANDLER( combatsc_scrollram_w );

PALETTE_INIT( combatsc );
PALETTE_INIT( combatscb );
VIDEO_START( combatsc );
VIDEO_START( combatscb );
VIDEO_UPDATE( combatscb );
VIDEO_UPDATE( combatsc );
