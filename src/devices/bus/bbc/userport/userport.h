// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC User Port emulation

**********************************************************************

  Pinout:

            +5V   1   2  CB1
            +5V   3   4  CB2
             0V   5   6  PB0
             0V   7   8  PB1
             0V   9  10  PB2
             0V  11  12  PB3
             0V  13  14  PB4
             0V  15  16  PB5
             0V  17  18  PB6
             0V  19  20  PB7

  Signal Definitions:

  Connected directly to the 6522 User VIA.

**********************************************************************/

#pragma once

#ifndef __BBC_USERPORT_SLOT__
#define __BBC_USERPORT_SLOT__

#include "emu.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_userport_device;

// ======================> device_bbc_userport_interface

class device_bbc_userport_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_bbc_userport_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_bbc_userport_interface();

	virtual UINT8 read_portb() { return 0xff; };
	virtual UINT8 read_cb1() { return 0xff; };
	virtual UINT8 read_cb2() { return 0xff; };

protected:
	bbc_userport_device *m_slot;
};

// ======================> bbc_userport_device

class bbc_userport_device : public device_t,
	public device_slot_interface
{
public:
	// construction/destruction
	bbc_userport_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~bbc_userport_device() {}

	UINT8 read_portb();
	UINT8 read_cb1();
	UINT8 read_cb2();

	// device-level overrides
	virtual void device_start() override;

protected:
	device_bbc_userport_interface *m_device;
};

// device type definition
extern const device_type BBC_USERPORT_SLOT;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BBC_USERPORT_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, BBC_USERPORT_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_BBC_USERPORT_PB_HANDLER(_devcb) \
	devcb = &bbc_userport_device::set_pb_handler(*device, DEVCB_##_devcb);

#define MCFG_BBC_USERPORT_CB1_HANDLER(_devcb) \
	devcb = &bbc_userport_device::set_cb1_handler(*device, DEVCB_##_devcb);

#define MCFG_BBC_USERPORT_CB2_HANDLER(_devcb) \
	devcb = &bbc_userport_device::set_cb2_handler(*device, DEVCB_##_devcb);


SLOT_INTERFACE_EXTERN( bbc_userport_devices );


#endif
