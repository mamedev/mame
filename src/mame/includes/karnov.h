/*************************************************************************

    Karnov - Wonder Planet - Chelnov

*************************************************************************/

typedef struct _karnov_state karnov_state;
struct _karnov_state
{
	/* memory pointers */
	UINT16 *    videoram;
	UINT16 *    ram;
	UINT16 *    pf_data;
//  UINT16 *    spriteram;  // currently this uses generic buffered spriteram

	/* video-related */
	bitmap_t    *bitmap_f;
	tilemap     *fix_tilemap;
	int         flipscreen;
	UINT16      scroll[2];

	/* misc */
	UINT16      i8751_return, i8751_needs_ack, i8751_coin_pending, i8751_command_queue;
	int         i8751_level;	// needed by chelnov
	int         microcontroller_id, coin_mask;
	int         latch;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
};

enum {
	KARNOV = 0,
	KARNOVJ,
	CHELNOV,
	CHELNOVJ,
	CHELNOVW,
	WNDRPLNT
};


/*----------- defined in video/karnov.c -----------*/

WRITE16_HANDLER( karnov_playfield_swap_w );
WRITE16_HANDLER( karnov_videoram_w );

void karnov_flipscreen_w(running_machine *machine, int data);

PALETTE_INIT( karnov );
VIDEO_START( karnov );
VIDEO_START( wndrplnt );
VIDEO_UPDATE( karnov );
