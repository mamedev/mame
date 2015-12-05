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
	nes_somari_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override { write_m(space, offset + 0x100, data, mem_mask); }
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(mmc1_w);
	virtual DECLARE_WRITE8_MEMBER(mmc3_w);
	virtual DECLARE_WRITE8_MEMBER(vrc2_w);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;
private:
	void update_prg();
	void update_chr();
	void update_mirror();
	void bank_update_switchmode();

	UINT8 m_board_mode;

	// MMC3 - inherited from txrom
	UINT8 m_mmc3_mirror_reg;

	// MMC1
	UINT8 m_count;
	UINT8 m_mmc1_latch;
	UINT8 m_mmc1_reg[4];

	// VRC2
	UINT8 m_vrc_prg_bank[2];
	UINT8 m_vrc_vrom_bank[8];
	UINT8 m_vrc_mirror_reg;
};



// device type definition
extern const device_type NES_SOMARI;

#endif
