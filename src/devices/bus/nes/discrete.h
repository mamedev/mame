// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_DISCRETE_H
#define MAME_BUS_NES_DISCRETE_H

#pragma once

#include "nxrom.h"


// ======================> nes_74x161x161x32_device

class nes_74x161x161x32_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_74x161x161x32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_74x139x74_device

class nes_74x139x74_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_74x139x74_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_74x377_device

class nes_74x377_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_74x377_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_74x161x138_device

class nes_74x161x138_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_74x161x138_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_74X161X161X32, nes_74x161x161x32_device)
DECLARE_DEVICE_TYPE(NES_74X139X74,     nes_74x139x74_device)
DECLARE_DEVICE_TYPE(NES_74X377,        nes_74x377_device)
DECLARE_DEVICE_TYPE(NES_74X161X138,    nes_74x161x138_device)

#endif // MAME_BUS_NES_DISCRETE_H
