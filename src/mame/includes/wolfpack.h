class wolfpack_state : public driver_device
{
public:
	wolfpack_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int collision;
	UINT8* alpha_num_ram;
	unsigned current_index;
	UINT8 video_invert;
	UINT8 ship_reflect;
	UINT8 pt_pos_select;
	UINT8 pt_horz;
	UINT8 pt_pic;
	UINT8 ship_h;
	UINT8 torpedo_pic;
	UINT8 ship_size;
	UINT8 ship_h_precess;
	UINT8 ship_pic;
	UINT8 torpedo_h;
	UINT8 torpedo_v;
	UINT8* LFSR;
	bitmap_t* helper;
};


/*----------- defined in video/wolfpack.c -----------*/

PALETTE_INIT( wolfpack );
VIDEO_UPDATE( wolfpack );
VIDEO_START( wolfpack );
VIDEO_EOF( wolfpack );

WRITE8_HANDLER( wolfpack_video_invert_w );
WRITE8_HANDLER( wolfpack_ship_reflect_w );
WRITE8_HANDLER( wolfpack_pt_pos_select_w );
WRITE8_HANDLER( wolfpack_pt_horz_w );
WRITE8_HANDLER( wolfpack_pt_pic_w );
WRITE8_HANDLER( wolfpack_ship_h_w );
WRITE8_HANDLER( wolfpack_torpedo_pic_w );
WRITE8_HANDLER( wolfpack_ship_size_w );
WRITE8_HANDLER( wolfpack_ship_h_precess_w );
WRITE8_HANDLER( wolfpack_ship_pic_w );
WRITE8_HANDLER( wolfpack_torpedo_h_w );
WRITE8_HANDLER( wolfpack_torpedo_v_w );
