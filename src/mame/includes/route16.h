/*----------- defined in video/route16.c -----------*/

extern UINT8 *route16_videoram1;
extern UINT8 *route16_videoram2;
extern size_t route16_videoram_size;

WRITE8_HANDLER( route16_out0_w );
WRITE8_HANDLER( route16_out1_w );
VIDEO_UPDATE( route16 );
VIDEO_UPDATE( stratvox );
VIDEO_UPDATE( ttmahjng );
