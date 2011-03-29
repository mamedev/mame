#include "sound/samples.h"

#define SAMPLE_LENGTH 32

class polyplay_state : public driver_device
{
public:
	polyplay_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	int freq1;
	int freq2;
	int channel_playing1;
	int channel_playing2;
	INT16 backgroundwave[SAMPLE_LENGTH];
	int prescale1;
	int prescale2;
	int channel1_active;
	int channel1_const;
	int channel2_active;
	int channel2_const;
	timer_device* timer;
	int last;
	UINT8 *characterram;
};


/*----------- defined in audio/polyplay.c -----------*/

void polyplay_set_channel1(running_machine &machine, int active);
void polyplay_set_channel2(running_machine &machine, int active);
void polyplay_play_channel1(running_machine &machine, int data);
void polyplay_play_channel2(running_machine &machine, int data);
SAMPLES_START( polyplay_sh_start );


/*----------- defined in video/polyplay.c -----------*/

PALETTE_INIT( polyplay );
VIDEO_START( polyplay );
SCREEN_UPDATE( polyplay );
WRITE8_HANDLER( polyplay_characterram_w );
