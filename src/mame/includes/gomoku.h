#include "devlegcy.h"

/*----------- defined in audio/gomoku.c -----------*/

extern UINT8 *gomoku_soundregs1;
extern UINT8 *gomoku_soundregs2;

WRITE8_HANDLER( gomoku_sound1_w );
WRITE8_HANDLER( gomoku_sound2_w );

DECLARE_LEGACY_SOUND_DEVICE(GOMOKU, gomoku_sound);


/*----------- defined in video/gomoku.c -----------*/

extern UINT8 *gomoku_videoram;
extern UINT8 *gomoku_colorram;
extern UINT8 *gomoku_bgram;

PALETTE_INIT( gomoku );
VIDEO_START( gomoku );
VIDEO_UPDATE( gomoku );

WRITE8_HANDLER( gomoku_videoram_w );
WRITE8_HANDLER( gomoku_colorram_w );
WRITE8_HANDLER( gomoku_bgram_w );
WRITE8_HANDLER( gomoku_flipscreen_w );
WRITE8_HANDLER( gomoku_bg_dispsw_w );
