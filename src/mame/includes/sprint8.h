/*----------- defined in drivers/sprint8.c -----------*/

void sprint8_set_collision(int n);


/*----------- defined in video/sprint8.c -----------*/

extern UINT8* sprint8_video_ram;
extern UINT8* sprint8_pos_h_ram;
extern UINT8* sprint8_pos_v_ram;
extern UINT8* sprint8_pos_d_ram;

VIDEO_EOF( sprint8 );
VIDEO_START( sprint8 );
VIDEO_UPDATE( sprint8 );

WRITE8_HANDLER( sprint8_video_ram_w );
