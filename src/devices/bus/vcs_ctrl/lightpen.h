// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System lightpen emulation

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_LIGHTPEN_H
#define MAME_BUS_VCS_CTRL_LIGHTPEN_H

#pragma once

#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vcs_lightpen_device

class vcs_lightpen_device : public device_t,
							public device_vcs_control_port_interface
{
public:
	// construction/destruction
	vcs_lightpen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_vcs_control_port_interface overrides
	virtual uint8_t vcs_joy_r() override;

	DECLARE_INPUT_CHANGED_MEMBER( trigger );

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_joy;
	required_ioport m_lightx;
	required_ioport m_lighty;
};


// device type definition
DECLARE_DEVICE_TYPE(VCS_LIGHTPEN, vcs_lightpen_device)

#endif // MAME_BUS_VCS_CTRL_LIGHTPEN_H
