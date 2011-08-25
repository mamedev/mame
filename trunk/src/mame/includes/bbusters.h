class bbusters_state : public driver_device
{
public:
	bbusters_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_eprom_data(*this, "eeprom") { }

	UINT16 *m_videoram;
	UINT16 *m_ram;
	optional_shared_ptr<UINT16> m_eprom_data;
	int m_sound_status;
	int m_gun_select;

	tilemap_t *m_fix_tilemap;
	tilemap_t *m_pf1_tilemap;
	tilemap_t *m_pf2_tilemap;
	const UINT8 *m_scale_table_ptr;
	UINT8 m_scale_line_count;

	UINT16 *m_pf1_data;
	UINT16 *m_pf2_data;
	UINT16 *m_pf1_scroll_data;
	UINT16 *m_pf2_scroll_data;
};


/*----------- defined in video/bbusters.c -----------*/

VIDEO_START( bbuster );
VIDEO_START( mechatt );
SCREEN_UPDATE( bbuster );
SCREEN_UPDATE( mechatt );

WRITE16_HANDLER( bbusters_pf1_w );
WRITE16_HANDLER( bbusters_pf2_w );
WRITE16_HANDLER( bbusters_video_w );
