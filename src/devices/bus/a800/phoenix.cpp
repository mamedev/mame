// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Phoenix/Blizzard cart schemes

RD5 is special here: once disarmed it cannot be armed again from software.

TODO:
- Reset button optionally located on cart allows it to be rearmed;
- Blizzard 32KB scheme has additional ROM bank: maps to $a000-$bfff then any access to CCTL
  bankswitch to the next index, eventually disarms RD5 after bank 3.
  Needs a dump to checkout, and it sounds more suited to derive from ultracart scheme rather
  than this.

Note:
- "Blizzard 4KB" note from .car specs suggests being a Phoenix in disguise,
  they are definitely a better suit to use the a800_phoenix def rather than blizzard_device
  given the RD4 access of latter.

**************************************************************************************************/

#include "emu.h"
#include "phoenix.h"

DEFINE_DEVICE_TYPE(A800_ROM_PHOENIX,       a800_rom_phoenix_device,       "a800_phoenix",  "Atari 800 Phoenix cartridge")
DEFINE_DEVICE_TYPE(A800_ROM_BLIZZARD_16KB, a800_rom_blizzard_16kb_device, "a800_blizzard", "Atari 800 Blizzard cartridge")

a800_rom_phoenix_device::a800_rom_phoenix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, type, tag, owner, clock)
{
}

a800_rom_phoenix_device::a800_rom_phoenix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_phoenix_device(mconfig, A800_ROM_PHOENIX, tag, owner, clock)
{
}

void a800_rom_phoenix_device::device_start()
{
	m_rom_mask = get_rom_size() - 1;
}

void a800_rom_phoenix_device::device_reset()
{
	rd4_w(0);
	rd5_w(1);
}

void a800_rom_phoenix_device::cart_map(address_map &map)
{
	map(0x2000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & m_rom_mask)]; })
	);
}

u8 a800_rom_phoenix_device::disable_rom_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
		rd5_w(0);

	return 0xff;
}

void a800_rom_phoenix_device::disable_rom_w(offs_t offset, u8 data)
{
	rd5_w(0);
}

void a800_rom_phoenix_device::cctl_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(a800_rom_phoenix_device::disable_rom_r), FUNC(a800_rom_phoenix_device::disable_rom_w));
}

/*-------------------------------------------------

 Blizzard 16KB carts

 -------------------------------------------------*/

a800_rom_blizzard_16kb_device::a800_rom_blizzard_16kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_phoenix_device(mconfig, A800_ROM_BLIZZARD_16KB, tag, owner, clock)
{
}

void a800_rom_blizzard_16kb_device::device_reset()
{
	rd_both_w(1);
}

void a800_rom_blizzard_16kb_device::cart_map(address_map &map)
{
	map(0x0000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & m_rom_mask)]; })
	);
}

u8 a800_rom_blizzard_16kb_device::disable_rom_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
		rd_both_w(0);

	return 0xff;
}

void a800_rom_blizzard_16kb_device::disable_rom_w(offs_t offset, u8 data)
{
	rd_both_w(0);
}
