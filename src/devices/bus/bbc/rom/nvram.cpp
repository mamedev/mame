// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro Sideways RAM (Battery Backup) emulation

***************************************************************************/

#include "emu.h"
#include "nvram.h"


namespace {

class bbc_nvram_device : public device_t, public device_bbc_rom_interface
{
public:
	bbc_nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_NVRAM, tag, owner, clock)
		, device_bbc_rom_interface(mconfig, *this)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		return get_nvram_base()[offset & (get_nvram_size() - 1)];
	}

	virtual void write(offs_t offset, uint8_t data) override
	{
		get_nvram_base()[offset & (get_nvram_size() - 1)] = data;
	}
};


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_nvram_device::device_start()
{
	nvram_alloc(0x4000);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_NVRAM, device_bbc_rom_interface, bbc_nvram_device, "bbc_nvram", "BBC Micro Sideways RAM (Battery Backup)")
