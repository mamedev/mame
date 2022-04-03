// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_HENGGEDIANZI_H
#define MAME_BUS_NES_HENGGEDIANZI_H

#pragma once

#include "nxrom.h"


// ======================> nes_hengg_srich_device

class nes_hengg_srich_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_hengg_srich_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_hengg_xhzs_device

class nes_hengg_xhzs_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_hengg_xhzs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_HENGG_SRICH, nes_hengg_srich_device)
DECLARE_DEVICE_TYPE(NES_HENGG_XHZS,  nes_hengg_xhzs_device)

#endif // MAME_BUS_NES_HENGGEDIANZI_H
