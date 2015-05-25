// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_BENSHIENG_H
#define __NES_BENSHIENG_H

#include "nxrom.h"


// ======================> nes_benshieng_device

class nes_benshieng_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_benshieng_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	void update_banks();
	UINT8 m_dipsetting;
	UINT8 m_mmc_prg_bank[4];
	UINT8 m_mmc_vrom_bank[4];
};


// device type definition
extern const device_type NES_BENSHIENG;

#endif
