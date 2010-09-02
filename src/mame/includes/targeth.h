/*----------- defined in video/targeth.c -----------*/

extern UINT16 *targeth_vregs;
extern UINT16 *targeth_videoram;
extern UINT16 *targeth_spriteram;

WRITE16_HANDLER( targeth_vram_w );
VIDEO_START( targeth );
VIDEO_UPDATE( targeth );
