// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom & SNES SuperScope

**********************************************************************/

#pragma once

#ifndef __SNES_SUPERSCOPE__
#define __SNES_SUPERSCOPE__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> snes_sscope_device

class snes_sscope_device : public device_t,
							public device_snes_control_port_interface
{
public:
	// construction/destruction
	snes_sscope_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_sms_control_port_interface overrides
	virtual UINT8 read_pin4() override;
	virtual void write_strobe(UINT8 data) override;
	virtual void port_poll() override;

private:
	required_ioport m_buttons;
	required_ioport m_xaxis;
	required_ioport m_yaxis;
	int m_strobe, m_idx;
	UINT32 m_latch;

	INT16 m_x, m_y;
	int m_turbo_lock, m_pause_lock, m_fire_lock;
};


// device type definition
extern const device_type SNES_SUPERSCOPE;


#endif
