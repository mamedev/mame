/*----------- defined in video/powerins.c -----------*/

extern UINT16 *powerins_vram_0, *powerins_vctrl_0;
extern UINT16 *powerins_vram_1, *powerins_vctrl_1;

WRITE16_HANDLER( powerins_flipscreen_w );
WRITE16_HANDLER( powerins_tilebank_w );

WRITE16_HANDLER( powerins_paletteram16_w );

WRITE16_HANDLER( powerins_vram_0_w );
WRITE16_HANDLER( powerins_vram_1_w );

VIDEO_START( powerins );
VIDEO_UPDATE( powerins );
