/*----------- defined in video/galpanic.c -----------*/

extern UINT16 *galpanic_bgvideoram,*galpanic_fgvideoram;
extern size_t galpanic_fgvideoram_size;

PALETTE_INIT( galpanic );
WRITE16_HANDLER( galpanic_bgvideoram_w );
WRITE16_HANDLER( galpanic_paletteram_w );
VIDEO_START( galpanic );
VIDEO_UPDATE( galpanic );
VIDEO_UPDATE( comad );


