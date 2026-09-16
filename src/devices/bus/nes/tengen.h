// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_TENGEN_H
#define MAME_BUS_NES_TENGEN_H

#pragma once

#include "nxrom.h"


class m6502_device;


// ======================> nes_tengen032_device

class nes_tengen032_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tengen032_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void ppu_bus_address(uint16_t ppu_addr, uint64_t ppu_cycles, int ppu_tick, bool odd_frame) override;
	virtual void ppu_to_mapper(int scanline, unsigned dot, int ppu_tick, uint16_t ppu_address) override;

	virtual void pcb_reset() override;

protected:
	nes_tengen032_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_chr();

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

	u8 m_latch;
	u8 m_mmc_vrom_bank[8];

private:
	void set_prg();
	void irq_clock();

	u16 m_irq_count;
	u16 m_irq_count_latch;

	u8 m_irq_mode;
	u8 m_irq_reset;
	u8 m_irq_enable;
	int delay_irq;
	u64 m_irq_delay_cpu_cycle;
	int m_irq_cpu_delay;

	u8 m_mmc_prg_bank[3];

	emu_timer *irq_timer;
	attotime timer_freq;

	uint64_t m_last_a12_low_cycle;
	uint16_t m_prev_ppu_addr;
	bool m_a12_low_seen;

	m6502_device *m_maincpu6502;
	bool m_irq_force_clock;
	
};


// ======================> nes_tengen037_device

class nes_tengen037_device : public nes_tengen032_device
{
public:
	// construction/destruction
	nes_tengen037_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void pcb_reset() override;

protected:
	virtual void set_chr() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TENGEN_800032, nes_tengen032_device)
DECLARE_DEVICE_TYPE(NES_TENGEN_800037, nes_tengen037_device)

#endif // MAME_BUS_NES_TENGEN_H