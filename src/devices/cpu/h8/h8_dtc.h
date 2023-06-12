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
	uint32_t m_base, m_sra, m_dar, m_cr;
	int32_t m_incs, m_incd;
	uint32_t m_count;
	int m_id;
	int m_next;
};

class h8_dtc_device : public device_t {
public:
	enum { DTC_CHAINED = 1000 };

	h8_dtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	template<typename T, typename U> h8_dtc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, U &&intc, int irq)
		: h8_dtc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_irq = irq;
	}

	uint8_t dtcer_r(offs_t offset);
	void dtcer_w(offs_t offset, uint8_t data);
	uint8_t dtvecr_r();
	void dtvecr_w(uint8_t data);

	bool trigger_dtc(int vector);
	void count_done(int id);

	inline h8_dtc_state *get_object(int vector) { return m_states + vector; }
	inline uint32_t get_vector_address(int vector) { return 0x400 | ((vector ? vector : m_dtvecr & 0x7f) << 1); }
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

	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t m_dtcer[6], m_dtvecr;
	int m_cur_active_vector;

	std::vector<int> m_waiting_vector, m_waiting_writeback;

	void edge(int vector);
	void queue(int vector);
};

DECLARE_DEVICE_TYPE(H8_DTC, h8_dtc_device)

#endif // MAME_CPU_H8_H8_DTC_H
