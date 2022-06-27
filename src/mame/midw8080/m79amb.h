// license:BSD-3-Clause
// copyright-holders:Al Kossow
#ifndef MAME_INCLUDES_M79AMB_H
#define MAME_INCLUDES_M79AMB_H

#pragma once

#include "cpu/i8085/i8085.h"
#include "sound/discrete.h"

class m79amb_state : public driver_device
{
public:
	m79amb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_mask(*this, "mask"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_self_test(*this, "SELF_TEST")
	{ }

	void m79amb(machine_config &config);

	void init_m79amb();

private:
	void ramtek_videoram_w(offs_t offset, uint8_t data);
	uint8_t gray5bit_controller0_r();
	uint8_t gray5bit_controller1_r();
	void m79amb_8000_w(uint8_t data);
	void m79amb_8002_w(uint8_t data);
	void m79amb_8003_w(uint8_t data);
	uint8_t inta_r();

	void machine_start() override;

	uint32_t screen_update_ramtek(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_mask;
	required_device<discrete_device> m_discrete;
	required_device<i8080_cpu_device> m_maincpu;

	output_finder<> m_self_test;

	/* misc */
	uint8_t m_lut_gun1[0x100]{};
	uint8_t m_lut_gun2[0x100]{};
};

/*----------- defined in audio/m79amb.c -----------*/

DISCRETE_SOUND_EXTERN( m79amb_discrete );

#endif // MAME_INCLUDES_M79AMB_H
