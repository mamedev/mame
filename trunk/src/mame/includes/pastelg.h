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
};


/*----------- defined in video/pastelg.c -----------*/

PALETTE_INIT( pastelg );
SCREEN_UPDATE( pastelg );
VIDEO_START( pastelg );

WRITE8_HANDLER( pastelg_clut_w );
WRITE8_HANDLER( pastelg_romsel_w );
WRITE8_HANDLER( threeds_romsel_w );
WRITE8_HANDLER( threeds_output_w );
WRITE8_HANDLER( pastelg_blitter_w );
READ8_HANDLER( threeds_rom_readback_r );

int pastelg_blitter_src_addr_r(address_space *space);
