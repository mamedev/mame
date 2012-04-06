#include "sound/discrete.h"

class circus_state : public driver_device
{
public:
	circus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 * m_videoram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int m_clown_x;
	int m_clown_y;
	int m_clown_z;

	/* devices */
	device_t *m_maincpu;
	samples_device *m_samples;
	device_t *m_discrete;

	/* game id */
	int m_game_id;
	DECLARE_READ8_MEMBER(circus_paddle_r);
	DECLARE_WRITE8_MEMBER(circus_videoram_w);
	DECLARE_WRITE8_MEMBER(circus_clown_x_w);
	DECLARE_WRITE8_MEMBER(circus_clown_y_w);
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



extern VIDEO_START( circus );
extern SCREEN_UPDATE_IND16( crash );
extern SCREEN_UPDATE_IND16( circus );
extern SCREEN_UPDATE_IND16( robotbwl );
extern SCREEN_UPDATE_IND16( ripcord );
