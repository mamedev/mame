/*----------- defined in video/realbrk.c -----------*/

#ifndef REALBRK_H
#define REALBRK_H

VIDEO_START(realbrk);
VIDEO_UPDATE(realbrk);
VIDEO_UPDATE(dai2kaku);

WRITE16_HANDLER( realbrk_vram_0_w );
WRITE16_HANDLER( realbrk_vram_1_w );
WRITE16_HANDLER( realbrk_vram_2_w );
WRITE16_HANDLER( realbrk_vregs_w );
WRITE16_HANDLER( realbrk_flipscreen_w );
WRITE16_HANDLER( dai2kaku_flipscreen_w );

//extern UINT16 *realbrk_vram_0, *realbrk_vram_1, *realbrk_vram_2, *realbrk_vregs;
extern UINT16 *realbrk_vram_0, *realbrk_vram_1, *realbrk_vram_2, *realbrk_vregs, *realbrk_vram_0ras, *realbrk_vram_1ras;

#endif

