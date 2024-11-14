// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_CENTRONICS_NLQ401_H
#define MAME_BUS_CENTRONICS_NLQ401_H

#pragma once

#include "ctronics.h"
#include "machine/tms1024.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nlq401_device

class nlq401_device : public device_t, public device_centronics_peripheral_interface
{
public:
	// device type constructor
	nlq401_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_centronics_peripheral_interface overrides
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
	virtual bool supports_pin35_5v() override { return true; }

private:
	void expander_w(u8 data);
	u8 expander_r();

	void mem_map(address_map &map) ATTR_COLD;

	required_device<tms1025_device> m_inpexp;
	required_device<tms1025_device> m_outexp;
};

// device type declaration
DECLARE_DEVICE_TYPE(NLQ401, nlq401_device)

#endif // MAME_BUS_CENTRONICS_NLQ401_H
