/***************************************************************************

    8080-based black and white hardware

****************************************************************************/

#include "includes/mw8080bw.h"

/* for games in 8080bw.c */
#define CABINET_PORT_TAG                  "CAB"


class _8080bw_state : public mw8080bw_state
{
public:
	_8080bw_state(const machine_config &mconfig, device_type type, const char *tag)
		: mw8080bw_state(mconfig, type, tag) { }

	/* misc game specific */
	emu_timer   *m_schaser_effect_555_timer;
	attotime    m_schaser_effect_555_time_remain;
	INT32       m_schaser_effect_555_time_remain_savable;
	int         m_schaser_effect_555_is_low;
	int         m_schaser_explosion;
	int         m_schaser_last_effect;
	UINT8       m_polaris_cloud_speed;
	UINT8       m_polaris_cloud_pos;
	UINT8       m_schaser_background_disable;
	UINT8       m_schaser_background_select;
	UINT8       m_c8080bw_flip_screen;
	UINT8       m_color_map;
	UINT8       m_screen_red;

	device_t *m_speaker;
	DECLARE_CUSTOM_INPUT_MEMBER(sflush_80_r);
	DECLARE_READ8_MEMBER(indianbt_r);
	DECLARE_WRITE8_MEMBER(steelwkr_sh_port_3_w);
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

SCREEN_UPDATE_RGB32( invadpt2 );
SCREEN_UPDATE_RGB32( ballbomb );
SCREEN_UPDATE_RGB32( schaser );
SCREEN_UPDATE_RGB32( schasercv );
SCREEN_UPDATE_RGB32( rollingc );
SCREEN_UPDATE_RGB32( polaris );
SCREEN_UPDATE_RGB32( lupin3 );
SCREEN_UPDATE_RGB32( cosmo );
SCREEN_UPDATE_RGB32( indianbt );
SCREEN_UPDATE_RGB32( shuttlei );
SCREEN_UPDATE_RGB32( sflush );

