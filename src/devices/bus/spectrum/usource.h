// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Spectrum Currah MicroSource emulation

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_USOURCE_H
#define MAME_BUS_SPECTRUM_USOURCE_H

#pragma once


#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_usource_device

class spectrum_usource_device :
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_usource_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual bool romcs() override;
	virtual uint8_t mreq_r(offs_t offset) override;

private:
	required_memory_region m_rom;

	bool m_romcs;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_USOURCE, spectrum_usource_device)


#endif // MAME_BUS_SPECTRUM_USOURCE_H
