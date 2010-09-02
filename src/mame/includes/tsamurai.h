/*----------- defined in video/tsamurai.c -----------*/

extern UINT8 *tsamurai_videoram;
extern UINT8 *tsamurai_colorram;
extern UINT8 *tsamurai_bg_videoram;

WRITE8_HANDLER( vsgongf_color_w );

WRITE8_HANDLER( tsamurai_bgcolor_w );
WRITE8_HANDLER( tsamurai_textbank1_w );
WRITE8_HANDLER( tsamurai_textbank2_w );

WRITE8_HANDLER( tsamurai_scrolly_w );
WRITE8_HANDLER( tsamurai_scrollx_w );
WRITE8_HANDLER( tsamurai_bg_videoram_w );
WRITE8_HANDLER( tsamurai_fg_videoram_w );
WRITE8_HANDLER( tsamurai_fg_colorram_w );

VIDEO_START( tsamurai );
VIDEO_UPDATE( tsamurai );

VIDEO_START( vsgongf );
VIDEO_UPDATE( vsgongf );
