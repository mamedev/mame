// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

AV Artisan/Realtec cart mappers

Goofy banking scheme. Maps every bank with $7e000 page at startup, remaps in $20000 chunks
depending on masked values between $404000 and $402000 then locks the mapper.

Practically only funnywld sets these to non-zero values, everything else just writes 0 to both
regs.

**************************************************************************************************/

#include "emu.h"
#include "avartisan.h"

#include "bus/generic/slot.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_AVARTISAN, megadrive_unl_avartisan_device, "megadrive_unl_avartisan", "Megadrive AV Artisan cart")

megadrive_unl_avartisan_device::megadrive_unl_avartisan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_UNL_AVARTISAN, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom_bank(*this, "rom_bank_%u", 0U)
{
}

void megadrive_unl_avartisan_device::device_start()
{
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x00'2000;
	device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / page_size),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				for (int i = 0; i < 0x40; i++)
					m_rom_bank[i]->configure_entry(entry, &base[page * page_size]);
			});
}

void megadrive_unl_avartisan_device::device_reset()
{
	// TODO: default
	// GPGX uses /VRES to bypass TMSS with a read at $100, is it even possible?
	for (int i = 0; i < 0x40; i++)
		m_rom_bank[i]->set_entry(0x3f);
	m_lock_config = false;
}

void megadrive_unl_avartisan_device::cart_map(address_map &map)
{
	for (int i = 0; i < 0x40; i++)
		map(0x00'0000 | (i * 0x2000), 0x00'1fff | (i * 0x2000)).mirror(0x380000).bankr(m_rom_bank[i]);
	map(0x40'2000, 0x40'2000).lw8(NAME([this] (offs_t offset, u8 data) { m_bank_size = (data & 3) << 4; }));
	map(0x40'4000, 0x40'4000).lw8(NAME([this] (offs_t offset, u8 data) { m_bank_sel = (data & 3) << 4; }));
	map(0x40'0000, 0x40'0000).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// 0 -> 1 transitions
			if (!m_lock_config && BIT(data, 0))
			{
				logerror("size %02x sel %02x (%02x)\n", m_bank_size, m_bank_sel, ~m_bank_size);
				u8 bank_base = m_bank_sel & m_bank_size;
				for (int i = 0; i < 0x40; i++)
				{
					m_rom_bank[i]->set_entry(((i & ~m_bank_size) | bank_base) & 0x3f);
				}

				m_lock_config = !!BIT(data, 0);
			}
		})
	);
}

