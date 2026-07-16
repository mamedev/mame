// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_PC9801_03_H
#define MAME_BUS_PC98_CBUS_PC9801_03_H

#pragma once

#include "slot.h"

#include "imagedev/cassette.h"
#include "machine/i8251.h"

class pc9801_03_device : public device_t
					   , public device_pc98_cbus_slot_interface
{
public:
	pc9801_03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::TAPE; }

protected:
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	void io_map(address_map &map) ATTR_COLD;

	void control_port_w(offs_t offset, u8 data);

	required_device<i8251_device> m_uart;
	required_device<cassette_image_device> m_cassette;
};

// device type definition
DECLARE_DEVICE_TYPE(PC9801_03, pc9801_03_device)

#endif // MAME_BUS_PC98_CBUS_PC9801_03_H
