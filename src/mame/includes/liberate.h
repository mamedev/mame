typedef struct _liberate_state liberate_state;
struct _liberate_state
{
	int bank;
	int latch;
	UINT8 gfx_rom_readback;
	int background_color;
	int background_disable;
	tilemap_t *background_tilemap;
	tilemap_t *fix_tilemap;
	UINT8 io_ram[16];
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *scratchram;
	UINT8 *charram;	/* prosoccr */
	UINT8 *bg_vram; /* prosport */
};


/*----------- defined in video/liberate.c -----------*/

PALETTE_INIT( liberate );
VIDEO_UPDATE( prosoccr );
VIDEO_UPDATE( prosport );
VIDEO_UPDATE( liberate );
VIDEO_UPDATE( boomrang );
VIDEO_START( prosoccr );
VIDEO_START( prosport );
VIDEO_START( boomrang );
VIDEO_START( liberate );

WRITE8_HANDLER( deco16_io_w );
WRITE8_HANDLER( prosoccr_io_w );
WRITE8_HANDLER( prosport_io_w );
WRITE8_HANDLER( prosport_paletteram_w );
WRITE8_HANDLER( prosport_bg_vram_w );
WRITE8_HANDLER( liberate_videoram_w );
WRITE8_HANDLER( liberate_colorram_w );

