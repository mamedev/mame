// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KL5C80A16 CPU

***************************************************************************/

#ifndef MAME_CPU_Z80_KL5C80A16_H
#define MAME_CPU_Z80_KL5C80A16_H

#pragma once

#include "kc82.h"
#include "kp69.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class kl5c80a16_device : public kc82_device
{
public:
	enum
	{
		KP69_IRR = KC82_A3 + 1, KP69_ISR, KP69_IVR, KP69_LER, KP69_PGR, KP69_IMR
	};

	// device type constructor
	kl5c80a16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto in_p0_callback() { return m_port_in_callback[0].bind(); }
	auto out_p0_callback() { return m_port_out_callback[0].bind(); }
	auto in_p1_callback() { return m_port_in_callback[1].bind(); }
	auto out_p1_callback() { return m_port_out_callback[1].bind(); }
	auto in_p2_callback() { return m_port_in_callback[2].bind(); }
	auto out_p2_callback() { return m_port_out_callback[2].bind(); }
	auto in_p3_callback() { return m_port_in_callback[3].bind(); }
	auto out_p3_callback() { return m_port_out_callback[3].bind(); }

	// misc. configuration
	void set_p0_3state(u8 value) { m_port_3state[0] = value; }
	void set_p1_3state(u8 value) { m_port_3state[1] = value; }
	void set_p2_3state(u8 value) { m_port_3state[2] = value; }
	void set_p3_3state(u8 value) { m_port_3state[3] = value; }

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
	u8 port_r(offs_t offset);
	void port_w(offs_t offset, u8 data);

	// internal address map
	void internal_io(address_map &map) ATTR_COLD;

	// subdevice finders
	required_device<kp69_device> m_kp69;

	// callback objects
	devcb_read8::array<4> m_port_in_callback;
	devcb_write8::array<4> m_port_out_callback;

	// parallel port state
	u8 m_port_data[4];
	u8 m_port_direction[4];
	u8 m_port_3state[4];
};


// device type declaration
DECLARE_DEVICE_TYPE(KL5C80A16, kl5c80a16_device)

#endif // MAME_CPU_Z80_KL5C80A16_H
