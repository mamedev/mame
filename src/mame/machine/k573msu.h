// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Konami 573 Multi Session Unit
 *
 */

#pragma once

#ifndef __K573MSU_H__
#define __K573MSU_H__

#include "emu.h"

extern const device_type KONAMI_573_MULTI_SESSION_UNIT;

class k573msu_device : public device_t
{
public:
	k573msu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;

	virtual const rom_entry *device_rom_region() const override;
};

#endif
