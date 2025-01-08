// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
/***************************************************************************

    h8s2319.h

    H8S-2319 family emulation

    H8S/2000-based mcus.

    Variant         ROM         RAM
    H8S/2310         -           2K
    H8S/2311        32K          2K
    H8S/2312         -           8K
    H8S/2313        64K          2K
    H8S/2315       384K          8K
    H8S/2316        64K          8K
    H8S/2317       128K          8K
    H8S/2318       256K          8K
    H8S/2319       512K          8K

***************************************************************************/

#ifndef MAME_CPU_H8_H8S2319_H
#define MAME_CPU_H8_H8S2319_H

#pragma once

#include "h8s2000.h"
#include "h8_intc.h"
#include "h8_adc.h"
#include "h8_dtc.h"
#include "h8_port.h"
#include "h8_timer8.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class h8s2319_device : public h8s2000_device {
public:
	h8s2319_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// I/O ports
	auto read_port1()  { return m_read_port [PORT_1].bind(); }
	auto write_port1() { return m_write_port[PORT_1].bind(); }
	auto read_port2()  { return m_read_port [PORT_2].bind(); }
	auto write_port2() { return m_write_port[PORT_2].bind(); }
	auto read_port3()  { return m_read_port [PORT_3].bind(); }
	auto write_port3() { return m_write_port[PORT_3].bind(); }
	auto read_port4()  { return m_read_port [PORT_4].bind(); }

	auto read_porta()  { return m_read_port [PORT_A].bind(); }
	auto write_porta() { return m_write_port[PORT_A].bind(); }
	auto read_portb()  { return m_read_port [PORT_B].bind(); }
	auto write_portb() { return m_write_port[PORT_B].bind(); }
	auto read_portc()  { return m_read_port [PORT_C].bind(); }
	auto write_portc() { return m_write_port[PORT_C].bind(); }
	auto read_portd()  { return m_read_port [PORT_D].bind(); }
	auto write_portd() { return m_write_port[PORT_D].bind(); }
	auto read_porte()  { return m_read_port [PORT_E].bind(); }
	auto write_porte() { return m_write_port[PORT_E].bind(); }
	auto read_portf()  { return m_read_port [PORT_F].bind(); }
	auto write_portf() { return m_write_port[PORT_F].bind(); }
	auto read_portg()  { return m_read_port [PORT_G].bind(); }
	auto write_portg() { return m_write_port[PORT_G].bind(); }

	// MD pins, default mode 7 (single chip), or mode 4 (ROMless)
	void set_mode(u8 mode) { m_md = mode & 7; }

	u8 sbycr_r();
	void sbycr_w(u8 data);
	u8 syscr_r();
	void syscr_w(u8 data);
	u8 mdcr_r();

protected:
	required_device<h8s_intc_device> m_intc;
	required_device<h8_adc_device> m_adc;
	required_device<h8_dtc_device> m_dtc;
	required_device_array<h8_port_device, 4> m_portn;
	required_device_array<h8_port_device, 7> m_porta;
	required_device_array<h8h_timer8_channel_device, 2> m_timer8;
	required_device<h8_timer16_device> m_timer16;
	required_device_array<h8s_timer16_channel_device, 6> m_timer16c;
	required_device<h8_watchdog_device> m_watchdog;

	memory_view m_ram_view;

	u32 m_rom_size;
	u32 m_ram_size;
	u8 m_md;
	u8 m_sbycr;
	u8 m_syscr;

	h8s2319_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map_delegate, u32 rom_size, u32 ram_size);
	h8s2319_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size);

	virtual bool exr_in_stack() const override;
	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual int trace_setup() override;
	virtual int trapa_setup() override;
	virtual void irq_setup() override;
	virtual void internal_update(u64 current_time) override;
	virtual void notify_standby(int state) override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_set_input(int inputnum, int state) override;
};

class h8s2310_device : public h8s2319_device {
public:
	h8s2310_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2311_device : public h8s2319_device {
public:
	h8s2311_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2312_device : public h8s2319_device {
public:
	h8s2312_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2313_device : public h8s2319_device {
public:
	h8s2313_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2315_device : public h8s2319_device {
public:
	h8s2315_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2316_device : public h8s2319_device {
public:
	h8s2316_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2317_device : public h8s2319_device {
public:
	h8s2317_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2318_device : public h8s2319_device {
public:
	h8s2318_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(H8S2310, h8s2310_device)
DECLARE_DEVICE_TYPE(H8S2311, h8s2311_device)
DECLARE_DEVICE_TYPE(H8S2312, h8s2312_device)
DECLARE_DEVICE_TYPE(H8S2313, h8s2313_device)
DECLARE_DEVICE_TYPE(H8S2315, h8s2315_device)
DECLARE_DEVICE_TYPE(H8S2316, h8s2316_device)
DECLARE_DEVICE_TYPE(H8S2317, h8s2317_device)
DECLARE_DEVICE_TYPE(H8S2318, h8s2318_device)
DECLARE_DEVICE_TYPE(H8S2319, h8s2319_device)

#endif // MAME_CPU_H8_H8S2319_H
