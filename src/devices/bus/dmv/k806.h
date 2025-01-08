// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
#ifndef MAME_BUS_DMV_K806_H
#define MAME_BUS_DMV_K806_H

#pragma once

#include "dmvbus.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/timer.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmv_k806_device

class dmv_k806_device :
		public device_t,
		public device_dmvslot_interface
{
public:
	// construction/destruction
	dmv_k806_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void io_read(int ifsel, offs_t offset, uint8_t &data) override;
	virtual void io_write(int ifsel, offs_t offset, uint8_t data) override;

private:
	int portt1_r();
	uint8_t port1_r();
	void port2_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(mouse_timer);

	required_device<upi41_cpu_device> m_mcu;
	required_ioport m_jumpers;
	required_ioport m_mouse_buttons;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;

	struct
	{
		int phase;
		int x;
		int y;
		int prev_x;
		int prev_y;
		int xa;
		int xb;
		int ya;
		int yb;
	} m_mouse;
};


// device type definition
DECLARE_DEVICE_TYPE(DMV_K806, dmv_k806_device)

#endif // MAME_BUS_DMV_K806_H
