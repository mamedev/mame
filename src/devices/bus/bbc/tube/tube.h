// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Tube emulation

**********************************************************************

  Pinout:

             0V   1   2  R/NW (read/not-write)
             0V   3   4  2MHzE
             0V   5   6  NIRQ (not-interrupt request)
             0V   7   8  NTUBE
             0V   9  10  NRST (not-reset)
             0V  11  12  D0
             0V  13  14  D1
             0V  15  16  D2
             0V  17  18  D3
             0V  19  20  D4
             0V  21  22  D5
             0V  23  24  D6
             0V  25  26  D7
             0V  27  28  A0
             0V  29  30  A1
            +5V  31  32  A2
            +5V  33  34  A3
            +5V  35  36  A4
            +5V  37  38  A5
            +5V  39  40  A6

**********************************************************************/

#pragma once

#ifndef __BBC_TUBE_SLOT__
#define __BBC_TUBE_SLOT__

#include "emu.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define BBC_TUBE_SLOT_TAG      "tube"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BBC_TUBE_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, BBC_TUBE_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_BBC_PASSTHRU_TUBE_SLOT_ADD() \
	MCFG_BBC_TUBE_SLOT_ADD(BBC_TUBE_SLOT_TAG, 0, bbc_tube_devices, nullptr)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_slot_device

class device_bbc_tube_interface;

class bbc_tube_slot_device : public device_t,
												public device_slot_interface
{
public:
	// construction/destruction
	bbc_tube_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~bbc_tube_slot_device() {}


protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;


	device_bbc_tube_interface *m_card;
};


// ======================> device_bbc_tube_interface

class device_bbc_tube_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_bbc_tube_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_bbc_tube_interface();

protected:
	bbc_tube_slot_device *m_slot;
};


// device type definition
extern const device_type BBC_TUBE_SLOT;

SLOT_INTERFACE_EXTERN( bbc_tube_ext_devices );
SLOT_INTERFACE_EXTERN( bbc_tube_int_devices );


#endif
