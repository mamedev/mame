// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/ondra.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_ONDRA_H
#define MAME_INCLUDES_ONDRA_H

#pragma once

#include "imagedev/cassette.h"
#include "machine/ram.h"

class ondra_state : public driver_device
{
public:
	ondra_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_video_enable(0),
		m_bank1_status(0),
		m_bank2_status(0),
		m_nmi_check_timer(nullptr),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_region_maincpu(*this, "maincpu"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_lines(*this, "LINE%u", 0),
		m_nmi(*this, "NMI")
	{
	}

	void ondra(machine_config &config);

private:
	DECLARE_READ8_MEMBER(ondra_keyboard_r);
	DECLARE_WRITE8_MEMBER(ondra_port_03_w);
	DECLARE_WRITE8_MEMBER(ondra_port_09_w);
	DECLARE_WRITE8_MEMBER(ondra_port_0a_w);
	uint32_t screen_update_ondra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	TIMER_CALLBACK_MEMBER(nmi_check_callback);

	void ondra_io(address_map &map);
	void ondra_mem(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void ondra_update_banks();

	uint8_t m_video_enable;
	uint8_t m_bank1_status;
	uint8_t m_bank2_status;
	emu_timer *m_nmi_check_timer;

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_memory_region m_region_maincpu;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_ioport_array<10> m_lines;
	required_ioport m_nmi;
};

#endif // MAME_INCLUDES_ONDRA_H
