class vulgus_state : public driver_device
{
public:
	vulgus_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *fgvideoram;
	UINT8 *bgvideoram;
	UINT8 *scroll_low;
	UINT8 *scroll_high;
	int palette_bank;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/vulgus.c -----------*/

WRITE8_HANDLER( vulgus_fgvideoram_w );
WRITE8_HANDLER( vulgus_bgvideoram_w );
WRITE8_HANDLER( vulgus_c804_w );
WRITE8_HANDLER( vulgus_palette_bank_w );

VIDEO_START( vulgus );
PALETTE_INIT( vulgus );
SCREEN_UPDATE( vulgus );
