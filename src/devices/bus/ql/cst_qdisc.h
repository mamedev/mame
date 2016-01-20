// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CST QL Disc Interface emulation

**********************************************************************/

#pragma once

#ifndef __CST_QL_DISC_INTERFACE__
#define __CST_QL_DISC_INTERFACE__

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cst_ql_disc_interface_t

class cst_ql_disc_interface_t : public device_t,
								public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	cst_ql_disc_interface_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_ql_expansion_card_interface overrides
	virtual UINT8 read(address_space &space, offs_t offset, UINT8 data) override;
	virtual void write(address_space &space, offs_t offset, UINT8 data) override;
};



// device type definition
extern const device_type CST_QL_DISC_INTERFACE;



#endif
