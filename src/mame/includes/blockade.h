#include "sound/discrete.h"
#include "sound/samples.h"

typedef struct _blockade_state blockade_state;
struct _blockade_state
{
	UINT8 *  videoram;

	/* video-related */
	tilemap  *bg_tilemap;

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
