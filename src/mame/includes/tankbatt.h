/*----------- defined in video/tankbatt.c -----------*/

extern UINT8 *tankbatt_bulletsram;
extern size_t tankbatt_bulletsram_size;

WRITE8_HANDLER( tankbatt_videoram_w );

PALETTE_INIT( tankbatt );
VIDEO_START( tankbatt );
VIDEO_UPDATE( tankbatt );
