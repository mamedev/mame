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


class firetrk_state : public driver_device
{
public:
	firetrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_in_service_mode;
	UINT32 m_dial[2];
	UINT8 m_steer_dir[2];
	UINT8 m_steer_flag[2];
	UINT8 m_gear;
	UINT8 *m_alpha_num_ram;
	UINT8 *m_playfield_ram;
	UINT8 *m_scroll_x;
	UINT8 *m_scroll_y;
	UINT8 *m_car_rot;
	UINT8 *m_drone_rot;
	UINT8 *m_drone_x;
	UINT8 *m_drone_y;
	UINT8 *m_blink;
	UINT8 m_flash;
	UINT8 m_crash[2];
	UINT8 m_skid[2];
	bitmap_ind16 m_helper1;
	bitmap_ind16 m_helper2;
	UINT32 m_color1_mask;
	UINT32 m_color2_mask;
	tilemap_t *m_tilemap1;
	tilemap_t *m_tilemap2;
	DECLARE_WRITE8_MEMBER(firetrk_output_w);
	DECLARE_WRITE8_MEMBER(superbug_output_w);
	DECLARE_WRITE8_MEMBER(montecar_output_1_w);
	DECLARE_WRITE8_MEMBER(montecar_output_2_w);
	DECLARE_READ8_MEMBER(firetrk_dip_r);
	DECLARE_READ8_MEMBER(montecar_dip_r);
	DECLARE_READ8_MEMBER(firetrk_input_r);
	DECLARE_READ8_MEMBER(montecar_input_r);
	DECLARE_WRITE8_MEMBER(blink_on_w);
	DECLARE_WRITE8_MEMBER(montecar_car_reset_w);
	DECLARE_WRITE8_MEMBER(montecar_drone_reset_w);
	DECLARE_WRITE8_MEMBER(steer_reset_w);
	DECLARE_WRITE8_MEMBER(crash_reset_w);
	DECLARE_CUSTOM_INPUT_MEMBER(steer_dir_r);
	DECLARE_CUSTOM_INPUT_MEMBER(steer_flag_r);
	DECLARE_CUSTOM_INPUT_MEMBER(skid_r);
	DECLARE_CUSTOM_INPUT_MEMBER(crash_r);
	DECLARE_CUSTOM_INPUT_MEMBER(gear_r);
	DECLARE_INPUT_CHANGED_MEMBER(service_mode_switch_changed);
	DECLARE_INPUT_CHANGED_MEMBER(firetrk_horn_changed);
	DECLARE_INPUT_CHANGED_MEMBER(gear_changed);
};


/*----------- defined in audio/firetrk.c -----------*/

WRITE8_DEVICE_HANDLER( firetrk_skid_reset_w );
WRITE8_DEVICE_HANDLER( montecar_skid_reset_w );
WRITE8_DEVICE_HANDLER( firetrk_crash_snd_w );
WRITE8_DEVICE_HANDLER( firetrk_skid_snd_w );
WRITE8_DEVICE_HANDLER( firetrk_motor_snd_w );
WRITE8_DEVICE_HANDLER( superbug_motor_snd_w );
WRITE8_DEVICE_HANDLER( firetrk_xtndply_w );

DISCRETE_SOUND_EXTERN( firetrk );
DISCRETE_SOUND_EXTERN( superbug );
DISCRETE_SOUND_EXTERN( montecar );


/*----------- defined in video/firetrk.c -----------*/

PALETTE_INIT( firetrk );
PALETTE_INIT( montecar );
VIDEO_START( firetrk );
VIDEO_START( superbug );
VIDEO_START( montecar );
SCREEN_UPDATE_IND16( firetrk );
SCREEN_UPDATE_IND16( superbug );
SCREEN_UPDATE_IND16( montecar );


