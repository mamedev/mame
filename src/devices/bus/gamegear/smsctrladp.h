// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Game Gear "SMS Controller Adaptor" emulation
    Also known as "Master Link" cable.

**********************************************************************/

#ifndef MAME_BUS_GAMEGEAR_SMSCRTLADP_H
#define MAME_BUS_GAMEGEAR_SMSCRTLADP_H

#pragma once


#include "ggext.h"
#include "bus/sms_ctrl/smsctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_ctrl_adaptor_device

class sms_ctrl_adaptor_device : public device_t,
							public device_gg_ext_port_interface
{
public:
	// construction/destruction
	sms_ctrl_adaptor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(th_pin_w);
	DECLARE_READ32_MEMBER(pixel_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_gg_ext_port_interface overrides
	virtual uint8_t peripheral_r() override;
	virtual void peripheral_w(uint8_t data) override;

private:
	required_device<sms_control_port_device> m_subctrl_port;
};


// device type definition
DECLARE_DEVICE_TYPE(SMS_CTRL_ADAPTOR, sms_ctrl_adaptor_device)


#endif // MAME_BUS_GAMEGEAR_SMSCRTLADP_H
