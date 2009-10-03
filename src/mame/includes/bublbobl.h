/*----------- defined in machine/bublbobl.c -----------*/

extern UINT8 *bublbobl_mcu_sharedram;

WRITE8_HANDLER( bublbobl_bankswitch_w );
WRITE8_HANDLER( tokio_bankswitch_w );
WRITE8_HANDLER( tokio_videoctrl_w );
WRITE8_HANDLER( bublbobl_nmitrigger_w );
READ8_HANDLER( tokio_mcu_r );
READ8_HANDLER( tokiob_mcu_r );
WRITE8_HANDLER( bublbobl_sound_command_w );
WRITE8_HANDLER( bublbobl_sh_nmi_disable_w );
WRITE8_HANDLER( bublbobl_sh_nmi_enable_w );
WRITE8_HANDLER( bublbobl_soundcpu_reset_w );
WRITE8_HANDLER( bublbobl_sound_status_w );
READ8_HANDLER( bublbobl_sound_status_r );
READ8_HANDLER( boblbobl_ic43_a_r );
WRITE8_HANDLER( boblbobl_ic43_a_w );
READ8_HANDLER( boblbobl_ic43_b_r );
WRITE8_HANDLER( boblbobl_ic43_b_w );

READ8_HANDLER( bublbobl_mcu_ddr1_r );
WRITE8_HANDLER( bublbobl_mcu_ddr1_w );
READ8_HANDLER( bublbobl_mcu_ddr2_r );
WRITE8_HANDLER( bublbobl_mcu_ddr2_w );
READ8_HANDLER( bublbobl_mcu_ddr3_r );
WRITE8_HANDLER( bublbobl_mcu_ddr3_w );
READ8_HANDLER( bublbobl_mcu_ddr4_r );
WRITE8_HANDLER( bublbobl_mcu_ddr4_w );
READ8_HANDLER( bublbobl_mcu_port1_r );
WRITE8_HANDLER( bublbobl_mcu_port1_w );
READ8_HANDLER( bublbobl_mcu_port2_r );
WRITE8_HANDLER( bublbobl_mcu_port2_w );
READ8_HANDLER( bublbobl_mcu_port3_r );
WRITE8_HANDLER( bublbobl_mcu_port3_w );
READ8_HANDLER( bublbobl_mcu_port4_r );
WRITE8_HANDLER( bublbobl_mcu_port4_w );

// for 68705 bootleg
INTERRUPT_GEN( bublbobl_m68705_interrupt );
READ8_HANDLER( bublbobl_68705_portA_r );
WRITE8_HANDLER( bublbobl_68705_portA_w );
WRITE8_HANDLER( bublbobl_68705_ddrA_w );
READ8_HANDLER( bublbobl_68705_portB_r );
WRITE8_HANDLER( bublbobl_68705_portB_w );
WRITE8_HANDLER( bublbobl_68705_ddrB_w );


/*----------- defined in video/bublbobl.c -----------*/

extern int bublbobl_video_enable;
extern UINT8 *bublbobl_objectram;
extern size_t bublbobl_objectram_size;

VIDEO_UPDATE( bublbobl );
