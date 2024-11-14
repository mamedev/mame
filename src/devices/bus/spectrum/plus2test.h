// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Spectrum +2 Test Software

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_PLUS2TEST_H
#define MAME_BUS_SPECTRUM_PLUS2TEST_H

#pragma once


#include "exp.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_plus2test_device

class spectrum_plus2test_device :
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_plus2test_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual bool romcs() override;
	virtual uint8_t mreq_r(offs_t offset) override;

private:
	required_memory_region m_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_PLUS2TEST, spectrum_plus2test_device)


#endif // MAME_BUS_SPECTRUM_PLUS2TEST_H
