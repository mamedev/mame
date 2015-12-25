// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 RAM Card emulation

**********************************************************************/

#pragma once

#ifndef __COMX_RAM__
#define __COMX_RAM__

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_ram_device

class comx_ram_device : public device_t,
						public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_comx_expansion_card_interface overrides
	virtual UINT8 comx_mrd_r(address_space &space, offs_t offset, int *extrom) override;
	virtual void comx_mwr_w(address_space &space, offs_t offset, UINT8 data) override;
	virtual void comx_io_w(address_space &space, offs_t offset, UINT8 data) override;

private:
	optional_shared_ptr<UINT8> m_ram;

	int m_bank;
};


// device type definition
extern const device_type COMX_RAM;


#endif
