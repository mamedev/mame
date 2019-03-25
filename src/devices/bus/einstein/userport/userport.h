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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_EINSTEIN_USERPORT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EINSTEIN_USERPORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(einstein_userport_cards, nullptr, false)

#define MCFG_EINSTEIN_USERPORT_BSTB_HANDLER(_devcb) \
	downcast<einstein_userport_device &>(*device).set_bstb_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_einstein_userport_interface;

class einstein_userport_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	einstein_userport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~einstein_userport_device();

	// callbacks
	template <class Object> devcb_base &set_bstb_handler(Object &&cb) { return m_bstb_handler.set_callback(std::forward<Object>(cb)); }

	// called from card device
	DECLARE_WRITE_LINE_MEMBER( bstb_w ) { m_bstb_handler(state); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER (write );
	DECLARE_WRITE_LINE_MEMBER( brdy_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	device_einstein_userport_interface *m_card;

private:
	devcb_write_line m_bstb_handler;
};

// class representing interface-specific live userport device
class device_einstein_userport_interface : public device_slot_card_interface
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

// supported devices
void einstein_userport_cards(device_slot_interface &device);

#endif // MAME_BUS_EINSTEIN_USERPORT_USERPORT_H
