// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Kizuna 4Players controller emulation

**********************************************************************/

#pragma once

#ifndef __NEOGEO_KIZ4P__
#define __NEOGEO_KIZ4P__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> neogeo_kizuna4p_device

class neogeo_kizuna4p_device : public device_t,
						public device_neogeo_control_port_interface
{
public:
	// construction/destruction
	neogeo_kizuna4p_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	
	// device_neogeo_control_port_interface overrides
	virtual UINT8 read_ctrl() override;
	virtual UINT8 read_start_sel() override;
	virtual void write_ctrlsel(UINT8 data) override;
	
private:
	required_ioport m_joy1;
	required_ioport m_joy2;
	required_ioport m_ss;
	UINT8 m_ctrl_sel;
};



// device type definition
extern const device_type NEOGEO_KIZ4P;


#endif
