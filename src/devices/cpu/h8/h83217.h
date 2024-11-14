// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
/***************************************************************************

    h83217.h

    H8/3217 family emulation

    H8-300-based mcus.

    Variant         ROM         RAM
    H8/3217         60K         2K
    H8/3216         48K         2K
    H8/3214         32K         1K
    H8/3212         16K         512B
    H8/3202         16K         512B

***************************************************************************/

#ifndef MAME_CPU_H8_H83217_H
#define MAME_CPU_H8_H83217_H

#pragma once

#include "h8.h"

#include "h8_intc.h"
#include "h8_port.h"
#include "h8_timer8.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class h83217_device : public h8_device {
public:
	h83217_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// I/O ports
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
	auto write_port7() { return m_write_port[PORT_7].bind(); }

	// MD pins, default mode 3 (single chip)
	void set_mode(u8 mode) { m_md = mode & 3; }

	u8 stcr_r();
	void stcr_w(u8 data);
	u8 syscr_r();
	void syscr_w(u8 data);
	u8 mdcr_r();

protected:
	h83217_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size);

	required_device<h8_intc_device> m_intc;
	required_device_array<h8_port_device, 7> m_port;
	required_device_array<h8_timer8_channel_device, 3> m_timer8;
	required_device<h8_timer16_device> m_timer16;
	required_device<h8325_timer16_channel_device> m_timer16_0;
	required_device<h8_watchdog_device> m_watchdog;

	memory_view m_ram_view;

	u32 m_rom_size;
	u32 m_ram_size;
	u8 m_md;
	u8 m_stcr;
	u8 m_syscr;

	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual void irq_setup() override;
	virtual void internal_update(u64 current_time) override;
	virtual void notify_standby(int state) override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_set_input(int inputnum, int state) override;
};

class h83216_device : public h83217_device {
public:
	h83216_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h83214_device : public h83217_device {
public:
	h83214_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h83212_device : public h83217_device {
public:
	h83212_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h83202_device : public h83217_device {
public:
	h83202_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(H83217, h83217_device)
DECLARE_DEVICE_TYPE(H83216, h83216_device)
DECLARE_DEVICE_TYPE(H83214, h83214_device)
DECLARE_DEVICE_TYPE(H83212, h83212_device)
DECLARE_DEVICE_TYPE(H83202, h83202_device)

#endif // MAME_CPU_H8_H83217_H
