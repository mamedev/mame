// license: BSD-3-Clause
// copyright-holders: Angelo Salese, superctr
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

#include <fstream>

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
 * Eventually times out with error 0008 at the title screen.
 *
 * Various combinations hold at boot calls different menus:
 * - START+A: parental control
 * - START+C: language select
 * - START+B allows access to a diagnostics menu if a special
 *   device is connected to controller port 2 that shorts
 *   bit 0 and 6. For segachnl, you can bypass this
 *   by noping out everything from $a61a to $a670 (or doing
 *   the same thing with breakpoints in the debugger).
 *
 * The BIOS also listens to controller port 2 in serial mode
 * for a diagnostics mode.
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_SEGACH_US, megadrive_segach_us_device, "megadrive_segach_us", "Megadrive Sega Channel US cart")

megadrive_segach_us_device::megadrive_segach_us_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_SEGACH_US, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, device_memory_interface( mconfig, *this )
	, m_packet_timer(*this, "packet_timer")
	, m_rom(*this, "rom")
	, m_space_tcu_config("tcu_io", ENDIANNESS_BIG, 8, 9, 0, address_map_constructor(FUNC(megadrive_segach_us_device::tcu_map), this))
{
}

void megadrive_segach_us_device::device_add_mconfig(machine_config &config)
{
	// each broadcast frame has 10 packets and 288 bytes per packet
	// assuming 12Mbps download rate
	TIMER(config, "packet_timer", 0).configure_periodic(FUNC(megadrive_segach_us_device::send_packets),
			attotime::from_hz((12*128072) / 2880));
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
	m_nvm.fill(0);
	load_packets();

	m_broadcast_packet = 0;

	m_game_id = 0;
	m_curr_packet = 0;
	m_packet_match = 0x7fff;
	m_gen_control = 0;
	m_gen_status = 0;
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
			// bit 0: PLL locked
			// bit 1: will cause error 0001 if set
			// bit 15: TCU is busy
			return 1;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			// 1: set for read, 0: set for write
			m_tcu_dir = BIT(data, 15);
			m_tcu_index = data & 0x1ff;
		})
	);
	// TCU DATA
	map(0x07, 0x07).lrw8(
		NAME([this] (offs_t offset) -> u16 {
			if (!m_tcu_dir && !machine().side_effects_disabled())
			{
				logerror("Attempting to read with TCU DIR unset\n");
				return 0xffff;
			}
			u16 data = space(0).read_byte(m_tcu_index);
			if (!machine().side_effects_disabled())
				m_tcu_index = (m_tcu_index + 1) & 0x1ff;
			return data;
		}),
		NAME([this] (offs_t offset, u16 data) {
			if (m_tcu_dir)
			{
				logerror("Attempting to write with TCU DIR set\n");
				return;
			}
			space(0).write_byte(m_tcu_index, data);
			m_tcu_index = (m_tcu_index + 1) & 0x1ff;
		})
	);
//  map(0x10, 0x11) fixit start address
//  map(0x12, 0x13) fixit work bound
//  map(0x20, 0x21) game timeout
	map(0x22, 0x23).lrw16( // game id
		NAME([this] (offs_t offset, u16 mem_mask) -> u16 {
			return m_game_id;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			logerror("game_id: %04x\n", data);
			m_game_id = data;
		})
	);
	map(0x24, 0x25).lrw16( // pkt match address
		NAME([this] (offs_t offset, u16 mem_mask) -> u16{
			return m_packet_match;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			logerror("packet_match: %04x\n", data);
			m_packet_match = data;
		})
	);
	map(0x26, 0x27).lrw16( // current superpacket
		NAME([this] (offs_t offset, u16 mem_mask) -> u16 {
			return m_curr_packet;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			logerror("curr_packet: %04x\n", data);
			m_curr_packet = data;
		})
	);
	map(0x30, 0x31).lrw16( // general status
		NAME([this] (offs_t offset, u16 mem_mask) -> u16 {
			// bit 0: download complete
			// bit 1-2: unknown status bits
			// bit 12-15: ASIC version? causes BIOS to return error 1.
			return m_gen_status;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			logerror("gen_status: %04x\n", data);
			m_gen_status &= ~data;
		})
	);
	map(0x32, 0x33).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) -> u16 {
			// bit 0: reset console
			// bit 1: download enable
			// bit 2: DRAM bus control?
			// bit 3: switch BIOS/game view
			// bit 4: reset CRC
			// bit 5: enable fixit buffer?
			return m_gen_control;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			logerror("gen_control: %04x\n", data);
			m_gen_control = data & 0x2e;
			if (data & 0x01)
				logerror("console reset\n");
			if (data & 0x02)
				logerror("start download\n");
			if (data & 0x04)
				logerror("CRC reset\n");
		})
	);
//  map(0x34, 0x35) error counter
//  map(0x40, 0x41) CRC input
//  map(0x42, 0x43) CRC low out
//  map(0x44, 0x45) CRC high out
//  map(0xf0, 0xff) SSF style bankswitch?
}

void megadrive_segach_us_device::tcu_map(address_map &map)
{
//  map(0x000, 0x00f) authorization map
	map(0x000, 0x00f).lr8(
		NAME([] () -> u8 {
			// this is a bitmap containing which service IDs are
			// authorized to play on the adapter.
			return 0xff;
		}));
//  map(0x010, 0x01f) free map
	map(0x010, 0x01f).lr8(
		NAME([] () -> u8 {
			// this is a bitmap containing which service IDs are
			// free to play on any adapter. only checked if the
			// authorization bit was 0.
			return 0xff;
		}));
//  map(0x020, 0x020) pay-to-play
//  map(0x023, 0x023) pay-to-play service ID
//  map(0x0a0, 0x0a0) transaction data
//  map(0x0b1, 0x0b2) game time remaining
//  map(0x0b3, 0x0b3) reset condition
//  map(0x0b4, 0x0b4) day of week
//  map(0x0b5, 0x0b5) week
//  map(0x0b6, 0x0b6) time of day timeout
//  map(0x0b7, 0x0b7) authorization byte
	map(0x0b7, 0x0b7).lr8(
		NAME([] () -> u8 {
			// bit 7 is set if the cable company has authorized the adapter
			return 0x80; // 0x00..0x04 if unauthorized
		}));
//  map(0x0b8, 0x0b8) checksum
//  map(0x0b9, 0x0b9) PLL type
	map(0x0b9, 0x0b9).lr8(
		NAME([] () -> u8 {
			return 0x81; // mitsubishi/philips
			// return 0x87; // rohm
		}));
//  map(0x0ba, 0x0bd) PLL data
//  map(0x0c0, 0x0c1) parental control password
//  map(0x0c2, 0x0c2) channel bitmap
	map(0x0c2, 0x0ca).lr8(
		NAME([this] (offs_t offset) -> u8 {
			// this is a bitmap with the corresponding bit set when the TCU
			// detects a channel
			logerror("TCU channel bitmap read offset %02x\n", offset);
			//return 0x00; // No signal :(
			return 0xFF; // Signal everywhere :D
		}));
//  map(0x0cb, 0x0cb) parental control level
//  map(0x0cc, 0x0cc) initialized
//  map(0x0cd, 0x0ce) random seed, (d?)word
//  map(0x0cf, 0x0cf) next DRAM test block
//  map(0x0e0, 0x0e0) station
//  map(0x0e1, 0x0e1) slot
//  map(0x0e2, 0x0e2) service ID
//  map(0x0e3, 0x0e4) filter code
//  map(0x0e5, 0x0e5) channel number
//  map(0x0f3, 0x0f6) unit addr

//  map(0x1e0, 0x1ff) mini NVRAM?
//  the following are hard coded for now:
//  map(0x1e0, 0x1e0) language
//  map(0x1e1, 0x1e1) help communications port
//  map(0x1e2, 0x1e2) menu lptr
//  map(0x1e6, 0x1e6) tuner type (0) TD1A (1) TD1B
	map(0x1e0, 0x1ff).lrw8(
		NAME([this] (offs_t offset, u16 mem_mask) -> u8 {
			return m_nvm[0x1e0 + (offset & 0x1f)];
		}),
		NAME([this] (offs_t offset, u8 data, u16 mem_mask) {
			m_nvm[0x1e0 + (offset & 0x1f)] = data;
		})
	);
}

void megadrive_segach_us_device::load_packets()
{
	static const char* filename = "dom0816.bin";
	if (std::ifstream packet_stream{filename, std::ios::binary | std::ios::ate})
	{
		size_t num_pipes = packet_stream.tellg() / (256 * 10);
		size_t num_packets = num_pipes * 10;
		m_packet.resize(num_packets);
		packet_stream.seekg(0);
		for (size_t curr_packet = 0; curr_packet < num_packets; curr_packet++)
		{
#pragma pack(push, 1)
			struct {
				u16 file_id;
				u16 service_id;
				u32 game_time;
				u16 address;
				u16 data[246 / 2];
			} converted_packet;
#pragma pack(pop)
			if (packet_stream.read((char*) &converted_packet, 256))
			{
				m_packet[curr_packet].file_id = converted_packet.file_id;
				m_packet[curr_packet].service_id = converted_packet.service_id;
				m_packet[curr_packet].game_time = converted_packet.game_time;
				m_packet[curr_packet].address = converted_packet.address;
				std::copy_n(&converted_packet.data[0], 246/2, &m_packet[curr_packet].data[0]);
			}
		}
		logerror("packets loaded: %d, pipes: %d\n", num_packets, num_pipes);
	}
	else
	{
		logerror("packet stream could not be loaded\n");
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(megadrive_segach_us_device::send_packets)
{
	if (m_packet.size())
	{
		//logerror("current packet = %d\n", m_broadcast_packet);
		for (int pipe = 0; pipe < 10; pipe++)
		{
			auto& packet = m_packet[m_broadcast_packet];
			//if (m_gen_control & 0x0002)
				//logerror("broadcast %d, wanted game id %d, got %d...\n", m_broadcast_packet, m_game_id, packet.file_id);
			if ((m_gen_control & 0x0002) && packet.file_id == m_game_id)
			{
				m_curr_packet = packet.address;
				//logerror("current packet = %d (%d)\n", m_curr_packet, m_broadcast_packet);
				if (m_curr_packet == m_packet_match)
				{
					logerror("download complete\n");
					m_gen_status |= 1;
					m_gen_control &= ~(0x02);
				}
				else
				{
					size_t dram_address = m_curr_packet * (246 / 2);
					for (int data_i = 0; data_i < (246 / 2); data_i++)
						m_ram[(dram_address++) & 0x1fffff] = packet.data[data_i];
					if (m_curr_packet < m_packet_match)
						m_packet_match = m_curr_packet;
				}
			}
			m_broadcast_packet = (m_broadcast_packet + 1) % m_packet.size();
		}
	}
}


