// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Ultraman

*************************************************************************/
#ifndef MAME_INCLUDES_ULTRAMAN_H
#define MAME_INCLUDES_ULTRAMAN_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "video/k051960.h"
#include "video/k051316.h"
#include "video/konami_helper.h"

class ultraman_state : public driver_device
{
public:
	ultraman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_k051316(*this, "k051316_%u", 1)
		, m_k051960(*this, "k051960")
		, m_soundlatch(*this, "soundlatch")
		, m_soundnmi(*this, "soundnmi")
	{
	}

	void ultraman(machine_config &config);

private:
	int        m_bank[3] = {};

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<k051316_device, 3> m_k051316;
	required_device<k051960_device> m_k051960;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<input_merger_device> m_soundnmi;

	void sound_nmi_enable_w(uint8_t data);
	void ultraman_gfxctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_ultraman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	K051316_CB_MEMBER(zoom_callback_1);
	K051316_CB_MEMBER(zoom_callback_2);
	K051316_CB_MEMBER(zoom_callback_3);
	K051960_CB_MEMBER(sprite_callback);
	void main_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_ULTRAMAN_H
