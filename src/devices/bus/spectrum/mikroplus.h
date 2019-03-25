// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Mikro-Plus Joystick Interface

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_MIKROPLUS_H
#define MAME_BUS_SPECTRUM_MIKROPLUS_H

#pragma once


#include "exp.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_mikroplus_device

class spectrum_mikroplus_device :
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_mikroplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual DECLARE_READ_LINE_MEMBER(romcs) override;
	virtual DECLARE_READ8_MEMBER(mreq_r) override;

private:
	DECLARE_READ8_MEMBER(joystick_r);

	required_memory_region m_rom;
	required_ioport m_joy;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_MIKROPLUS, spectrum_mikroplus_device)


#endif // MAME_BUS_SPECTRUM_MIKROPLUS_H
