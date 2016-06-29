// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    Sega SG-1000 expansion slot emulation

**********************************************************************


**********************************************************************/

#pragma once

#ifndef __SG1000_EXPANSION_SLOT__
#define __SG1000_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SG1000_EXPANSION_ADD(_tag, _slot_intf, _def_slot, _fixed) \
	MCFG_DEVICE_ADD(_tag, SG1000_EXPANSION_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _fixed)
#define MCFG_SG1000_EXPANSION_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sg1000_expansion_slot_device

class device_sg1000_expansion_slot_interface;

class sg1000_expansion_slot_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	sg1000_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~sg1000_expansion_slot_device();

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	bool is_readable(UINT8 offset);
	bool is_writeable(UINT8 offset);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	device_sg1000_expansion_slot_interface *m_device;
};


// ======================> device_sg1000_expansion_slot_interface

// class representing interface-specific live sg1000_expansion card
class device_sg1000_expansion_slot_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_sg1000_expansion_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_sg1000_expansion_slot_interface();

	virtual DECLARE_READ8_MEMBER(peripheral_r) { return 0xff; };
	virtual DECLARE_WRITE8_MEMBER(peripheral_w) { };

	virtual bool is_readable(UINT8 offset) { return true; };
	virtual bool is_writeable(UINT8 offset) { return true; };

protected:
	sg1000_expansion_slot_device *m_port;
};


// device type definition
extern const device_type SG1000_EXPANSION_SLOT;


SLOT_INTERFACE_EXTERN( sg1000_expansion_devices );


#endif
