/*----------- defined in video/tubep.c -----------*/

void tubep_vblank_end(void);
PALETTE_INIT( tubep );
VIDEO_UPDATE( tubep );
PALETTE_INIT( rjammer );
VIDEO_UPDATE( rjammer );
VIDEO_START( tubep );
VIDEO_RESET( tubep );

extern UINT8 *rjammer_backgroundram;
extern UINT8 *tubep_backgroundram;
extern UINT8 *tubep_textram;
extern UINT8 *tubep_sprite_colorsharedram;

extern WRITE8_HANDLER( tubep_textram_w );
extern WRITE8_HANDLER( rjammer_background_LS377_w );
extern WRITE8_HANDLER( rjammer_background_page_w );

extern WRITE8_HANDLER( tubep_colorproms_A4_line_w );
extern WRITE8_HANDLER( tubep_background_romselect_w );
extern WRITE8_HANDLER( tubep_background_a000_w );
extern WRITE8_HANDLER( tubep_background_c000_w );

extern WRITE8_HANDLER( tubep_sprite_control_w );

