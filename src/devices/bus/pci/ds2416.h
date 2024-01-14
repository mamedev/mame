// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_SOUND_DS2416_H
#define MAME_SOUND_DS2416_H

#pragma once

#include "ymp21.h"

class ds2416_device : public ymp21_device {
public:
	ds2416_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

DECLARE_DEVICE_TYPE(DS2416, ds2416_device)

#endif
