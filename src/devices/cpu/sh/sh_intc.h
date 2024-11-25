// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_intc.h

    SH interrupt controllers family

***************************************************************************/

#ifndef MAME_CPU_SH_SH_INTC_H
#define MAME_CPU_SH_SH_INTC_H

#pragma once

class sh7042_device;

class sh_intc_device : public device_t {
public:
	sh_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template<typename T> sh_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu) :
		sh_intc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	void interrupt_taken(int irqline, int vector);
	void internal_interrupt(int vector);
	void set_input(int inputnum, int state);

	u16 icr_r();
	void icr_w(offs_t, u16 data, u16 mem_mask);
	u16 isr_r();
	void isr_w(offs_t, u16 data, u16 mem_mask);
	u16 ipr_r(offs_t offset);
	void ipr_w(offs_t offset, u16 data, u16 mem_mask);

protected:
	static const u8 pribit[256];

	std::array<u32, 8> m_pending;
	std::array<u16, 8> m_ipr;

	u16 m_isr, m_icr;

	u8 m_lines;

	required_device<sh7042_device> m_cpu;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_irq();
};

DECLARE_DEVICE_TYPE(SH_INTC, sh_intc_device)

#endif // MAME_CPU_SH_SH_INTC_H
