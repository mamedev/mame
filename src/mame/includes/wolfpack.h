/*----------- defined in video/wolfpack.c -----------*/

extern int wolfpack_collision;
extern UINT8* wolfpack_alpha_num_ram;

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
