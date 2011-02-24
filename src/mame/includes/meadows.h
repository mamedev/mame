/*************************************************************************

    Meadows S2650 hardware

*************************************************************************/

#include "sound/samples.h"

class meadows_state : public driver_device
{
public:
	meadows_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 dac;
	int dac_enable;
	int channel;
	int freq1;
	int freq2;
	UINT8 latched_0c01;
	UINT8 latched_0c02;
	UINT8 latched_0c03;
	UINT8 main_sense_state;
	UINT8 audio_sense_state;
	UINT8 _0c00;
	UINT8 _0c01;
	UINT8 _0c02;
	UINT8 _0c03;
	tilemap_t *bg_tilemap;
};


/*----------- defined in audio/meadows.c -----------*/

SAMPLES_START( meadows_sh_start );
void meadows_sh_dac_w(running_machine *machine, int data);
void meadows_sh_update(running_machine *machine);


/*----------- defined in video/meadows.c -----------*/

VIDEO_START( meadows );
SCREEN_UPDATE( meadows );
WRITE8_HANDLER( meadows_videoram_w );
WRITE8_HANDLER( meadows_spriteram_w );

