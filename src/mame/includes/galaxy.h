// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha, Miodrag Milanovic
/*****************************************************************************
 *
 * includes/galaxy.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_GALAXY_H
#define MAME_INCLUDES_GALAXY_H

#pragma once

#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "screen.h"

class galaxy_state : public driver_device
{
public:
	galaxy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cassette(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_p_chargen(*this, "chargen")
		, m_io_keyboard(*this, "LINE%u", 0U)
	{ }

	void galaxy(machine_config &config);
	void galaxyp(machine_config &config);

	void init_galaxy();
	void init_galaxyp();

private:
	uint8_t keyboard_r(offs_t offset);
	void latch_w(uint8_t data);
	void machine_start() override;
	void machine_reset() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(gal_video);
	IRQ_CALLBACK_MEMBER(irq_callback);
	void set_timer();
	void setup_snapshot (const uint8_t * data, uint32_t size);
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);
	void galaxy_mem(address_map &map);
	void galaxyp_io(address_map &map);
	void galaxyp_mem(address_map &map);

	int m_interrupts_enabled = 0;
	uint8_t m_latch_value = 0U;
	uint32_t m_gal_cnt = 0U;
	uint8_t m_code = 0U;
	uint8_t m_first = 0U;
	uint32_t m_start_addr = 0U;
	emu_timer *m_gal_video_timer = nullptr;
	bitmap_ind16 m_bitmap{};

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<cassette_image_device> m_cassette;
	optional_device<ram_device> m_ram;
	required_region_ptr<u8> m_p_chargen;
	required_ioport_array<8> m_io_keyboard;
};

#endif // MAME_INCLUDES_GALAXY_H
