// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    pak.cpp

    Code for emulating standard MC-10 cartridges with only ROM

***************************************************************************/

#include "emu.h"
#include "pak.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mc10_pak_device

class mc10_pak_device :
		public device_t,
		public device_mc10cart_interface
{
public:
	// construction/destruction
	mc10_pak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual int max_rom_length() const override;

	virtual std::pair<std::error_condition, std::string> load() override;

protected:
	mc10_pak_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override;
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc10_pak_device - constructor
//-------------------------------------------------
mc10_pak_device::mc10_pak_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mc10cart_interface(mconfig, *this)
{
}

mc10_pak_device::mc10_pak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mc10_pak_device(mconfig, MC10_PAK, tag, owner, clock)
{
}

//-------------------------------------------------
//  rom constraints
//-------------------------------------------------

int mc10_pak_device::max_rom_length() const
{
	return 1024 * 16;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc10_pak_device::device_start()
{
}

//-------------------------------------------------
//  load - install ROM region
//-------------------------------------------------

std::pair<std::error_condition, std::string> mc10_pak_device::load()
{
	memory_region *const romregion(memregion("^rom"));
	assert(romregion != nullptr);

	// if the host has supplied a ROM space, install it
	owning_slot().memspace().install_rom(0x5000, 0x5000 + romregion->bytes(), romregion->base());

	return std::make_pair(std::error_condition(), std::string());
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MC10_PAK, device_mc10cart_interface, mc10_pak_device, "mc10pak", "MC-10 Program PAK")
