class liberate_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, liberate_state(machine)); }

	liberate_state(running_machine &machine)
		: driver_data_t(machine) { }

	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *paletteram;
	UINT8 *spriteram;
	UINT8 *scratchram;
	UINT8 *charram;	/* prosoccr */
	UINT8 *bg_vram; /* prosport */

	UINT8 io_ram[16];

	int bank;
	int latch;
	UINT8 gfx_rom_readback;
	int background_color;
	int background_disable;

	tilemap_t *back_tilemap;
	tilemap_t *fix_tilemap;

	running_device *maincpu;
	running_device *audiocpu;
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

