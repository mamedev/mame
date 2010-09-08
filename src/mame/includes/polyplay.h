#include "sound/samples.h"

class polyplay_state : public driver_device
{
public:
	polyplay_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in audio/polyplay.c -----------*/

void polyplay_set_channel1(int active);
void polyplay_set_channel2(int active);
void polyplay_play_channel1(running_machine *machine, int data);
void polyplay_play_channel2(running_machine *machine, int data);
SAMPLES_START( polyplay_sh_start );


/*----------- defined in video/polyplay.c -----------*/

extern UINT8 *polyplay_characterram;

PALETTE_INIT( polyplay );
VIDEO_START( polyplay );
VIDEO_UPDATE( polyplay );
WRITE8_HANDLER( polyplay_characterram_w );
