// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro Sideways ROM emulation

***************************************************************************/

#include "emu.h"
#include "rom.h"


namespace {

class bbc_rom_device : public device_t, public device_bbc_rom_interface
{
public:
	bbc_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_ROM, tag, owner, clock)
		, device_bbc_rom_interface(mconfig, *this)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		return get_rom_base()[offset & (get_rom_size() - 1)];
	}
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_ROM, device_bbc_rom_interface, bbc_rom_device, "bbc_rom", "BBC Micro Sideways ROM")
