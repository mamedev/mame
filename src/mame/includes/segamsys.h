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


extern DECLARE_READ8_HANDLER( md_sms_vdp_vcounter_r );
extern DECLARE_READ8_HANDLER( md_sms_vdp_data_r );
extern DECLARE_WRITE8_HANDLER( md_sms_vdp_data_w );
extern DECLARE_READ8_HANDLER( md_sms_vdp_ctrl_r );
extern DECLARE_WRITE8_HANDLER( md_sms_vdp_ctrl_w );

extern VIDEO_START(sms);
extern SCREEN_VBLANK(sms);
extern DECLARE_READ8_HANDLER( sms_vdp_2_data_r );
extern DECLARE_WRITE8_HANDLER( sms_vdp_2_data_w );
extern DECLARE_READ8_HANDLER( sms_vdp_2_ctrl_r );
extern DECLARE_WRITE8_HANDLER( sms_vdp_2_ctrl_w );
extern SCREEN_VBLANK(systeme);
extern SCREEN_UPDATE_RGB32(systeme);
extern MACHINE_RESET(systeme);
extern UINT8* sms_mainram;
extern UINT8* vdp2_vram_bank0;
extern UINT8* vdp2_vram_bank1;
extern UINT8* vdp1_vram_bank0;
extern UINT8* vdp1_vram_bank1;
extern void segae_set_vram_banks(UINT8 data);
DECLARE_READ8_HANDLER( sms_ioport_gg00_r );
void init_extra_gg_ports(running_machine* machine, const char* tag);
DECLARE_READ8_HANDLER (megatech_sms_ioport_dc_r);
DECLARE_READ8_HANDLER (megatech_sms_ioport_dd_r);
DECLARE_READ8_HANDLER( smsgg_backupram_r );
DECLARE_WRITE8_HANDLER( smsgg_backupram_w );
extern void megatech_set_genz80_as_sms_standard_map(running_machine &machine, const char* tag, int mapper);
MACHINE_CONFIG_EXTERN(sms);
extern void init_sms(running_machine &machine);
extern void init_smspal(running_machine &machine);
extern void init_smscm(running_machine &machine);
extern void init_smsgg(running_machine &machine);

INPUT_PORTS_EXTERN(sms);
INPUT_PORTS_EXTERN(gamegear);
extern UINT8* smsgg_backupram;
