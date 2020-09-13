// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

    SNK NEO-ZMC(2) Z80 Memory controller emulation

    reference:
		NEO-ZMC - https://wiki.neogeodev.org/index.php?title=NEO-ZMC
		NEO-ZMC2 - https://wiki.neogeodev.org/index.php?title=NEO-ZMC2

    This chip is Z80 Memory controller, it's allow accessing >64K area
    in Z80 with bankswitching.

    Used in most of Neo Geo and Neo Print cartridges,
    also integrated in NEO-ZMC2, NEO-CMC.
	Some bootleg and Homebrew hardwares are uses cloned ZMC function
	in FPGA or TTL logic.

	NEO-ZMC2 is integrated with PRO-CT0,
	Used for replace PRO-CT0 in later hardwares
	(MVS motherboards, AES cartridges)

    Decoded address map is:
    0x0000-0x7fff Pass-through area
    0x8000-0xbfff Slot 3, Size 0x4000 bytes
    0xc000-0xdfff Slot 2, Size 0x2000 bytes
    0xe000-0xefff Slot 1, Size 0x1000 bytes
    0xf000-0xf7ff Slot 0, Size 0x0800 bytes
    0xf800-0xffff Unknown/Unused?

    TODO:
    - Write in bankswitched area is allowed?

***************************************************************************/

#include "emu.h"
#include "neo_zmc.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

void neo_zmc_core_device::map(address_map &map)
{
	map(0x0000, 0x7fff).r(FUNC(neo_zmc_core_device::rom_r));
	map(0x8000, 0xbfff).r(FUNC(neo_zmc_core_device::banked_rom_r<3>));
	map(0xc000, 0xdfff).r(FUNC(neo_zmc_core_device::banked_rom_r<2>));
	map(0xe000, 0xefff).r(FUNC(neo_zmc_core_device::banked_rom_r<1>));
	map(0xf000, 0xf7ff).r(FUNC(neo_zmc_core_device::banked_rom_r<0>));
}

// device type definition
DEFINE_DEVICE_TYPE(NEO_ZMC,  neo_zmc_device,  "neo_zmc",  "SNK NEO-ZMC Z80 Memory controller")
DEFINE_DEVICE_TYPE(NEO_ZMC2, neo_zmc2_device, "neo_zmc2", "SNK NEO-ZMC2 Z80 Memory controller with Sprite ROM data serializer")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neo_zmc_device - constructor
//-------------------------------------------------

neo_zmc_core_device::neo_zmc_core_device(const machine_config &mconfig, device_type &type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_default_rom(*this, DEVICE_SELF)
{
}

neo_zmc_device::neo_zmc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: neo_zmc_core_device(mconfig, NEO_ZMC, tag, owner, clock)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 19) // maybe 22 bit address? (0x8000-0xefff slot can be accessable >512KB area?)
{
}

neo_zmc2_device::neo_zmc2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: neo_zmc_core_device(mconfig, NEO_ZMC2, tag, owner, clock)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 22) // all 22 bit address bus pin is spotted
	, m_pro_ct0(*this, "pro_ct0")
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neo_zmc_core_device::device_start()
{
	save_item(NAME(m_bank));
}

void neo_zmc_device::device_start()
{
	// install default ROM
	if ((m_default_rom != nullptr) && !has_configured_map(0))
	{
		offs_t mask = std::min<offs_t>(0x7ffff, m_default_rom.mask()); // 19(22?) bit
		space(0).install_rom(0, mask, (~mask) & 0x7ffff, m_default_rom.target());
	}

	// Find our direct access
	space(0).cache(m_cache);

	neo_zmc_core_device::device_start();
}

void neo_zmc2_device::device_start()
{
	// install default ROM
	if ((m_default_rom != nullptr) && !has_configured_map(0))
	{
		offs_t mask = std::min<offs_t>(0x3fffff, m_default_rom.mask()); // 22 bit
		space(0).install_rom(0, mask, (~mask) & 0x3fffff, m_default_rom.target());
	}

	// Find our direct access
	space(0).cache(m_cache);

	neo_zmc_core_device::device_start();
}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void neo_zmc2_device::device_add_mconfig(machine_config &config)
{
	ALPHA_8921(config, m_pro_ct0, DERIVED_CLOCK(1,1)); // integrated
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector neo_zmc_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_data_config) };
}

device_memory_interface::space_config_vector neo_zmc2_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_data_config) };
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  bank_w - Change bank value
//-------------------------------------------------

void neo_zmc_core_device::bank_w(offs_t offset, u8 data)
{
	const u8 slot = BIT(offset, 0, 2); // A0-A1 select slot
	const u32 bank = BIT(offset, 8, 8); // A8-A15 bank data
	m_bank[slot] = bank << (11 + slot);
}
