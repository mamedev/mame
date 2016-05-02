// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn SegaTap / Team Play emulation

**********************************************************************/

#pragma once

#ifndef __SATURN_SEGATAP__
#define __SATURN_SEGATAP__


#include "emu.h"
#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saturn_segatap_device

class saturn_segatap_device : public device_t,
							public device_saturn_control_port_interface
{
public:
	// construction/destruction
	saturn_segatap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	
	// device_saturn_control_port_interface overrides
	virtual UINT8 read_ctrl(UINT8 offset) override;
	virtual UINT8 read_status() override { return 0x04; }
	virtual UINT8 read_id(int idx) override;

private:
	required_device<saturn_control_port_device> m_subctrl1_port;
	required_device<saturn_control_port_device> m_subctrl2_port;
	required_device<saturn_control_port_device> m_subctrl3_port;
	required_device<saturn_control_port_device> m_subctrl4_port;
};


// device type definition
extern const device_type SATURN_SEGATAP;


#endif
