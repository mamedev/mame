// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    Axell AG-2 / AX51201 graphics controller.

    ROM-derived implementation. Register names are descriptive, not official.
    CPU programs: cscaoysd 2041e/20594 (transfer), 20630 (draw), 20700-20d00
    (sheet/cell construction), and the Hide & Seek SH-2 graphics library.

    +02 command: 42 transfer, 02 display list, 80/82 RM1 initialize/advance.
    +0e interrupt mask, +0f pending/W1C; causes 01/04/10/40.
    +16/18 draw destination, +1a/1c/1e/20 inclusive drawing window.
    +22 sheets, +24 cells, +26 order list, +28 list mode.
    +34 source low, +36 source high, +38/3a destination, +3c/3e size-1,
    +40 transfer mode: 01 raw ROM, 09 Huffman/RLE, 13 host RAM.
    +42 display control; +49 bit1 selects framebuffer +4a/4c or +4e/50.
    +56 visible width-1, +5c visible lines.

    The Huffman/RLE decompressor is shared with the AG-1 (axell_decomp.cpp);
    on the AG-2 its source is linear. The CG ROM is read through the device
    ROM interface, in little-endian byte order after the board ROM load.
    AG-2 cells have 16-, 18- and 28-byte forms, whose flags and layout differ
    from AX51102 records.

    Remaining hardware uncertainties:
    - Colour-effect rounding, additive/subtractive strength and unobserved
      modulation modes 3, 4 and 7 need controlled hardware comparisons.
    - Format-1 upper storage bits and RGB565 palette-index-zero transparency
      are inferred from observed assets, not verified hardware rules. Pixel
      formats 5, 6 and 7 have not been identified and are not drawn.
    - Rotation step precision and subpixel coverage are approximations. No
      non-affine perspective descriptor has been identified in captured lists.
    - List ordering (+26/+28), framebuffer preservation and the clear trigger
      are inferred from the games; clearing on a display-buffer change is a
      provisional implementation, not an identified hardware control.
    - Register masks/reset values, SDRAM mapping and complete CRTC/blanking
      behaviour remain unverified. Engine costs and completion timing are
      estimates; drawing is atomic and there is no FIFO/contention model.
    - RM1 supports the observed 640x480 4:2:0 streams. Physical YUV packing,
      chroma filtering, RGB conversion/rounding and reference-buffer placement
      need hardware captures, as do reset, error and presentation timing.
      See axell_ag2_rm1.cpp for reconstruction-specific limitations.
*/
#include "emu.h"

#include "axell_ag2.h"
#include "axell_decomp.h"

#include "screen.h"

#include <algorithm>
#include <cstdlib>

#define LOG_REGISTERS (1U << 1)
#define LOG_HOST      (1U << 2)
#define LOG_REPEATS   (1U << 3)
#define LOG_STATS     (1U << 4)
#define VERBOSE       (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(AXELL_AG2, axell_ag2_device, "axell_ag2", "Axell AG-2 AX51201 Graphics Display Controller")

// The compressed stream is linear on the AG-2. Its first byte, which gives
// the width of the block on the AG-1, is skipped.
class axell_ag2_device::linear_decompressor : public axell_decompressor
{
public:
	linear_decompressor(axell_ag2_device &host, u32 source, u8 *dest, u32 x_bits, u32 x_mask, u32 y_mask) :
		axell_decompressor(dest, x_bits, x_mask, y_mask, true),
		m_host(host),
		m_source(source + 1),
		m_consumed(0)
	{
	}

protected:
	virtual u8 fetch_byte() override
	{
		if (m_source >= CGROM_SIZE || m_consumed >= CGROM_SIZE)
			m_error = true;
		++m_consumed;
		return m_host.rom8(m_source++);
	}

private:
	axell_ag2_device &m_host;
	u32 m_source;
	u32 m_consumed;
};


axell_ag2_device::axell_ag2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	axell_ag2_device(mconfig, tag, owner, clock, 0x1000000)
{
}

axell_ag2_device::axell_ag2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 vram_size) :
	device_t(mconfig, AXELL_AG2, tag, owner, clock),
	device_rom_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_host_config("host", ENDIANNESS_BIG, 8, 17, 0, address_map_constructor(FUNC(axell_ag2_device::host_map), this)),
	m_vram_config("vram", ENDIANNESS_LITTLE, 8, 25, 0),
	m_event_cb(*this),
	m_host(*this, "host", HOST_SIZE, ENDIANNESS_BIG),
	m_vram(*this, "vram", vram_size, ENDIANNESS_LITTLE),
	m_vram_size(vram_size),
	m_vram_mask(vram_size - 1),
	m_completion_timer{},
	m_pending(0),
	m_active(0),
	m_vblank(false),
	m_rm1_active(false),
	m_rm1_have_anchor(false),
	m_rm1_base(0),
	m_rm1_first_packet(0),
	m_rm1_next_packet(0),
	m_rm1_packet_count(0),
	m_rm1_max_payload(0),
	m_rm1_decoded(0),
	m_rm1_next_display(0),
	m_rm1_anchor_display(0),
	m_rm1_anchor_advance(0),
	m_rm1_ready_slot(0),
	m_reads{},
	m_writes{},
	m_last_offset(HOST_SIZE),
	m_last_data(0),
	m_last_write(false),
	m_repeats(0),
	m_transfers(0),
	m_draws(0),
	m_decode_errors(0),
	m_rm1_commands(0)
{
}

device_memory_interface::space_config_vector axell_ag2_device::memory_space_config() const
{
	auto spaces = device_rom_interface::memory_space_config();
	spaces.emplace_back(AS_HOST, &m_host_config);
	spaces.emplace_back(AS_VRAM, &m_vram_config);
	return spaces;
}

void axell_ag2_device::host_map(address_map &map)
{
	map(0, HOST_SIZE - 1).rw(FUNC(axell_ag2_device::host_r), FUNC(axell_ag2_device::host_w));
}

u16 axell_ag2_device::vram16(u32 x, u32 y) const
{
	const u32 address = vram_addr(x * 2, y);
	return m_vram[address] | (m_vram[(address + 1) & m_vram_mask] << 8);
}

void axell_ag2_device::vram_w16(u32 x, u32 y, u16 data)
{
	const u32 address = vram_addr(x * 2, y);
	m_vram[address] = data;
	m_vram[(address + 1) & m_vram_mask] = data >> 8;
}

void axell_ag2_device::device_validity_check(validity_checker &valid) const
{
	if (m_vram_size < 0x800000 || m_vram_size > 0x2000000 || (m_vram_size & (m_vram_size - 1)))
		osd_printf_error("VRAM must be 8, 16 or 32 MiB\n");
}

void axell_ag2_device::device_start()
{
	space(AS_VRAM).install_ram(0, m_vram_size - 1, m_vram.target());
	for (auto &timer : m_completion_timer)
		timer = timer_alloc(FUNC(axell_ag2_device::complete), this);
	screen().register_vblank_callback(vblank_state_delegate(&axell_ag2_device::vblank, this));
	save_item(NAME(m_pending));
	save_item(NAME(m_vblank));
	save_item(NAME(m_rm1_active));
	save_item(NAME(m_rm1_have_anchor));
	save_item(NAME(m_rm1_base));
	save_item(NAME(m_rm1_first_packet));
	save_item(NAME(m_rm1_next_packet));
	save_item(NAME(m_rm1_packet_count));
	save_item(NAME(m_rm1_max_payload));
	save_item(NAME(m_rm1_decoded));
	save_item(NAME(m_rm1_next_display));
	save_item(NAME(m_rm1_anchor_display));
	save_item(NAME(m_rm1_anchor_advance));
	save_item(NAME(m_rm1_ready_slot));
}

void axell_ag2_device::device_reset()
{
	flush_repeats();
	m_last_offset = HOST_SIZE;
	std::fill_n(&m_host[REG_BASE], HOST_SIZE - REG_BASE, 0); // unverified reset register values
	m_pending = 0;
	m_vblank = false;
	rm1_reset();
	for (auto timer : m_completion_timer)
		timer->adjust(attotime::never);
	update_events();
}

void axell_ag2_device::device_post_load()
{
	rm1_restore();
	update_screen_size();
	m_active = m_pending & reg8(0x0e) & 0x55;
}

u8 axell_ag2_device::host_r(offs_t offset)
{
	assert(offset < HOST_SIZE);
	const u8 data = (offset == REG_BASE + 0x0f) ? m_pending : m_host[offset];
	trace(false, offset, data);
	return data;
}

void axell_ag2_device::host_w(offs_t offset, u8 data)
{
	assert(offset < HOST_SIZE);
	trace(true, offset, data);
	if (offset == REG_BASE + 0x0f)
	{
		m_pending &= ~data;
		update_events();
		return;
	}
	const u8 previous = m_host[offset];
	m_host[offset] = data;
	if (offset == REG_BASE + 0x0e)
		update_events();
	else if (offset == REG_BASE + 2)
		command_w(data);
	else if (offset == REG_BASE + 0x49 && BIT(previous ^ data, 1))
		clear_frame_buffer(!BIT(data, 1)); // TODO: identify the actual clear control
	else if (offset == REG_BASE + 0x57 || offset == REG_BASE + 0x5d)
		update_screen_size();
}

void axell_ag2_device::update_events()
{
	const u8 active = m_pending & reg8(0x0e) & 0x55;
	const u8 changed = active ^ m_active;
	m_active = active;
	for (u32 n = 0; n < 4; n++)
		if (BIT(changed, n * 2))
			m_event_cb[n](BIT(active, n * 2));
}

void axell_ag2_device::schedule_event(u8 event, u64 clocks)
{
	// one timer per engine; clock and execution cost are provisional
	const u32 index = (event == 4) ? 0 : (event == 0x10) ? 1 : 2;
	m_completion_timer[index]->adjust(attotime::from_ticks(std::max<u64>(1, clocks), clock()), event);
}

TIMER_CALLBACK_MEMBER(axell_ag2_device::complete)
{
	if (param == 1)
	{
		m_host[REG_BASE + 0x98] = m_rm1_ready_slot;
		// Clearing bit3 at EOF also permits the firmware's command80 helper
		// to initialize another movie. The last valid output slot stays visible.
		m_host[REG_BASE + 0x9f] = m_rm1_next_display == m_rm1_packet_count ? 0x02 : 0x09;
	}
	m_pending |= u8(param);
	update_events();
}

void axell_ag2_device::command_w(u8 data)
{
	if (BIT(data, 7))
	{
		if (data == 0x80)
			rm1_reset();
		else if (data == 0x82)
			rm1_advance();
		return;
	}
	if (!BIT(data, 1))
		return;
	if (BIT(data, 6))
		transfer();
	else
	{
		const u32 pixels = draw_list();
		++m_draws;
		schedule_event(0x10, std::max<u32>(64, pixels));
	}
}

void axell_ag2_device::rm1_reset()
{
	// Hide & Seek issues 80 before programming the movie registers. Its exact
	// abort/reset semantics are unverified; it must leave the first 82 usable.
	rm1_clear();
	update_events();
}

void axell_ag2_device::rm1_clear()
{
	m_completion_timer[2]->adjust(attotime::never);
	m_pending &= ~u8(1);
	m_rm1_decoder.reset();
	m_rm1_active = m_rm1_have_anchor = false;
	m_rm1_base = m_rm1_first_packet = m_rm1_next_packet = 0;
	m_rm1_packet_count = m_rm1_max_payload = m_rm1_decoded = 0;
	m_rm1_next_display = m_rm1_anchor_display = 0;
	m_rm1_anchor_advance = m_rm1_ready_slot = 0;
	m_host[REG_BASE + 0x98] = m_host[REG_BASE + 0x9f] = 0;
}

u32 axell_ag2_device::rm1_read(u32 address, u32 bytes)
{
	if (bytes > 4 || u64(address) + bytes > CGROM_SIZE)
		throw axell_rm1::decode_error("RM1 field exceeds CG ROM");
	u32 result = 0;
	for (u32 i = 0; i < bytes; ++i)
		result = (result << 8) | rom8(address + i);
	return result;
}

const u8 *axell_ag2_device::rm1_payload(u32 address, u32 size)
{
	if (u64(address) + size > CGROM_SIZE)
		throw axell_rm1::decode_error("RM1 payload exceeds CG ROM");
	m_rm1_buffer.resize(size);
	for (u32 i = 0; i < size; ++i)
		m_rm1_buffer[i] = rom8(address + i);
	return m_rm1_buffer.data();
}

void axell_ag2_device::rm1_initialize()
{
	m_rm1_base = (u32(reg16(0x84)) << 16) | reg16(0x86);
	if (u64(m_rm1_base) + 0x24 > CGROM_SIZE
		|| rm1_read(m_rm1_base, 4) != 0x432d5241 || rm1_read(m_rm1_base + 4, 4) != 0x50494301)
		throw axell_rm1::decode_error("missing C-RAPIC v1 container");
	const u32 first = rm1_read(m_rm1_base + 0x10, 4);
	m_rm1_packet_count = rm1_read(m_rm1_base + 0x14, 4);
	m_rm1_max_payload = rm1_read(m_rm1_base + 0x18, 4);
	if (first < 0x24 || u64(m_rm1_base) + first > CGROM_SIZE
		|| !m_rm1_packet_count || m_rm1_packet_count > (CGROM_SIZE - m_rm1_base - first) / 8
		|| rm1_read(m_rm1_base + 0x1c, 2) != 639 || rm1_read(m_rm1_base + 0x1e, 2) != 479)
		throw axell_rm1::decode_error("unsupported RM1 container extent or dimensions");
	m_rm1_first_packet = m_rm1_next_packet = m_rm1_base + first;
	m_rm1_active = true;
}

void axell_ag2_device::rm1_write_frame(u32 slot, const axell_rm1::frame &picture)
{
	// The three programmed X coordinates are in two-pixel units: Hide & Seek
	// uses 0/320/640, with format-4 sprite pixel origins 0/640/1280.
	// Provisional packing: Y0,Cb,Y1,Cr at the normal 4096-byte VRAM pitch.
	// Physical byte order and the 4:2:0->4:2:2 chroma filter need hardware data.
	const u32 dx = u32(reg16(0x70 + slot * 4)) * 4;
	const u32 dy = reg16(0x72 + slot * 4);
	for (u32 y = 0; y < axell_rm1::frame::HEIGHT; ++y)
		for (u32 x = 0; x < axell_rm1::frame::WIDTH; x += 2)
		{
			const u32 yi = y * axell_rm1::frame::WIDTH + x;
			const u32 ci = (y / 2) * (axell_rm1::frame::WIDTH / 2) + x / 2;
			m_vram[vram_addr(dx + x * 2, dy + y)] = picture.y[yi];
			m_vram[vram_addr(dx + x * 2 + 1, dy + y)] = picture.cb[ci];
			m_vram[vram_addr(dx + x * 2 + 2, dy + y)] = picture.y[yi + 1];
			m_vram[vram_addr(dx + x * 2 + 3, dy + y)] = picture.cr[ci];
		}
}

u32 axell_ag2_device::rm1_decode_next()
{
	if (m_rm1_decoded >= m_rm1_packet_count)
		throw axell_rm1::decode_error("RM1 presentation exceeds packet count");
	const u32 position = m_rm1_next_packet;
	const u32 flags = rm1_read(position, 1);
	const u32 kind = flags & 3;
	const u32 order = rm1_read(position + 4, 1);
	const u32 size = rm1_read(position + 5, 3);
	if (kind == 3 || rm1_read(position + 1, 3) || size > m_rm1_max_payload
		|| u64(position) + 8 + size > CGROM_SIZE)
		throw axell_rm1::decode_error("invalid RM1 packet extent/type");
	// Look-ahead flags describe packet kinds, not output buffer indices.
	u64 next = u64(position) + 8 + size;
	for (u32 distance = 1; distance <= 2; ++distance)
	{
		u32 expected = 3;
		if (m_rm1_decoded + distance < m_rm1_packet_count)
		{
			if (next + 8 > CGROM_SIZE)
				throw axell_rm1::decode_error("truncated RM1 look-ahead packet");
			expected = rm1_read(u32(next), 1) & 3;
			next += 8 + rm1_read(u32(next) + 5, 3);
		}
		if (((flags >> (distance * 2)) & 3) != expected)
			throw axell_rm1::decode_error("RM1 packet kind look-ahead mismatch");
	}
	u32 presentation;
	if (kind < 2)
	{
		presentation = m_rm1_have_anchor ? m_rm1_anchor_display + m_rm1_anchor_advance : 0;
		if (m_rm1_have_anchor && !m_rm1_anchor_advance)
			throw axell_rm1::decode_error("nonadvancing RM1 anchor");
	}
	else
	{
		if (!m_rm1_have_anchor || order > m_rm1_anchor_display)
			throw axell_rm1::decode_error("RM1 B picture has no valid display anchor");
		presentation = m_rm1_anchor_display - order;
	}
	if (presentation >= m_rm1_packet_count || presentation < m_rm1_next_display)
		throw axell_rm1::decode_error("invalid RM1 presentation order");
	const axell_rm1::frame &picture = m_rm1_decoder.decode_packet(kind, rm1_payload(position + 8, size), size);
	if (kind < 2)
	{
		m_rm1_anchor_picture = picture;
		m_rm1_anchor_display = presentation;
		m_rm1_anchor_advance = order;
		m_rm1_have_anchor = true;
	}
	if (presentation == m_rm1_next_display)
		rm1_write_frame(m_rm1_ready_slot, picture);
	m_rm1_next_packet += 8 + size;
	++m_rm1_decoded;
	return presentation;
}

void axell_ag2_device::rm1_advance()
{
	// These states satisfy the firmware's exact 9f predicates. Names and cycle
	// timing are inferred: 08 blocks another 82, 09 publishes a frame, bit1 EOS.
	if (((reg8(0x9f) & 9) == 8) || BIT(reg8(0x9f), 1))
		return;
	m_host[REG_BASE + 0x9f] = 8;
	++m_rm1_commands;
	const u32 decoded_before = m_rm1_decoded;
	try
	{
		if (!m_rm1_active)
			rm1_initialize();
		if (m_rm1_next_display >= m_rm1_packet_count)
			throw axell_rm1::decode_error("RM1 advance past end of movie");
		// The host can draw the preceding slot while requesting an advance.
		// Rotate presentation surfaces, keeping decoder anchors separately.
		// Physical workspace/reference allocation and slot order are unverified.
		m_rm1_ready_slot = m_rm1_next_display % 3;
		if (m_rm1_have_anchor && m_rm1_next_display == m_rm1_anchor_display)
			rm1_write_frame(m_rm1_ready_slot, m_rm1_anchor_picture);
		else
		{
			// A future anchor may precede the requested B picture in the stream.
			// Decode it without overwriting any surface being drawn by the host.
			for (;;)
			{
				const u32 presentation = rm1_decode_next();
				if (presentation == m_rm1_next_display)
					break;
			}
		}
		++m_rm1_next_display;
		schedule_event(1, std::max<u64>(64, u64(m_rm1_decoded - decoded_before) * 640 * 480));
	}
	catch (const axell_rm1::decode_error &error)
	{
		++m_decode_errors;
		logerror("RM1 failed container=%08x packet=%u: %s\n", m_rm1_base, m_rm1_decoded, error.what());
		// error status bits are unknown: no completion is signalled, and
		// command 80 reinitializes the engine
	}
}

void axell_ag2_device::rm1_restore()
{
	m_rm1_decoder.reset();
	if (!m_rm1_active)
		return;
	try
	{
		u32 position = m_rm1_first_packet;
		for (u32 index = 0; index < m_rm1_decoded; ++index)
		{
			const u32 kind = rm1_read(position, 1) & 3;
			const u32 size = rm1_read(position + 5, 3);
			if (size > m_rm1_max_payload || u64(position) + 8 + size > CGROM_SIZE)
				throw axell_rm1::decode_error("invalid saved RM1 packet extent");
			const axell_rm1::frame &picture = m_rm1_decoder.decode_packet(kind, rm1_payload(position + 8, size), size);
			if (kind < 2)
				m_rm1_anchor_picture = picture;
			position += 8 + size;
		}
		if (position != m_rm1_next_packet)
			throw axell_rm1::decode_error("saved RM1 packet position mismatch");
	}
	catch (const axell_rm1::decode_error &error)
	{
		logerror("RM1 state reconstruction failed: %s\n", error.what());
		rm1_clear();
	}
}

void axell_ag2_device::transfer()
{
	const u32 source = reg16(0x34) | (u32(reg16(0x36)) << 16);
	const u32 dx = reg16(0x38) * 2;
	const u32 dy = reg16(0x3a);
	const u32 width = ((reg16(0x3c) & 0x3ff) + 1) * 2;
	const u32 height = (reg16(0x3e) & 0x3ff) + 1;
	const u8 mode = reg8(0x40) & 0x1f;
	++m_transfers;
	bool error = false;
	if (mode == 9)
		error = decompress(source, width, height, dx, dy);
	else if (mode == 1 || mode == 0x13)
	{
		for (u32 y = 0; y < height; y++)
			for (u32 x = 0; x < width; x++)
			{
				const u32 a = source + y * width + x;
				m_vram[vram_addr(dx + x, dy + y)] = mode == 1 ? rom8(a) : list8(a);
			}
	}
	else
	{
		error = true;
		logerror("unimplemented transfer mode %02x\n", mode);
	}
	if (error)
	{
		++m_decode_errors;
		logerror("transfer failed mode=%02x source=%08x size=%ux%u\n", mode, source, width, height);
	}
	schedule_event(4, std::max<u64>(64, u64(width) * height));
}

void axell_ag2_device::update_screen_size()
{
	const u32 width = reg16(0x56) + 1;
	const u32 height = reg16(0x5c);
	if (width < 120 || width > 1024 || height < 100 || height > 768)
		return;
	if (screen().visible_area().width() != width || screen().visible_area().height() != height)
		screen().configure(width, height, rectangle(0, width - 1, 0, height - 1), screen().frame_period());
}

void axell_ag2_device::clear_frame_buffer(bool second)
{
	const u32 x = reg16(second ? 0x4e : 0x4a);
	const u32 y = reg16(second ? 0x50 : 0x4c);
	for (int row = 0; row < screen().visible_area().height(); row++)
		for (int col = 0; col < screen().visible_area().width(); col++)
			vram_w16(x + col, y + row, 0x0000);
}

void axell_ag2_device::vblank(screen_device &screen, bool state)
{
	if (state && !m_vblank)
	{
		m_pending |= 0x40;
		update_events();
	}
	m_vblank = state;
}

u32 axell_ag2_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);
	if (!BIT(reg8(0x42), 7))
		return 0;
	const bool second = BIT(reg8(0x49), 1);
	const u32 sx = reg16(second ? 0x4e : 0x4a);
	const u32 sy = reg16(second ? 0x50 : 0x4c);
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u32 *const dst = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			const u16 c = vram16(sx + x, sy + y);
			dst[x] = rgb_t(pal5bit(c >> 11), pal6bit(c >> 5), pal5bit(c));
		}
	}
	return 0;
}

u32 axell_ag2_device::draw_list()
{
	u32 pixels = 0;
	for (u32 n = 0; n < 1024; n++)
	{
		// Both observed +28 values (0 and 1) use sequential sheet records.
		const u16 sheet = reg16(0x22) + n * 12;
		const u8 flags = list8(sheet);
		if (!BIT(flags, 0))
			pixels += draw_sheet(sheet);
		if (BIT(flags, 7))
			break;
	}
	return pixels;
}

u32 axell_ag2_device::draw_sheet(u16 sheet)
{
	u32 pixels = 0;
	u16 cell = reg16(0x24) + list16(sheet + 10);
	for (u32 n = 0; n < 1024; n++)
	{
		const u8 flags = list8(cell);
		if (!BIT(flags, 0))
			pixels += draw_cell(cell, sheet);
		if (BIT(flags, 7))
			break;
		// yujogs 01ccc-01cf4 walks these three record sizes.
		const u32 mode = BIT(flags, 5, 2);
		cell += mode == 0 ? 16 : mode == 1 ? 18 : 28;
	}
	return pixels;
}

bool axell_ag2_device::sample_pixel(u16 cell, u16 sheet, int u, int v, u16 &color) const
{
	const u32 format = BIT(list8(cell + 3), 3, 3);
	if (format == 4)
	{
		// Provisional RM1 output layout: packed Y0,Cb,Y1,Cr, as written by
		// rm1_write_frame(). Byte order and full-range BT.601 conversion
		// are unverified.
		const u32 pair_x = (u32(u) & ~1U) * 2;
		const int y = m_vram[vram_addr(pair_x + (u32(u) & 1U) * 2, v)];
		const int cb = int(m_vram[vram_addr(pair_x + 1, v)]) - 128;
		const int cr = int(m_vram[vram_addr(pair_x + 3, v)]) - 128;
		const auto channel = [] (int value) { return std::clamp((value + 500'000) / 1'000'000, 0, 255); };
		const int r = channel(y * 1'000'000 + 1'402'000 * cr);
		const int g = channel(y * 1'000'000 - 344'136 * cb - 714'136 * cr);
		const int b = channel(y * 1'000'000 + 1'772'000 * cb);
		color = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
		return true; // movie pixels are opaque
	}
	const bool rgb565 = format == 3 ? BIT(list8(cell + 3), 0) : BIT(list8(sheet), 5);
	if (format == 3)
		color = vram16(u, v);
	else
	{
		const u8 palette = BIT(list8(cell), 3) ? list8(cell + 1) : list8(sheet + 1);
		u8 index = m_vram[vram_addr(format == 0 ? u >> 1 : u, v)];
		if (format == 0)
			index = BIT(index, (u & 1) * 4, 4);
		else if (format == 1)
			index &= 0x3f; // Upper two storage bits are zero in the observed assets.
		// Anpanman 008fac selects format 1 for a 64-entry palette. Its
		// 0801bc stride table gives both formats 1 and 2 one byte per pixel.
		const u32 entry = (u32(palette) << (format == 0 ? 4 : format == 1 ? 6 : 8)) + index;
		color = vram16(entry & 255, entry >> 8);
		// RGB565 palettes cannot use the RGB555 high-bit transparency flag.
		// Index zero is transparent in the observed BPSC/TMSC assets.
		if (rgb565 && !index)
			return false;
	}
	if (!rgb565)
	{
		if (BIT(color, 15))
			return false;
		color = ((color & 0x7fe0) << 1) | (color & 0x1f) | ((color >> 4) & 0x20);
	}
	return true;
}

void axell_ag2_device::draw_pixel(int x, int y, u16 color, u8 modulation)
{
	const u32 dx = reg16(0x16) + x - (reg16(0x1a) & 0xfff);
	const u32 dy = reg16(0x18) + y - (reg16(0x1c) & 0xfff);
	const u32 mode = modulation >> 5;
	const u32 level = modulation & 31;
	if (mode)
	{
		const u16 dest = vram16(dx, dy);
		const auto channel = [mode, level] (u32 s, u32 d, u32 maximum) -> u32
		{
			switch (mode)
			{
			case 1:
				return (s * (32 - level) + d * level) >> 5;
			case 2:
				return (s * (32 - level)) >> 5;
			// Yujogs' mode-5 sparkle/flash is bright on hardware; its source
			// texture contains yellow/white pixels, so this operation adds.
			// Mode 6 draws the grey oval mask beneath actors (object 61).
			// cscaoysd 20bba-20cbe also pairs these for opposite signed fades.
			case 5:
				return std::min(maximum, d + s);
			case 6:
				return d > s ? d - s : 0;
			default:
				return s; // TODO: other modulation operations
			}
		};
		color = (channel(color >> 11, dest >> 11, 31) << 11)
				| (channel((color >> 5) & 63, (dest >> 5) & 63, 63) << 5)
				| channel(color & 31, dest & 31, 31);
	}
	vram_w16(dx, dy, color);
}

u32 axell_ag2_device::draw_cell(u16 cell, u16 sheet)
{
	const u8 flags = list8(cell);
	const u32 format = BIT(list8(cell + 3), 3, 3);
	if (format > 4)
		return 0; // TODO: other unverified formats
	const u32 mode = BIT(flags, 5, 2);
	const u32 quadrant = BIT(list8(cell + 3), 6, 2);
	const u16 px = list16(cell + 8);
	const u16 py = list16(cell + 10);
	const int sw = list8(cell + 12) | ((px >> 4) & 0x300);
	const int sh = list8(cell + 13) | ((py >> 4) & 0x300);
	const int cx = BIT(px, 11) ? -(px & 0x7ff) : (px & 0x7ff);
	const int cy = BIT(py, 11) ? -(py & 0x7ff) : (py & 0x7ff);
	const int scale_x = list8(sheet + 4);
	const int scale_y = list8(sheet + 5);
	const bool flip_x = BIT(list8(sheet), 1);
	const bool flip_y = BIT(list8(sheet), 2);
	// Sheet bit 6 selects cell modulation. Unused sheet bytes retain RAM
	// test patterns (notably aa) in alternate dchopper display lists.
	const u8 modulation = BIT(list8(sheet), 6) ? list8(cell + 2) : list8(sheet + 2);
	const int bits = format == 0 ? 4 : format <= 2 ? 8 : 16;
	const int source_x = list16(cell + 4);
	const int source_y = list16(cell + 6);
	if (!scale_x || !scale_y)
		return 0;
	const int wx0 = reg16(0x1a) & 0xfff;
	const int wy0 = reg16(0x1c) & 0xfff;
	const int wx1 = reg16(0x1e) & 0xfff;
	const int wy1 = reg16(0x20) & 0xfff;
	const auto sheet_x = [this, sheet, scale_x, flip_x] (int x)
	{
		return list16(sheet + 6) + (((flip_x ? -x : x) * scale_x) >> 5);
	};
	const auto sheet_y = [this, sheet, scale_y, flip_y] (int y)
	{
		return list16(sheet + 8) + (((flip_y ? -y : y) * scale_y) >> 5);
	};

	if (mode == 2)
	{
		// Six signed edge words and an unsigned height describe the four
		// bounding lines. yujogs 01e1e-0238c computes their intercepts and
		// endpoint displacements using sin/cos and the bounding height.
		// The two texture axes are reconstructed from the edge intersections.
		// The edges are 15-bit signed values: Anpanman's constructor
		// (006e1a-006eb2) clears bit 15 of negative results. The bounding
		// height is unsigned 15-bit.
		// TODO: subpixel rounding and non-affine/perspective extensions.
		const auto edge = [this, cell] (u32 offset) -> s64 { return util::sext(list16(cell + offset), 15); };
		const s64 li = edge(14);
		const s64 ri = edge(16);
		const s64 la = edge(18);
		const s64 lb = edge(20);
		const s64 ra = edge(22);
		const s64 rb = edge(24);
		const s64 height = list16(cell + 26) & 0x7fff;
		if (!sw || !sh || !height || la == lb || ra == rb)
			return 0;
		const s64 ly = li * height * 256 / (la - lb);
		const s64 lx = la * ly / height;
		const s64 ry = ri * height * 256 / (ra - rb);
		const s64 rx = ra * ry / height;
		const auto tx = [this, sheet, scale_x, flip_x, cx] (s64 x)
		{
			return s64(list16(sheet + 6)) * 256 + (flip_x ? -1 : 1) * (cx * 256 + x) * scale_x / 32;
		};
		const auto ty = [this, sheet, scale_y, flip_y, cy] (s64 y)
		{
			return s64(list16(sheet + 8)) * 256 + (flip_y ? -1 : 1) * (cy * 256 + y) * scale_y / 32;
		};
		const s64 ox = tx(0);
		const s64 oy = ty(0);
		const s64 ax = tx(rx) - ox;
		const s64 ay = ty(ry) - oy;
		const s64 bx = tx(lx) - ox;
		const s64 by = ty(ly) - oy;
		const double determinant = double(ax) * by - double(ay) * bx;
		if (!determinant)
			return 0;
		const s64 min_x = std::min({ox, ox + ax, ox + bx, ox + ax + bx}) >> 8;
		const s64 max_x = (std::max({ox, ox + ax, ox + bx, ox + ax + bx}) + 255) >> 8;
		const s64 min_y = std::min({oy, oy + ay, oy + by, oy + ay + by}) >> 8;
		const s64 max_y = (std::max({oy, oy + ay, oy + by, oy + ay + by}) + 255) >> 8;
		if (min_x > wx1 || max_x < wx0 || min_y > wy1 || max_y < wy0)
			return 0;
		const int left = std::max<s64>(wx0, min_x);
		const int right = std::min<s64>(wx1, max_x);
		const int top = std::max<s64>(wy0, min_y);
		const int bottom = std::min<s64>(wy1, max_y);
		u32 pixels = 0;
		for (int y = top; y <= bottom; y++)
			for (int x = left; x <= right; x++)
			{
				const s64 dx = x * 256 + 128 - ox;
				const s64 dy = y * 256 + 128 - oy;
				// TODO: hardware precision of the inverse mapping
				const double s = (double(dx) * by - double(dy) * bx) / determinant;
				const double t = (double(ax) * dy - double(ay) * dx) / determinant;
				if (s < 0 || s >= 1 || t < 0 || t >= 1)
					continue;
				const auto sample = [] (double position, int extent) { return std::min(int(position * extent), extent - 1); };
				int u = sample(s, sw);
				int v = sample(t, sh);
				switch (quadrant)
				{
				case 1:
					u = sample(t, sw);
					v = sh - 1 - sample(s, sh);
					break;
				case 2:
					u = sw - 1 - u;
					v = sh - 1 - v;
					break;
				case 3:
					u = sw - 1 - sample(t, sw);
					v = sample(s, sh);
					break;
				}
				u16 color;
				if (sample_pixel(cell, sheet, source_x / bits + u, source_y / 4 + v, color))
				{
					draw_pixel(x, y, color, modulation);
					++pixels;
				}
			}
		return pixels;
	}

	const u32 size_offset = mode == 1 ? 16 : 14;
	int dw = list8(cell + size_offset) | (BIT(flags, 2) ? ((px >> 6) & 0x300) : 0);
	int dh = list8(cell + size_offset + 1) | (BIT(flags, 1) ? ((py >> 6) & 0x300) : 0);
	if (!mode)
	{
		// Type 0 always has independent 10-bit destination dimensions.
		// Anpanman 007308-0073ec and 00777a-00784e populate these with
		// flags 00/80, including 5x10 sources enlarged to 320x480 for fades.
		// Cell flag bits 1/2 do not enable scaling in this record form.
		dw = list8(cell + 14) | ((px >> 6) & 0x300);
		dh = list8(cell + 15) | ((py >> 6) & 0x300);
	}
	if (mode == 3)
	{
		// Quarter turns use the 28-byte record without the edge divisions
		// that would be singular at 0/90/180/270 degrees.
		dw = list16(cell + 16) & 0x7fff;
		dh = list16(cell + 26) & 0x7fff;
	}
	// Type 1 stores vector components, which may individually be zero.
	// Types 0/3 instead describe a source rectangle; an empty one draws nothing.
	if (!dw || !dh || (mode != 1 && (!sw || !sh)))
		return 0;
	const int x0 = sheet_x(cx);
	const int x1 = sheet_x(cx + dw);
	const int y0 = sheet_y(cy);
	const int y1 = sheet_y(cy + dh);
	const int x = std::min(x0, x1);
	const int y = std::min(y0, y1);
	dw = std::abs(x1 - x0);
	dh = std::abs(y1 - y0);
	if (!dw || !dh)
		return 0;
	const int left = std::max(x, wx0);
	const int right = std::min(x + dw - 1, wx1);
	const int top = std::max(y, wy0);
	const int bottom = std::min(y + dh - 1, wy1);
	u32 pixels = 0;
	for (int row = top; row <= bottom; row++)
		for (int col = left; col <= right; col++)
		{
			const s64 sx = flip_x ? dw - 1 - (col - x) : col - x;
			const s64 sy = flip_y ? dh - 1 - (row - y) : row - y;
			int u;
			int v;
			if (mode == 1)
			{
				// 18-byte source-vector form, also used by the AX51102.
				const int csign = quadrant == 1 || quadrant == 2 ? -1 : 1;
				const int ssign = quadrant >= 2 ? -1 : 1;
				// TODO: hardware step precision
				const s64 area = s64(dw) * dh;
				const auto floor_div = [] (s64 value, s64 divisor) { return value / divisor - (value % divisor < 0); };
				u = floor_div(s64(source_x) * area + (sx * sw * csign * dh + sy * list8(cell + 14) * ssign * dw) * bits, area * bits);
				v = floor_div(s64(source_y) * area + (-sx * sh * ssign * dh + sy * list8(cell + 15) * csign * dw) * 4, area * 4);
			}
			else
			{
				u = sx * sw / dw;
				v = sy * sh / dh;
				if (mode == 3)
					switch (quadrant)
					{
					case 1:
						u = sy * sw / dh;
						v = sh - 1 - sx * sh / dw;
						break;
					case 2:
						u = sw - 1 - u;
						v = sh - 1 - v;
						break;
					case 3:
						u = sw - 1 - sy * sw / dh;
						v = sx * sh / dw;
						break;
					}
				u += source_x / bits;
				v += source_y / 4;
			}
			u16 color;
			if (sample_pixel(cell, sheet, u, v, color))
			{
				draw_pixel(col, row, color, modulation);
				++pixels;
			}
		}
	return pixels;
}

void axell_ag2_device::flush_repeats()
{
	if (m_repeats)
		LOGMASKED(LOG_REGISTERS | LOG_HOST, "REPEAT additional=%u offset=%05X data=%02X\n", m_repeats, m_last_offset, m_last_data);
	m_repeats = 0;
}

void axell_ag2_device::device_stop()
{
	flush_repeats();
	LOGMASKED(LOG_STATS, "TOTAL transfers=%u draws=%u decode_errors=%u\n", m_transfers, m_draws, m_decode_errors);
	for (u32 n = 0; n != 256; ++n)
		if (m_reads[n] || m_writes[n])
			LOGMASKED(LOG_STATS, "COUNTS reg=%02X reads=%u writes=%u\n", n, m_reads[n], m_writes[n]);
}

const char *axell_ag2_device::register_name(u8 offset)
{
	switch (offset)
	{
	case 0x00: return "CONTROL_00";
	case 0x01: return "CONTROL_01";
	case 0x02: return "COMMAND";
	case 0x0d: return "CONTROL_0D";
	case 0x0e: return "EVENT_ENABLE";
	case 0x0f: return "EVENT_STATUS_W1C";
	case 0x16: case 0x17: return "DRAW_DEST_X";
	case 0x18: case 0x19: return "DRAW_DEST_Y";
	case 0x22: case 0x23: return "SHEET_BASE";
	case 0x24: case 0x25: return "CELL_BASE";
	case 0x26: case 0x27: return "ORDER_PARAMETER";
	case 0x28: return "LIST_MODE";
	case 0x1a: case 0x1b: case 0x1c: case 0x1d:
	case 0x1e: case 0x1f: case 0x20: case 0x21: return "DRAW_BOUNDS";
	case 0x34: case 0x35: case 0x36: case 0x37: return "TRANSFER_SOURCE";
	case 0x38: case 0x39: case 0x3a: case 0x3b: return "TRANSFER_DEST";
	case 0x3c: case 0x3d: return "TRANSFER_WIDTH_MINUS_ONE";
	case 0x3e: case 0x3f: return "TRANSFER_HEIGHT_MINUS_ONE";
	case 0x40: return "TRANSFER_MODE";
	case 0x42: return "DISPLAY_CONTROL";
	case 0x49: return "DISPLAY_BUFFER_SELECT";
	case 0x4a: case 0x4b: case 0x4c: case 0x4d:
	case 0x4e: case 0x4f: case 0x50: case 0x51: return "DISPLAY_BASE_PARAMETER";
	case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x5a: case 0x5b: case 0x5c: case 0x5d: return "DISPLAY_TIMING";
	case 0x98: return "STATUS_98";
	case 0x9f: return "STATUS_9F";
	default: return "UNKNOWN";
	}
}

void axell_ag2_device::trace(bool write, offs_t offset, u8 data)
{
	if (!(VERBOSE & (LOG_REGISTERS | LOG_HOST | LOG_STATS)) || machine().side_effects_disabled())
		return;
	const bool reg = offset >= REG_BASE && offset < REG_BASE + 0x100;
	if (reg && (VERBOSE & LOG_STATS))
		(write ? m_writes : m_reads)[offset - REG_BASE]++;
	if (!(VERBOSE & (reg ? LOG_REGISTERS : LOG_HOST)))
		return;
	const std::string context = machine().describe_context();
	if (!(VERBOSE & LOG_REPEATS) && m_last_offset == offset && m_last_write == write && m_last_data == data && m_last_context == context)
	{
		++m_repeats;
		return;
	}
	flush_repeats();
	m_last_offset = offset;
	m_last_write = write;
	m_last_data = data;
	m_last_context = context;
	if (reg)
		LOGMASKED(LOG_REGISTERS, "%s %s8 reg=%02X data=%02X %s\n", context, write ? "W" : "R", offset - REG_BASE, data, register_name(offset - REG_BASE));
	else
		LOGMASKED(LOG_HOST, "%s %s8 host=%05X data=%02X opaque\n", context, write ? "W" : "R", offset, data);
}

bool axell_ag2_device::decompress(u32 source, int width, int height, int dx, int dy)
{
	linear_decompressor decompressor(*this, source, m_vram.target(), VRAM_X_BITS, 0xfff, (m_vram_size >> VRAM_X_BITS) - 1);

	// bits 8-10 of the AG-1 block width, not needed with a linear source
	decompressor.get_bits(3);

	return decompressor.decompress(dx, dy, width, height);
}
