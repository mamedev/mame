// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro Sideways RAM emulation

***************************************************************************/

#include "emu.h"
#include "ram.h"


namespace {

class bbc_ram_device : public device_t, public device_bbc_rom_interface
{
public:
	bbc_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_RAM, tag, owner, clock)
		, device_bbc_rom_interface(mconfig, *this)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		return get_ram_base()[offset & (get_ram_size() - 1)];
	}

	virtual void write(offs_t offset, uint8_t data) override
	{
		get_ram_base()[offset & (get_ram_size() - 1)] = data;
	}
};


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_ram_device::device_start()
{
	ram_alloc(0x8000);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_RAM, device_bbc_rom_interface, bbc_ram_device, "bbc_ram", "BBC Micro Sideways RAM")
