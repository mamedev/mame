// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Gender Adapter" emulation

**********************************************************************/

#pragma once

#ifndef __SMS_GENDER_ADAPTER__
#define __SMS_GENDER_ADAPTER__


#include "emu.h"
#include "smsexp.h"
#include "bus/sega8/sega8_slot.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_gender_adapter_device

class sms_gender_adapter_device : public device_t,
							public device_sms_expansion_slot_interface
{
public:
	// construction/destruction
	sms_gender_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_sms_expansion_slot_interface overrides
	virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_mapper(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual int get_lphaser_xoffs() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<sega8_cart_slot_device> m_subslot;
};


// device type definition
extern const device_type SMS_GENDER_ADAPTER;


#endif
