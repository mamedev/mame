// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_MSX_WRITE_H
#define MAME_BUS_MSX_SLOT_MSX_WRITE_H

#pragma once

#include "slot.h"
#include "machine/nvram.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_MSX_WRITE, msx_slot_msx_write_device)


class msx_slot_msx_write_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_msx_write_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_rom_start(const char *region, u32 offset) { m_rom_region.set_tag(region); m_region_offset = offset; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	template <int Bank> void bank_w(u8 data);

	required_memory_region m_rom_region;
	required_ioport m_switch_port;
	memory_bank_array_creator<2> m_rombank;
	memory_view m_view[2];
	u32 m_region_offset;
	bool m_enabled;
};


#endif // MAME_BUS_MSX_SLOT_MSX_WRITE_H
