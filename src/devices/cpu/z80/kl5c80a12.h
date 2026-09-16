// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KL5C80A12 CPU

***************************************************************************/

#ifndef MAME_CPU_Z80_KL5C80A12_H
#define MAME_CPU_Z80_KL5C80A12_H

#pragma once

#include "kc82.h"
#include "kp69.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class kl5c80a12_device : public kc82_device
{
public:
	enum
	{
		KP69_IRR = KC82_A3 + 1, KP69_ISR, KP69_IVR, KP69_LER, KP69_PGR, KP69_IMR
	};

	// device type constructor
	kl5c80a12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto in_p0_callback() { return m_porta_in_callback[0].bind(); }
	auto out_p0_callback() { return m_porta_out_callback[0].bind(); }
	auto in_p1_callback() { return m_porta_in_callback[1].bind(); }
	auto out_p1_callback() { return m_porta_out_callback[1].bind(); }
	auto in_p2_callback() { return m_portb_in_callback[0].bind(); }
	auto out_p2_callback() { return m_portb_out_callback[0].bind(); }
	auto in_p3_callback() { return m_portb_in_callback[1].bind(); }
	auto out_p3_callback() { return m_portb_out_callback[1].bind(); }
	auto in_p4_callback() { return m_portb_in_callback[2].bind(); }
	auto out_p4_callback() { return m_portb_out_callback[2].bind(); }

	// misc. configuration
	void set_p0_3state(u8 value) { m_porta_3state[0] = value; }
	void set_p1_3state(u8 value) { m_porta_3state[1] = value; }
	void set_p2_3state(u8 value) { m_portb_3state[0] = value; }
	void set_p3_3state(u8 value) { m_portb_3state[1] = value; }
	void set_p4_3state(u8 value) { m_portb_3state[2] = value; }

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 2); }

private:
	// internal I/O handlers
	u8 porta_r(offs_t offset);
	void porta_w(offs_t offset, u8 data);
	u8 portb_r(offs_t offset);
	void portb_w(offs_t offset, u8 data);
	u8 portb_control_r();
	void portb_control_w(u8 data);

	// internal helpers
	void portb_update_output(unsigned n);

	// internal address maps
	void internal_ram(address_map &map) ATTR_COLD;
	void internal_io(address_map &map) ATTR_COLD;

	// subdevice finders
	required_device<kp69_device> m_kp69;

	// callback objects
	devcb_read8::array<2> m_porta_in_callback;
	devcb_write8::array<2> m_porta_out_callback;
	devcb_read8::array<3> m_portb_in_callback;
	devcb_write8::array<3> m_portb_out_callback;

	// parallel port A state
	u8 m_porta_data[2];
	u8 m_porta_direction[2];
	u8 m_porta_3state[2];

	// parallel port B state
	u8 m_portb_data[3];
	u8 m_portb_direction;
	u8 m_portb_3state[3];
};


// device type declaration
DECLARE_DEVICE_TYPE(KL5C80A12, kl5c80a12_device)

#endif // MAME_CPU_Z80_KL5C80A12_H
