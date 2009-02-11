/***************************************************************************

    Videa Gridlee hardware

    driver by Aaron Giles

***************************************************************************/


/*----------- defined in audio/gridlee.c -----------*/

WRITE8_HANDLER( gridlee_sound_w );

DEVICE_GET_INFO( gridlee_sound );
#define SOUND_GRIDLEE DEVICE_GET_INFO_NAME(gridlee_sound)


/*----------- defined in video/gridlee.c -----------*/

/* video driver data & functions */
extern UINT8 gridlee_cocktail_flip;

PALETTE_INIT( gridlee );
VIDEO_START( gridlee );
VIDEO_UPDATE( gridlee );

WRITE8_HANDLER( gridlee_cocktail_flip_w );
WRITE8_HANDLER( gridlee_videoram_w );
WRITE8_HANDLER( gridlee_palette_select_w );
