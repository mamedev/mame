/*----------- defined in video/vigilant.c -----------*/

VIDEO_START( vigilant );
VIDEO_RESET( vigilant );
WRITE8_HANDLER( vigilant_paletteram_w );
WRITE8_HANDLER( vigilant_horiz_scroll_w );
WRITE8_HANDLER( vigilant_rear_horiz_scroll_w );
WRITE8_HANDLER( vigilant_rear_color_w );
VIDEO_UPDATE( vigilant );
VIDEO_UPDATE( kikcubic );
