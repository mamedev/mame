// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_IBM_RTPC_KLS_H
#define MAME_IBM_RTPC_KLS_H

#pragma once

#include "rtpc_kbdc.h"

#include "bus/rs232/rs232.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "sound/spkrdev.h"

class rtpc_kls_device : public device_t
{
public:
	rtpc_kls_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// input/output lines
	auto atn() { return m_atn.bind(); }
	auto irq() { return m_irq.bind(); }
	void mcu_reset_w(int state);
	void rtc_sqw_w(int state);
	void rtc_irq_w(int state);

	// device address map
	void map(address_map &map);

protected:
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void mcu_map(address_map &map) ATTR_COLD;

	void cmd_w(u16 data);
	void mcu_port1_w(u8 data);
	void mcu_port2_w(u8 data);
	void mcu_port3_w(u8 data);
	void ppi_portc_w(u8 data);
	void speaker();

private:
	required_device<i8051_device> m_mcu;
	required_device<i8255_device> m_ppi;
	required_device<rtpc_kbdc_device> m_kbdc;
	required_device<rs232_port_device> m_locc;
	required_device_array<input_merger_all_high_device, 2> m_rcv;

	optional_device<speaker_sound_device> m_speaker;

	required_ioport m_keylock;

	devcb_write_line m_atn;
	devcb_write_line m_irq;

	u8 m_cmd;

	u8 m_mcu_p[4];
	u8 m_ppi_p[3];
};

DECLARE_DEVICE_TYPE(RTPC_KLS, rtpc_kls_device)

#endif // MAME_IBM_RTPC_KLS_H
