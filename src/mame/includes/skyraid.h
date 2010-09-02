#include "sound/discrete.h"

class skyraid_state : public driver_device
{
public:
	skyraid_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int analog_range;
	int analog_offset;

	int scroll;

	UINT8* alpha_num_ram;
	UINT8* pos_ram;
	UINT8* obj_ram;

	bitmap_t *helper;
};


/*----------- defined in audio/skyraid.c -----------*/

DISCRETE_SOUND_EXTERN( skyraid );

extern WRITE8_DEVICE_HANDLER( skyraid_sound_w );


/*----------- defined in video/skyraid.c -----------*/

VIDEO_START(skyraid);
VIDEO_UPDATE(skyraid);
