// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_BUS_ATA_FX5400W_H
#define MAME_BUS_ATA_FX5400W_H

#pragma once

#include "atapicdr.h"

class mitsumi_fx5400w_device : public atapi_cdrom_device
{
public:
	mitsumi_fx5400w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;

	virtual bool is_ready() override { return true; }
};

DECLARE_DEVICE_TYPE(FX5400W, mitsumi_fx5400w_device)

#endif // MAME_BUS_ATA_FX5400W_H
