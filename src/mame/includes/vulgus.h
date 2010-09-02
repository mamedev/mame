/*----------- defined in video/vulgus.c -----------*/

extern UINT8 *vulgus_fgvideoram;
extern UINT8 *vulgus_bgvideoram;
extern UINT8 *vulgus_scroll_low,*vulgus_scroll_high;

WRITE8_HANDLER( vulgus_fgvideoram_w );
WRITE8_HANDLER( vulgus_bgvideoram_w );
WRITE8_HANDLER( vulgus_c804_w );
WRITE8_HANDLER( vulgus_palette_bank_w );

VIDEO_START( vulgus );
PALETTE_INIT( vulgus );
VIDEO_UPDATE( vulgus );
