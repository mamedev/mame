// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Telenova Compis keyboard emulation

*********************************************************************/

#ifndef MAME_TELENOVA_COMPISKB_H
#define MAME_TELENOVA_COMPISKB_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/spkrdev.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> compis_keyboard_device

class compis_keyboard_device : public device_t
{
public:
	// construction/destruction
	compis_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_tx_handler() { return m_out_tx_handler.bind(); }

	void si_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<i8748_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<9> m_y;
	required_ioport m_special;
	devcb_write_line m_out_tx_handler;
	output_finder<> m_led_caps;

	uint8_t m_bus;
	uint8_t m_keylatch;

	uint8_t bus_r();
	void bus_w(uint8_t data);
	uint8_t p1_r();
	uint8_t p2_r();
};


// device type definition
DECLARE_DEVICE_TYPE(COMPIS_KEYBOARD, compis_keyboard_device)

#endif // MAME_TELENOVA_COMPISKB_H
