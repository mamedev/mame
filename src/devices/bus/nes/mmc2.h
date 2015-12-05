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
	nes_pxrom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_pxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(pxrom_write);
	virtual DECLARE_WRITE8_MEMBER(write_h) override { pxrom_write(space, offset, data, mem_mask); }

	virtual void ppu_latch(offs_t offset) override;
	virtual void pcb_reset() override;

protected:
	UINT8 m_reg[4];
	int m_latch1, m_latch2;
};


// ======================> nes_fxrom_device

class nes_fxrom_device : public nes_pxrom_device
{
public:
	// construction/destruction
	nes_fxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;
};





// device type definition
extern const device_type NES_PXROM;
extern const device_type NES_FXROM;

#endif
