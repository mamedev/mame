// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_SOMARI_H
#define __NES_SOMARI_H

#include "mmc3.h"


// ======================> nes_somari_device

class nes_somari_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_somari_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { write_m(space, offset + 0x100, data, mem_mask); }
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void mmc1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void mmc3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void vrc2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
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
extern const device_type NES_SOMARI;

#endif
