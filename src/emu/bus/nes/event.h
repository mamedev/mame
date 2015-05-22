// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_EVENT_H
#define __NES_EVENT_H

#include "mmc1.h"


// ======================> nes_event_device

class nes_event_device : public nes_sxrom_device
{
public:
	// construction/destruction
	nes_event_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual ioport_constructor device_input_ports() const;
	virtual void update_regs(int reg);

	required_ioport m_dsw;

	virtual void pcb_reset();

protected:
	virtual void set_prg();
	virtual void set_chr();
	int m_nwc_init;

	static const device_timer_id TIMER_EVENT = 0;
	emu_timer *event_timer;
	attotime timer_freq;

	UINT32 m_timer_count;
	int m_timer_on, m_timer_enabled;
};


// device type definition
extern const device_type NES_EVENT;

#endif
