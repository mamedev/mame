class tbowl_state : public driver_device
{
public:
	tbowl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int adpcm_pos[2];
	int adpcm_end[2];
	int adpcm_data[2];
	UINT8 *shared_ram;
	UINT8 *txvideoram;
	UINT8 *bgvideoram;
	UINT8 *bg2videoram;
	UINT8 *spriteram;
	tilemap_t *tx_tilemap;
	tilemap_t *bg_tilemap;
	tilemap_t *bg2_tilemap;
	UINT16 xscroll;
	UINT16 yscroll;
	UINT16 bg2xscroll;
	UINT16 bg2yscroll;
};


/*----------- defined in video/tbowl.c -----------*/

WRITE8_HANDLER( tbowl_bg2videoram_w );
WRITE8_HANDLER( tbowl_bgvideoram_w );
WRITE8_HANDLER( tbowl_txvideoram_w );

WRITE8_HANDLER( tbowl_bg2xscroll_lo );
WRITE8_HANDLER( tbowl_bg2xscroll_hi );
WRITE8_HANDLER( tbowl_bg2yscroll_lo );
WRITE8_HANDLER( tbowl_bg2yscroll_hi );
WRITE8_HANDLER( tbowl_bgxscroll_lo );
WRITE8_HANDLER( tbowl_bgxscroll_hi );
WRITE8_HANDLER( tbowl_bgyscroll_lo );
WRITE8_HANDLER( tbowl_bgyscroll_hi );

VIDEO_START( tbowl );
SCREEN_UPDATE( tbowl );
