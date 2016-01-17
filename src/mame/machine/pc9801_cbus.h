// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**********************************************************************


**********************************************************************/

#pragma once

#ifndef __PC9801_SLOTCBUS__
#define __PC9801_SLOTCBUS__

#include "emu.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************


#define MCFG_PC9801CBUS_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, PC9801CBUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************




//class pc9801_slot_device;

#if 0
class device_pc9801_slot_card_interface : public device_slot_card_interface
{
	friend class pc9801_slot_device;

public:
	// construction/destruction
	device_pc9801_slot_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_pc9801_card_interface();
};
#endif

// ======================> pc9801_slot_device

class pc9801_slot_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	pc9801_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;
//private:
//  device_pc9801_slot_card_interface *m_card;

};


// device type definition
extern const device_type PC9801CBUS_SLOT;



#endif
