#ifndef MAME_BUS_WAVEBLASTER_DB50XG_H
#define MAME_BUS_WAVEBLASTER_DB50XG_H

// Yamaha DB50XG

#pragma once

#include "waveblaster.h"
#include "cpu/h8/h83002.h"
#include "sound/swp00.h"

DECLARE_DEVICE_TYPE(DB50XG, db50xg_device)

class db50xg_device : public device_t, public device_waveblaster_interface
{
public:
	db50xg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~db50xg_device();

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

#endif // MAME_BUS_WAVEBLASTER_DB50XG_H
