// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Archimedes Econet Module

**********************************************************************/


#ifndef MAME_BUS_ARCHIMEDES_ECONET_ECONET_H
#define MAME_BUS_ARCHIMEDES_ECONET_ECONET_H

#pragma once


#include "slot.h"
#include "machine/mc6854.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> arc_econet_adf10_device

class arc_econet_device:
	public device_t,
	public device_archimedes_econet_interface
{
public:
	// construction/destruction
	arc_econet_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

private:
	required_device<mc6854_device> m_adlc;
};



// device type definition
DECLARE_DEVICE_TYPE(ARC_ECONET, arc_econet_device);


#endif /* MAME_BUS_ARCHIMEDES_ECONET_ECONET_H */
