/* System E stuff */

enum sms_mapper
{
	MAPPER_STANDARD = 0,
	MAPPER_CODEMASTERS = 1
};

ADDRESS_MAP_EXTERN( sms_io_map, 8 );

extern SCREEN_UPDATE_RGB32(megatech_bios);
extern SCREEN_UPDATE_RGB32(megaplay_bios);
extern SCREEN_UPDATE_RGB32(megatech_md_sms);
extern void init_megatech_bios(running_machine &machine);
extern void init_hazemd_segasyse(running_machine &machine);
extern MACHINE_RESET(megatech_bios);
extern MACHINE_RESET(megatech_md_sms);
extern SCREEN_VBLANK(megatech_bios);
extern SCREEN_VBLANK(megatech_md_sms);

extern DECLARE_READ8_HANDLER( sms_vcounter_r );
extern DECLARE_READ8_HANDLER( sms_vdp_data_r );
extern DECLARE_WRITE8_HANDLER( sms_vdp_data_w );
extern DECLARE_READ8_HANDLER( sms_vdp_ctrl_r );
extern DECLARE_WRITE8_HANDLER( sms_vdp_ctrl_w );

extern void init_for_megadrive(running_machine &machine);
extern void segae_md_sms_stop_scanline_timer(void);
extern void megatech_set_genz80_as_sms_standard_map(running_machine &machine, const char* tag, int mapper);
