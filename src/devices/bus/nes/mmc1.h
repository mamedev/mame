// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_MMC1_H
#define MAME_BUS_NES_MMC1_H

#pragma once

#include "nxrom.h"


// ======================> nes_sxrom_device
class m6502_device;
class nes_sxrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void mmc1_ppu_phase(bool upper_chr, uint16_t ppu_address) override;
	virtual void pcb_reset() override;
	
protected:
	nes_sxrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void update_regs(int reg);
	void set_prg(int prg_base, int prg_mask);
	void set_chr(int chr_base, int chr_mask);
	virtual void set_prg();
	virtual void set_chr() { set_chr(0x00, 0x1f); }
	virtual void set_mirror();
	virtual bool prgram_enabled() const;
	virtual u8 prgram_bank() const;
	
	u8 mmc1_active_chr_reg() const;
	bool m_powered = false;
	bool m_mmc1_upper_chr;
	uint16_t m_mmc1_ppu_addr;
	u8 m_reg[4];

private:
	u8 m_latch;
	u8 m_count;
	m6502_device* m_maincpu6502 = nullptr;
};


class nes_sxrom_ext_device : public nes_sxrom_device
{
public:
	nes_sxrom_ext_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void set_prg() override;
	virtual void set_chr() override;
	virtual u8 prgram_bank() const override;
	virtual void update_regs(int reg) override;
};

// ======================> nes_snrom_device

class nes_snrom_device : public nes_sxrom_device
{
public:
	nes_snrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void set_chr() override;
	virtual bool prgram_enabled() const override;
};


// ======================> nes_sorom_device

class nes_sorom_device : public nes_sxrom_device
{
public:
	nes_sorom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;

protected:
	virtual void set_chr() override;
};

// ======================> nes_surom_device

class nes_surom_device : public nes_sxrom_device
{
public:
	nes_surom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void set_prg() override;
	virtual void set_chr() override;
	virtual u8 prgram_bank() const override;
	virtual void update_regs(int reg) override;
};

// ======================> nes_szrom_device

class nes_szrom_device : public nes_sxrom_device
{
public:
	nes_szrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;

protected:
	virtual void set_chr() override;
	virtual u8 prgram_bank() const override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_SXROM, nes_sxrom_device)
DECLARE_DEVICE_TYPE(NES_SXROM_EXT, nes_sxrom_ext_device)
DECLARE_DEVICE_TYPE(NES_SNROM, nes_snrom_device)
DECLARE_DEVICE_TYPE(NES_SOROM, nes_sorom_device)
DECLARE_DEVICE_TYPE(NES_SUROM, nes_surom_device)
DECLARE_DEVICE_TYPE(NES_SZROM, nes_szrom_device)

#endif // MAME_BUS_NES_MMC1_H