// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_IREM_H
#define MAME_BUS_NES_IREM_H

#pragma once

#include "nxrom.h"


// ======================> nes_lrog017_device

class nes_lrog017_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_lrog017_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_holydivr_device

class nes_holydivr_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_holydivr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_tam_s1_device

class nes_tam_s1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tam_s1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_g101_device

class nes_g101_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_g101_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

	uint8_t     m_latch;
};


// ======================> nes_h3001_device

class nes_h3001_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_h3001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	uint16_t     m_irq_count, m_irq_count_latch;
	int        m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_LROG017,  nes_lrog017_device)
DECLARE_DEVICE_TYPE(NES_HOLYDIVR, nes_holydivr_device)
DECLARE_DEVICE_TYPE(NES_TAM_S1,   nes_tam_s1_device)
DECLARE_DEVICE_TYPE(NES_G101,     nes_g101_device)
DECLARE_DEVICE_TYPE(NES_H3001,    nes_h3001_device)

#endif // MAME_BUS_NES_IREM_H
