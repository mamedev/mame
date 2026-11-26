// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    Axell AG-2 / AX51201 graphics display controller.

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
    +54 line total in two-pixel units, +56 visible width-1, +5a frame total
    in half-lines, +5c visible lines.
    +98 RM1 output slot; +9f RM1 status: bit 3 set while an advance is in
    progress, bit 0 when a frame has been published, bit 1 at the end of
    the stream (inferred from the firmware's status tests).

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

#define LOG_REGS (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_REGS)
#include "logmacro.h"

#define LOGREGS(...) LOGMASKED(LOG_REGS, __VA_ARGS__)


DEFINE_DEVICE_TYPE(AX51201, ax51201_device, "ax51201", "Axell AG-2 AX51201 graphics display controller")

// The compressed stream is linear on the AG-2. Its first byte, which gives
// the width of the block on the AG-1, is skipped.
class ax51201_device::linear_decompressor : public axell_decompressor
{
public:
	linear_decompressor(ax51201_device &host, u32 source, u8 *dest, u32 x_bits, u32 x_mask, u32 y_mask) :
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
	ax51201_device &m_host;
	u32 m_source;
	u32 m_consumed;
};


ax51201_device::ax51201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ax51201_device(mconfig, tag, owner, clock, 0x100'0000)
{
}

ax51201_device::ax51201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 vram_size) :
	device_t(mconfig, AX51201, tag, owner, clock),
	device_rom_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_vram_config("vram", ENDIANNESS_LITTLE, 8, 25, 0),
	m_event_cb(*this),
	m_host(*this, "host", HOST_SIZE, ENDIANNESS_BIG),
	m_vram(*this, "vram", vram_size, ENDIANNESS_LITTLE),
	m_vram_size(vram_size),
	m_vram_mask(vram_size - 1),
	m_completion_timer{},
	m_active(0)
{
}

device_memory_interface::space_config_vector ax51201_device::memory_space_config() const
{
	auto spaces = device_rom_interface::memory_space_config();
	spaces.emplace_back(AS_VRAM, &m_vram_config);
	return spaces;
}

void ax51201_device::map(address_map &map)
{
	map(0x00000, 0x1ffff).rw(FUNC(ax51201_device::ram_r), FUNC(ax51201_device::ram_w));
	map(0x1fc00, 0x1fcff).w(FUNC(ax51201_device::reg_w));
	map(0x1fc02, 0x1fc02).w(FUNC(ax51201_device::command_w));
	map(0x1fc0e, 0x1fc0e).w(FUNC(ax51201_device::event_mask_w));
	map(0x1fc0f, 0x1fc0f).rw(FUNC(ax51201_device::event_status_r), FUNC(ax51201_device::event_status_w));
	map(0x1fc42, 0x1fc42).w(FUNC(ax51201_device::display_control_w));
	map(0x1fc49, 0x1fc49).w(FUNC(ax51201_device::display_select_w));
	map(0x1fc4a, 0x1fc51).w(FUNC(ax51201_device::display_base_w));
	map(0x1fc54, 0x1fc5d).w(FUNC(ax51201_device::display_timing_w));
}

u16 ax51201_device::vram16(u32 x, u32 y) const
{
	const u32 address = vram_addr(x * 2, y);
	return m_vram[address] | (m_vram[(address + 1) & m_vram_mask] << 8);
}

void ax51201_device::vram_w16(u32 x, u32 y, u16 data)
{
	const u32 address = vram_addr(x * 2, y);
	m_vram[address] = data;
	m_vram[(address + 1) & m_vram_mask] = data >> 8;
}

void ax51201_device::device_validity_check(validity_checker &valid) const
{
	if (m_vram_size < 0x80'0000 || m_vram_size > 0x200'0000 || (m_vram_size & (m_vram_size - 1)))
		osd_printf_error("VRAM must be 8, 16 or 32 MiB\n");
}

void ax51201_device::device_start()
{
	space(AS_VRAM).install_ram(0, m_vram_size - 1, m_vram.target());
	for (auto &timer : m_completion_timer)
		timer = timer_alloc(FUNC(ax51201_device::complete), this);
	screen().register_vblank_callback(vblank_state_delegate(&ax51201_device::vblank, this));

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

void ax51201_device::device_reset()
{
	std::fill_n(&m_host[REG_BASE], HOST_SIZE - REG_BASE, 0); // unverified reset register values
	m_pending = 0;
	m_vblank = false;
	rm1_reset();
	for (auto timer : m_completion_timer)
		timer->adjust(attotime::never);
	update_events();
}

void ax51201_device::device_post_load()
{
	rm1_restore();
	update_screen_size();
	m_active = m_pending & reg8(0x0e) & 0x55;
}

u8 ax51201_device::ram_r(offs_t offset)
{
	return m_host[offset];
}

void ax51201_device::ram_w(offs_t offset, u8 data)
{
	m_host[offset] = data;
}

void ax51201_device::reg_w(offs_t offset, u8 data)
{
	LOGREGS("%s: register %02x = %02x\n", machine().describe_context(), offset, data);
	m_host[REG_BASE + offset] = data;
}

void ax51201_device::command_w(u8 data)
{
	reg_w(0x02, data);
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
	{
		transfer();
	}
	else
	{
		const u32 pixels = draw_list();
		schedule_event(0x10, std::max<u32>(64, pixels));
	}
}

void ax51201_device::event_mask_w(u8 data)
{
	reg_w(0x0e, data);
	update_events();
}

u8 ax51201_device::event_status_r()
{
	return m_pending;
}

void ax51201_device::event_status_w(u8 data)
{
	LOGREGS("%s: clear events %02x\n", machine().describe_context(), data);
	m_pending &= ~data;
	update_events();
}

void ax51201_device::display_control_w(u8 data)
{
	screen().update_partial(screen().vpos());
	reg_w(0x42, data);
}

void ax51201_device::display_select_w(u8 data)
{
	screen().update_partial(screen().vpos());
	const u8 previous = reg8(0x49);
	reg_w(0x49, data);
	if (BIT(previous ^ data, 1))
		clear_frame_buffer(!BIT(data, 1)); // TODO: identify the actual clear control
}

void ax51201_device::display_base_w(offs_t offset, u8 data)
{
	screen().update_partial(screen().vpos());
	reg_w(0x4a + offset, data);
}

void ax51201_device::display_timing_w(offs_t offset, u8 data)
{
	reg_w(0x54 + offset, data);
	update_screen_size();
}

void ax51201_device::update_events()
{
	const u8 active = m_pending & reg8(0x0e) & 0x55;
	const u8 changed = active ^ m_active;
	m_active = active;
	for (u32 n = 0; n < 4; n++)
	{
		if (BIT(changed, n * 2))
			m_event_cb[n](BIT(active, n * 2));
	}
}

void ax51201_device::schedule_event(u8 event, u64 clocks)
{
	// one timer per engine; clock and execution cost are provisional
	const u32 index = (event == 4) ? 0 : (event == 0x10) ? 1 : 2;
	m_completion_timer[index]->adjust(attotime::from_ticks(std::max<u64>(1, clocks), clock()), event);
}

TIMER_CALLBACK_MEMBER(ax51201_device::complete)
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

void ax51201_device::rm1_reset()
{
	// Hide & Seek issues 80 before programming the movie registers. Its exact
	// abort/reset semantics are unverified; it must leave the first 82 usable.
	rm1_clear();
	update_events();
}

void ax51201_device::rm1_clear()
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

u32 ax51201_device::rm1_read(u32 address, u32 bytes)
{
	if (bytes > 4 || u64(address) + bytes > CGROM_SIZE)
		throw axell_rm1::decode_error("RM1 field exceeds CG ROM");
	u32 result = 0;
	for (u32 i = 0; i < bytes; ++i)
		result = (result << 8) | rom8(address + i);
	return result;
}

const u8 *ax51201_device::rm1_payload(u32 address, u32 size)
{
	if (u64(address) + size > CGROM_SIZE)
		throw axell_rm1::decode_error("RM1 payload exceeds CG ROM");
	m_rm1_buffer.resize(size);
	for (u32 i = 0; i < size; ++i)
		m_rm1_buffer[i] = rom8(address + i);
	return m_rm1_buffer.data();
}

void ax51201_device::rm1_initialize()
{
	m_rm1_base = (u32(reg16(0x84)) << 16) | reg16(0x86);
	if (u64(m_rm1_base) + 0x24 > CGROM_SIZE
		|| rm1_read(m_rm1_base, 4) != 0x432d'5241 || rm1_read(m_rm1_base + 4, 4) != 0x5049'4301)
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

void ax51201_device::rm1_write_frame(u32 slot, const axell_rm1::frame &picture)
{
	// The three programmed X coordinates are in two-pixel units: Hide & Seek
	// uses 0/320/640, with format-4 sprite pixel origins 0/640/1280.
	// Provisional packing: Y0,Cb,Y1,Cr at the normal 4096-byte VRAM pitch.
	// Physical byte order and the 4:2:0->4:2:2 chroma filter need hardware data.
	const u32 dx = u32(reg16(0x70 + slot * 4)) * 4;
	const u32 dy = reg16(0x72 + slot * 4);
	for (u32 y = 0; y < axell_rm1::frame::HEIGHT; ++y)
	{
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
}

u32 ax51201_device::rm1_decode_next()
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

void ax51201_device::rm1_advance()
{
	// +9f bit 3 blocks another 82 while an advance is in progress; bit 1 is
	// set at the end of the stream
	if (((reg8(0x9f) & 9) == 8) || BIT(reg8(0x9f), 1))
		return;
	m_host[REG_BASE + 0x9f] = 8;
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
		{
			rm1_write_frame(m_rm1_ready_slot, m_rm1_anchor_picture);
		}
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
		logerror("RM1 failed container=%08x packet=%u: %s\n", m_rm1_base, m_rm1_decoded, error.what());
		// error status bits are unknown: no completion is signalled, and
		// command 80 reinitializes the engine
	}
}

void ax51201_device::rm1_restore()
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

void ax51201_device::transfer()
{
	const u32 source = reg16(0x34) | (u32(reg16(0x36)) << 16);
	const u32 dx = reg16(0x38) * 2;
	const u32 dy = reg16(0x3a);
	const u32 width = ((reg16(0x3c) & 0x3ff) + 1) * 2;
	const u32 height = (reg16(0x3e) & 0x3ff) + 1;
	const u8 mode = reg8(0x40) & 0x1f;
	bool error = false;
	if (mode == 9)
	{
		error = decompress(source, width, height, dx, dy);
	}
	else if (mode == 1 || mode == 0x13)
	{
		for (u32 y = 0; y < height; y++)
		{
			for (u32 x = 0; x < width; x++)
			{
				const u32 a = source + y * width + x;
				m_vram[vram_addr(dx + x, dy + y)] = mode == 1 ? rom8(a) : list8(a);
			}
		}
	}
	else
	{
		error = true;
		logerror("unimplemented transfer mode %02x\n", mode);
	}
	if (error)
		logerror("transfer failed mode=%02x source=%08x size=%ux%u\n", mode, source, width, height);
	schedule_event(4, std::max<u64>(64, u64(width) * height));
}

void ax51201_device::update_screen_size()
{
	// Hide & Seek programs 400/1050 with 640x480 (the 800x525 VGA timing)
	// and the Banpresto and Tomy games 254/524 with 320x240 (508x262, a
	// 240-line timing), so the totals are taken as two-pixel and half-line
	// units. The dot clock is not identified; the frame period is kept.
	const u32 width = reg16(0x56) + 1;
	const u32 height = reg16(0x5c);
	if (width < 120 || width > 1024 || height < 100 || height > 768)
		return;
	u32 htotal = reg16(0x54) * 2;
	u32 vtotal = reg16(0x5a) / 2;
	if (htotal <= width || htotal > 2048)
		htotal = width;
	if (vtotal <= height || vtotal > 1024)
		vtotal = height;
	if (screen().width() != htotal || screen().height() != vtotal
		|| screen().visible_area().width() != width || screen().visible_area().height() != height)
		screen().configure(htotal, vtotal, rectangle(0, width - 1, 0, height - 1), screen().frame_period());
}

void ax51201_device::clear_frame_buffer(bool second)
{
	const u32 x = reg16(second ? 0x4e : 0x4a);
	const u32 y = reg16(second ? 0x50 : 0x4c);
	for (int row = 0; row < screen().visible_area().height(); row++)
	{
		for (int col = 0; col < screen().visible_area().width(); col++)
			vram_w16(x + col, y + row, 0x0000);
	}
}

void ax51201_device::vblank(screen_device &screen, bool state)
{
	if (state && !m_vblank)
	{
		m_pending |= 0x40;
		update_events();
	}
	m_vblank = state;
}

u32 ax51201_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

u32 ax51201_device::draw_list()
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

u32 ax51201_device::draw_sheet(u16 sheet)
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

bool ax51201_device::sample_pixel(const cell_context &context, int u, int v, u16 &color) const
{
	if (context.format == 4)
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
	if (context.format == 3)
	{
		color = vram16(u, v);
	}
	else
	{
		u8 index = m_vram[vram_addr(context.format == 0 ? u >> 1 : u, v)];
		if (context.format == 0)
			index = BIT(index, (u & 1) * 4, 4);
		else if (context.format == 1)
			index &= 0x3f; // Upper two storage bits are zero in the observed assets.
		const u32 entry = context.palette_base + index;
		color = vram16(entry & 255, entry >> 8);
		// RGB565 palettes cannot use the RGB555 high-bit transparency flag.
		// Index zero is transparent in the observed BPSC/TMSC assets.
		if (context.rgb565 && !index)
			return false;
	}
	if (!context.rgb565)
	{
		if (BIT(color, 15))
			return false;
		color = ((color & 0x7fe0) << 1) | (color & 0x1f) | ((color >> 4) & 0x20);
	}
	return true;
}

void ax51201_device::draw_pixel(const cell_context &context, int x, int y, u16 color)
{
	const u32 dx = context.dest_x + x;
	const u32 dy = context.dest_y + y;
	if (context.blend_mode)
	{
		const u16 dest = vram16(dx, dy);
		const u32 mode = context.blend_mode;
		const u32 level = context.blend_level;
		const auto channel = [mode, level] (u32 s, u32 d, u32 maximum) -> u32
		{
			switch (mode)
			{
			case 1:
				return (s * (32 - level) + d * level) >> 5;
			case 2:
				return (s * (32 - level)) >> 5;
			case 5: // additive (yujogs sparkles, cscaoysd fades)
				return std::min(maximum, d + s);
			case 6: // subtractive (yujogs actor shadows, cscaoysd fades)
				return d > s ? d - s : 0;
			default:
				return s; // TODO: modulation modes 3, 4 and 7
			}
		};
		color = (channel(color >> 11, dest >> 11, 31) << 11)
				| (channel((color >> 5) & 63, (dest >> 5) & 63, 63) << 5)
				| channel(color & 31, dest & 31, 31);
	}
	vram_w16(dx, dy, color);
}

u32 ax51201_device::draw_cell(u16 cell, u16 sheet)
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
	const int bits = format == 0 ? 4 : format <= 2 ? 8 : 16;
	const int source_x = list16(cell + 4);
	const int source_y = list16(cell + 6);
	if (!scale_x || !scale_y)
		return 0;
	const int wx0 = reg16(0x1a) & 0xfff;
	const int wy0 = reg16(0x1c) & 0xfff;
	const int wx1 = reg16(0x1e) & 0xfff;
	const int wy1 = reg16(0x20) & 0xfff;

	// Sheet bit 6 selects cell modulation. Unused sheet bytes retain RAM
	// test patterns (notably aa) in alternate dchopper display lists.
	const u8 modulation = BIT(list8(sheet), 6) ? list8(cell + 2) : list8(sheet + 2);
	// Anpanman 008fac selects format 1 for a 64-entry palette. Its
	// 0801bc stride table gives both formats 1 and 2 one byte per pixel.
	const u8 palette = BIT(flags, 3) ? list8(cell + 1) : list8(sheet + 1);
	cell_context context;
	context.format = format;
	context.rgb565 = format == 3 ? BIT(list8(cell + 3), 0) : BIT(list8(sheet), 5);
	context.palette_base = u32(palette) << (format == 0 ? 4 : format == 1 ? 6 : 8);
	context.blend_mode = modulation >> 5;
	context.blend_level = modulation & 31;
	context.dest_x = u32(int(reg16(0x16)) - wx0);
	context.dest_y = u32(int(reg16(0x18)) - wy0);

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
		{
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
				if (sample_pixel(context, source_x / bits + u, source_y / 4 + v, color))
				{
					draw_pixel(context, x, y, color);
					++pixels;
				}
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
	{
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
				{
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
				}
				u += source_x / bits;
				v += source_y / 4;
			}
			u16 color;
			if (sample_pixel(context, u, v, color))
			{
				draw_pixel(context, col, row, color);
				++pixels;
			}
		}
	}
	return pixels;
}

bool ax51201_device::decompress(u32 source, int width, int height, int dx, int dy)
{
	linear_decompressor decompressor(*this, source, m_vram.target(), VRAM_X_BITS, 0xfff, (m_vram_size >> VRAM_X_BITS) - 1);

	// bits 8-10 of the AG-1 block width, not needed with a linear source
	decompressor.get_bits(3);

	return decompressor.decompress(dx, dy, width, height);
}
