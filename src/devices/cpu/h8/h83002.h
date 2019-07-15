// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h83002.h

    H8/3002

    H8/300H-based mcus.


***************************************************************************/

#ifndef MAME_CPU_H8_H83002_H
#define MAME_CPU_H8_H83002_H

#pragma once

#include "h8h.h"
#include "h8_adc.h"
#include "h8_dma.h"
#include "h8_port.h"
#include "h8_intc.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class h83002_device : public h8h_device {
public:
	h83002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto tend0() { return tend0_cb.bind(); }
	auto tend1() { return tend1_cb.bind(); }

	DECLARE_READ8_MEMBER(syscr_r);
	DECLARE_WRITE8_MEMBER(syscr_w);

	DECLARE_READ8_MEMBER(rtmcsr_r);
	DECLARE_WRITE8_MEMBER(rtmcsr_w);

protected:
	required_device<h8h_intc_device> intc;
	required_device<h8_adc_device> adc;
	optional_device<h8_dma_device> dma;
	optional_device<h8_dma_channel_device> dma0;
	optional_device<h8_dma_channel_device> dma1;
	required_device<h8_port_device> port4;
	required_device<h8_port_device> port6;
	required_device<h8_port_device> port7;
	required_device<h8_port_device> port8;
	required_device<h8_port_device> port9;
	required_device<h8_port_device> porta;
	required_device<h8_port_device> portb;
	required_device<h8_timer16_device> timer16;
	required_device<h8h_timer16_channel_device> timer16_0;
	required_device<h8h_timer16_channel_device> timer16_1;
	required_device<h8h_timer16_channel_device> timer16_2;
	required_device<h8h_timer16_channel_device> timer16_3;
	required_device<h8h_timer16_channel_device> timer16_4;
	required_device<h8_sci_device> sci0;
	required_device<h8_sci_device> sci1;
	required_device<h8_watchdog_device> watchdog;

	uint8_t syscr;
	uint8_t rtmcsr;

	devcb_write_line tend0_cb, tend1_cb;

	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual int trapa_setup() override;
	virtual void irq_setup() override;
	virtual void internal_update(uint64_t current_time) override;
	virtual void device_add_mconfig(machine_config &config) override;
	void map(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
};

DECLARE_DEVICE_TYPE(H83002, h83002_device)

#endif // MAME_CPU_H8_H83002_H
