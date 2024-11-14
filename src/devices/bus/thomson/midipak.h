// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_THOMSON_MIDIPAK_H
#define MAME_BUS_THOMSON_MIDIPAK_H

#include "extension.h"

class logimus_midipak_device : public device_t, public thomson_extension_interface
{
public:
	logimus_midipak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void rom_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(LOGIMUS_MIDIPAK, logimus_midipak_device)

#endif // MAME_BUS_THOMSON_MIDIPAK_H
