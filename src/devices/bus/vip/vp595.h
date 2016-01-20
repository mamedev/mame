// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Simple Sound Board VP595 emulation

**********************************************************************/

#pragma once

#ifndef __VP595__
#define __VP595__

#include "emu.h"
#include "exp.h"
#include "sound/cdp1863.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vp595_device

class vp595_device : public device_t,
						public device_vip_expansion_card_interface
{
public:
	// construction/destruction
	vp595_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_vip_expansion_card_interface overrides
	virtual void vip_io_w(address_space &space, offs_t offset, UINT8 data) override;
	virtual void vip_q_w(int state) override;

private:
	required_device<cdp1863_device> m_pfg;
};


// device type definition
extern const device_type VP595;


#endif
