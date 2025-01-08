// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h83032.h

    H8-3032 family emulation

    H8-300H-based mcus.

    Variant         ROM        RAM
    H8/3030         16K         512b
    H8/3031         32K         1K
    H8/3032         64K         2K


***************************************************************************/

#ifndef MAME_CPU_H8_H83032_H
#define MAME_CPU_H8_H83032_H

#include "h8h.h"
#include "h8_adc.h"
#include "h8_port.h"
#include "h8_intc.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class h83032_device : public h8h_device {
public:
	h83032_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto read_port1()  { return m_read_port [PORT_1].bind(); }
	auto write_port1() { return m_write_port[PORT_1].bind(); }
	auto read_port2()  { return m_read_port [PORT_2].bind(); }
	auto write_port2() { return m_write_port[PORT_2].bind(); }
	auto read_port3()  { return m_read_port [PORT_3].bind(); }
	auto write_port3() { return m_write_port[PORT_3].bind(); }
	auto read_port5()  { return m_read_port [PORT_5].bind(); }
	auto write_port5() { return m_write_port[PORT_5].bind(); }
	auto read_port6()  { return m_read_port [PORT_6].bind(); }
	auto write_port6() { return m_write_port[PORT_6].bind(); }
	auto read_port7()  { return m_read_port [PORT_7].bind(); }
	auto read_port8()  { return m_read_port [PORT_8].bind(); }
	auto write_port8() { return m_write_port[PORT_8].bind(); }
	auto read_port9()  { return m_read_port [PORT_9].bind(); }
	auto write_port9() { return m_write_port[PORT_9].bind(); }
	auto read_porta()  { return m_read_port [PORT_A].bind(); }
	auto write_porta() { return m_write_port[PORT_A].bind(); }
	auto read_portb()  { return m_read_port [PORT_B].bind(); }
	auto write_portb() { return m_write_port[PORT_B].bind(); }

	u8 syscr_r();
	void syscr_w(u8 data);

protected:
	h83032_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 start);

	required_device<h8h_intc_device> m_intc;
	required_device<h8_adc_device> m_adc;
	required_device<h8_port_device> m_port1;
	required_device<h8_port_device> m_port2;
	required_device<h8_port_device> m_port3;
	required_device<h8_port_device> m_port5;
	required_device<h8_port_device> m_port6;
	required_device<h8_port_device> m_port7;
	required_device<h8_port_device> m_port8;
	required_device<h8_port_device> m_port9;
	required_device<h8_port_device> m_porta;
	required_device<h8_port_device> m_portb;
	required_device<h8_port_device> m_portc;
	required_device<h8_timer16_device> m_timer16;
	required_device<h8h_timer16_channel_device> m_timer16_0;
	required_device<h8h_timer16_channel_device> m_timer16_1;
	required_device<h8h_timer16_channel_device> m_timer16_2;
	required_device<h8h_timer16_channel_device> m_timer16_3;
	required_device<h8h_timer16_channel_device> m_timer16_4;
	required_device<h8_watchdog_device> m_watchdog;

	u32 m_ram_start;
	u8 m_syscr;

	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
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

class h83031_device : public h83032_device {
public:
	h83031_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h83030_device : public h83032_device {
public:
	h83030_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(H83032, h83032_device)
DECLARE_DEVICE_TYPE(H83031, h83031_device)
DECLARE_DEVICE_TYPE(H83030, h83030_device)

#endif // MAME_CPU_H8_H83032_H
