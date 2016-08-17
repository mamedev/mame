// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "pci-ac97.h"

const device_type AC97 = &device_creator<ac97_device>;

DEVICE_ADDRESS_MAP_START(native_audio_mixer_map, 32, ac97_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(native_audio_bus_mastering_map, 32, ac97_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(mixer_map, 32, ac97_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(bus_mastering_map, 32, ac97_device)
ADDRESS_MAP_END

ac97_device::ac97_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, AC97, "AC97 audio", tag, owner, clock, "ac97", __FILE__)
{
}

void ac97_device::device_start()
{
	pci_device::device_start();
	add_map(256, M_IO, FUNC(ac97_device::native_audio_mixer_map));
	add_map(64,  M_IO, FUNC(ac97_device::native_audio_bus_mastering_map));
	add_map(512, M_MEM, FUNC(ac97_device::mixer_map));
	add_map(256, M_MEM, FUNC(ac97_device::bus_mastering_map));
}

void ac97_device::device_reset()
{
	pci_device::device_reset();
}
