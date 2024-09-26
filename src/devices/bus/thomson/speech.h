// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_THOMSON_SPEECH_H
#define MAME_BUS_THOMSON_SPEECH_H

#include "extension.h"

class thomson_speech_device : public device_t, public thomson_extension_interface
{
public:
	thomson_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void rom_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(THOMSON_SPEECH, thomson_speech_device)

#endif // MAME_BUS_THOMSON_SPEECH_H
