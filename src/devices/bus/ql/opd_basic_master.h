// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    OPD Basic Master emulation

**********************************************************************/

#pragma once

#ifndef __OPD_BASIC_MASTER__
#define __OPD_BASIC_MASTER__

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> opd_basic_master_t

class opd_basic_master_t : public device_t,
							public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	opd_basic_master_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	opd_basic_master_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int ram_size);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();

	// device_ql_expansion_card_interface overrides
	virtual UINT8 read(address_space &space, offs_t offset, UINT8 data);
	virtual void write(address_space &space, offs_t offset, UINT8 data);
};



// device type definition
extern const device_type OPD_BASIC_MASTER;



#endif
