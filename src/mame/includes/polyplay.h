#include "sound/samples.h"

#define SAMPLE_LENGTH 32

class polyplay_state : public driver_device
{
public:
	polyplay_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	int m_freq1;
	int m_freq2;
	int m_channel_playing1;
	int m_channel_playing2;
	INT16 m_backgroundwave[SAMPLE_LENGTH];
	int m_prescale1;
	int m_prescale2;
	int m_channel1_active;
	int m_channel1_const;
	int m_channel2_active;
	int m_channel2_const;
	timer_device* m_timer;
	int m_last;
	UINT8 *m_characterram;
	DECLARE_WRITE8_MEMBER(polyplay_sound_channel);
	DECLARE_WRITE8_MEMBER(polyplay_start_timer2);
	DECLARE_READ8_MEMBER(polyplay_random_read);
	DECLARE_WRITE8_MEMBER(polyplay_characterram_w);
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
SCREEN_UPDATE_IND16( polyplay );
