// license:BSD-3-Clause
// copyright-holders:Brice Onken

/*
 * Sony NWB-512 Monochrome Framebuffer and NWP-512D 15" Monochrome CRT Monitor
 *
 * The NWB-512 is a NEWS expansion card (board DSC-001) that provides a single-plane monochrome output at 816x1024
 * resolution. It has hardware-accelerated graphics, provided by the Sony CXD1029 raster operation controller, and pairs
 * with the NWP-512D 15" monochrome CRT display.
 *
 * This expansion card supports the following NEWS workstations:
 *  - All NWS-800/900 machines
 *  - NWS-1410/1510
 *  - PWS-1630
 *  - All NWS-1700/1800/1900 machines
 *  - All NWS-3800 machines
 *
 * Sources:
 *  - http://bitsavers.org/pdf/sony/news/1580/9-974-300-11_NWS-1510_1530_1580_EK_Model_Service_Manual_1989.pdf
 *     Note: The service manual for the 1500 series describes in detail how the raster operations generally work.
 *     However, while the newer framebuffers use a similar raster engine (using the WSC-SBLT that is an upgraded
 *     version of the CXD1029) and operate in a similar way, the surrounding details are different.
 *     I might not have captured all of those differences accurately yet.
 *  - http://bitsavers.org/pdf/sony/news/Sony_NEWS_Technical_Manual_3ed_199103.pdf
 *
 * TODO:
 *   - Add NEWS I/O Bus slot and convert this to it instead of regular device
 *   - Cycle-accurate timing
 *   - Interrupt support for CPU-only models (iopboot, mrx, and the monitor don't use them because they have the free cycles to poll)
 *   - Dimmer support
 *   - Refactoring for multiplanar framebuffer support (NWB-225A color framebuffer, etc.). The implementation uses the same raster operation controller,
 *     so the eventual drivers for those (when ROM dumps and board pictures are available) will be able to share code with the NWB-512.
 *   - Determine dimming amounts from real hardware
 */

#include "emu.h"
#include "nwb_512.h"

#define LOG_ROP_CONFIG 1 << 1
#define LOG_ROP 1 << 2
#define LOG_CTRL 1 << 3

#define VERBOSE (LOG_CTRL | LOG_ROP_CONFIG | LOG_GENERAL)
#include "logmacro.h"

#include <format>

DEFINE_DEVICE_TYPE(NWB_512, nwb_512_device, "nwb_512", "Sony NWB-512 monochrome framebuffer")

namespace
{
	// Total memory region is 2048 x 2048 pixels (512KiB)
	constexpr uint16_t X_MAX = 2048;
	constexpr uint16_t Y_MAX = 2048;

	// Takes in an x,y coordinate and returns the byte offset into the framebuffer memory region
	uint32_t coordinate_to_byte_offset(const uint16_t x, const uint16_t y)
	{
		return (y % Y_MAX) * 256 + ((x % X_MAX) >> 3);
	}

	// Takes in an x,y coordinate and returns the 16-bit aligned byte offset into the framebuffer memory region
	uint32_t coordinate_to_byte_offset_aligned(const uint16_t x, const uint16_t y)
	{
		return coordinate_to_byte_offset(x, y) & ~0x1;
	}

	// Gets the pixel offset of a given x coordinate
	uint8_t x_to_pixel_offset(const uint16_t x)
	{
		return 7 - (x & 7); // pixel 0 = bit 7
	}

	// Most registers on this card require word alignment.
	// This helper enforces as such to make it easier to validate assumptions made about this fact.
	void check_mem_mask(uint16_t mem_mask)
	{
		if (mem_mask != 0xffff)
		{
			fatalerror("fb: non-word write!");
		}
	}

	// Extracts the X address from the data provided to an FB write
	uint16_t x_address(uint16_t data)
	{
		return data & 0xfff; // When X is supplied, it is in D11-D0
	}

	// Extracts the Y address from the address provided to an FB write
	uint16_t y_address(uint16_t data) // word offset
	{
		return data & 0xfff; // When Y is supplied, it is in A12-A1, but is already shifted down because it is mapped in the memory map function at word width.
	}
}

nwb_512_device::nwb_512_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NWB_512, tag, owner, clock),
	  m_nwp_512d(*this, "nwp_512d"),
	  m_vram(*this, "vram")
{
}

void nwb_512_device::device_start()
{
	// TODO: save state support
}

void nwb_512_device::device_reset()
{
	reset_state();
}

void nwb_512_device::device_add_mconfig(machine_config &config)
{
	// Sony NWP-512D 15" Monochrome CRT Monitor
	constexpr uint16_t HORIZONTAL = 816;
	constexpr uint16_t VERTICAL = 1024;
	SCREEN(config, m_nwp_512d);
	m_nwp_512d->set_screen_update(FUNC(nwb_512_device::screen_update));
	m_nwp_512d->set_raw(HORIZONTAL * VERTICAL * 60,
						HORIZONTAL, 0, HORIZONTAL,
						VERTICAL, 0, VERTICAL);
	m_nwp_512d->set_physical_aspect(51, 64); // portrait mode CRT, square-ish pixels/~4:5 ratio assumed

	// VRAM consists of two types of memory chips:
	// 4x NEC uPD41264C-12 64K x 4 dual-port DRAM chips (visible screen and small off-screen area)
	// 12x Fujitsu MB81464-12 64K x 4 DRAM chips (off-screen buffer)
	RAM(config, m_vram);
	m_vram->set_default_value(0);
	m_vram->set_default_size("512KiB");
}

void nwb_512_device::map(address_map &map)
{
	map(0x00000, 0x0ffff).w(FUNC(nwb_512_device::destination_w)); // A13 = load shift, A14 = start execution, not sure about A15
	map(0x1a000, 0x1bfff).w(FUNC(nwb_512_device::pixel_w));
	map(0x20000, 0x27fff).w(FUNC(nwb_512_device::source_w));	 // A13 = load shift, A14 = start execution
	map(0x40000, 0x55fff).w(FUNC(nwb_512_device::limit_w));		 // A16 = source-space select (FB vs host), A14 = start execution
	map(0x60000, 0x60fff).w(FUNC(nwb_512_device::rop_config_w)); // rop configuration TODO: is the end addr correct here?
	map(0x70000, 0x70fff).rw(FUNC(nwb_512_device::fifo_r), FUNC(nwb_512_device::fifo_w));
	map(0x80000, 0x80001).rw(FUNC(nwb_512_device::status_r), FUNC(nwb_512_device::ctrl_w));
	map(0xa0000, 0xa0000).w(FUNC(nwb_512_device::planemask_w));

	map(0xc0000, 0xfffff).rom().region("krom", 0);
}

u32 nwb_512_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, [[maybe_unused]] const rectangle &cliprect)
{
	const rgb_t white = current_dim_color();

	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; ++y)
	{
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 8)
		{
			const uint32_t byte_offset = coordinate_to_byte_offset(x, y); // Y: 2048 / 8 = 256 bytes per row, not all visible.
			const uint8_t byte = m_vram->read(byte_offset);

			for (int i = 0; i < 8 && (x + i) <= screen.visible_area().max_x; ++i)
			{
				bitmap.pix(y, x + i) = byte & (1u << x_to_pixel_offset(x + i)) ? rgb_t::black() : white;
			}
		}
	}

	return 0;
}

void nwb_512_device::reset_state()
{
	// TODO: actually reset state
	m_state.busy = false;
	m_state = {};
}

void nwb_512_device::check_operation_start(offs_t offset)
{
	logerror("op arm: %-7s func=0x%x %s %s src=(0x%x,0x%x) dst=(0x%x,0x%x) lim=(0x%x,0x%x)\n",
			 (m_state.fb_read && m_state.fb_write) ? "FB->FB" : (!m_state.fb_read && m_state.fb_write) ? "host->FB"
															: (m_state.fb_read && !m_state.fb_write)   ? "FB->host"
																									   : "??",
			 m_state.rop_function,
			 m_state.x_direction ? "xfwd" : "xrev",
			 m_state.y_direction ? "yfwd" : "yrev",
			 m_state.source_x, m_state.source_y,
			 m_state.destination_x, m_state.destination_y,
			 m_state.limit_x, m_state.limit_y);

	if (!(offset & (0x4000 >> 1))) // word offset
	{
		return;
	}

	LOG("Operation started\n");

	m_state.busy = true;
	// Adjust source and destination coordinates for the reverse direction if necessary,
	// because the left edge is always inclusive (right edge exclusive).
	if (!m_state.x_direction)
	{
		--m_state.destination_x;
		--m_state.source_x;
		--m_state.limit_x;
	}

	m_state.current_destination_x = m_state.destination_x;

	const uint16_t width = m_state.x_direction ? m_state.limit_x - m_state.destination_x  // fwd: [dest, limit)
											   : m_state.destination_x - m_state.limit_x; // rev: [limit, dest)
	const uint16_t src_phase = m_state.x_direction ? m_state.source_x & 15 : 15 - (m_state.source_x & 15);
	const uint16_t dest_phase = m_state.x_direction ? m_state.destination_x & 15 : 15 - (m_state.destination_x & 15);

	m_state.prime = src_phase > dest_phase;
	m_state.prime_pending = m_state.prime;
	m_state.source_words_per_row = (src_phase + width + 15) >> 4;
	m_state.dest_words_per_row = (dest_phase + width + 15) >> 4;
	m_state.source_words_left = m_state.source_words_per_row;
	m_state.dest_words_left = m_state.dest_words_per_row;
	LOG("Source words per row: %d, Destination words per row: %d\n", m_state.source_words_per_row, m_state.dest_words_per_row);

	if (std::abs(m_state.dest_words_per_row - m_state.source_words_per_row) > 1)
	{
		fatalerror("nwb512: source (0x%x) and destination (0x%x) words per row not aligned!", m_state.dest_words_per_row, m_state.source_words_per_row);
	}

	// Update masks to preserve framebuffer contents that shouldn't be modified
	uint16_t x_lo, x_hi;
	if (m_state.x_direction)
	{
		x_lo = m_state.destination_x;
		x_hi = m_state.limit_x - 1;
	}
	else
	{
		x_lo = m_state.limit_x + 1;
		x_hi = m_state.destination_x;
	}
	m_state.mask1 = ~(0xffff >> (x_lo & 0xf));
	m_state.mask2 = 0xffff >> ((x_hi & 0xf) + 1);

	// TODO: This executes copies immediately, faster than real hardware.
	// It also doesn't use the proper registers
	if (m_state.fb_read && m_state.fb_write)
	{
		uint16_t source_x = m_state.source_x;
		uint16_t source_y = m_state.source_y;

		while (m_state.busy)
		{
			const uint16_t source_data = coordinate_to_data(source_x, source_y);

			const uint16_t destination_y_snapshot = m_state.destination_y; // if the row is advanced, we need to reset the source - TODO: should this be in end_row?
			LOGMASKED(LOG_ROP, "ROP for sx = 0x%x sy = 0x%x data = 0x%x\n", source_x, source_y, source_data);
			rop_cycle(source_data); // When the ROP is done, this function will clear m_state.busy

			if (m_state.destination_y != destination_y_snapshot)
			{
				source_x = m_state.source_x;
				source_y += m_state.y_direction ? 1 : -1;
			}
			else
			{
				source_x += m_state.x_direction ? 16 : -16;
			}
		}
	}
}

void nwb_512_device::destination_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	check_mem_mask(mem_mask);

	m_state.destination_x = x_address(data);
	m_state.destination_y = y_address(offset);
	LOG("Destination set to (0x%x, 0x%x) x word offset = 0x%x\n", m_state.destination_x, m_state.destination_y, (m_state.destination_x - 1) >> 4);

	if (offset & (0x2000 >> 1))
	{
		// Determine how much we need to shift source by to align it with destination
		// TODO: double-check that the math works here for both xfwd and xrev
		m_state.shift = (m_state.destination_x - m_state.source_x) & 15;
		LOG("destination_w: Calculated shift: 0x%x - 0x%x & 15 = 0x%x\n", m_state.destination_x, m_state.source_x, m_state.shift);
	}

	check_operation_start(offset);
}

void nwb_512_device::source_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("source_w(0x%x, 0x%x, 0x%x)\n", offset, data, mem_mask);
	check_mem_mask(mem_mask);

	m_state.source_x = x_address(data);
	m_state.source_y = y_address(offset);

	if (offset & (0x2000 >> 1))
	{
		// how much do we need to shift source by to align it with destination?
		// TODO: double-check that the math works here for both xfwd and xrev
		m_state.shift = (m_state.destination_x % 16 - m_state.source_x % 16) & 0xf; // direction determined by x direction
		LOG("source_w: Calculated shift: 0x%x % 16 - 0x%x % 16 = 0x%x\n", m_state.destination_x, m_state.source_x, m_state.shift);
	}

	LOG("Source set to (0x%x, 0x%x)\n", m_state.source_x, m_state.source_y);
	check_operation_start(offset);
}

void nwb_512_device::limit_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	check_mem_mask(mem_mask);
	LOG("limit_w(0x%x, 0x%x, 0x%x)\n", offset, data, mem_mask);

	// TODO: this feels like a hack
	const bool op_start = offset & (0x4000 >> 1);
	const bool fb_read = op_start ? offset & (0x10000 >> 1) : true;
	const bool fb_write = op_start;
	if (fb_read != m_state.fb_read || fb_write != m_state.fb_write)
	{
		LOG("limit_w: override arm=%d%d -> limit=%d%d\n", m_state.fb_read, m_state.fb_write, fb_read, fb_write);
	}
	m_state.fb_read = fb_read;
	m_state.fb_write = fb_write;

	// if (m_state.fb_read != fb_read)
	// {
	// 	m_state.fb_read = fb_read;
	// 	LOG("limit_w override for source: offset = 0x%x fbread = %s fbwrite = %s\n", offset, m_state.fb_read ? "true" : "false", m_state.fb_write ? "true" : "false");
	// }

	m_state.limit_x = x_address(data);
	m_state.limit_y = y_address(offset);
	// const uint16_t last_pixel = (m_state.limit_x - 1) & 0xf;     // last pixel's position in the last word
	// m_state.mask2 = 0xffff >> (last_pixel + 1);        // preserve everything at/after the right edge

	LOG("Limit set to (0x%x, 0x%x) x word offset = 0x%x\n", m_state.limit_x, m_state.limit_y, (m_state.limit_x - 1) >> 4);
	check_operation_start(offset);
}

void nwb_512_device::shift_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	check_mem_mask(mem_mask);

	m_state.shift = data;
	LOG("Shift set to 0x%x\n", m_state.shift);
}

void nwb_512_device::rop_config_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	check_enabled();
	check_mem_mask(mem_mask);

	LOGMASKED(LOG_ROP_CONFIG, "(%s) rop_config_w(0x%x, 0x%x) = ", machine().describe_context(), offset, data);

	// These mappings are an educated guess based on NWS-1500 service manual and iopboot/mrx behavior
	offset <<= 1; // use byte offset
	m_state.suppress_advance = offset & 0x4;
	m_state.x_direction = offset & 0x10;
	m_state.y_direction = offset & 0x20;
	m_state.row_fill = offset & 0x80;
	m_state.fb_read = offset & 0x100;
	m_state.fb_write = offset & 0x200;
	m_state.pixel_mode = offset & 0x800;

	if (offset & 0x400) // If set, load ROP function
	{
		m_state.rop_function = data & 0xf;
	}

	LOGMASKED(LOG_ROP_CONFIG, "%s%s%s%s%s%s%s\n",
			  m_state.fb_write ? "fbwrite " : "",
			  m_state.fb_read ? "fbread " : "",
			  m_state.row_fill ? "rowfill " : "",
			  m_state.y_direction ? "yfwd " : "yrev ",
			  m_state.x_direction ? "xfwd " : "xrev ",
			  m_state.suppress_advance ? "supadv " : "",
			  m_state.pixel_mode ? "pixel " : "word ",
			  offset & 400 ? "func=0x" + std::format("{:x}", m_state.rop_function) : "");

	// TODO: this check causes issues, maybe the ROP engine can be configured on the fly?
	//       Should also check with not resetting everything when the card is disabled,
	//       and see if that means this check can come back
	if (m_state.busy)
	{
		LOG("IOP tried to reconfigure while op was in progress\n");
		LOG("limit_y = 0x%x dest_words_left = 0x%x source_words_left = 0x%x destination_y = 0x%x\n", m_state.limit_y, m_state.dest_words_left, m_state.source_words_left, m_state.destination_y);
		// fatalerror("IOP tried to reconfigure while op was in progress!");
	}
}

void nwb_512_device::pixel_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// Set pixel to 1 at X (data), Y (offset[12:1])
	check_mem_mask(mem_mask);
	if (!m_state.pixel_mode)
	{
		fatalerror("Pixel write in word mode");
	}

	const uint16_t x = x_address(data);
	const uint16_t y = y_address(offset);
	const uint32_t byte_offset = coordinate_to_byte_offset(x, y);
	const uint16_t x_bit = x_to_pixel_offset(x);
	const uint8_t destination = m_vram->read(byte_offset) & (1u << x_bit) ? 1 : 0;

	// Use the result of the ROP engine before setting the pixel
	// TODO: consolidate set_pixel into this function, it is only used here.
	const bool new_state = execute_rop(m_state.rop_function, 1, destination) & 1;
	LOG("pixel_w(0x%x, 0x%x) => Change (0x%x, 0x%x) from %u to %u\n", offset, data, x, y, destination, new_state ? 1u : 0u);
	set_pixel(x, y, new_state);
}

uint16_t nwb_512_device::execute_rop(const uint8_t opcode, const uint16_t source, const uint16_t destination)
{
	uint16_t result;
	[[maybe_unused]] std::string op;

	switch (opcode)
	{
	case 0x0:
		op = "~S";
		result = ~source;
		break;
	case 0x1:
		op = "~(S | D)";
		result = ~(source | destination);
		break;
	case 0x2:
		op = "~S & D";
		result = ~source & destination;
		break;
	case 0x3:
		op = "0";
		result = 0;
		break;
	case 0x4:
		op = "~(S & D)";
		result = ~(source & destination);
		break;
	case 0x5:
		op = "~D";
		result = ~destination;
		break;
	case 0x6:
		op = "S ^ D";
		result = source ^ destination;
		break;
	case 0x7:
		op = "S & ~D";
		result = source & ~destination;
		break;
	case 0x8:
		op = "~S | D";
		result = ~source | destination;
		break;
	case 0x9:
		op = "~(S ^ D)";
		result = ~(source ^ destination);
		break;
	case 0xa:
		op = "D";
		result = destination;
		break;
	case 0xb:
		op = "S & D";
		result = source & destination;
		break;
	case 0xc:
		op = "1";
		result = 0xffff;
		break;
	case 0xd:
		op = "S | ~D";
		result = source | ~destination;
		break;
	case 0xe:
		op = "S | D";
		result = source | destination;
		break;
	case 0xf:
		op = "S";
		result = source;
		break;
	default:
		fatalerror("nwb_512: invalid ROP function code 0x%x", opcode);
	}

	LOGMASKED(LOG_ROP, "execute_rop(0x%x/%s, 0x%x, 0x%x) = 0x%x\n", opcode, op, source, destination, result);
	return result;
}

uint16_t nwb_512_device::shift(uint16_t data)
{
	// Calculate source by shifting bits and update carryover
	// TODO: clean this up
	uint16_t result;
	if (m_state.x_direction)
	{
		result = uint16_t(data >> m_state.shift | m_state.carry);
		m_state.carry = uint16_t(uint32_t(data) << (16 - m_state.shift) & 0xffff);
	}
	else
	{
		// TODO: might be better to calculate shift differently based on rev flag instead, mainly if the computation of state can be centralized
		//       But, the direction is still different, so doing the computation in the state update doesn't get rid of this if/else
		const uint16_t shift_complement = (16 - m_state.shift) & 0xf;
		result = uint16_t(data << shift_complement | m_state.carry);
		m_state.carry = uint16_t(uint32_t(data) >> (16 - shift_complement) & 0xffff);
	}

	return result;
}

void nwb_512_device::rop(uint16_t source)
{
	const offs_t byte_offset = coordinate_to_byte_offset_aligned(m_state.current_destination_x, m_state.destination_y);
	const uint16_t destination = coordinate_to_data(m_state.current_destination_x, m_state.destination_y);

	// Execute raster operation with the appropriate source and destination data
	uint16_t result = execute_rop(m_state.rop_function, source, destination);

	LOG("rop_cycle:\n");
	LOG("\tx,y(0x%x, 0x%x) => offs 0x%x\n", m_state.current_destination_x, m_state.destination_y, byte_offset);
	LOG("\tshift = 0x%x, source = 0x%x, carry = 0x%x, destination = 0x%x\n", m_state.shift, source, m_state.carry, destination);
	LOG("\trop result = 0x%x\n", result);
	const bool first_word = m_state.dest_words_left == m_state.dest_words_per_row;
	const bool last_word = m_state.dest_words_left == 1;

	if (first_word) // first word of row
	{
		const uint16_t mask = m_state.x_direction ? m_state.mask1 : m_state.mask2;
		result = (result & ~mask) | (destination & mask);
	}

	if (last_word) // last word of row
	{
		const uint16_t mask = m_state.x_direction ? m_state.mask2 : m_state.mask1;
		result = (result & ~mask) | (destination & mask);
	}

	// Write result back to VRAM
	// LOG("\tdestination (0x%x, 0x%x) => 0x%x = 0x%x 0x%x\n", m_state.current_destination_x, m_state.destination_y, byte_offset, result >> 8 & 0xff, result & 0xff);
	m_vram->write(byte_offset, result >> 8 & 0xff);
	m_vram->write(byte_offset + 1, result & 0xff);

	// Advance ROCC state
	m_state.current_destination_x += m_state.x_direction ? 16 : -16;
	--m_state.dest_words_left;
}

void nwb_512_device::end_row()
{
	// LOG("\tdone with row, advancing y\n");
	m_state.current_destination_x = m_state.destination_x;
	m_state.current_source_x = m_state.source_x;
	m_state.destination_y += m_state.y_direction ? 1 : -1;
	m_state.source_y += m_state.y_direction ? 1 : -1;
	m_state.carry = 0;

	if (!m_state.limit_y)
	{
		LOG("\tROP done!\n");
		m_state.busy = false; // TODO: use granular states from DSC-008 description?
		m_state.source_x = 0;
		m_state.destination_x = 0;
	}
	else
	{
		m_state.dest_words_left = m_state.dest_words_per_row;
		m_state.source_words_left = m_state.source_words_per_row;
		m_state.prime_pending = m_state.prime;
		m_state.limit_y--;
		// LOG("\tdecremented limit to 0x%x\n", m_state.limit_y);
	}
}

void nwb_512_device::rop_cycle(uint16_t data)
{
	if (m_state.prime_pending)
	{
		// If the source row has more data than the dest row, all we need to do is prime the shifter so carry is correct.
		// After this, the source and destination counts will be aligned.
		logerror("Applying primed extra read!\n");
		shift(data);
		m_state.prime_pending = false;
		--m_state.source_words_left;
		return;
	}

	data = shift(data);
	rop(data);
	m_state.source_words_left--;

	if (m_state.source_words_left == 0 && m_state.dest_words_left > 0)
	{
		rop(m_state.carry); // shifter remainder as the transaction is complete, nothing further from the source
	}

	// LOG("dest words left = 0x%x\n", m_state.dest_words_left);
	if (m_state.dest_words_left == 0)
	{
		end_row(); // reset X, step Y, rearm mask1, clear carry, maybe finish op
	}
}

uint16_t nwb_512_device::fifo_r(offs_t offset, uint16_t mem_mask)
{
	LOG("(%s) fifo_r(0x%x, 0x%x)\n", machine().describe_context(), offset, mem_mask);
	check_mem_mask(mem_mask);
	if (m_state.fb_write || !m_state.fb_read)
	{
		fatalerror("Wrong mode for read");
	}
	if (!m_state.busy)
	{
		fatalerror("No words left in op!\n");
	}

	uint16_t word = 0;

	if (m_state.prime_pending)
	{
		// If the source row has more data than the dest row, prime the shifter
		// After this, the source and destination counts will be aligned.
		logerror("Applying primed extra read!\n");
		word = coordinate_to_data(m_state.current_source_x, m_state.source_y);
		shift(word);
		m_state.prime_pending = false;
		m_state.current_source_x += m_state.x_direction ? 16 : -16;
	}

	// word = shift(sxa, sya)
	word = coordinate_to_data(m_state.current_source_x, m_state.source_y);
	LOG("  fifo_r: word before shift = 0x%x\n", word);
	word = shift(word);
	LOG("  fifo_r: word after shift = 0x%x\n", word);

	// Advance ROCC state
	m_state.current_source_x += m_state.x_direction ? 16 : -16;
	m_state.current_destination_x += m_state.x_direction ? 16 : -16;
	--m_state.dest_words_left;
	LOG("  fifo_r: advanced source to %d, destination to %d, and there are %d words left\n", m_state.current_source_x, m_state.current_destination_x, m_state.dest_words_left);

	// if (dxa > lxa) end row (reload X, Y++, carry = 0, re-prime if needed)
	if (m_state.dest_words_left == 0)
	{
		LOG("  fifo_r: calling end row\n", word);
		end_row();
	}

	return word;
}

void nwb_512_device::fifo_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("(%s) fifo_w(0x%x, 0x%x, 0x%x)\n", machine().describe_context(), offset, data, mem_mask);
	check_mem_mask(mem_mask);
	if (m_state.fb_read && !m_state.fb_write)
	{
		LOG("Start fbread op\n");
		// A write to this port when armed for reading starts the process
		m_state.busy = true; // TODO: exact value

		// The DSC-008 section of the NWS-15xx service manual implies that the direction bit is not
		// supported/used for reads. For now, assume the same for the NWB-512.
		if (!m_state.x_direction)
		{
			fatalerror("Reverse reads are not supported!");
		}

		m_state.current_destination_x = m_state.destination_x;
		m_state.current_source_x = m_state.source_x;

		const uint16_t src_phase = m_state.x_direction ? m_state.source_x & 15 : 15 - (m_state.source_x & 15);
		const uint16_t dest_phase = m_state.x_direction ? m_state.destination_x & 15 : 15 - (m_state.destination_x & 15);

		// The row end logic is determined from the destination (which must be aligned with the source)
		m_state.prime = src_phase > dest_phase;
		m_state.prime_pending = m_state.prime;
		m_state.dest_words_per_row = (m_state.limit_x >> 4) - (m_state.source_x >> 4) + (m_state.prime ? 0 : 1);
		m_state.dest_words_left = m_state.dest_words_per_row;
		LOG("Destination words per row: %d\n", m_state.dest_words_per_row);

		return;
	}

	if (m_state.pixel_mode)
	{
		fatalerror("word write in pixel mode");
	}

	if (!m_state.busy)
	{
		fatalerror("IOP tried to push data without starting an op!");
	}

	// TODO: validate offset versus the setup in the ROP reg

	if (!m_state.row_fill)
	{
		// LOG("fb_fifo_w: pushing unpacked word 0x%x!\n", data);
		rop_cycle(data);
	}
	else
	{
		const uint16_t word_count = ((m_state.limit_x + 0xf) >> 4) - (m_state.destination_x >> 4); // +0xf is ceiling to prevent off-by-ones for unaligned ops
		// LOG("fb_fifo_w: pushing 0x%x %d times to row 0x%x vs limit 0x%x\n", data, word_count, m_state.destination_y, m_state.limit_y);

		for (uint16_t i = 0; i < word_count; i++)
		{
			rop_cycle(data);
		}
	}
}

void nwb_512_device::set_pixel(uint16_t x, uint16_t y, bool on)
{
	// Determine byte offset in VRAM
	const uint32_t byte_offset = coordinate_to_byte_offset(x, y);
	const uint16_t x_bit = x_to_pixel_offset(x);

	// Set desired pixel
	uint8_t pixel = m_vram->read(byte_offset);
	pixel = (pixel & ~(1u << x_bit)) | ((on ? 1u : 0u) << x_bit);
	m_vram->write(byte_offset, pixel);
}

uint16_t nwb_512_device::status_r()
{
	return 0x20 | (m_state.busy ? 0x3 : 0x0);
}

void nwb_512_device::ctrl_w(uint16_t data)
{
	// NWS-1580 uses a newer version of the framebuffer that uses a similar ROP engine, these definitions
	// can be found in the NWS-1580 service manual.
	const bool enabled = data & 0x1;
	LOGMASKED(LOG_CTRL, "Write FB control = 0x%x (%s)\n", data, machine().describe_context());
	LOGMASKED(LOG_CTRL, "\t%s framebuffer\n", enabled ? "enable" : "reset");
	LOGMASKED(LOG_CTRL, "\t%s kanji ROM\n", data & 0x2 ? "enable" : "disable"); // TODO: not for first-gen fb?
	LOGMASKED(LOG_CTRL, "\t%s v-sync interrupt\n", data & 0x8 ? "enable" : "disable");
	LOGMASKED(LOG_CTRL, "\t%s command completion interrupt\n", data & 0x10 ? "enable" : "disable");
	LOGMASKED(LOG_CTRL, "\tunknown %s\n", data & 0x20 ? "set" : "cleared");
	LOGMASKED(LOG_CTRL, "\tdimmer setting = 0x%x\n", (data & 0xC0) >> 6);

	if (!enabled)
	{
		reset_state();
	}
	m_state.ctrl = data;
}

void nwb_512_device::planemask_w(uint8_t data)
{
	LOG("Write plane mask = 0x%x\n", data);
}

uint16_t nwb_512_device::coordinate_to_data(uint16_t x, uint16_t y)
{
	const uint32_t byte_offset = coordinate_to_byte_offset_aligned(x, y);
	return (m_vram->read(byte_offset) << 8) | m_vram->read(byte_offset + 1);
}

rgb_t nwb_512_device::current_dim_color()
{
	// Assume darkest dim is 25% brightness (0b11) with 50% (0b10) and 75% (0b01) as the other options.
	// The DSC-001 dim functions in iopboot/mrx write 0b11 to bits 7:6 of the status register when the
	// sxsession screen saver is active. The NWS-15xx service manual backs this up for the DSC-007 from 0b00 (brightest) -> 0b11 (darkest),
	// but does not elaborate on the exact degree of dimming.
	const uint8_t dim = (m_state.ctrl & 0xC0) >> 6;
	if (!dim)
	{
		return rgb_t::white();
	}

	const uint8_t gray_value = 64 * (4 - dim);
	return rgb_t(gray_value, gray_value, gray_value);
}

ROM_START(nwb_512)
// NWB-512 Kanji ROM
ROM_REGION32_BE(0x40000, "krom", 0)
ROM_LOAD16_BYTE("nwb512-ic128-hn62301ap-aa1.bin", 0x0000, 0x20000, CRC(3eaa0dbc) SHA1(9c4e66cc122fd60963f51649c945cdfc39f230eb))
ROM_LOAD16_BYTE("nwb512-ic127-hn62301ap-aa2.bin", 0x0001, 0x20000, CRC(b8705af7) SHA1(915333f082ecbfee512f1166fc38d9dd29dcaf26))
ROM_END

const tiny_rom_entry *nwb_512_device::device_rom_region() const
{
	return ROM_NAME(nwb_512);
}
