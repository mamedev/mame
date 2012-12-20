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
		: mw8080bw_state(mconfig, type, tag),
		m_schaser_effect_555_timer(*this, "schaser_sh_555"),
		m_claybust_gun_on(*this, "claybust_gun"),
		m_discrete(*this, "discrete")
	{ }

	/* devices/memory pointers */
	optional_device<timer_device> m_schaser_effect_555_timer;
	optional_device<timer_device> m_claybust_gun_on;
	optional_device<discrete_device> m_discrete;
	device_t *m_speaker;


	/* misc game specific */
	UINT8 m_color_map;
	UINT8 m_screen_red;
	UINT8 m_fleet_step;

	attotime m_schaser_effect_555_time_remain;
	INT32 m_schaser_effect_555_time_remain_savable;
	int m_schaser_effect_555_is_low;
	int m_schaser_explosion;
	int m_schaser_last_effect;
	UINT8 m_polaris_cloud_speed;
	UINT8 m_polaris_cloud_pos;
	UINT8 m_schaser_background_disable;
	UINT8 m_schaser_background_select;
	UINT16 m_claybust_gun_pos;


	DECLARE_CUSTOM_INPUT_MEMBER(sflush_80_r);
	DECLARE_INPUT_CHANGED_MEMBER(claybust_gun_trigger);
	DECLARE_CUSTOM_INPUT_MEMBER(claybust_gun_on_r);

	DECLARE_READ8_MEMBER(indianbt_r);
	DECLARE_READ8_MEMBER(polaris_port00_r);
	DECLARE_WRITE8_MEMBER(steelwkr_sh_port_3_w);
	DECLARE_WRITE8_MEMBER(invadpt2_sh_port_1_w);
	DECLARE_WRITE8_MEMBER(invadpt2_sh_port_2_w);
	DECLARE_WRITE8_MEMBER(spacerng_sh_port_2_w);
	DECLARE_WRITE8_MEMBER(spcewars_sh_port_w);
	DECLARE_WRITE8_MEMBER(lrescue_sh_port_1_w);
	DECLARE_WRITE8_MEMBER(lrescue_sh_port_2_w);
	DECLARE_WRITE8_MEMBER(cosmo_sh_port_2_w);
	DECLARE_READ8_MEMBER(darthvdr_01_r);
	DECLARE_WRITE8_MEMBER(darthvdr_00_w);
	DECLARE_WRITE8_MEMBER(darthvdr_08_w);
	DECLARE_WRITE8_MEMBER(ballbomb_01_w);
	DECLARE_WRITE8_MEMBER(ballbomb_sh_port_1_w);
	DECLARE_WRITE8_MEMBER(ballbomb_sh_port_2_w);
	DECLARE_WRITE8_MEMBER(indianbt_sh_port_1_w);
	DECLARE_WRITE8_MEMBER(indianbt_sh_port_2_w);
	DECLARE_WRITE8_MEMBER(schaser_sh_port_1_w);
	DECLARE_WRITE8_MEMBER(schaser_sh_port_2_w);
	DECLARE_WRITE8_MEMBER(rollingc_sh_port_w);
	DECLARE_READ8_MEMBER(invrvnge_02_r);
	DECLARE_WRITE8_MEMBER(invrvnge_sh_port_1_w);
	DECLARE_WRITE8_MEMBER(invrvnge_sh_port_2_w);
	DECLARE_WRITE8_MEMBER(lupin3_00_w);
	DECLARE_WRITE8_MEMBER(lupin3_sh_port_1_w);
	DECLARE_WRITE8_MEMBER(lupin3_sh_port_2_w);
	DECLARE_READ8_MEMBER(schasercv_02_r);
	DECLARE_WRITE8_MEMBER(schasercv_sh_port_1_w);
	DECLARE_WRITE8_MEMBER(schasercv_sh_port_2_w);
	DECLARE_WRITE8_MEMBER(yosakdon_sh_port_1_w);
	DECLARE_WRITE8_MEMBER(yosakdon_sh_port_2_w);
	DECLARE_READ8_MEMBER(shuttlei_ff_r);
	DECLARE_WRITE8_MEMBER(shuttlei_ff_w);
	DECLARE_WRITE8_MEMBER(shuttlei_sh_port_1_w);
	DECLARE_WRITE8_MEMBER(shuttlei_sh_port_2_w);
	DECLARE_READ8_MEMBER(claybust_gun_lo_r);
	DECLARE_READ8_MEMBER(claybust_gun_hi_r);
	DECLARE_READ8_MEMBER(invmulti_eeprom_r);
	DECLARE_WRITE8_MEMBER(invmulti_eeprom_w);
	DECLARE_WRITE8_MEMBER(invmulti_bank_w);

	DECLARE_DRIVER_INIT(invmulti);
	DECLARE_DRIVER_INIT(spacecom);
	DECLARE_DRIVER_INIT(vortex);

	DECLARE_MACHINE_START(extra_8080bw);
	DECLARE_MACHINE_START(schaser);
	DECLARE_MACHINE_RESET(schaser);
	DECLARE_MACHINE_START(polaris);
	DECLARE_MACHINE_START(darthvdr);
	DECLARE_MACHINE_RESET(darthvdr);
	DECLARE_MACHINE_RESET(invmulti);
	DECLARE_MACHINE_START(extra_8080bw_sh);
	DECLARE_MACHINE_START(extra_8080bw_vh);
	DECLARE_MACHINE_START(schaser_sh);
	DECLARE_MACHINE_RESET(schaser_sh);
	DECLARE_MACHINE_START(claybust);

	UINT32 screen_update_invadpt2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cosmo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_rollingc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_schaser(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_schasercv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_sflush(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_indianbt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_lupin3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_polaris(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ballbomb(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_shuttlei(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spacecom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(polaris_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(claybust_gun_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(schaser_effect_555_cb);
	DECLARE_WRITE8_MEMBER(indianbt_sh_port_3_w);
	DECLARE_WRITE8_MEMBER(polaris_sh_port_1_w);
	DECLARE_WRITE8_MEMBER(polaris_sh_port_2_w);
	DECLARE_WRITE8_MEMBER(polaris_sh_port_3_w);
};


/*----------- defined in audio/8080bw.c -----------*/
extern const samples_interface lrescue_samples_interface;
extern const samples_interface lupin3_samples_interface;

DISCRETE_SOUND_EXTERN( ballbomb );
DISCRETE_SOUND_EXTERN( indianbt );
DISCRETE_SOUND_EXTERN( polaris );

extern const sn76477_interface lupin3_sn76477_interface;
extern const sn76477_interface schaser_sn76477_interface;
DISCRETE_SOUND_EXTERN( schaser );

