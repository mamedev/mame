// license:BSD-3-Clause
// copyright-holders:R. Belmont, AJR

#ifndef MAME_CPU_H8500_H8510_H
#define MAME_CPU_H8500_H8510_H

#pragma once

#include "h8500.h"
#include "h8500_frt.h"
#include "h8500_intc.h"
#include "cpu/h8/h8_port.h"
#include "cpu/h8/h8_adc.h"
#include "cpu/h8/h8_dtc.h"
#include "cpu/h8/h8_dma.h"
#include "cpu/h8/h8_sci.h"
#include "cpu/h8/h8_timer8.h"
#include "cpu/h8/h8_watchdog.h"

class h8510_device : public h8500_device
{
public:
	auto read_port1() { return m_read_port[PORT_1].bind(); }
	auto write_port1() { return m_write_port[PORT_1].bind(); }
	auto read_port2() { return m_read_port[PORT_2].bind(); }
	auto write_port2() { return m_write_port[PORT_2].bind(); }
	auto read_port3() { return m_read_port[PORT_3].bind(); }
	auto write_port3() { return m_write_port[PORT_3].bind(); }
	auto read_port4() { return m_read_port[PORT_4].bind(); }
	auto write_port4() { return m_write_port[PORT_4].bind(); }
	auto read_port5() { return m_read_port[PORT_5].bind(); }
	auto write_port5() { return m_write_port[PORT_5].bind(); }
	auto read_port6() { return m_read_port[PORT_6].bind(); }
	auto write_port6() { return m_write_port[PORT_6].bind(); }
	auto read_port7() { return m_read_port[PORT_7].bind(); }	// port 7 is read-only on the H8/510
	auto read_port8() { return m_read_port[PORT_8].bind(); }
	auto write_port8() { return m_write_port[PORT_8].bind(); }

protected:
	h8510_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void execute_set_input(int inputnum, int state) override;

	virtual void internal_update(u64 current_time) override;
	virtual void interrupt_taken() override;
	using h8500_device::internal_update;
	virtual void notify_standby(int state) override;
	virtual void update_irq_filter() override;
	virtual void irq_setup() override;

	required_device<h8500_intc_device> m_intc;
	required_device<h8_adc_device> m_adc;
	required_device<h8_timer16_device> m_frt;
	required_device<h8500_frt_device> m_frt1;
	required_device<h8500_frt_device> m_frt2;
	required_device<h8_timer8_channel_device> m_tmr;
	required_device<h8_port_device> m_port1;
	required_device<h8_port_device> m_port2;
	required_device<h8_port_device> m_port3;
	required_device<h8_port_device> m_port4;
	required_device<h8_port_device> m_port5;
	required_device<h8_port_device> m_port6;
	required_device<h8_port_device> m_port7;
	required_device<h8_port_device> m_port8;
	required_device<h8_watchdog_device> m_watchdog;

private:
	void internal_map(address_map &map) ATTR_COLD;
};

class hd6415108_device : public h8510_device
{
public:
	// device type constructor
	hd6415108_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(HD6415108, hd6415108_device)

#endif // MAME_CPU_H8500_H8510_H
