// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom & SNES SuperScope

**********************************************************************/

#ifndef MAME_BUS_SNES_CTRL_SUPERSCOPE_H
#define MAME_BUS_SNES_CTRL_SUPERSCOPE_H

#pragma once


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
	snes_sscope_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_snes_control_port_interface overrides
	virtual uint8_t read_pin4() override;
	virtual void write_strobe(uint8_t data) override;
	virtual void port_poll() override;

private:
	required_ioport m_buttons;
	required_ioport m_xaxis;
	required_ioport m_yaxis;
	int m_strobe, m_idx;
	uint32_t m_latch;

	int16_t m_x, m_y;
	int m_turbo_lock, m_pause_lock, m_fire_lock;
};


// device type definition
DECLARE_DEVICE_TYPE(SNES_SUPERSCOPE, snes_sscope_device)

#endif // MAME_BUS_SNES_CTRL_SUPERSCOPE_H
