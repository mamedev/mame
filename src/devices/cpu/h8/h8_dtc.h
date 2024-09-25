// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_dtc.h

    H8 Data Transfer Controller

***************************************************************************/

#ifndef MAME_CPU_H8_H8_DTC_H
#define MAME_CPU_H8_H8_DTC_H

#pragma once

#include "h8_intc.h"

#include <list>

struct h8_dtc_state {
	u32 m_base, m_sra, m_dar, m_cr;
	s32 m_incs, m_incd;
	u32 m_count;
	int m_id;
	int m_next;
};

class h8_dtc_device : public device_t {
public:
	enum { DTC_CHAINED = 1000 };

	h8_dtc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template<typename T, typename U> h8_dtc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, U &&intc, int irq)
		: h8_dtc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_irq = irq;
	}

	u8 dtcer_r(offs_t offset);
	void dtcer_w(offs_t offset, u8 data);
	u8 dtvecr_r();
	void dtvecr_w(u8 data);

	bool trigger_dtc(int vector);
	void count_done(int id);

	inline h8_dtc_state *get_object(int vector) { return m_states + vector; }
	inline u32 get_vector_address(int vector) { return 0x400 | ((vector ? vector : m_dtvecr & 0x7f) << 1); }
	int get_waiting_vector();
	int get_waiting_writeback();
	void vector_done(int vector);
	void writeback_done(int vector);

protected:
	static const int vector_to_enable[];
	required_device<h8_device> m_cpu;
	required_device<h8_intc_device> m_intc;
	const char *m_intc_tag;
	int m_irq;
	h8_dtc_state m_states[92];

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u8 m_dtcer[6], m_dtvecr;
	int m_cur_active_vector;

	std::vector<int> m_waiting_vector, m_waiting_writeback;

	void edge(int vector);
	void queue(int vector);
};

DECLARE_DEVICE_TYPE(H8_DTC, h8_dtc_device)

#endif // MAME_CPU_H8_H8_DTC_H
