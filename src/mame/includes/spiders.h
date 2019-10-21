// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Sigma Spiders hardware

***************************************************************************/
#ifndef MAME_INCLUDES_SPIDERS_H
#define MAME_INCLUDES_SPIDERS_H

#pragma once

#include "machine/6821pia.h"
#include "sound/discrete.h"
#include "video/mc6845.h"
#include "emupal.h"

class spiders_state : public driver_device
{
public:
	spiders_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ram(*this, "ram")
		, m_discrete(*this, "discrete")
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_audiocpu(*this, "audiocpu")
		, m_pia(*this, "pia%u", 1U)
	{ }

	void spiders(machine_config &config);

private:
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(gfx_rom_intf_w);
	DECLARE_READ8_MEMBER(gfx_rom_r);
	virtual void machine_start() override;
	INTERRUPT_GEN_MEMBER(update_pia_1);
	DECLARE_WRITE_LINE_MEMBER(ic60_74123_output_changed);
	DECLARE_WRITE8_MEMBER(spiders_audio_command_w);
	DECLARE_WRITE8_MEMBER(spiders_audio_a_w);
	DECLARE_WRITE8_MEMBER(spiders_audio_b_w);
	DECLARE_WRITE8_MEMBER(spiders_audio_ctrl_w);

	MC6845_UPDATE_ROW(crtc_update_row);

	void spiders_audio(machine_config &config);
	void spiders_audio_map(address_map &map);
	void spiders_main_map(address_map &map);

	required_shared_ptr<uint8_t> m_ram;
	required_device<discrete_device> m_discrete;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_audiocpu;
	required_device_array<pia6821_device, 4> m_pia;

	uint8_t m_flipscreen;
	uint16_t m_gfx_rom_address;
	uint8_t m_gfx_rom_ctrl_mode;
	uint8_t m_gfx_rom_ctrl_latch;
	uint8_t m_gfx_rom_ctrl_data;
};

#endif // MAME_INCLUDES_SPIDERS_H
