class taxidriv_state : public driver_device
{
public:
	taxidriv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vram0(*this, "vram0"),
		m_vram1(*this, "vram1"),
		m_vram2(*this, "vram2"),
		m_vram3(*this, "vram3"),
		m_vram4(*this, "vram4"),
		m_vram5(*this, "vram5"),
		m_vram6(*this, "vram6"),
		m_vram7(*this, "vram7"),
		m_scroll(*this, "scroll"){ }

	int m_s1;
	int m_s2;
	int m_s3;
	int m_s4;
	int m_latchA;
	int m_latchB;
	required_shared_ptr<UINT8> m_vram0;
	required_shared_ptr<UINT8> m_vram1;
	required_shared_ptr<UINT8> m_vram2;
	required_shared_ptr<UINT8> m_vram3;
	required_shared_ptr<UINT8> m_vram4;
	required_shared_ptr<UINT8> m_vram5;
	required_shared_ptr<UINT8> m_vram6;
	required_shared_ptr<UINT8> m_vram7;
	required_shared_ptr<UINT8> m_scroll;
	int m_bghide;
	int m_spritectrl[9];
	DECLARE_WRITE8_MEMBER(p2a_w);
	DECLARE_WRITE8_MEMBER(p2b_w);
	DECLARE_WRITE8_MEMBER(p2c_w);
	DECLARE_WRITE8_MEMBER(p3a_w);
	DECLARE_WRITE8_MEMBER(p3b_w);
	DECLARE_WRITE8_MEMBER(p3c_w);
	DECLARE_WRITE8_MEMBER(p4a_w);
	DECLARE_WRITE8_MEMBER(p4b_w);
	DECLARE_WRITE8_MEMBER(p4c_w);
	DECLARE_READ8_MEMBER(p0a_r);
	DECLARE_READ8_MEMBER(p0c_r);
	DECLARE_WRITE8_MEMBER(p0b_w);
	DECLARE_WRITE8_MEMBER(p0c_w);
	DECLARE_READ8_MEMBER(p1b_r);
	DECLARE_READ8_MEMBER(p1c_r);
	DECLARE_WRITE8_MEMBER(p1a_w);
	DECLARE_WRITE8_MEMBER(p1c_w);
	DECLARE_READ8_MEMBER(p8910_0a_r);
	DECLARE_READ8_MEMBER(p8910_1a_r);
	DECLARE_WRITE8_MEMBER(p8910_0b_w);
	DECLARE_WRITE8_MEMBER(taxidriv_spritectrl_w);
	virtual void palette_init();
	UINT32 screen_update_taxidriv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
