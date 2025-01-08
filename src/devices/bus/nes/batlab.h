// license:BSD-3-Clause
// copyright-holders:kmg
#ifndef MAME_BUS_NES_BATLAB_H
#define MAME_BUS_NES_BATLAB_H

#pragma once

#include "mmc3.h"


// ======================> nes_batmap_000_device

class nes_batmap_000_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_batmap_000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_batmap_srrx_device

class nes_batmap_srrx_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_batmap_srrx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual u8 read_m(offs_t offset) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void hblank_irq(int scanline, bool vblank, bool blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 read_dpcm();
	u8 m_reg;
	u32 m_dpcm_addr;
	u8 m_dpcm_ctrl;

	u16 m_irq_count, m_irq_count_latch;
	int m_irq_enable;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_BATMAP_000,  nes_batmap_000_device)
DECLARE_DEVICE_TYPE(NES_BATMAP_SRRX, nes_batmap_srrx_device)

#endif // MAME_BUS_NES_BATLAB_H
