// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom NASBUS

    77-pin slot

     1  0V
     2  0V
     3  0V
     4  0V
     5  Clock
     6  (spare)
     7  (spare)
     8  (spare)
     9  /RAM DISABLE
    10  /RESET SWITCH
    11  /NASCOM MEM
    12  /NASCOM IO
    13  /DBDR
    14  /RESET
    15  /HALT
    16  /BAI
    17  /BAO
    18  /BUSRQ
    19  IEI
    20  IEO
    21  (reserved for /NMI)
    22  /INT
    23  /WAIT
    24  /RFSH
    25  /MI
    26  /IORQ
    27  /MREQ
    28  /WR
    29  /RD
    30  A0
    31  A1
    32  A2
    33  A3
    34  A4
    35  A5
    36  A6
    37  A7
    38  A8
    39  A9
    40  A10
    41  A11
    42  A12
    43  A13
    44  A14
    45  A15
    46  (reserved)
    47  (reserved)
    48  (reserved)
    49  (reserved)
    50  D0
    51  D1
    52  D2
    53  D3
    54  D4
    55  D5
    56  D6
    57  D7
    58  (reserved)
    59  (reserved)
    60  (reserved)
    61  (reserved)
    62  (reserved)
    63  (reserved)
    64  (reserved)
    65  (reserved)
    66  (unused)
    67  (unused)
    68  -5V
    69  -5V
    70  -12V
    71  -12V
    72  Keyway
    73  +12V
    74  +12V
    75  +5V
    76  +5V
    77  +5V
    78  +5V

***************************************************************************/

#pragma once

#ifndef __NASBUS_H__
#define __NASBUS_H__

#include "emu.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define NASBUS_TAG "nasbus"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NASBUS_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NASBUS, 0)

#define MCFG_NASBUS_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, NASBUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	nasbus_slot_device::set_nasbus_slot(*device, owner, NASBUS_TAG);

#define MCFG_NASBUS_RAM_DISABLE_HANDLER(_devcb) \
	devcb = &nasbus_device::set_ram_disable_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class device_nasbus_card_interface;

// ======================> nasbus_slot_device

class nasbus_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	nasbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	nasbus_slot_device(const machine_config &mconfig, device_type type, const char *name,
		const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;

	// inline configuration
	static void set_nasbus_slot(device_t &device, device_t *owner, const char *nasbus_tag);

protected:
	// configuration
	const char *m_nasbus_tag;
};

// device type definition
extern const device_type NASBUS_SLOT;

// ======================> nasbus_device

class nasbus_device : public device_t
{
public:
	// construction/destruction
	nasbus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~nasbus_device();

	template<class _Object> static devcb_base &set_ram_disable_handler(device_t &device, _Object object)
		{ return downcast<nasbus_device &>(device).m_ram_disable_handler.set_callback(object); }

	void add_card(device_nasbus_card_interface *card);

	void set_program_space(address_space *program);
	void set_io_space(address_space *io);

	// from cards
	DECLARE_WRITE_LINE_MEMBER( ram_disable_w );

	address_space *m_program;
	address_space *m_io;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	simple_list<device_nasbus_card_interface> m_dev;

	devcb_write_line m_ram_disable_handler;
};

// device type definition
extern const device_type NASBUS;

// ======================> device_nasbus_interface

class device_nasbus_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_nasbus_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_nasbus_card_interface();

	void set_nasbus_device(nasbus_device *nasbus);

	device_nasbus_card_interface *next() const { return m_next; }
	device_nasbus_card_interface *m_next;

protected:
	nasbus_device *m_nasbus;
};

// include here so drivers don't need to
#include "cards.h"

#endif // __NASBUS_H__
