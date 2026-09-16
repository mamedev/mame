// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Konami 573 Network PCB Unit
 *
 */
#ifndef MAME_BUS_PCCARD_K573NPU_H
#define MAME_BUS_PCCARD_K573NPU_H

#pragma once

#include "pccard.h"

class k573npu_device : public device_t,
	public device_pccard_interface
{
public:
	k573npu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void maincpu_program_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(KONAMI_573_NETWORK_PCB_UNIT, k573npu_device)

#endif // MAME_BUS_PCCARD_K573NPU_H
