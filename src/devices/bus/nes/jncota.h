// license:BSD-3-Clause
// copyright-holders:kmg
#ifndef MAME_BUS_NES_JNCOTA_H
#define MAME_BUS_NES_JNCOTA_H

#pragma once

#include "nxrom.h"


// ======================> nes_jncota_kt1001_device

class nes_jncota_kt1001_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jncota_kt1001_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg[3];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_JNCOTA_KT1001, nes_jncota_kt1001_device)

#endif // MAME_BUS_NES_JNCOTA_H
