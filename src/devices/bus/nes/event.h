// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_EVENT_H
#define MAME_BUS_NES_EVENT_H

#pragma once

#include "mmc1.h"


// ======================> nes_event_device

class nes_event_device : public nes_sxrom_device
{
public:
	// construction/destruction
	nes_event_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;

	virtual void pcb_reset() override;

protected:
	static constexpr device_timer_id TIMER_EVENT = 0;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void update_regs(int reg) override;
	virtual void set_prg() override;
	virtual void set_chr() override;

	required_ioport m_dsw;

	int m_nwc_init;

	emu_timer *event_timer;
	attotime timer_freq;

	uint32_t m_timer_count;
	int m_timer_on, m_timer_enabled;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_EVENT, nes_event_device)

#endif // MAME_BUS_NES_EVENT_H
