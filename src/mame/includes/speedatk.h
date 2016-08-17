// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli
#include "video/mc6845.h"

class speedatk_state : public driver_device
{
public:
	speedatk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram") { }

	required_device<cpu_device> m_maincpu;
	required_device<h46505_device> m_crtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	UINT8 m_crtc_vreg[0x100];
	UINT8 m_crtc_index;
	UINT8 m_flip_scr;
	UINT8 m_mux_data;
	UINT8 m_km_status;
	UINT8 m_coin_settings;
	UINT8 m_coin_impulse;

	DECLARE_READ8_MEMBER(key_matrix_r);
	DECLARE_WRITE8_MEMBER(key_matrix_w);
	DECLARE_READ8_MEMBER(key_matrix_status_r);
	DECLARE_WRITE8_MEMBER(key_matrix_status_w);
	DECLARE_WRITE8_MEMBER(m6845_w);
	DECLARE_WRITE8_MEMBER(output_w);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(speedatk);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT8 iox_key_matrix_calc(UINT8 p_side);
};
