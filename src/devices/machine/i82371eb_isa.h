// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_I82371EB_ISA_H
#define MAME_MACHINE_I82371EB_ISA_H

#pragma once

#include "machine/i82371sb.h"

class i82371eb_isa_device : public i82371sb_isa_device
{
public:
	template <typename T>
	i82371eb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: i82371eb_isa_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	i82371eb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual void config_map(address_map &map) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(I82371EB_ISA, i82371eb_isa_device)

#endif // MAME_MACHINE_I82371EB_ISA_H
