// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi M58846 MCU

*/

#ifndef MAME_CPU_MELPS4_M58846_H
#define MAME_CPU_MELPS4_M58846_H

#pragma once

#include "melps4.h"

// note: for pinout and more info, see melps4.h


class m58846_device : public melps4_cpu_device
{
public:
	m58846_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_one() override;

	// device_disasm_interface overrides
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	// timers
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void write_v(uint8_t data) override;

	emu_timer *m_timer;
	void reset_timer();
};


DECLARE_DEVICE_TYPE(M58846, m58846_device)

#endif // MAME_CPU_MELPS4_M58846_H
