/*----------- defined in audio/suna8.c -----------*/

WRITE8_HANDLER( suna8_play_samples_w );
WRITE8_HANDLER( suna8_samples_number_w );
void suna8_sh_start(void);


/*----------- defined in video/suna8.c -----------*/

extern UINT8 suna8_rombank, suna8_spritebank, suna8_palettebank;
extern UINT8 suna8_unknown;

WRITE8_HANDLER( suna8_spriteram_w );			// for debug
WRITE8_HANDLER( suna8_banked_spriteram_w );	// for debug

READ8_HANDLER( suna8_banked_paletteram_r );
READ8_HANDLER( suna8_banked_spriteram_r );

WRITE8_HANDLER( brickzn_banked_paletteram_w );

VIDEO_START( suna8_textdim0 );
VIDEO_START( suna8_textdim8 );
VIDEO_START( suna8_textdim12 );
VIDEO_UPDATE( suna8 );
