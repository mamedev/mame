// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_TENGEN_H
#define MAME_BUS_NES_TENGEN_H

#include "nxrom.h"


// ======================> nes_tengen032_device

class nes_tengen032_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tengen032_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void hblank_irq(int scanline, bool vblank, bool blanked) override;
	virtual void pcb_reset() override;

protected:
	nes_tengen032_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_chr();

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

	u8 m_latch;
	u8 m_mmc_vrom_bank[8];

private:
	void set_prg();
	void irq_clock(int blanked);

	u16 m_irq_count, m_irq_count_latch;
	u8 m_irq_mode, m_irq_reset;
	u8 m_irq_enable, m_irq_pending;

	u8 m_mmc_prg_bank[3];

	emu_timer *irq_timer;
	attotime timer_freq;
};


// ======================> nes_tengen037_device

class nes_tengen037_device : public nes_tengen032_device
{
public:
	// construction/destruction
	nes_tengen037_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void write_h(offs_t offset, u8 data) override;

protected:
	virtual void set_chr() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TENGEN_800032, nes_tengen032_device)
DECLARE_DEVICE_TYPE(NES_TENGEN_800037, nes_tengen037_device)

#endif // MAME_BUS_NES_TENGEN_H
