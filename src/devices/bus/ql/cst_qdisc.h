// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CST QL Disc Interface emulation

**********************************************************************/

#ifndef MAME_BUS_QL_CST_QDISC_H
#define MAME_BUS_QL_CST_QDISC_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cst_ql_disc_interface_device

class cst_ql_disc_interface_device : public device_t,
								public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	cst_ql_disc_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
DECLARE_DEVICE_TYPE(CST_QL_DISC_INTERFACE, cst_ql_disc_interface_device)

#endif // MAME_BUS_QL_CST_QDISC_H
