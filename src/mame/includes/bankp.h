/***************************************************************************

    Bank Panic

***************************************************************************/


#define BANKP_MASTER_CLOCK 15468000
#define BANKP_CPU_CLOCK (BANKP_MASTER_CLOCK/6)
#define BANKP_SN76496_CLOCK (BANKP_MASTER_CLOCK/6)

class bankp_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, bankp_state(machine)); }

	bankp_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 * videoram;
	UINT8 * colorram;
	UINT8 * videoram2;
	UINT8 * colorram2;

	/* video-related */
	tilemap_t *bg_tilemap, *fg_tilemap;
	int     scroll_x, priority;
};


/*----------- defined in video/bankp.c -----------*/

WRITE8_HANDLER( bankp_videoram_w );
WRITE8_HANDLER( bankp_colorram_w );
WRITE8_HANDLER( bankp_videoram2_w );
WRITE8_HANDLER( bankp_colorram2_w );
WRITE8_HANDLER( bankp_scroll_w );
WRITE8_HANDLER( bankp_out_w );

PALETTE_INIT( bankp );
VIDEO_START( bankp );
VIDEO_UPDATE( bankp );


