// license:BSD-3-Clause
// copyright-holders:Brice Onken

// Sony NWB-512 Monochrome Framebuffer and NWP-512D 15" Monochrome CRT Monitor
// The NWB-512 is a NEWS expansion card (model DSC-001) that provides a single-plane monochrome output at 816x1024
// resolution. It has hardware-accelerated graphics, provided by the Sony CXD1029 raster operation controller, and pairs
// with the NWP-512D 15" monochrome CRT display.

#include "emu.h"
#include "nwb_512.h"

#define LOG_FB_FB 1 << 1
#define LOG_ROP_CONFIG 1 << 2
#define LOG_ROP 1 << 3

#define VERBOSE (LOG_GENERAL|LOG_FB_FB|LOG_ROP_CONFIG)
#include "logmacro.h"

#include <format>

DEFINE_DEVICE_TYPE(NWB_512,  nwb_512_device,  "nwb_512",  "Sony NWB-512 monochrome framebuffer")


nwb_512_device::nwb_512_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NWB_512, tag, owner, clock),
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
	// NWP-512D 15" Monochrome CRT Monitor
	constexpr uint16_t HORIZONTAL = 816; // make this 1024 instead of 816 to view offscreen area
	constexpr uint16_t VERTICAL = 1024;
	SCREEN(config, m_nwp_512d);
	m_nwp_512d->set_screen_update(FUNC(nwb_512_device::screen_update));
	m_nwp_512d->set_raw(HORIZONTAL * VERTICAL * 60,
	                    HORIZONTAL, 0, HORIZONTAL,
	                    VERTICAL, 0, VERTICAL);

	// VRAM consists of two types of memory chips:
	// 4x NEC uPD41264C-12 64K x 4 dual-port DRAM chips (visible screen and small off-screen area)
	// 12x Fujitsu MB81464-12 64K x 4 DRAM chips (off-screen buffer)
	RAM(config, m_vram);
	m_vram->set_default_value(0);
	m_vram->set_default_size("512KiB");
}

void nwb_512_device::map(address_map &map)
{
	map(0x02000, 0x02001).w(FUNC(nwb_512_device::shift_w)); // shift count when only "load shift" is asserted
	map(0x08000, 0x0ffff).w(FUNC(nwb_512_device::destination_w)); // A13 = load shift, A14 = start execution
	map(0x20000, 0x27fff).w(FUNC(nwb_512_device::source_w)); // A13 = load shift, A14 = start execution
	map(0x40000, 0x55fff).w(FUNC(nwb_512_device::limit_w)); // A16 = source-space select (FB vs host), A14 = start execution
	map(0x1a000, 0x1bfff).w(FUNC(nwb_512_device::pixel_w));
	map(0x60000, 0x60fff).w(FUNC(nwb_512_device::rop_config_w)); // rop configuration TODO: is the end addr correct here?
	map(0x70000, 0x70fff).w(FUNC(nwb_512_device::fifo_w)); // data ports
	map(0x80000, 0x80001).rw(FUNC(nwb_512_device::status_r), FUNC(nwb_512_device::ctrl_w));
	map(0xa0000, 0xa0000).w(FUNC(nwb_512_device::planemask_w));

	map(0xc0000, 0xfffff).rom().region("krom", 0);
}

u32 nwb_512_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, [[maybe_unused]] const rectangle &cliprect)
{
	// TODO: optimize this to read one byte at a time rather than bit
	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
	{
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x++)
		{
			const uint32_t byte_offset = y * 256 + (x >> 3); // Y: 2048 / 8 = 256 bytes per row, not all visible.
			const uint16_t x_bit = 7 - (x & 7); // pixel 0 = bit 7

			// Set desired pixel
			uint8_t pixel = m_vram->read(byte_offset);
			pixel = pixel & (1u << x_bit);
			bitmap.pix(y, x) = pixel ? rgb_t::white() : rgb_t::black();
			// bitmap.pix(y, x) = pixel ? rgb_t::black() : rgb_t::white();

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

uint16_t x_address(uint16_t data)
{
	return data & 0xfff; // When X is supplied, it is in D11-D0
}

uint16_t y_address(uint16_t data) // word offset
{
	return data & 0xfff; // When Y is supplied, it is in A12-A1, but is already shifted down.
}

void check_mem_mask(uint16_t mem_mask)
{
	if (mem_mask != 0xffff)
	{
		fatalerror("fb: non-word write!");
	}
}

void nwb_512_device::check_operation_start(offs_t offset)
{
	if (m_state.fb_read && !m_state.fb_write)
	{
		fatalerror("nwb512: fb read only not implemented yet!");
	}

	if (offset & (0x4000 >> 1)) // word offset
	{
		LOG("Operation started\n");
		m_state.busy = true;
		m_state.current_x = m_state.destination_x;

		const uint16_t width = m_state.x_direction ? m_state.limit_x - m_state.destination_x // fwd: [dest, limit)
												: m_state.destination_x - m_state.limit_x; // rev TODO: double-check this
		const uint16_t src_phase  = m_state.fb_read && m_state.fb_write ?
										m_state.x_direction ? m_state.source_x & 15 : 15 - (m_state.source_x & 15) :
										0;
		const uint16_t dest_phase = m_state.x_direction ? m_state.destination_x & 15 : 15 - (m_state.destination_x & 15);
		m_state.source_words_per_row = (src_phase  + width + 15) >> 4;
		m_state.dest_words_per_row = (dest_phase + width + 15) >> 4;
		m_state.source_words_left = m_state.source_words_per_row;
		m_state.dest_words_left = m_state.dest_words_per_row;

		if (m_state.fb_read && m_state.fb_write &&
			m_state.dest_words_per_row != m_state.source_words_per_row &&
		    m_state.dest_words_per_row != m_state.source_words_per_row + 1 &&
		    m_state.dest_words_per_row != m_state.source_words_per_row - 1)
		{
			fatalerror("nwb512: fb->fb copy set but source (0x%x) and destination (0x%x) words per row not aligned!", m_state.dest_words_per_row, m_state.source_words_per_row);
		}

		// TODO: This executes copies at once, faster than real hardware. Needs more work to be cycle-accurate.
		// TODO: make m_state.fb_read only work
		if (m_state.fb_read && m_state.fb_write)
		{
			const int dx = int(m_state.source_x) - int(m_state.destination_x);
			const int dy = int(m_state.source_y) - int(m_state.destination_y);
			m_state.current_x = m_state.destination_x;

			unsigned panic = 0;
			while (m_state.busy && panic < 0xf0000)
			{
				const uint16_t sx = m_state.current_x + dx;
				const uint16_t sy = m_state.destination_y + dy;
				const uint32_t byte_offset = sy * 256 + ((sx >> 4) << 1);
				const uint16_t data = (m_vram->read(byte_offset) << 8) | m_vram->read(byte_offset + 1);
				// LOGMASKED(LOG_FB_FB, "ROP for sx = 0x%x sy = 0x%x byte_offset = 0x%x data = 0x%x panic = 0x%x\n", sx, sy, byte_offset, data, panic);
				rop_cycle(data); // When the ROP is done, this function will clear m_state.busy
				++panic;
			}
			if (panic >= 0xf0000)
			{
				fatalerror("yikes bro");
				// machine().debug_break();
			}
		}
		else if (m_state.fb_read && !m_state.fb_write)
		{
			fatalerror("nwb_512: fb read only not implemented");
		}
	}
}

void nwb_512_device::destination_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	check_mem_mask(mem_mask);

	m_state.destination_x = x_address(data);
	m_state.destination_y = y_address(offset);

	if (offset & (0x2000 >> 1))
	{
		// how much do we need to shift source by to align it with destination?
		m_state.shift = (m_state.destination_x % 16 - m_state.source_x % 16) & 0xf;
		LOG("destination_w: Calculated shift: 0x%x % 16 - 0x%x % 16 = 0x%x\n", m_state.destination_x, m_state.source_x, m_state.shift);
	}

	// Compute first mask
	m_state.mask1 = ~(0xffff >> (m_state.destination_x % 16)); // determine first mask based on bit offset in first word TODO: direction?

	LOG("Destination set to (0x%x, 0x%x) x word offset = 0x%x\n", m_state.destination_x, m_state.destination_y, (m_state.destination_x - 1) >> 4);
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

	// TODO: this probably applies to other register writes
	const bool fb_read = offset & (0x10000 >> 1);
	if (m_state.fb_read != fb_read)
	{
		m_state.fb_read = fb_read;
		LOG("limit_w override for source: offset = 0x%x fbread = %s fbwrite = %s\n", offset, m_state.fb_read ? "true" : "false", m_state.fb_write ? "true" : "false");
	}

	m_state.limit_x = x_address(data);
	m_state.limit_y = y_address(offset);
	const uint16_t last_pixel = (m_state.limit_x - 1) & 0xf;     // last pixel's position in the last word
	m_state.mask2 = 0xffff >> (last_pixel + 1);        // preserve everything at/after the right edge

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

	if (m_state.busy)
	{
		// `m_state.limit_y`, `m_state.dest_words_left`,
		// `m_state.source_words_left`, `m_state.destination_y`, and a running count of
		// `fifo_w`
		LOG("IOP tried to reconfigure while op was in progress\n");
		LOG("limit_y = 0x%x dest_words_left = 0x%x source_words_left = 0x%x destination_y = 0x%x fifo write count = 0x%x\n", m_state.limit_y, m_state.dest_words_left, m_state.source_words_left, m_state.destination_y, m_debug.fifo_call_count);
		fatalerror("IOP tried to reconfigure while op was in progress!");
	}

	m_debug = {}; // clear any leftover debug data

	LOGMASKED(LOG_ROP_CONFIG, "(%s) rop_config_w(0x%x, 0x%x) = ", machine().describe_context(), offset, data);

	// These mappings are an educated guess based on NWS-1500 service manual and iopboot/mrx behavior
	offset <<= 1; // use byte offset
	m_state.suppress_advance = offset & 0x4;
	m_state.x_direction = offset & 0x10;
	m_state.y_direction = offset & 0x20;
	m_state.row_fill = offset & 0x80;
	m_state.fb_read = offset & 0x100;
	m_state.fb_write = offset & 0x200;

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
		offset & 400 ? "func=0x" + std::format("{:x}", m_state.rop_function) : "");
}

void nwb_512_device::pixel_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// Set pixel to 1 at X (data), Y (offset[12:1])
	check_mem_mask(mem_mask);
	LOG("pixel_w(0x%x, 0x%x)\n", offset, data);
	set_pixel(x_address(data), y_address(offset), true);
}

uint16_t nwb_512_device::execute_rop(const uint8_t opcode, const uint16_t source, const uint16_t destination)
{
	uint16_t result;
	[[maybe_unused]] std::string op; // Unused when tracing is disabled

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
		result = uint16_t(data << m_state.shift | m_state.carry);
		m_state.carry = uint16_t(uint32_t(data) >> (16 - m_state.shift) & 0xffff);
	}

	return result;
}

void nwb_512_device::rop(uint16_t source)
{
	// Get destination data
	// Graphics ops are always word-aligned. However, mask1 and mask2 make non-aligned access work as expected.
	uint32_t byte_offset = m_state.destination_y * 256 + ((m_state.current_x >> 4) << 1); // Y: 1024 / 8 = 128 bytes per row. Guarantee X is word-aligned first by chopping LSB TODO: make a function, this is done in multiple places
	uint16_t destination = m_vram->read(byte_offset) << 8 | m_vram->read(byte_offset + 1);

	// Execute raster operation with the appropriate source and destination data
	uint16_t result = execute_rop(m_state.rop_function, source, destination);

	// LOG("rop_cycle:\n");
	// LOG("\tx,y(0x%x, 0x%x) => offs 0x%x\n", m_state.current_x, m_state.destination_y, byte_offset);
	// LOG("\tshift = 0x%x, source = 0x%x, carry = 0x%x, destination = 0x%x\n", m_state.shift, source, m_state.carry, destination);
	// LOG("\trop result = 0x%x\n", result);

	if (m_state.dest_words_left == m_state.dest_words_per_row) // first word of row
	{
		result = (result & ~m_state.mask1) | (destination & m_state.mask1);
	}

	if (m_state.dest_words_left == 1) // last word of row
	{
		result = (result & ~m_state.mask2) | (destination & m_state.mask2);
	}

	// Write result back to VRAM
	// LOG("\tdestination (0x%x, 0x%x) => 0x%x = 0x%x 0x%x\n", m_state.current_x, m_state.destination_y, byte_offset, result >> 8 & 0xff, result & 0xff);
	m_vram->write(byte_offset, result >> 8 & 0xff);
	m_vram->write(byte_offset + 1, result & 0xff);

	// Advance ROCC state
	m_state.current_x += m_state.x_direction ? 16 : -16;
	--m_state.dest_words_left;
}

void nwb_512_device::end_row()
{
	// LOG("\tdone with row, advancing y\n");
	m_state.current_x = m_state.destination_x;
	m_state.destination_y += m_state.y_direction ? 1 : -1;
	m_state.carry = 0;

	if (!m_state.limit_y)
	{
		LOG("\tROP done!\n");
		m_state.busy = false;
		m_state.source_x = 0;
		m_state.destination_x = 0;
	}
	else
	{
		m_state.dest_words_left = m_state.dest_words_per_row;
		// temp: m_state.source_words_left = m_state.source_words_per_row;
		m_state.limit_y--;
		// LOG("\tdecremented limit to 0x%x\n", m_state.limit_y);
	}

	// temp
	if (!m_state.source_words_left)
	{
		m_state.source_words_left = m_state.source_words_per_row;
	}
	// end temp
}

void nwb_512_device::rop_cycle(uint16_t data)
{
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

void nwb_512_device::fifo_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("(%s) fifo_w(0x%x, 0x%x, 0x%x), push #%d\n", machine().describe_context(), offset, data, mem_mask, m_debug.fifo_call_count);
	check_mem_mask(mem_mask);
	++m_debug.fifo_call_count;

	if (!m_state.busy)
	{
		// LOG("fb_fifo_w: IOP tried to push data without starting an op!\n");
		// machine().debug_break();
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
	// Wrap to framebuffer size
	x %= 2048;
	y %= 2048;

	// LOG("set_pixel(0x%x, 0x%x) = %d\n", x, y, on ? 1 : 0);
	// Determine byte offset in VRAM
	const uint32_t byte_offset = y * 256 + (x >> 3); // Y: 1024 / 8 = 128 bytes per row
	const uint16_t x_bit = 7 - (x & 7); // pixel 0 = bit 7

	// Set desired pixel
	uint8_t pixel = m_vram->read(byte_offset);
	pixel = (pixel & ~(1u << x_bit)) | ((on ? 1u : 0u) << x_bit);
	m_vram->write(byte_offset, pixel);
}

uint8_t nwb_512_device::status_r()
{
	return 0x20 | (m_state.busy ? 0x3 : 0x0);
}

void nwb_512_device::ctrl_w(uint8_t data) // confirmed 8-bit write
{
	// NWS-1580 uses a newer version of the framebuffer that uses a similar ROP engine, these definitions
	// can be found in the NWS-1580 service manual.
	const bool enabled = data & 0x1;
	LOG("Write FB control = 0x%x (%s)\n", data, machine().describe_context());
	LOG("\t%s framebuffer\n", enabled ? "enable" : "reset");
	LOG("\t%s kanji ROM\n", data & 0x2 ? "enable" : "disable"); // TODO: not for first-gen fb?
	LOG("\t%s v-sync interrupt\n", data & 0x8 ? "enable" : "disable");
	LOG("\t%s command completion interrupt\n", data & 0x10 ? "enable" : "disable");
	LOG("\treserved %s\n", data & 0x20 ? "set" : "cleared");
	LOG("\tdimmer setting = 0x%x\n", (data & 0xC0) >> 6);

	m_state.ctrl = data;
	if (!enabled)
	{
		reset_state();
	}
}

void nwb_512_device::planemask_w(uint8_t data)
{
	LOG("Write plane mask = 0x%x\n", data);
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
