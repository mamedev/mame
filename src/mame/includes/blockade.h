#include "sound/discrete.h"
#include "sound/samples.h"

class blockade_state : public driver_device
{
public:
	blockade_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *  videoram;

	/* video-related */
	tilemap_t  *bg_tilemap;

	/* input-related */
	UINT8 coin_latch;  /* Active Low */
	UINT8 just_been_reset;
};


/*----------- defined in video/blockade.c -----------*/

WRITE8_HANDLER( blockade_videoram_w );

VIDEO_START( blockade );
VIDEO_UPDATE( blockade );

/*----------- defined in audio/blockade.c -----------*/

extern const samples_interface blockade_samples_interface;
DISCRETE_SOUND_EXTERN( blockade );

WRITE8_DEVICE_HANDLER( blockade_sound_freq_w );
WRITE8_HANDLER( blockade_env_on_w );
WRITE8_HANDLER( blockade_env_off_w );
