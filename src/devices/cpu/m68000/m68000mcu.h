// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_CPU_M68000_M68000mcu_H
#define MAME_CPU_M68000_M68000mcu_H

#pragma once

#include "m68000.h"

class m68000_mcu_device : public m68000_device
{
protected:
	// Opcode handlers (d = direct, i = indirect, f = full, p = partial)
	using handlerm = void (m68000_mcu_device::*)();

#include "m68000mcu-head.h"

	static const handlerm s_handlers_dfm[];
	static const handlerm s_handlers_ifm[];
	static const handlerm s_handlers_dpm[];
	static const handlerm s_handlers_ipm[];

	m68000_mcu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void execute_run() override;
	void recompute_bcount(uint64_t event_time);
	static void add_event(uint64_t &event_time, uint64_t new_event);
	void internal_update();

	virtual void internal_update(uint64_t current_time) = 0;
	virtual void device_start() override ATTR_COLD;

	void set_current_interrupt_level(u32 level);
};

#endif
