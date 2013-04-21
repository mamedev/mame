#ifndef __NES_SOMARI_H
#define __NES_SOMARI_H

#include "machine/nes_mmc3.h"


// ======================> nes_somari_device

class nes_somari_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_somari_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(mmc1_w);
	virtual DECLARE_WRITE8_MEMBER(mmc3_w);
	virtual DECLARE_WRITE8_MEMBER(vrc2_w);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();
private:
	void mmc1_set_prg();
	void mmc1_set_chr();
	void bank_update_switchmode();

	UINT8 m_board_mode;

	// MMC3 - inherited from txrom

	// MMC1
	UINT8 m_count;
	UINT8 m_mmc1_latch;
	UINT8 m_mmc1_reg[4];

	// VRC2
	UINT8 m_vrc_prg_bank[2];
	UINT8 m_vrc_vrom_bank[8];
};



// device type definition
extern const device_type NES_SOMARI;

#endif
