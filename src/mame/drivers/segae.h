/* System E stuff */

extern VIDEO_UPDATE(megatech_bios);
extern VIDEO_UPDATE(megatech_md_sms);
extern DRIVER_INIT( megatech_bios );
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
extern WRITE8_HANDLER( sms_sn76496_w );

