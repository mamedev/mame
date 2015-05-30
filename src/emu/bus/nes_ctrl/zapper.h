// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Zapper Lightgun

**********************************************************************/

#pragma once

#ifndef __NES_ZAPPER__
#define __NES_ZAPPER__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_zapper_device

class nes_zapper_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_zapper_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual UINT8 read_bit34();
	virtual UINT8 read_exp(offs_t offset);

private:
	required_ioport m_lightx;
	required_ioport m_lighty;
	required_ioport m_trigger;
};


// device type definition
extern const device_type NES_ZAPPER;


#endif
