/*----------- defined in video/tceptor.c -----------*/

PALETTE_INIT( tceptor );
VIDEO_START( tceptor );
VIDEO_UPDATE( tceptor );
VIDEO_EOF( tceptor );

WRITE8_HANDLER( tceptor_tile_ram_w );
WRITE8_HANDLER( tceptor_tile_attr_w );
WRITE8_HANDLER( tceptor_bg_ram_w );
WRITE8_HANDLER( tceptor_bg_scroll_w );

extern UINT8 *tceptor_tile_ram;
extern UINT8 *tceptor_tile_attr;
extern UINT8 *tceptor_bg_ram;
extern UINT16 *tceptor_sprite_ram;
