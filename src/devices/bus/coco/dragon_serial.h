// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
#ifndef MAME_BUS_COCO_DRAGON_SERIAL_H
#define MAME_BUS_COCO_DRAGON_SERIAL_H

#pragma once

#include "cococart.h"
#include "machine/6850acia.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dragon_serial_device

class dragon_serial_device :
	public device_t,
	public device_cococart_interface
{
public:
	// construction/destruction
	dragon_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type imperfect_features() { return feature::COMMS; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual u8 *get_cart_base() override;
	virtual memory_region *get_cart_memregion() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual u8 cts_read(offs_t offset) override;
	virtual u8 scs_read(offs_t offset) override;
	virtual void scs_write(offs_t offset, u8 data) override;

private:
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);

	required_memory_region m_eprom;
	required_device<acia6850_device> m_acia;
};


// device type definition
DECLARE_DEVICE_TYPE(DRAGON_SERIAL, dragon_serial_device)

#endif // MAME_BUS_COCO_DRAGON_SERIAL_H
