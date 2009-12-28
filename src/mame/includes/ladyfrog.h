/*************************************************************************

    Lady Frog

*************************************************************************/

typedef struct _ladyfrog_state ladyfrog_state;
struct _ladyfrog_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	UINT8 *    scrlram;
//      UINT8 *    paletteram;    // currently this uses generic palette handling
//      UINT8 *    paletteram2;   // currently this uses generic palette handling
	size_t     videoram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;
	int        tilebank, palette_bank;
	int        spritetilebase;

	/* misc */
	int        sound_nmi_enable, pending_nmi;
	int        snd_flag;
	UINT8      snd_data;

	/* devices */
	const device_config *audiocpu;
};


/*----------- defined in video/ladyfrog.c -----------*/

WRITE8_HANDLER( ladyfrog_videoram_w );
WRITE8_HANDLER( ladyfrog_spriteram_w );
WRITE8_HANDLER( ladyfrog_palette_w );
WRITE8_HANDLER( ladyfrog_gfxctrl_w );
WRITE8_HANDLER( ladyfrog_gfxctrl2_w );
WRITE8_HANDLER( ladyfrog_scrlram_w );

READ8_HANDLER( ladyfrog_spriteram_r );
READ8_HANDLER( ladyfrog_palette_r );
READ8_HANDLER( ladyfrog_scrlram_r );
READ8_HANDLER( ladyfrog_videoram_r );

VIDEO_START( ladyfrog );
VIDEO_START( toucheme );
VIDEO_UPDATE( ladyfrog );
