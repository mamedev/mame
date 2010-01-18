/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/


#define CPUTAG_MCU "mcu"


typedef struct _champbas_state champbas_state;
struct _champbas_state
{
	/* memory pointers */
	UINT8 *        bg_videoram;
	UINT8 *        spriteram;
	UINT8 *        spriteram_2;
	size_t         spriteram_size;

	/* video-related */
	tilemap_t        *bg_tilemap;
	UINT8          gfx_bank;
	UINT8          palette_bank;

	/* misc */
	int            watchdog_count;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *mcu;
};


/*----------- defined in video/champbas.c -----------*/

WRITE8_HANDLER( champbas_bg_videoram_w );
WRITE8_HANDLER( champbas_gfxbank_w );
WRITE8_HANDLER( champbas_palette_bank_w );
WRITE8_HANDLER( champbas_flipscreen_w );

PALETTE_INIT( champbas );
PALETTE_INIT( exctsccr );
VIDEO_START( champbas );
VIDEO_START( exctsccr );
VIDEO_UPDATE( champbas );
VIDEO_UPDATE( exctsccr );


