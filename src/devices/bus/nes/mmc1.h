// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_MMC1_H
#define MAME_BUS_NES_MMC1_H

#pragma once

#include "nxrom.h"


// ======================> nes_sxrom_device

class nes_sxrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	nes_sxrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	TIMER_CALLBACK_MEMBER(resync_callback);

	virtual void update_regs(int reg);      // this is needed to simplify NES-EVENT pcb implementation, which handle differently some regs!
	void set_prg(int prg_base, int prg_mask);
	void set_chr(int chr_base, int chr_mask);
	virtual void set_prg();
//  virtual void set_prg() { set_prg(0x00, 0x0f); }
	virtual void set_chr() { set_chr(0x00, 0x1f); }

	uint8_t m_reg[4];
	int m_reg_write_enable;
	int m_latch;
	int m_count;
};


// ======================> nes_sorom_device

class nes_sorom_device : public nes_sxrom_device
{
public:
	// construction/destruction
	nes_sorom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
};


// ======================> nes_szrom_device

class nes_szrom_device : public nes_sxrom_device
{
public:
	// construction/destruction
	nes_szrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_SXROM, nes_sxrom_device)
DECLARE_DEVICE_TYPE(NES_SOROM, nes_sorom_device)
DECLARE_DEVICE_TYPE(NES_SZROM, nes_szrom_device)

#endif // MAME_BUS_NES_MMC1_H
