// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_AVE_H
#define MAME_BUS_NES_AVE_H

#pragma once

#include "nxrom.h"


// ======================> nes_nina001_device

class nes_nina001_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nina001_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
};


// ======================> nes_nina006_device

class nes_nina006_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nina006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;
};


// ======================> nes_maxi15_device

class nes_maxi15_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_maxi15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg[2];
};



// device type definition
DECLARE_DEVICE_TYPE(NES_NINA001, nes_nina001_device)
DECLARE_DEVICE_TYPE(NES_NINA006, nes_nina006_device)
DECLARE_DEVICE_TYPE(NES_MAXI15,  nes_maxi15_device)

#endif // MAME_BUS_NES_AVE_H
