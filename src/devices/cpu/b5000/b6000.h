// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B6000 MCU

*/

#ifndef MAME_CPU_B5000_B6000_H
#define MAME_CPU_B5000_B6000_H

#pragma once

#include "b5000.h"

// pinout reference

/*

*/

class b6000_cpu_device : public b5000_cpu_device
{
public:
	b6000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	b6000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device-level overrides
	virtual void device_reset() override;

	void update_speaker();
	virtual void execute_one() override;
	virtual u16 decode_digit(u8 data) override;

	void program_512x8(address_map &map);

	// opcode handlers
	virtual void op_tkbs() override;
	virtual void op_atbz() override;
};


DECLARE_DEVICE_TYPE(B6000, b6000_cpu_device)

#endif // MAME_CPU_B5000_B6000_H
