class bloodbro_state : public driver_device
{
public:
	bloodbro_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *bgvideoram;
	UINT16 *fgvideoram;
	UINT16 *txvideoram;
	UINT16 *scroll;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
	tilemap_t *tx_tilemap;
};


/*----------- defined in video/bloodbro.c -----------*/

WRITE16_HANDLER( bloodbro_bgvideoram_w );
WRITE16_HANDLER( bloodbro_fgvideoram_w );
WRITE16_HANDLER( bloodbro_txvideoram_w );

SCREEN_UPDATE( bloodbro );
SCREEN_UPDATE( weststry );
SCREEN_UPDATE( skysmash );
VIDEO_START( bloodbro );
