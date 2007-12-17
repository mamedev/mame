/***************************************************************************

 Lasso and similar hardware

***************************************************************************/

/*----------- defined in video/lasso.c -----------*/

extern UINT8 *lasso_videoram;
extern UINT8 *lasso_colorram;
extern UINT8 *lasso_spriteram;
extern size_t lasso_spriteram_size;
extern UINT8 *lasso_bitmap_ram;
extern UINT8 *wwjgtin_track_scroll;

WRITE8_HANDLER( lasso_videoram_w );
WRITE8_HANDLER( lasso_colorram_w );
WRITE8_HANDLER( lasso_backcolor_w );
WRITE8_HANDLER( lasso_video_control_w );
WRITE8_HANDLER( wwjgtin_video_control_w );
WRITE8_HANDLER( pinbo_video_control_w );
WRITE8_HANDLER( wwjgtin_lastcolor_w );

PALETTE_INIT( lasso );
PALETTE_INIT( wwjgtin );

VIDEO_START( lasso );
VIDEO_START( wwjgtin );
VIDEO_START( pinbo );

VIDEO_UPDATE( lasso );
VIDEO_UPDATE( chameleo );
VIDEO_UPDATE( wwjgtin );
