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

#pragma once

#ifndef __SVI3X8_SLOT_H__
#define __SVI3X8_SLOT_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SVI_SLOT_BUS_ADD \
	MCFG_DEVICE_ADD("slotbus", SVI_SLOT_BUS, 0)

#define MCFG_SVI_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, SVI_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	svi_slot_device::set_bus(*device, owner, "slotbus");

#define MCFG_SVI_SLOT_INT_HANDLER(_devcb) \
	devcb = &svi_slot_bus_device::set_int_handler(*device, DEVCB_##_devcb);

#define MCFG_SVI_SLOT_ROMDIS_HANDLER(_devcb) \
	devcb = &svi_slot_bus_device::set_romdis_handler(*device, DEVCB_##_devcb);

#define MCFG_SVI_SLOT_RAMDIS_HANDLER(_devcb) \
	devcb = &svi_slot_bus_device::set_ramdis_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_svi_slot_interface;

// ======================> svi_slot_bus_device

class svi_slot_bus_device : public device_t
{
public:
	// construction/destruction
	svi_slot_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~svi_slot_bus_device();

	// callbacks
	template<class _Object> static devcb_base &set_int_handler(device_t &device, _Object object)
		{ return downcast<svi_slot_bus_device &>(device).m_int_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_romdis_handler(device_t &device, _Object object)
		{ return downcast<svi_slot_bus_device &>(device).m_romdis_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_ramdis_handler(device_t &device, _Object object)
		{ return downcast<svi_slot_bus_device &>(device).m_ramdis_handler.set_callback(object); }

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void add_card(device_svi_slot_interface *card);

	// from slot
	DECLARE_WRITE_LINE_MEMBER( int_w ) { m_int_handler(state); };
	DECLARE_WRITE_LINE_MEMBER( romdis_w ) { m_romdis_handler(state); };
	DECLARE_WRITE_LINE_MEMBER( ramdis_w ) { m_ramdis_handler(state); };

	// from host
	DECLARE_READ8_MEMBER( mreq_r );
	DECLARE_WRITE8_MEMBER( mreq_w );
	DECLARE_READ8_MEMBER( iorq_r );
	DECLARE_WRITE8_MEMBER( iorq_w );

	DECLARE_WRITE_LINE_MEMBER( bk21_w );
	DECLARE_WRITE_LINE_MEMBER( bk22_w );
	DECLARE_WRITE_LINE_MEMBER( bk31_w );
	DECLARE_WRITE_LINE_MEMBER( bk32_w );

private:
	simple_list<device_svi_slot_interface> m_dev;

	devcb_write_line m_int_handler;
	devcb_write_line m_romdis_handler;
	devcb_write_line m_ramdis_handler;
};

// device type definition
extern const device_type SVI_SLOT_BUS;

// ======================> svi_slot_device

class svi_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	svi_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration
	static void set_bus(device_t &device, device_t *owner, const char *bus_tag);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// configuration
	const char *m_bus_tag;
};

// device type definition
extern const device_type SVI_SLOT;

// ======================> svi_slot_device

class device_svi_slot_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_svi_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_svi_slot_interface();

	void set_bus_device(svi_slot_bus_device *bus);

	device_svi_slot_interface *next() const { return m_next; }
	device_svi_slot_interface *m_next;

	virtual DECLARE_READ8_MEMBER( mreq_r ) { return 0xff; };
	virtual DECLARE_WRITE8_MEMBER( mreq_w ) {};
	virtual DECLARE_READ8_MEMBER( iorq_r ) { return 0xff; };
	virtual DECLARE_WRITE8_MEMBER( iorq_w ) {};

	virtual DECLARE_WRITE_LINE_MEMBER( bk21_w ) {};
	virtual DECLARE_WRITE_LINE_MEMBER( bk22_w ) {};
	virtual DECLARE_WRITE_LINE_MEMBER( bk31_w ) {};
	virtual DECLARE_WRITE_LINE_MEMBER( bk32_w ) {};

protected:
	svi_slot_bus_device *m_bus;
};

// include here so drivers don't need to
#include "cards.h"

#endif // __SVI3X8_SLOT_H__
