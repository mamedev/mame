// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Decision Mate V keyboard emulation

*********************************************************************/
#ifndef MAME_NCR_DMV_KEYB_H
#define MAME_NCR_DMV_KEYB_H

#pragma once

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmv_keyboard_device

class dmv_keyboard_device : public device_t
{
public:
	// construction/destruction
	dmv_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void sd_poll_w(int state);
	int sd_poll_r();

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<i8741a_device> m_maincpu;
	required_ioport_array<16> m_keyboard;

	uint8_t m_col;
	int     m_sd_data_state;
	int     m_sd_poll_state;

	uint8_t port1_r();
	uint8_t port2_r();
	void port2_w(uint8_t data);
};


// device type declaration
DECLARE_DEVICE_TYPE(DMV_KEYBOARD, dmv_keyboard_device)


#endif // MAME_NCR_DMV_KEYB_H
