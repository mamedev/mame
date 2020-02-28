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
		, m_region_gfx1(*this, "gfx1")
	{
	}

	void galaxy(machine_config &config);
	void galaxyp(machine_config &config);

	void init_galaxy();
	void init_galaxyp();

private:
	DECLARE_READ8_MEMBER(galaxy_keyboard_r);
	DECLARE_WRITE8_MEMBER(galaxy_latch_w);
	virtual void video_start() override;
	DECLARE_MACHINE_RESET(galaxy);
	DECLARE_MACHINE_RESET(galaxyp);
	uint32_t screen_update_galaxy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(galaxy_interrupt);
	TIMER_CALLBACK_MEMBER(gal_video);
	IRQ_CALLBACK_MEMBER(galaxy_irq_callback);
	void galaxy_set_timer();
	void galaxy_setup_snapshot (const uint8_t * data, uint32_t size);
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);
	void galaxy_mem(address_map &map);
	void galaxyp_io(address_map &map);
	void galaxyp_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_memory_region m_region_gfx1;
	ioport_port *m_io_ports[8];

	int m_interrupts_enabled;
	uint8_t m_latch_value;
	uint32_t m_gal_cnt;
	uint8_t m_code;
	uint8_t m_first;
	uint32_t m_start_addr;
	emu_timer *m_gal_video_timer;
	bitmap_ind16 m_bitmap;
};

#endif // MAME_INCLUDES_GALAXY_H
