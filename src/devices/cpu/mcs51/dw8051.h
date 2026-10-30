// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_CPU_MCS51_DW8051_H
#define MAME_CPU_MCS51_DW8051_H

#pragma once

#include "i8052.h"

// Four-clock DW8051 core configuration used by the Nordic nRF24E1.
class dw8051_device : public i8052_device
{
public:
	dw8051_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	dw8051_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 3) / 4; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return cycles * 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 13; }

	// mcs51_cpu_device implementation
	virtual void sfr_map(address_map &map) override ATTR_COLD;
	virtual bool instruction_enabled() const override { return !(m_pcon & 3); }
	virtual void prepare_instruction(u8 op) override;
	virtual void update_peripherals() override;
	virtual u8 interrupt_mask() override;
	virtual void interrupt_taken() override;
	virtual bool timer1_split_uses_input() const override { return true; }
	virtual void update_timer_t2(int cycles) override;
	virtual void handle_irq(int irqline, int state, u32 new_state, u32 tr_state) override;

	u8 m_irq_delay;

private:
	static const u8 INSTRUCTION_CYCLES[256];
	u8 pointer_r(offs_t offset);
	void pointer_w(offs_t offset, u8 data);
	void pointer_select_w(u8 data);
	void interrupt_enable_w(u8 data);
	void interrupt_priority_w(u8 data);

	u16 m_other_dptr;
	u8 m_dps;
	u8 m_ckcon;
	u8 m_timer_divider;
};

DECLARE_DEVICE_TYPE(DW8051, dw8051_device)

#endif // MAME_CPU_MCS51_DW8051_H
