// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_SOMARI_H
#define MAME_BUS_NES_SOMARI_H

#pragma once

#include "mmc3.h"


// ======================> nes_somari_device

class nes_somari_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_somari_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override { write_m(offset + 0x100, data); }
	virtual void write_m(offs_t offset, uint8_t data) override;
	void mmc1_w(offs_t offset, uint8_t data);
	void mmc3_w(offs_t offset, uint8_t data);
	void vrc2_w(offs_t offset, uint8_t data);
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_prg();
	void update_chr();
	void update_mirror();
	void bank_update_switchmode();

	uint8_t m_board_mode;

	// MMC3 - inherited from txrom
	uint8_t m_mmc3_mirror_reg;

	// MMC1
	uint8_t m_count;
	uint8_t m_mmc1_latch;
	uint8_t m_mmc1_reg[4];

	// VRC2
	uint8_t m_vrc_prg_bank[2];
	uint8_t m_vrc_vrom_bank[8];
	uint8_t m_vrc_mirror_reg;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_SOMARI, nes_somari_device)

#endif // MAME_BUS_NES_SOMARI_H
