class bking_state : public driver_device
{
public:
	bking_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *     m_playfield_ram;

	/* video-related */
	bitmap_t    *m_tmp_bitmap1;
	bitmap_t    *m_tmp_bitmap2;
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
};


/*----------- defined in video/bking.c -----------*/

WRITE8_HANDLER( bking_xld1_w );
WRITE8_HANDLER( bking_yld1_w );
WRITE8_HANDLER( bking_xld2_w );
WRITE8_HANDLER( bking_yld2_w );
WRITE8_HANDLER( bking_xld3_w );
WRITE8_HANDLER( bking_yld3_w );
WRITE8_HANDLER( bking_msk_w );
WRITE8_HANDLER( bking_cont1_w );
WRITE8_HANDLER( bking_cont2_w );
WRITE8_HANDLER( bking_cont3_w );
WRITE8_HANDLER( bking_hitclr_w );
WRITE8_HANDLER( bking_playfield_w );

READ8_HANDLER( bking_input_port_5_r );
READ8_HANDLER( bking_input_port_6_r );
READ8_HANDLER( bking_pos_r );

PALETTE_INIT( bking );
VIDEO_START( bking );
SCREEN_UPDATE( bking );
SCREEN_EOF( bking );
