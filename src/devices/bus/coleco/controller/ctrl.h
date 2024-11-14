// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ColecoVision controller port emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_COLECO_CTRL_H
#define MAME_BUS_COLECO_CTRL_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class colecovision_control_port_device;


// ======================> device_colecovision_control_port_interface

class device_colecovision_control_port_interface : public device_interface
{
public:
	virtual uint8_t joy_r() { return 0xff; }
	virtual void common0_w(int state) { m_common0 = state; }
	virtual void common1_w(int state) { m_common1 = state; }

protected:
	// construction/destruction
	device_colecovision_control_port_interface(const machine_config &mconfig, device_t &device);

	colecovision_control_port_device *m_port;

	int m_common0;
	int m_common1;
};


// ======================> colecovision_control_port_device

class colecovision_control_port_device : public device_t,
								public device_single_card_slot_interface<device_colecovision_control_port_interface>
{
public:
	// construction/destruction
	template <typename T>
	colecovision_control_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: colecovision_control_port_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	colecovision_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	auto irq() { return m_write_irq.bind(); }

	// computer interface
	uint8_t read() { uint8_t data = 0xff; if (exists()) data = m_device->joy_r(); return data; }

	void common0_w(int state) { if (exists()) m_device->common0_w(state); }
	void common1_w(int state) { if (exists()) m_device->common1_w(state); }

	bool exists() { return m_device != nullptr; }

	void irq_w(int state) { m_write_irq(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_colecovision_control_port_interface *m_device;

private:
	devcb_write_line m_write_irq;
};


// device type definition
DECLARE_DEVICE_TYPE(COLECOVISION_CONTROL_PORT, colecovision_control_port_device)

void colecovision_control_port_devices(device_slot_interface &device);


#endif // MAME_BUS_COLECO_CTRL_H
