// license: GPL-2.0+
// copyright-holders: Dirk Best
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

#ifndef MAME_BUS_VTECH_IOEXP_IOEXP_H
#define MAME_BUS_VTECH_IOEXP_IOEXP_H

#pragma once

#include "machine/bankdev.h"

// include here so drivers don't need to
#include "carts.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_vtech_ioexp_interface;

class vtech_ioexp_slot_device : public device_t, public device_single_card_slot_interface<device_vtech_ioexp_interface>
{
	friend class device_vtech_ioexp_interface;
public:
	// construction/destruction
	vtech_ioexp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner)
		: vtech_ioexp_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		vtech_ioexp_slot_carts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}
	vtech_ioexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~vtech_ioexp_slot_device();

	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	required_address_space m_iospace;

	device_vtech_ioexp_interface *m_module;
};

// class representing interface-specific live ioexp device
class device_vtech_ioexp_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_vtech_ioexp_interface();

	virtual uint8_t iorq_r(offs_t offset) { return 0xff; }
	virtual void iorq_w(offs_t offset, uint8_t data) { }

protected:
	device_vtech_ioexp_interface(const machine_config &mconfig, device_t &device);

	vtech_ioexp_slot_device *m_slot;
};

// base io expansion device
class vtech_ioexp_device : public device_t, public device_vtech_ioexp_interface
{
public:
	// construction/destruction
	vtech_ioexp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	virtual void io_map(address_map &map) { }

	required_device<address_map_bank_device> m_io;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_IOEXP_SLOT, vtech_ioexp_slot_device)

#endif // MAME_BUS_VTECH_IOEXP_IOEXP_H
