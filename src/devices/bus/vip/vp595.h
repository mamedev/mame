// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Simple Sound Board VP595 emulation

**********************************************************************/

#ifndef MAME_BUS_VIP_VP595_H
#define MAME_BUS_VIP_VP595_H

#pragma once

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
	vp595_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_vip_expansion_card_interface overrides
	virtual void vip_io_w(offs_t offset, uint8_t data) override;
	virtual void vip_q_w(int state) override;

private:
	required_device<cdp1863_device> m_pfg;
};


// device type definition
DECLARE_DEVICE_TYPE(VP595, vp595_device)

#endif // MAME_BUS_VIP_VP595_H
