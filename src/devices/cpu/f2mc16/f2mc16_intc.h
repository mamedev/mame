// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series INTC

***************************************************************************/

#ifndef MAME_CPU_F2MC16_F2MC16_INTC_H
#define MAME_CPU_F2MC16_F2MC16_INTC_H

#pragma once

#include "f2mc16.h"

class f2mc16_intc_device :
	public device_t
{
public:
	f2mc16_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template<unsigned N> auto i2osclr() { return m_i2osclr_cb[N - m_external_interrupt_vector].bind(); }

	uint8_t icr_r(offs_t offset);
	void icr_w(offs_t offset, uint8_t data);
	uint8_t enir_r();
	void enir_w(uint8_t data);
	uint8_t eirr_r();
	void eirr_w(uint8_t data);
	uint8_t elvr_r(offs_t offset);
	void elvr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t dirr_r();
	void dirr_w(uint8_t data);

	void set_irq(uint8_t vector, int state);
	void set_completion_request(uint8_t vector, int state);
	IRQ_CALLBACK_MEMBER(irq_acknowledge_callback);

protected:
	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update();

	f2mc16_device *m_cpu;
	devcb_write_line::array<0x20> m_i2osclr_cb;
	static constexpr uint8_t m_external_interrupt_vector = 0x0b;
	uint8_t m_digm_vector;

	uint8_t m_icr[0x10];
	uint8_t m_ics[0x10];
	uint8_t m_dirr;
	uint8_t m_enir;
	uint8_t m_eirr;
	uint16_t m_elvr;
	uint32_t m_irq_state;
	uint32_t m_completion_request_state;
	uint8_t m_active_irq;
};

DECLARE_DEVICE_TYPE(F2MC16_INTC, f2mc16_intc_device)

#endif
