// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Henson CFA3000 Analogue

**********************************************************************/


#ifndef MAME_BUS_BBC_ANALOGUE_CFA3000A_H
#define MAME_BUS_BBC_ANALOGUE_CFA3000A_H

#pragma once

#include "analogue.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cfa3000_anlg_device

class cfa3000_anlg_device :
	public device_t,
	public device_bbc_analogue_interface
{
public:
	// construction/destruction
	cfa3000_anlg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t ch_r(int channel) override;
	virtual uint8_t pb_r() override;

private:
	required_ioport_array<4> m_channel;
	required_ioport m_buttons;
};


// device type definition
DECLARE_DEVICE_TYPE(CFA3000_ANLG, cfa3000_anlg_device)


#endif // MAME_BUS_BBC_ANALOGUE_CFA3000A_H
