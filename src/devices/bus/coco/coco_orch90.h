// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#pragma once

#ifndef __COCO_ORCH90_H__
#define __COCO_ORCH90_H__

#include "sound/dac.h"
#include "cococart.h"

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
extern const device_type COCO_ORCH90;

#endif  /* __COCO_ORCH90_H__ */
