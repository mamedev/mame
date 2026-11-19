// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Angelo Salese
/**************************************************************************************************

Telelink II

4-bit X2212 NVRAM and other stuff not known at current stage.
RD4 hardwired to +5V

TODO:
- CCTL mapping details are unknown;
- Requires modem i/f to work;

**************************************************************************************************/

#include "emu.h"
#include "telelink2.h"

DEFINE_DEVICE_TYPE(A800_ROM_TELELINK2, a800_rom_telelink2_device, "a800_tlink2",   "Atari 8-bit Telelink II cart")


a800_rom_telelink2_device::a800_rom_telelink2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_TELELINK2, tag, owner, clock)
	, m_nvram(*this, "nvram")
{
}

void a800_rom_telelink2_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

void a800_rom_telelink2_device::device_start()
{
	const u32 nvram_size = 0x100;

	m_nvram_ptr = std::make_unique<uint8_t[]>(nvram_size);
	m_nvram->set_base(m_nvram_ptr.get(), nvram_size);

	save_pointer(NAME(m_nvram_ptr), nvram_size);
}

void a800_rom_telelink2_device::device_reset()
{
}


void a800_rom_telelink2_device::cart_map(address_map &map)
{
	// 4-bit NVRAM
	map(0x1000, 0x10ff).mirror(0xf00).lrw8(
		NAME([this](offs_t offset) { return m_nvram_ptr[offset & 0xff]; }),
		NAME([this](offs_t offset, u8 data) { m_nvram_ptr[offset & 0xff] = data | 0xf0; })
	);
	map(0x2000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[offset & 0x1fff]; })
	);
}

void a800_rom_telelink2_device::cctl_map(address_map &map)
{
//  map(0x01, 0x01) read before reading NVRAM, value discarded
//  map(0x02, 0x02) written before writing NVRAM when changing stored information
}
