class pacland_state : public driver_device
{
public:
	pacland_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_videoram;
	UINT8 *m_videoram2;
	UINT8 *m_spriteram;
	UINT8 m_palette_bank;
	const UINT8 *m_color_prom;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	bitmap_t *m_fg_bitmap;
	UINT32 *m_transmask[3];
	UINT16 m_scroll0;
	UINT16 m_scroll1;
};


/*----------- defined in video/pacland.c -----------*/

WRITE8_HANDLER( pacland_videoram_w );
WRITE8_HANDLER( pacland_videoram2_w );
WRITE8_HANDLER( pacland_scroll0_w );
WRITE8_HANDLER( pacland_scroll1_w );
WRITE8_HANDLER( pacland_bankswitch_w );

PALETTE_INIT( pacland );
VIDEO_START( pacland );
SCREEN_UPDATE( pacland );
