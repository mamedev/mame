// license:BSD-3-Clause
/**********************************************************************

    Atari CX22/CX80 Trak-Ball

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_TRAKBALL_H
#define MAME_BUS_VCS_CTRL_TRAKBALL_H

#pragma once

#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> atari_trakball_device

class atari_trakball_device : public device_t,
							public device_vcs_control_port_interface
{
public:
	// construction/destruction
	atari_trakball_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	DECLARE_INPUT_CHANGED_MEMBER( trakball_moved );

protected:
	// device_t implementation
	virtual void device_start() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	// device_vcs_control_port_interface overrides
	virtual u8 vcs_joy_r() override;

private:
	required_ioport m_trakballb;
	required_ioport_array<2> m_trakballxy;

	void trakball_pos_and_dir_upd(int axis);

	uint32_t m_last_pos[2];
	uint8_t m_last_direction[2];
};


// device type declaration
DECLARE_DEVICE_TYPE(ATARI_TRAKBALL, atari_trakball_device)

#endif // MAME_BUS_VCS_CTRL_TRAKBALL_H
