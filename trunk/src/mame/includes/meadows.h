/*************************************************************************

    Meadows S2650 hardware

*************************************************************************/

#include "sound/samples.h"

class meadows_state : public driver_device
{
public:
	meadows_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 m_dac;
	int m_dac_enable;
	int m_channel;
	int m_freq1;
	int m_freq2;
	UINT8 m_latched_0c01;
	UINT8 m_latched_0c02;
	UINT8 m_latched_0c03;
	UINT8 m_main_sense_state;
	UINT8 m_audio_sense_state;
	UINT8 m_0c00;
	UINT8 m_0c01;
	UINT8 m_0c02;
	UINT8 m_0c03;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
};


/*----------- defined in audio/meadows.c -----------*/

SAMPLES_START( meadows_sh_start );
void meadows_sh_dac_w(running_machine &machine, int data);
void meadows_sh_update(running_machine &machine);


/*----------- defined in video/meadows.c -----------*/

VIDEO_START( meadows );
SCREEN_UPDATE( meadows );
WRITE8_HANDLER( meadows_videoram_w );
WRITE8_HANDLER( meadows_spriteram_w );

