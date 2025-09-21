// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

X-in-1 pirate multigame handling (with actual menu vs. /VRES in multigame)

https://segaretro.org/Mega_Drive_unlicensed_multi-carts_(S_series)

TODO:
- wisdomtc may use a single mapper bank granularity of $80000

**************************************************************************************************/

#include "emu.h"
#include "mcpirate.h"

#include "bus/generic/slot.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_MCPIRATE, megadrive_mcpirate_device, "megadrive_mcpirate", "Megadrive multigame pirate cart")

megadrive_mcpirate_device::megadrive_mcpirate_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_MCPIRATE, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom_bank(*this, "rom_bank_%u", 0U)
{
}

void megadrive_mcpirate_device::device_start()
{
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x02'0000;
	m_bank_mask = device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / page_size),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				for (int i = 0; i < 4; i++)
					m_rom_bank[i]->configure_entry(entry, &base[page * page_size]);
			});
	logerror("Bank mask %02x\n", m_bank_mask);
}

void megadrive_mcpirate_device::device_reset()
{
	for (int i = 0; i < 4; i++)
		m_rom_bank[i]->set_entry(i);
}

void megadrive_mcpirate_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x01'ffff).mirror(0x38'0000).bankr(m_rom_bank[0]);
	map(0x02'0000, 0x03'ffff).mirror(0x38'0000).bankr(m_rom_bank[1]);
	map(0x04'0000, 0x05'ffff).mirror(0x38'0000).bankr(m_rom_bank[2]);
	map(0x06'0000, 0x07'ffff).mirror(0x38'0000).bankr(m_rom_bank[3]);
}


void megadrive_mcpirate_device::time_io_map(address_map &map)
{
	map(0x00, 0x00).select(0x3e).lw8(
		NAME([this] (offs_t offset, u8 data) {
			(void)data;
			const u8 page_sel = offset >> 1;
			logerror("Bank select %02x\n", page_sel);
			for (int i = 0; i < 4; i++)
				m_rom_bank[i]->set_entry((page_sel + i) & m_bank_mask);
		})
	);
}
