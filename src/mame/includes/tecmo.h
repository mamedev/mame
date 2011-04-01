class tecmo_state : public driver_device
{
public:
	tecmo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_adpcm_pos;
	int m_adpcm_end;
	int m_adpcm_data;
	int m_video_type;
	UINT8 *m_txvideoram;
	UINT8 *m_fgvideoram;
	UINT8 *m_bgvideoram;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 m_fgscroll[3];
	UINT8 m_bgscroll[3];
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
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
