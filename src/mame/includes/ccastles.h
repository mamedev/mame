/*************************************************************************

    Atari Crystal Castles hardware

*************************************************************************/

typedef struct _ccastles_state ccastles_state;
struct _ccastles_state
{
	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  spriteram;
//  UINT8 *  nvram_stage;   // currently this uses generic nvram handlers
//  UINT8 *  nvram;     // currently this uses generic nvram handlers

	/* video-related */
	const UINT8 *syncprom;
	const UINT8 *wpprom;
	const UINT8 *priprom;
	bitmap_t *spritebitmap;
	double rweights[3], gweights[3], bweights[3];
	UINT8 video_control[8];
	UINT8 bitmode_addr[2];
	UINT8 hscroll;
	UINT8 vscroll;

	/* misc */
	int      vblank_start;
	int      vblank_end;
	emu_timer *irq_timer;
	UINT8    irq_state;
	UINT8    nvram_store[2];

	/* devices */
	const device_config *maincpu;
};


/*----------- defined in video/ccastles.c -----------*/


VIDEO_START( ccastles );
VIDEO_UPDATE( ccastles );

WRITE8_HANDLER( ccastles_hscroll_w );
WRITE8_HANDLER( ccastles_vscroll_w );
WRITE8_HANDLER( ccastles_video_control_w );

WRITE8_HANDLER( ccastles_paletteram_w );
WRITE8_HANDLER( ccastles_videoram_w );

READ8_HANDLER( ccastles_bitmode_r );
WRITE8_HANDLER( ccastles_bitmode_w );
WRITE8_HANDLER( ccastles_bitmode_addr_w );
