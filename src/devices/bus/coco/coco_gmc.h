// license:BSD-3-Clause
// copyright-holders:tim lindner
#ifndef MAME_DEVICES_BUS_COCO_COCO_GMC_H
#define MAME_DEVICES_BUS_COCO_COCO_GMC_H

#pragma once

#include "coco_pak.h"
#include "sound/sn76496.h"

// ======================> coco_pak_banked_device

class coco_pak_gmc_device :
		public coco_pak_banked_device
{
public:
		// construction/destruction
		coco_pak_gmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
		virtual machine_config_constructor device_mconfig_additions() const override;

protected:
		// device-level overrides
		virtual DECLARE_WRITE8_MEMBER(write) override;
private:
		required_device<sn76489a_device> m_psg;
};


// device type definition
DECLARE_DEVICE_TYPE(COCO_PAK_GMC, coco_pak_gmc_device)

#endif  // MAME_DEVICES_BUS_COCO_COCO_GMC_H
