// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 1 keyboard emulation

*********************************************************************/
#ifndef MAME_NOKIA_MM1KB_H
#define MAME_NOKIA_MM1KB_H

#pragma once

#include "sound/beep.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mm1_keyboard_device

class mm1_keyboard_device : public device_t
{
public:
	// construction/destruction
	mm1_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto kbst_wr_callback() { return m_write_kbst.bind(); }

	uint8_t read() { return m_data; }

	void bell_w(int state) { m_beeper->set_state(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(scan_keyboard);

private:
	devcb_write_line m_write_kbst;

	required_device<beep_device> m_beeper;
	required_memory_region m_rom;
	required_ioport_array<10> m_y;
	required_ioport m_special;

	int m_sense;
	int m_drive;
	uint8_t m_data;

	emu_timer *m_scan_timer; // scan timer
};


// device type definition
DECLARE_DEVICE_TYPE(MM1_KEYBOARD, mm1_keyboard_device)


#endif // MAME_NOKIA_MM1KB_H
