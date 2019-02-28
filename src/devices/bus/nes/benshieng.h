// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_BENSHIENG_H
#define MAME_BUS_NES_BENSHIENG_H

#pragma once

#include "nxrom.h"


// ======================> nes_benshieng_device

class nes_benshieng_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_benshieng_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_banks();
	uint8_t m_dipsetting;
	uint8_t m_mmc_prg_bank[4];
	uint8_t m_mmc_vrom_bank[4];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_BENSHIENG, nes_benshieng_device)

#endif // MAME_BUS_NES_BENSHIENG_H
