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

/*
 * 1800-in-1 mapper
 *
 * https://segaretro.org/Mega_Drive_unlicensed_multi-carts_(unsorted;_9_in_1)#9_in_1
 *
 * Changes bank on strobe reads, shuffled.
 * All Sega references looks either skipped or patched.
 *
 * Mapping (from 9-in-1 menu):
 * | gamename                | log | phy |
 * | 0001 RABO III           | 18  | 02  |
 * | 0002 SPACE INVADERS     | 00  | 00  |
 * | 0003 SUPPER VOLLEY BALL | 1c  | 06  |
 * | 0004 KLAX               | 0a  | 08  |
 * | 0005 PACMANIA           | 1a  | 0a  |
 * | 0006 TECMO WORLD CAP 92 | 0e  | 0c  |
 * | 0007 DOUBLE CLUTCH      | 04  | 04  |
 * | 0008 COLUMNS            | 1e  | 0e  |
 * | 0009 BLOCK OUT          | 7e  | 0f  |
 *
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_18KIN1, megadrive_18kin1_device, "megadrive_18kin1", "Megadrive 1800-in-1 pirate cart")

megadrive_18kin1_device::megadrive_18kin1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_18KIN1, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom_bank(*this, "rom_bank_%u", 0U)
	, m_config(*this, "CONFIG")
{
}

void megadrive_18kin1_device::device_start()
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

void megadrive_18kin1_device::device_reset()
{
	for (int i = 0; i < 4; i++)
		m_rom_bank[i]->set_entry(i);
}

INPUT_PORTS_START( _18kin1 )
	// Menu reads /time register contents for number of entries displayed.
	// This doesn't change the number of actual games.
	// TODO: dips or jumpers?
	PORT_START("CONFIG")
	PORT_CONFNAME(0x03,  0x03, "Menu title")
	PORT_CONFSETTING(    0x00, "9 in 1")
	PORT_CONFSETTING(    0x01, "190 in 1")
	PORT_CONFSETTING(    0x02, "888 in 1")
	PORT_CONFSETTING(    0x03, "1800 in 1")
INPUT_PORTS_END

ioport_constructor megadrive_18kin1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( _18kin1 );
}

void megadrive_18kin1_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x01'ffff).mirror(0x38'0000).bankr(m_rom_bank[0]);
	map(0x02'0000, 0x03'ffff).mirror(0x38'0000).bankr(m_rom_bank[1]);
	map(0x04'0000, 0x05'ffff).mirror(0x38'0000).bankr(m_rom_bank[2]);
	map(0x06'0000, 0x07'ffff).mirror(0x38'0000).bankr(m_rom_bank[3]);
}

void megadrive_18kin1_device::time_io_map(address_map &map)
{
	map(0x01, 0x01).select(0x7e).lr8(
		NAME([this] (offs_t offset) {
			const u8 page_sel = bitswap<4>(offset, 1, 2, 4, 5);
			logerror("Bank select log: %02x phy: %02x & %02x\n", offset, page_sel, m_bank_mask);
			if (!machine().side_effects_disabled())
			{
				for (int i = 0; i < 4; i++)
					m_rom_bank[i]->set_entry((page_sel + i) & m_bank_mask);
			}
			return m_config->read() & 3;
		})
	);
}

/*
 * Golden Mega 250-in-1
 *
 * Sonic 2, Alex Kidd, Trampoline Terror and Tecmo Cup Football Game (the Captain Tsubasa prototype)
 * mixed in 250 variants ...
 *
 * | gamename                 | log | phy |
 * | 001. SONIC 2             | 11  | 04  |
 * | 013. TECMO CUP SOCCER IV | a0  | 02  |
 * | 030. ALEX KIDD           | 40  | 01  |
 * | 063. TRAMPOLINE TERROR   | 00  | 00  |
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_GOLDM250, megadrive_goldm250_device, "megadrive_goldm250", "Megadrive Golden Mega 250-in-1 pirate cart")

megadrive_goldm250_device::megadrive_goldm250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_GOLDM250, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom_bank(*this, "rom_bank_%u", 0U)
{
}

void megadrive_goldm250_device::device_start()
{
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x04'0000;
	m_bank_mask = device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / page_size),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				for (int i = 0; i < 4; i++)
					m_rom_bank[i]->configure_entry(entry, &base[page * page_size]);
			});
	logerror("Bank mask %02x\n", m_bank_mask);
}

void megadrive_goldm250_device::device_reset()
{
	for (int i = 0; i < 4; i++)
		m_rom_bank[i]->set_entry(i);
}

void megadrive_goldm250_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x03'ffff).mirror(0x30'0000).bankr(m_rom_bank[0]);
	map(0x04'0000, 0x07'ffff).mirror(0x30'0000).bankr(m_rom_bank[1]);
	map(0x08'0000, 0x0b'ffff).mirror(0x30'0000).bankr(m_rom_bank[2]);
	map(0x0c'0000, 0x0f'ffff).mirror(0x30'0000).bankr(m_rom_bank[3]);
	map(0x08'9000, 0x08'9001).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
		// writes in word units
		if (ACCESSING_BITS_0_7)
		{
			const u8 page_sel = bitswap<3>(data, 0, 7, 6);
			logerror("Bank select log: %02x phy: %02x & %02x\n", data, page_sel, m_bank_mask);
			for (int i = 0; i < 4; i++)
				m_rom_bank[i]->set_entry((page_sel + i) & m_bank_mask);
		}
	}));
}


