// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC keyboard emulation

*********************************************************************/

#ifndef MAME_WANG_WANGPCKB_H
#define MAME_WANG_WANGPCKB_H

#pragma once

#include "cpu/mcs51/mcs51.h"

#include "sound/sn76496.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_keyboard_device

class wangpc_keyboard_device :  public device_t
{
public:
	// construction/destruction
	wangpc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto txd_handler() { return m_txd_handler.bind(); }

	void write_rxd(int state);

	void wangpc_keyboard_io(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<i8051_device> m_maincpu;
	required_ioport_array<16> m_y;
	devcb_write_line m_txd_handler;
	output_finder<6> m_leds;

	uint8_t m_keylatch;
	int m_rxd;

	uint8_t kb_p1_r();
	void kb_p1_w(uint8_t data);
	void kb_p2_w(uint8_t data);
	uint8_t kb_p3_r();
	void kb_p3_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(WANGPC_KEYBOARD, wangpc_keyboard_device)

#endif // MAME_WANG_WANGPCKB_H
