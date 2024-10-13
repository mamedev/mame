// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_mtu.h

    SH Multifunction timer pulse unit

***************************************************************************/

#ifndef MAME_CPU_SH_SH_MTU_H
#define MAME_CPU_SH_SH_MTU_H

#pragma once

// To generalize eventually
class sh7042_device;
class sh_intc_device;

class sh_mtu_channel_device : public device_t {
public:
	enum {
		CHAIN,
		INPUT_A,
		INPUT_B,
		INPUT_C,
		INPUT_D,
		DIV_1,
		DIV_2,
		DIV_4,
		DIV_8,
		DIV_16,
		DIV_32,
		DIV_64,
		DIV_128,
		DIV_256,
		DIV_512,
		DIV_1024,
		DIV_2048,
		DIV_4096
	};

	enum {
		TGR_CLEAR_NONE = -1,
		TGR_CLEAR_EXT  = -2
	};

	enum {
		IRQ_A = 0x01,
		IRQ_B = 0x02,
		IRQ_C = 0x04,
		IRQ_D = 0x08,
		IRQ_V = 0x10,
		IRQ_U = 0x20,
		IRQ_E = 0x80
	};

	sh_mtu_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T, typename U> sh_mtu_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, int tgr_count, int tier_mask, U &&intc, int irq_base,
															int t0, int t1, int t2, int t3, int t4, int t5, int t6, int t7)
		: sh_mtu_channel_device(mconfig, tag, owner)
	{
		set_info(cpu, intc);
		m_tgr_count = tgr_count;
		m_tbr_count = 0;
		m_tier_mask = tier_mask;

		m_interrupt[0] = irq_base;
		m_interrupt[1] = irq_base + 1;
		m_interrupt[2] = tier_mask & 0x04 ? -1 : irq_base + 2;
		m_interrupt[3] = tier_mask & 0x08 ? -1 : irq_base + 3;
		m_interrupt[4] = irq_base + 4;
		m_interrupt[5] = tier_mask & 0x20 ? -1 : irq_base + 5;

		m_count_types[0] = t0;
		m_count_types[1] = t1;
		m_count_types[2] = t2;
		m_count_types[3] = t3;
		m_count_types[4] = t4;
		m_count_types[5] = t5;
		m_count_types[6] = t6;
		m_count_types[7] = t7;
	}

	template<typename T, typename U> void set_info(T &&cpu, U &&intc) { m_cpu.set_tag(std::forward<T>(cpu)); m_intc.set_tag(std::forward<U>(intc)); }
	template<typename T> void set_chain(T &&chain) { m_chained_timer.set_tag(std::forward<T>(chain)); }

	u8 tcr_r();
	void tcr_w(u8 data);
	u8 tmdr_r();
	void tmdr_w(u8 data);
	u8 tior_r();
	void tior_w(u8 data);
	u8 tier_r();
	void tier_w(u8 data);
	u8 tsr_r();
	void tsr_w(u8 data);
	u16 tcnt_r();
	void tcnt_w(offs_t, u16 data, u16 mem_mask);
	u16 tgr_r(offs_t reg);
	void tgr_w(offs_t reg, u16 data, u16 mem_mask);
	u16 tgrc_r(offs_t reg);
	void tgrc_w(offs_t reg, u16 data, u16 mem_mask);

	void set_enable(bool enable);
	u64 internal_update(u64 current_time);

protected:
	required_device<sh7042_device> m_cpu;
	required_device<sh_intc_device> m_intc;
	optional_device<sh_mtu_channel_device> m_chained_timer;
	int m_interrupt[6];
	u8 m_tier_mask;

	int m_tgr_count, m_tbr_count;
	int m_tgr_clearing;
	u8 m_tcr, m_tmdr, m_tior, m_tier, m_tsr;
	int m_clock_type, m_clock_divider;
	u16 m_tcnt;
	std::array<u16, 4> m_tgr;
	u64 m_last_clock_update, m_event_time;
	u32 m_phase, m_counter_cycle;
	bool m_counter_incrementing;
	bool m_channel_active;
	std::array<int, 8> m_count_types;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	void update_counter(u64 cur_time = 0);
	void recalc_event(u64 cur_time = 0);
};

class sh_mtu_device : public device_t {
public:
	sh_mtu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> sh_mtu_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, int timer_count)
		: sh_mtu_device(mconfig, tag, owner)
	{
		set_info(cpu);
		m_timer_count = timer_count;
	}

	template<typename T> void set_info(T &&cpu) { m_cpu.set_tag(std::forward<T>(cpu)); }

	u8 tstr_r();
	void tstr_w(u8 data);
	u8 tsyr_r();
	void tsyr_w(u8 data);
	u8 toer_r();
	void toer_w(u8 data);
	u8 tocr_r();
	void tocr_w(u8 data);
	u8 tgcr_r();
	void tgcr_w(u8 data);
	u16 tcdr_r();
	void tcdr_w(offs_t, u16 data, u16 mem_mask);
	u16 tddr_r();
	void tddr_w(offs_t, u16 data, u16 mem_mask);
	u16 tcnts_r();
	void tcnts_w(offs_t, u16 data, u16 mem_mask);
	u16 tcbr_r();
	void tcbr_w(offs_t, u16 data, u16 mem_mask);

protected:
	required_device<sh7042_device> m_cpu;
	required_device_array<sh_mtu_channel_device, 5> m_timer_channel;

	int m_timer_count;

	u8 m_tstr, m_tsyr, m_toer, m_tocr, m_tgcr;
	u16 m_tcdr, m_tddr, m_tcnts, m_tcbr;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SH_MTU, sh_mtu_device)
DECLARE_DEVICE_TYPE(SH_MTU_CHANNEL, sh_mtu_channel_device)

#endif
