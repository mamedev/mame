#include "sound/discrete.h"
#include "sound/samples.h"

class blockade_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, blockade_state(machine)); }

	blockade_state(running_machine &machine)
		: driver_data_t(machine) { }

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
