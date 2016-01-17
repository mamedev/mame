// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/galeb.h
 *
 ****************************************************************************/

#ifndef GALEB_H_
#define GALEB_H_


class galeb_state : public driver_device
{
public:
	galeb_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_video_ram;
	DECLARE_READ8_MEMBER(galeb_keyboard_r);
	virtual void video_start() override;
	UINT32 screen_update_galeb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

/*----------- defined in video/galeb.c -----------*/

extern const gfx_layout galeb_charlayout;

#endif /* GALEB_H_ */
