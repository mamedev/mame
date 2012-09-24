class hyhoo_state : public driver_device
{
public:
	hyhoo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_clut(*this, "clut"){ }

	required_shared_ptr<UINT8> m_clut;
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
	bitmap_rgb32 m_tmpbitmap;
	DECLARE_WRITE8_MEMBER(hyhoo_blitter_w);
	DECLARE_WRITE8_MEMBER(hyhoo_romsel_w);
	DECLARE_DRIVER_INIT(hyhoo2);
	DECLARE_DRIVER_INIT(hyhoo);
	virtual void video_start();
	UINT32 screen_update_hyhoo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(blitter_timer_callback);
};
