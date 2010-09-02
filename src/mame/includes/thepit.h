/*----------- defined in video/thepit.c -----------*/

extern UINT8 *thepit_videoram;
extern UINT8 *thepit_colorram;
extern UINT8 *thepit_attributesram;
extern UINT8 *thepit_spriteram;
extern size_t thepit_spriteram_size;

PALETTE_INIT( thepit );
PALETTE_INIT( suprmous );
VIDEO_START( thepit );
VIDEO_UPDATE( thepit );
WRITE8_HANDLER( thepit_videoram_w );
WRITE8_HANDLER( thepit_colorram_w );
WRITE8_HANDLER( thepit_flip_screen_x_w );
WRITE8_HANDLER( thepit_flip_screen_y_w );
READ8_HANDLER( thepit_input_port_0_r );
WRITE8_HANDLER( intrepid_graphics_bank_w );
