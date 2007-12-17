/*************************************************************************

    Atari Return of the Jedi hardware

*************************************************************************/

/*----------- defined in video/jedi.c -----------*/

extern UINT8 *jedi_PIXIRAM;
extern UINT8 *jedi_backgroundram;
extern size_t jedi_backgroundram_size;

VIDEO_START( jedi );
VIDEO_UPDATE( jedi );

WRITE8_HANDLER( jedi_alpha_banksel_w );
WRITE8_HANDLER( jedi_paletteram_w );
WRITE8_HANDLER( jedi_backgroundram_w );
WRITE8_HANDLER( jedi_vscroll_w );
WRITE8_HANDLER( jedi_hscroll_w );
WRITE8_HANDLER( jedi_video_off_w );
WRITE8_HANDLER( jedi_PIXIRAM_w );
