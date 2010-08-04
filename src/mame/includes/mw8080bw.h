/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "sound/discrete.h"
#include "sound/sn76477.h"
#include "sound/samples.h"


#define MW8080BW_MASTER_CLOCK             (19968000)
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

/* +4 is added to HBSTART because the hardware displays that many pixels after
   setting HBLANK */
#define MW8080BW_HPIXCOUNT                (MW8080BW_HBSTART + 4)


class mw8080bw_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mw8080bw_state(machine)); }

	mw8080bw_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *     main_ram;
	UINT8 *     colorram;
	size_t      main_ram_size;

	/* sound-related */
	UINT8       port_1_last;
	UINT8       port_2_last;
	UINT8       port_1_last_extra;
	UINT8       port_2_last_extra;
	UINT8       port_3_last_extra;

	/* misc game specific */
	emu_timer   *schaser_effect_555_timer;
	attotime    schaser_effect_555_time_remain;
	INT32       schaser_effect_555_time_remain_savable;
	int         schaser_effect_555_is_low;
	int         schaser_explosion;
	int         schaser_last_effect;
	UINT8       sfl_int;
	UINT8       polaris_cloud_speed;
	UINT8       polaris_cloud_pos;
	UINT8       schaser_background_disable;
	UINT8       schaser_background_select;
	UINT8       c8080bw_flip_screen;
	UINT8       color_map;
	UINT8       screen_red;


	UINT16      phantom2_cloud_counter;
	UINT8       invaders_flip_screen;
	UINT8       rev_shift_res;
	UINT8       maze_tone_timing_state;	/* output of IC C1, pin 5 */
	UINT8       desertgun_controller_select;
	UINT8       clowns_controller_select;

	UINT8       spcenctr_strobe_state;
	UINT8       spcenctr_trench_width;
	UINT8       spcenctr_trench_center;
	UINT8       spcenctr_trench_slope[16];  /* 16x4 bit RAM */

	/* timer */
	emu_timer   *interrupt_timer;

	/* devices */
	running_device *maincpu;
	running_device *mb14241;
	running_device *samples;
	running_device *samples1;
	running_device *samples2;
	running_device *speaker;
	running_device *sn1;
	running_device *sn2;
	running_device *sn;
	running_device *discrete;
};


/*----------- defined in drivers/mw8080bw.c -----------*/

MACHINE_DRIVER_EXTERN( mw8080bw_root );
MACHINE_DRIVER_EXTERN( invaders );
extern const char layout_invaders[];

#define SEAWOLF_GUN_PORT_TAG			("GUN")

#define TORNBASE_CAB_TYPE_UPRIGHT_OLD	(0)
#define TORNBASE_CAB_TYPE_UPRIGHT_NEW	(1)
#define TORNBASE_CAB_TYPE_COCKTAIL		(2)
UINT8 tornbase_get_cabinet_type(running_machine *machine);

#define DESERTGU_GUN_X_PORT_TAG			("GUNX")
#define DESERTGU_GUN_Y_PORT_TAG			("GUNY")

#define INVADERS_CAB_TYPE_PORT_TAG		("CAB")
#define INVADERS_P1_CONTROL_PORT_TAG	("CONTP1")
#define INVADERS_P2_CONTROL_PORT_TAG	("CONTP2")

/* for games in 8080bw.c */
#define CABINET_PORT_TAG                  "CAB"


CUSTOM_INPUT( invaders_in1_control_r );
CUSTOM_INPUT( invaders_in2_control_r );

int invaders_is_cabinet_cocktail(running_machine *machine);

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


/*----------- defined in machine/mw8080bw.c -----------*/

MACHINE_START( mw8080bw );
MACHINE_RESET( mw8080bw );


/*----------- defined in audio/mw8080bw.c -----------*/

WRITE8_DEVICE_HANDLER( midway_tone_generator_lo_w );
WRITE8_DEVICE_HANDLER( midway_tone_generator_hi_w );

MACHINE_DRIVER_EXTERN( seawolf_audio );
WRITE8_HANDLER( seawolf_audio_w );

MACHINE_DRIVER_EXTERN( gunfight_audio );
WRITE8_HANDLER( gunfight_audio_w );

MACHINE_DRIVER_EXTERN( tornbase_audio );
WRITE8_DEVICE_HANDLER( tornbase_audio_w );

MACHINE_DRIVER_EXTERN( zzzap_audio );
WRITE8_HANDLER( zzzap_audio_1_w );
WRITE8_HANDLER( zzzap_audio_2_w );

MACHINE_DRIVER_EXTERN( maze_audio );
void maze_write_discrete(running_device *device, UINT8 maze_tone_timing_state);

MACHINE_DRIVER_EXTERN( boothill_audio );
WRITE8_DEVICE_HANDLER( boothill_audio_w );

MACHINE_DRIVER_EXTERN( checkmat_audio );
WRITE8_DEVICE_HANDLER( checkmat_audio_w );

MACHINE_DRIVER_EXTERN( desertgu_audio );
WRITE8_DEVICE_HANDLER( desertgu_audio_1_w );
WRITE8_DEVICE_HANDLER( desertgu_audio_2_w );

MACHINE_DRIVER_EXTERN( dplay_audio );
WRITE8_DEVICE_HANDLER( dplay_audio_w );

MACHINE_DRIVER_EXTERN( gmissile_audio );
WRITE8_HANDLER( gmissile_audio_1_w );
WRITE8_HANDLER( gmissile_audio_2_w );
WRITE8_HANDLER( gmissile_audio_3_w );

MACHINE_DRIVER_EXTERN( m4_audio );
WRITE8_HANDLER( m4_audio_1_w );
WRITE8_HANDLER( m4_audio_2_w );

MACHINE_DRIVER_EXTERN( clowns_audio );
WRITE8_HANDLER( clowns_audio_1_w );
WRITE8_DEVICE_HANDLER( clowns_audio_2_w );

MACHINE_DRIVER_EXTERN( spacwalk_audio );
WRITE8_DEVICE_HANDLER( spacwalk_audio_1_w );
WRITE8_DEVICE_HANDLER( spacwalk_audio_2_w );

MACHINE_DRIVER_EXTERN( shuffle_audio );
WRITE8_DEVICE_HANDLER( shuffle_audio_1_w );
WRITE8_DEVICE_HANDLER( shuffle_audio_2_w );

MACHINE_DRIVER_EXTERN( dogpatch_audio );
WRITE8_HANDLER( dogpatch_audio_w );

MACHINE_DRIVER_EXTERN( spcenctr_audio );
WRITE8_DEVICE_HANDLER( spcenctr_audio_1_w );
WRITE8_DEVICE_HANDLER( spcenctr_audio_2_w );
WRITE8_DEVICE_HANDLER( spcenctr_audio_3_w );

MACHINE_DRIVER_EXTERN( phantom2_audio );
WRITE8_HANDLER( phantom2_audio_1_w );
WRITE8_HANDLER( phantom2_audio_2_w );

MACHINE_DRIVER_EXTERN( bowler_audio );
WRITE8_DEVICE_HANDLER( bowler_audio_1_w );
WRITE8_HANDLER( bowler_audio_2_w );
WRITE8_HANDLER( bowler_audio_3_w );
WRITE8_HANDLER( bowler_audio_4_w );
WRITE8_HANDLER( bowler_audio_5_w );
WRITE8_HANDLER( bowler_audio_6_w );

MACHINE_DRIVER_EXTERN( invaders_samples_audio );
MACHINE_DRIVER_EXTERN( invaders_audio );
WRITE8_DEVICE_HANDLER( invaders_audio_1_w );
WRITE8_DEVICE_HANDLER( invaders_audio_2_w );

MACHINE_DRIVER_EXTERN( blueshrk_audio );
WRITE8_DEVICE_HANDLER( blueshrk_audio_w );

MACHINE_DRIVER_EXTERN( invad2ct_audio );
WRITE8_DEVICE_HANDLER( invad2ct_audio_1_w );
WRITE8_DEVICE_HANDLER( invad2ct_audio_2_w );
WRITE8_DEVICE_HANDLER( invad2ct_audio_3_w );
WRITE8_DEVICE_HANDLER( invad2ct_audio_4_w );

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


/*----------- defined in video/mw8080bw.c -----------*/

VIDEO_UPDATE( mw8080bw );

VIDEO_UPDATE( spcenctr );

VIDEO_UPDATE( phantom2 );
VIDEO_EOF( phantom2 );

VIDEO_UPDATE( invaders );


/*----------- defined in video/8080bw.c -----------*/

MACHINE_START( extra_8080bw_vh );

VIDEO_UPDATE( invadpt2 );
VIDEO_UPDATE( ballbomb );
VIDEO_UPDATE( schaser );
VIDEO_UPDATE( schasercv );
VIDEO_UPDATE( rollingc );
VIDEO_UPDATE( polaris );
VIDEO_UPDATE( lupin3 );
VIDEO_UPDATE( cosmo );
VIDEO_UPDATE( indianbt );
VIDEO_UPDATE( shuttlei );
VIDEO_UPDATE( sflush );

