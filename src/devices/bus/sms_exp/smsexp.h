// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System expansion slot emulation

**********************************************************************


**********************************************************************/

#pragma once

#ifndef __SMS_EXPANSION_SLOT__
#define __SMS_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SMS_EXPANSION_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, SMS_EXPANSION_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_expansion_slot_device

class device_sms_expansion_slot_interface;

class sms_expansion_slot_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	sms_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~sms_expansion_slot_device();

	// reading and writing
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_mapper(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	int get_lphaser_xoffs();

	device_sms_expansion_slot_interface *m_device;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> device_sms_expansion_slot_interface

// class representing interface-specific live sms_expansion card
class device_sms_expansion_slot_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_sms_expansion_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_sms_expansion_slot_interface();

	// reading and writing
	virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return 0xff; };
	virtual void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { };
	virtual void write_mapper(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) {};
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return 0xff; };
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { };

	virtual int get_lphaser_xoffs() { return 0; };
};


// device type definition
extern const device_type SMS_EXPANSION_SLOT;


// slot devices
#include "gender.h"

SLOT_INTERFACE_EXTERN( sms_expansion_devices );


#endif
