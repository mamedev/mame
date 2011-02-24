#include "sound/discrete.h"

class circus_state : public driver_device
{
public:
	circus_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 * videoram;

	/* video-related */
	tilemap_t  *bg_tilemap;
	int clown_x, clown_y, clown_z;

	/* devices */
	device_t *maincpu;
	device_t *samples;
	device_t *discrete;

	/* game id */
	int game_id;
};



/*----------- defined in audio/circus.c -----------*/

extern WRITE8_HANDLER( circus_clown_z_w );

DISCRETE_SOUND_EXTERN( circus );
DISCRETE_SOUND_EXTERN( robotbwl );
DISCRETE_SOUND_EXTERN( crash );
extern const samples_interface circus_samples_interface;
extern const samples_interface crash_samples_interface;
extern const samples_interface ripcord_samples_interface;
extern const samples_interface robotbwl_samples_interface;

/*----------- defined in video/circus.c -----------*/

extern WRITE8_HANDLER( circus_clown_x_w );
extern WRITE8_HANDLER( circus_clown_y_w );

extern WRITE8_HANDLER( circus_videoram_w );

extern VIDEO_START( circus );
extern SCREEN_UPDATE( crash );
extern SCREEN_UPDATE( circus );
extern SCREEN_UPDATE( robotbwl );
extern SCREEN_UPDATE( ripcord );
