// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_CENTRONICS_MZ1P16_H
#define MAME_BUS_CENTRONICS_MZ1P16_H

#pragma once

#include "ctronics.h"
#include "machine/buffer.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mz1p16_device

class mz1p16_device : public device_t, public device_centronics_peripheral_interface
{
public:
	// device type constructor
	mz1p16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	void reset_sw_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_centronics_peripheral_interface implementation
	virtual void input_data0(int state) override;
	virtual void input_data1(int state) override;
	virtual void input_data2(int state) override;
	virtual void input_data3(int state) override;
	virtual void input_data4(int state) override;
	virtual void input_data5(int state) override;
	virtual void input_data6(int state) override;
	virtual void input_data7(int state) override;
	virtual void input_strobe(int state) override;
	virtual void input_init(int state) override;
	virtual bool supports_pin35_5v() override { return false; }

private:
	void xy_step_w(u8 data);
	void control_w(u8 data);

	required_device<cpu_device> m_mcu;
	required_device<input_buffer_device> m_buffer;
};

// device type declaration
DECLARE_DEVICE_TYPE(MZ1P16, mz1p16_device)

#endif // MAME_BUS_CENTRONICS_MZ1P16_H
