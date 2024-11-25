// license:BSD-3-Clause
// copyright-holders: Jonathan Gevaryahu
/*
 * pinsnd88.h - D-12338-567 Williams Pin Sound '88 board (M68B09E + YM2151 + DAC, two channels)
 *
 */

#ifndef MAME_PINBALL_PINSND88_H
#define MAME_PINBALL_PINSND88_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "sound/dac.h"
#include "sound/ymopm.h"


class pinsnd88_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	pinsnd88_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// overrides
	virtual void device_reset() override ATTR_COLD; // power up reset

	// note to keep synchronization working, the host machine should have synchronization timer expired delegates
	// if possible, before writing to the following things:
	void strobe_w(int state); // external write to board (J1 pin 13)
	void data_w(uint8_t data); // external write to board data bus (J1 pins 3 thru 10 for D0-D7)
	void resetq_w(int state); // external write to board /RESET (J1 pin 18)

	// callbacks
	auto syncq_cb() { return m_syncq_cb.bind(); }

	void pinsnd88_map(address_map &map) ATTR_COLD;

protected:
	// constructor with overridable type for subclass, in case we want to put NARC's sound board in here eventually
	//pinsnd88_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	// overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(deferred_sync_w);

	// members
	required_device<mc6809e_device> m_cpu;
	required_device<ad7224_device> m_dac;
	required_device<ym2151_device> m_ym2151;
	required_memory_bank m_cpubank;

private:
	TIMER_CALLBACK_MEMBER(sync_callback);

	// members
	devcb_write_line m_syncq_cb;
	uint8_t m_old_resetq_state;
	uint8_t m_data_in;
	required_device<generic_latch_8_device> m_inputlatch;
	emu_timer* m_sync_timer;

	void common_reset(); // common reset function used by both internal and external reset
	void sync_w(uint8_t data); // internal helper for deferred sync writeback
	void bgbank_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(PINSND88, pinsnd88_device)

#endif // MAME_PINBALL_PINSND88_H
