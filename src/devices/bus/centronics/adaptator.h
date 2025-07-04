// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_CENTRONICS_ADAPTATOR_H
#define MAME_BUS_CENTRONICS_ADAPTATOR_H

#pragma once

#include "ctronics.h"
#include "bus/vcs_ctrl/ctrl.h"

class adaptator_multitap_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	adaptator_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_strobe(int state) override;
	virtual void input_data0(int state) override { if (state) m_ddr |= 0x01; else m_ddr &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_ddr |= 0x02; else m_ddr &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_ddr |= 0x04; else m_ddr &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_ddr |= 0x08; else m_ddr &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_ddr |= 0x10; else m_ddr &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_ddr |= 0x20; else m_ddr &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_ddr |= 0x40; else m_ddr &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_ddr |= 0x80; else m_ddr &= ~0x80; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device_array<vcs_control_port_device, 2> m_joy;

	u8 m_ddr;
};

// device type definition
DECLARE_DEVICE_TYPE(ADAPTATOR_MULTITAP, adaptator_multitap_device)


#endif // MAME_BUS_CENTRONICS_ADAPTATOR_H
