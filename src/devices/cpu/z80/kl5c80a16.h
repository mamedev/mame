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

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 2); }

private:
	// internal address map
	void internal_io(address_map &map);

	// subdevice finders
	required_device<kp69_device> m_kp69;
};


// device type declaration
DECLARE_DEVICE_TYPE(KL5C80A16, kl5c80a16_device)

#endif // MAME_CPU_Z80_KL5C80A16_H
