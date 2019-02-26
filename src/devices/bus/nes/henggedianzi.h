// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_HENGGEDIANZI_H
#define MAME_BUS_NES_HENGGEDIANZI_H

#pragma once

#include "nxrom.h"


// ======================> nes_hengg_srich_device

class nes_hengg_srich_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_hengg_srich_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_hengg_xhzs_device

class nes_hengg_xhzs_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_hengg_xhzs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_hengg_shjy3_device

class nes_hengg_shjy3_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_hengg_shjy3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_banks();

	uint16_t m_irq_count, m_irq_count_latch;
	int m_irq_enable;

	int m_chr_mode;
	uint8_t m_mmc_prg_bank[2];
	uint8_t m_mmc_vrom_bank[8];
	uint8_t m_mmc_extra_bank[8];
};

// device type definition
DECLARE_DEVICE_TYPE(NES_HENGG_SRICH, nes_hengg_srich_device)
DECLARE_DEVICE_TYPE(NES_HENGG_XHZS,  nes_hengg_xhzs_device)
DECLARE_DEVICE_TYPE(NES_HENGG_SHJY3, nes_hengg_shjy3_device)

#endif // MAME_BUS_NES_HENGGEDIANZI_H
