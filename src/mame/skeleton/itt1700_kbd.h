// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_SKELETON_ITT1700_KBD
#define MAME_SKELETON_ITT1700_KBD

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> itt1700_keyboard_device

class itt1700_keyboard_device : public device_t
{
public:
	// device constructor
	itt1700_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// host interface
	void line1_w(int state);
	void line2_w(int state);
	void clock_w(int state);
	int sense_r();

protected:
	// device-specific overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	// internal state
	bool m_clock_state;
	bool m_line1_state;
	bool m_line2_state;
	u8 m_scan_counter;

	// key input matrix
	required_ioport_array<16> m_keys;
};

// device type definition
DECLARE_DEVICE_TYPE(ITT1700_KEYBOARD, itt1700_keyboard_device)

#endif // MAME_SKELETON_ITT1700_KBD_H
