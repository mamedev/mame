// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller

#ifndef MAME_CPU_UPD7810_UPD7811_H
#define MAME_CPU_UPD7810_UPD7811_H

#pragma once

#include "upd7810.h"

DECLARE_DEVICE_TYPE(UPD7811,  upd7811_device)
DECLARE_DEVICE_TYPE(UPD78C11, upd78c11_device)

class upd7811_device : public upd7810_device
{
public:
	// construction/destruction
	upd7811_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class upd78c11_device : public upd78c10_device
{
public:
	// construction/destruction
	upd78c11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_CPU_UPD7810_UPD7811_H
