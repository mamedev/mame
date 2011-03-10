/***************************************************************************

    8080-based black and white hardware

****************************************************************************/

#include "includes/mw8080bw.h"

/* for games in 8080bw.c */
#define CABINET_PORT_TAG                  "CAB"


class _8080bw_state : public mw8080bw_state
{
public:
	_8080bw_state(running_machine &machine, const driver_device_config_base &config)
		: mw8080bw_state(machine, config) { }

	/* misc game specific */
	emu_timer   *schaser_effect_555_timer;
	attotime    schaser_effect_555_time_remain;
	INT32       schaser_effect_555_time_remain_savable;
	int         schaser_effect_555_is_low;
	int         schaser_explosion;
	int         schaser_last_effect;
	UINT8       polaris_cloud_speed;
	UINT8       polaris_cloud_pos;
	UINT8       schaser_background_disable;
	UINT8       schaser_background_select;
	UINT8       c8080bw_flip_screen;
	UINT8       color_map;
	UINT8       screen_red;

	device_t *speaker;
};


/*----------- defined in audio/8080bw.c -----------*/

MACHINE_START( extra_8080bw_sh );

WRITE8_HANDLER( invadpt2_sh_port_1_w );
WRITE8_HANDLER( invadpt2_sh_port_2_w );

WRITE8_HANDLER( spcewars_sh_port_w );

WRITE8_HANDLER( lrescue_sh_port_1_w );
WRITE8_HANDLER( lrescue_sh_port_2_w );
extern const samples_interface lrescue_samples_interface;

WRITE8_HANDLER( cosmo_sh_port_2_w );

WRITE8_HANDLER( ballbomb_sh_port_1_w );
WRITE8_HANDLER( ballbomb_sh_port_2_w );

WRITE8_HANDLER( indianbt_sh_port_1_w );
WRITE8_HANDLER( indianbt_sh_port_2_w );
WRITE8_DEVICE_HANDLER( indianbt_sh_port_3_w );
DISCRETE_SOUND_EXTERN( indianbt );

WRITE8_DEVICE_HANDLER( polaris_sh_port_1_w );
WRITE8_DEVICE_HANDLER( polaris_sh_port_2_w );
WRITE8_DEVICE_HANDLER( polaris_sh_port_3_w );
DISCRETE_SOUND_EXTERN( polaris );

MACHINE_RESET( schaser_sh );
MACHINE_START( schaser_sh );
WRITE8_HANDLER( schaser_sh_port_1_w );
WRITE8_HANDLER( schaser_sh_port_2_w );
extern const sn76477_interface schaser_sn76477_interface;
DISCRETE_SOUND_EXTERN( schaser );

WRITE8_HANDLER( rollingc_sh_port_w );

WRITE8_HANDLER( invrvnge_sh_port_w );

WRITE8_HANDLER( lupin3_sh_port_1_w );
WRITE8_HANDLER( lupin3_sh_port_2_w );

WRITE8_HANDLER( schasercv_sh_port_1_w );
WRITE8_HANDLER( schasercv_sh_port_2_w );

WRITE8_HANDLER( yosakdon_sh_port_1_w );
WRITE8_HANDLER( yosakdon_sh_port_2_w );

WRITE8_HANDLER( shuttlei_sh_port_1_w );
WRITE8_HANDLER( shuttlei_sh_port_2_w );


/*----------- defined in video/8080bw.c -----------*/

MACHINE_START( extra_8080bw_vh );

SCREEN_UPDATE( invadpt2 );
SCREEN_UPDATE( ballbomb );
SCREEN_UPDATE( schaser );
SCREEN_UPDATE( schasercv );
SCREEN_UPDATE( rollingc );
SCREEN_UPDATE( polaris );
SCREEN_UPDATE( lupin3 );
SCREEN_UPDATE( cosmo );
SCREEN_UPDATE( indianbt );
SCREEN_UPDATE( shuttlei );
SCREEN_UPDATE( sflush );

