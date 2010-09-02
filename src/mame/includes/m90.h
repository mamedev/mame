/*----------- defined in video/m90.c -----------*/

extern UINT16 *m90_video_data;

VIDEO_START( m90 );
VIDEO_START( dynablsb );
VIDEO_START( bomblord );
VIDEO_UPDATE( m90 );
VIDEO_UPDATE( dynablsb );
VIDEO_UPDATE( bomblord );
WRITE16_HANDLER( m90_video_w );
WRITE16_HANDLER( m90_video_control_w );
