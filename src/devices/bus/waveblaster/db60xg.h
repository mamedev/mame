#ifndef MAME_BUS_WAVEBLASTER_DB60XG_H
#define MAME_BUS_WAVEBLASTER_DB60XG_H

// Yamaha DB60XG / NEC XR385

#pragma once

#include "waveblaster.h"
#include "cpu/h8/h83002.h"
#include "sound/swp00.h"

DECLARE_DEVICE_TYPE(DB60XG, db60xg_device)

class db60xg_device : public device_t, public device_waveblaster_interface
{
public:
	db60xg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~db60xg_device();

	virtual void midi_rx(int state) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<h83002_device> m_cpu;
	required_device<swp00_device> m_swp00;

	void map(address_map &map);
};

#endif // MAME_BUS_WAVEBLASTER_DB60XG_H
