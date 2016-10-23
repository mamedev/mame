// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_MMC2_H
#define __NES_MMC2_H

#include "nxrom.h"


// ======================> nes_pxrom_device

class nes_pxrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_pxrom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_pxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void pxrom_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { pxrom_write(space, offset, data, mem_mask); }

	virtual void ppu_latch(offs_t offset) override;
	virtual void pcb_reset() override;

protected:
	uint8_t m_reg[4];
	int m_latch1, m_latch2;
};


// ======================> nes_fxrom_device

class nes_fxrom_device : public nes_pxrom_device
{
public:
	// construction/destruction
	nes_fxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};





// device type definition
extern const device_type NES_PXROM;
extern const device_type NES_FXROM;

#endif
