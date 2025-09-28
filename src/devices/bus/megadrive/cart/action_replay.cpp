// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/***********************************************************************************************

Datel Action Replay lock-on carts

TODO:
- Actual cheat interface (needs a dynamic overlay for user work RAM mods)

***********************************************************************************************/

#include "emu.h"
#include "action_replay.h"

#include "options.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_ACTION_REPLAY, megadrive_action_replay_device, "megadrive_ar", "Megadrive Datel Action Replay cart")

megadrive_action_replay_device::megadrive_action_replay_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, type, tag, owner, clock)
	, m_lockon_cart(*this, "cart")
	, m_ar_view(*this, "ar_view")
{
}

megadrive_action_replay_device::megadrive_action_replay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_action_replay_device(mconfig, MEGADRIVE_ACTION_REPLAY, tag, owner, clock)
{
}

void megadrive_action_replay_device::device_add_mconfig(machine_config &config)
{
	MEGADRIVE_CART_SLOT(config, m_lockon_cart, this->clock(), megadrive_cart_options, nullptr).set_must_be_loaded(false);
}

void megadrive_action_replay_device::device_start()
{
	megadrive_rom_device::device_start();
}

void megadrive_action_replay_device::device_reset()
{
	megadrive_rom_device::device_reset();
	m_ar_view.select(0);
}

void megadrive_action_replay_device::unlock_cart_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (data == 0xffff)
	{
		m_ar_view.disable();
	}
}

void megadrive_action_replay_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x7f'ffff).rw(m_lockon_cart, FUNC(megadrive_cart_slot_device::base_r), FUNC(megadrive_cart_slot_device::base_w));
	map(0x00'0000, 0x01'ffff).view(m_ar_view);
	m_ar_view[0](0x00'0000, 0x00'7fff).mirror(0x00'8000).bankr(m_rom);
	m_ar_view[0](0x01'0006, 0x01'0007).w(FUNC(megadrive_action_replay_device::unlock_cart_w));
}

void megadrive_action_replay_device::time_io_map(address_map &map)
{
	map(0x00, 0xff).rw(m_lockon_cart, FUNC(megadrive_cart_slot_device::time_r), FUNC(megadrive_cart_slot_device::time_w));
}
