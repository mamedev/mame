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
	uint8_t m_latch_value;
	uint32_t m_gal_cnt;
	uint8_t m_code;
	uint8_t m_first;
	uint32_t m_start_addr;
	emu_timer *m_gal_video_timer;
	bitmap_ind16 m_bitmap;
	uint8_t galaxy_keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void galaxy_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_galaxy();
	void init_galaxyp();
	virtual void video_start() override;
	void machine_reset_galaxy();
	void machine_reset_galaxyp();
	uint32_t screen_update_galaxy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void galaxy_interrupt(device_t &device);
	void gal_video(void *ptr, int32_t param);
	int galaxy_irq_callback(device_t &device, int irqline);
	void galaxy_set_timer();
	void galaxy_setup_snapshot (const uint8_t * data, uint32_t size);
	required_device<cpu_device> m_maincpu;
	DECLARE_SNAPSHOT_LOAD_MEMBER( galaxy );
protected:
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_memory_region m_region_gfx1;
	ioport_port *m_io_ports[8];
};


#endif /* GALAXY_H_ */
