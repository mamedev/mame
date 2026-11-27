// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,kmg
#ifndef MAME_BUS_NES_EVENT_H
#define MAME_BUS_NES_EVENT_H

#pragma once

#include "mmc1.h"
#include "mmc3.h"


// ======================> nes_event_device

class nes_event_device : public nes_sxrom_device
{
public:
	// construction/destruction
	nes_event_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void set_prg() override;
	virtual void set_chr() override;
	// $A000 is repurposed (PRG select and timer control), so its bits are not SNROM/SOROM-style PRG/RAM lines
	virtual bool chr_bits_drive_prg() const override { return false; }

	TIMER_CALLBACK_MEMBER(event_tick);
	// M2 at the cartridge connector: while the timer runs it counts M2 rises (one per CPU cycle)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state && m_m2_running) event_tick(0); }
	bool m_m2_running = false;

	required_ioport m_dsw;

	int m_nwc_init;

	emu_timer *event_timer;

	u32 m_timer_count;
	int m_timer_on, m_timer_enabled;
};

// ======================> nes_event2_device

class nes_event2_device : public nes_tqrom_device
{
public:
	// construction/destruction
	nes_event2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(event_tick);
	// M2 at the cartridge connector: the counter advances once per CPU cycle, at the M2 rise
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) event_tick(0); }

	required_ioport m_dsw;

	bool m_tqrom_mode;

	emu_timer *event_timer;

	u32 m_timer_count;
	int m_timer_enabled;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_EVENT,  nes_event_device)
DECLARE_DEVICE_TYPE(NES_EVENT2, nes_event2_device)

#endif // MAME_BUS_NES_EVENT_H
