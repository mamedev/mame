
class bublbobl_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, bublbobl_state(machine)); }

	bublbobl_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  mcu_sharedram;
	UINT8 *  videoram;
	UINT8 *  objectram;
//  UINT8 *  paletteram;    // currently this uses generic palette handling
	size_t   videoram_size;
	size_t   objectram_size;

	/* missb2.c also needs the following */
	UINT8 *  bgvram;
	UINT8 *  bg_paletteram;

	/* video-related */
	int      video_enable;

	/* sound-related */
	int      sound_nmi_enable, pending_nmi, sound_status;

	/* mcu-related */
	/* Tokio*/
	int      tokio_prot_count;
	/* Bubble Bobble MCU */
	UINT8    ddr1, ddr2, ddr3, ddr4;
	UINT8    port1_in, port2_in, port3_in, port4_in;
	UINT8    port1_out, port2_out, port3_out, port4_out;
	/* Bubble Bobble 68705 */
	UINT8    port_a_in, port_a_out, ddr_a;
	UINT8    port_b_in, port_b_out, ddr_b;
	int      address, latch;
	/* Bobble Bobble */
	int      ic43_a, ic43_b;

	/* devices */
	running_device *maincpu;
	running_device *mcu;
	running_device *audiocpu;
	running_device *slave;
};



/*----------- defined in machine/bublbobl.c -----------*/

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
READ8_HANDLER( bublbobl_68705_port_a_r );
WRITE8_HANDLER( bublbobl_68705_port_a_w );
WRITE8_HANDLER( bublbobl_68705_ddr_a_w );
READ8_HANDLER( bublbobl_68705_port_b_r );
WRITE8_HANDLER( bublbobl_68705_port_b_w );
WRITE8_HANDLER( bublbobl_68705_ddr_b_w );


/*----------- defined in video/bublbobl.c -----------*/

VIDEO_UPDATE( bublbobl );
