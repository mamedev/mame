// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari
// thanks-to:Michael Strutts, Marco Cassili
/***************************************************************************

    8080-based black and white hardware

****************************************************************************/

#include "includes/mw8080bw.h"
#include "sound/sn76477.h"
#include "sound/speaker.h"
#include "machine/eepromser.h"
/* for games in 8080bw.c */
#define CABINET_PORT_TAG                  "CAB"


class _8080bw_state : public mw8080bw_state
{
public:
	_8080bw_state(const machine_config &mconfig, device_type type, const char *tag)
		: mw8080bw_state(mconfig, type, tag),
		m_schaser_effect_555_timer(*this, "schaser_sh_555"),
		m_claybust_gun_on(*this, "claybust_gun"),
		m_discrete(*this, "discrete"),
		m_speaker(*this, "speaker"),
		m_eeprom(*this, "eeprom"),
		m_sn(*this, "snsnd"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gunx(*this, "GUNX"),
		m_guny(*this, "GUNY")
	{ }

	/* devices/memory pointers */
	optional_device<timer_device> m_schaser_effect_555_timer;
	optional_device<timer_device> m_claybust_gun_on;
	optional_device<discrete_device> m_discrete;
	optional_device<speaker_sound_device> m_speaker;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<sn76477_device> m_sn;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;

	/* misc game specific */
	optional_ioport m_gunx;
	optional_ioport m_guny;
	uint8_t m_color_map;
	uint8_t m_screen_red;
	uint8_t m_fleet_step;

	attotime m_schaser_effect_555_time_remain;
	int32_t m_schaser_effect_555_time_remain_savable;
	int m_schaser_effect_555_is_low;
	int m_schaser_explosion;
	int m_schaser_last_effect;
	uint8_t m_polaris_cloud_speed;
	uint8_t m_polaris_cloud_pos;
	uint8_t m_schaser_background_disable;
	uint8_t m_schaser_background_select;
	uint16_t m_claybust_gun_pos;

	ioport_value sflush_80_r(ioport_field &field, void *param);
	void claybust_gun_trigger(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	ioport_value claybust_gun_on_r(ioport_field &field, void *param);

	uint8_t indianbt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t polaris_port00_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void steelwkr_sh_port_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invadpt2_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invadpt2_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spacerng_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spcewars_sh_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lrescue_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lrescue_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cosmicmo_05_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cosmo_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t darthvdr_01_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void darthvdr_00_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void darthvdr_08_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ballbomb_01_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ballbomb_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ballbomb_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void indianbt_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void indianbt_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void indianbtbr_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void indianbtbr_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t indianbtbr_01_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void schaser_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void schaser_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rollingc_sh_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t invrvnge_02_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void invrvnge_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invrvnge_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lupin3_00_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lupin3_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lupin3_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t schasercv_02_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void schasercv_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void schasercv_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void yosakdon_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void yosakdon_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t shuttlei_ff_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void shuttlei_ff_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shuttlei_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shuttlei_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t claybust_gun_lo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t claybust_gun_hi_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t invmulti_eeprom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void invmulti_eeprom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invmulti_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t rollingc_scattered_colorram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rollingc_scattered_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rollingc_scattered_colorram2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rollingc_scattered_colorram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t schaser_scattered_colorram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void schaser_scattered_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_invmulti();
	void init_spacecom();
	void init_vortex();
	void init_attackfc();

	void machine_start_extra_8080bw();
	void machine_start_rollingc();
	void machine_start_sflush();
	void machine_start_schaser();
	void machine_start_schasercv();
	void machine_reset_schaser();
	void machine_start_polaris();
	void machine_start_darthvdr();
	void machine_reset_darthvdr();
	void machine_start_extra_8080bw_sh();
	void machine_start_extra_8080bw_vh();
	void machine_start_schaser_sh();
	void machine_reset_schaser_sh();
	void machine_start_claybust();

	void palette_init_rollingc(palette_device &palette);
	void palette_init_sflush(palette_device &palette);

	uint32_t screen_update_invadpt2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cosmo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_rollingc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_schaser(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_schasercv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sflush(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_indianbt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_lupin3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_polaris(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ballbomb(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_shuttlei(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spacecom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void polaris_interrupt(device_t &device);
	void claybust_gun_callback(timer_device &timer, void *ptr, int32_t param);
	void schaser_effect_555_cb(timer_device &timer, void *ptr, int32_t param);
	void indianbt_sh_port_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void polaris_sh_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void polaris_sh_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void polaris_sh_port_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void schaser_reinit_555_time_remain();
	inline void set_pixel( bitmap_rgb32 &bitmap, uint8_t y, uint8_t x, int color );
	inline void set_8_pixels( bitmap_rgb32 &bitmap, uint8_t y, uint8_t x, uint8_t data, int fore_color, int back_color );
	void clear_extra_columns( bitmap_rgb32 &bitmap, int color );
};


/*----------- defined in audio/8080bw.c -----------*/
extern const char *const lrescue_sample_names[];
extern const char *const lupin3_sample_names[];

DISCRETE_SOUND_EXTERN( ballbomb );
DISCRETE_SOUND_EXTERN( indianbt );
DISCRETE_SOUND_EXTERN( polaris );
DISCRETE_SOUND_EXTERN( schaser );
