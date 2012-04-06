
class bublbobl_state : public driver_device
{
public:
	bublbobl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_mcu_sharedram;
	UINT8 *  m_videoram;
	UINT8 *  m_objectram;
//  UINT8 *  paletteram;    // currently this uses generic palette handling
	size_t   m_videoram_size;
	size_t   m_objectram_size;

	/* video-related */
	int      m_video_enable;

	/* sound-related */
	int      m_sound_nmi_enable;
	int		 m_pending_nmi;
	int		 m_sound_status;

	/* mcu-related */
	/* Tokio*/
	int      m_tokio_prot_count;
	/* Bubble Bobble MCU */
	UINT8    m_ddr1;
	UINT8	 m_ddr2;
	UINT8	 m_ddr3;
	UINT8	 m_ddr4;
	UINT8    m_port1_in;
	UINT8	 m_port2_in;
	UINT8	 m_port3_in;
	UINT8	 m_port4_in;
	UINT8    m_port1_out;
	UINT8	 m_port2_out;
	UINT8    m_port3_out;
	UINT8    m_port4_out;
	/* Bubble Bobble 68705 */
	UINT8    m_port_a_in;
	UINT8    m_port_a_out;
	UINT8    m_ddr_a;
	UINT8    m_port_b_in;
	UINT8    m_port_b_out;
	UINT8    m_ddr_b;
	int      m_address;
	int      m_latch;
	/* Bobble Bobble */
	int      m_ic43_a;
	int      m_ic43_b;

	/* devices */
	device_t *m_maincpu;
	device_t *m_mcu;
	device_t *m_audiocpu;
	device_t *m_slave;
	DECLARE_WRITE8_MEMBER(bublbobl_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tokio_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tokio_videoctrl_w);
	DECLARE_WRITE8_MEMBER(bublbobl_nmitrigger_w);
	DECLARE_READ8_MEMBER(tokio_mcu_r);
	DECLARE_READ8_MEMBER(tokiob_mcu_r);
	DECLARE_WRITE8_MEMBER(bublbobl_sound_command_w);
	DECLARE_WRITE8_MEMBER(bublbobl_sh_nmi_disable_w);
	DECLARE_WRITE8_MEMBER(bublbobl_sh_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(bublbobl_soundcpu_reset_w);
	DECLARE_READ8_MEMBER(bublbobl_sound_status_r);
	DECLARE_WRITE8_MEMBER(bublbobl_sound_status_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_ddr1_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_ddr1_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_ddr2_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_ddr2_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_ddr3_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_ddr3_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_ddr4_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_ddr4_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_port1_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_port1_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_port2_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_port2_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_port3_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_port3_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_port4_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_port4_w);
	DECLARE_READ8_MEMBER(boblbobl_ic43_a_r);
	DECLARE_WRITE8_MEMBER(boblbobl_ic43_a_w);
	DECLARE_WRITE8_MEMBER(boblbobl_ic43_b_w);
	DECLARE_READ8_MEMBER(boblbobl_ic43_b_r);
	DECLARE_READ8_MEMBER(bublbobl_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(bublbobl_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(bublbobl_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(bublbobl_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(bublbobl_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(bublbobl_68705_ddr_b_w);
};



/*----------- defined in machine/bublbobl.c -----------*/



// for 68705 bootleg
INTERRUPT_GEN( bublbobl_m68705_interrupt );


/*----------- defined in video/bublbobl.c -----------*/

SCREEN_UPDATE_IND16( bublbobl );
