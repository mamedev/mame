// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h83337.h

    H8-3337 family emulation

    H8-300-based mcus.

    Variant         ROM        RAM
    H8/3334         32K         1K
    H8/3336         48K         2K
    H8/3337         60K         2K

    The 3394, 3396, and 3397 variants are the mask-rom versions.

***************************************************************************/

#ifndef MAME_CPU_H8_H83337_H
#define MAME_CPU_H8_H83337_H

#pragma once

#include "h8.h"
#include "h8_intc.h"
#include "h8_adc.h"
#include "h8_port.h"
#include "h8_timer8.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class h83337_device : public h8_device {
public:
	h83337_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	h83337_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t start);

	required_device<h8_intc_device> m_intc;
	required_device<h8_adc_device> m_adc;
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
	required_device<h8_timer8_channel_device> m_timer8_1;
	required_device<h8_timer16_device> m_timer16;
	required_device<h8_timer16_channel_device> m_timer16_0;
	required_device<h8_watchdog_device> m_watchdog;

	uint8_t m_syscr;
	uint32_t m_ram_start;

	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual void irq_setup() override;
	virtual void internal_update(uint64_t current_time) override;
	virtual void device_add_mconfig(machine_config &config) override;
	void map(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
};

class h83334_device : public h83337_device {
public:
	h83334_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class h83336_device : public h83337_device {
public:
	h83336_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(H83334, h83334_device)
DECLARE_DEVICE_TYPE(H83336, h83336_device)
DECLARE_DEVICE_TYPE(H83337, h83337_device)

#endif // MAME_CPU_H8_H83337_H
