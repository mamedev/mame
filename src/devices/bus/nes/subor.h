// license:BSD-3-Clause
// copyright-holders:Kaz, Fabio Priuli
#ifndef MAME_BUS_NES_SUBOR_H
#define MAME_BUS_NES_SUBOR_H

#pragma once

#include "nxrom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_subor0_device

class nes_subor0_device :
		public nes_nrom_device
{
public:
	// construction/destruction
	nes_subor0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_reg[4];
};

// ======================> nes_subor1_device

class nes_subor1_device :
		public nes_nrom_device
{
public:
	// construction/destruction
	nes_subor1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_reg[4];
};

// ======================> nes_subor2_device

class nes_subor2_device :
		public nes_nrom_device
{
public:
	// construction/destruction
	nes_subor2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t nt_r(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual uint8_t read_l(offs_t offset) override;

	virtual void ppu_latch(offs_t offset) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_banks();
	uint8_t m_switch_reg, m_bank_reg, m_chr_banking, m_page;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_SUBOR0, nes_subor0_device)
DECLARE_DEVICE_TYPE(NES_SUBOR1, nes_subor1_device)
DECLARE_DEVICE_TYPE(NES_SUBOR2, nes_subor2_device)

#endif // MAME_BUS_NES_SUBOR_H
