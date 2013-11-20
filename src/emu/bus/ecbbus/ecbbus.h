// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Kontron Europe Card Bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                              A  B  C
                +5 V    ---   *  1  *   --- +5V
                D5      ---   *  2  *   --- D0
                D6      ---   *  3  *   --- D7
                D3      ---   *  4  *   --- D2
                D4      ---   *  5  *   --- A0
                A2      ---   *  6  *   --- A3
                A4      ---   *  7  *   --- A1
                A5      ---   *  8  *   --- A8
                A6      ---   *  9  *   --- A7
                WAIT*   ---   * 10  *   --- D8
                BUSRQ*  ---   * 11  *   --- IEI
                BAI1    ---   * 12  *   --- D9
                +12 V   ---   * 13  *   --- D10
                D11     ---   * 14  *   --- D1
                -5 V    ---   * 15  *   --- -15 V
                phi2    ---   * 16  *   --- IEO
                BAO1    ---   * 17  *   --- A11
                A14     ---   * 18  *   --- A10
                +15 V   ---   * 19  *   --- D13
                M1*     ---   * 20  *   --- NMI*
                D14     ---   * 21  *   --- INT*
                D15     ---   * 22  *   --- WR*
                DPR*    ---   * 23  *   --- D12
                +5VBAT  ---   * 24  *   --- RD*
                phiN    ---   * 25  *   --- HALT*
                WRITE EN---   * 26  *   --- PWRRCL*
                IORQ*   ---   * 27  *   --- A12
                RFSH*   ---   * 28  *   --- A15
                A13     ---   * 29  *   --- PHI
                A9      ---   * 30  *   --- MREQ*
                BUSAK*  ---   * 31  *   --- RESET*
                GND     ---   * 32  *   --- GND

**********************************************************************/

#pragma once

#ifndef __ECBBUS__
#define __ECBBUS__

#include "emu.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define ECBBUS_TAG          "ecbbus"


#define MAX_ECBBUS_SLOTS    16



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ECBBUS_ADD(_cpu_tag, _config) \
	MCFG_DEVICE_ADD(ECBBUS_TAG, ECBBUS, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	ecbbus_device::static_set_cputag(*device, _cpu_tag);


#define ECBBUS_INTERFACE(_name) \
	const ecbbus_interface (_name) =


#define MCFG_ECBBUS_SLOT_ADD(_num, _tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, ECBBUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
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
	devcb_write_line    m_out_int_cb;
	devcb_write_line    m_out_nmi_cb;
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

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

private:
	// internal state
	cpu_device   *m_maincpu;

	devcb_resolved_write_line   m_out_int_func;
	devcb_resolved_write_line   m_out_nmi_func;

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

public:
	ecbbus_slot_device  *m_slot;
};


// slot devices
#include "grip.h"

SLOT_INTERFACE_EXTERN( ecbbus_cards );



#endif
