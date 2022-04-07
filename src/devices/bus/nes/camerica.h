// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_CAMERICA_H
#define MAME_BUS_NES_CAMERICA_H

#pragma once

#include "nxrom.h"


// ======================> nes_bf9093_device

class nes_bf9093_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bf9093_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bf9096_device

class nes_bf9096_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bf9096_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override { write_h(offset, data); }

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_bank_base, m_latch;
};


// ======================> nes_golden5_device

class nes_golden5_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_golden5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_bank_base, m_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_BF9093,  nes_bf9093_device)
DECLARE_DEVICE_TYPE(NES_BF9096,  nes_bf9096_device)
DECLARE_DEVICE_TYPE(NES_GOLDEN5, nes_golden5_device)

#endif // MAME_BUS_NES_CAMERICA_H
