// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Dempa Shinbunsha Joystick

***************************************************************************/

#ifndef __CENTRONICS_DSJOY_H__
#define __CENTRONICS_DSJOY_H__

#pragma once

#include "ctronics.h"

class dempa_shinbunsha_joystick_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	dempa_shinbunsha_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual ioport_constructor device_input_ports() const;

	virtual DECLARE_WRITE_LINE_MEMBER( input_data0 ) { if (state) m_data |= 0x01; else m_data &= ~0x01; update_perror(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data1 ) { if (state) m_data |= 0x02; else m_data &= ~0x02; update_perror(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data2 ) { if (state) m_data |= 0x04; else m_data &= ~0x04; update_perror(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data3 ) { if (state) m_data |= 0x08; else m_data &= ~0x08; update_perror(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data4 ) { if (state) m_data |= 0x10; else m_data &= ~0x10; update_perror(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data5 ) { if (state) m_data |= 0x20; else m_data &= ~0x20; update_perror(); }

private:
	required_ioport m_lptjoy;

	void update_perror();

	UINT8 m_data;
	int m_perror;
};

// device type definition
extern const device_type DEMPA_SHINBUNSHA_JOYSTICK;

#endif
