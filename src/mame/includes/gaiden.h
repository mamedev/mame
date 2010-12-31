/***************************************************************************

    Ninja Gaiden

***************************************************************************/

class gaiden_state : public driver_device
{
public:
	gaiden_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    videoram;
	UINT16 *    videoram2;
	UINT16 *    videoram3;
	UINT16 *    spriteram;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t   *text_layer,*foreground,*background;
	bitmap_t    *sprite_bitmap, *tile_bitmap_bg, *tile_bitmap_fg;
	UINT16      tx_scroll_x, tx_scroll_y;
	UINT16      bg_scroll_x, bg_scroll_y;
	UINT16      fg_scroll_x, fg_scroll_y;
	INT8		tx_offset_y, bg_offset_y, fg_offset_y, spr_offset_y;

	/* misc */
	int         sprite_sizey;
	int         prot, jumpcode;
	const int   *raiga_jumppoints;

	/* devices */
	device_t *audiocpu;
};


/*----------- defined in video/gaiden.c -----------*/

VIDEO_START( gaiden );
VIDEO_START( raiga );
VIDEO_START( drgnbowl );
VIDEO_START( mastninj );

VIDEO_UPDATE( gaiden );
VIDEO_UPDATE( raiga );
VIDEO_UPDATE( drgnbowl );

WRITE16_HANDLER( gaiden_videoram_w );
WRITE16_HANDLER( gaiden_videoram2_w );
READ16_HANDLER( gaiden_videoram2_r );
WRITE16_HANDLER( gaiden_videoram3_w );
READ16_HANDLER( gaiden_videoram3_r );

WRITE16_HANDLER( gaiden_flip_w );
WRITE16_HANDLER( gaiden_txscrollx_w );
WRITE16_HANDLER( gaiden_txscrolly_w );
WRITE16_HANDLER( gaiden_fgscrollx_w );
WRITE16_HANDLER( gaiden_fgscrolly_w );
WRITE16_HANDLER( gaiden_bgscrollx_w );
WRITE16_HANDLER( gaiden_bgscrolly_w );
WRITE16_HANDLER( gaiden_txoffsety_w );
WRITE16_HANDLER( gaiden_fgoffsety_w );
WRITE16_HANDLER( gaiden_bgoffsety_w );
WRITE16_HANDLER( gaiden_sproffsety_w );
