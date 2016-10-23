// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_REXSOFT_H
#define __NES_REXSOFT_H

#include "mmc3.h"


// ======================> nes_rex_dbz5_device

class nes_rex_dbz5_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_rex_dbz5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override { return read_l(space, offset, mem_mask); }
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void chr_cb( int start, int bank, int source ) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_extra;
};


// ======================> nes_rex_sl1632_device

class nes_rex_sl1632_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_rex_sl1632_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

protected:
	virtual void set_prg(int prg_base, int prg_mask) override;
	virtual void set_chr(uint8_t chr, int chr_base, int chr_mask) override;

	uint8_t m_mode, m_mirror;
	uint8_t m_extra_bank[12];
};





// device type definition
extern const device_type NES_REX_DBZ5;
extern const device_type NES_REX_SL1632;

#endif
