/***************************************************************************

    Sega Zaxxon hardware

***************************************************************************/

/*----------- defined in audio/zaxxon.c -----------*/

WRITE8_HANDLER( zaxxon_sound_a_w );
WRITE8_HANDLER( zaxxon_sound_b_w );
WRITE8_HANDLER( zaxxon_sound_c_w );

WRITE8_HANDLER( congo_sound_b_w );
WRITE8_HANDLER( congo_sound_c_w );

MACHINE_DRIVER_EXTERN( zaxxon_samples );
MACHINE_DRIVER_EXTERN( congo_samples );


/*----------- defined in video/zaxxon.c -----------*/

WRITE8_HANDLER( zaxxon_flipscreen_w );
WRITE8_HANDLER( zaxxon_fg_color_w );
WRITE8_HANDLER( zaxxon_bg_position_w );
WRITE8_HANDLER( zaxxon_bg_color_w );
WRITE8_HANDLER( zaxxon_bg_enable_w );

WRITE8_HANDLER( zaxxon_videoram_w );
WRITE8_HANDLER( congo_colorram_w );

WRITE8_HANDLER( congo_fg_bank_w );
WRITE8_HANDLER( congo_color_bank_w );
WRITE8_HANDLER( congo_sprite_custom_w );

PALETTE_INIT( zaxxon );

VIDEO_START( zaxxon );
VIDEO_START( razmataz );
VIDEO_START( congo );

VIDEO_UPDATE( zaxxon );
VIDEO_UPDATE( razmataz );
VIDEO_UPDATE( congo );
VIDEO_UPDATE( futspy );
