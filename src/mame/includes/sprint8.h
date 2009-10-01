#include "sound/discrete.h"

/*----------- defined in drivers/sprint8.c -----------*/

void sprint8_set_collision(running_machine *machine, int n);


/*----------- defined in video/sprint8.c -----------*/

extern UINT8* sprint8_video_ram;
extern UINT8* sprint8_pos_h_ram;
extern UINT8* sprint8_pos_v_ram;
extern UINT8* sprint8_pos_d_ram;
extern UINT8* sprint8_team;

PALETTE_INIT( sprint8 );
VIDEO_EOF( sprint8 );
VIDEO_START( sprint8 );
VIDEO_UPDATE( sprint8 );

WRITE8_HANDLER( sprint8_video_ram_w );


/*----------- defined in audio/sprint8.c -----------*/

DISCRETE_SOUND_EXTERN( sprint8 );

WRITE8_DEVICE_HANDLER( sprint8_crash_w );
WRITE8_DEVICE_HANDLER( sprint8_screech_w );
WRITE8_DEVICE_HANDLER( sprint8_attract_w );
WRITE8_DEVICE_HANDLER( sprint8_motor_w );
