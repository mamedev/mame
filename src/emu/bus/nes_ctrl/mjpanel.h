// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Mahjong Panel

**********************************************************************/

#pragma once

#ifndef __NES_MJPANEL__
#define __NES_MJPANEL__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_mjpanel_device

class nes_mjpanel_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_mjpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual UINT8 read_exp(offs_t offset);
	virtual void write(UINT8 data);

private:
	required_ioport_array<4> m_panel;
	UINT32 m_latch;
};


// device type definition
extern const device_type NES_MJPANEL;


#endif
