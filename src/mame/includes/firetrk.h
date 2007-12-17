/*************************************************************************

Atari Fire Truck + Super Bug + Monte Carlo driver

*************************************************************************/

#include "sound/discrete.h"

#define FIRETRUCK_MOTOR_DATA	NODE_01
#define FIRETRUCK_HORN_EN		NODE_02
#define FIRETRUCK_SIREN_DATA	NODE_03
#define FIRETRUCK_CRASH_DATA	NODE_04
#define FIRETRUCK_SKID_EN		NODE_05
#define FIRETRUCK_BELL_EN		NODE_06
#define FIRETRUCK_ATTRACT_EN	NODE_07
#define FIRETRUCK_XTNDPLY_EN	NODE_08

#define SUPERBUG_SPEED_DATA		FIRETRUCK_MOTOR_DATA
#define SUPERBUG_CRASH_DATA		FIRETRUCK_CRASH_DATA
#define SUPERBUG_SKID_EN		FIRETRUCK_SKID_EN
#define SUPERBUG_ASR_EN			FIRETRUCK_XTNDPLY_EN
#define SUPERBUG_ATTRACT_EN		FIRETRUCK_ATTRACT_EN

#define MONTECAR_MOTOR_DATA			FIRETRUCK_MOTOR_DATA
#define MONTECAR_CRASH_DATA			FIRETRUCK_CRASH_DATA
#define MONTECAR_DRONE_MOTOR_DATA	FIRETRUCK_SIREN_DATA
#define MONTECAR_SKID_EN			FIRETRUCK_SKID_EN
#define MONTECAR_DRONE_LOUD_DATA	FIRETRUCK_BELL_EN
#define MONTECAR_BEEPER_EN			FIRETRUCK_XTNDPLY_EN
#define MONTECAR_ATTRACT_INV		FIRETRUCK_ATTRACT_EN

#define GAME_IS_FIRETRUCK   (firetrk_game == 1)
#define GAME_IS_SUPERBUG    (firetrk_game == 2)
#define GAME_IS_MONTECARLO  (firetrk_game == 3)


/*----------- defined in drivers/firetrk.c -----------*/

extern int firetrk_game;


/*----------- defined in audio/firetrk.c -----------*/

WRITE8_HANDLER( firetrk_skid_reset_w );
WRITE8_HANDLER( firetrk_crash_snd_w );
WRITE8_HANDLER( firetrk_skid_snd_w );
WRITE8_HANDLER( firetrk_motor_snd_w );
WRITE8_HANDLER( firetrk_xtndply_w );
WRITE8_HANDLER( firetrk_asr_w );
WRITE8_HANDLER( firetrk_out2_w );

DISCRETE_SOUND_EXTERN( firetrk );
DISCRETE_SOUND_EXTERN( superbug );
DISCRETE_SOUND_EXTERN( montecar );


/*----------- defined in video/firetrk.c -----------*/

extern VIDEO_UPDATE( firetrk );
extern VIDEO_START( firetrk );
extern VIDEO_EOF( firetrk );

extern WRITE8_HANDLER( firetrk_vert_w );
extern WRITE8_HANDLER( firetrk_horz_w );
extern WRITE8_HANDLER( firetrk_drone_hpos_w );
extern WRITE8_HANDLER( firetrk_drone_vpos_w );
extern WRITE8_HANDLER( firetrk_drone_rot_w );
extern WRITE8_HANDLER( firetrk_playfield_w );
extern WRITE8_HANDLER( firetrk_car_rot_w );

extern void firetrk_set_flash(int flag);
extern void firetrk_set_blink(int flag);

extern UINT8* firetrk_alpha_num_ram;
extern UINT8* firetrk_playfield_ram;

extern int firetrk_crash[2];
extern int firetrk_skid[2];
