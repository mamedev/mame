// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Einstein User Port

    16-pin slot

    +5V     1   2  D0
    GND     3   4  D1
    BRDY    5   6  D2
    GND     7   8  D3
    GND     9  10  D4
    /BSTB  11  12  D5
    GND    13  14  D6
    +5V    15  16  D7

***************************************************************************/

#ifndef MAME_BUS_EINSTEIN_USERPORT_USERPORT_H
#define MAME_BUS_EINSTEIN_USERPORT_USERPORT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_einstein_userport_interface;

// supported devices
void einstein_userport_cards(device_slot_interface &device);

class einstein_userport_device : public device_t, public device_single_card_slot_interface<device_einstein_userport_interface>
{
public:
	// construction/destruction
	einstein_userport_device(machine_config const &mconfig, char const *tag, device_t *owner)
		: einstein_userport_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		einstein_userport_cards(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	einstein_userport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~einstein_userport_device();

	// callbacks
	auto bstb_handler() { return m_bstb_handler.bind(); }

	// called from card device
	void bstb_w(int state) { m_bstb_handler(state); }

	uint8_t read();
	void write(uint8_t data);
	void brdy_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_einstein_userport_interface *m_card;

private:
	devcb_write_line m_bstb_handler;
};

// class representing interface-specific live userport device
class device_einstein_userport_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_einstein_userport_interface();

	virtual uint8_t read() { return 0xff; }
	virtual void write(uint8_t data) { }
	virtual void brdy_w(int state) { }

protected:
	device_einstein_userport_interface(const machine_config &mconfig, device_t &device);

	einstein_userport_device *m_slot;
};

// device type definition
DECLARE_DEVICE_TYPE(EINSTEIN_USERPORT, einstein_userport_device)

#endif // MAME_BUS_EINSTEIN_USERPORT_USERPORT_H
