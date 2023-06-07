// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese
/**************************************************************************************************

SpartaDOS X (SDX) cart emulation

https://sdx.atari8.info/index.php?show=en_introduction

SDX 128KB a newer format used by the 200x releases: adds an extra bank access to $d5f0-$d5f7.
Should mirror $e8-$ef to $fx by logic.

**************************************************************************************************/

#include "emu.h"
#include "sparta.h"
#include "a800_carts.h"

//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(A800_ROM_SPARTADOS,       a800_rom_spartados_device,       "a800_sparta",       "Atari 8-bit SpartaDOS X cart")
DEFINE_DEVICE_TYPE(A800_ROM_SPARTADOS_128KB, a800_rom_spartados_128kb_device, "a800_sparta_128kb", "Atari 8-bit SpartaDOS X 128KB cart")

a800_rom_spartados_device::a800_rom_spartados_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, type, tag, owner, clock)
	, m_bank(0)
	, m_subcart(*this, "subcart")
	, m_cart_view(*this, "cart_view")
{

}

a800_rom_spartados_device::a800_rom_spartados_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_spartados_device(mconfig, A800_ROM_SPARTADOS, tag, owner, clock)
{
}

static void spartados_carts(device_slot_interface &device)
{
	// NB: game carts will usually override SDX boot, meaning that they aren't all that useful to hook up here.
	// also a SDX attached to another SDX just produces a black screen.
	// we just hook them all up for the sake of completeness.
	a800_left(device);
}

void a800_rom_spartados_device::subcart_rd4_w(int state)
{
	m_subcart_rd4_enabled = state;
	if (m_subcart_enabled)
	{
		rd4_w(m_subcart_rd4_enabled);
		rd5_w(m_subcart_rd5_enabled);
	}
}

void a800_rom_spartados_device::subcart_rd5_w(int state)
{
	m_subcart_rd5_enabled = state;
	if (m_subcart_enabled)
	{
		rd4_w(m_subcart_rd4_enabled);
		rd5_w(m_subcart_rd5_enabled);
	}
}

void a800_rom_spartados_device::device_add_mconfig(machine_config &config)
{
	A800_CART_SLOT(config, m_subcart, spartados_carts, nullptr);
	m_subcart->rd4_callback().set(FUNC(a800_rom_spartados_device::subcart_rd4_w));
	m_subcart->rd5_callback().set(FUNC(a800_rom_spartados_device::subcart_rd5_w));
}

void a800_rom_spartados_device::device_start()
{
	save_item(NAME(m_bank));
	save_item(NAME(m_subcart_enabled));
}

void a800_rom_spartados_device::device_reset()
{
	m_bank = 7;
	m_subcart_enabled = false;
	if (!m_subcart->exists())
		m_subcart_rd4_enabled = m_subcart_rd5_enabled = 0;
	else
		std::tie(m_subcart_rd4_enabled, m_subcart_rd5_enabled) = m_subcart->get_initial_rd_state();

	m_cart_view.select(0);
}


void a800_rom_spartados_device::cart_map(address_map &map)
{
	map(0x0000, 0x3fff).view(m_cart_view);
	m_cart_view[0](0x2000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)]; })
	);
	m_cart_view[1](0x0000, 0x1fff).rw(m_subcart, FUNC(a800_cart_slot_device::read_cart<0>), FUNC(a800_cart_slot_device::write_cart<0>));
	m_cart_view[1](0x2000, 0x3fff).rw(m_subcart, FUNC(a800_cart_slot_device::read_cart<1>), FUNC(a800_cart_slot_device::write_cart<1>));
}

void a800_rom_spartados_device::cctl_map(address_map &map)
{
	map(0x00, 0xff).rw(m_subcart, FUNC(a800_cart_slot_device::read_cctl), FUNC(a800_cart_slot_device::write_cctl));
	map(0xe0, 0xe7).rw(FUNC(a800_rom_spartados_device::rom_bank_r), FUNC(a800_rom_spartados_device::rom_bank_w));
	map(0xe8, 0xef).rw(FUNC(a800_rom_spartados_device::subslot_r), FUNC(a800_rom_spartados_device::subslot_w));
}

inline void a800_rom_spartados_device::bank_config_access(offs_t offset)
{
	rd4_w(0);
	rd5_w(1);
	m_cart_view.select(0);
	m_bank = ((offset ^ m_bank_mask) & m_bank_mask);
	m_subcart_enabled = false;
}

uint8_t a800_rom_spartados_device::rom_bank_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
		bank_config_access(offset);

	return m_subcart->read_cctl(offset | 0xe0);
}

void a800_rom_spartados_device::rom_bank_w(offs_t offset, uint8_t data)
{
	bank_config_access(offset);
	m_subcart->write_cctl(offset | 0xe0, data);
}

inline void a800_rom_spartados_device::subslot_config_access(offs_t offset)
{
	if (!BIT(offset, 2))
	{
		m_subcart_enabled = true;
		rd4_w(m_subcart_rd4_enabled);
		rd5_w(m_subcart_rd5_enabled);
		m_cart_view.select(1);
	}
	else
	{
		m_subcart_enabled = false;
		rd4_w(0);
		rd5_w(0);
	}
}

u8 a800_rom_spartados_device::subslot_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
		subslot_config_access(offset);

	return m_subcart->read_cctl(offset | 0xe8);
}

void a800_rom_spartados_device::subslot_w(offs_t offset, u8 data)
{
	subslot_config_access(offset);

	return m_subcart->write_cctl(offset | 0xe8, data);
}

/*-------------------------------------------------

 SpartaDOS 128KB variant

 -------------------------------------------------*/

a800_rom_spartados_128kb_device::a800_rom_spartados_128kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_spartados_device(mconfig, A800_ROM_SPARTADOS_128KB, tag, owner, clock)
{
}

// NB: .select not .mirror, we need the offset to propagate to sub cart cctl
void a800_rom_spartados_128kb_device::cctl_map(address_map &map)
{
	a800_rom_spartados_device::cctl_map(map);
	map(0xe0, 0xe7).select(0x10).rw(FUNC(a800_rom_spartados_128kb_device::rom_bank_r), FUNC(a800_rom_spartados_128kb_device::rom_bank_w));
	map(0xe8, 0xef).select(0x10).rw(FUNC(a800_rom_spartados_128kb_device::subslot_r), FUNC(a800_rom_spartados_128kb_device::subslot_w));
}

inline void a800_rom_spartados_128kb_device::bank_config_access(offs_t offset)
{
	a800_rom_spartados_device::bank_config_access(offset);
	const u8 upper_bank = !BIT(offset, 4);
	m_bank = ((offset ^ 7) & 7) | (upper_bank << 3);
}
