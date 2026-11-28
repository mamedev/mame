// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Damian Rogers
/***********************************************************************************************

Sega Sonic & Knuckles lock-on cart

Board: MPR-16910 (IC1, 2 MB game ROM), MPR-16911 (IC2, 256 KB patch ROM for sonic2), 74HC74 +
74AC139 + 74AC08 glue logic, and a pass-through slot on top.

- $000000-$1fffff: IC1
- $200000-$3fffff: the cart on top
- A 74HC74 flip-flop is clocked by /TIME (B31) and latches VD0 (edge pin A20), so any write to
  $a130xx sets or clears it. When set, IC2 replaces the cart on top at $300000-$3fffff.

The cart on top gets the CPU address unchanged, A21 included. The flip-flop has no reset input
wired; the program clears it at every boot.

The program writes 0 to $a130f1 at every boot before it looks at the cart on top, then 1 before
jumping to IC2 when that cart is sonic2. With sonic3 on top it writes 1 and 0 around each access
to that cart's FRAM. /TIME writes are also passed on to the cart on top, so the same write
switches its FRAM.

TODO:
- /TIME reads also clock the flip-flop, latching whatever is on VD0 (undriven for $a130f1)
- /VRES (B27) is not wired to the cart on top, so the reset button does not reset it

***********************************************************************************************/

#include "emu.h"
#include "sk.h"

#include "options.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM_SK, megadrive_rom_sk_device, "megadrive_rom_sk", "Megadrive Sonic & Knuckles lock-on cart")

megadrive_rom_sk_device::megadrive_rom_sk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_ROM_SK, tag, owner, clock)
	, device_megadrive_cart_interface(mconfig, *this)
	, m_lockon_cart(*this, "cart")
	, m_rom(*this, "rom")
	, m_patch_rom(*this, "patch_rom")
	, m_patch_view(*this, "patch_view")
{
}

void megadrive_rom_sk_device::device_add_mconfig(machine_config &config)
{
	MEGADRIVE_CART_SLOT(config, m_lockon_cart, this->clock(), megadrive_cart_options, nullptr).set_must_be_loaded(false);
}

std::error_condition megadrive_rom_sk_device::load()
{
	memory_region *const romregion(cart_rom_region());
	if (!romregion || (romregion->bytes() != 0x34'0000))
	{
		osd_printf_error("Sonic & Knuckles cart needs a 0x340000 byte ROM region\n");
		return image_error::INVALIDLENGTH;
	}

	u8 *const base = &romregion->as_u8();
	m_rom->configure_entry(0, &base[0x00'0000]);
	m_patch_rom->configure_entry(0, &base[0x30'0000]);

	return std::error_condition();
}

void megadrive_rom_sk_device::device_start()
{
}

void megadrive_rom_sk_device::time_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (BIT(data, 0))
		m_patch_view.select(0);
	else
		m_patch_view.disable();
	m_lockon_cart->time_w(offset, data, mem_mask);
}

void megadrive_rom_sk_device::cart_map(address_map &map)
{
	// A21 is decoded by the 74AC139 and also wired straight to the cart on top
	map(0x00'0000, 0x3f'ffff).rw(m_lockon_cart, FUNC(megadrive_cart_slot_device::base_r), FUNC(megadrive_cart_slot_device::base_w));
	map(0x00'0000, 0x1f'ffff).bankr(m_rom).nopw();
	map(0x30'0000, 0x3f'ffff).view(m_patch_view);
	m_patch_view[0](0x30'0000, 0x33'ffff).mirror(0x0c'0000).bankr(m_patch_rom).nopw();
}

void megadrive_rom_sk_device::time_io_map(address_map &map)
{
	map(0x00, 0xff).r(m_lockon_cart, FUNC(megadrive_cart_slot_device::time_r)).w(FUNC(megadrive_rom_sk_device::time_w));
}
