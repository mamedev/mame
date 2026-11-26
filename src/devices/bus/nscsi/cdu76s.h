// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
#ifndef MAME_BUS_NSCSI_CDU76S_H
#define MAME_BUS_NSCSI_CDU76S_H

#pragma once

#include "bus/nscsi/cd.h"

class nscsi_cdu76s_device : public nscsi_cdrom_device
{
public:
	nscsi_cdu76s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(NSCSI_CDU76S, nscsi_cdu76s_device)

#endif // MAME_BUS_NSCSI_CDU76S_H
