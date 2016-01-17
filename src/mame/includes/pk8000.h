// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
class pk8000_base_state : public driver_device
{
public:
	pk8000_base_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	DECLARE_READ8_MEMBER(pk8000_video_color_r);
	DECLARE_WRITE8_MEMBER(pk8000_video_color_w);
	DECLARE_READ8_MEMBER(pk8000_text_start_r);
	DECLARE_WRITE8_MEMBER(pk8000_text_start_w);
	DECLARE_READ8_MEMBER(pk8000_chargen_start_r);
	DECLARE_WRITE8_MEMBER(pk8000_chargen_start_w);
	DECLARE_READ8_MEMBER(pk8000_video_start_r);
	DECLARE_WRITE8_MEMBER(pk8000_video_start_w);
	DECLARE_READ8_MEMBER(pk8000_color_start_r);
	DECLARE_WRITE8_MEMBER(pk8000_color_start_w);
	DECLARE_READ8_MEMBER(pk8000_color_r);
	DECLARE_WRITE8_MEMBER(pk8000_color_w);
	DECLARE_READ8_MEMBER(pk8000_84_porta_r);
	DECLARE_WRITE8_MEMBER(pk8000_84_porta_w);
	DECLARE_WRITE8_MEMBER(pk8000_84_portc_w);

	DECLARE_PALETTE_INIT(pk8000);

	UINT32 pk8000_video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *videomem);
protected:
	UINT8 m_pk8000_text_start;
	UINT8 m_pk8000_chargen_start;
	UINT8 m_pk8000_video_start;
	UINT8 m_pk8000_color_start;

	UINT8 m_pk8000_video_mode;
	UINT8 m_pk8000_video_color;
	UINT8 m_pk8000_color[32];
	UINT8 m_pk8000_video_enable;
	required_device<cpu_device> m_maincpu;
};
