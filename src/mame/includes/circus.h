#include "sound/discrete.h"

#define GAME_IS_CIRCUS		(circus_game == 1)
#define GAME_IS_ROBOTBWL	(circus_game == 2)
#define GAME_IS_CRASH		(circus_game == 3)
#define GAME_IS_RIPCORD		(circus_game == 4)

/*----------- defined in audio/circus.c -----------*/

extern int circus_game;
extern WRITE8_HANDLER( circus_clown_z_w );

DISCRETE_SOUND_EXTERN( circus );
DISCRETE_SOUND_EXTERN( robotbwl );
DISCRETE_SOUND_EXTERN( crash );
extern const samples_interface circus_samples_interface;
extern const samples_interface crash_samples_interface;
extern const samples_interface ripcord_samples_interface;
extern const samples_interface robotbwl_samples_interface;

/*----------- defined in video/circus.c -----------*/

extern int clown_z;

extern WRITE8_HANDLER( circus_clown_x_w );
extern WRITE8_HANDLER( circus_clown_y_w );

extern WRITE8_HANDLER( circus_videoram_w );

extern VIDEO_START( circus );
extern VIDEO_UPDATE( crash );
extern VIDEO_UPDATE( circus );
extern VIDEO_UPDATE( robotbwl );
extern VIDEO_UPDATE( ripcord ); //AT
