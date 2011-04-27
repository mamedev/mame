class hyhoo_state : public driver_device
{
public:
	hyhoo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_clut;
	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_src_addr;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_gfxrom;
	int m_dispflag;
	int m_highcolorflag;
	int m_flipscreen;
	bitmap_t *m_tmpbitmap;
};


/*----------- defined in video/hyhoo.c -----------*/

SCREEN_UPDATE( hyhoo );
VIDEO_START( hyhoo );

WRITE8_HANDLER( hyhoo_blitter_w );
WRITE8_HANDLER( hyhoo_romsel_w );
