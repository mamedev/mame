// license:BSD-3-Clause
// copyright-holders:Brian H. Gilbert
#ifndef MAME_BUS_NES_MINDKIDS_H
#define MAME_BUS_NES_MINDKIDS_H

#pragma once

#include "mmc3.h"
#include "sound/ay8910.h"


// ======================> nes_smd172b_device

class nes_smd172b_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_smd172b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void chr_w(offs_t offset, u8 data) override;

	virtual void set_smd172b_submapper(u8 val) override { m_submapper = val; }

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void set_prg(int prg_base, int prg_mask) override;
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;

	// the H3001 and FME-7 IRQ counters count CPU cycles
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;

private:
	enum : u8 { MODE_SUPERVISOR, MODE_UXROM, MODE_MMC1, MODE_SUROM, MODE_BNROM, MODE_CNROM, MODE_AXROM, MODE_GNROM,
		MODE_MMC3, MODE_TXSROM, MODE_MMC6, MODE_152, MODE_H3001, MODE_FME7, MODE_UNSUPPORTED };

	void select_mode();
	void update_banks();
	void set_mirror_reg4();
	int prg_and() const { return (m_reg[3] ^ (m_submapper == 2 ? 0x00 : 0xff)) & 0xff; }
	int prg_or() const { return m_reg[1] | (m_reg[2] << 8); }
	int chr_and() const { return (((m_reg[4] << 2) & 0xe0) ^ 0xff) & 0xff; }
	int chr_or() const { return m_reg[6]; }
	int prg8(int bank) const { return (bank & prg_and()) | (prg_or() & ~prg_and()); }
	int chr1(int bank) const { return (bank & chr_and()) | (chr_or() & ~chr_and()); }

	void mmc1_write(offs_t offset, u8 data);
	void h3001_write(offs_t offset, u8 data);
	void fme7_write(offs_t offset, u8 data);

	required_device<sunsoft_5b_sound_device> m_ym2149;

	u8 m_submapper;
	u8 m_reg[8];        // supervisor registers $5000-$5007
	u8 m_mode;
	u8 m_dlatch;        // discrete modes' latch

	u8 m_mmc1_reg[4];
	u8 m_mmc1_shift;
	u8 m_mmc1_count;
	u64 m_mmc1_last_write;

	u8 m_h3001_prg[2];
	u8 m_h3001_swap;
	u8 m_h3001_nt;
	u8 m_h3001_chr[8];
	u8 m_h3001_irq_enable;
	u16 m_h3001_irq_count, m_h3001_irq_latch;

	u8 m_fme7_cmd;
	u8 m_fme7_prg[4];   // $6000, $8000, $A000, $C000
	u8 m_fme7_chr[8];
	u8 m_fme7_irq_ctrl;
	u16 m_fme7_irq_count;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_SMD172B, nes_smd172b_device)

#endif // MAME_BUS_NES_MINDKIDS_H
