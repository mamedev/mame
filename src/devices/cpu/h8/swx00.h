// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sxw00.h

    Yamaha sound generator including a h8s2000 core, an AWM2 sound generator,
    a dsp and a bunch of more standard h8 peripherals.

    It has two buses, "s" and "c".  What is on which depends on the mode bits.
    Bit 0 is called "Dual" and bit1 "separate".  It's not sure yet but it may
    be that the h8 core can be disabled, or an internal "pass the data to the
    peripherals" internal program can be activated.

    Mode  H8   S                          C                          Example
    0     on   program+wave roms          effects ram                MU15
    0     on   program+wave roms          effects ram                QY100 sub
    1     on   wave rom, effects ram      program rom+ram            PSR340
    1     on   effects ram                program rom+ram            QY100 main
    3     off? wave rom                   effects ram                PSR540



***************************************************************************/

#ifndef MAME_CPU_H8_SWX00_H
#define MAME_CPU_H8_SWX00_H

#pragma once

#include "h8s2000.h"
#include "h8_intc.h"
#include "h8_adc.h"
#include "h8_dma.h"
#include "h8_port.h"
#include "h8_timer8.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class swx00_device : public h8s2000_device {
public:
	enum {
		MODE_DUAL = 1,
		MODE_SEPARATE = 2
	};

	swx00_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u8 mode = 0);

	int s_bus_id() const { return m_mode & MODE_DUAL ? AS_DATA : AS_PROGRAM; }
	int c_bus_id() const { return m_mode & MODE_DUAL ? AS_PROGRAM : AS_DATA; }

#if 0
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
#endif

	u8 syscr_r();
	void syscr_w(u8 data);

protected:
	required_device<h8s_intc_device> m_intc;
#if 0
	required_device<h8_adc_device> m_adc;
	required_device<h8s_dma_device> m_dma;
	required_device<h8s_dma_channel_device> m_dma0;
	required_device<h8s_dma_channel_device> m_dma1;
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
#endif

	address_space_config m_data_config;

	u8 m_mode;
	u8 m_syscr;

	virtual bool exr_in_stack() const override;
	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual int trace_setup() override;
	virtual int trapa_setup() override;
	virtual void irq_setup() override;
	virtual void internal_update(u64 current_time) override;
	virtual void notify_standby(int state) override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual space_config_vector memory_space_config() const override;

	void map(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
};

DECLARE_DEVICE_TYPE(SWX00, swx00_device)

#endif // MAME_CPU_H8_SWX00_H
