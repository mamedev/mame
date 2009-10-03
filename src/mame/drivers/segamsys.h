/* System E stuff */

enum sms_mapper
{
	MAPPER_STANDARD = 0,
	MAPPER_CODEMASTERS = 1
};


extern VIDEO_UPDATE(megatech_bios);
extern VIDEO_UPDATE(megaplay_bios);
extern VIDEO_UPDATE(megatech_md_sms);
extern DRIVER_INIT( megatech_bios );
extern DRIVER_INIT( hazemd_segasyse );
extern MACHINE_RESET(megatech_bios);
extern MACHINE_RESET(megatech_md_sms);
extern VIDEO_EOF(megatech_bios);
extern VIDEO_EOF(megatech_md_sms);

extern READ8_HANDLER( sms_vcounter_r );
extern READ8_HANDLER( sms_vdp_data_r );
extern WRITE8_HANDLER( sms_vdp_data_w );
extern READ8_HANDLER( sms_vdp_ctrl_r );
extern WRITE8_HANDLER( sms_vdp_ctrl_w );

extern void init_for_megadrive(running_machine *machine);
extern void segae_md_sms_stop_scanline_timer(void);


extern READ8_HANDLER( md_sms_vdp_vcounter_r );
extern READ8_HANDLER( md_sms_vdp_data_r );
extern WRITE8_HANDLER( md_sms_vdp_data_w );
extern READ8_HANDLER( md_sms_vdp_ctrl_r );
extern WRITE8_HANDLER( md_sms_vdp_ctrl_w );

extern VIDEO_START(sms);
extern READ8_HANDLER( sms_vdp_2_data_r );
extern WRITE8_HANDLER( sms_vdp_2_data_w );
extern READ8_HANDLER( sms_vdp_2_ctrl_r );
extern WRITE8_HANDLER( sms_vdp_2_ctrl_w );
extern VIDEO_EOF(systeme);
extern VIDEO_UPDATE(systeme);
extern MACHINE_RESET(systeme);
extern UINT8* sms_mainram;
extern UINT8* vdp2_vram_bank0;
extern UINT8* vdp2_vram_bank1;
extern UINT8* vdp1_vram_bank0;
extern UINT8* vdp1_vram_bank1;
extern void segae_set_vram_banks(UINT8 data);
extern void megatech_set_genz80_as_sms_standard_map(running_machine *machine, const char* tag, int mapper);
MACHINE_DRIVER_EXTERN(sms);
extern DRIVER_INIT(sms);
extern DRIVER_INIT(smspal);
extern DRIVER_INIT(smscm);
extern DRIVER_INIT( smsgg );

INPUT_PORTS_EXTERN(sms);
INPUT_PORTS_EXTERN(gamegear);
extern UINT8* smsgg_backupram;





