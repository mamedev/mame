// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Zapper Lightgun

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_ZAPPER
#define MAME_BUS_NES_CTRL_ZAPPER

#pragma once


#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_zapper_device

class nes_zapper_device : public device_t,
							public device_video_interface,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_zapper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_bit34() override;
	virtual uint8_t read_exp(offs_t offset) override;

private:
	required_ioport m_lightx;
	required_ioport m_lighty;
	required_ioport m_trigger;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_ZAPPER, nes_zapper_device)

#endif // MAME_BUS_NES_CTRL_ZAPPER
