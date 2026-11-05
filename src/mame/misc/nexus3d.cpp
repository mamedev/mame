// license:BSD-3-Clause
// copyright-holders:Scott Stone
/************************************************************************

    NEXUS 3D Version 1.0 Board from Interpark

    Games on this platform:

    Arcana Heart FULL, Examu Inc, 2006

    MagicEyes VRENDER 3D Soc (200 MHz ARM920T CPU / GFX / Sound)
    Also Has 2x QDSP QS1000 for sound

    Music is provisionally working using Hidden Catch's external QS1000
    MIDI firmware in place of U60's undumped internal ROM. P1.0 selects its
    QS1001A instrument map; four piano-layer pointers are adapted to the
    stock sample ROM. This substitute is not verified original firmware.
    Replace it and remove the compatibility patches when U60 is dumped.

    TODO:
    - identify the on-board copy-protection device and verify its simulated protocol
    - video timing, MLC layer controls, command FIFO and remaining GTE operations
    - perspective correction, depth test, texture filtering and texture formats
    - dump the second QS1000 internal ROM and verify music timbre and mixing
    - verify sound ROM banking and board clock frequencies

*/

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "machine/ds1307.h"
#include "machine/gen_latch.h"
#include "machine/i2cmem.h"
#include "machine/nandflash.h"
#include "sound/qs1000.h"
#include "video/poly.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "multibyte.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <iterator>
#include <memory>

#define LOG_PROTECTION (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

namespace {

// Examu on-board copy-protection key, simulated from the host software.
// No device firmware or hardware trace is available. The identification table
// has a 128-byte length header in both games; bus turnaround and timing remain
// hypotheses rather than a low-level implementation.
// Command 04 returns a length, identification bytes, then ff.  Command 01
// reads the presumed DIP switch bank.  Full also writes 02 then 00 with the
// strobe low on coin 2; this output-only transaction has no reply.
class nexus3d_key_device : public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::PROTECTION; }

	nexus3d_key_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto inputs_callback() { return m_inputs_cb.bind(); }
	auto counter_callback() { return m_counter_cb.bind(); }

	void host_w(uint8_t data, uint8_t drive_mask, int strobe);
	uint8_t data_r() const;
	int ready_r() const;

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void begin_reply(uint8_t command);
	void counter_w(bool state);
	TIMER_CALLBACK_MEMBER(reply_ready);

	required_ioport m_config;
	devcb_read8 m_inputs_cb;
	devcb_write_line m_counter_cb;
	emu_timer *m_reply_timer;
	bool m_strobe;
	bool m_command_pending;
	bool m_busy;
	bool m_counter_mode;
	bool m_counter_level;
	uint8_t m_reply[129];
	uint16_t m_reply_length;
	uint16_t m_reply_index;
	uint8_t m_data;
};

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NEXUS3D_KEY, nexus3d_key_device, nexus3d_key_device,
		"nexus3d_key", "Examu on-board copy-protection key (HLE)")

namespace {

nexus3d_key_device::nexus3d_key_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEXUS3D_KEY, tag, owner, clock),
	m_config(*this, "CONFIG"),
	m_inputs_cb(*this, 0xff),
	m_counter_cb(*this),
	m_strobe(true)
{
}

static INPUT_PORTS_START( nexus3d_key )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Examu on-board copy-protection key (simulation)" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END

ioport_constructor nexus3d_key_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(nexus3d_key);
}

void nexus3d_key_device::device_start()
{
	m_reply_timer = timer_alloc(FUNC(nexus3d_key_device::reply_ready), this);
	m_counter_level = false;
	m_counter_cb(0);
	save_item(NAME(m_strobe));
	save_item(NAME(m_command_pending));
	save_item(NAME(m_busy));
	save_item(NAME(m_counter_mode));
	save_item(NAME(m_counter_level));
	save_item(NAME(m_reply));
	save_item(NAME(m_reply_length));
	save_item(NAME(m_reply_index));
	save_item(NAME(m_data));
}

void nexus3d_key_device::device_reset()
{
	m_reply_timer->adjust(attotime::never);
	m_command_pending = m_busy = false;
	m_counter_mode = false;
	counter_w(false);
	std::fill(std::begin(m_reply), std::end(m_reply), 0xff);
	m_reply_length = m_reply_index = 0;
	m_data = 0xff;
}

uint8_t nexus3d_key_device::data_r() const
{
	return BIT(m_config->read(), 0) ? m_data : 0xff;
}

int nexus3d_key_device::ready_r() const
{
	return !BIT(m_config->read(), 0) || !m_busy;
}

void nexus3d_key_device::host_w(uint8_t data, uint8_t drive_mask, int strobe)
{
	bool const falling = m_strobe && !strobe;
	m_strobe = bool(strobe);
	if (!BIT(m_config->read(), 0))
		return;

	if (drive_mask == 0xff && data == 0x02 && !m_strobe)
		m_counter_mode = true;
	if (m_counter_mode)
	{
		// Full's coin-2 handler holds 02 across the falling strobe, then
		// clears the data before raising the strobe. It never reads a reply.
		counter_w(drive_mask == 0xff && BIT(data, 1) && !m_strobe);
		m_command_pending = m_busy = false;
		m_reply_length = m_reply_index = 0;
		m_reply_timer->adjust(attotime::never);
		if (m_strobe)
			m_counter_mode = false;
		return;
	}

	if (drive_mask == 0xff && !m_strobe)
	{
		m_reply_timer->adjust(attotime::never);
		m_command_pending = m_busy = true;
		m_reply_length = m_reply_index = 0;
	}
	else if (m_command_pending && !drive_mask)
	{
		m_command_pending = false;
		begin_reply(data);
	}
	else if (falling && m_reply_length && !m_busy)
	{
		++m_reply_index;
		m_busy = true;
		// Provisional service delay; the physical device clock is unknown.
		m_reply_timer->adjust(attotime::from_usec(50));
	}
}

void nexus3d_key_device::counter_w(bool state)
{
	if (m_counter_level != state)
	{
		m_counter_level = state;
		m_counter_cb(state ? 1 : 0);
	}
}

void nexus3d_key_device::begin_reply(uint8_t command)
{
	// Comparison data embedded in both games, not a dump of the key ROM.
	static constexpr uint8_t IDENTIFICATION[] = {
		0xc4, 0xff, 0xff, 0xea, 0x43, 0x72, 0x61, 0x7a, 0x79, 0x20, 0x41, 0x72, 0x63, 0x61, 0x64, 0x65,
		0x20, 0x42, 0x6e, 0x42, 0x20, 0x28, 0x56, 0x45, 0x47, 0x41, 0x33, 0x29, 0x00, 0x00, 0x00, 0x00,
		0xe8, 0x13, 0x9f, 0xe5, 0x50, 0x00, 0x81, 0xe5, 0x0e, 0xf0, 0xa0, 0xe1, 0xdc, 0x03, 0x9f, 0xe5,
		0x4c, 0x00, 0x90, 0xe5, 0x0e, 0xf0, 0xa0, 0xe1, 0xd0, 0x13, 0x9f, 0xe5, 0x00, 0x00, 0xa0, 0xe3,
		0x10, 0x00, 0x81, 0xe5, 0x0e, 0xf0, 0xa0, 0xe1, 0x70, 0x40, 0x2d, 0xe9, 0xbc, 0x53, 0x9f, 0xe5,
		0x00, 0x40, 0xa0, 0xe1, 0x44, 0x00, 0x85, 0xe5, 0x3c, 0x00, 0xa0, 0xe3, 0x48, 0x00, 0x85, 0xe5,
		0x0a, 0x00, 0x54, 0xe3, 0x07, 0x00, 0x00, 0xaa, 0x00, 0x00, 0x54, 0xe3, 0x03, 0x00, 0x00, 0xaa,
		0x12, 0x00, 0xa0, 0xe3, 0x03, 0xec, 0xff, 0xeb, 0x00, 0x00, 0xa0, 0xe3, 0x70, 0x80, 0xbd, 0xe8
	};

	LOGMASKED(LOG_PROTECTION, "%s: command %02x\n", machine().describe_context(), command);
	switch (command)
	{
	case 0x00:
		// Bus initialisation/release; neither game requests a reply.
		m_busy = false;
		m_data = 0xff;
		return;
	case 0x01:
		m_reply[0] = m_inputs_cb();
		m_reply_length = 1;
		break;
	case 0x04:
		m_reply[0] = std::size(IDENTIFICATION);
		std::copy(std::begin(IDENTIFICATION), std::end(IDENTIFICATION), &m_reply[1]);
		m_reply_length = std::size(IDENTIFICATION) + 1;
		break;
	default:
		logerror("%s: unhandled protection command %02x\n", machine().describe_context(), command);
		return;
	}
	m_reply_index = 0;
	// Provisional service delay, as for subsequent response bytes.
	m_reply_timer->adjust(attotime::from_usec(50));
}

TIMER_CALLBACK_MEMBER(nexus3d_key_device::reply_ready)
{
	m_busy = false;
	if (m_reply_index < m_reply_length)
		m_data = m_reply[m_reply_index];
	else
	{
		m_data = 0xff;
		m_reply_length = m_reply_index = 0;
	}
}

// VR3511F has the older GRP3D command set, also used by MES_GRP3D02.
// In particular, its texture and primitive registers differ from POLLUX.

// Register state latched when work is queued, so the CPU can program the
// next command while the rasterizer threads are still drawing.
struct nexus3d_poly_data
{
	uint32_t state;         // 0x28
	uint32_t blend;         // 0x30
	uint32_t segment;       // 0x38
	uint32_t key;           // 0x40
	uint32_t texinfo;       // 0x50
	uint32_t texsize;       // 0x54
	uint32_t texblend;      // 0x60
	uint32_t fill;          // 0xa0
	uint32_t target;        // 0xb0
	uint16_t palette[256];
};

class nexus3d_renderer : public poly_manager<float, nexus3d_poly_data, 7>
{
public:
	nexus3d_renderer(running_machine &machine, address_space &space, uint32_t *mainram, uint32_t *texram, uint32_t *regs,
		float (*working)[4], float (*output)[3][4], uint16_t (*palette)[256])
		: poly_manager<float, nexus3d_poly_data, 7>(machine),
		m_space(space), m_mainram(mainram), m_texram(texram), m_regs(regs), m_working(working), m_output(output), m_palette(palette),
		m_texture_used{}, m_pending(false), m_unprotected(false)
	{ }

	void execute(uint32_t control);
	uint16_t display_pixel(int x, int y) const;

	// Queued work reads only the texture blocks it has marked and writes only
	// render targets at 0x07000000-0x07ffffff (anything else is finished
	// before execute() returns), so only CPU accesses to those have to wait.
	static constexpr offs_t TARGET_START = 0x07000000, TARGET_END = 0x07ffffff;
	void sync();
	bool pending() const { return m_pending; }
	bool texture_used(offs_t address) const { return m_pending && BIT(m_texture_used[(address & 0x7ffffff) >> 17], (address >> 11) & 63); }

private:
	static uint32_t tiled_address(uint32_t segment, uint32_t x, uint32_t y, bool texture);
	static uint32_t target_address(uint32_t info, uint32_t x, uint32_t y);
	static bool in_target_window(uint32_t segment) { return (segment & 0x7f8) == (TARGET_START >> 21); }
	uint32_t *memory_pointer(uint32_t address) const;
	uint16_t read_pixel(uint32_t address) const;
	void write_pixel(uint32_t address, uint16_t color);
	void fill_scanline(int32_t y, extent_t const &extent, nexus3d_poly_data const &data, int threadid);
	void draw_scanline(int32_t y, extent_t const &extent, nexus3d_poly_data const &data, int threadid);
	void fill_memory();
	void load_palette();
	void latch_draw();
	void mark_texture(uint32_t segment, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);
	void draw_primitives();
	void run_gte(int vertex, bool render);
	void draw_triangle();
	uint16_t texture_pixel(nexus3d_poly_data const &data, int u, int v) const;
	float reg_float(int address) const { return std::bit_cast<float>(m_regs[address / 4]); }

	address_space &m_space;
	uint32_t *const m_mainram;
	uint32_t *const m_texram;
	uint32_t *const m_regs;
	float (*const m_working)[4];
	float (*const m_output)[3][4];
	uint16_t (*const m_palette)[256];
	uint64_t m_texture_used[0x8000000 >> 17]; // one bit per 2 KiB block (64 bytes by 32 lines)
	bool m_pending;
	bool m_unprotected;
};

uint32_t nexus3d_renderer::tiled_address(uint32_t segment, uint32_t x, uint32_t y, bool texture)
{
	uint32_t address = (segment & 0x7ff) << 21;
	address |= ((y & 0x3e0) << 11) | ((x & 0x7c0) << 5);
	if (texture)
		address |= ((y & 0x1e) << 6) | ((x & 0x3c) << 1) | ((y & 1) << 2) | (x & 3);
	else
		address |= ((y & 0x1f) << 6) | (x & 0x3f);
	return address;
}

uint32_t nexus3d_renderer::target_address(uint32_t info, uint32_t x, uint32_t y)
{
	// Each two-bit table entry selects a 512-byte by 256-line quadrant.
	x *= 2;
	x = (x & 0x1ff) | (BIT(info, ((x >> 9) & 3) * 2, 2) << 9);
	y = (y & 0xff) | (BIT(info, 8 + ((y >> 8) & 3) * 2, 2) << 8);
	return tiled_address(BIT(info, 16, 11), x, y, false);
}

uint32_t *nexus3d_renderer::memory_pointer(uint32_t address) const
{
	// Sub-segments retain the CPU memory-bank bits: 038 is in main SDRAM,
	// while 338 is in texture SDRAM. Rasterizer workers use the shared RAM directly.
	switch (address & 0xf8000000)
	{
	case 0x00000000: return &m_mainram[address / 4];
	case 0x60000000: return &m_texram[(address - 0x60000000) / 4];
	default: return nullptr;
	}
}

uint16_t nexus3d_renderer::read_pixel(uint32_t address) const
{
	uint32_t const *const word = memory_pointer(address);
	return word ? (*word >> ((address & 2) * 8)) : 0xffff;
}

void nexus3d_renderer::write_pixel(uint32_t address, uint16_t color)
{
	if (uint32_t *const word = memory_pointer(address))
	{
		uint32_t const shift = (address & 2) * 8;
		*word = (*word & ~(0xffffU << shift)) | (uint32_t(color) << shift);
	}
}

uint16_t nexus3d_renderer::display_pixel(int x, int y) const
{
	return read_pixel(target_address(m_regs[0x98 / 4], x, y));
}

void nexus3d_renderer::execute(uint32_t control)
{
	if (BIT(control, 2))
		return;
	switch (control & 3)
	{
	case 0: latch_draw(); draw_primitives(); break;
	case 1: latch_draw(); run_gte(BIT(control, 6, 2) % 3, true); break;
	case 2: latch_draw(); draw_triangle(); break;
	case 3:
		switch (BIT(control, 3, 2))
		{
		case 1: fill_memory(); break;
		case 2: load_palette(); break;
		case 3: break; // texture cache invalidation
		}
		break;
	}
	// Work the CPU-side checks can't see has to finish now.
	if (m_unprotected)
		sync();
}

void nexus3d_renderer::sync()
{
	wait();
	if (m_pending)
	{
		std::fill(std::begin(m_texture_used), std::end(m_texture_used), 0);
		m_pending = m_unprotected = false;
	}
}

void nexus3d_renderer::latch_draw()
{
	nexus3d_poly_data &data = object_data().next();
	data.state = m_regs[0x28 / 4];
	data.blend = m_regs[0x30 / 4];
	data.segment = m_regs[0x38 / 4];
	data.key = m_regs[0x40 / 4];
	data.texinfo = m_regs[0x50 / 4];
	data.texsize = m_regs[0x54 / 4];
	data.texblend = m_regs[0x60 / 4];
	data.target = m_regs[0xb0 / 4];
	m_pending = true;
	if (!in_target_window(BIT(data.target, 16, 11)))
		m_unprotected = true;
	if (!BIT(data.state, 1))
		return;

	// Mark the texture memory texture_pixel() can read.
	uint32_t const info = data.texinfo, size = data.texsize;
	uint32_t const format = BIT(info, 19, 2);
	uint32_t const shift = (format < 2) ? 2 - format : 0;
	bool const indexed = BIT(info, 16);
	uint32_t const umask = ((BIT(size, 16, 8) + 1) << (indexed ? 4 : (shift + 1))) - 1;
	uint32_t const vmask = ((BIT(size, 24, 8) + 1) << (indexed ? 4 : 1)) - 1;
	uint32_t const xbase = (BIT(info, 25) << 10) | ((size & 0xff) << 2);
	uint32_t const ybase = (BIT(info, 26) << 9) | (BIT(size, 8, 8) << 1);
	uint32_t const segment = data.segment & 0x7ff;
	if (format < 2)
		std::copy(std::begin(m_palette[0]), std::end(m_palette[0]), std::begin(data.palette));
	if ((segment & 0x7c0) != 0x300)
		m_unprotected = true;
	else if (indexed)
	{
		// The tile map, then tiles anywhere on its page.
		mark_texture(segment, xbase, xbase + (umask >> 3) * 2 + 1, ybase, ybase + (vmask >> 3));
		mark_texture(segment, BIT(info, 25) << 10, (BIT(info, 25) << 10) | 0x3ff, BIT(info, 26) << 9, (BIT(info, 26) << 9) | 0x1ff);
	}
	else
		mark_texture(segment, xbase, xbase + ((umask * 2) >> shift) + 1, ybase, ybase + vmask);
}

void nexus3d_renderer::mark_texture(uint32_t segment, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
	// Coordinates past the sub-segment wrap; just cover all of it.
	if (x1 > 0x7ff || y1 > 0x3ff)
	{
		x0 = y0 = 0;
		x1 = 0x7ff;
		y1 = 0x3ff;
	}
	for (uint32_t y = y0 & ~0x1f; y <= y1; y += 0x20)
	{
		for (uint32_t x = x0 & ~0x3f; x <= x1; x += 0x40)
		{
			uint32_t const block = (tiled_address(segment, x, y, true) & 0x7ffffff) >> 11;
			m_texture_used[block >> 6] |= uint64_t(1) << (block & 63);
		}
	}
}

void nexus3d_renderer::fill_memory()
{
	uint32_t const start = m_regs[0xa4 / 4], end = m_regs[0xa8 / 4];
	vertex_t const v0(float(start & 0x3ff), float(BIT(start, 16, 10)));
	vertex_t const v1(float((end & 0x3ff) + 1), float(BIT(end, 16, 10) + 1));
	nexus3d_poly_data &data = object_data().next();
	data.fill = m_regs[0xa0 / 4];
	m_pending = true;
	if (!in_target_window(BIT(data.fill, 16, 11)))
		m_unprotected = true;
	render_tile<0>(rectangle(0, 1023, 0, 1023), render_delegate(&nexus3d_renderer::fill_scanline, this), v0, v1);
}

void nexus3d_renderer::fill_scanline(int32_t y, extent_t const &extent, nexus3d_poly_data const &data, int threadid)
{
	uint32_t const info = data.fill;
	for (int x = extent.startx; x < extent.stopx; x++)
		write_pixel(tiled_address(BIT(info, 16, 11), x * 2, y, false), uint16_t(info));
}

void nexus3d_renderer::load_palette()
{
	uint32_t const info = m_regs[0x48 / 4];
	uint32_t const segment = m_regs[0x38 / 4] & 0x7ff;
	uint32_t const x = (BIT(info, 25) << 10) | ((info & 0xff) << 2);
	uint32_t const y = (BIT(info, 26) << 9) | (BIT(info, 8, 8) << 1);
	int const count = BIT(info, 17) ? 256 : 16;
	int const first = BIT(info, 17) ? 0 : BIT(info, 21, 4) * 16;
	for (int i = 0; i < count; i++)
		m_palette[BIT(info, 16)][first + i] = read_pixel(tiled_address(segment, x + (i & 7) * 2, y + (i >> 3), true));
}

uint16_t nexus3d_renderer::texture_pixel(nexus3d_poly_data const &data, int u, int v) const
{
	uint32_t const info = data.texinfo, size = data.texsize;
	uint32_t const format = BIT(info, 19, 2);
	uint32_t const shift = (format < 2) ? 2 - format : 0;
	bool const indexed = BIT(info, 16);
	uint32_t const umask = ((BIT(size, 16, 8) + 1) << (indexed ? 4 : (shift + 1))) - 1;
	uint32_t const vmask = ((BIT(size, 24, 8) + 1) << (indexed ? 4 : 1)) - 1;
	u = BIT(info, 0) ? std::clamp(u, 0, int(umask)) : (u & umask);
	v = BIT(info, 1) ? std::clamp(v, 0, int(vmask)) : (v & vmask);
	uint32_t const xbase = (BIT(info, 25) << 10) | ((size & 0xff) << 2);
	uint32_t const ybase = (BIT(info, 26) << 9) | (BIT(size, 8, 8) << 1);
	uint32_t const segment = data.segment & 0x7ff;
	uint32_t address;
	if (indexed)
	{
		uint16_t const tile = read_pixel(tiled_address(segment, xbase + (u >> 3) * 2, ybase + (v >> 3), true));
		uint32_t const columns = 64U << shift;
		uint32_t const x = ((tile & (columns - 1)) * 8 + (u & 7)) * 2 >> shift;
		uint32_t const y = ((tile / columns) & 63) * 8 + (v & 7);
		address = tiled_address(segment, (BIT(info, 25) << 10) | x, (BIT(info, 26) << 9) | y, true);
	}
	else
		address = tiled_address(segment, xbase + ((u * 2) >> shift), ybase + v, true);
	if (format >= 2)
		return read_pixel(address);
	uint8_t const packed = read_pixel(address) >> ((address & 1) * 8);
	uint8_t const index = format ? packed : ((packed >> ((u & 1) ? 0 : 4)) & 15);
	return data.palette[index + (format ? 0 : BIT(info, 21, 4) * 16)];
}

void nexus3d_renderer::run_gte(int vertex, bool render)
{
	// Input and constant vectors are separate banks on this revision.
	// TODO: instruction pipeline timing, loop instructions and reciprocal latency.
	static constexpr char swizzle[32][5] = {
		"xyzw", "yzwx", "zwxy", "wxyz", "xywz", "ywzx", "zxyw", "wzxy",
		"xzyw", "ywxz", "zywx", "wxzy", "xwyz", "yzxw", "zxwy", "wyzx",
		"xzwy", "yxzw", "zwyx", "wyxz", "xwzy", "yxwz", "zyxw", "wzyx",
		"xxxx", "yyyy", "zzzz", "wwww", "xxxx", "yyyy", "zzzz", "wwww"
	};
	for (int pc = 0; pc < 64; pc++)
	{
		uint32_t const instruction = m_regs[0x200 / 4 + pc];
		uint32_t const opcode = instruction >> 27;
		if (opcode == 1)
			return;
		if (opcode == 0 || opcode == 25)
			continue;
		if (opcode == 4 || opcode == 5)
		{
			if (render || opcode == 5)
				draw_triangle();
			continue;
		}
		float a[4], b[4], result[4];
		int const src0 = BIT(instruction, 17, 4), src1 = BIT(instruction, 12, 4);
		for (int c = 0; c < 4; c++)
		{
			a[c] = BIT(instruction, 21) ? reg_float(0x400 + src0 * 16 + c * 4) : m_working[src0][c];
			b[c] = BIT(instruction, 16) ? reg_float(0x300 + src1 * 16 + c * 4) : m_working[src1][c];
		}
		float dot = 0;
		for (int c = 0; c < ((opcode == 8 || opcode == 10) ? 3 : 4); c++)
			dot += a[c] * b[c];
		for (int c = 0; c < 4; c++)
		{
			switch (opcode)
			{
			case 6: result[c] = 1.0f / std::sqrt(std::abs(a[3])); break;
			case 7: case 8: result[c] = dot; break;
			case 9: case 10: result[c] = std::max(dot, 0.0f); break;
			case 12: result[c] = a[c] + b[c]; break;
			case 13: result[c] = a[c] - b[c]; break;
			case 14: result[c] = (a[c] >= b[c]) ? 1.0f : 0.0f; break;
			case 15: result[c] = (a[c] < b[c]) ? 1.0f : 0.0f; break;
			case 16: result[c] = std::max(a[c], b[c]); break;
			case 17: result[c] = std::min(a[c], b[c]); break;
			case 18: result[c] = a[c] * b[c]; break;
			case 20: result[c] = a[c]; break;
			case 21: result[c] = b[c]; break;
			case 23: result[c] = 1.0f; break;
			case 24: result[c] = 0.0f; break;
			default: return;
			}
		}
		int const dest = BIT(instruction, 22, 4);
		if (BIT(instruction, 26) && (dest & 3) == 3)
			return;
		float *const output = BIT(instruction, 26) ? m_output[(dest >> 2) == 3 ? vertex : (dest >> 2)][dest & 3] : m_working[dest];
		int const modifier = BIT(instruction, 7, 5);
		for (int c = 0; c < 4; c++)
			if (BIT(instruction, c + 3))
				output[c] = result[(swizzle[modifier][c] - 'x') & 3] * ((modifier >= 28) ? -1.0f : 1.0f);
	}
}

void nexus3d_renderer::draw_primitives()
{
	uint32_t const command = m_regs[0x70 / 4];
	uint32_t const count = (command & 0xffff) + 1;
	uint32_t const type = BIT(command, 21, 2);
	m_regs[0x14 / 4] = 0;
	// TODO: indexed primitives, rectangle lists and the direct TSE input format.
	if (!BIT(command, 25) || BIT(command, 23) || type == 3)
		return;
	for (uint32_t i = 0; i < count; i++)
	{
		uint32_t dest = 0x400 / 4;
		for (int stream = 0; stream < 4; stream++)
		{
			uint32_t const descriptor = m_regs[0x78 / 4 + stream];
			uint32_t const format = descriptor & 7;
			if (format >= 6)
				continue;
			uint32_t const bytes = 8U << format;
			uint32_t const address = (descriptor & ~7U) + i * bytes;
			for (uint32_t j = 0; j < bytes / 4 && dest < 0x500 / 4; j++)
				m_regs[dest++] = m_space.read_dword(address + j * 4);
		}
		int const vertex = (type == 1 && i) ? 1 + ((i - 1) & 1) : i % 3;
		bool const render = type ? (i >= 2) : ((i % 3) == 2);
		run_gte(vertex, render);
		m_regs[0x14 / 4] = (i + 1) & 0xffff;
	}
}

void nexus3d_renderer::draw_triangle()
{
	vertex_t vertices[3];
	for (int i = 0; i < 3; i++)
	{
		float const w = m_output[i][0][3];
		// TODO: homogeneous clipping for primitives crossing the near plane.
		if (!(w > 0.0f))
			return;
		// Convert the rasterizer's integer pixel centres to poly_manager's half-pixel centres.
		vertices[i].x = reg_float(0xf8) + reg_float(0xfc) * m_output[i][0][0] / w + 0.5f;
		vertices[i].y = reg_float(0xf0) - reg_float(0xf4) * m_output[i][0][1] / w + 0.5f;
		if (!std::isfinite(vertices[i].x) || !std::isfinite(vertices[i].y))
			return;
		for (int c = 0; c < 4; c++)
			vertices[i].p[c] = m_output[i][1][c];
		vertices[i].p[4] = m_output[i][2][0];
		vertices[i].p[5] = m_output[i][2][1];
		vertices[i].p[6] = m_output[i][0][2] / w;
	}
	uint32_t const start = m_regs[0xb4 / 4], end = m_regs[0xb8 / 4];
	rectangle const clip(start & 0x3ff, end & 0x3ff, BIT(start, 16, 10), BIT(end, 16, 10));
	// Uses the state latch_draw() queued for this command.
	render_triangle<7>(clip, render_delegate(&nexus3d_renderer::draw_scanline, this), vertices[0], vertices[1], vertices[2]);
}

void nexus3d_renderer::draw_scanline(int32_t y, extent_t const &extent, nexus3d_poly_data const &data, int threadid)
{
	uint32_t const state = data.state;
	for (int x = extent.startx; x < extent.stopx; x++)
	{
		float p[7];
		for (int c = 0; c < 7; c++)
			p[c] = extent.param[c].start + (x - extent.startx) * extent.param[c].dpdx;
		int color[4];
		for (int c = 0; c < 4; c++)
			color[c] = std::isfinite(p[c]) ? int(std::clamp(p[c], 0.0f, 255.0f)) : 0;
		if (BIT(state, 1))
		{
			if (!std::isfinite(p[4]) || !std::isfinite(p[5]) || std::abs(p[4]) > 65535 || std::abs(p[5]) > 65535)
				continue;
			uint16_t const texel = texture_pixel(data, int(std::floor(p[4])), int(std::floor(p[5])));
			if (BIT(state, 3) && texel == uint16_t(data.key))
				continue;
			int const texture[4] = { 255, pal5bit(texel >> 11), pal6bit(texel >> 5), pal5bit(texel) };
			uint32_t const blend = data.texblend;
			for (int c = 0; c < 4; c++)
			{
				int const a = BIT(blend, 4, 2) == 1 ? color[c] : texture[c];
				int const b = BIT(blend, 6, 2) == 1 ? color[c] : texture[c];
				switch (blend & 15)
				{
				case 0: color[c] = a; break;
				case 1: color[c] = b; break;
				case 2: color[c] = (a * b) / 255; break;
				case 3: color[c] = std::min(a + b, 255); break;
				}
			}
		}
		uint32_t const address = target_address(data.target, x, y);
		if (BIT(state, 7))
		{
			uint16_t const pixel = read_pixel(address);
			int const dest[4] = { 255, pal5bit(pixel >> 11), pal6bit(pixel >> 5), pal5bit(pixel) };
			uint32_t const blend = data.blend;
			for (int c = 1; c < 4; c++)
			{
				auto factor = [&] (uint32_t function)
				{
					int value = 0;
					if (BIT(function, 1)) value |= color[c];
					if (BIT(function, 2)) value |= color[0];
					if (BIT(function, 3)) value |= dest[c];
					return BIT(function, 4) ? 255 - value : value;
				};
				color[c] = std::min(255, (color[c] * factor(blend & 31) + dest[c] * factor(BIT(blend, 5, 5))) / 255);
			}
		}
		write_pixel(address, ((color[1] >> 3) << 11) | ((color[2] >> 2) << 5) | (color[3] >> 3));
	}
}

class nexus3d_state : public driver_device
{
public:
	nexus3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_texram(*this, "texram"),
		m_nand(*this, "nand"),
		m_eeprom(*this, "eeprom"),
		m_rtc(*this, "rtc"),
		m_key(*this, "key"),
		m_qs1000(*this, "qs1000"),
		m_music_qs1000(*this, "music"),
		m_soundlatch(*this, "soundlatch"),
		m_soundrom(*this, "qs1000:cpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void nexus3d(machine_config &config);

	void init_nexus3d() ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint32_t> m_mainram;
	required_shared_ptr<uint32_t> m_texram;
	required_device<samsung_k9f2g08u0m_device> m_nand;
	required_device<i2cmem_device> m_eeprom;
	required_device<i2c_ds1307_device> m_rtc;
	required_device<nexus3d_key_device> m_key;
	required_device<qs1000_device> m_qs1000;
	required_device<qs1000_device> m_music_qs1000;
	required_device<generic_latch_8_device> m_soundlatch;
	required_region_ptr<uint8_t> m_soundrom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

//  uint32_t nexus3d_unk2_r();
//  uint32_t nexus3d_unk3_r();
//  void nexus3d_unk2_w(uint32_t data);
//  void nexus3d_unk3_w(uint32_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void nexus3d_map(address_map &map) ATTR_COLD;

	uint32_t m_intsrc = 0, m_intmode = 0, m_intpend = 0, m_intmask = 0xffffffff, m_intlevel = 0, m_intlines = 0;
	uint32_t int_source_r();
	void int_source_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t int_mode_r();
	void int_mode_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t int_pending_r();
	void int_ack_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t int_level_r();
	uint32_t int_mask_r();
	void int_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void update_interrupts();

	uint32_t vrender3d_status_r();
	uint32_t graphics_r(offs_t offset);
	void graphics_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	std::unique_ptr<nexus3d_renderer> m_renderer;
	memory_passthrough_handler m_texram_tap;
	memory_passthrough_handler m_target_tap;
	uint32_t m_graphics_regs[0x800 / 4] = {};
	float m_gte_working[16][4] = {};
	float m_gte_output[3][3][4] = {};
	uint16_t m_texture_palette[2][256] = {};
	uint32_t m_mlc_regs[0x60 / 4] = {};
	uint32_t mlc_r(offs_t offset);
	void mlc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t gpio_function_r();
	void gpio_function_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void gpio_select_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t gpio_r(offs_t offset);
	void gpio_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void update_key();
	uint32_t m_gpio_function[16] = {};
	uint16_t m_gpio_regs[16][16] = {};
	uint16_t m_gpio_select = 0;

	uint16_t system_timer_r(offs_t offset);
	void system_timer_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void update_system_timer(int channel);
	void update_timer_interrupt();
	TIMER_CALLBACK_MEMBER(system_timer_expired);
	emu_timer *m_system_timer[4] = {};
	uint16_t m_system_timer_period[4] = {};
	uint16_t m_system_timer_count[4] = {};
	uint16_t m_system_timer_control = 0;
	uint16_t m_system_timer_pending = 0;

	uint16_t i2c_r(offs_t offset);
	void i2c_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void i2c_resume();
	void i2c_sda_w(int state);
	void i2c_scl_w(int state);
	int i2c_sda_r();
	void rtc_sda_w(int state);
	int m_rtc_sda = 1;
	bool i2c_send_byte(uint8_t data);
	TIMER_CALLBACK_MEMBER(i2c_complete);
	emu_timer *m_i2c_timer = nullptr;
	uint16_t m_i2c_regs[4] = {};
	bool m_i2c_active = false;
	bool m_i2c_start = false;
	bool m_i2c_stop = false;

	uint16_t display_r(offs_t offset);
	void display_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void update_display_interrupt();
	uint16_t m_display_regs[0x50 / 2] = {};
	uint8_t sound_data_r(offs_t offset);
	void sound_port1_w(uint8_t data);
	void sound_port3_w(uint8_t data);
	uint8_t music_port3_r();
	TIMER_CALLBACK_MEMBER(music_serial_w);
	void sound_irq_w(int state);
	uint8_t m_sound_port1 = 0xff;
	bool m_music_rxd = true;
};

uint8_t nexus3d_state::sound_data_r(offs_t offset)
{
	// P1.7 selects the command latch; P1.0-4 select a 64 KiB ROM bank.
	if (!BIT(m_sound_port1, 7))
		return m_soundlatch->read();
	return m_soundrom[((m_sound_port1 & 0x1f) << 16) | (offset + 0x100)];
}

void nexus3d_state::sound_port1_w(uint8_t data)
{
	if (!BIT(data, 6) && BIT(m_sound_port1, 6))
		m_soundlatch->acknowledge_w();
	m_sound_port1 = data;
}

void nexus3d_state::sound_irq_w(int state)
{
	m_qs1000->cpu().set_input_line(MCS51_INT0_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

void nexus3d_state::sound_port3_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(nexus3d_state::music_serial_w), this), BIT(data, 1));
}

TIMER_CALLBACK_MEMBER(nexus3d_state::music_serial_w)
{
	m_music_rxd = param;
}

uint8_t nexus3d_state::music_port3_r()
{
	return 0xfe | m_music_rxd;
}

void nexus3d_state::video_start()
{
	m_renderer = std::make_unique<nexus3d_renderer>(machine(), m_maincpu->space(AS_PROGRAM), m_mainram.target(), m_texram.target(),
		m_graphics_regs, m_gte_working, m_gte_output, m_texture_palette);
	save_item(NAME(m_graphics_regs));
	save_item(NAME(m_gte_working));
	save_item(NAME(m_gte_output));
	save_item(NAME(m_texture_palette));
	save_item(NAME(m_mlc_regs));

	// CPU writes to texture memory that queued work reads, and CPU accesses
	// to render targets, wait for the rasterizer. A read tap runs after the
	// read, so it reads the word again once rendering has finished.
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_texram_tap = space.install_write_tap(
			0x60000000, 0x67ffffff, "texram_w",
			[this] (offs_t offset, uint32_t &data, uint32_t mem_mask)
			{
				if (m_renderer->texture_used(offset))
					m_renderer->sync();
			},
			&m_texram_tap);
	m_target_tap = space.install_readwrite_tap(
			nexus3d_renderer::TARGET_START, nexus3d_renderer::TARGET_END, "target",
			[this] (offs_t offset, uint32_t &data, uint32_t mem_mask)
			{
				if (m_renderer->pending())
				{
					m_renderer->sync();
					data = m_mainram[(offset & 0x07ffffff) / 4];
				}
			},
			[this] (offs_t offset, uint32_t &data, uint32_t mem_mask)
			{
				if (m_renderer->pending())
					m_renderer->sync();
			},
			&m_target_tap);
}

uint32_t nexus3d_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_renderer->sync();
	uint32_t const base = m_mlc_regs[0x24 / 4];
	uint32_t const pitch = m_mlc_regs[0x28 / 4];
	uint32_t const key = m_mlc_regs[0x50 / 4];
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			uint16_t color = m_renderer->display_pixel(x, y);
			// TODO: remaining MLC layer controls, window offsets and pixel formats.
			uint32_t const address = base + y * pitch + x * 2;
			if (BIT(m_mlc_regs[0], 0) && address >= 0x02000000 && address < 0x02400000)
			{
				uint16_t const foreground = m_mainram[address / 4] >> ((address & 2) * 8);
				if (!BIT(key, 16) || foreground != uint16_t(key))
					color = foreground;
			}
			bitmap.pix(y, x) = color;
		}
	}

	return 0;
}

uint32_t nexus3d_state::vrender3d_status_r()
{
	// Rendering isn't timed, so the engine always reports idle. Low 16 bits count fetched vertices.
	return 0x00bf0000 | m_graphics_regs[0x14 / 4];
}

uint32_t nexus3d_state::graphics_r(offs_t offset)
{
	return m_graphics_regs[offset];
}

void nexus3d_state::graphics_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0x18 / 4)
	{
		uint32_t const pending = (m_graphics_regs[offset] & 3) & ~(data & mem_mask);
		COMBINE_DATA(&m_graphics_regs[offset]);
		m_graphics_regs[offset] = (m_graphics_regs[offset] & 0x300) | pending;
	}
	else
	{
		COMBINE_DATA(&m_graphics_regs[offset]);
		if (offset == 0x20 / 4 && ACCESSING_BITS_0_7)
			m_renderer->execute(m_graphics_regs[offset]);
	}
}

uint32_t nexus3d_state::mlc_r(offs_t offset)
{
	return m_mlc_regs[offset];
}

void nexus3d_state::mlc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_mlc_regs[offset]);
}

void nexus3d_state::update_interrupts()
{
	m_intsrc |= m_intlines;
	uint32_t const active = m_intsrc & ~m_intmask;
	if (!m_intpend && (active & ~m_intmode))
	{
		// The selected request remains latched until INTPND is acknowledged.
		m_intlevel = std::countr_zero(active & ~m_intmode);
		m_intpend = 1U << m_intlevel;
	}
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, m_intpend ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, (active & m_intmode) ? ASSERT_LINE : CLEAR_LINE);
}

uint32_t nexus3d_state::int_source_r()
{
	return m_intsrc;
}

void nexus3d_state::int_source_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_intsrc &= ~(data & mem_mask);
	update_interrupts();
}

uint32_t nexus3d_state::int_mode_r()
{
	return m_intmode;
}

void nexus3d_state::int_mode_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_intmode);
	update_interrupts();
}


uint32_t nexus3d_state::int_mask_r()
{
	return m_intmask;
}

void nexus3d_state::int_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_intmask);
	update_interrupts();
}

uint32_t nexus3d_state::int_pending_r()
{
	return m_intpend;
}

void nexus3d_state::int_ack_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_intpend &= ~(data & mem_mask);
	if (!m_intpend)
		m_intlevel = 0;
	update_interrupts();
}

uint32_t nexus3d_state::int_level_r()
{
	return m_intlevel;
}

void nexus3d_state::gpio_select_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_gpio_select);
}

uint32_t nexus3d_state::gpio_function_r()
{
	return m_gpio_function[(m_gpio_select ^ 8) & 15];
}

void nexus3d_state::gpio_function_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint8_t const port = (m_gpio_select ^ 8) & 15;
	COMBINE_DATA(&m_gpio_function[port]);
	if (port == 2 || port == 15)
		update_key();
}

uint16_t nexus3d_state::gpio_r(offs_t offset)
{
	uint8_t const port = offset >> 4, reg = offset & 15;
	uint16_t result = m_gpio_regs[port][reg];
	if ((port == 2 || port == 15) && (reg == 3 || reg == 7))
	{
		uint16_t const connected = (port == 2) ? 0x1000 : 0x3fc0;
		uint16_t const pins = (port == 2) ? (m_key->ready_r() << 12) : (uint16_t(m_key->data_r()) << 6);
		for (int pin = 0; pin < 16; ++pin)
		{
			if (BIT(connected, pin) && BIT(m_gpio_function[port], pin * 2, 2) == 0)
			{
				if (reg == 3)
					result = (result & ~(1U << pin)) | (pins & (1U << pin));
				else if (BIT(m_gpio_regs[port][1 + pin / 8], (pin & 7) * 2, 2) == 0 && !BIT(pins, pin))
					result |= 1U << pin;
			}
		}
	}
	// The SDK samples inputs by selecting low-level detection and clearing the
	// pending bit.  An asserted level remains pending after that clear.
	// TODO: GPIO edge detection, other level modes, and interrupt routing.
	return result;
}

void nexus3d_state::gpio_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t &reg = m_gpio_regs[offset >> 4][offset & 15];
	if ((offset & 15) == 7)
		reg &= ~(data & mem_mask); // interrupt pending: write one to clear
	else
		COMBINE_DATA(&reg);
	if ((offset & 15) == 3 && ((offset >> 4) == 2 || (offset >> 4) == 15))
		update_key();
}

void nexus3d_state::update_key()
{
	// GPIO port 15 pins 6-13 carry D0-D7; port 2 pins 12/13 are ready/strobe.
	uint8_t drive_mask = 0;
	for (int bit = 0; bit < 8; ++bit)
	{
		if (BIT(m_gpio_function[15], (bit + 6) * 2, 2) == 1)
			drive_mask |= 1U << bit;
	}
	int const strobe = (BIT(m_gpio_function[2], 26, 2) == 1) ? BIT(m_gpio_regs[2][3], 13) : 1;
	m_key->host_w(BIT(m_gpio_regs[15][3], 6, 8), drive_mask, strobe);
}

// TODO: derive the timer input from the clock generator and confirm the divisor.
static constexpr uint32_t SYSTEM_TIMER_CLOCK = 93'750;

uint16_t nexus3d_state::system_timer_r(offs_t offset)
{
	if (offset < 4)
	{
		if (!m_system_timer[offset]->expire().is_never())
			return m_system_timer_period[offset] + 1 - m_system_timer[offset]->remaining().as_ticks(SYSTEM_TIMER_CLOCK);
		return m_system_timer_count[offset];
	}
	if (offset < 8)
		return m_system_timer_period[offset - 4];
	if (offset == 8)
		return m_system_timer_control;
	if (offset == 9)
		return m_system_timer_pending;
	return 0;
}

void nexus3d_state::system_timer_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= 4 && offset < 8)
	{
		int const channel = offset - 4;
		m_system_timer_count[channel] = system_timer_r(channel);
		COMBINE_DATA(&m_system_timer_period[channel]);
		update_system_timer(channel);
	}
	else if (offset == 8)
	{
		for (int channel = 0; channel < 4; channel++)
			m_system_timer_count[channel] = system_timer_r(channel);
		uint16_t const old_control = m_system_timer_control;
		COMBINE_DATA(&m_system_timer_control);
		if ((old_control ^ m_system_timer_control) & 0xffc0)
			for (int channel = 0; channel < 4; channel++)
				update_system_timer(channel);
		update_timer_interrupt();
	}
	else if (offset == 9)
	{
		m_system_timer_pending &= ~(data & mem_mask);
		update_timer_interrupt();
	}
}

void nexus3d_state::update_system_timer(int channel)
{
	uint8_t const mode = BIT(m_system_timer_control, 14 - channel * 2, 2);
	if (BIT(m_system_timer_control, 6) && mode)
	{
		uint32_t const ticks = uint16_t(m_system_timer_period[channel] - m_system_timer_count[channel]) + 1;
		m_system_timer[channel]->adjust(attotime::from_ticks(ticks, SYSTEM_TIMER_CLOCK), channel);
	}
	else
		m_system_timer[channel]->adjust(attotime::never);
}

void nexus3d_state::update_timer_interrupt()
{
	if (m_system_timer_control & m_system_timer_pending & 15)
		m_intlines |= 1U << 10;
	else
		m_intlines &= ~(1U << 10);
	update_interrupts();
}

TIMER_CALLBACK_MEMBER(nexus3d_state::system_timer_expired)
{
	m_system_timer_count[param] = 0;
	m_system_timer_pending |= 1U << param;
	update_timer_interrupt();
	update_system_timer(param);
}

// The four halfword registers follow the MMSP2 I2C controller layout.
uint16_t nexus3d_state::i2c_r(offs_t offset)
{
	return m_i2c_regs[offset];
}

void nexus3d_state::i2c_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t const pending = m_i2c_regs[0] & 0x10;
	COMBINE_DATA(&m_i2c_regs[offset]);
	if (offset == 0)
	{
		// Software can clear the pending bit, but cannot set it.
		m_i2c_regs[0] = (m_i2c_regs[0] & ~0x10) | (pending & m_i2c_regs[0]);
		i2c_resume();
	}
	else if (offset == 1 && BIT(m_i2c_regs[1], 4))
	{
		if (BIT(m_i2c_regs[1], 5))
			m_i2c_start = true;
		else if (m_i2c_active)
			m_i2c_stop = true;
		i2c_resume();
	}
}

void nexus3d_state::i2c_resume()
{
	if (!BIT(m_i2c_regs[0], 4) && (m_i2c_active || m_i2c_start || m_i2c_stop)
		&& m_i2c_timer->expire().is_never())
	{
		// TODO: derive PCLK from the clock controller, and implement the I2C IRQ.
		uint32_t const divisor = (BIT(m_i2c_regs[0], 6) ? 512 : 16) * ((m_i2c_regs[0] & 15) + 1);
		m_i2c_timer->adjust(attotime::from_ticks(divisor * (m_i2c_stop ? 1 : 9), m_maincpu->clock() / 2));
	}
}

void nexus3d_state::i2c_sda_w(int state)
{
	m_eeprom->write_sda(state);
	m_rtc->sda_write(state);
}

void nexus3d_state::i2c_scl_w(int state)
{
	m_eeprom->write_scl(state);
	m_rtc->scl_write(state);
}

int nexus3d_state::i2c_sda_r()
{
	return m_eeprom->read_sda() & m_rtc_sda;
}

void nexus3d_state::rtc_sda_w(int state)
{
	m_rtc_sda = state;
}

bool nexus3d_state::i2c_send_byte(uint8_t data)
{
	for (int bit = 7; bit >= 0; bit--)
	{
		i2c_sda_w(BIT(data, bit));
		i2c_scl_w(1);
		i2c_scl_w(0);
	}
	i2c_sda_w(1);
	i2c_scl_w(1);
	bool const ack = !i2c_sda_r();
	i2c_scl_w(0);
	return ack;
}

TIMER_CALLBACK_MEMBER(nexus3d_state::i2c_complete)
{
	if (m_i2c_stop)
	{
		i2c_sda_w(0);
		i2c_scl_w(1);
		i2c_sda_w(1);
		m_i2c_active = m_i2c_stop = false;
		return;
	}
	else if (m_i2c_start)
	{
		i2c_sda_w(1);
		i2c_scl_w(1);
		i2c_sda_w(0);
		i2c_scl_w(0);
		// The controller supplies the R/W bit from the master mode.
		uint8_t const address = (m_i2c_regs[3] & 0xfe) | !BIT(m_i2c_regs[1], 6);
		bool const ack = i2c_send_byte(address);
		m_i2c_regs[1] = (m_i2c_regs[1] & ~1) | !ack;
		m_i2c_start = false;
		m_i2c_active = true;
	}
	else if (BIT(m_i2c_regs[1], 6))
	{
		bool const ack = i2c_send_byte(m_i2c_regs[3]);
		m_i2c_regs[1] = (m_i2c_regs[1] & ~1) | !ack;
	}
	else
	{
		uint8_t data = 0;
		i2c_sda_w(1);
		for (int bit = 7; bit >= 0; bit--)
		{
			i2c_scl_w(1);
			data |= i2c_sda_r() << bit;
			i2c_scl_w(0);
		}
		i2c_sda_w(!BIT(m_i2c_regs[0], 7));
		i2c_scl_w(1);
		i2c_scl_w(0);
		i2c_sda_w(1);
		m_i2c_regs[3] = data;
	}
	m_i2c_regs[0] |= 0x10;
}


uint16_t nexus3d_state::display_r(offs_t offset)
{
	return m_display_regs[offset];
}

void nexus3d_state::display_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x46 / 2)
	{
		uint16_t const pending = (m_display_regs[offset] & 3) & ~(data & mem_mask);
		COMBINE_DATA(&m_display_regs[offset]);
		m_display_regs[offset] = (m_display_regs[offset] & ~3) | pending;
		update_display_interrupt();
	}
	else
		COMBINE_DATA(&m_display_regs[offset]);
}

void nexus3d_state::update_display_interrupt()
{
	uint16_t const irq = m_display_regs[0x46 / 2];
	if (irq & (irq >> 4) & 3)
		m_intlines |= 1U << 1;
	else
		m_intlines &= ~(1U << 1);
	update_interrupts();
}

void nexus3d_state::nexus3d_map(address_map &map)
{
	map(0x00000000, 0x07ffffff).ram().share("mainram");

	map(0x60000000, 0x67ffffff).ram().share("texram");

	// JAMMA player and system inputs, eight bits per bank.
	map(0x8c000000, 0x8c000003).portr("IN0");
	map(0x8c800000, 0x8c800003).portr("IN1");
	map(0x8d000000, 0x8d000003).portr("IN2");
	map(0x8d800000, 0x8d800000).w(m_soundlatch, FUNC(generic_latch_8_device::write));

	// flash
	map(0x9c000000, 0x9c000003).rw(m_nand, FUNC(nand_device::data_r), FUNC(nand_device::data_w));
	map(0x9c000010, 0x9c000013).w(m_nand, FUNC(nand_device::command_w));
	map(0x9c000018, 0x9c00001b).w(m_nand, FUNC(nand_device::address_w));

	// read on irq 9 service, unknown purpose
	map(0xc0000200, 0xc00002bf).nopr();

	// on irq, acknowledge happens to both 800 and 810 ports
	map(0xc0000800, 0xc0000803).rw(FUNC(nexus3d_state::int_source_r), FUNC(nexus3d_state::int_source_w));
	map(0xc0000804, 0xc0000807).rw(FUNC(nexus3d_state::int_mode_r), FUNC(nexus3d_state::int_mode_w));
	map(0xc0000808, 0xc000080b).rw(FUNC(nexus3d_state::int_mask_r), FUNC(nexus3d_state::int_mask_w));
	map(0xc0000810, 0xc0000813).rw(FUNC(nexus3d_state::int_pending_r), FUNC(nexus3d_state::int_ack_w));
	map(0xc0000814, 0xc0000817).r(FUNC(nexus3d_state::int_level_r));

	// lots of accesses in this range
	// 0xc00018xx seems CRTC related
	// GPIO pin function selection
	map(0xc0000910, 0xc0000913).rw(FUNC(nexus3d_state::gpio_function_r), FUNC(nexus3d_state::gpio_function_w));
	map(0xc000091e, 0xc000091f).w(FUNC(nexus3d_state::gpio_select_w));
	map(0xc0000a00, 0xc0000a13).rw(FUNC(nexus3d_state::system_timer_r), FUNC(nexus3d_state::system_timer_w));

	map(0xc0000d00, 0xc0000d07).rw(FUNC(nexus3d_state::i2c_r), FUNC(nexus3d_state::i2c_w));

	map(0xc0001800, 0xc000184f).rw(FUNC(nexus3d_state::display_r), FUNC(nexus3d_state::display_w));
	map(0xc0001c00, 0xc0001c5f).rw(FUNC(nexus3d_state::mlc_r), FUNC(nexus3d_state::mlc_w));

	map(0xc0000f00, 0xc00010ff).rw(FUNC(nexus3d_state::gpio_r), FUNC(nexus3d_state::gpio_w));

	map(0xe0000000, 0xe00007ff).rw(FUNC(nexus3d_state::graphics_r), FUNC(nexus3d_state::graphics_w));
	map(0xe0000014, 0xe0000017).r(FUNC(nexus3d_state::vrender3d_status_r));

}

static INPUT_PORTS_START( nexus3d )
	// Command 01 is used as an active-low configuration byte by both games.
	// The mapping to the physical DIP switch positions remains unverified.
	PORT_START("DSW")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPNAME( 0x20, 0x20, "ROM Check Error Delay" )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "ROM Check Error Messages" )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Check All ROM Files at Startup" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void nexus3d_state::machine_start()
{
	m_i2c_timer = timer_alloc(FUNC(nexus3d_state::i2c_complete), this);
	save_item(NAME(m_i2c_regs));
	save_item(NAME(m_rtc_sda));
	save_item(NAME(m_i2c_active));
	save_item(NAME(m_i2c_start));
	save_item(NAME(m_i2c_stop));
	for (auto &timer : m_system_timer)
		timer = timer_alloc(FUNC(nexus3d_state::system_timer_expired), this);
	save_item(NAME(m_system_timer_period));
	save_item(NAME(m_system_timer_count));
	save_item(NAME(m_system_timer_control));
	save_item(NAME(m_system_timer_pending));
	save_item(NAME(m_intsrc));
	save_item(NAME(m_intmode));
	save_item(NAME(m_intpend));
	save_item(NAME(m_intmask));
	save_item(NAME(m_intlevel));
	save_item(NAME(m_intlines));
	save_item(NAME(m_display_regs));
	save_item(NAME(m_gpio_function));
	save_item(NAME(m_gpio_regs));
	save_item(NAME(m_gpio_select));
	save_item(NAME(m_sound_port1));
	save_item(NAME(m_music_rxd));
	// U46 uses MOVC throughout bank 0, including song data above 0x7fff.
	m_qs1000->cpu().space(AS_PROGRAM).install_rom(0x0000, 0xffff, &m_soundrom[0]);
	m_qs1000->cpu().space(AS_DATA).install_read_handler(0x0100, 0xffff, read8sm_delegate(*this, FUNC(nexus3d_state::sound_data_r)));
}

void nexus3d_state::machine_reset()
{
	m_renderer->sync();
	std::fill(std::begin(m_graphics_regs), std::end(m_graphics_regs), 0);
	std::fill(std::begin(m_mlc_regs), std::end(m_mlc_regs), 0);
	for (auto &vector : m_gte_working)
		std::fill(std::begin(vector), std::end(vector), 0.0f);
	for (auto &vertex : m_gte_output)
		for (auto &vector : vertex)
			std::fill(std::begin(vector), std::end(vector), 0.0f);
	for (auto &palette : m_texture_palette)
		std::fill(std::begin(palette), std::end(palette), 0);
	std::fill(std::begin(m_i2c_regs), std::end(m_i2c_regs), 0);
	m_i2c_active = m_i2c_start = m_i2c_stop = false;
	m_i2c_timer->adjust(attotime::never);
	i2c_scl_w(1);
	i2c_sda_w(1);
	m_intsrc = m_intmode = m_intpend = m_intlevel = m_intlines = 0;
	m_intmask = 0xffffffff;
	update_interrupts();
	std::fill(std::begin(m_gpio_function), std::end(m_gpio_function), 0);
	for (auto &port : m_gpio_regs)
		std::fill(std::begin(port), std::end(port), 0);
	m_gpio_select = 0;
	m_sound_port1 = 0xff;
	std::fill(std::begin(m_display_regs), std::end(m_display_regs), 0);
	m_system_timer_control = m_system_timer_pending = 0;
	std::fill(std::begin(m_system_timer_period), std::end(m_system_timer_period), 0);
	std::fill(std::begin(m_system_timer_count), std::end(m_system_timer_count), 0);
	for (auto &timer : m_system_timer)
		timer->adjust(attotime::never);

	// NAND boot loads two data pages, omitting each page's spare area.
	uint8_t const *const boot = memregion("nand")->base();
	for (int page = 0; page < 2; page++)
		for (int word = 0; word < 512; word++)
			m_mainram[page * 512 + word] = get_u32le(&boot[page * 0x840 + word * 4]);
}

void nexus3d_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_display_regs[0x46 / 2] |= 2;
		update_display_interrupt();
		// Bound the queue when screen updates are skipped.
		m_renderer->sync();
	}
}

void nexus3d_state::nexus3d(machine_config &config)
{
	/* basic machine hardware */
	ARM920T(config, m_maincpu, 200000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &nexus3d_state::nexus3d_map);

	SCREEN(config, m_screen);
	m_screen->set_raw((XTAL(14'318'181)*2), 454*2, 0, 640, 262*2, 0, 480); // not accurate, needs CRTC understanding
	m_screen->set_screen_update(FUNC(nexus3d_state::screen_update));
	m_screen->screen_vblank().set(FUNC(nexus3d_state::screen_vblank));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_565);

	SAMSUNG_K9F2G08U0M(config, m_nand);
	I2C_24C16(config, m_eeprom); // Operator settings and bookkeeping; 11-bit EEPROM address.
	// The software uses the DS1307 register layout; verify the part on the PCB.
	I2C_DS1307(config, m_rtc);
	m_rtc->sda_callback().set(FUNC(nexus3d_state::rtc_sda_w));
	NEXUS3D_KEY(config, m_key);
	m_key->inputs_callback().set_ioport("DSW");
	// The 02 output sequence is issued by Full's coin-slot-2 handler.
	m_key->counter_callback().set([this] (int state) { machine().bookkeeping().coin_counter_w(1, state); });

	SPEAKER(config, "speaker", 2).front();
	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->set_separate_acknowledge(true);
	m_soundlatch->data_pending_callback().set(FUNC(nexus3d_state::sound_irq_w));
	QS1000(config, m_qs1000, 24_MHz_XTAL);
	m_qs1000->set_external_rom(true);
	m_qs1000->p1_out().set(FUNC(nexus3d_state::sound_port1_w));
	// U46 sequences music over TXD; U60's MIDI input connection is inferred.
	m_qs1000->p3_out().set(FUNC(nexus3d_state::sound_port3_w));
	m_qs1000->add_route(0, "speaker", 1.0, 0);
	m_qs1000->add_route(1, "speaker", 1.0, 1);

	QS1000(config, m_music_qs1000, 24_MHz_XTAL);
	// Provisional substitute for U60's undumped internal MIDI interpreter.
	m_music_qs1000->set_external_rom(true);
	m_music_qs1000->p1_in().set_constant(1); // Hidden Catch firmware: select QS1001A tables.
	m_music_qs1000->p3_in().set(FUNC(nexus3d_state::music_port3_r));
	m_music_qs1000->add_route(0, "speaker", 1.0, 0);
	m_music_qs1000->add_route(1, "speaker", 1.0, 1);
}



ROM_START( acheart )
	ROM_REGION( 0x10800898, "nand", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "arcanaheart.u1",     0x000000, 0x10800898, CRC(109bf439) SHA1(33fd39355923ef384d5eaeec8ae3f296509bde93) )

	ROM_REGION( 0x1000000, "qs1000", ROMREGION_ERASEFF )
	ROM_LOAD( "u38.bin",     0x000000, 0x200000, CRC(29ecfba3) SHA1(ab02c7a579a3c05a19b79e42342fd5ed84c7b046) )
	ROM_LOAD( "u39.bin",     0x200000, 0x200000, CRC(eef0b1ee) SHA1(5508e6b2f0ae1555662793313a05e94a87599890) )
	ROM_LOAD( "u44.bin",     0x400000, 0x200000, CRC(b9723bdf) SHA1(769090ada7375ecb3d0bc10e89fe74a8e89129f2) )
	ROM_LOAD( "u45.bin",     0x600000, 0x200000, CRC(1c6a3169) SHA1(34a2ca00a403dc1e3909ed1c55320cf2bbd9d49e) )

	ROM_REGION( 0x008000, "music:cpu", 0 )
	ROM_LOAD( "hc_u107.bin", 0x000000, 0x008000, BAD_DUMP CRC(afd5263d) SHA1(71ace1b749d8a6b84d08b97185e7e512d04e4b8d) ) // Provisional substitute from Hidden Catch; U60 internal ROM is undumped.

	ROM_REGION( 0x080000, "music", 0 )
	ROM_LOAD( "qs1001a.u43", 0x000000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )

	ROM_REGION( 0x200000, "qs1000:cpu", 0 )
	ROM_LOAD( "u46.bin",     0x000000, 0x200000, CRC(1e8a7e73) SHA1(3270bc359b266e57debf8fd4283a46e08d679ae2) )
ROM_END


ROM_START( acheartf )
	ROM_REGION( 0x10800898, "nand", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "arcanaheartfull.u1",     0x000000, 0x10800898, CRC(54b57a9d) SHA1(dee5a43b3aea854d2b98869dca74c57b66fb06eb))

	ROM_REGION( 0x1000000, "qs1000", ROMREGION_ERASEFF )
	ROM_LOAD( "u38.bin",     0x000000, 0x200000, CRC(29ecfba3) SHA1(ab02c7a579a3c05a19b79e42342fd5ed84c7b046) )
	ROM_LOAD( "u39.bin",     0x200000, 0x200000, CRC(eef0b1ee) SHA1(5508e6b2f0ae1555662793313a05e94a87599890) )
	ROM_LOAD( "u44.bin",     0x400000, 0x200000, CRC(b9723bdf) SHA1(769090ada7375ecb3d0bc10e89fe74a8e89129f2) )
	ROM_LOAD( "u45.bin",     0x600000, 0x200000, CRC(1c6a3169) SHA1(34a2ca00a403dc1e3909ed1c55320cf2bbd9d49e) )

	ROM_REGION( 0x008000, "music:cpu", 0 )
	ROM_LOAD( "hc_u107.bin", 0x000000, 0x008000, BAD_DUMP CRC(afd5263d) SHA1(71ace1b749d8a6b84d08b97185e7e512d04e4b8d) ) // Provisional substitute from Hidden Catch; U60 internal ROM is undumped.

	ROM_REGION( 0x080000, "music", 0 )
	ROM_LOAD( "qs1001a.u43", 0x000000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )

	ROM_REGION( 0x200000, "qs1000:cpu", 0 )
	ROM_LOAD( "u46.bin",     0x000000, 0x200000, CRC(1e8a7e73) SHA1(3270bc359b266e57debf8fd4283a46e08d679ae2) )
ROM_END

void nexus3d_state::init_nexus3d()
{
#if 1
	// HACK: provisional compatibility patches for Hidden Catch's MIDI firmware.
	// Programs 0-2 use custom piano tables absent from the stock QS1001A ROM.
	// The inferred replacements select its first five 88-note tables; program 2's
	// second layer already points to 0x003d40. Remove these when U60 is dumped.
	uint8_t *const rom = memregion("music:cpu")->base();
	rom[0x1680] = 0x00; // Program 0, layer 1: 0x01afa0 -> 0x003500
	rom[0x1700] = 0x35;
	rom[0x1701] = 0x00;
	rom[0x1681] = 0x00; // Program 1, layer 1: 0x01b1b0 -> 0x003710
	rom[0x1702] = 0x37;
	rom[0x1703] = 0x10;
	rom[0x1801] = 0x00; // Program 1, layer 2: 0x01ad90 -> 0x003920
	rom[0x1882] = 0x39;
	rom[0x1883] = 0x20;
	rom[0x1682] = 0x00; // Program 2, layer 1: 0x01afa0 -> 0x003b30
	rom[0x1704] = 0x3b;
	rom[0x1705] = 0x30;
#endif
}

} // anonymous namespace


GAME( 2005, acheart,  0, nexus3d, nexus3d, nexus3d_state, init_nexus3d, ROT0, "Examu", "Arcana Heart",      MACHINE_NOT_WORKING )
GAME( 2006, acheartf, 0, nexus3d, nexus3d, nexus3d_state, init_nexus3d, ROT0, "Examu", "Arcana Heart Full", MACHINE_NOT_WORKING ) // has a "for use in Japan" texture uploaded at startup right after framebuffer space
