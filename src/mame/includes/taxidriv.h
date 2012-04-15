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
};


/*----------- defined in video/taxidriv.c -----------*/

WRITE8_DEVICE_HANDLER( taxidriv_spritectrl_w );

SCREEN_UPDATE_IND16( taxidriv );
