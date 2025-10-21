// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_CBUS_WAVESTAR_H
#define MAME_BUS_CBUS_WAVESTAR_H

#pragma once

#include "pc9801_cbus.h"

#include "cpu/h8/h83042.h"

class qvision_wavestar_device : public device_t
{
public:
	qvision_wavestar_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND | feature::MICROPHONE; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void h8_map(address_map &map) ATTR_COLD;

	required_device<pc9801_slot_device> m_bus;
	required_device<h83040_device> m_cpu;
};


DECLARE_DEVICE_TYPE(QVISION_WAVESTAR,  qvision_wavestar_device)


#endif // MAME_BUS_CBUS_WAVESTAR_H
