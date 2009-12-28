/***************************************************************************

    Black Tiger

***************************************************************************/

typedef struct _blktiger_state blktiger_state;
struct _blktiger_state
{
	/* memory pointers */
	UINT8 * txvideoram;
//  UINT8 * spriteram;  // currently this uses generic buffer_spriteram_w
//  UINT8 * paletteram; // currently this uses generic palette handling
//  UINT8 * paletteram2;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t *tx_tilemap, *bg_tilemap8x4, *bg_tilemap4x8;
	UINT32  scroll_bank;
	UINT8   scroll_x[2];
	UINT8   scroll_y[2];
	UINT8   *scroll_ram;
	UINT8   screen_layout;
	UINT8   chon, objon, bgon;

	/* mcu-related */
	UINT8   z80_latch, i8751_latch;

	/* devices */
	const device_config *mcu;
	const device_config *audiocpu;
};


/*----------- defined in video/blktiger.c -----------*/

WRITE8_HANDLER( blktiger_screen_layout_w );

READ8_HANDLER( blktiger_bgvideoram_r );
WRITE8_HANDLER( blktiger_bgvideoram_w );
WRITE8_HANDLER( blktiger_txvideoram_w );
WRITE8_HANDLER( blktiger_video_control_w );
WRITE8_HANDLER( blktiger_video_enable_w );
WRITE8_HANDLER( blktiger_bgvideoram_bank_w );
WRITE8_HANDLER( blktiger_scrollx_w );
WRITE8_HANDLER( blktiger_scrolly_w );

VIDEO_START( blktiger );
VIDEO_UPDATE( blktiger );
VIDEO_EOF( blktiger );
