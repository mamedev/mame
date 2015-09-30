// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha, Miodrag Milanovic
/*****************************************************************************
 *
 * includes/galaxy.h
 *
 ****************************************************************************/

#ifndef GALAXY_H_
#define GALAXY_H_

#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"


class galaxy_state : public driver_device
{
public:
	galaxy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_region_gfx1(*this, "gfx1") { }

	int m_interrupts_enabled;
	UINT8 m_latch_value;
	UINT32 m_gal_cnt;
	UINT8 m_code;
	UINT8 m_first;
	UINT32 m_start_addr;
	emu_timer *m_gal_video_timer;
	bitmap_ind16 m_bitmap;
	DECLARE_READ8_MEMBER(galaxy_keyboard_r);
	DECLARE_WRITE8_MEMBER(galaxy_latch_w);
	DECLARE_DRIVER_INIT(galaxy);
	DECLARE_DRIVER_INIT(galaxyp);
	virtual void video_start();
	DECLARE_MACHINE_RESET(galaxy);
	DECLARE_MACHINE_RESET(galaxyp);
	UINT32 screen_update_galaxy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(galaxy_interrupt);
	TIMER_CALLBACK_MEMBER(gal_video);
	IRQ_CALLBACK_MEMBER(galaxy_irq_callback);
	void galaxy_set_timer();
	void galaxy_setup_snapshot (const UINT8 * data, UINT32 size);
	required_device<cpu_device> m_maincpu;
	DECLARE_SNAPSHOT_LOAD_MEMBER( galaxy );
protected:
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_memory_region m_region_gfx1;
	ioport_port *m_io_ports[8];
};


#endif /* GALAXY_H_ */
