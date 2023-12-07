#ifndef MAME_BUS_WAVEBLASTER_OMNIWAVE_H
#define MAME_BUS_WAVEBLASTER_OMNIWAVE_H

// Samsung Omniwave

#pragma once

#include "waveblaster.h"
#include "sound/ks0164.h"

DECLARE_DEVICE_TYPE(OMNIWAVE, omniwave_device)

class omniwave_device : public device_t, public device_waveblaster_interface
{
public:
	omniwave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~omniwave_device();

	virtual void midi_rx(int state) override;

protected:
	virtual void device_start() override;
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<ks0164_device> m_ks0164;
};

#endif // MAME_BUS_WAVEBLASTER_OMNIWAVE_H
