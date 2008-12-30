#ifdef MAME_DEBUG
#define LOGLEVEL  5
#else
#define LOGLEVEL  0
#endif
#define LOG(n,x)  do { if (LOGLEVEL >= n) logerror x; } while (0)


#define DECOCASS_TAPE DEVICE_GET_INFO_NAME(decocass_tape)
DEVICE_GET_INFO( decocass_tape );

#define MDRV_DECOCASS_TAPE_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, DECOCASS_TAPE, 0)


extern WRITE8_HANDLER( decocass_coin_counter_w );
extern WRITE8_HANDLER( decocass_sound_command_w );
extern READ8_HANDLER( decocass_sound_data_r );
extern READ8_HANDLER( decocass_sound_ack_r );
extern WRITE8_HANDLER( decocass_sound_data_w );
extern READ8_HANDLER( decocass_sound_command_r );
extern TIMER_DEVICE_CALLBACK( decocass_audio_nmi_gen );
extern WRITE8_HANDLER( decocass_sound_nmi_enable_w );
extern READ8_HANDLER( decocass_sound_nmi_enable_r );
extern READ8_HANDLER( decocass_sound_data_ack_reset_r );
extern WRITE8_HANDLER( decocass_sound_data_ack_reset_w );
extern WRITE8_HANDLER( decocass_nmi_reset_w );
extern WRITE8_HANDLER( decocass_quadrature_decoder_reset_w );
extern WRITE8_HANDLER( decocass_adc_w );
extern READ8_HANDLER( decocass_input_r );

extern WRITE8_HANDLER( decocass_reset_w );

extern READ8_HANDLER( decocass_e5xx_r );
extern WRITE8_HANDLER( decocass_e5xx_w );
extern WRITE8_HANDLER( decocass_de0091_w );
extern WRITE8_HANDLER( decocass_e900_w );

extern MACHINE_START( decocass );
extern MACHINE_RESET( decocass );
extern MACHINE_RESET( ctsttape );
extern MACHINE_RESET( chwy );
extern MACHINE_RESET( clocknch );
extern MACHINE_RESET( ctisland );
extern MACHINE_RESET( csuperas );
extern MACHINE_RESET( castfant );
extern MACHINE_RESET( cluckypo );
extern MACHINE_RESET( cterrani );
extern MACHINE_RESET( cexplore );
extern MACHINE_RESET( cprogolf );
extern MACHINE_RESET( cmissnx );
extern MACHINE_RESET( cdiscon1 );
extern MACHINE_RESET( cptennis );
extern MACHINE_RESET( ctornado );
extern MACHINE_RESET( cbnj );
extern MACHINE_RESET( cburnrub );
extern MACHINE_RESET( cbtime );
extern MACHINE_RESET( cgraplop );
extern MACHINE_RESET( cgraplp2 );
extern MACHINE_RESET( clapapa );
extern MACHINE_RESET( cfghtice );
extern MACHINE_RESET( cprobowl );
extern MACHINE_RESET( cnightst );
extern MACHINE_RESET( cprosocc );
extern MACHINE_RESET( cppicf );
extern MACHINE_RESET( cscrtry );
extern MACHINE_RESET( cflyball );
extern MACHINE_RESET( cbdash );
extern MACHINE_RESET( czeroize );

extern WRITE8_HANDLER( i8041_p1_w );
extern READ8_HANDLER( i8041_p1_r );
extern WRITE8_HANDLER( i8041_p2_w );
extern READ8_HANDLER( i8041_p2_r );

void decocass_machine_state_save_init(running_machine *machine);

/*----------- defined in video/decocass.c -----------*/
extern WRITE8_HANDLER( decocass_paletteram_w );
extern WRITE8_HANDLER( decocass_charram_w );
extern WRITE8_HANDLER( decocass_fgvideoram_w );
extern WRITE8_HANDLER( decocass_colorram_w );
extern WRITE8_HANDLER( decocass_bgvideoram_w );
extern WRITE8_HANDLER( decocass_tileram_w );
extern WRITE8_HANDLER( decocass_objectram_w );

extern WRITE8_HANDLER( decocass_watchdog_count_w );
extern WRITE8_HANDLER( decocass_watchdog_flip_w );
extern WRITE8_HANDLER( decocass_color_missiles_w );
extern WRITE8_HANDLER( decocass_mode_set_w );
extern WRITE8_HANDLER( decocass_color_center_bot_w );
extern WRITE8_HANDLER( decocass_back_h_shift_w );
extern WRITE8_HANDLER( decocass_back_vl_shift_w );
extern WRITE8_HANDLER( decocass_back_vr_shift_w );
extern WRITE8_HANDLER( decocass_part_h_shift_w );
extern WRITE8_HANDLER( decocass_part_v_shift_w );
extern WRITE8_HANDLER( decocass_center_h_shift_space_w );
extern WRITE8_HANDLER( decocass_center_v_shift_w );

extern VIDEO_START( decocass );
extern VIDEO_UPDATE( decocass );

extern UINT8 *decocass_charram;
extern UINT8 *decocass_fgvideoram;
extern UINT8 *decocass_colorram;
extern UINT8 *decocass_bgvideoram;
extern UINT8 *decocass_tileram;
extern UINT8 *decocass_objectram;
extern size_t decocass_fgvideoram_size;
extern size_t decocass_colorram_size;
extern size_t decocass_bgvideoram_size;
extern size_t decocass_tileram_size;
extern size_t decocass_objectram_size;

void decocass_video_state_save_init(running_machine *machine);
