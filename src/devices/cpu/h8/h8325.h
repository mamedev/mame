// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
/***************************************************************************

    h8325.h

    H8/325 family emulation

    H8-300-based mcus.

    Variant         ROM         RAM
    H8/3257         60K         2K
    H8/3256         48K         2K
    H8/325          32K         1K
    H8/324          24K         1K
    H8/323          16K         512B
    H8/322          8K          256B

***************************************************************************/

#ifndef MAME_CPU_H8_H8325_H
#define MAME_CPU_H8_H8325_H

#pragma once

#include "h8.h"

#include "h8_intc.h"
#include "h8_port.h"
#include "h8_timer8.h"
#include "h8_timer16.h"
#include "h8_sci.h"

class h8325_device : public h8_device {
public:
	h8325_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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

	u8 syscr_r();
	void syscr_w(u8 data);
	u8 mdcr_r();

protected:
	h8325_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size);

	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 2); }

	required_device<h8325_intc_device> m_intc;
	required_device_array<h8_port_device, 7> m_port;
	required_device_array<h8_timer8_channel_device, 2> m_timer8;
	required_device<h8_timer16_device> m_timer16;
	required_device<h8325_timer16_channel_device> m_timer16_0;

	memory_view m_ram_view;

	u32 m_rom_size;
	u32 m_ram_size;
	u8 m_md;
	u8 m_mds;
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

class h83256_device : public h8325_device {
public:
	h83256_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h83257_device : public h8325_device {
public:
	h83257_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8324_device : public h8325_device {
public:
	h8324_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8323_device : public h8325_device {
public:
	h8323_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8322_device : public h8325_device {
public:
	h8322_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(H83257, h83257_device)
DECLARE_DEVICE_TYPE(H83256, h83256_device)
DECLARE_DEVICE_TYPE(H8325, h8325_device)
DECLARE_DEVICE_TYPE(H8324, h8324_device)
DECLARE_DEVICE_TYPE(H8323, h8323_device)
DECLARE_DEVICE_TYPE(H8322, h8322_device)

#endif // MAME_CPU_H8_H8325_H
