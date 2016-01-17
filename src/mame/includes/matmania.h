// license:BSD-3-Clause
// copyright-holders:Brad Oliver

class matmania_state : public driver_device
{
public:
	matmania_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_colorram3(*this, "colorram3"),
		m_scroll(*this, "scroll"),
		m_pageselect(*this, "pageselect"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_videoram3;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_colorram3;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_pageselect;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_tmpbitmap;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap2;

	/* maniach 68705 protection */
	UINT8 m_port_a_in;
	UINT8 m_port_a_out;
	UINT8 m_ddr_a;
	UINT8 m_port_b_in;
	UINT8 m_port_b_out;
	UINT8 m_ddr_b;
	UINT8 m_port_c_in;
	UINT8 m_port_c_out;
	UINT8 m_ddr_c;
	UINT8 m_from_main;
	UINT8 m_from_mcu;
	int m_mcu_sent;
	int m_main_sent;

	DECLARE_READ8_MEMBER(maniach_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(maniach_68705_port_a_w);
	DECLARE_READ8_MEMBER(maniach_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(maniach_68705_port_b_w);
	DECLARE_READ8_MEMBER(maniach_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(maniach_68705_port_c_w);
	DECLARE_WRITE8_MEMBER(maniach_68705_ddr_a_w);
	DECLARE_WRITE8_MEMBER(maniach_68705_ddr_b_w);
	DECLARE_WRITE8_MEMBER(maniach_68705_ddr_c_w);
	DECLARE_WRITE8_MEMBER(maniach_mcu_w);
	DECLARE_READ8_MEMBER(maniach_mcu_r);
	DECLARE_READ8_MEMBER(maniach_mcu_status_r);

	DECLARE_WRITE8_MEMBER(matmania_sh_command_w);
	DECLARE_WRITE8_MEMBER(maniach_sh_command_w);
	DECLARE_WRITE8_MEMBER(matmania_paletteram_w);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(matmania);
	DECLARE_MACHINE_START(maniach);
	DECLARE_MACHINE_RESET(maniach);
	UINT32 screen_update_matmania(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_maniach(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
