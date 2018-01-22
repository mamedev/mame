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

	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_READ8_MEMBER(read_m) override { return read_l(space, offset, mem_mask); }
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual void chr_cb( int start, int bank, int source ) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_extra;
};


// ======================> nes_rex_sl1632_device

class nes_rex_sl1632_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_rex_sl1632_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void set_prg(int prg_base, int prg_mask) override;
	virtual void set_chr(uint8_t chr, int chr_base, int chr_mask) override;

	uint8_t m_mode, m_mirror;
	uint8_t m_extra_bank[12];
};



// device type definition
DECLARE_DEVICE_TYPE(NES_REX_DBZ5,   nes_rex_dbz5_device)
DECLARE_DEVICE_TYPE(NES_REX_SL1632, nes_rex_sl1632_device)

#endif // MAME_BUS_NES_REXSOFT_H
