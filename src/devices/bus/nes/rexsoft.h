// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_REXSOFT_H
#define MAME_BUS_NES_REXSOFT_H

#pragma once

#include "mmc3.h"


// ======================> nes_rex_dbz5_device

class nes_rex_dbz5_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_rex_dbz5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override { return read_l(offset); }
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_extra;
};


// ======================> nes_rex_sl1632_device

class nes_rex_sl1632_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_rex_sl1632_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg(int prg_base, int prg_mask) override;

	u8 m_mode;
	u8 m_mirror[2];
	u8 m_vrc2_prg_bank[2];
	u8 m_vrc2_vrom_bank[8];
};



// device type definition
DECLARE_DEVICE_TYPE(NES_REX_DBZ5,   nes_rex_dbz5_device)
DECLARE_DEVICE_TYPE(NES_REX_SL1632, nes_rex_sl1632_device)

#endif // MAME_BUS_NES_REXSOFT_H
