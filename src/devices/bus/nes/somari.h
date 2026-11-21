// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_SOMARI_H
#define MAME_BUS_NES_SOMARI_H

#pragma once

#include "mmc3.h"


// ======================> nes_somari_device

class nes_somari_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_somari_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override { write_m(offset + 0x100, data); }
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_somari_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 mmc1_prg_shift);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// outer bank of the reset-based multicart (NES 2.0 submapper 3); with m_outer_on clear nothing is changed.
	// PRG in 8 KiB banks, CHR in 1 KiB banks: bank = (bank & and) | or
	bool m_outer_on = false;
	u16 m_prg_and = 0xff, m_prg_or = 0;
	u16 m_chr_and = 0x3ff, m_chr_or = 0;

	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	void update_all_banks();

private:
	void mmc1_reset_latch();
	void mmc1_w(offs_t offset, u8 data);
	void vrc2_w(offs_t offset, u8 data);
	void update_prg();
	void update_chr();
	void update_mirror();

	u8 m_board_mode;

	// MMC1
	u8 m_mmc1_count;
	u8 m_mmc1_latch;
	u8 m_mmc1_reg[4];
	const u8 m_mmc1_prg_shift;

	// VRC2
	u8 m_vrc_prg_bank[2];
	u8 m_vrc_vrom_bank[8];
	u8 m_vrc_mirror;
};


// ======================> nes_huang2_device

class nes_huang2_device : public nes_somari_device
{
public:
	// construction/destruction
	nes_huang2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_somari_mc_device

class nes_somari_mc_device : public nes_somari_device
{
public:
	// construction/destruction
	nes_somari_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void pcb_reset() override;

protected:
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_game;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_SOMARI, nes_somari_device)
DECLARE_DEVICE_TYPE(NES_SOMARI_MC, nes_somari_mc_device)
DECLARE_DEVICE_TYPE(NES_HUANG2, nes_huang2_device)

#endif // MAME_BUS_NES_SOMARI_H
