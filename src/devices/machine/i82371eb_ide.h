// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_I82371EB_IDE_H
#define MAME_MACHINE_I82371EB_IDE_H

#pragma once

#include "machine/i82371sb.h"

class i82371eb_ide_device : public i82371sb_ide_device
{
public:
	template <typename T>
	i82371eb_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: i82371eb_ide_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	i82371eb_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(I82371EB_IDE, i82371eb_ide_device)

#endif // MAME_MACHINE_I82371EB_IDE_H
