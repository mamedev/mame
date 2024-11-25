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
#include "sound/swx00.h"

class swx00_device : public h8s2000_device, public device_mixer_interface {
public:
	enum {
		AS_C = AS_PROGRAM,
		AS_S = AS_DATA
	};

	enum {
		MODE_DUAL = 1,
		MODE_SEPARATE = 2
	};

	swx00_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u8 mode = 0);

	auto read_pdt()    { return m_read_pdt.bind(); }
	auto write_pdt()   { return m_write_pdt.bind(); }
	auto read_pad()    { return m_read_pad.bind(); }
	auto write_pad()   { return m_write_pad.bind(); }
	auto write_cmah()  { return m_write_cmah.bind(); }
	auto write_txd()  { return m_write_txd.bind(); }

	u8 syscr_r();
	void syscr_w(u8 data);

protected:
	required_device<h8s_intc_device> m_intc;
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

	required_device<swx00_sound_device> m_swx00;

	devcb_read16 m_read_pdt;
	devcb_write16 m_write_pdt;
	devcb_read8 m_read_pad;
	devcb_write8 m_write_pad;
	devcb_write8 m_write_cmah;
	devcb_write8 m_write_txd;

	address_space_config m_s_config;

	memory_access<24, 1, -1, ENDIANNESS_BIG>::specific m_s;

	u16 m_pdt, m_pdt_ddr;
	u8 m_pad;

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
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	u16 s_r(offs_t offset);

	void pdt_ddr_w(offs_t, u16 data, u16 mem_mask);
	u16 pdt_r();
	void pdt_w(offs_t, u16 data, u16 mem_mask);
	u8 pad_r();
	void pad_w(u8 data);
	void cmah_w(u8 data);
	void txd_w(u8 data);

	u16 pdt_default_r();
	void pdt_default_w(u16 data);
	u8 pad_default_r();
	void pad_default_w(u8 data);
	void cmah_default_w(u8 data);
	void txd_default_w(u8 data);

	void map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_set_input(int inputnum, int state) override;
};

DECLARE_DEVICE_TYPE(SWX00, swx00_device)

#endif // MAME_CPU_H8_SWX00_H
