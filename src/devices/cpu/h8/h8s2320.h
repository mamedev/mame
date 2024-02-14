// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2320.h

    H8S-2320 family emulation

    H8S/2600-based mcus.

    Variant           ROM        RAM
    H8S/2320           -          4K
    H8S/2321           -          4K
    H8S/2322           -          8K
    H8S/2323          32K         8K
    H8S/2324           -         32K
    H8S/2326         512K         8K
    H8S/2327         128K         8K
    H8S/2328         256K         8K
    H8S/2329         384K        32K



***************************************************************************/

#ifndef MAME_CPU_H8_H8S2320_H
#define MAME_CPU_H8_H8S2320_H

#pragma once

#include "h8s2000.h"
#include "h8_intc.h"
#include "h8_adc.h"
#include "h8_dma.h"
#include "h8_dtc.h"
#include "h8_port.h"
#include "h8_timer8.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class h8s2320_device : public h8s2000_device {
public:
	h8s2320_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto read_port1()  { return m_read_port [PORT_1].bind(); }
	auto write_port1() { return m_write_port[PORT_1].bind(); }
	auto read_port2()  { return m_read_port [PORT_2].bind(); }
	auto write_port2() { return m_write_port[PORT_2].bind(); }
	auto read_port3()  { return m_read_port [PORT_3].bind(); }
	auto write_port3() { return m_write_port[PORT_3].bind(); }
	auto read_port4()  { return m_read_port [PORT_4].bind(); }
	auto read_port5()  { return m_read_port [PORT_5].bind(); }
	auto write_port5() { return m_write_port[PORT_5].bind(); }
	auto read_port6()  { return m_read_port [PORT_6].bind(); }
	auto write_port6() { return m_write_port[PORT_6].bind(); }
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

	auto tend0_cb() { return m_tend_cb[0].bind(); }
	auto tend1_cb() { return m_tend_cb[1].bind(); }

	u8 syscr_r();
	void syscr_w(u8 data);

protected:
	required_device<h8s_intc_device> m_intc;
	required_device<h8_adc_device> m_adc;
	optional_device<h8s_dma_device> m_dma;
	optional_device<h8s_dma_channel_device> m_dma0;
	optional_device<h8s_dma_channel_device> m_dma1;
	required_device<h8_dtc_device> m_dtc;
	required_device<h8_port_device> m_port1;
	required_device<h8_port_device> m_port2;
	required_device<h8_port_device> m_port3;
	required_device<h8_port_device> m_port4;
	required_device<h8_port_device> m_port5;
	required_device<h8_port_device> m_port6;
	required_device<h8_port_device> m_porta;
	required_device<h8_port_device> m_portb;
	required_device<h8_port_device> m_portc;
	required_device<h8_port_device> m_portd;
	required_device<h8_port_device> m_porte;
	required_device<h8_port_device> m_portf;
	required_device<h8_port_device> m_portg;
	required_device<h8h_timer8_channel_device> m_timer8_0;
	required_device<h8h_timer8_channel_device> m_timer8_1;
	required_device<h8_timer16_device> m_timer16;
	required_device<h8s_timer16_channel_device> m_timer16_0;
	required_device<h8s_timer16_channel_device> m_timer16_1;
	required_device<h8s_timer16_channel_device> m_timer16_2;
	required_device<h8s_timer16_channel_device> m_timer16_3;
	required_device<h8s_timer16_channel_device> m_timer16_4;
	required_device<h8s_timer16_channel_device> m_timer16_5;
	required_device<h8_watchdog_device> m_watchdog;

	devcb_write_line::array<2> m_tend_cb;

	u32 m_ram_start;
	u8 m_syscr;

	h8s2320_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 start);

	virtual bool exr_in_stack() const override;
	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual int trace_setup() override;
	virtual int trapa_setup() override;
	virtual void irq_setup() override;
	virtual void internal_update(u64 current_time) override;
	virtual void device_add_mconfig(machine_config &config) override;
	void map(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
};

class h8s2321_device : public h8s2320_device {
public:
	h8s2321_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2322_device : public h8s2320_device {
public:
	h8s2322_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2323_device : public h8s2320_device {
public:
	h8s2323_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2324_device : public h8s2320_device {
public:
	h8s2324_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2326_device : public h8s2320_device {
public:
	h8s2326_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2327_device : public h8s2320_device {
public:
	h8s2327_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2328_device : public h8s2320_device {
public:
	h8s2328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2329_device : public h8s2320_device {
public:
	h8s2329_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(H8S2320, h8s2320_device)
DECLARE_DEVICE_TYPE(H8S2321, h8s2321_device)
DECLARE_DEVICE_TYPE(H8S2322, h8s2322_device)
DECLARE_DEVICE_TYPE(H8S2323, h8s2323_device)
DECLARE_DEVICE_TYPE(H8S2324, h8s2324_device)
DECLARE_DEVICE_TYPE(H8S2326, h8s2326_device)
DECLARE_DEVICE_TYPE(H8S2327, h8s2327_device)
DECLARE_DEVICE_TYPE(H8S2328, h8s2328_device)
DECLARE_DEVICE_TYPE(H8S2329, h8s2329_device)

#endif // MAME_CPU_H8_H8S2320_H
