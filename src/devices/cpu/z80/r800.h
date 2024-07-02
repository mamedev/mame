// license:BSD-3-Clause
// copyright-holders:AJR,Wilbert Pol
/***************************************************************************

    ASCII R800 CPU

***************************************************************************/

#ifndef MAME_CPU_Z80_R800_H
#define MAME_CPU_Z80_R800_H

#pragma once

#include "z80.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class r800_device : public z80_device
{
public:
	// device type constructor
	r800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type imperfect_features() { return feature::TIMING; }

protected:
	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); }

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	u8 r800_sll(u8 value);
	void mulub(u8 value);
	void muluw(u16 value);

	virtual void do_op() override;
};

// device type declaration
DECLARE_DEVICE_TYPE(R800, r800_device)

#endif // MAME_CPU_Z80_R800_H
