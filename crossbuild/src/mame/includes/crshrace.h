/*----------- defined in video/crshrace.c -----------*/

extern UINT16 *crshrace_videoram1,*crshrace_videoram2;

WRITE16_HANDLER( crshrace_videoram1_w );
WRITE16_HANDLER( crshrace_videoram2_w );
WRITE16_HANDLER( crshrace_roz_bank_w );
WRITE16_HANDLER( crshrace_gfxctrl_w );

VIDEO_START( crshrace );
VIDEO_EOF( crshrace );
VIDEO_UPDATE( crshrace );
