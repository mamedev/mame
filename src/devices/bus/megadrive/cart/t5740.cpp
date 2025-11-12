// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

WaterMelon T-5740

https://segaretro.org/Pier_Solar_and_the_Great_Architects

TODO:
- several unknowns in TIME io range;

**************************************************************************************************/

#include "emu.h"
#include "t5740.h"

#include "bus/generic/slot.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_HB_PSOLAR, megadrive_hb_psolar_device, "megadrive_hb_psolar", "Megadrive Pier Solar T-5740 cart")

megadrive_hb_psolar_device::megadrive_hb_psolar_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_HB_PSOLAR, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom_bank(*this, "rom_bank_%u", 0U)
	, m_spi_eeprom(*this, "spi_eeprom")
{
}

void megadrive_hb_psolar_device::device_add_mconfig(machine_config &config)
{
	M95320_EEPROM(config, m_spi_eeprom, 0);
}

void megadrive_hb_psolar_device::device_start()
{
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x08'0000;
	device_generic_cart_interface::map_non_power_of_two(
		unsigned(romregion->bytes() / page_size),
		[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
		{
			for (int i = 0; i < 8; i++)
				m_rom_bank[i]->configure_entry(entry, &base[page * page_size]);
		});
}

void megadrive_hb_psolar_device::device_reset()
{
	for (int i = 0; i < 8; i++)
		m_rom_bank[i]->set_entry(i);
}

void megadrive_hb_psolar_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x07'ffff).bankr(m_rom_bank[0]);
	// HACK: protection workaround
	// At startup each bank is 32K mirrored, and an unspecified condition unlocks the
	// full bank. PC=15e6 reads it 3 times, with byte accesses, expecting to read the cart header
	// otherwise will crash during the "not endorsed by Sega" screen.
	map(0x01'8100, 0x01'81ff).lr16(NAME([this] (offs_t offset, u16 mem_mask) {
		const auto base = reinterpret_cast<u16 const *>(m_rom_bank[0]->base());
		const u32 rom_address = 0x1'8100 + (offset << 1);
		if (mem_mask != 0xffff)
		{
			return base[(rom_address & 0x7fff) >> 1];
		}
		return base[rom_address >> 1];
	}));
	map(0x08'0000, 0x0f'ffff).bankr(m_rom_bank[1]);
	map(0x10'0000, 0x17'ffff).bankr(m_rom_bank[2]);
	map(0x18'0000, 0x1f'ffff).bankr(m_rom_bank[3]);
	map(0x20'0000, 0x27'ffff).bankr(m_rom_bank[4]);
	map(0x28'0000, 0x2f'ffff).bankr(m_rom_bank[5]);
	map(0x30'0000, 0x37'ffff).bankr(m_rom_bank[6]);
	map(0x38'0000, 0x3f'ffff).bankr(m_rom_bank[7]);
}

void megadrive_hb_psolar_device::time_io_map(address_map &map)
{
	map(0x01, 0x01).lw8(NAME([] (offs_t offset, u8 data) {
		//logerror("Mode %02x\n", data);
		// x--- enable SPI bus?
		// --x- enable bankswitch addresses?
		// ---x always on
	}));
	map(0x03, 0x03).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[5]->set_entry(data & 0xf); }));
	map(0x05, 0x05).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[6]->set_entry(data & 0xf); }));
	map(0x07, 0x07).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[7]->set_entry(data & 0xf); }));
	map(0x09, 0x09).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_spi_eeprom->set_si_line(BIT(data, 0));
			m_spi_eeprom->set_sck_line(BIT(data, 1));
			m_spi_eeprom->set_halt_line(BIT(data, 2));
			m_spi_eeprom->set_cs_line(BIT(data, 3));
		})
	);
	map(0x0b, 0x0b).lr8(
		NAME([this] (offs_t offset) {
			return m_spi_eeprom->get_so_line() & 1;
		})
	);
}
