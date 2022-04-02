// license:BSD-3-Clause
// copyright-holders:Dan Boris
/***************************************************************************

    Sun Electronics Arabian hardware

    driver by Dan Boris

***************************************************************************/
#ifndef MAME_INCLUDES_ARABIAN_H
#define MAME_INCLUDES_ARABIAN_H

#pragma once

#include "cpu/mb88xx/mb88xx.h"
#include "emupal.h"

class arabian_state : public driver_device
{
public:
	arabian_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_custom_cpu_ram(*this, "custom_cpu_ram"),
		m_blitter(*this, "blitter"),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_palette(*this, "palette")
	{ }

	void arabian(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void main_io_map(address_map &map);
	void main_map(address_map &map);

private:
	uint8_t mcu_port_r0_r();
	uint8_t mcu_port_r1_r();
	uint8_t mcu_port_r2_r();
	uint8_t mcu_port_r3_r();
	void mcu_port_r0_w(uint8_t data);
	void mcu_port_r1_w(uint8_t data);
	void mcu_port_r2_w(uint8_t data);
	void mcu_port_r3_w(uint8_t data);
	uint8_t mcu_portk_r();
	void mcu_port_o_w(uint8_t data);
	void mcu_port_p_w(uint8_t data);
	void arabian_blitter_w(offs_t offset, uint8_t data);
	void arabian_videoram_w(offs_t offset, uint8_t data);
	void ay8910_porta_w(uint8_t data);
	void ay8910_portb_w(uint8_t data);
	void arabian_palette(palette_device &palette) const;
	uint32_t screen_update_arabian(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void blit_area(uint8_t plane, uint16_t src, uint8_t x, uint8_t y, uint8_t sx, uint8_t sy);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_custom_cpu_ram;
	required_shared_ptr<uint8_t> m_blitter;

	std::unique_ptr<uint8_t[]>  m_main_bitmap{};
	std::unique_ptr<uint8_t[]>  m_converted_gfx{};

	/* video-related */
	uint8_t    m_video_control = 0U;
	uint8_t    m_flip_screen = 0U;

	/* MCU */
	uint8_t    m_mcu_port_o = 0U;
	uint8_t    m_mcu_port_p = 0U;
	uint8_t    m_mcu_port_r[4]{};

	required_device<cpu_device> m_maincpu;
	required_device<mb8841_cpu_device> m_mcu;
	required_device<palette_device> m_palette;
};

#endif // MAME_INCLUDES_ARABIAN_H
