/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/


#define MW8080BW_MASTER_CLOCK				(19968000)
#define MW8080BW_CPU_CLOCK					(MW8080BW_MASTER_CLOCK / 10)
#define MW8080BW_PIXEL_CLOCK				(MW8080BW_MASTER_CLOCK / 4)
#define MW8080BW_HTOTAL						(0x140)
#define MW8080BW_HBEND						(0x000)
#define MW8080BW_HBSTART					(0x100)
#define MW8080BW_VTOTAL						(0x106)
#define MW8080BW_VBEND						(0x000)
#define MW8080BW_VBSTART					(0x0e0)
#define MW8080BW_VCOUNTER_START_NO_VBLANK	(0x020)
#define MW8080BW_VCOUNTER_START_VBLANK		(0x0da)
#define MW8080BW_INT_TRIGGER_COUNT_1		(0x080)
#define MW8080BW_INT_TRIGGER_VBLANK_1		(0)
#define MW8080BW_INT_TRIGGER_COUNT_2		MW8080BW_VCOUNTER_START_VBLANK
#define MW8080BW_INT_TRIGGER_VBLANK_2		(1)

/* +4 is added to HBSTART because the hardware displays that many pixels after
   setting HBLANK */
#define MW8080BW_HPIXCOUNT		(MW8080BW_HBSTART + 4)


/*----------- defined in drivers/mw8080bw.c -----------*/

extern UINT8 *mw8080bw_ram;
extern size_t mw8080bw_ram_size;

MACHINE_DRIVER_EXTERN( mw8080bw_root );
MACHINE_DRIVER_EXTERN( invaders );
extern const char layout_invaders[];

#define SEAWOLF_GUN_PORT_TAG			("GUN")

#define TORNBASE_CAB_TYPE_UPRIGHT_OLD	(0)
#define TORNBASE_CAB_TYPE_UPRIGHT_NEW	(1)
#define TORNBASE_CAB_TYPE_COCKTAIL		(2)
UINT8 tornbase_get_cabinet_type(void);

#define DESERTGU_GUN_X_PORT_TAG			("GUNX")
#define DESERTGU_GUN_Y_PORT_TAG			("GUNY")
void desertgun_set_controller_select(UINT8 data);

void clowns_set_controller_select(UINT8 data);

void spcenctr_set_strobe_state(UINT8 data);
UINT8 spcenctr_get_trench_width(void);
UINT8 spcenctr_get_trench_center(void);
UINT8 spcenctr_get_trench_slope(UINT8 addr);

UINT16 phantom2_get_cloud_counter(void);
void phantom2_set_cloud_counter(UINT16 data);

#define INVADERS_CAB_TYPE_PORT_TAG		("CAB")
#define INVADERS_P1_CONTROL_PORT_TAG	("CONTP1")
#define INVADERS_P2_CONTROL_PORT_TAG	("CONTP2")

UINT32 invaders_in2_control_r(void *param);
UINT8 invaders_is_flip_screen(void);
void invaders_set_flip_screen(UINT8 data);
int invaders_is_cabinet_cocktail(void);

#define BLUESHRK_SPEAR_PORT_TAG			("SPEAR")


/*----------- defined in machine/mw8080bw.c -----------*/

MACHINE_START( mw8080bw );
MACHINE_RESET( mw8080bw );


/*----------- defined in audio/mw8080bw.c -----------*/

WRITE8_HANDLER( midway_tone_generator_lo_w );
WRITE8_HANDLER( midway_tone_generator_hi_w );

MACHINE_DRIVER_EXTERN( seawolf_audio );
WRITE8_HANDLER( seawolf_audio_w );

MACHINE_DRIVER_EXTERN( gunfight_audio );
WRITE8_HANDLER( gunfight_audio_w );

MACHINE_DRIVER_EXTERN( tornbase_audio );
WRITE8_HANDLER( tornbase_audio_w );

MACHINE_DRIVER_EXTERN( zzzap_audio );
WRITE8_HANDLER( zzzap_audio_1_w );
WRITE8_HANDLER( zzzap_audio_2_w );

MACHINE_DRIVER_EXTERN( maze_audio );
void maze_write_discrete(UINT8 maze_tone_timing_state);

MACHINE_DRIVER_EXTERN( boothill_audio );
WRITE8_HANDLER( boothill_audio_w );

MACHINE_DRIVER_EXTERN( checkmat_audio );
WRITE8_HANDLER( checkmat_audio_w );

MACHINE_DRIVER_EXTERN( desertgu_audio );
WRITE8_HANDLER( desertgu_audio_1_w );
WRITE8_HANDLER( desertgu_audio_2_w );

MACHINE_DRIVER_EXTERN( dplay_audio );
WRITE8_HANDLER( dplay_audio_w );

MACHINE_DRIVER_EXTERN( gmissile_audio );
WRITE8_HANDLER( gmissile_audio_1_w );
WRITE8_HANDLER( gmissile_audio_2_w );
WRITE8_HANDLER( gmissile_audio_3_w );

MACHINE_DRIVER_EXTERN( m4_audio );
WRITE8_HANDLER( m4_audio_1_w );
WRITE8_HANDLER( m4_audio_2_w );

MACHINE_DRIVER_EXTERN( clowns_audio );
WRITE8_HANDLER( clowns_audio_1_w );
WRITE8_HANDLER( clowns_audio_2_w );

MACHINE_DRIVER_EXTERN( shuffle_audio );
WRITE8_HANDLER( shuffle_audio_1_w );
WRITE8_HANDLER( shuffle_audio_2_w );

MACHINE_DRIVER_EXTERN( dogpatch_audio );
WRITE8_HANDLER( dogpatch_audio_w );

MACHINE_DRIVER_EXTERN( spcenctr_audio );
WRITE8_HANDLER( spcenctr_audio_1_w );
WRITE8_HANDLER( spcenctr_audio_2_w );
WRITE8_HANDLER( spcenctr_audio_3_w );

MACHINE_DRIVER_EXTERN( phantom2_audio );
WRITE8_HANDLER( phantom2_audio_1_w );
WRITE8_HANDLER( phantom2_audio_2_w );

MACHINE_DRIVER_EXTERN( bowler_audio );
WRITE8_HANDLER( bowler_audio_1_w );
WRITE8_HANDLER( bowler_audio_2_w );
WRITE8_HANDLER( bowler_audio_3_w );
WRITE8_HANDLER( bowler_audio_4_w );
WRITE8_HANDLER( bowler_audio_5_w );
WRITE8_HANDLER( bowler_audio_6_w );

MACHINE_DRIVER_EXTERN( invaders_samples_audio );
MACHINE_DRIVER_EXTERN( invaders_audio );
WRITE8_HANDLER( invaders_audio_1_w );
WRITE8_HANDLER( invaders_audio_2_w );

MACHINE_DRIVER_EXTERN( blueshrk_audio );
WRITE8_HANDLER( blueshrk_audio_w );

MACHINE_DRIVER_EXTERN( invad2ct_audio );
WRITE8_HANDLER( invad2ct_audio_1_w );
WRITE8_HANDLER( invad2ct_audio_2_w );
WRITE8_HANDLER( invad2ct_audio_3_w );
WRITE8_HANDLER( invad2ct_audio_4_w );



/*----------- defined in video/mw8080bw.c -----------*/

VIDEO_UPDATE( mw8080bw );

VIDEO_UPDATE( spcenctr );

VIDEO_UPDATE( phantom2 );
VIDEO_EOF( phantom2 );

VIDEO_UPDATE( invaders );
