#ifndef MAME_BUS_WAVEBLASTER_WG130_H
#define MAME_BUS_WAVEBLASTER_WG130_H

// Casio WG130

#pragma once

#include "waveblaster.h"
#include "cpu/h8/gt913.h"

DECLARE_DEVICE_TYPE(WG130, wg130_device)

class wg130_device : public device_t, public device_waveblaster_interface
{
public:
	wg130_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~wg130_device();

	virtual void midi_rx(int state) override;

protected:
	virtual void device_start() override;
	const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<gt913_device> m_gt913;

	void map(address_map &map);
};

#endif // MAME_BUS_WAVEBLASTER_WG130_H
