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
	nes_lrog017_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_holydivr_device

class nes_holydivr_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_holydivr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_tam_s1_device

class nes_tam_s1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tam_s1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_g101_device

class nes_g101_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_g101_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_g101_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 prg_mask);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void set_prg();
	u8 m_latch;

private:
	u8 m_reg;
	const u8 m_prg_mask;
};


// ======================> nes_h3001_device

class nes_h3001_device : public nes_g101_device
{
public:
	// construction/destruction
	nes_h3001_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	u16 m_irq_count, m_irq_count_latch;
	u8 m_irq_enable;

	emu_timer *irq_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_LROG017,  nes_lrog017_device)
DECLARE_DEVICE_TYPE(NES_HOLYDIVR, nes_holydivr_device)
DECLARE_DEVICE_TYPE(NES_TAM_S1,   nes_tam_s1_device)
DECLARE_DEVICE_TYPE(NES_G101,     nes_g101_device)
DECLARE_DEVICE_TYPE(NES_H3001,    nes_h3001_device)

#endif // MAME_BUS_NES_IREM_H
