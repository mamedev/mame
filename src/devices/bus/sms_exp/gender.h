// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Gender Adapter" emulation

**********************************************************************/

#ifndef MAME_BUS_SMS_EXP_GENDER_H
#define MAME_BUS_SMS_EXP_GENDER_H

#pragma once


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
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
	virtual void write_mapper(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

	virtual int get_lphaser_xoffs() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<sega8_cart_slot_device> m_subslot;
};


// device type definition
DECLARE_DEVICE_TYPE(SMS_GENDER_ADAPTER, sms_gender_adapter_device)

#endif // MAME_BUS_SMS_EXP_GENDER_H
