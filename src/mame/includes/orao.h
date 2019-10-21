// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/orao.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_ORAO_H
#define MAME_INCLUDES_ORAO_H

#pragma once

#include "sound/spkrdev.h"
#include "imagedev/cassette.h"

class orao_state : public driver_device
{
public:
	orao_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_memory(*this, "memory"),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_line(*this, "LINE.%u", 0),
		m_beep(0)
	{ }

	void orao(machine_config &config);

	void init_orao();
	void init_orao103();

private:
	DECLARE_READ8_MEMBER(orao_io_r);
	DECLARE_WRITE8_MEMBER(orao_io_w);
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_orao(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void orao_mem(address_map &map);

	required_shared_ptr<uint8_t> m_memory;
	required_shared_ptr<uint8_t> m_video_ram;
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<20> m_line;
	uint8_t m_beep;
};

#endif // MAME_INCLUDES_ORAO_H
