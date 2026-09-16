// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SOUND_PCI_AC97_H
#define MAME_SOUND_PCI_AC97_H

#pragma once

#include "machine/pci.h"

class ac97_device : public pci_device {
public:
	ac97_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t main_id, uint32_t revision, uint32_t subdevice_id)
		: ac97_device(mconfig, tag, owner, clock)
	{
		set_ids(main_id, revision, 0x040300, subdevice_id);
	}
	ac97_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void native_audio_mixer_map(address_map &map) ATTR_COLD;
	void native_audio_bus_mastering_map(address_map &map) ATTR_COLD;
	void mixer_map(address_map &map) ATTR_COLD;
	void bus_mastering_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(AC97, ac97_device)

#endif // MAME_SOUND_PCI_AC97_H
