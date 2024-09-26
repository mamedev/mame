// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_SHARED_MS7004_H
#define MAME_SHARED_MS7004_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"
#include "sound/beep.h"

#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ms7004_device

class ms7004_device : public device_t //, public device_serial_interface
{
public:
	// construction/destruction
	ms7004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto tx_handler() { return m_tx_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }

	void write_rxd(int state);

	void ms7004_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<i8035_device> m_maincpu;
	required_device<beep_device> m_speaker;
	required_device<i8243_device> m_i8243;

	required_ioport_array<16> m_kbd;

	int m_keylatch;                 // keyboard row latch
	uint8_t m_p1;
	uint8_t m_p2;

	devcb_write_line m_tx_handler;
	devcb_write_line m_rts_handler;

	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
	int t1_r();
	template<int P> void i8243_port_w(uint8_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(MS7004, ms7004_device)

#endif // MAME_SHARED_MS7004_H
