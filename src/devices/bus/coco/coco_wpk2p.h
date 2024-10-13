// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
#ifndef MAME_BUS_COCO_COCO_WPK2P_H
#define MAME_BUS_COCO_COCO_WPK2P_H

#pragma once

#include "cococart.h"
#include "video/v9938.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_wpk2p_device

class coco_wpk2p_device :
	public device_t,
	public device_cococart_interface
{
public:
	// construction/destruction
	coco_wpk2p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<v9958_device> m_v9958;
};


// device type definition
DECLARE_DEVICE_TYPE(COCO_WPK2P, coco_wpk2p_device)

#endif // MAME_BUS_COCO_COCO_WPK2P_H
