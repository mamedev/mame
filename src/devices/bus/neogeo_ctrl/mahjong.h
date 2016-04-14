// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Mahjong controller emulation

**********************************************************************/

#pragma once

#ifndef __NEOGEO_MJCTRL__
#define __NEOGEO_MJCTRL__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> neogeo_mjctrl_device

class neogeo_mjctrl_device : public device_t,
							public device_neogeo_control_port_interface
{
public:
	// construction/destruction
	neogeo_mjctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_intv_control_port_interface overrides
	virtual UINT8 read_ctrl() override;
	virtual UINT8 read_start_sel() override;
	virtual void write_ctrlsel(UINT8 data) override;

private:
	required_ioport_array<4> m_mjpanel;
	required_ioport m_ss;
	UINT8 m_ctrl_sel;
};


// device type definition
extern const device_type NEOGEO_MJCTRL;


#endif
