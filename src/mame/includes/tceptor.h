class tceptor_state : public driver_device
{
public:
	tceptor_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 m6809_irq_enable;
	UINT8 m68k_irq_enable;
	UINT8 mcu_irq_enable;
	UINT8 *m68k_shared_ram;
	UINT8 *tile_ram;
	UINT8 *tile_attr;
	UINT8 *bg_ram;
	UINT16 *sprite_ram;
	int sprite16;
	int sprite32;
	int bg;
	tilemap_t *tx_tilemap;
	tilemap_t *bg1_tilemap;
	tilemap_t *bg2_tilemap;
	INT32 bg1_scroll_x;
	INT32 bg1_scroll_y;
	INT32 bg2_scroll_x;
	INT32 bg2_scroll_y;
	bitmap_t *temp_bitmap;
	UINT16 *sprite_ram_buffered;
	int is_mask_spr[1024/16];
};


/*----------- defined in video/tceptor.c -----------*/

PALETTE_INIT( tceptor );
VIDEO_START( tceptor );
SCREEN_UPDATE( tceptor );
SCREEN_EOF( tceptor );

WRITE8_HANDLER( tceptor_tile_ram_w );
WRITE8_HANDLER( tceptor_tile_attr_w );
WRITE8_HANDLER( tceptor_bg_ram_w );
WRITE8_HANDLER( tceptor_bg_scroll_w );

