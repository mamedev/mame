/**********************************************************************

    Conitec Datensysteme ECB Bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __ECBBUS__
#define __ECBBUS__

#include "emu.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define ECBBUS_TAG			"ecbbus"


#define MAX_ECBBUS_SLOTS	16



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ECBBUS_ADD(_cpu_tag, _config) \
	MCFG_DEVICE_ADD(ECBBUS_TAG, ECBBUS, 0) \
	MCFG_DEVICE_CONFIG(_config) \
    ecbbus_device::static_set_cputag(*device, _cpu_tag);


#define ECBBUS_INTERFACE(_name) \
	const ecbbus_interface (_name) =


#define MCFG_ECBBUS_SLOT_ADD(_num, _tag, _slot_intf, _def_slot, _def_inp) \
    MCFG_DEVICE_ADD(_tag, ECBBUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false) \
	ecbbus_slot_device::static_set_ecbbus_slot(*device, ECBBUS_TAG, _num);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ecbbus_slot_device

class ecbbus_device;

class ecbbus_slot_device : public device_t,
						   public device_slot_interface
{
public:
	// construction/destruction
	ecbbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

    // inline configuration
    static void static_set_ecbbus_slot(device_t &device, const char *tag, int num);

private:
	// configuration
	const char *m_bus_tag;
	int m_bus_num;
	ecbbus_device  *m_bus;
};


// device type definition
extern const device_type ECBBUS_SLOT;


// ======================> ecbbus_interface

struct ecbbus_interface
{
    devcb_write_line	m_out_int_cb;
    devcb_write_line	m_out_nmi_cb;
};

class device_ecbbus_card_interface;


// ======================> ecbbus_device

class ecbbus_device : public device_t,
					  public ecbbus_interface
{
public:
	// construction/destruction
	ecbbus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);

	void add_ecbbus_card(device_ecbbus_card_interface *card, int pos);

	DECLARE_READ8_MEMBER( mem_r );
	DECLARE_WRITE8_MEMBER( mem_w );

	DECLARE_READ8_MEMBER( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

	DECLARE_WRITE_LINE_MEMBER( int_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

private:
	// internal state
	device_t   *m_maincpu;

	devcb_resolved_write_line	m_out_int_func;
	devcb_resolved_write_line	m_out_nmi_func;

	device_ecbbus_card_interface *m_ecbbus_device[MAX_ECBBUS_SLOTS];
	const char *m_cputag;
};


// device type definition
extern const device_type ECBBUS;


// ======================> device_ecbbus_card_interface

// class representing interface-specific live ecbbus card
class device_ecbbus_card_interface : public device_slot_card_interface
{
	friend class ecbbus_device;

public:
	// construction/destruction
	device_ecbbus_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_ecbbus_card_interface();

	// optional operation overrides
	virtual UINT8 ecbbus_mem_r(offs_t offset) { return 0; };
	virtual void ecbbus_mem_w(offs_t offset, UINT8 data) { };
	virtual UINT8 ecbbus_io_r(offs_t offset) { return 0; };
	virtual void ecbbus_io_w(offs_t offset, UINT8 data) { };
	virtual UINT32 ecbbus_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return false; }

public:
	ecbbus_device  *m_ecb;
};

#endif
