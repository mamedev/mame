class bbusters_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, bbusters_state(machine)); }

	bbusters_state(running_machine &machine)
		: driver_data_t(machine) { }

	UINT16 *videoram;
	UINT16 *ram;
	UINT16 *eprom_data;
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
