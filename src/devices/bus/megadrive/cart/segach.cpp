// license: BSD-3-Clause
// copyright-holders: Angelo Salese, superctr, Nathan Misner
/**************************************************************************************************

Sega Channel

**************************************************************************************************/

#include <fstream>

#include "emu.h"
#include "segach.h"
#include "segach_img.h"

#include "bus/generic/slot.h"

/*
 * Sega Channel Game no Kanzume "digest" RAM cart, developed by CRI
 *
 * https://segaretro.org/Game_no_Kanzume_Otokuyou
 *
 * TODO:
 * - some unknowns, needs PCB picture;
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
 * Use -quik to load a broadcast IMG file, for example DOM0816.IMG
 * If no IMG is loaded, simulates no signal (you will get error 0005)
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
 * Holding A, B and/or C at Sega Channel loading logo screen will color cycle GFXs
 *
 * The BIOS also listens to controller port 2 in serial mode
 * for a diagnostics mode.
 *
 * TODO:
 * - Automatic reset timer (used for "Test Drive" demos)
 * - NVRAM
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_SEGACH_US, megadrive_segach_us_device, "megadrive_segach_us", "Megadrive Sega Channel US cart")

megadrive_segach_us_device::megadrive_segach_us_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_SEGACH_US, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, device_memory_interface( mconfig, *this )
	, m_packet_timer(*this, "packet_timer")
	, m_rom(*this, "rom")
	, m_space_tcu_config("tcu_io", ENDIANNESS_BIG, 8, 9, 0, address_map_constructor(FUNC(megadrive_segach_us_device::tcu_map), this))
	, m_sram_view(*this, "sram_view")
	, m_game_view(*this, "game_view")
	, m_game_sram_view(*this, "game_sram_view")
{
}

void megadrive_segach_us_device::device_add_mconfig(machine_config &config)
{
	// each broadcast frame has 10 packets and 288 bytes per packet
	// assuming 12Mbps download rate
	TIMER(config, "packet_timer", 0).configure_periodic(FUNC(megadrive_segach_us_device::send_packets),
			attotime::from_hz((12*128072) / 2880));

	QUICKLOAD(config, "packet_stream", "img").set_load_callback(FUNC(megadrive_segach_us_device::quickload_cb));
}

static INPUT_PORTS_START( megadrive_segach_us )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Menu") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(megadrive_segach_us_device::menu_pressed), 0)
INPUT_PORTS_END

ioport_constructor megadrive_segach_us_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( megadrive_segach_us );
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
	m_dram.resize(0x40'0000 / 2);
	m_sram.resize(0x2000);
	m_nvm.fill(0);

	m_broadcast_count = 0;

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
	map(0x00'0000, 0x0f'ffff).bankr(m_rom);

	// TODO: SSF banking
	map(0x10'0000, 0x4f'ffff).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) { return m_dram[offset]; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_dram[offset]); })
	);
	map(0x20'0000, 0x3f'ffff).view(m_sram_view);
	m_sram_view[0](0x20'0000, 0x3f'ffff).rw(FUNC(megadrive_segach_us_device::sram_r), FUNC(megadrive_segach_us_device::sram_w));

	map(0x00'0000, 0x3f'ffff).view(m_game_view);
	m_game_view[0](0x00'0000, 0x3f'ffff).lr16(
		// TODO: would need to confirm on real adapter if DRAM is locked in game mode.
		NAME([this] (offs_t offset, u16 mem_mask) { return m_dram[offset]; })
	);
	m_game_view[0](0x20'0000, 0x3f'ffff).view(m_game_sram_view);
	m_game_sram_view[0](0x20'0000, 0x3f'ffff).rw(FUNC(megadrive_segach_us_device::sram_r), FUNC(megadrive_segach_us_device::sram_w));
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
			// bit 0: reset console? (could also be bit 3)
			// bit 1: download enable
			// bit 2: packet match counter reset
			// bit 3: switch BIOS/game view (could also be bit 0)
			// bit 4: reset CRC
			// bit 5: enable fixit buffer
			// bit 8: memory configuration, set if 3MB detected?
			return m_gen_control;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			logerror("gen_control: %04x\n", data);
			m_gen_control = data & 0x12e;
			if (data & 0x01)
				logerror("console reset\n");
			if (data & 0x02)
				logerror("start download\n");
			if (data & 0x04)
				m_packet_match = 0x7fff; // side effect?
			if (data & 0x10)
				m_crc = 0;
			if ((data & 0x09) == 0x09)
			{
				// boot game
				m_game_view.select(0);
				m_slot->vres_w(1);
				m_slot->vres_w(0);
			}
		})
	);
//  map(0x34, 0x35) error counter
	map(0x40, 0x41).lw16( // CRC input
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			crc_write(data);
		})
	);
	map(0x42, 0x43).lr16( // CRC low out
		NAME([this] (offs_t offset, u16 mem_mask) -> u16 {
			return crc_read() & 0xffff;
		})
	);
	map(0x44, 0x45).lr16( // CRC high out
		NAME([this] (offs_t offset, u16 mem_mask) -> u16 {
			return crc_read() >> 16;
		})
	);
	map(0xf0, 0xf1).w(FUNC(megadrive_segach_us_device::sram_enable_w));
//  map(0xf0, 0xff) SSF style bankswitch?
}

void megadrive_segach_us_device::tcu_map(address_map &map)
{
	map(0x000, 0x00f).lr8( // authorized service map
		NAME([] () -> u8 {
			// this is a bitmap containing which service IDs are
			// authorized to play on the adapter.
			return 0xff;
		}));
	map(0x010, 0x01f).lr8( // free service map
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
	map(0x0b7, 0x0b7).lr8( // authorization byte
		NAME([] () -> u8 {
			// bit 7 is set if the cable company has authorized the adapter
			return 0x80; // 0x00..0x04 if unauthorized
		}));
//  map(0x0b8, 0x0b8) checksum
	map(0x0b9, 0x0b9).lr8( // PLL type
		NAME([] () -> u8 {
			return 0x81; // mitsubishi/philips
			// return 0x87; // rohm
		}));
//  map(0x0ba, 0x0bd) PLL data
	map(0x0c0, 0x0c1).lrw8( // parental control password
		NAME([this] (offs_t offset) -> u8 {
			if (!machine().side_effects_disabled())
				logerror("TCU: read parental control password %d: %02x\n", offset, m_nvm[0xc0 + offset]);
			return m_nvm[0xc0 + offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_nvm[0xc0 + offset] = data;
			logerror("TCU: write parental control password %d: %02x\n", offset, data);
			m_nvm[0xcb] = data;
		})
	);
	map(0x0c2, 0x0ca).lr8( // channel bitmap
		NAME([this] (offs_t offset) -> u8 {
			// this is a bitmap with the corresponding bit set when the TCU
			// detects a channel
			if (!machine().side_effects_disabled())
				logerror("TCU: read channel bitmap read offset %02x\n", offset);
			if (m_broadcast.size())
				return 0xFF; // Signal everywhere :D
			else
				return 0x00; // No signal :(
		}));
	map(0x0cb, 0x0cb).lrw8( // parental control level
		NAME([this] (offs_t offset) -> u8 {
			if (!machine().side_effects_disabled())
				logerror("TCU: read parental control level: %02x\n", m_nvm[0xcb]);
			return m_nvm[0xcb];
		}),
		NAME([this] (offs_t offset, u8 data) {
			logerror("TCU: write parental control level: %02x\n", data);
			m_nvm[0xcb] = data;
		})
	);
//  map(0x0cc, 0x0cc) initialized
//  map(0x0cd, 0x0ce) random seed, (d?)word
	map(0x0cd, 0x0ce).lrw8(
		NAME([this] (offs_t offset) -> u8 {
			return m_nvm[0xcd + offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_nvm[0xcd + offset] = data;
		})
	);
//  map(0x0cf, 0x0cf) next DRAM test block
//  map(0x0e0, 0x0e0) station ID
	map(0x0e0, 0x0e0).lw8(
		NAME([this] (offs_t offset, u8 data) {
			logerror("TCU: write station ID: %02x\n", data);
			// set the "logical channel"
			m_nvm[0xe5] = data;
		})
	);
//  map(0x0e1, 0x0e1) slot
//  map(0x0e2, 0x0e2) service ID
//  map(0x0e3, 0x0e4) filter code
//  map(0x0e5, 0x0e5) channel number
	map(0x0e5, 0x0e5).lr8(
		// After setting the station ID, the BIOS wants the "logical channel" to match
		// a certain value and it will keep tuning to find it, eventually timing out
		// if it isn't correct.
		// Rather than trying to calculate what the "logical channel" is,
		// I'm just bruteforcing a solution here...
		NAME([this] (offs_t offset) {
			uint8_t data = m_nvm[0xe5];
			if (!machine().side_effects_disabled())
			{
				logerror("TCU: read channel number: %02x\n", m_nvm[0xe5]);
				m_nvm[0xe5] = (data + 1) % 16;
			}
			return data;
		})
	);

//  map(0x0f3, 0x0f6) unit addr

//  map(0x1e0, 0x1ff) mini NVRAM?
//  the following are hard coded for now:
//  map(0x1e0, 0x1e0) language
//  map(0x1e1, 0x1e1) help communications port
//  map(0x1e2, 0x1e2) menu lptr
//  map(0x1e6, 0x1e6) tuner type (0) TD1A (1) TD1B
	map(0x1e0, 0x1ff).lrw8(
		NAME([this] (offs_t offset) -> u8 {
			return m_nvm[0x1e0 + offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_nvm[0x1e0 + offset] = data;
		})
	);
}

QUICKLOAD_LOAD_MEMBER(megadrive_segach_us_device::quickload_cb)
{
	auto helper = megadrive_segach_us_img();

	// Load packets stream
	size_t num_frames = image.length() / helper.FRAME_LEN;
	m_broadcast.resize(num_frames);

	struct game_file {
		int time;
		int time_buf;
		int time_bit;
	};
	std::map<u16, game_file> files;

	// first pass: get gametime bits
	for (size_t frame = 0; frame < num_frames; frame++)
	{
		u8 data[helper.FRAME_LEN];
		image.fread(&data[0], helper.FRAME_LEN);

		std::array<std::array<u8, helper.PACKET_LEN>, 10> pipes;
		helper.deweave(pipes, data);
		for (auto &pipe : pipes)
		{
			helper.deinterleave(pipe);

			u16 file_id = 0;
			helper.or_bits(&pipe[0], 39, (u8*) &file_id, 14);
			file_id = bitswap<14>(file_id, 0,1,2,3,4,5,6,7,8,9,10,11,12,13);

			u8 game_time_sync = 0;
			helper.or_bits(&pipe[0], 30, (uint8_t *)&game_time_sync, 1);
			u8 game_time_bit = 0;
			helper.or_bits(&pipe[0], 31, (uint8_t *)&game_time_bit, 1);

			if (!files.count(file_id))
			{
				game_file new_file;
				new_file.time = 0;
				new_file.time_buf = 0;
				new_file.time_bit = 0;
				files[file_id] = new_file;
			}
			auto &file = files[file_id];

			if (game_time_sync)
			{
				file.time = file.time_buf * 20;
				file.time_buf = 0;
				file.time_bit = 15;
			}
			if (file.time_bit >= 0)
			{
				file.time_buf |= game_time_bit << file.time_bit;
				file.time_bit--;
			}
		}
	}

	// second pass: copy everything else
	image.fseek(0, SEEK_SET);
	for (size_t frame = 0; frame < num_frames; frame++)
	{
		u8 data[helper.FRAME_LEN];
		image.fread(&data[0], helper.FRAME_LEN);

		std::array<std::array<u8, helper.PACKET_LEN>, 10> pipes;
		helper.deweave(pipes, data);

		size_t pipe_num = 0;
		for (auto& pipe : pipes)
		{
			helper.deinterleave(pipe);
			u8 data[helper.PACKET_DATA_LEN];

			u16 file_id = 0;
			helper.or_bits(&pipe[0], 39, (u8*) &file_id, 14);
			file_id = bitswap<14>(file_id, 0,1,2,3,4,5,6,7,8,9,10,11,12,13);

			u8 service_id = 0;
			helper.or_bits(&pipe[0], 32, (u8*) &service_id, 7);
			service_id = bitswap<7>(service_id, 0,1,2,3,4,5,6);

			u16 address = 0;
			helper.or_bits(&pipe[0], 53, (u8*) &address, 15);
			address = bitswap<15>(address, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14);

			helper.get_data(&pipe[0], &data[0]);

			auto& packet = m_broadcast[frame][pipe_num];
			packet.file_id = file_id;
			packet.service_id = service_id;
			packet.game_time = files[file_id].time;
			packet.address = address;

			for (size_t word_pos = 0; word_pos < helper.PACKET_DATA_LEN / 2; word_pos ++)
			{
				size_t byte_pos = word_pos * 2;
				packet.data[word_pos] = (data[byte_pos] << 8) | data[byte_pos + 1];
			}
			pipe_num++;
		}
	}

	logerror("broadcast frames loaded: %d\n", num_frames);
	return std::make_pair(std::error_condition(), std::string());
}

TIMER_DEVICE_CALLBACK_MEMBER(megadrive_segach_us_device::send_packets)
{
	if (m_broadcast.size())
	{
		//logerror("current packet = %d\n", m_broadcast_packet);
		for (int pipe = 0; pipe < 10; pipe++)
		{
			auto& packet = m_broadcast[m_broadcast_count][pipe];
			if ((m_gen_control & 0x0002) && packet.file_id == m_game_id)
			{
				// the adapter can probably not read all pipes at a time
				// the menu is broadcast on two pipes with offset addresses,
				// so skip the copy to prevent confusing the receiver.
				if (packet.file_id == 0 && pipe != 0)
					continue;
				m_curr_packet = packet.address;
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
						m_dram[(dram_address++) & 0x1fffff] = packet.data[data_i];
					if (m_curr_packet < m_packet_match)
						m_packet_match = m_curr_packet;
				}
			}
		}
		m_broadcast_count = (m_broadcast_count + 1) % m_broadcast.size();
	}
}

void megadrive_segach_us_device::sram_enable_w(offs_t offset, u16 data, u16 mem_mask)
{
	// the BIOS wants to switch in SRAM by writing bit 0...
	logerror("sram enable: %04x", data);
	if (data & 3)
	{
		m_sram_view.select(0);
		m_game_sram_view.select(0);
	}
	else
	{
		m_sram_view.disable();
		m_game_sram_view.disable();
	}
}

u16 megadrive_segach_us_device::sram_r(offs_t offset)
{
	const u32 sram_offset = offset & 0x1fff;
	return 0xff00 | m_sram[sram_offset];
}

void megadrive_segach_us_device::sram_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		const u32 sram_offset = offset & 0x1fff;
		m_sram[sram_offset] = data & 0xff;
	}
}


void megadrive_segach_us_device::crc_write(u16 data)
{
	u32 crc = m_crc;
	for (int i = 0; i < 16; i++)
	{
		u32 input_bit = (data & (0x8000 >> i)) ? 1 : 0;
		u32 crc_msb = (crc & 0x80000000) ? 1 : 0;
		if (input_bit == crc_msb)
		{
			crc = crc << 1;
		}
		else
		{
			crc = (crc << 1) ^ 0x04C11DB7;
		}
	}
	m_crc = crc;
}

u32 megadrive_segach_us_device::crc_read() const
{
	return bitswap<32>(m_crc, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
			16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
}

INPUT_CHANGED_MEMBER( megadrive_segach_us_device::menu_pressed )
{
	if (newval)
	{
		// boot BIOS.
		m_game_view.disable();
		m_slot->vres_w(1);
		m_slot->vres_w(0);
	}
}
