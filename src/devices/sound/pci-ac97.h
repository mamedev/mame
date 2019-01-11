// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SOUND_PCI_AC97_H
#define MAME_SOUND_PCI_AC97_H

#pragma once

#include "machine/pci.h"

#define MCFG_AC97_ADD(_tag, _main_id, _revision, _subdevice_id) \
		MCFG_PCI_DEVICE_ADD(_tag, AC97, _main_id, _revision, 0x040300, _subdevice_id)

class ac97_device : public pci_device {
public:
	ac97_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void native_audio_mixer_map(address_map &map);
	void native_audio_bus_mastering_map(address_map &map);
	void mixer_map(address_map &map);
	void bus_mastering_map(address_map &map);
};

DECLARE_DEVICE_TYPE(AC97, ac97_device)

#endif // MAME_SOUND_PCI_AC97_H
