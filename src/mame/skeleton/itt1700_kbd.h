// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_MACHINE_ITT1700_KBD
#define MAME_MACHINE_ITT1700_KBD

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
	DECLARE_WRITE_LINE_MEMBER(line1_w);
	DECLARE_WRITE_LINE_MEMBER(line2_w);
	DECLARE_WRITE_LINE_MEMBER(clock_w);
	DECLARE_READ_LINE_MEMBER(sense_r);

protected:
	// device-specific overrides
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;

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

#endif // MAME_MACHINE_ITT1700_KBD_H
