// license:BSD-3-Clause
// copyright-holders: kmg, Fabio Priuli
#ifndef MAME_BUS_NES_SEALIE_H
#define MAME_BUS_NES_SEALIE_H

#pragma once

#include "nxrom.h"


// ======================> nes_cufrom_device

class nes_cufrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cufrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_unrom512_device

class nes_unrom512_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_unrom512_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_CUFROM,   nes_cufrom_device)
DECLARE_DEVICE_TYPE(NES_UNROM512, nes_unrom512_device)

#endif // MAME_BUS_NES_SEALIE_H
