// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari
// thanks-to:Michael Strutts, Marco Cassili
/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "machine/mb14241.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "sound/sn76477.h"
#include "sound/samples.h"


#define MW8080BW_MASTER_CLOCK             (19968000.0)
#define MW8080BW_CPU_CLOCK                (MW8080BW_MASTER_CLOCK / 10)
#define MW8080BW_PIXEL_CLOCK              (MW8080BW_MASTER_CLOCK / 4)
#define MW8080BW_HTOTAL                   (0x140)
#define MW8080BW_HBEND                    (0x000)
#define MW8080BW_HBSTART                  (0x100)
#define MW8080BW_VTOTAL                   (0x106)
#define MW8080BW_VBEND                    (0x000)
#define MW8080BW_VBSTART                  (0x0e0)
#define MW8080BW_VCOUNTER_START_NO_VBLANK (0x020)
#define MW8080BW_VCOUNTER_START_VBLANK    (0x0da)
#define MW8080BW_INT_TRIGGER_COUNT_1      (0x080)
#define MW8080BW_INT_TRIGGER_VBLANK_1     (0)
#define MW8080BW_INT_TRIGGER_COUNT_2      MW8080BW_VCOUNTER_START_VBLANK
#define MW8080BW_INT_TRIGGER_VBLANK_2     (1)
#define MW8080BW_60HZ                     (MW8080BW_PIXEL_CLOCK / MW8080BW_HTOTAL / MW8080BW_VTOTAL)

/* +4 is added to HBSTART because the hardware displays that many pixels after
   setting HBLANK */
#define MW8080BW_HPIXCOUNT                (MW8080BW_HBSTART + 4)


class mw8080bw_state : public driver_device
{
public:
	mw8080bw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_mb14241(*this,"mb14241"),
		m_watchdog(*this, "watchdog"),
		m_main_ram(*this, "main_ram"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_discrete(*this, "discrete"),
		m_samples(*this, "samples"),
		m_samples1(*this, "samples1"),
		m_samples2(*this, "samples2"),
		m_sn1(*this, "sn1"),
		m_sn2(*this, "sn2"),
		m_sn(*this, "snsnd"),
		m_screen(*this, "screen")
	{ }

	/* device/memory pointers */
	required_device<cpu_device> m_maincpu;
	optional_device<mb14241_device> m_mb14241;
	optional_device<watchdog_timer_device> m_watchdog;
	required_shared_ptr<uint8_t> m_main_ram;
	optional_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_colorram2;
	optional_device<discrete_device> m_discrete;

	/* sound-related */
	uint8_t       m_port_1_last;
	uint8_t       m_port_2_last;
	uint8_t       m_port_1_last_extra;
	uint8_t       m_port_2_last_extra;
	uint8_t       m_port_3_last_extra;

	/* misc game specific */
	uint16_t      m_phantom2_cloud_counter;
	uint8_t       m_flip_screen;
	uint8_t       m_rev_shift_res;
	uint8_t       m_maze_tone_timing_state;   /* output of IC C1, pin 5 */
	uint8_t       m_desertgun_controller_select;
	uint8_t       m_clowns_controller_select;

	uint8_t       m_spcenctr_strobe_state;
	uint8_t       m_spcenctr_trench_width;
	uint8_t       m_spcenctr_trench_center;
	uint8_t       m_spcenctr_trench_slope[16];  /* 16x4 bit RAM */
	uint8_t       m_spcenctr_bright_control;
	uint8_t       m_spcenctr_brightness;

	std::unique_ptr<uint8_t[]> m_scattered_colorram;
	std::unique_ptr<uint8_t[]> m_scattered_colorram2;

	/* timer */
	emu_timer   *m_interrupt_timer;

	/* other devices */
	optional_device<samples_device> m_samples;
	optional_device<samples_device> m_samples1;
	optional_device<samples_device> m_samples2;
	optional_device<sn76477_device> m_sn1;
	optional_device<sn76477_device> m_sn2;
	optional_device<sn76477_device> m_sn;
	required_device<screen_device> m_screen;

	uint8_t mw8080bw_shift_result_rev_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mw8080bw_reversable_shift_result_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mw8080bw_reversable_shift_count_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void seawolf_explosion_lamp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void seawolf_periscope_lamp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gunfight_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tornbase_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void maze_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void maze_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void checkmat_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spcenctr_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bowler_shift_result_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bowler_lights_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bowler_lights_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value seawolf_erase_input_r(ioport_field &field, void *param);
	ioport_value tornbase_hit_left_input_r(ioport_field &field, void *param);
	ioport_value tornbase_hit_right_input_r(ioport_field &field, void *param);
	ioport_value tornbase_pitch_left_input_r(ioport_field &field, void *param);
	ioport_value tornbase_pitch_right_input_r(ioport_field &field, void *param);
	ioport_value tornbase_score_input_r(ioport_field &field, void *param);
	ioport_value desertgu_gun_input_r(ioport_field &field, void *param);
	ioport_value desertgu_dip_sw_0_1_r(ioport_field &field, void *param);
	ioport_value dplay_pitch_left_input_r(ioport_field &field, void *param);
	ioport_value dplay_pitch_right_input_r(ioport_field &field, void *param);
	ioport_value clowns_controller_r(ioport_field &field, void *param);
	ioport_value invaders_coin_input_r(ioport_field &field, void *param);
	ioport_value invaders_sw6_sw7_r(ioport_field &field, void *param);
	ioport_value invaders_sw5_r(ioport_field &field, void *param);
	ioport_value blueshrk_coin_input_r(ioport_field &field, void *param);
	ioport_value invaders_in0_control_r(ioport_field &field, void *param);
	ioport_value invaders_in1_control_r(ioport_field &field, void *param);
	ioport_value invaders_in2_control_r(ioport_field &field, void *param);
	void seawolf_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gunfight_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zzzap_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zzzap_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gmissile_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gmissile_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gmissile_audio_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m4_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m4_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clowns_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void phantom2_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void phantom2_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bowler_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bowler_audio_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bowler_audio_4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bowler_audio_5_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bowler_audio_6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void machine_start_mw8080bw();
	void machine_reset_mw8080bw();
	void machine_start_maze();
	void machine_start_boothill();
	void machine_start_desertgu();
	void machine_start_gmissile();
	void machine_start_m4();
	void machine_start_clowns();
	void machine_start_spcenctr();
	void machine_start_phantom2();
	void machine_start_invaders();
	void sound_start_samples();
	uint32_t screen_update_mw8080bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spcenctr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_phantom2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_invaders(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_phantom2(screen_device &screen, bool state);
	void maze_tone_timing_timer_callback(void *ptr, int32_t param);
	void mw8080bw_interrupt_callback(void *ptr, int32_t param);
	void spcenctr_strobe_timer_callback(timer_device &timer, void *ptr, int32_t param);
	void midway_tone_generator_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void midway_tone_generator_hi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tornbase_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void boothill_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void checkmat_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void desertgu_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void desertgu_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dplay_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clowns_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spacwalk_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spacwalk_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shuffle_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shuffle_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dogpatch_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spcenctr_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spcenctr_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spcenctr_audio_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bowler_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invaders_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invaders_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blueshrk_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invad2ct_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invad2ct_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invad2ct_audio_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invad2ct_audio_4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void maze_update_discrete();
	void maze_write_discrete(uint8_t maze_tone_timing_state);
	uint8_t vpos_to_vysnc_chain_counter( int vpos );
	int vysnc_chain_counter_to_vpos( uint8_t counter, int vblank );
	void mw8080bw_create_interrupt_timer(  );
	void mw8080bw_start_interrupt_timer(  );
	uint8_t tornbase_get_cabinet_type();
	int invaders_is_cabinet_cocktail();
};


#define SEAWOLF_GUN_PORT_TAG            ("GUN")

#define TORNBASE_CAB_TYPE_UPRIGHT_OLD   (0)
#define TORNBASE_CAB_TYPE_UPRIGHT_NEW   (1)
#define TORNBASE_CAB_TYPE_COCKTAIL      (2)

#define DESERTGU_GUN_X_PORT_TAG         ("GUNX")
#define DESERTGU_GUN_Y_PORT_TAG         ("GUNY")

#define INVADERS_CAB_TYPE_PORT_TAG      ("CAB")
#define INVADERS_P1_CONTROL_PORT_TAG    ("CONTP1")
#define INVADERS_P2_CONTROL_PORT_TAG    ("CONTP2")
#define INVADERS_COIN_INPUT_PORT_TAG    ("COIN")
#define INVADERS_SW6_SW7_PORT_TAG       ("SW6SW7")
#define INVADERS_SW5_PORT_TAG           ("SW5")

#define BLUESHRK_SPEAR_PORT_TAG         ("IN0")

#define INVADERS_CONTROL_PORT_P1 \
	PORT_START(INVADERS_P1_CONTROL_PORT_TAG) \
	INVADERS_CONTROL_PORT_PLAYER(1)

#define INVADERS_CONTROL_PORT_P2 \
	PORT_START(INVADERS_P2_CONTROL_PORT_TAG) \
	INVADERS_CONTROL_PORT_PLAYER(2)

#define INVADERS_CONTROL_PORT_PLAYER(player) \
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(player) \
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(player) \
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(player) \
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

#define INVADERS_CAB_TYPE_PORT \
	PORT_START(INVADERS_CAB_TYPE_PORT_TAG) \
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) \
	PORT_CONFSETTING(    0x00, DEF_STR( Upright ) ) \
	PORT_CONFSETTING(    0x01, DEF_STR( Cocktail ) )

/*----------- defined in drivers/mw8080bw.c -----------*/

MACHINE_CONFIG_EXTERN( mw8080bw_root );
MACHINE_CONFIG_EXTERN( invaders );
extern const internal_layout layout_invaders;

/*----------- defined in audio/mw8080bw.c -----------*/


MACHINE_CONFIG_EXTERN( seawolf_audio );

MACHINE_CONFIG_EXTERN( gunfight_audio );

MACHINE_CONFIG_EXTERN( tornbase_audio );

MACHINE_CONFIG_EXTERN( zzzap_audio );

MACHINE_CONFIG_EXTERN( maze_audio );

MACHINE_CONFIG_EXTERN( boothill_audio );

MACHINE_CONFIG_EXTERN( checkmat_audio );

MACHINE_CONFIG_EXTERN( desertgu_audio );

MACHINE_CONFIG_EXTERN( dplay_audio );

MACHINE_CONFIG_EXTERN( gmissile_audio );

MACHINE_CONFIG_EXTERN( m4_audio );

MACHINE_CONFIG_EXTERN( clowns_audio );

MACHINE_CONFIG_EXTERN( spacwalk_audio );

MACHINE_CONFIG_EXTERN( shuffle_audio );

MACHINE_CONFIG_EXTERN( dogpatch_audio );

MACHINE_CONFIG_EXTERN( spcenctr_audio );

MACHINE_CONFIG_EXTERN( phantom2_audio );

MACHINE_CONFIG_EXTERN( bowler_audio );

MACHINE_CONFIG_EXTERN( invaders_samples_audio );
MACHINE_CONFIG_EXTERN( invaders_audio );

MACHINE_CONFIG_EXTERN( blueshrk_audio );

MACHINE_CONFIG_EXTERN( invad2ct_audio );
