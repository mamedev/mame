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

#ifndef MAME_BUS_NASBUS_NASBUS_H
#define MAME_BUS_NASBUS_NASBUS_H

#pragma once

#include <functional>
#include <vector>



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class device_nasbus_card_interface;
class nasbus_device;

// ======================> nasbus_slot_device

class nasbus_slot_device : public device_t, public device_single_card_slot_interface<device_nasbus_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	nasbus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&bus, U &&opts, char const *dflt)
		: nasbus_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		set_bus(std::forward<T>(bus));
	}
	nasbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_bus(T &&tag) { m_bus.set_tag(std::forward<T>(tag)); }

protected:
	nasbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	required_device<nasbus_device> m_bus;
};

// device type definition
DECLARE_DEVICE_TYPE(NASBUS_SLOT, nasbus_slot_device)

// ======================> nasbus_device

class nasbus_device : public device_t
{
	friend class device_nasbus_card_interface;
public:
	// construction/destruction
	nasbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~nasbus_device();

	auto ram_disable() { return m_ram_disable_handler.bind(); }

	void add_card(device_nasbus_card_interface &card);

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

	// from cards
	void ram_disable_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	using card_vector = std::vector<std::reference_wrapper<device_nasbus_card_interface> >;

	required_address_space m_program;
	required_address_space m_io;

	devcb_write_line m_ram_disable_handler;

	card_vector m_dev;
};

// device type definition
DECLARE_DEVICE_TYPE(NASBUS, nasbus_device)

// ======================> device_nasbus_interface

class device_nasbus_card_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_nasbus_card_interface();

	void set_nasbus_device(nasbus_device &nasbus);

protected:
	device_nasbus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	void ram_disable_w(int state) { m_nasbus->ram_disable_w(state); }

	address_space &program_space() { return *m_nasbus->m_program; }
	address_space &io_space() { return *m_nasbus->m_io; }

private:
	nasbus_device *m_nasbus;
};

// include here so drivers don't need to
#include "cards.h"

#endif // MAME_BUS_NASBUS_NASBUS_H
