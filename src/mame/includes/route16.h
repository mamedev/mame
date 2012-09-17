class route16_state : public driver_device
{
public:
	route16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_sharedram(*this, "sharedram"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"){ }

	required_shared_ptr<UINT8> m_sharedram;
	UINT8 m_ttmahjng_port_select;
	int m_speakres_vrx;
	required_shared_ptr<UINT8> m_videoram1;
	required_shared_ptr<UINT8> m_videoram2;
	UINT8 m_flipscreen;
	UINT8 m_palette_1;
	UINT8 m_palette_2;
	DECLARE_READ8_MEMBER(sharedram_r);
	DECLARE_WRITE8_MEMBER(sharedram_w);
	DECLARE_WRITE8_MEMBER(route16_sharedram_w);
	DECLARE_WRITE8_MEMBER(ttmahjng_input_port_matrix_w);
	DECLARE_READ8_MEMBER(ttmahjng_input_port_matrix_r);
	DECLARE_READ8_MEMBER(speakres_in3_r);
	DECLARE_WRITE8_MEMBER(speakres_out2_w);
	DECLARE_READ8_MEMBER(routex_prot_read);
	DECLARE_WRITE8_MEMBER(route16_out0_w);
	DECLARE_WRITE8_MEMBER(route16_out1_w);
	DECLARE_WRITE8_MEMBER(stratvox_sn76477_w);
	DECLARE_DRIVER_INIT(route16);
	DECLARE_DRIVER_INIT(route16a);
	UINT32 screen_update_route16(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_stratvox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ttmahjng(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/route16.c -----------*/




