class bking_state : public driver_device
{
public:
	bking_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *     m_playfield_ram;

	/* video-related */
	bitmap_ind16    m_tmp_bitmap1;
	bitmap_ind16    m_tmp_bitmap2;
	tilemap_t     *m_bg_tilemap;
	int         m_pc3259_output[4];
	int         m_pc3259_mask;
	UINT8       m_xld1;
	UINT8		m_xld2;
	UINT8		m_xld3;
	UINT8       m_yld1;
	UINT8		m_yld2;
	UINT8		m_yld3;
	int         m_ball1_pic;
	int			m_ball2_pic;
	int         m_crow_pic;
	int			m_crow_flip;
	int         m_palette_bank;
	int			m_controller;
	int			m_hit;

	/* sound-related */
	int         m_sound_nmi_enable;
	int			m_pending_nmi;

	/* misc */
	int         m_addr_h;
	int			m_addr_l;

	/* devices */
	device_t *m_audiocpu;

#if 0
	/* 68705 */
	UINT8 m_port_a_in;
	UINT8 m_port_a_out;
	UINT8 m_ddr_a;
	UINT8 m_port_b_in;
	UINT8 m_port_b_out;
	UINT8 m_ddr_b;
#endif
	DECLARE_READ8_MEMBER(bking_sndnmi_disable_r);
	DECLARE_WRITE8_MEMBER(bking_sndnmi_enable_w);
	DECLARE_WRITE8_MEMBER(bking_soundlatch_w);
	DECLARE_WRITE8_MEMBER(bking3_addr_l_w);
	DECLARE_WRITE8_MEMBER(bking3_addr_h_w);
	DECLARE_READ8_MEMBER(bking3_extrarom_r);
	DECLARE_READ8_MEMBER(bking3_ext_check_r);
	DECLARE_READ8_MEMBER(bking3_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(bking3_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(bking3_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(bking3_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(bking3_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(bking3_68705_ddr_b_w);
	DECLARE_READ8_MEMBER(bking3_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(bking_xld1_w);
	DECLARE_WRITE8_MEMBER(bking_yld1_w);
	DECLARE_WRITE8_MEMBER(bking_xld2_w);
	DECLARE_WRITE8_MEMBER(bking_yld2_w);
	DECLARE_WRITE8_MEMBER(bking_xld3_w);
	DECLARE_WRITE8_MEMBER(bking_yld3_w);
	DECLARE_WRITE8_MEMBER(bking_cont1_w);
	DECLARE_WRITE8_MEMBER(bking_cont2_w);
	DECLARE_WRITE8_MEMBER(bking_cont3_w);
	DECLARE_WRITE8_MEMBER(bking_msk_w);
	DECLARE_WRITE8_MEMBER(bking_hitclr_w);
	DECLARE_WRITE8_MEMBER(bking_playfield_w);
	DECLARE_READ8_MEMBER(bking_input_port_5_r);
	DECLARE_READ8_MEMBER(bking_input_port_6_r);
	DECLARE_READ8_MEMBER(bking_pos_r);
};


/*----------- defined in video/bking.c -----------*/



PALETTE_INIT( bking );
VIDEO_START( bking );
SCREEN_UPDATE_IND16( bking );
SCREEN_VBLANK( bking );
