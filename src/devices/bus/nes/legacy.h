// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_LEGACY_H
#define MAME_BUS_NES_LEGACY_H

#pragma once

#include "nxrom.h"


// ======================> nes_ffe3_device

class nes_ffe3_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ffe3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
};


// ======================> nes_ffe4_device

class nes_ffe4_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ffe4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual uint8_t chr_r(offs_t offset) override;
	virtual void chr_w(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	nes_ffe4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

	uint16_t m_irq_count;
	int m_irq_enable;

	emu_timer *irq_timer;

	uint8_t m_latch;
	int m_exram_enabled;
	int m_exram_bank;
	uint8_t m_exram[0x8000];
};


// ======================> nes_ffe8_device

class nes_ffe8_device : public nes_ffe4_device
{
public:
	// construction/destruction
	nes_ffe8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override {}

	virtual void pcb_reset() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_FFE3, nes_ffe3_device)
DECLARE_DEVICE_TYPE(NES_FFE4, nes_ffe4_device)
DECLARE_DEVICE_TYPE(NES_FFE8, nes_ffe8_device)

#endif // MAME_BUS_NES_LEGACY_H
