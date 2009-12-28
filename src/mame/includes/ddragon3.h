/*************************************************************************

    Double Dragon 3 & The Combatribes

*************************************************************************/


typedef struct _ddragon3_state ddragon3_state;
struct _ddragon3_state
{
	/* memory pointers */
	UINT16 *        bg_videoram;
	UINT16 *        fg_videoram;
	UINT16 *        spriteram;
//  UINT16 *        paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t         *fg_tilemap, *bg_tilemap;
	UINT16          vreg;
	UINT16          bg_scrollx;
	UINT16          bg_scrolly;
	UINT16          fg_scrollx;
	UINT16          fg_scrolly;
	UINT16          bg_tilebase;

	/* misc */
	UINT16          io_reg[8];

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
};


/*----------- defined in video/ddragon3.c -----------*/

extern WRITE16_HANDLER( ddragon3_bg_videoram_w );
extern WRITE16_HANDLER( ddragon3_fg_videoram_w );
extern WRITE16_HANDLER( ddragon3_scroll_w );
extern READ16_HANDLER( ddragon3_scroll_r );

extern VIDEO_START( ddragon3 );
extern VIDEO_UPDATE( ddragon3 );
extern VIDEO_UPDATE( ctribe );
