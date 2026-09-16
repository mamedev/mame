// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    OPD Basic Master emulation

**********************************************************************/

#ifndef MAME_BUS_QL_OPD_BASIC_MASTER_H
#define MAME_BUS_QL_OPD_BASIC_MASTER_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> opd_basic_master_device

class opd_basic_master_device : public device_t, public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	opd_basic_master_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_ql_expansion_card_interface overrides
	virtual uint8_t read(offs_t offset, uint8_t data) override;
	virtual void write(offs_t offset, uint8_t data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(OPD_BASIC_MASTER, opd_basic_master_device)

#endif // MAME_BUS_QL_OPD_BASIC_MASTER_H
