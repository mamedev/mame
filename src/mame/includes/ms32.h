class ms32_state : public driver_device
{
public:
	ms32_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *nvram_8;
	UINT32 *mahjong_input_select;
	UINT32 to_main;
	UINT16 irqreq;
	tilemap_t *tx_tilemap;
	tilemap_t *roz_tilemap;
	tilemap_t *bg_tilemap;
	tilemap_t *bg_tilemap_alt;
	UINT8* priram_8;
	UINT16* palram_16;
	UINT16* rozram_16;
	UINT16* lineram_16;
	UINT16* sprram_16;
	UINT16* txram_16;
	UINT16* bgram_16;
	UINT32 tilemaplayoutcontrol;
	UINT16* f1superb_extraram_16;
	tilemap_t* extra_tilemap;
	UINT32 *roz_ctrl;
	UINT32 *tx_scroll;
	UINT32 *bg_scroll;
	UINT32 *mainram;
	bitmap_t* temp_bitmap_tilemaps;
	bitmap_t* temp_bitmap_sprites;
	bitmap_t* temp_bitmap_sprites_pri;
	int reverse_sprite_order;
	int flipscreen;
	UINT32 brt[4];
	int brt_r;
	int brt_g;
	int brt_b;
};


/*----------- defined in video/ms32.c -----------*/

//extern UINT32 *ms32_fce00000;


WRITE32_HANDLER( ms32_brightness_w );

WRITE32_HANDLER( ms32_gfxctrl_w );
VIDEO_START( ms32 );
VIDEO_START( f1superb );
SCREEN_UPDATE( ms32 );
