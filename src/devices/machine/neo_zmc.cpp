// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

	NEO-ZMC Memory controller emulation

	reference: https://wiki.neogeodev.org/index.php?title=NEO-ZMC

	ZMC is Z80 memory controller.
	It has 4 slot for bankswitching, Each slot is different size.

	Address bus is 19 bit,
	(maybe 22? Slot #3 is accessible higher than 512KB area?)
	it's can be allow accessing ~512KB ROM area with bankswitch in Z80.

	Address map
	0x0000-0x7fff Pass-through
	0x8000-0xbfff Slot #3
	0xc000-0xdfff Slot #2
	0xe000-0xefff Slot #1
	0xf000-0xf7ff Slot #0
	Remain is RAM and/or other peripherals/devices.

	Early implementation is TTL logic,
	Also integrated in NEO-ZMC2 and NEO-CMC.

***************************************************************************/

#include "emu.h"
#include "neo_zmc.h"
#include <algorithm>



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(NEO_ZMC, neo_zmc_device, "neo_zmc", "SNK NEO-ZMC Memory controller")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

void neo_zmc_device::map(address_map &map)
{
	// TODO: can be writeable?
	map(0x0000, 0x7fff).r(FUNC(neo_zmc_device::rom_r)); // A15 == 0
	map(0x8000, 0xbfff).r(FUNC(neo_zmc_device::banked_rom_r<3>)); // A15 == 1, A14 == 0
	map(0xc000, 0xdfff).r(FUNC(neo_zmc_device::banked_rom_r<2>)); // A15, A14 == 1, A13 == 0
	map(0xe000, 0xefff).r(FUNC(neo_zmc_device::banked_rom_r<1>)); // A15, A14, A13 == 1, A12 == 0
	map(0xf000, 0xf7ff).r(FUNC(neo_zmc_device::banked_rom_r<0>)); // A15, A14, A13, A12 == 1
}

//-------------------------------------------------
//  neo_zmc_device - constructor
//-------------------------------------------------

neo_zmc_device::neo_zmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NEO_ZMC, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_rom_space_config("rom_space", ENDIANNESS_LITTLE, 8, 19) /* 22 bit? most of cartridges are 512KB max */
	, m_default_rom(*this, DEVICE_SELF)
{
	std::fill(std::begin(m_bank), std::end(m_bank), 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neo_zmc_device::device_start()
{
	if ((m_default_rom != nullptr) && (!has_configured_map(0)))
		space(0).install_rom(0, m_default_rom.mask(), (~m_default_rom.mask()) & 0x7ffff, m_default_rom.target());

	space(0).cache(m_rom_cache);

	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void neo_zmc_device::device_reset()
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector neo_zmc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_rom_space_config)
	};
}


//-------------------------------------------------
//  set_bank_r - change bank slot (READ)
//-------------------------------------------------

u8 neo_zmc_device::set_bank_r(offs_t offset)
{
	// A0-1 - Select slot
	// A8-A15 - Bankswitch data
	m_bank[offset & 3] = ((offset >> 8) & 0xff) << (11 + (offset & 3));
	return 0;
}
