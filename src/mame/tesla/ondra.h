// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/ondra.h
 *
 ****************************************************************************/
#ifndef MAME_TESLA_ONDRA_H
#define MAME_TESLA_ONDRA_H

#pragma once

#include "imagedev/cassette.h"
#include "sound/beep.h"
#include "machine/ram.h"

class ondra_state : public driver_device
{
public:
	ondra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_rom(*this, "maincpu")
		, m_bank1(*this, "bank1")
		, m_bank3(*this, "bank3")
		, m_beep(*this, "beeper")
		, m_io_keyboard(*this, "LINE%u", 0U)
	{ }

	void ondra(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(nmi_button);

private:
	u8 keyboard_r(offs_t offset);
	void port03_w(u8 data);
	u8 port09_r();
	void port0a_w(u8 data);
	u32 screen_update_ondra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void update_banks();

	bool m_video_enable = false;
	u8 m_bank_status = 0;
	u8 m_bank_old = 0;

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_memory_bank m_bank1;
	required_memory_bank m_bank3;
	required_device<beep_device> m_beep;
	required_ioport_array<10> m_io_keyboard;
};

#endif // MAME_TESLA_ONDRA_H
