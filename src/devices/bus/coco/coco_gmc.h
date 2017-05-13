// license:BSD-3-Clause
// copyright-holders:tim lindner
#pragma once

#ifndef __COCO_GMC_H__
#define __COCO_GMC_H__

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
extern const device_type COCO_PAK_GMC;
#endif  /* __COCO_GMC_H__ */
