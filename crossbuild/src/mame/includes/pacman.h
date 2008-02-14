/*************************************************************************

    Namco PuckMan

**************************************************************************/

/*----------- defined in video/pacman.c -----------*/

PALETTE_INIT( pacman );
VIDEO_START( pacman );
VIDEO_UPDATE( pacman );

WRITE8_HANDLER( pacman_videoram_w );
WRITE8_HANDLER( pacman_colorram_w );
WRITE8_HANDLER( pacman_flipscreen_w );


VIDEO_START( pengo );

WRITE8_HANDLER( pengo_palettebank_w );
WRITE8_HANDLER( pengo_colortablebank_w );
WRITE8_HANDLER( pengo_gfxbank_w );


WRITE8_HANDLER( vanvan_bgcolor_w );


VIDEO_START( s2650games );
VIDEO_UPDATE( s2650games );

extern UINT8 *s2650games_spriteram;
extern UINT8 *s2650games_tileram;

WRITE8_HANDLER( s2650games_videoram_w );
WRITE8_HANDLER( s2650games_colorram_w );
WRITE8_HANDLER( s2650games_scroll_w );
WRITE8_HANDLER( s2650games_tilesbank_w );


VIDEO_START( jrpacman );

WRITE8_HANDLER( jrpacman_videoram_w );
WRITE8_HANDLER( jrpacman_charbank_w );
WRITE8_HANDLER( jrpacman_spritebank_w );
WRITE8_HANDLER( jrpacman_scroll_w );
WRITE8_HANDLER( jrpacman_bgpriority_w );


/*----------- defined in machine/pacplus.c -----------*/

void pacplus_decode(void);


/*----------- defined in machine/jumpshot.c -----------*/

void jumpshot_decode(void);


/*----------- defined in machine/theglobp.c -----------*/

MACHINE_RESET( theglobp );
READ8_HANDLER( theglobp_decrypt_rom );


/*----------- defined in machine/mspacman.c -----------*/

MACHINE_RESET( mspacman );
WRITE8_HANDLER( mspacman_activate_rom );

/*----------- defined in machine/acitya.c -------------*/

MACHINE_RESET( acitya );
READ8_HANDLER( acitya_decrypt_rom );
