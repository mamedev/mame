class pastelg_state : public driver_device
{
public:
	pastelg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_mux_data;
	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_src_addr;
	int m_gfxrom;
	int m_dispflag;
	int m_flipscreen;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_palbank;
	UINT8 *m_videoram;
	UINT8 *m_clut;
	int m_flipscreen_old;
	DECLARE_READ8_MEMBER(pastelg_sndrom_r);
	DECLARE_READ8_MEMBER(pastelg_irq_ack_r);
	DECLARE_READ8_MEMBER(threeds_inputport1_r);
	DECLARE_READ8_MEMBER(threeds_inputport2_r);
	DECLARE_WRITE8_MEMBER(threeds_inputportsel_w);
	DECLARE_WRITE8_MEMBER(pastelg_clut_w);
	DECLARE_WRITE8_MEMBER(pastelg_blitter_w);
	DECLARE_WRITE8_MEMBER(threeds_romsel_w);
	DECLARE_WRITE8_MEMBER(threeds_output_w);
	DECLARE_READ8_MEMBER(threeds_rom_readback_r);
	DECLARE_WRITE8_MEMBER(pastelg_romsel_w);
};


/*----------- defined in video/pastelg.c -----------*/

PALETTE_INIT( pastelg );
SCREEN_UPDATE_IND16( pastelg );
VIDEO_START( pastelg );


int pastelg_blitter_src_addr_r(address_space *space);
