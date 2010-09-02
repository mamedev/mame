/*----------- defined in video/namcos86.c -----------*/

extern UINT8 *rthunder_videoram1, *rthunder_videoram2, *rthunder_spriteram;

PALETTE_INIT( namcos86 );
VIDEO_START( namcos86 );
VIDEO_UPDATE( namcos86 );
VIDEO_EOF( namcos86 );

READ8_HANDLER( rthunder_videoram1_r );
WRITE8_HANDLER( rthunder_videoram1_w );
READ8_HANDLER( rthunder_videoram2_r );
WRITE8_HANDLER( rthunder_videoram2_w );
WRITE8_HANDLER( rthunder_scroll0_w );
WRITE8_HANDLER( rthunder_scroll1_w );
WRITE8_HANDLER( rthunder_scroll2_w );
WRITE8_HANDLER( rthunder_scroll3_w );
WRITE8_HANDLER( rthunder_backcolor_w );
WRITE8_HANDLER( rthunder_tilebank_select_w );
READ8_HANDLER( rthunder_spriteram_r );
WRITE8_HANDLER( rthunder_spriteram_w );
