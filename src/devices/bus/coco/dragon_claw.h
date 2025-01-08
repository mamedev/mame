// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
#ifndef MAME_BUS_COCO_DRAGON_CLAW_H
#define MAME_BUS_COCO_DRAGON_CLAW_H

#pragma once

#include "cococart.h"
#include "machine/6522via.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dragon_claw_device

class dragon_claw_device :
	public device_t,
	public device_cococart_interface,
	public device_cococart_host_interface
{
public:
	// construction/destruction
	dragon_claw_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config& config) override;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u8 cts_read(offs_t offset) override;
	virtual void cts_write(offs_t offset, u8 data) override;
	virtual u8 scs_read(offs_t offset) override;
	virtual void scs_write(offs_t offset, u8 data) override;
	virtual void set_sound_enable(bool sound_enable) override;

	virtual u8 *get_cart_base() override;
	virtual u32 get_cart_size() override;

	virtual address_space &cartridge_space() override;

private:
	void irq_w(int state);

	required_device<via6522_device> m_via;
	required_device<cococart_slot_device> m_slot;
	required_ioport m_links;
};


// device type definition
DECLARE_DEVICE_TYPE(DRAGON_CLAW, device_cococart_interface)

#endif // MAME_BUS_COCO_DRAGON_CLAW_H
