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
	int tattass_eprom_bit;
	int lastClock;
	char buffer[32];
	int bufPtr;
	int pendingCommand;
	int readBitCount;
	int byteAddr;
	UINT8 bsmt_latch;
	UINT8 bsmt_reset;

	int ace_ram_dirty;
	int has_ace_ram;
	UINT32 *ace_ram;
		
	UINT8 *dirty_palette;

	int pri;
	bitmap_t *tilemap_alpha_bitmap;


	UINT16 spriteram16[0x1000];
	UINT16 spriteram16_buffered[0x1000];
	UINT16 spriteram16_2[0x1000];
	UINT16 spriteram16_2_buffered[0x1000];
	UINT16    pf1_rowscroll[0x1000];
	UINT16    pf2_rowscroll[0x1000];
	UINT16    pf3_rowscroll[0x1000];
	UINT16    pf4_rowscroll[0x1000];
	// we use the pointers below to store a 32-bit copy..
	UINT32 *pf1_rowscroll32;
	UINT32 *pf2_rowscroll32;
	UINT32 *pf3_rowscroll32;
	UINT32 *pf4_rowscroll32;
	
	device_t *deco16ic;
};

class dragngun_state : public deco32_state
{
public:
	dragngun_state(running_machine &machine, const driver_device_config_base &config)
		: deco32_state(machine, config) { }

	UINT32 *dragngun_sprite_layout_0_ram;
	UINT32 *dragngun_sprite_layout_1_ram;
	UINT32 *dragngun_sprite_lookup_0_ram;
	UINT32 *dragngun_sprite_lookup_1_ram;
	UINT32 dragngun_sprite_ctrl;
	int dragngun_lightgun_port;
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
