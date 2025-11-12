// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Super Mario World 64 cart mappers

https://segaretro.org/Super_Mario_World_64

Looks its own thing compared to anything else

TODO:
- some blanks, sound can crash after a while (which looks the main thing they bothered to protect);

**************************************************************************************************/

#include "emu.h"
#include "smw64.h"

#include "bus/generic/slot.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_SMW64, megadrive_unl_smw64_device, "megadrive_unl_smw64", "Megadrive Super Mario World 64 cart")

megadrive_unl_smw64_device::megadrive_unl_smw64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_SMW64, tag, owner, clock)
	, m_page_rom(*this, "page_rom_%u", 0U)
{
}

void megadrive_unl_smw64_device::device_start()
{
	megadrive_rom_device::device_start();
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x01'0000;
	device_generic_cart_interface::map_non_power_of_two(
		unsigned(romregion->bytes() / page_size),
		[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
		{
			m_page_rom[0]->configure_entry(entry, &base[page * page_size]);
			m_page_rom[1]->configure_entry(entry, &base[page * page_size]);
		});

	save_pointer(NAME(m_latch), 3);
	save_pointer(NAME(m_ctrl), 2);
	save_pointer(NAME(m_data), 2);
	save_item(NAME(m_page_67));
}

void megadrive_unl_smw64_device::device_reset()
{
	megadrive_rom_device::device_reset();

	m_page_rom[0]->set_entry(8);
	m_page_rom[1]->set_entry(8);
}

void megadrive_unl_smw64_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x07'ffff).mirror(0x08'0000).bankr(m_rom);
	map(0x60'0000, 0x60'ffff).mirror(0x08'0000).bankr(m_page_rom[0]);
	map(0x61'0000, 0x61'ffff).mirror(0x08'0000).bankr(m_page_rom[1]);
	map(0x60'0001, 0x60'0001).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_ctrl[0] = data;
		})
	);
	map(0x60'0003, 0x60'0003).lw8(
		NAME([this] (offs_t offset, u8 data) {
			switch(m_ctrl[0] & 7)
			{
				case 0: m_latch[0] = ((m_latch[0] ^ m_data[0]) ^ data) & 0xfe; break;
				case 1: m_latch[1] = data & 0xfe; break;
				// often, unknown purpose
				case 6: break;
				case 7: m_page_rom[1]->set_entry(8 + ((data & 0x1c) >> 2)); break;
				default:
					logerror("$60'0003 unknown mode %02x\n", m_ctrl[0]);
					break;
			}
			m_data[0] = data;
		})
	);
	map(0x61'0003, 0x61'0003).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_page_61 = data;
		})
	);
	map(0x64'0001, 0x64'0001).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_ctrl[1] = data;
		})
	);
	// 0x64'0003 rmw at startup (& 0xc0 ^ 0x55)
	map(0x64'0003, 0x64'0003).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_data[1] = data;
		})
	);
	// unknown writes, in tandem with mode 6 in $60'0003
	map(0x66'0000, 0x66'0001).nopw();
	map(0x66'0001, 0x66'0001).lr8(NAME([this] () { return m_latch[0]; }));
	map(0x66'0003, 0x66'0003).lr8(NAME([this] () { return m_latch[0] + 1; }));
	map(0x66'0005, 0x66'0005).lr8(NAME([this] () { return m_latch[1]; }));
	map(0x66'0007, 0x66'0007).lr8(NAME([this] () { return m_latch[1] + 1; }));
	map(0x66'0009, 0x66'0009).lr8(NAME([this] () { return m_latch[2]; }));
	map(0x66'000b, 0x66'000b).lr8(NAME([this] () { return m_latch[2] + 1; }));
	map(0x66'000d, 0x66'000d).lr8(NAME([this] () { return m_latch[2] + 2; }));
	map(0x66'000f, 0x66'000f).lr8(NAME([this] () { return m_latch[2] + 3; }));

	map(0x67'0001, 0x67'0001).select(2).lr8(
		NAME([this] (offs_t offset) {
			u8 res = 0;
			if (BIT(m_page_61, 7))
			{
				if (BIT(m_page_67, 6))
					res = m_ctrl[1] & m_data[1];
				else
					res = m_ctrl[1] ^ 0xff;
			}

			if (BIT(offset, 1))
				res &= 0x7f;
			else if (!machine().side_effects_disabled() && BIT(m_page_67, 7))
			{
				if (BIT(m_page_67, 5))
					m_latch[2] = (m_data[1] << 2) & 0xfc;
				else
					m_latch[0] = (m_data[0] ^ (m_ctrl[1] << 1)) & 0xfe;
			}

			return res;
		})
	);
	map(0x67'0001, 0x67'0001).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_page_67 = data;
			if (BIT(m_page_61, 7))
				m_page_rom[0]->set_entry(8 + ((data & 0x1c) >> 2));
		})
	);

}
