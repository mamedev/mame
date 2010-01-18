
typedef struct _dec8_state dec8_state;
struct _dec8_state
{
	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  pf0_data;
	UINT8 *  pf1_data;
	UINT8 *  row;
//  UINT8 *  paletteram;    // currently this uses generic palette handling
//  UINT8 *  paletteram_2;  // currently this uses generic palette handling
//  UINT8 *  spriteram; // currently this uses buffered_spriteram in some games
	size_t   videoram_size;

	/* video-related */
	tilemap_t  *pf0_tilemap, *pf1_tilemap, *fix_tilemap;
	//int      scroll1[4];
	int      scroll2[4];
	int      pf0_control[0x20], pf1_control[0x20];
	int      gfx_mask, game_uses_priority;

	/* misc */
	int      nmi_enable, int_enable;
	int      i8751_return, i8751_value;
	int      coin1, coin2, latch, snd;
	int      msm5205next;
	int      toggle;

	/* devices */
	running_device *maincpu;
	running_device *subcpu;
	running_device *audiocpu;
	running_device *mcu;
};

/*----------- defined in video/dec8.c -----------*/


PALETTE_INIT( ghostb );
VIDEO_UPDATE( cobracom );
VIDEO_UPDATE( ghostb );
VIDEO_UPDATE( srdarwin );
VIDEO_UPDATE( gondo );
VIDEO_UPDATE( garyoret );
VIDEO_UPDATE( lastmiss );
VIDEO_UPDATE( shackled );
VIDEO_UPDATE( oscar );
VIDEO_START( cobracom );
VIDEO_START( oscar );
VIDEO_START( ghostb );
VIDEO_START( lastmiss );
VIDEO_START( shackled );
VIDEO_START( srdarwin );
VIDEO_START( gondo );
VIDEO_START( garyoret );

WRITE8_HANDLER( dec8_bac06_0_w );
WRITE8_HANDLER( dec8_bac06_1_w );
WRITE8_HANDLER( dec8_pf0_data_w );
WRITE8_HANDLER( dec8_pf1_data_w );
READ8_HANDLER( dec8_pf0_data_r );
READ8_HANDLER( dec8_pf1_data_r );
WRITE8_HANDLER( srdarwin_videoram_w );
WRITE8_HANDLER( dec8_scroll2_w );
WRITE8_HANDLER( srdarwin_control_w );
WRITE8_HANDLER( gondo_scroll_w );
WRITE8_HANDLER( shackled_control_w );
WRITE8_HANDLER( lastmiss_control_w );
WRITE8_HANDLER( lastmiss_scrollx_w );
WRITE8_HANDLER( lastmiss_scrolly_w );
WRITE8_HANDLER( dec8_videoram_w );
