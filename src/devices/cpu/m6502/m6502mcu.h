// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502mcu.h

    Helper for 6502 variants with internal peripherals

***************************************************************************/

#ifndef MAME_CPU_M6502_M6502MCU_H
#define MAME_CPU_M6502_M6502MCU_H

#pragma once

#include "m6502.h"


template <typename Base>
class m6502_mcu_device_base : public Base {
protected:
	m6502_mcu_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void internal_update() { internal_update(this->total_cycles()); }
	virtual void internal_update(uint64_t current_time) = 0;
	void recompute_bcount(uint64_t event_time);

	static void add_event(uint64_t &event_time, uint64_t new_event)
	{
		if(new_event && (!event_time || event_time > new_event))
			event_time = new_event;
	}

	virtual void execute_run() override;
};

#endif // MAME_CPU_M6502_M6502MCU_H
