// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtanic Video 80/82 Board

**********************************************************************/


#ifndef MAME_BUS_TANBUS_TUG8082_H
#define MAME_BUS_TANBUS_TUG8082_H

#pragma once

#include "bus/tanbus/tanbus.h"
#include "cpu/m6502/m6502.h"
#include "machine/i8212.h"

#include "emupal.h"
#include "screen.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_tug8082_device :
	public device_t,
	public device_tanbus_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	tanbus_tug8082_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

private:
	void bus_irq_w(int state);
	void vdu_irq_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device_array<i8212_device, 2> m_iop;
	required_ioport_array<2> m_dips;
	required_memory_region m_rom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;

	void vid8082_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_TUG8082, tanbus_tug8082_device)


#endif // MAME_BUS_TANBUS_TUG8082_H
