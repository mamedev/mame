// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

    SNK NEO-ZMC Z80 bankswitch controller

    This chip handles the Z80 bankswitching method,
    also integrated in:
    - NEO-ZMC2 (with ALPHA-8921: see machine/alpha_8921.cpp) and;
    - NEO-CMC (with (optional) Z80 ROM encryption).

    Used by:
    - neogeo/neoprint.cpp

    - neogeo/neogeo.cpp should be used in cartridge if chip exists

    reference: https://wiki.neogeodev.org/index.php?title=NEO-ZMC

***************************************************************************/

#include "emu.h"
#include "neo_zmc.h"

#include <algorithm>

void neo_zmc_device::bank_map(address_map &map)
{
	map(0x0000, 0x7fff).lr8(NAME([this](offs_t offset) -> u8 { return read_byte(offset); }));
	map(0x8000, 0xbfff).r(FUNC(neo_zmc_device::banked_r<3>));
	map(0xc000, 0xdfff).r(FUNC(neo_zmc_device::banked_r<2>));
	map(0xe000, 0xefff).r(FUNC(neo_zmc_device::banked_r<1>));
	map(0xf000, 0xf7ff).r(FUNC(neo_zmc_device::banked_r<0>));
}

DEFINE_DEVICE_TYPE(NEO_ZMC, neo_zmc_device, "neo_zmc", "SNK NEO-ZMC bankswitch controller")

neo_zmc_device::neo_zmc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NEO_ZMC, tag, owner, clock)
	, device_rom_interface(mconfig, *this)
	, m_banksel{0}
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neo_zmc_device::device_start()
{
	save_item(NAME(m_banksel));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void neo_zmc_device::device_reset()
{
	std::fill_n(std::begin(m_banksel), 4, 0);
}

//-------------------------------------------------
//  bank_r - set bank value (read)
//-------------------------------------------------

u8 neo_zmc_device::bank_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		const u8 index = offset & 3;
		m_banksel[index] = (offset >> 8) & (0xff >> index); // or non-masked?
	}
	return 0;
}
