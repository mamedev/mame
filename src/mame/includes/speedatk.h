// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli
#ifndef MAME_INCLUDES_SPEEDATK_H
#define MAME_INCLUDES_SPEEDATK_H

#pragma once

#include "video/mc6845.h"
#include "emupal.h"

class speedatk_state : public driver_device
{
public:
	speedatk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram")
	{ }

	void speedatk(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	uint8_t m_crtc_vreg[0x100]{};
	uint8_t m_crtc_index = 0;
	uint8_t m_flip_scr = 0;
	uint8_t m_mux_data = 0;
	uint8_t m_km_status = 0;
	uint8_t m_coin_settings = 0;
	uint8_t m_coin_impulse = 0;

	uint8_t key_matrix_r();
	void key_matrix_w(uint8_t data);
	uint8_t key_matrix_status_r();
	void key_matrix_status_w(uint8_t data);
	void m6845_w(offs_t offset, uint8_t data);
	void output_w(uint8_t data);

	void speedatk_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t iox_key_matrix_calc(uint8_t p_side);

	void speedatk_io(address_map &map);
	void speedatk_mem(address_map &map);
};

#endif // MAME_INCLUDES_SPEEDATK_H
