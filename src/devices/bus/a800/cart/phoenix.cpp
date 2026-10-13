// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Phoenix/Blizzard cart schemes

RD5 is special here: once disarmed it cannot be armed again from software.

TODO:
- Reset button optionally located on cart allows RD5 to be rearmed;

Notes:
- "Blizzard 4KB" note from .car specs suggests being a Phoenix in disguise,
  they are definitely a better suit to use the a800_phoenix def rather than blizzard_device
  given the RD4 access of latter.
- For "Blizzard 32KB" cfr. ultracart.cpp
- "Phoenix AST2K" is an oddity, according to Kr0tki on AtariAge forums:
  "AST 2000 was two cartridges in one - it contained an AST Utility cartridge,
  and a cartridge for Turbo 2000 (another turbo system popular in Poland).
  The "AST 2000" contained a 16KB ROM and a two-position switch, which allowed to choose between
  one of two 8KB banks. [...] the second half (Turbo 2000) [...] contained a capacitor that
  automatically switched off the cartridge from memory after a certain time [...]"
  https://forums.atariage.com/topic/169199-mess-a800-cartridge-software-list/#comment-2136371



**************************************************************************************************/

#include "emu.h"
#include "phoenix.h"

DEFINE_DEVICE_TYPE(A800_ROM_PHOENIX,       a800_rom_phoenix_device,       "a800_phoenix",       "Atari 8-bit Phoenix cart")
DEFINE_DEVICE_TYPE(A800_ROM_BLIZZARD_16KB, a800_rom_blizzard_16kb_device, "a800_blizzard",      "Atari 8-bit Blizzard 16KB ROM cart")
DEFINE_DEVICE_TYPE(A800_ROM_PHOENIX_AST2K, a800_rom_phoenix_ast2k_device, "a800_phoenix_ast2k", "Atari 8-bit Phoenix AST2K 2-in-1 cart")

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

/*-------------------------------------------------

 Phoenix AST2K variant

 -------------------------------------------------*/

a800_rom_phoenix_ast2k_device::a800_rom_phoenix_ast2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_phoenix_device(mconfig, A800_ROM_PHOENIX_AST2K, tag, owner, clock)
	, m_dsw(*this, "DSW")
	, m_rom_select(0)
{
}

static INPUT_PORTS_START(ast2k)
	PORT_START("DSW")
	// two position switch located on cart
	// we default to Turbo 2000 mode since it's a bit more worth in regtest scenarios
	PORT_DIPNAME(0x01, 0x01, "Boot mode" )
	PORT_DIPSETTING(0x00,    "AST Utility" )
	PORT_DIPSETTING(0x01,    "Turbo 2000" )
INPUT_PORTS_END

ioport_constructor a800_rom_phoenix_ast2k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ast2k);
}

void a800_rom_phoenix_ast2k_device::device_start()
{
//  a800_rom_phoenix_device::device_start();
	// FIXME: not initing this properly, needs memory_region
	m_rom_mask = 0x1fff;
	m_rd5_disarm_timer = timer_alloc(FUNC(a800_rom_phoenix_ast2k_device::rd5_disarm_cb), this);
	save_item(NAME(m_rom_select));
}

void a800_rom_phoenix_ast2k_device::device_reset()
{
	a800_rom_phoenix_device::device_reset();

	m_rom_select = BIT(m_dsw->read(), 0) << 13;

	// TODO: unknown RD5 disarm time details
	// - frame 137 is the first attempt of Turbo 2k software that tries to write to RD5 space;
	// - is the disarm mechanic really disabled in AST Utility mode?
	// - 4 seconds is a very lax time so that it doesn't backfire in case of atari400 driver timing changes;
	m_rd5_disarm_timer->adjust((m_rom_select) ? attotime::from_seconds(4) : attotime::never);
}

TIMER_CALLBACK_MEMBER(a800_rom_phoenix_ast2k_device::rd5_disarm_cb)
{
	logerror("RD5 disarmed thru timer\n");
	rd5_w(0);
}

void a800_rom_phoenix_ast2k_device::cart_map(address_map &map)
{
	map(0x2000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & m_rom_mask) | m_rom_select]; })
	);
}
