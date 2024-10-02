// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_cmt.h

    SH Compare/Match timer subsystem


***************************************************************************/

#ifndef MAME_CPU_SH_SH_CMT_H
#define MAME_CPU_SH_SH_CMT_H

#pragma once

class sh7042_device;
class sh_intc_device;

class sh_cmt_device : public device_t {
public:
	sh_cmt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template<typename T, typename U> sh_cmt_device(const machine_config &mconfig, const char *tag, device_t *owner,
												   T &&cpu, U &&intc, int vect0, int vect1) :
		sh_cmt_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_intc_vector[0] = vect0;
		m_intc_vector[1] = vect1;
	}

	u64 internal_update(u64 current_time);

	u16 cmstr_r();
	void cmstr_w(offs_t, u16 data, u16 mem_mask);
	u16 cmcsr0_r();
	void cmcsr0_w(offs_t, u16 data, u16 mem_mask);
	u16 cmcnt0_r();
	void cmcnt0_w(offs_t, u16 data, u16 mem_mask);
	u16 cmcor0_r();
	void cmcor0_w(offs_t, u16 data, u16 mem_mask);
	u16 cmcsr1_r();
	void cmcsr1_w(offs_t, u16 data, u16 mem_mask);
	u16 cmcnt1_r();
	void cmcnt1_w(offs_t, u16 data, u16 mem_mask);
	u16 cmcor1_r();
	void cmcor1_w(offs_t, u16 data, u16 mem_mask);

protected:
	required_device<sh7042_device> m_cpu;
	required_device<sh_intc_device> m_intc;
	std::array<u64, 2> m_next_event;
	std::array<int, 2> m_intc_vector;
	u16 m_str;
	std::array<u16, 2> m_csr;
	std::array<u16, 2> m_cnt;
	std::array<u16, 2> m_cor;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void csr_w(int reg, u16 data, u16 mem_mask);
	void cnt_w(int reg, u16 data, u16 mem_mask);
	void cor_w(int reg, u16 data, u16 mem_mask);

	void clock_start(int clk);
	void compute_next_event(int clk);
	void cnt_update(int clk, u64 current_time);
};

DECLARE_DEVICE_TYPE(SH_CMT, sh_cmt_device)

#endif // MAME_CPU_SH_SH_CMT_H
