// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD MMU SuperAdapter emulation

**********************************************************************/

#ifndef MAME_BUS_C128_SUPERCPU128_MMU_H
#define MAME_BUS_C128_SUPERCPU128_MMU_H

#pragma once

#include "mmu.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cmd_mmu_superadapter_device

class cmd_mmu_superadapter_device : public c128_mmu_mos8722_device
{
public:
	cmd_mmu_superadapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_c128_mmu_interface implementation
	virtual uint8_t read(offs_t offset, uint8_t data) override;
	virtual void write(offs_t offset, uint8_t data) override;
	virtual int aec_r(int state) override { return state || m_dma; }
	// FIXME: the board must see the ungated /DMA somewhere, since AEC at the socket is already gated by U61
	virtual void dma_w(int state) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	bool m_dma;
	bool m_drive;
	uint8_t m_latch;

	void update();
};


DECLARE_DEVICE_TYPE(CMD_MMU_SUPERADAPTER, cmd_mmu_superadapter_device)

#endif // MAME_BUS_C128_SUPERCPU128_MMU_H
