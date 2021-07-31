// license:BSD-3-Clause
// copyright-holders: kmg, Fabio Priuli
#ifndef MAME_BUS_NES_BITCORP_H
#define MAME_BUS_NES_BITCORP_H

#pragma once

#include "nxrom.h"


// ======================> nes_bc3150_device

class nes_bc3150_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bc3150_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// DIP switch handlers
	DECLARE_INPUT_CHANGED_MEMBER(dsw_changed);

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;

	required_ioport m_dsw;

private:
	void update_banks();
};


// ======================> nes_bc4602_device

class nes_bc4602_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bc4602_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// DIP switch handlers
	DECLARE_INPUT_CHANGED_MEMBER(dsw_changed);

	virtual u8 read_l(offs_t offset) override;
	virtual u8 read_m(offs_t offset) override;
	virtual void write_ex(offs_t offset, u8 data) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_ioport m_dsw;

private:
	void update_banks();
	void write_45(offs_t offset, u8 data);
	u8 m_dipsetting;
	u8 m_reg[3];
	u16 m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_BC3150, nes_bc3150_device)
DECLARE_DEVICE_TYPE(NES_BC4602, nes_bc4602_device)

#endif // MAME_BUS_NES_BITCORP_H
