class deco32_state : public driver_device
{
public:
	deco32_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 *ram;
	int raster_enable;
	timer_device *raster_irq_timer;
	UINT8 nslasher_sound_irq;
	int strobe;
	int dragngun_lightgun_port;
	int tattass_eprom_bit;
	int lastClock;
	char buffer[32];
	int bufPtr;
	int pendingCommand;
	int readBitCount;
	int byteAddr;
	UINT8 bsmt_latch;
	UINT8 bsmt_reset;
	UINT32 *pf1_data;
	UINT32 *pf2_data;
	UINT32 *pf3_data;
	UINT32 *pf4_data;
	UINT32 *pf12_control;
	UINT32 *pf34_control;
	UINT32 *pf1_rowscroll;
	UINT32 *pf2_rowscroll;
	UINT32 *pf3_rowscroll;
	UINT32 *pf4_rowscroll;
	UINT32 *dragngun_sprite_layout_0_ram;
	UINT32 *dragngun_sprite_layout_1_ram;
	UINT32 *dragngun_sprite_lookup_0_ram;
	UINT32 *dragngun_sprite_lookup_1_ram;
	UINT32 *ace_ram;
	UINT16 *raster_display_list;
	int raster_display_position;
	UINT8 *dirty_palette;
	tilemap_t *pf1_tilemap;
	tilemap_t *pf1a_tilemap;
	tilemap_t *pf2_tilemap;
	tilemap_t *pf3_tilemap;
	tilemap_t *pf4_tilemap;
	int pf1_bank;
	int pf2_bank;
	int pf3_bank;
	int pf4_bank;
	int pf1_flip;
	int pf2_flip;
	int pf3_flip;
	int pf4_flip;
	int pf2_colourbank;
	int pf4_colourbank;
	int pri;
	bitmap_t *sprite0_mix_bitmap;
	bitmap_t *sprite1_mix_bitmap;
	bitmap_t *tilemap_alpha_bitmap;
	UINT32 dragngun_sprite_ctrl;
	int ace_ram_dirty;
	int has_ace_ram;
	bitmap_t *sprite_priority_bitmap;
	int last_pf3_bank;
};


/*----------- defined in video/deco32.c -----------*/

VIDEO_START( captaven );
VIDEO_START( fghthist );
VIDEO_START( dragngun );
VIDEO_START( lockload );
VIDEO_START( nslasher );

SCREEN_EOF( captaven );
SCREEN_EOF( dragngun );

SCREEN_UPDATE( captaven );
SCREEN_UPDATE( fghthist );
SCREEN_UPDATE( dragngun );
SCREEN_UPDATE( nslasher );


WRITE32_HANDLER( deco32_pf1_data_w );
WRITE32_HANDLER( deco32_pf2_data_w );
WRITE32_HANDLER( deco32_pf3_data_w );
WRITE32_HANDLER( deco32_pf4_data_w );

WRITE32_HANDLER( deco32_nonbuffered_palette_w );
WRITE32_HANDLER( deco32_buffered_palette_w );
WRITE32_HANDLER( deco32_palette_dma_w );

WRITE32_HANDLER( deco32_pri_w );
WRITE32_HANDLER( dragngun_sprite_control_w );
WRITE32_HANDLER( dragngun_spriteram_dma_w );
WRITE32_HANDLER( deco32_ace_ram_w );
