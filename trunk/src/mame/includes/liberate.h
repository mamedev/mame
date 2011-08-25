class liberate_state : public driver_device
{
public:
	liberate_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_paletteram;
	UINT8 *m_spriteram;
	UINT8 *m_scratchram;
	UINT8 *m_charram;	/* prosoccr */
	UINT8 *m_bg_vram; /* prosport */

	UINT8 m_io_ram[16];

	int m_bank;
	int m_latch;
	UINT8 m_gfx_rom_readback;
	int m_background_color;
	int m_background_disable;

	tilemap_t *m_back_tilemap;
	tilemap_t *m_fix_tilemap;

	device_t *m_maincpu;
	device_t *m_audiocpu;
};


/*----------- defined in video/liberate.c -----------*/

PALETTE_INIT( liberate );
SCREEN_UPDATE( prosoccr );
SCREEN_UPDATE( prosport );
SCREEN_UPDATE( liberate );
SCREEN_UPDATE( boomrang );
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

