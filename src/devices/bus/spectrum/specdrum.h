// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cheetah Marketing SpecDrum emulation

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_SPECDRUM_H
#define MAME_BUS_SPECTRUM_SPECDRUM_H

#pragma once


#include "exp.h"
#include "sound/dac.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_specdrum_device

class spectrum_specdrum_device :
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_specdrum_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void iorq_w(offs_t offset, uint8_t data) override;

private:
	required_device<dac_byte_interface> m_dac;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_SPECDRUM, spectrum_specdrum_device)


#endif // MAME_BUS_SPECTRUM_SPECDRUM_H
