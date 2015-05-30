// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ I/O Expansion Slot

    30-pin slot

    15  GND             16  NC
    14  /WR             17  NC
    13  A3              18  NC
    12  A6              19  +5VDC
    11  A2              20  NC
    10  A5              21  D4
     9  D0              22  D5
     8  D2              23  D7
     7  D6              24  D1
     6  D3              25  /RD
     5  /IORQ           26  A1
     4  +5VDC           27  A4
     3  NC              28  A0
     2  NC              29  A7
     1  NC              30  NC

***************************************************************************/

#pragma once

#ifndef __VTECH_IOEXP_H__
#define __VTECH_IOEXP_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_IOEXP_SLOT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, IOEXP_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(ioexp_slot_carts, NULL, false)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_ioexp_interface;

class ioexp_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	ioexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~ioexp_slot_device();

	void set_io_space(address_space *io);

	address_space *m_io;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	device_ioexp_interface *m_cart;
};

// class representing interface-specific live ioexp device
class device_ioexp_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_ioexp_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_ioexp_interface();

protected:
	ioexp_slot_device *m_slot;
};

// device type definition
extern const device_type IOEXP_SLOT;

// include here so drivers don't need to
#include "carts.h"

#endif // __VTECH_IOEXP_H__
