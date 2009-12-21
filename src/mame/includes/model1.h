/*----------- defined in machine/model1.c -----------*/

extern const mb86233_cpu_core model1_vr_tgp_config;
extern int model1_dump;

READ16_HANDLER( model1_tgp_copro_r );
WRITE16_HANDLER( model1_tgp_copro_w );
READ16_HANDLER( model1_tgp_copro_adr_r );
WRITE16_HANDLER( model1_tgp_copro_adr_w );
READ16_HANDLER( model1_tgp_copro_ram_r );
WRITE16_HANDLER( model1_tgp_copro_ram_w );

READ16_HANDLER( model1_vr_tgp_r );
WRITE16_HANDLER( model1_vr_tgp_w );
READ16_HANDLER( model1_tgp_vr_adr_r );
WRITE16_HANDLER( model1_tgp_vr_adr_w );
READ16_HANDLER( model1_vr_tgp_ram_r );
WRITE16_HANDLER( model1_vr_tgp_ram_w );

ADDRESS_MAP_EXTERN( model1_vr_tgp_map, 32 );

MACHINE_START( model1 );

void model1_vr_tgp_reset( running_machine *machine );
void model1_tgp_reset(running_machine *machine, int swa);


/*----------- defined in video/model1.c -----------*/

extern UINT16 *model1_display_list0, *model1_display_list1;
extern UINT16 *model1_color_xlat;

VIDEO_START(model1);
VIDEO_UPDATE(model1);
VIDEO_EOF(model1);

READ16_HANDLER( model1_listctl_r );
WRITE16_HANDLER( model1_listctl_w );
