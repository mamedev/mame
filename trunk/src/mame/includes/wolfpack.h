class wolfpack_state : public driver_device
{
public:
	wolfpack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_collision;
	UINT8* m_alpha_num_ram;
	unsigned m_current_index;
	UINT8 m_video_invert;
	UINT8 m_ship_reflect;
	UINT8 m_pt_pos_select;
	UINT8 m_pt_horz;
	UINT8 m_pt_pic;
	UINT8 m_ship_h;
	UINT8 m_torpedo_pic;
	UINT8 m_ship_size;
	UINT8 m_ship_h_precess;
	UINT8 m_ship_pic;
	UINT8 m_torpedo_h;
	UINT8 m_torpedo_v;
	UINT8* m_LFSR;
	bitmap_t* m_helper;
};


/*----------- defined in video/wolfpack.c -----------*/

PALETTE_INIT( wolfpack );
SCREEN_UPDATE( wolfpack );
VIDEO_START( wolfpack );
SCREEN_EOF( wolfpack );

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
