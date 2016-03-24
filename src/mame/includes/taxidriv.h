// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
class taxidriv_state : public driver_device
{
public:
	taxidriv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vram0(*this, "vram0"),
		m_vram1(*this, "vram1"),
		m_vram2(*this, "vram2"),
		m_vram3(*this, "vram3"),
		m_vram4(*this, "vram4"),
		m_vram5(*this, "vram5"),
		m_vram6(*this, "vram6"),
		m_vram7(*this, "vram7"),
		m_scroll(*this, "scroll")  { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_vram0;
	required_shared_ptr<UINT8> m_vram1;
	required_shared_ptr<UINT8> m_vram2;
	required_shared_ptr<UINT8> m_vram3;
	required_shared_ptr<UINT8> m_vram4;
	required_shared_ptr<UINT8> m_vram5;
	required_shared_ptr<UINT8> m_vram6;
	required_shared_ptr<UINT8> m_vram7;
	required_shared_ptr<UINT8> m_scroll;

	int m_s1;
	int m_s2;
	int m_s3;
	int m_s4;
	int m_latchA;
	int m_latchB;
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
	DECLARE_WRITE8_MEMBER(spritectrl_w);

	virtual void machine_start() override;
	DECLARE_PALETTE_INIT(taxidriv);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
