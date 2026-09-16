// license:BSD-3-Clause
// copyright-holders:kmg
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


// ======================> nes_dpcmcart_device

class nes_dpcmcart_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_dpcmcart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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

protected:
	// construction/destruction
	nes_unrom512_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_8bitxmas_device

class nes_8bitxmas_device : public nes_unrom512_device
{
public:
	// construction/destruction
	nes_8bitxmas_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_led();
	u8 m_led;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_8BITXMAS, nes_8bitxmas_device)
DECLARE_DEVICE_TYPE(NES_CUFROM,   nes_cufrom_device)
DECLARE_DEVICE_TYPE(NES_DPCMCART, nes_dpcmcart_device)
DECLARE_DEVICE_TYPE(NES_UNROM512, nes_unrom512_device)

#endif // MAME_BUS_NES_SEALIE_H
