/*----------- defined in drivers/fgoal.c -----------*/

extern UINT8* fgoal_video_ram;
extern int fgoal_player;


/*----------- defined in video/fgoal.c -----------*/

VIDEO_START( fgoal );
VIDEO_UPDATE( fgoal );

WRITE8_HANDLER( fgoal_color_w );
WRITE8_HANDLER( fgoal_xpos_w );
WRITE8_HANDLER( fgoal_ypos_w );

