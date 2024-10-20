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
	nes_bf9093_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bf9096_device

class nes_bf9096_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bf9096_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	nes_bf9096_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool page_swap);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg;
	const bool m_page_swap;
};


// ======================> nes_bf9096a_device

class nes_bf9096a_device : public nes_bf9096_device
{
public:
	// construction/destruction
	nes_bf9096a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_golden5_device

class nes_golden5_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_golden5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_lock, m_reg;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_BF9093,  nes_bf9093_device)
DECLARE_DEVICE_TYPE(NES_BF9096,  nes_bf9096_device)
DECLARE_DEVICE_TYPE(NES_BF9096A, nes_bf9096a_device)
DECLARE_DEVICE_TYPE(NES_GOLDEN5, nes_golden5_device)

#endif // MAME_BUS_NES_CAMERICA_H
