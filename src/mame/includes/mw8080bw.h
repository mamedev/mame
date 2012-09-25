/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

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
		m_main_ram(*this, "main_ram"),
		m_colorram(*this, "colorram")
	{ }

	/* device/memory pointers */
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_main_ram;
	optional_shared_ptr<UINT8> m_colorram;

	/* sound-related */
	UINT8       m_port_1_last;
	UINT8       m_port_2_last;
	UINT8       m_port_1_last_extra;
	UINT8       m_port_2_last_extra;
	UINT8       m_port_3_last_extra;

	/* misc game specific */
	UINT16      m_phantom2_cloud_counter;
	UINT8       m_invaders_flip_screen;
	UINT8       m_rev_shift_res;
	UINT8       m_maze_tone_timing_state;	/* output of IC C1, pin 5 */
	UINT8       m_desertgun_controller_select;
	UINT8       m_clowns_controller_select;

	UINT8       m_spcenctr_strobe_state;
	UINT8       m_spcenctr_trench_width;
	UINT8       m_spcenctr_trench_center;
	UINT8       m_spcenctr_trench_slope[16];  /* 16x4 bit RAM */

	/* timer */
	emu_timer   *m_interrupt_timer;

	/* other devices */
	device_t *m_mb14241;
	samples_device *m_samples;
	samples_device *m_samples1;
	samples_device *m_samples2;
	device_t *m_sn1;
	device_t *m_sn2;
	device_t *m_sn;
	device_t *m_discrete;

	DECLARE_READ8_MEMBER(mw8080bw_shift_result_rev_r);
	DECLARE_READ8_MEMBER(mw8080bw_reversable_shift_result_r);
	DECLARE_WRITE8_MEMBER(mw8080bw_reversable_shift_count_w);
	DECLARE_WRITE8_MEMBER(seawolf_explosion_lamp_w);
	DECLARE_WRITE8_MEMBER(seawolf_periscope_lamp_w);
	DECLARE_WRITE8_MEMBER(gunfight_io_w);
	DECLARE_WRITE8_MEMBER(tornbase_io_w);
	DECLARE_WRITE8_MEMBER(maze_coin_counter_w);
	DECLARE_WRITE8_MEMBER(maze_io_w);
	DECLARE_WRITE8_MEMBER(checkmat_io_w);
	DECLARE_WRITE8_MEMBER(spcenctr_io_w);
	DECLARE_READ8_MEMBER(bowler_shift_result_r);
	DECLARE_WRITE8_MEMBER(bowler_lights_1_w);
	DECLARE_WRITE8_MEMBER(bowler_lights_2_w);
	DECLARE_CUSTOM_INPUT_MEMBER(seawolf_erase_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tornbase_hit_left_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tornbase_hit_right_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tornbase_pitch_left_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tornbase_pitch_right_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tornbase_score_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(desertgu_gun_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(desertgu_dip_sw_0_1_r);
	DECLARE_CUSTOM_INPUT_MEMBER(dplay_pitch_left_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(dplay_pitch_right_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(clowns_controller_r);
	DECLARE_CUSTOM_INPUT_MEMBER(invaders_coin_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(invaders_sw6_sw7_r);
	DECLARE_CUSTOM_INPUT_MEMBER(invaders_sw5_r);
	DECLARE_CUSTOM_INPUT_MEMBER(blueshrk_coin_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(invaders_in0_control_r);
	DECLARE_CUSTOM_INPUT_MEMBER(invaders_in1_control_r);
	DECLARE_CUSTOM_INPUT_MEMBER(invaders_in2_control_r);
	DECLARE_WRITE8_MEMBER(seawolf_audio_w);
	DECLARE_WRITE8_MEMBER(gunfight_audio_w);
	DECLARE_WRITE8_MEMBER(zzzap_audio_1_w);
	DECLARE_WRITE8_MEMBER(zzzap_audio_2_w);
	DECLARE_WRITE8_MEMBER(gmissile_audio_1_w);
	DECLARE_WRITE8_MEMBER(gmissile_audio_2_w);
	DECLARE_WRITE8_MEMBER(gmissile_audio_3_w);
	DECLARE_WRITE8_MEMBER(m4_audio_1_w);
	DECLARE_WRITE8_MEMBER(m4_audio_2_w);
	DECLARE_WRITE8_MEMBER(clowns_audio_1_w);
	DECLARE_WRITE8_MEMBER(phantom2_audio_1_w);
	DECLARE_WRITE8_MEMBER(phantom2_audio_2_w);
	DECLARE_WRITE8_MEMBER(bowler_audio_2_w);
	DECLARE_WRITE8_MEMBER(bowler_audio_3_w);
	DECLARE_WRITE8_MEMBER(bowler_audio_4_w);
	DECLARE_WRITE8_MEMBER(bowler_audio_5_w);
	DECLARE_WRITE8_MEMBER(bowler_audio_6_w);
	DECLARE_MACHINE_START(mw8080bw);
	DECLARE_MACHINE_RESET(mw8080bw);
	DECLARE_MACHINE_START(maze);
	DECLARE_MACHINE_START(boothill);
	DECLARE_MACHINE_START(desertgu);
	DECLARE_MACHINE_START(gmissile);
	DECLARE_MACHINE_START(m4);
	DECLARE_MACHINE_START(clowns);
	DECLARE_MACHINE_START(spcenctr);
	DECLARE_MACHINE_START(phantom2);
	DECLARE_MACHINE_START(invaders);
	UINT32 screen_update_mw8080bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spcenctr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_phantom2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_invaders(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_phantom2(screen_device &screen, bool state);
	TIMER_CALLBACK_MEMBER(maze_tone_timing_timer_callback);
	TIMER_CALLBACK_MEMBER(mw8080bw_interrupt_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(spcenctr_strobe_timer_callback);
};


#define SEAWOLF_GUN_PORT_TAG			("GUN")

#define TORNBASE_CAB_TYPE_UPRIGHT_OLD	(0)
#define TORNBASE_CAB_TYPE_UPRIGHT_NEW	(1)
#define TORNBASE_CAB_TYPE_COCKTAIL		(2)

#define DESERTGU_GUN_X_PORT_TAG			("GUNX")
#define DESERTGU_GUN_Y_PORT_TAG			("GUNY")

#define INVADERS_CAB_TYPE_PORT_TAG		("CAB")
#define INVADERS_P1_CONTROL_PORT_TAG	("CONTP1")
#define INVADERS_P2_CONTROL_PORT_TAG	("CONTP2")
#define INVADERS_COIN_INPUT_PORT_TAG	("COIN")
#define INVADERS_SW6_SW7_PORT_TAG		("SW6SW7")
#define INVADERS_SW5_PORT_TAG			("SW5")

#define BLUESHRK_SPEAR_PORT_TAG			("IN0")

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
extern const char layout_invaders[];

UINT8 tornbase_get_cabinet_type(running_machine &machine);

int invaders_is_cabinet_cocktail(running_machine &machine);

/*----------- defined in audio/mw8080bw.c -----------*/

DECLARE_WRITE8_DEVICE_HANDLER( midway_tone_generator_lo_w );
DECLARE_WRITE8_DEVICE_HANDLER( midway_tone_generator_hi_w );

MACHINE_CONFIG_EXTERN( seawolf_audio );

MACHINE_CONFIG_EXTERN( gunfight_audio );

MACHINE_CONFIG_EXTERN( tornbase_audio );
DECLARE_WRITE8_DEVICE_HANDLER( tornbase_audio_w );

MACHINE_CONFIG_EXTERN( zzzap_audio );

MACHINE_CONFIG_EXTERN( maze_audio );
void maze_write_discrete(device_t *device, UINT8 maze_tone_timing_state);

MACHINE_CONFIG_EXTERN( boothill_audio );
DECLARE_WRITE8_DEVICE_HANDLER( boothill_audio_w );

MACHINE_CONFIG_EXTERN( checkmat_audio );
DECLARE_WRITE8_DEVICE_HANDLER( checkmat_audio_w );

MACHINE_CONFIG_EXTERN( desertgu_audio );
DECLARE_WRITE8_DEVICE_HANDLER( desertgu_audio_1_w );
DECLARE_WRITE8_DEVICE_HANDLER( desertgu_audio_2_w );

MACHINE_CONFIG_EXTERN( dplay_audio );
DECLARE_WRITE8_DEVICE_HANDLER( dplay_audio_w );

MACHINE_CONFIG_EXTERN( gmissile_audio );

MACHINE_CONFIG_EXTERN( m4_audio );

MACHINE_CONFIG_EXTERN( clowns_audio );
DECLARE_WRITE8_DEVICE_HANDLER( clowns_audio_2_w );

MACHINE_CONFIG_EXTERN( spacwalk_audio );
DECLARE_WRITE8_DEVICE_HANDLER( spacwalk_audio_1_w );
DECLARE_WRITE8_DEVICE_HANDLER( spacwalk_audio_2_w );

MACHINE_CONFIG_EXTERN( shuffle_audio );
DECLARE_WRITE8_DEVICE_HANDLER( shuffle_audio_1_w );
DECLARE_WRITE8_DEVICE_HANDLER( shuffle_audio_2_w );

MACHINE_CONFIG_EXTERN( dogpatch_audio );
DECLARE_WRITE8_DEVICE_HANDLER( dogpatch_audio_w );

MACHINE_CONFIG_EXTERN( spcenctr_audio );
DECLARE_WRITE8_DEVICE_HANDLER( spcenctr_audio_1_w );
DECLARE_WRITE8_DEVICE_HANDLER( spcenctr_audio_2_w );
DECLARE_WRITE8_DEVICE_HANDLER( spcenctr_audio_3_w );

MACHINE_CONFIG_EXTERN( phantom2_audio );

MACHINE_CONFIG_EXTERN( bowler_audio );
DECLARE_WRITE8_DEVICE_HANDLER( bowler_audio_1_w );

MACHINE_CONFIG_EXTERN( invaders_samples_audio );
MACHINE_CONFIG_EXTERN( invaders_audio );
DECLARE_WRITE8_DEVICE_HANDLER( invaders_audio_1_w );
DECLARE_WRITE8_DEVICE_HANDLER( invaders_audio_2_w );

MACHINE_CONFIG_EXTERN( blueshrk_audio );
DECLARE_WRITE8_DEVICE_HANDLER( blueshrk_audio_w );

MACHINE_CONFIG_EXTERN( invad2ct_audio );
DECLARE_WRITE8_DEVICE_HANDLER( invad2ct_audio_1_w );
DECLARE_WRITE8_DEVICE_HANDLER( invad2ct_audio_2_w );
DECLARE_WRITE8_DEVICE_HANDLER( invad2ct_audio_3_w );
DECLARE_WRITE8_DEVICE_HANDLER( invad2ct_audio_4_w );
