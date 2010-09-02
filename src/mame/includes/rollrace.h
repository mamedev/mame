/*----------- defined in video/rollrace.c -----------*/

extern UINT8 *rollrace_videoram;
extern UINT8 *rollrace_colorram;

VIDEO_UPDATE( rollrace );

WRITE8_HANDLER( rollrace_charbank_w );
WRITE8_HANDLER( rollrace_backgroundpage_w );
WRITE8_HANDLER( rollrace_backgroundcolor_w );
WRITE8_HANDLER( rollrace_bkgpen_w );
WRITE8_HANDLER( rollrace_flipy_w );
WRITE8_HANDLER( rollrace_spritebank_w );
WRITE8_HANDLER( rollrace_flipx_w );

