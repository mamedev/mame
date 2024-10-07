// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 keyboard emulation

*********************************************************************/

#ifndef MAME_ACT_VICTOR9K_KB_H
#define MAME_ACT_VICTOR9K_KB_H


#pragma once

#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> victor_9000_keyboard_device

class victor_9000_keyboard_device :  public device_t
{
public:
	// construction/destruction
	victor_9000_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto kbrdy_handler() { return m_kbrdy_cb.bind(); }
	auto kbdata_handler() { return m_kbdata_cb.bind(); }

	void kback_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<i8021_device> m_maincpu;
	required_ioport_array<13> m_y;

	devcb_write_line   m_kbrdy_cb;
	devcb_write_line   m_kbdata_cb;

	uint8_t m_p1;
	uint8_t m_keylatch;
	int m_stb;
	int m_y12;
	int m_kbrdy;
	int m_kbdata;
	int m_kback;

	uint8_t kb_p1_r();
	void kb_p1_w(uint8_t data);
	void kb_p2_w(uint8_t data);
	int kb_t1_r();
};


// device type definition
DECLARE_DEVICE_TYPE(VICTOR9K_KEYBOARD, victor_9000_keyboard_device)

#endif // MAME_ACT_VICTOR9K_KB_H
