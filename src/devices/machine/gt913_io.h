// license:BSD-3-Clause
// copyright-holders: Devin Acker
/***************************************************************************
    Casio GT913 I/O (HLE)
***************************************************************************/

#ifndef MAME_MACHINE_GT913_IO_H
#define MAME_MACHINE_GT913_IO_H

#pragma once

#include "cpu/h8/h8_intc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gt913_kbd_hle_device

class gt913_io_hle_device : public device_t
{
public:
	// construction/destruction
	gt913_io_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	gt913_io_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc, int t0irq, int t1irq)
		: gt913_io_hle_device(mconfig, tag, owner, 0)
	{
		m_intc_tag = intc;
		m_timer_irq[0] = t0irq;
		m_timer_irq[1] = t1irq;
	}

	void timer_control_w(offs_t offset, uint8_t data);
	uint8_t timer_control_r(offs_t offset);
	void timer_rate0_w(uint16_t data);
	void timer_rate1_w(uint8_t data);

	void adc_control_w(uint8_t data);
	uint8_t adc_control_r();
	uint8_t adc_data_r();

protected:
	void timer_adjust(offs_t num);
	void timer_check_irq(offs_t num);

	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_device<cpu_device> m_cpu;
	address_space *m_cpu_io;
	h8_intc_device *m_intc;
	const char *m_intc_tag;

	/* timers */
	uint8_t m_timer_control[2];
	uint16_t m_timer_rate[2];
	int m_timer_irq[2];
	bool m_timer_irq_pending[2];
	emu_timer *m_timer[2];

	/* 2x ADC */
	bool m_adc_enable, m_adc_channel;
	uint8_t m_adc_data[2];

};

// device type definition
DECLARE_DEVICE_TYPE(GT913_IO_HLE, gt913_io_hle_device)

#endif // MAME_MACHINE_GT913_IO_H
