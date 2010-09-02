extern UINT16 *thoop2_vregs;
extern UINT16 *thoop2_videoram;
extern UINT16 *thoop2_spriteram;

WRITE16_HANDLER( thoop2_vram_w );
VIDEO_START( thoop2 );
VIDEO_UPDATE( thoop2 );
