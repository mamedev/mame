// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    First Byte Printer Interface

**********************************************************************/


#ifndef MAME_BUS_ELECTRON_FBPRINT_H
#define MAME_BUS_ELECTRON_FBPRINT_H

#include "exp.h"
#include "bus/centronics/ctronics.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_fbprint_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_fbprint_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;

	int m_centronics_busy;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_FBPRINT, electron_fbprint_device)


#endif /* MAME_BUS_ELECTRON_FBPRINT_H */
