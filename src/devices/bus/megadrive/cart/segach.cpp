// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Sega Channel Game no Kanzume "digest" RAM cart, developed by CRI

https://segaretro.org/Game_no_Kanzume_Otokuyou

TODO:
- some unknowns, needs PCB picture;
- Scientific Atlanta Sega Channel cart also derives from SSF but has it's own mapper scheme
  cfr. SCTOOLS/MENUTEST/HARDWARE.I in Sega Channel Jan 1996 dev CD dump.
  The SCI-ATL ASIC also contains an unspecified TCU device (or that's behind the RF shield?),
  in word status/data pair, where status direction is at bit 15 (1) read (0) write

**************************************************************************************************/

#include "emu.h"
#include "segach.h"

#include "bus/generic/slot.h"

/*
 * Game no Kanzume digest cart
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_SEGACH_JP, megadrive_segach_jp_device, "megadrive_segach_jp", "Megadrive Sega Channel Game no Kanzume RAM cart")


megadrive_segach_jp_device::megadrive_segach_jp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_SEGACH_JP, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom(*this, "rom")
	, m_ram_view(*this, "ram_view")
{
}

void megadrive_segach_jp_device::device_start()
{
	memory_region *const romregion(cart_rom_region());
	device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes()),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				m_rom->configure_entry(0, &base[0]);
			});
	m_ram.resize(0x40'000 / 2);
}

void megadrive_segach_jp_device::device_reset()
{
	m_ram_view.disable();
}

void megadrive_segach_jp_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x2f'ffff).bankr(m_rom);
	// NOTE: menu system writes extra couple writes at $40000,
	// programming mistake?
	map(0x00'0000, 0x03'ffff).view(m_ram_view);
	// TODO: Fatal error: Attempt to register save state entry after state registration is closed!
//  m_ram_view[0](0x00'0000, 0x03'ffff).ram();
	m_ram_view[0](0x00'0000, 0x03'ffff).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) { return m_ram[offset]; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_ram[offset]); })
	);
}

void megadrive_segach_jp_device::time_io_map(address_map &map)
{
//  map(0x01, 0x01) unknown, used in tandem with 0xf1 writes, ram bank select?
	map(0xf1, 0xf1).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// assumed, bclr #$1 at PC=5e282
			if (BIT(data, 1))
				m_ram_view.disable();
			else
				m_ram_view.select(0);
			// bit 0: write protect as per SSF2 mapper?
			if (data & 0xfd)
				popmessage("megadrive_segach_jp: unhandled $f1 write %02x", data);
		})
	);
}

/*
 * Scientific Atlanta US Sega Channel
 *
 * https://segaretro.org/Sega_Channel#Photo_gallery
 *
 * Various combinations hold at boot calls different menus:
 * - START+A: parental control
 * - START+C: language select
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_SEGACH_US, megadrive_segach_us_device, "megadrive_segach_us", "Megadrive Sega Channel US cart")

megadrive_segach_us_device::megadrive_segach_us_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_SEGACH_US, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, device_memory_interface( mconfig, *this )
	, m_rom(*this, "rom")
	, m_space_tcu_config("tcu_io", ENDIANNESS_BIG, 16, 9, 0, address_map_constructor(FUNC(megadrive_segach_us_device::tcu_map), this))
{
}

void megadrive_segach_us_device::device_start()
{
	memory_region *const romregion(cart_rom_region());
	device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes()),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				m_rom->configure_entry(0, &base[0]);
			});
	m_ram.resize(0x40'0000 / 2);
}

void megadrive_segach_us_device::device_reset()
{
}

device_memory_interface::space_config_vector megadrive_segach_us_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_space_tcu_config)
	};
}

void megadrive_segach_us_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x03'ffff).bankr(m_rom);
	// TODO: SRAM overlay at 0x20'0000 (length 0x8000)

	// TODO: banked?
	map(0x10'0000, 0x4f'ffff).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) { return m_ram[offset]; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_ram[offset]); })
	);
}

void megadrive_segach_us_device::time_io_map(address_map &map)
{
	// TCU STATUS
	map(0x04, 0x05).lrw16(
		NAME([] (offs_t offset, u16 mem_mask) {
			// status for something
			return 0xf;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			// assume always 16-bit access
			// 1: set for read, 0: set for write
			m_tcu_dir = BIT(data, 15);
			m_tcu_index = data & 0x1ff;
		})
	);
	// TCU DATA
	map(0x06, 0x07).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) -> u16 {
			if (!m_tcu_dir && !machine().side_effects_disabled())
			{
				logerror("Attempting to read with TCU DIR unset\n");
				return 0xffff;
			}
			return space(0).read_word(m_tcu_index, mem_mask);
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (m_tcu_dir)
			{
				logerror("Attempting to write with TCU DIR set\n");
				return;
			}
			space(0).write_word(m_tcu_index, data, mem_mask);
		})
	);
//  map(0x10, 0x11) fixit start address
//  map(0x12, 0x13) fixit work bound
//  map(0x20, 0x21) game timeout
//  map(0x22, 0x23) game id
//  map(0x24, 0x25) pkt match address
//  map(0x26, 0x27) current spacket
//  map(0x30, 0x31) gen status
//  map(0x32, 0x33) gen control
//  map(0x34, 0x35) error counter
//  map(0x40, 0x41) CRC input
//  map(0x42, 0x43) CRC low out
//  map(0x44, 0x45) CRC high out
//  map(0xf0, 0xff) SSF style bankswitch?
}

void megadrive_segach_us_device::tcu_map(address_map &map)
{
//  map(0x000, 0x000) AUTH map
//  map(0x010, 0x010) free map
//  map(0x020, 0x020) p2play
//  map(0x023, 0x023) p2pservid
//  map(0x0a0, 0x0a0) transdata
//  map(0x0b1, 0x0b2) gametime
//  map(0x0b3, 0x0b3) resetcond
//  map(0x0b4, 0x0b4) dayofweek
//  map(0x0b5, 0x0b5) week
//  map(0x0b6, 0x0b6) todtimout
//  map(0x0b7, 0x0b7) authbyte
//  map(0x0b8, 0x0b8) checksum
//  map(0x0b9, 0x0b9) PLL config
//  map(0x0ba, 0x0bd) PLL data
//  map(0x0c0, 0x0c1) password
//  map(0x0c2, 0x0c2) chanbitmap
//  map(0x0cb, 0x0cb) parental control level
//  map(0x0cc, 0x0cc) initialized
//  map(0x0cd, 0x0ce) random seed, (d?)word
//  map(0x0cf, 0x0cf) next DRAM test block
//  map(0x0e0, 0x0e0) station
//  map(0x0e1, 0x0e1) slot
//  map(0x0e2, 0x0e2) service ID
//  map(0x0e3, 0x0e3) filter code
//  map(0x0e5, 0x0e5) channel number
//  map(0x0f3, 0x0f6) unit addr
//  map(0x1e0, 0x1e0) language
//  map(0x1e1, 0x1e1) help communications port
//  map(0x1e2, 0x1e2) menu lptr
//  map(0x1e6, 0x1e6) tuner type (0) TD1A (1) TD1B
//  map(0x1e1, 0x1ff) mini NVRAM?
}
