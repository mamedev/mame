// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe Expansion Slot

    64-pin slot

     1A DBDIR       1C IORQL
     2A RDL         2C MREQL
     3A WRL         3C HALTL
     4A BUSAKL      4C NMIL
     5A WAITL       5C INTL
     6A BUSREQL     6C CD1
     7A RESETL      7C CD0
     8A CM1L        8C CD7
     9A REFRESHL    9C CD2
    10A 0 VOLTS    10C +5 VOLTS
    11A A0         11C CD6
    12A A1         12C CD5
    13A A2         13C CD3
    14A A3         14C CD4
    15A A4         15C CPU CLK
    16A A5         16C A15
    17A A6         17C A14
    18A A7         18C A13
    19A A8         19C A12
    20A A9         20C A11
    21A A10        21C DISK 2L
    22A MSEINTL    22C ROMCSL
    23A XMEML      23C EARMIC
    24A 8 MHz      24C DISK 1L
    25A RED 1      25C PRINTL
    26A GREEN 1    26C BLUE 1
    27A C SYNC     27C ROMCSRL
    28A SPEN       28C AUDIO RIGHT
    29A BLUE 0     29C AUDIO LEFT
    30A RED 0      30C COMP VIDEO
    31A BRIGHT     31C GREEN 0
    32A +5 VOLTS   32C 0 VOLTS

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_EXPANSION_EXPANSION_H
#define MAME_BUS_SAMCOUPE_EXPANSION_EXPANSION_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_samcoupe_expansion_interface;

// ======================> samcoupe_expansion_device

class samcoupe_expansion_device : public device_t, public device_single_card_slot_interface<device_samcoupe_expansion_interface>
{
public:
	// construction/destruction
	template <typename T>
	samcoupe_expansion_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts)
		: samcoupe_expansion_device(mconfig, tag, owner, uint32_t(0))
	{
		option_reset();
		opts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	samcoupe_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~samcoupe_expansion_device();

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }

	// called from cart device
	void int_w(int state) { m_int_handler(state); }

	// called from host
	uint8_t mreq_r(offs_t offset);
	void mreq_w(offs_t offset, uint8_t data);
	uint8_t iorq_r(offs_t offset);
	void iorq_w(offs_t offset, uint8_t data);

	void xmem_w(int state);
	void print_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_int_handler;

	device_samcoupe_expansion_interface *m_module;
};

// ======================> device_samcoupe_expansion_interface

class device_samcoupe_expansion_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_samcoupe_expansion_interface();

	virtual uint8_t mreq_r(offs_t offset) { return 0xff; }
	virtual void mreq_w(offs_t offset, uint8_t data) { }
	virtual uint8_t iorq_r(offs_t offset) { return 0xff; }
	virtual void iorq_w(offs_t offset, uint8_t data) { }

	virtual void xmem_w(int state) { }
	virtual void print_w(int state) { }

protected:
	device_samcoupe_expansion_interface(const machine_config &mconfig, device_t &device);

	samcoupe_expansion_device *m_expansion;
};

// device type definition
DECLARE_DEVICE_TYPE(SAMCOUPE_EXPANSION, samcoupe_expansion_device)

// include here so drivers don't need to
#include "modules.h"

#endif // MAME_BUS_SAMCOUPE_EXPANSION_EXPANSION_H
