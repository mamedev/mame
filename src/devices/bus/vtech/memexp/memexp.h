// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Memory Expansion Slot

    44-pin slot

    22  GND             23  NC
    21  +5VDC           24  A11
    20  /IORQ           25  A12
    19  /RD             26  A13
    18  /NMI            27  A14
    17  /WAIT           28  A15
    16  /MI             29  CLK
    15  /RFSH           30  D4
    14  D7              31  D3
    13  D2              32  D5
    12  A1              33  D6
    11  A2              34  NC
    10  A3              35  A0
     9  A4              36  D0
     8  A5              37  D1
     7  A6              38  /INT
     6  A7              39  /HALT
     5  A8              40  /MERQ
     4  A9              41  /WR
     3  A10             42  /NC
     2  /RESET          43  +9VDC
     1  GND             44  NC

***************************************************************************/

#ifndef MAME_BUS_VTECH_MEMEXP_MEMEXP_H
#define MAME_BUS_VTECH_MEMEXP_MEMEXP_H

#pragma once

// include here so drivers don't need to
#include "carts.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_vtech_memexp_interface;

class vtech_memexp_slot_device : public device_t, public device_slot_interface
{
	friend class device_vtech_memexp_interface;
public:
	// construction/destruction
	vtech_memexp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner)
		: vtech_memexp_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		vtech_memexp_carts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}
	vtech_memexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~vtech_memexp_slot_device();

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }
	auto reset_handler() { return m_reset_handler.bind(); }

	// called from cart device
	DECLARE_WRITE_LINE_MEMBER( int_w ) { m_int_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_nmi_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( reset_w ) { m_reset_handler(state); }

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	required_address_space m_program;
	required_address_space m_io;

	device_vtech_memexp_interface *m_cart;

private:
	devcb_write_line m_int_handler;
	devcb_write_line m_nmi_handler;
	devcb_write_line m_reset_handler;
};

// class representing interface-specific live memexp device
class device_vtech_memexp_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_vtech_memexp_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_vtech_memexp_interface();

protected:
	address_space &program_space() { return *m_slot->m_program; }
	address_space &io_space() { return *m_slot->m_io; }

	vtech_memexp_slot_device *m_slot;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_MEMEXP_SLOT, vtech_memexp_slot_device)

#endif // MAME_BUS_VTECH_MEMEXP_MEMEXP_H
