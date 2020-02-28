// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_CONY_H
#define MAME_BUS_NES_CONY_H

#pragma once

#include "nxrom.h"


// ======================> nes_cony_device

class nes_cony_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cony_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	static constexpr device_timer_id TIMER_IRQ = 0;

	nes_cony_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void set_prg();
	virtual void set_chr();

	uint16_t m_irq_count;
	int m_irq_enable;

	emu_timer *irq_timer;

	uint8_t m_latch1, m_latch2;
	uint8_t m_low_reg[4];
	uint8_t m_reg[10];
	uint8_t m_extra1;
};


// ======================> nes_yoko_device

class nes_yoko_device : public nes_cony_device
{
public:
	// construction/destruction
	nes_yoko_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	virtual void set_prg() override;
	virtual void set_chr() override;

	uint8_t m_extra2;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_CONY, nes_cony_device)
DECLARE_DEVICE_TYPE(NES_YOKO, nes_yoko_device)

#endif // MAME_BUS_NES_CONY_H
