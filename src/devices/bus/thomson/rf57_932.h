// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_THOMSON_RF57_932_H
#define MAME_BUS_THOMSON_RF57_932_H

#include "extension.h"

class rf57_932_device : public device_t, public thomson_extension_interface
{
public:
	rf57_932_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void rom_map(address_map &map) override;
	virtual void io_map(address_map &map) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
};

// device type declaration
DECLARE_DEVICE_TYPE(RF57_932, rf57_932_device)

#endif // MAME_BUS_THOMSON_RF57_932_H
