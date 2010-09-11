class bbusters_state : public driver_device
{
public:
	bbusters_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  eprom_data(*this, "eeprom") { }

	UINT16 *videoram;
	UINT16 *ram;
	optional_shared_ptr<UINT16> eprom_data;
	int sound_status;
	int gun_select;

	tilemap_t *fix_tilemap;
	tilemap_t *pf1_tilemap;
	tilemap_t *pf2_tilemap;
	const UINT8 *scale_table_ptr;
	UINT8 scale_line_count;

	UINT16 *pf1_data;
	UINT16 *pf2_data;
	UINT16 *pf1_scroll_data;
	UINT16 *pf2_scroll_data;
};


/*----------- defined in video/bbusters.c -----------*/

VIDEO_START( bbuster );
VIDEO_START( mechatt );
VIDEO_UPDATE( bbuster );
VIDEO_UPDATE( mechatt );

WRITE16_HANDLER( bbusters_pf1_w );
WRITE16_HANDLER( bbusters_pf2_w );
WRITE16_HANDLER( bbusters_video_w );
