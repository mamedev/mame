// license:BSD-3-Clause
// copyright-holders:smf, Olivier Galibert
/***************************************************************************

    Namco C77

    Custom H8 used on Cyberlead cabinet I/O boards

***************************************************************************/

#ifndef MAME_CPU_H8_C77_H
#define MAME_CPU_H8_C77_H

#pragma once

#include "h8.h"
#include "h8_intc.h"
//#include "h8_adc.h"
#include "h8_port.h"
#include "h8_timer8.h"
#include "h8_sci.h"

class namco_c77_device :
	public h8_device
{
public:
	namco_c77_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_port1()  { return m_read_port [PORT_1].bind(); }
	auto write_port1() { return m_write_port[PORT_1].bind(); }
	auto read_port2()  { return m_read_port [PORT_2].bind(); }
	auto write_port2() { return m_write_port[PORT_2].bind(); }
	auto read_port3()  { return m_read_port [PORT_3].bind(); }
	auto write_port3() { return m_write_port[PORT_3].bind(); }
	auto read_port4()  { return m_read_port [PORT_4].bind(); }
	auto write_port4() { return m_write_port[PORT_4].bind(); }
	auto read_port5()  { return m_read_port [PORT_5].bind(); }
	auto write_port5() { return m_write_port[PORT_5].bind(); }
	auto read_port6()  { return m_read_port [PORT_6].bind(); }
	auto write_port6() { return m_write_port[PORT_6].bind(); }
	auto read_port7()  { return m_read_port [PORT_7].bind(); }
	auto read_port8()  { return m_read_port [PORT_8].bind(); }
	auto write_port8() { return m_write_port[PORT_8].bind(); }
	auto read_port9()  { return m_read_port [PORT_9].bind(); }
	auto write_port9() { return m_write_port[PORT_9].bind(); }

	uint8_t wscr_r();
	void wscr_w(uint8_t data);
	uint8_t stcr_r();
	void stcr_w(uint8_t data);
	uint8_t syscr_r();
	void syscr_w(uint8_t data);
	uint8_t mdcr_r();
	void mdcr_w(uint8_t data);

protected:
	namco_c77_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t start);

	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override;

	// device_execute_interface
	virtual void execute_set_input(int inputnum, int state) override;

	// h8_device
	virtual void irq_setup() override;
	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual void internal_update(uint64_t current_time) override;
	virtual void notify_standby(int state) override;

	void map(address_map &map) ATTR_COLD;

	required_device<h8_intc_device> m_intc;
	required_device<h8_port_device> m_port1;
	required_device<h8_port_device> m_port2;
	required_device<h8_port_device> m_port3;
	required_device<h8_port_device> m_port4;
	required_device<h8_port_device> m_port5;
	required_device<h8_port_device> m_port6;
	required_device<h8_port_device> m_port7;
	required_device<h8_port_device> m_port8;
	required_device<h8_port_device> m_port9;
	required_device<h8_timer8_channel_device> m_timer8_0;
	//required_device<h8_timer8_channel_device> m_timer8_1;

	uint8_t m_syscr;
	uint32_t m_ram_start;
};

DECLARE_DEVICE_TYPE(NAMCO_C77, namco_c77_device)

#endif // MAME_CPU_H8_C77_H
