// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_TENGEN_H
#define MAME_BUS_NES_TENGEN_H

#include "nxrom.h"


// ======================> nes_tengen008_device

class nes_tengen008_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tengen008_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_tengen032_device

class nes_tengen032_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tengen032_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void tengen032_write(offs_t offset, uint8_t data);
	virtual void write_h(offs_t offset, uint8_t data) override { tengen032_write(offset, data); }

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	nes_tengen032_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void chr_cb(int start, int bank, int source);

	void set_prg();
	void set_chr();
	void irq_clock(int blanked);

	uint16_t m_irq_count, m_irq_count_latch;
	uint8_t m_irq_mode, m_irq_reset;
	int m_irq_enable;

	uint8_t m_latch;
	uint8_t m_mmc_prg_bank[3];
	uint8_t m_mmc_vrom_bank[8];

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
	attotime timer_freq;
};


// ======================> nes_tengen037_device

class nes_tengen037_device : public nes_tengen032_device
{
public:
	// construction/destruction
	nes_tengen037_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void chr_cb(int start, int bank, int source) override;

protected:
	void set_mirror();
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TENGEN_800008, nes_tengen008_device)
DECLARE_DEVICE_TYPE(NES_TENGEN_800032, nes_tengen032_device)
DECLARE_DEVICE_TYPE(NES_TENGEN_800037, nes_tengen037_device)

#endif // MAME_BUS_NES_TENGEN_H
