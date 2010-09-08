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
};


/*----------- defined in audio/meadows.c -----------*/

SAMPLES_START( meadows_sh_start );
void meadows_sh_dac_w(running_machine *machine, int data);
void meadows_sh_update(running_machine *machine);
extern UINT8 meadows_0c00;
extern UINT8 meadows_0c01;
extern UINT8 meadows_0c02;
extern UINT8 meadows_0c03;


/*----------- defined in video/meadows.c -----------*/

VIDEO_START( meadows );
VIDEO_UPDATE( meadows );
WRITE8_HANDLER( meadows_videoram_w );
WRITE8_HANDLER( meadows_spriteram_w );

