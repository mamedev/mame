// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Konami 573 Memory Card Reader
 *
 */

#pragma once

#ifndef __K573MCR_H__
#define __K573MCR_H__

#include "emu.h"

extern const device_type KONAMI_573_MEMORY_CARD_READER;

class k573mcr_device : public device_t
{
public:
	k573mcr_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;

	virtual const rom_entry *device_rom_region() const override;
};

#endif
