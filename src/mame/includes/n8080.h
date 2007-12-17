/*----------- defined in video/n8080.c -----------*/

extern int helifire_flash;

extern int spacefev_red_screen;
extern int spacefev_red_cannon;

WRITE8_HANDLER( n8080_video_control_w );

PALETTE_INIT( n8080 );
PALETTE_INIT( helifire );

VIDEO_START( spacefev );
VIDEO_START( sheriff );
VIDEO_START( helifire );
VIDEO_UPDATE( spacefev );
VIDEO_UPDATE( sheriff );
VIDEO_UPDATE( helifire );
VIDEO_EOF( helifire );

void spacefev_start_red_cannon(void);


/*----------- defined in audio/n8080.c -----------*/

MACHINE_DRIVER_EXTERN( spacefev_sound );
MACHINE_DRIVER_EXTERN( sheriff_sound );
MACHINE_DRIVER_EXTERN( helifire_sound );

WRITE8_HANDLER( n8080_sound_1_w );
WRITE8_HANDLER( n8080_sound_2_w );

