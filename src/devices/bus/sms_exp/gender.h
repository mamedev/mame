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
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;
	virtual DECLARE_WRITE8_MEMBER(write_mapper) override;
	virtual DECLARE_READ8_MEMBER(read_ram) override;
	virtual DECLARE_WRITE8_MEMBER(write_ram) override;

	virtual int get_lphaser_xoffs() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<sega8_cart_slot_device> m_subslot;
};


// device type definition
DECLARE_DEVICE_TYPE(SMS_GENDER_ADAPTER, sms_gender_adapter_device)

#endif // MAME_BUS_SMS_EXP_GENDER_H
