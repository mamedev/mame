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
	nes_nina001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_m) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_nina006_device

class nes_nina006_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nina006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_l) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_maxi15_device

class nes_maxi15_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_maxi15_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_banks();

	uint8_t m_reg, m_bank;
};



// device type definition
DECLARE_DEVICE_TYPE(NES_NINA001, nes_nina001_device)
DECLARE_DEVICE_TYPE(NES_NINA006, nes_nina006_device)
DECLARE_DEVICE_TYPE(NES_MAXI15,  nes_maxi15_device)

#endif // MAME_BUS_NES_AVE_H
