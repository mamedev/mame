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

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	uint8_t m_crtc_vreg[0x100];
	uint8_t m_crtc_index;
	uint8_t m_flip_scr;
	uint8_t m_mux_data;
	uint8_t m_km_status;
	uint8_t m_coin_settings;
	uint8_t m_coin_impulse;

	uint8_t key_matrix_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void key_matrix_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t key_matrix_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void key_matrix_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m6845_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_speedatk(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t iox_key_matrix_calc(uint8_t p_side);
};
