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
 *   - Cycle-accurate timing (especially for fb->fb copies and status bits)
 *   - Interrupt support for CPU-only models (iopboot, mrx, and the monitor don't use them because they have the free cycles to poll)
 *   - Refactoring for multiplanar framebuffer support (NWB-225A color framebuffer, etc.). The implementation uses the same raster operation controller,
 *     so the eventual drivers for those (when ROM dumps and board pictures are available) will be able to share code with the NWB-512.
 *   - Determine dimming amounts from real hardware
 */

#include "emu.h"
#include "nwb_512.h"

#define LOG_CTRL (1U << 1)
#define LOG_ROP (1U << 2)
#define LOG_DATA (1U << 3)

#define VERBOSE 0
#include "logmacro.h"

#include <array>


namespace {

// Total memory region is 2048 x 2048 pixels (512KiB)
constexpr uint16_t X_MAX = 2048;
constexpr uint16_t Y_MAX = 2048;

// Hardware raster operations for logging
constexpr std::array RASTER_OPS = {
		"~S", "~(S | D)", "~S & D", "0", "~(S & D)", "~D", "S ^ D", "S & ~D", "~S | D", "~(S ^ D)", "D", "S & D", "1", "S | ~D", "S | D", "S"};

// Takes in an x,y coordinate and returns the word offset into the framebuffer memory region
constexpr uint32_t coordinate_to_word_offset(uint16_t x, uint16_t y)
{
	return (y % Y_MAX) * (X_MAX / 16) + ((x % X_MAX) >> 4);
}

// Gets the pixel offset of a given x coordinate
constexpr uint16_t x_to_pixel_offset(uint16_t x)
{
	return  ~x & 15; // pixel 0 = bit 15
}

// Extracts the X address from the data provided to an FB write
constexpr uint16_t x_address(uint16_t data)
{
	return data & 0xfff; // When X is supplied, it is in D11-D0
}

// Extracts the Y address from the address provided to an FB write
constexpr uint16_t y_address(uint16_t data) // word offset
{
	return data & 0xfff; // When Y is supplied, it is in A12-A1
}

// True if address written specifies that the write should also load the shift
constexpr bool shift_load_requested(offs_t offset) // word offset
{
	return offset & (0x2000 >> 1);
}

// True if address written specifies operation start
constexpr bool execution_requested(offs_t offset) // word offset
{
	return offset & (0x4000 >> 1);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE(NWB_512, nwb_512_device, "nwb_512", "Sony NWB-512 monochrome framebuffer")


nwb_512_device::nwb_512_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NWB_512, tag, owner, clock),
	m_nwp_512d(*this, "nwp_512d"),
	m_vram(*this, "vram", 0x80000, ENDIANNESS_BIG)
{
}

void nwb_512_device::map(address_map &map)
{
	map(0x00000, 0x0ffff).w(FUNC(nwb_512_device::destination_w));
	map(0x1a000, 0x1bfff).w(FUNC(nwb_512_device::pixel_w));
	map(0x20000, 0x27fff).w(FUNC(nwb_512_device::source_w));
	map(0x40000, 0x55fff).w(FUNC(nwb_512_device::limit_w));
	map(0x60000, 0x60fff).w(FUNC(nwb_512_device::rop_config_w));
	map(0x70000, 0x70fff).rw(FUNC(nwb_512_device::fifo_r), FUNC(nwb_512_device::fifo_w));
	map(0x80000, 0x80001).rw(FUNC(nwb_512_device::status_r), FUNC(nwb_512_device::ctrl_w));
	map(0xa0000, 0xa0003).noprw(); // Unknown, possibly planemask on multiplanar graphics cards with the same ROP engine

	map(0xc0000, 0xfffff).rom().region("krom", 0);
}

void nwb_512_device::device_start()
{
	save_item(NAME(m_state.ctrl));

	save_item(NAME(m_state.destination_x));
	save_item(NAME(m_state.current_destination_x));
	save_item(NAME(m_state.destination_y));

	save_item(NAME(m_state.source_x));
	save_item(NAME(m_state.current_source_x));
	save_item(NAME(m_state.source_y));

	save_item(NAME(m_state.limit_x));
	save_item(NAME(m_state.limit_y));
	save_item(NAME(m_state.rop_function));

	// Status
	save_item(NAME(m_state.req));
	save_item(NAME(m_state.exec));

	// Configuration flags
	save_item(NAME(m_state.x_direction));
	save_item(NAME(m_state.y_direction));
	save_item(NAME(m_state.row_fill));
	save_item(NAME(m_state.fb_read));
	save_item(NAME(m_state.fb_write));
	save_item(NAME(m_state.pixel_mode));

	// Internal state
	save_item(NAME(m_state.shift));
	save_item(NAME(m_state.carry));
	save_item(NAME(m_state.source_words_per_row));
	save_item(NAME(m_state.dest_words_per_row));
	save_item(NAME(m_state.source_words_left));
	save_item(NAME(m_state.dest_words_left));
	save_item(NAME(m_state.mask1));
	save_item(NAME(m_state.mask2));
	save_item(NAME(m_state.prime));
	save_item(NAME(m_state.prime_pending));
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

	// TODO: measure or find documentation for video timing
	// The board has a 70.7 MHz crystal
	// 1066 total lines guessed based on SXGA 1280*1024 mode
	// 1106 total horizontal clocks calculated to give ~59.97 Hz refresh
	SCREEN(config, m_nwp_512d);
	m_nwp_512d->set_raw(70.7_MHz_XTAL,
			1106, 0, HORIZONTAL,
			1066, 0, VERTICAL);
	m_nwp_512d->set_physical_aspect(4, 5); // portrait mode CRT, square-ish pixels/~4:5 ratio assumed
	m_nwp_512d->set_screen_update(FUNC(nwb_512_device::screen_update));
}

u32 nwb_512_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t white = current_dim_color();
	const rgb_t black = rgb_t::black();
	const uint16_t *const vram = m_vram;

	for (int y = cliprect.top(); y <= cliprect.bottom(); ++y)
	{
		const uint16_t *const row = &vram[coordinate_to_word_offset(0, y)]; // Y: 2048 / 8 = 256 bytes per row, not all visible.
		uint32_t *dst_pixel = &bitmap.pix(y, cliprect.min_x);

		for (int x = cliprect.left(); x <= cliprect.right(); ++x)
		{
			// The NWB-512 is a dimmable pure-monochrome display. Based on pictures of the similar NWS-711 monitor
			// and the fact that NEWS Desk looks as it does on the laptop displays when assuming black-on-white,
			// the NWB-512 is most likely also black-on-white (1 = black, 0 = white).
			*dst_pixel++ = BIT(row[x >> 4], x_to_pixel_offset(x)) ? black : white;
		}
	}

	return 0;
}

void nwb_512_device::destination_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	check_mem_mask(mem_mask);
	if (!m_state.enabled())
	{
		logerror("%s destination_w: framebuffer not enabled\n", machine().describe_context());
		return;
	}

	m_state.destination_x = x_address(data);
	m_state.destination_y = y_address(offset);
	LOGMASKED(LOG_ROP, "%s destination_w(0x%x, 0x%x) x word offset = 0x%x\n", machine().describe_context(), m_state.destination_x, m_state.destination_y, m_state.destination_x >> 4);

	if (shift_load_requested(offset))
	{
		m_state.load_shift_count();
	}

	check_operation_start(offset);
}

void nwb_512_device::source_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_ROP, "%s source_w(0x%x, 0x%x, 0x%x)\n", machine().describe_context(), offset, data, mem_mask);
	check_mem_mask(mem_mask);
	if (!m_state.enabled())
	{
		logerror("%s source_w: framebuffer not enabled\n", machine().describe_context());
		return;
	}

	m_state.source_x = x_address(data);
	m_state.source_y = y_address(offset);

	if (shift_load_requested(offset))
	{
		m_state.load_shift_count();
	}

	LOGMASKED(LOG_ROP, "Source set to (0x%x, 0x%x)\n", m_state.source_x, m_state.source_y);
	check_operation_start(offset);
}

void nwb_512_device::limit_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_ROP, "%s limit_w(0x%x, 0x%x, 0x%x)\n", machine().describe_context(), offset, data, mem_mask);
	check_mem_mask(mem_mask);
	if (!m_state.enabled())
	{
		logerror("%s limit_w: framebuffer not enabled\n", machine().describe_context());
		return;
	}

	// The read/write bits seem to be set here in a slightly different way from the DSC-008.
	// This behavior is an educated guess based on what iopboot, mrx, the NWS-800 monitor, and NWS-3800 monitor expect.
	const bool write_op_start = execution_requested(offset);
	m_state.fb_write = write_op_start;                                 // A14 = start write op
	m_state.fb_read = !write_op_start || (offset & (0x10000 >> 1)); // A14 clear = read only, otherwise read source type from A16 if writing

	m_state.limit_x = x_address(data);
	m_state.limit_y = y_address(offset);

	LOGMASKED(LOG_ROP, "Limit set to (0x%x, 0x%x) x word offset = 0x%x, source=%s destination=%s\n", m_state.limit_x, m_state.limit_y, (m_state.limit_x - 1) >> 4, m_state.fb_read ? "fb" : "host", m_state.fb_write ? "fb" : "host");
	check_operation_start(offset);
}

void nwb_512_device::rop_config_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_ROP, "(%s) rop_config_w(0x%x, 0x%x)\n", machine().describe_context(), offset, data);
	check_mem_mask(mem_mask);
	if (!m_state.enabled())
	{
		logerror("%s rop_config_w: framebuffer not enabled\n", machine().describe_context());
		return;
	}

	// These mappings are an educated guess based on NWS-1500 service manual and iopboot/mrx behavior
	// Experimentation on hardware or an NWB-512/DSC-001 service manual would be required to determine the function of the
	// other bits, since the DSC-008 probably differs from this card.
	offset <<= 1; // use byte offset
	m_state.x_direction = offset & 0x10;
	m_state.y_direction = offset & 0x20;
	m_state.row_fill = offset & 0x80;
	m_state.pixel_mode = offset & 0x800;

	const bool load_rop_function = offset & 0x400;
	if (load_rop_function)
	{
		m_state.rop_function = data & 0xf;
	}

	LOGMASKED(LOG_ROP, "\t%s%s%s%s%s\n",
			  m_state.row_fill ? "rowfill " : "",
			  m_state.y_direction ? "yfwd " : "yrev ",
			  m_state.x_direction ? "xfwd " : "xrev ",
			  m_state.pixel_mode ? "pixel " : "word ",
			  load_rop_function ? util::string_format("func=0x%x/%s", m_state.rop_function, RASTER_OPS[m_state.rop_function]) : "");
}

void nwb_512_device::pixel_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_ROP, "(%s) pixel_w(0x%x, 0x%x)\n", machine().describe_context(), offset, data);
	check_mem_mask(mem_mask);
	if (!m_state.enabled())
	{
		logerror("%s pixel_w: framebuffer not enabled\n", machine().describe_context());
		return;
	}

	if (!m_state.pixel_mode)
	{
		logerror("%s pixel_w: Pixel write in word mode\n", machine().describe_context());
	}

	// Set pixel to 1 at X (data), Y (offset[12:1])
	const uint16_t x = x_address(data);
	const uint16_t y = y_address(offset);
	const uint32_t word_offset = coordinate_to_word_offset(x, y);
	const uint16_t x_bit = x_to_pixel_offset(x);
	const uint8_t destination = BIT(m_vram[word_offset], x_bit);

	// Use the result of the ROP engine before setting the pixel
	const bool new_state = execute_rop(m_state.rop_function, 1, destination) & 1;
	LOGMASKED(LOG_DATA, "pixel_w(0x%x, 0x%x) => Change (0x%x, 0x%x) from %u to %u\n", offset, data, x, y, destination, new_state ? 1u : 0u);

	// Set desired pixel
	uint16_t pixel = m_vram[word_offset];
	pixel = (pixel & ~(1U << x_bit)) | ((new_state ? 1U : 0U) << x_bit);
	m_vram[word_offset] = pixel;
}

uint16_t nwb_512_device::fifo_r(offs_t offset, uint16_t mem_mask)
{
	if (machine().side_effects_disabled())
	{
		// Reads only happen in the forward direction, so the shift logic to peek at the word is simple.
		uint16_t peek = coordinate_to_data(m_state.current_source_x, m_state.source_y);
		return uint16_t(peek >> m_state.shift | m_state.carry);
	}

	LOGMASKED(LOG_DATA, "(%s) fifo_r(0x%x, 0x%x)\n", machine().describe_context(), offset, mem_mask);
	check_mem_mask(mem_mask);
	if (!m_state.enabled())
	{
		logerror("%s fifo_r: framebuffer not enabled\n", machine().describe_context());
		return 0;
	}
	if (!m_state.is_fb_to_host() || !m_state.exec)
	{
		logerror("%s fifo_r: when read op is not executing\n", machine().describe_context());
		return 0;
	}

	uint16_t word = 0;

	m_state.req = true; // FB executing

	if (m_state.prime_pending)
	{
		// If the source row's coordinate is greater (or less when reverse), prime the shifter,
		// because two initial reads are needed. After this, the source and destination counts will be aligned.
		word = coordinate_to_data(m_state.current_source_x, m_state.source_y);
		shift(word);
		m_state.prime_pending = false;
		m_state.advance_sxa();
	}

	word = coordinate_to_data(m_state.current_source_x, m_state.source_y);
	word = shift(word);

	// Advance ROCC state
	m_state.advance_sxa();
	m_state.advance_dxa();

	if (m_state.dest_words_left == 0)
	{
		end_row();
	}

	m_state.req = false; // Waiting for data from host (or done with op)

	return word;
}

void nwb_512_device::fifo_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_DATA, "(%s) fifo_w(0x%x, 0x%x, 0x%x)\n", machine().describe_context(), offset, data, mem_mask);
	check_mem_mask(mem_mask);
	if (!m_state.enabled())
	{
		logerror("%s fifo_w: framebuffer not enabled\n", machine().describe_context());
		return;
	}

	// One write kickstarts the reading process when triggered op is a read
	if (m_state.is_fb_to_host())
	{
		LOGMASKED(LOG_DATA, "%s fifo_w: start read operation\n", machine().describe_context());
		start_operation();
		return;
	}

	if (m_state.pixel_mode || !m_state.exec)
	{
		logerror("%s fifo_w: word write in pixel mode or not active, ignoring\n", machine().describe_context());
		return;
	}

	m_state.req = true; // FB executing
	if (!m_state.row_fill)
	{
		rop_cycle(data);
	}
	else
	{
		// Assume row fill always populates the rest of the destination after running input data through the shifter
		// I haven't found any evidence to the contrary.
		const uint16_t shifted_data = shift(data);
		while (m_state.dest_words_left > 0)
		{
			rop(shifted_data);
		}
		end_row();
	}
	m_state.req = false; // Waiting for next host read (or op complete)
}

uint16_t nwb_512_device::status_r()
{
	// The framebuffer font is only rendered properly if bit 5 is set.
	// This might be some kind of ID or revision flag for the Kanji ROM (or the framebuffer as a whole).
	return 0x20 | (m_state.exec ? 0x2 : 0x0) | (m_state.req ? 0x1 : 0x0);
}

void nwb_512_device::ctrl_w(uint16_t data)
{
	LOGMASKED(LOG_CTRL, "%s ctrl_w(0x%x)\n", machine().describe_context(), data);

	// NWS-1580 uses a newer version of the framebuffer that uses a similar ROP engine, these definitions
	// can be found in the NWS-1580 service manual.
	const bool enabled = data & 0x1;
	if (!enabled)
	{
		reset_state();
	}

	m_state.ctrl = data & 0xff;
}

void nwb_512_device::reset_state()
{
	// The DSC-008 service manual does not elaborate on what is or isn't cleared on disable
	// (and system behavior doesn't seem to care).
	m_state = {};
}

void nwb_512_device::check_mem_mask(uint16_t mem_mask)
{
	if (mem_mask != 0xffff)
	{
		// Registers are 16-bit. Log this just in case to chase down issues later.
		logerror("%s unexpected non-word access with mem mask 0x%x!\n", machine().describe_context(), mem_mask);
	}
}

void nwb_512_device::check_operation_start(offs_t offset)
{
	if (!execution_requested(offset))
	{
		return;
	}

	// Adjust source and destination coordinates for the reverse direction if necessary,
	// because the left edge is always inclusive (right edge exclusive).
	if (!m_state.x_direction)
	{
		--m_state.destination_x;
		--m_state.source_x;
		--m_state.limit_x;
	}

	start_operation();

	// TODO: This executes copies immediately, faster than real hardware
	if (m_state.is_fb_to_fb() && m_state.exec)
	{
		m_state.req = true; // FB executing, no input from host needed, so req will stay high throughout
		while (m_state.busy())
		{
			const uint16_t source_data = coordinate_to_data(m_state.current_source_x, m_state.source_y);
			LOGMASKED(LOG_DATA, "fb->fb: sx = 0x%x sy = 0x%x data = 0x%x\n", m_state.current_source_x, m_state.source_y, source_data);
			rop_cycle(source_data); // When the ROP is done, this will clear the status flags
		}
	}
}

uint16_t nwb_512_device::coordinate_to_data(uint16_t x, uint16_t y)
{
	const uint32_t word_offset = coordinate_to_word_offset(x, y);
	return m_vram[word_offset];
}

uint16_t nwb_512_device::execute_rop(uint8_t opcode, uint16_t source, uint16_t destination)
{
	uint16_t result;

	switch (opcode)
	{
	case 0x0:
		result = ~source;
		break;
	case 0x1:
		result = ~(source | destination);
		break;
	case 0x2:
		result = ~source & destination;
		break;
	case 0x3:
		result = 0;
		break;
	case 0x4:
		result = ~(source & destination);
		break;
	case 0x5:
		result = ~destination;
		break;
	case 0x6:
		result = source ^ destination;
		break;
	case 0x7:
		result = source & ~destination;
		break;
	case 0x8:
		result = ~source | destination;
		break;
	case 0x9:
		result = ~(source ^ destination);
		break;
	case 0xa:
		result = destination;
		break;
	case 0xb:
		result = source & destination;
		break;
	case 0xc:
		result = 0xffff;
		break;
	case 0xd:
		result = source | ~destination;
		break;
	case 0xe:
		result = source | destination;
		break;
	case 0xf:
		result = source;
		break;
	default:
		fatalerror("nwb_512: invalid ROP function code 0x%x", opcode);
	}

	LOGMASKED(LOG_DATA, "execute_rop(0x%x/%s, 0x%x, 0x%x) = 0x%x\n", opcode, RASTER_OPS[opcode], source, destination, result);
	return result;
}

uint16_t nwb_512_device::shift(uint16_t data)
{
	// Calculate source by shifting bits and update carryover
	uint16_t result;
	if (m_state.x_direction) // L -> R
	{
		result = uint16_t(data >> m_state.shift | m_state.carry);
		m_state.carry = uint16_t(uint32_t(data) << (16 - m_state.shift) & 0xffff);
	}
	else // R -> L
	{
		const uint16_t shift_complement = (16 - m_state.shift) & 0xf;
		result = uint16_t(data << shift_complement | m_state.carry);
		m_state.carry = uint16_t(uint32_t(data) >> (16 - shift_complement) & 0xffff);
	}

	return result;
}

void nwb_512_device::rop(uint16_t source)
{
	const offs_t word_offset = coordinate_to_word_offset(m_state.current_destination_x, m_state.destination_y);
	const uint16_t destination = coordinate_to_data(m_state.current_destination_x, m_state.destination_y);

	// Execute raster operation with the appropriate source and destination data
	uint16_t result = execute_rop(m_state.rop_function, source, destination);

	LOGMASKED(LOG_DATA, "rop_cycle:\n");
	LOGMASKED(LOG_DATA, "\tx,y(0x%x, 0x%x) => offs 0x%x\n", m_state.current_destination_x, m_state.destination_y, word_offset);
	LOGMASKED(LOG_DATA, "\tshift = 0x%x, source = 0x%x, carry = 0x%x, destination = 0x%x\n", m_state.shift, source, m_state.carry, destination);
	LOGMASKED(LOG_DATA, "\trop result = 0x%x\n", result);
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
	m_vram[word_offset] = result;

	// Advance ROCC state
	m_state.advance_dxa();
	m_state.advance_sxa();
}

void nwb_512_device::rop_cycle(uint16_t data)
{
	if (m_state.prime_pending)
	{
		// If the source is ahead of the destination (in the scan direction), we need to prime the shifter so carry is correct.
		shift(data);
		m_state.prime_pending = false;
		m_state.advance_sxa();

		if (m_state.source_words_left == 0 && m_state.dest_words_left > 0)
		{
			// In this case, the prime has to be written directly, because the row consisted of one source word and one destination word
			// which still triggers the priming mechanism (/PRED, set when), as the applied shift is still needed to align the source data properly.
			rop(m_state.carry);
		}

		// Because we are writing a word, we might have to end the row
		if (m_state.dest_words_left == 0)
		{
			end_row();
		}

		return;
	}

	data = shift(data);
	rop(data);

	if (m_state.source_words_left == 0 && m_state.dest_words_left > 0)
	{
		rop(m_state.carry); // shifter remainder as the transaction is complete, nothing further from the source
	}

	if (m_state.dest_words_left == 0)
	{
		end_row(); // reset X, step Y, rearm mask1, clear carry, maybe finish op
	}
}

void nwb_512_device::end_row()
{
	m_state.current_destination_x = m_state.destination_x;
	m_state.current_source_x = m_state.source_x;
	m_state.advance_dya();
	m_state.advance_sya();
	m_state.carry = 0;

	if (!m_state.limit_y)
	{
		end_operation();
	}
	else
	{
		m_state.dest_words_left = m_state.dest_words_per_row;
		m_state.source_words_left = m_state.source_words_per_row;
		m_state.prime_pending = m_state.prime;
		--m_state.limit_y;
	}
}

void nwb_512_device::start_operation()
{
	LOGMASKED(LOG_ROP, "start_operation()\n");

	// The DSC-008 section of the NWS-15xx service manual implies that the direction bit is not
	// supported/used for reads. For now, assume the same for the NWB-512.
	if (m_state.is_fb_to_host() && !m_state.x_direction)
	{
		logerror("%s start_operation: Reverse reads are not supported, skipping\n", machine().describe_context());
		return;
	}

	// Set exec flag as operation is starting.
	// req will be set only while the fb is actively doing something
	m_state.exec = true;

	// Latch SRXA and DRXA into SXA and DXA
	m_state.current_destination_x = m_state.destination_x;
	m_state.current_source_x = m_state.source_x;

	// Compute the phase of the source and destination (used to determine how the shifter is loaded)
	const uint16_t src_phase = (m_state.x_direction ? m_state.source_x : ~m_state.source_x) & 15;
	const uint16_t dest_phase = (m_state.x_direction ? m_state.destination_x : ~m_state.destination_x) & 15;
	const bool read = !m_state.fb_write;

	// fwd: [dest, limit), rev: [limit, dest), but read is inclusive rather than exclusive and only works in the forward direction.
	const uint16_t width = (m_state.x_direction ? m_state.limit_x - m_state.destination_x : m_state.destination_x - m_state.limit_x) + (read ? 1 : 0);

	m_state.prime = src_phase > dest_phase; // In this case, we will need to fetch an extra word from the source
	m_state.prime_pending = m_state.prime;

	m_state.dest_words_per_row = ((dest_phase + width + 15) >> 4)                                     // Round up by adding 15 to ensure all words are captured
								 - (read && m_state.prime ? 1 : 0);                                   // Will read one fewer words per row if we need to prime the shifter to align
	m_state.source_words_per_row = read ? m_state.dest_words_per_row : (src_phase + width + 15) >> 4; // Not used for reads

	m_state.source_words_left = m_state.source_words_per_row;
	m_state.dest_words_left = m_state.dest_words_per_row;
	LOGMASKED(LOG_DATA, "start_operation: Source words per row: %d, Destination words per row: %d, priming required = %s\n", m_state.source_words_per_row, m_state.dest_words_per_row, m_state.prime ? "yes" : "no");

	// If writing to the framebuffer, update masks to preserve bits that shouldn't be modified
	if (m_state.fb_write)
	{
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
	}

	// Sanity-check to avoid causing unexpected member variable wrapping issues in the driver.
	// I never saw these cases while testing, and I am not sure yet what real hardware would do here.
	if (m_state.dest_words_per_row == 0)
	{
		logerror("%s start_operation: unexpected empty operation detected, skipping\n", machine().describe_context());
		end_operation();
	}
	else if (m_state.x_direction ? (m_state.destination_x > m_state.limit_x) : (m_state.limit_x > m_state.destination_x))
	{
		logerror("%s start_operation: invalid destination/limit configuration, skipping\n", machine().describe_context());
		end_operation();
	}
}

void nwb_512_device::end_operation()
{
	LOGMASKED(LOG_ROP, "ROP completed!\n");
	m_state.req = false;
	m_state.exec = false;

	// The DSC-008 instruction manual mentions that SRXA/DRXA[3:0] are cleared independently (see: /CLRDEST and /SFH).
	// This may not be accurate. Hardware confirmation is needed.
	m_state.source_x &= ~0xf;
	m_state.destination_x &= ~0xf;
}

rgb_t nwb_512_device::current_dim_color()
{
	// Assume darkest dim is 25% brightness (0b11) with 50% (0b10) and 75% (0b01) as the other options.
	// The DSC-001 dim functions in iopboot/mrx write 0b11 to bits 7:6 of the status register when the
	// sxsession screen saver is active. The NWS-15xx service manual backs this up for the DSC-007 from 0b00 (brightest) -> 0b11 (darkest),
	// but does not elaborate on the exact degree of dimming.
	const uint8_t dim = BIT(m_state.ctrl, 6, 2);
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
	ROM_LOAD16_BYTE("hn62301ap-aa1.ic128", 0x0000, 0x20000, CRC(3eaa0dbc) SHA1(9c4e66cc122fd60963f51649c945cdfc39f230eb))
	ROM_LOAD16_BYTE("hn62301ap-aa2.ic127", 0x0001, 0x20000, CRC(b8705af7) SHA1(915333f082ecbfee512f1166fc38d9dd29dcaf26))
ROM_END

const tiny_rom_entry *nwb_512_device::device_rom_region() const
{
	return ROM_NAME(nwb_512);
}
