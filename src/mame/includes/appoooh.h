

typedef struct _appoooh_state appoooh_state;
struct _appoooh_state
{
	/* memory pointers */
	UINT8 *  bg_videoram;
	UINT8 *  bg_colorram;
	UINT8 *  fg_videoram;
	UINT8 *  fg_colorram;
	UINT8 *  spriteram;
	UINT8 *  spriteram_2;

	/* video-related */
	tilemap_t  *fg_tilemap, *bg_tilemap;
	int scroll_x;
	int priority;

	/* sound-related */
	UINT32   adpcm_data;
	UINT32   adpcm_address;

	/* devices */
	const device_config *adpcm;
};

#define CHR1_OFST   0x00  /* palette page of char set #1 */
#define CHR2_OFST   0x10  /* palette page of char set #2 */


/* ----------- defined in video/appoooh.c -----------*/

WRITE8_HANDLER( appoooh_fg_videoram_w );
WRITE8_HANDLER( appoooh_fg_colorram_w );
WRITE8_HANDLER( appoooh_bg_videoram_w );
WRITE8_HANDLER( appoooh_bg_colorram_w );
PALETTE_INIT( appoooh );
PALETTE_INIT( robowres );
WRITE8_HANDLER( appoooh_scroll_w );
WRITE8_HANDLER( appoooh_out_w );
VIDEO_START( appoooh );
VIDEO_UPDATE( appoooh );
VIDEO_UPDATE( robowres );
