/***************************************************************************

  Fast Freddie/Jump Coaster hardware
  driver by Zsolt Vasvari

***************************************************************************/

/*----------- defined in video/fastfred.c -----------*/

extern UINT8 *fastfred_videoram;
extern UINT8 *fastfred_spriteram;
extern size_t fastfred_spriteram_size;
extern UINT8 *fastfred_attributesram;

PALETTE_INIT( fastfred );
VIDEO_START( fastfred );
WRITE8_HANDLER( fastfred_videoram_w );
WRITE8_HANDLER( fastfred_attributes_w );
WRITE8_HANDLER( fastfred_charbank1_w );
WRITE8_HANDLER( fastfred_charbank2_w );
WRITE8_HANDLER( fastfred_colorbank1_w );
WRITE8_HANDLER( fastfred_colorbank2_w );
WRITE8_HANDLER( fastfred_background_color_w );
WRITE8_HANDLER( fastfred_flip_screen_x_w );
WRITE8_HANDLER( fastfred_flip_screen_y_w );
VIDEO_UPDATE( fastfred );

extern UINT8 *imago_fg_videoram;
VIDEO_START( imago );
VIDEO_UPDATE( imago );
WRITE8_HANDLER( imago_fg_videoram_w );
WRITE8_HANDLER( imago_charbank_w );
