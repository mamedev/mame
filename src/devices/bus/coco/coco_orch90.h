// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#ifndef MAME_BUS_COCO_COCO_ORCH90_H
#define MAME_BUS_COCO_COCO_ORCH90_H

#pragma once

#include "cococart.h"
#include "sound/dac.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_orch90_device

class coco_orch90_device :
		public device_t,
		public device_cococart_interface
{
public:
		// construction/destruction
		coco_orch90_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual DECLARE_WRITE8_MEMBER(write) override;
private:
		// internal state
		required_device<dac_byte_interface> m_ldac;
		required_device<dac_byte_interface> m_rdac;
};


// device type definition
DECLARE_DEVICE_TYPE(COCO_ORCH90, coco_orch90_device)

#endif // MAME_BUS_COCO_COCO_ORCH90_H
