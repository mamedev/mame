// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn Keyboard emulation

**********************************************************************/

#pragma once

#ifndef __SATURN_KEYBD__
#define __SATURN_KEYBD__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saturn_keybd_device

class saturn_keybd_device : public device_t,
							public device_saturn_control_port_interface
{
public:
	// construction/destruction
	saturn_keybd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_saturn_control_port_interface overrides
	virtual UINT8 read_ctrl(UINT8 offset) override;
	virtual UINT8 read_status() override { return 0xf1; }
	virtual UINT8 read_id(int idx) override { return m_ctrl_id; }

private:
	UINT8 m_status;
	UINT8 m_data;
	UINT8 m_prev_data;
	UINT16 m_repeat_count;

	UINT16 get_game_key();

	required_ioport_array<16> m_key;
	required_ioport m_key_s1;
};

// device type definition
extern const device_type SATURN_KEYBD;


#endif
