// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_MMC3_H
#define MAME_BUS_NES_MMC3_H

#pragma once

#include "nxrom.h"


// ======================> nes_txrom_device

class nes_txrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void txrom_write(offs_t offset, uint8_t data);
	virtual void write_h(offs_t offset, uint8_t data) override { txrom_write(offset, data); }
	virtual void prg_cb(int start, int bank);
	virtual void chr_cb(int start, int bank, int source);

	virtual void hblank_irq(int scanline, bool vblank, bool blanked) override;
	virtual void pcb_reset() override;

protected:
	nes_txrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { mmc3_start(); }

	virtual void set_prg(int prg_base, int prg_mask);
	virtual void set_chr(uint8_t chr, int chr_base, int chr_mask);
	void mmc3_start();
	void mmc3_common_initialize(int prg_mask, int chr_mask, int IRQ_type);

	// are there MMC3 clones which need more regs?
	uint16_t m_mmc_prg_bank[4];
	uint16_t m_mmc_vrom_bank[8];  // a few clones need more than the 6 banks used by base MMC3 (e.g. waixing_g)
	uint8_t m_mmc_mirror;

	int m_prg_base, m_prg_mask; // MMC3 based multigame carts select a block of banks by using these (and then act like normal MMC3),
	int m_chr_base, m_chr_mask; // while MMC3 and clones (mapper 118 & 119) simply set them as 0 and 0xff resp.

	int m_latch;
	int m_wram_protect;
	int m_alt_irq;

	uint16_t m_irq_count, m_irq_count_latch;
	uint8_t m_irq_clear;
	int m_irq_enable;
};


// ======================> nes_hkrom_device

class nes_hkrom_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_hkrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	int m_wram_enable;
	uint8_t m_mmc6_reg;

	// MMC-6 contains 1K of internal ram, battery backed up
	uint8_t m_mmc6_ram[0x400];
};


// ======================> nes_txsrom_device

class nes_txsrom_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_txsrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// construction/destruction
	nes_txsrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;
};


// ======================> nes_tqrom_device

class nes_tqrom_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_tqrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void chr_cb(int start, int bank, int source) override;

protected:
	// construction/destruction
	nes_tqrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_qj_device

class nes_qj_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_qj_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_zz_device

class nes_zz_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_zz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TXROM,  nes_txrom_device)
DECLARE_DEVICE_TYPE(NES_HKROM,  nes_hkrom_device)
DECLARE_DEVICE_TYPE(NES_TXSROM, nes_txsrom_device)
DECLARE_DEVICE_TYPE(NES_TQROM,  nes_tqrom_device)
DECLARE_DEVICE_TYPE(NES_QJ_PCB, nes_qj_device)
DECLARE_DEVICE_TYPE(NES_ZZ_PCB, nes_zz_device)

#endif // MAME_BUS_NES_MMC3_H
