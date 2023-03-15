// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SVI 318/328 Expansion Slot

    50-pin slot

     1  +5V         2  +5V
     3  +12V        4  -12V
     5  GND         6  /WAIT
     7  /RST        8  CPUCLK
     9  A15        10  A14
    11  A13        12  A12
    13  A11        14  A10
    15  A9         16  A8
    17  A7         18  A6
    19  A5         20  A4
    21  A3         22  A2
    23  A1         24  A0
    25  /RFSH      26  GND
    27  /M1        28  GND
    29  /WR        30  /MREQ
    31  /IORQ      32  /RD
    33  D0         34  D1
    35  D2         36  D3
    37  D4         38  D5
    39  D6         40  D7
    41  CSOUND     42  /INT
    43  /RAMDIS    44  /ROMDIS
    45  /BK32      46  /BK31
    47  /BK22      48  /BK21
    49  GND        50  GND

***************************************************************************/

#ifndef MAME_BUS_SVI3X8_SLOT_SLOT_H
#define MAME_BUS_SVI3X8_SLOT_SLOT_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_svi_slot_interface;

// ======================> svi_slot_bus_device

class svi_slot_bus_device : public device_t
{
public:
	// construction/destruction
	svi_slot_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~svi_slot_bus_device();

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }
	auto romdis_handler() { return m_romdis_handler.bind(); }
	auto ramdis_handler() { return m_ramdis_handler.bind(); }

	void add_card(device_svi_slot_interface &card);

	// from slot
	DECLARE_WRITE_LINE_MEMBER( int_w ) { m_int_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( romdis_w ) { m_romdis_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( ramdis_w ) { m_ramdis_handler(state); }

	// from host
	uint8_t mreq_r(offs_t offset);
	void mreq_w(offs_t offset, uint8_t data);
	uint8_t iorq_r(offs_t offset);
	void iorq_w(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( bk21_w );
	DECLARE_WRITE_LINE_MEMBER( bk22_w );
	DECLARE_WRITE_LINE_MEMBER( bk31_w );
	DECLARE_WRITE_LINE_MEMBER( bk32_w );

private:
	// device-level overrides
	virtual void device_start() override;

	simple_list<device_svi_slot_interface> m_dev;

	devcb_write_line m_int_handler;
	devcb_write_line m_romdis_handler;
	devcb_write_line m_ramdis_handler;
};

// device type definition
DECLARE_DEVICE_TYPE(SVI_SLOT_BUS, svi_slot_bus_device)

// ======================> svi_slot_device

class svi_slot_device : public device_t, public device_single_card_slot_interface<device_svi_slot_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	svi_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&bus, U &&opts, char const *dflt)
		: svi_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		set_bus(std::forward<T>(bus));
	}

	svi_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_bus(T &&tag) { m_bus.set_tag(std::forward<T>(tag)); }

protected:
	// device-level overrides
	virtual void device_start() override;

	// configuration
	required_device<svi_slot_bus_device> m_bus;
};

// device type definition
DECLARE_DEVICE_TYPE(SVI_SLOT, svi_slot_device)

// ======================> svi_slot_device

class device_svi_slot_interface : public device_interface
{
	template <class ElementType> friend class simple_list;
public:
	// construction/destruction
	virtual ~device_svi_slot_interface();

	void set_bus_device(svi_slot_bus_device &bus);

	device_svi_slot_interface *next() const { return m_next; }

	virtual uint8_t mreq_r(offs_t offset) { return 0xff; }
	virtual void mreq_w(offs_t offset, uint8_t data) { }
	virtual uint8_t iorq_r(offs_t offset) { return 0xff; }
	virtual void iorq_w(offs_t offset, uint8_t data) { }

	virtual DECLARE_WRITE_LINE_MEMBER( bk21_w ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( bk22_w ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( bk31_w ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( bk32_w ) { }

protected:
	device_svi_slot_interface(const machine_config &mconfig, device_t &device);

	svi_slot_bus_device *m_bus;

private:
	device_svi_slot_interface *m_next;
};

// include here so drivers don't need to
#include "cards.h"

#endif // MAME_BUS_SVI3X8_SLOT_SLOT_H
