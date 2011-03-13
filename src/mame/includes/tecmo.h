class tecmo_state : public driver_device
{
public:
	tecmo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int adpcm_pos;
	int adpcm_end;
	int adpcm_data;
	int video_type;
	UINT8 *txvideoram;
	UINT8 *fgvideoram;
	UINT8 *bgvideoram;
	tilemap_t *tx_tilemap;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
	UINT8 fgscroll[3];
	UINT8 bgscroll[3];
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/tecmo.c -----------*/

WRITE8_HANDLER( tecmo_txvideoram_w );
WRITE8_HANDLER( tecmo_fgvideoram_w );
WRITE8_HANDLER( tecmo_bgvideoram_w );
WRITE8_HANDLER( tecmo_fgscroll_w );
WRITE8_HANDLER( tecmo_bgscroll_w );
WRITE8_HANDLER( tecmo_flipscreen_w );

VIDEO_START( tecmo );
SCREEN_UPDATE( tecmo );
