// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen, R. Belmont, David Haywood
/*
    SI Electronics System Board Y2 (2009)
    Emulation by Peter Wilhelmsen and R. Belmont, based on a skeleton by David Haywood

    The power-on reset/first-stage boot mechanism is unidentified. For now, we copy
    decrypted U101[0x200..0x801ff] to RAM 0x00100000 and enter the recovered NAND
    loader at 0x80100000. The original loader installs its own TLB entries and
    copies ELF sections; the emulator does not load the game ELF directly.

    TODO: GE microcode/T&L, SA sorting, exact buffer packing and raster rules,
    video timing, additional movie modes, sound/I/O and actual reset/strap state.
*/

#include "emu.h"

#include "cpu/mips/mips3.h"
#include "machine/nvram.h"
#include "sound/roland_rpp324.h"
#include "video/poly.h"

#include "screen.h"
#include "speaker.h"

#include "corefloat.h"
#include "endianness.h"
#include "multibyte.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include <numbers>
#include <utility>
#include <vector>

#define LOG_DMA     (1U << 1)
#define LOG_LIST    (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_DMA | LOG_LIST)
#include "logmacro.h"

#define LOGDMA(...)     LOGMASKED(LOG_DMA, __VA_ARGS__)
#define LOGLIST(...)    LOGMASKED(LOG_LIST, __VA_ARGS__)


namespace {

constexpr auto TEXTURE_MORTON = [] {
	std::array<u16, 64> result{};
	for (unsigned n = 0; n < 64; ++n)
	{
		for (unsigned bit = 0; bit < 6; ++bit)
		{
			result[n] |= BIT(n, bit) << (bit * 2);
		}
	}
	return result;
}();

class system_board_y2_state;

// a vertex of a pre-transformed strip
struct y2_vertex
{
	float x, y, z, u, v;
	u32 color;
};

// everything a triangle's scanlines need, captured at submission so later list commands cannot change it
struct y2_polygon
{
	y2_vertex a, b, c; // vertices, wound so that the area is positive
	float area; // twice the area
	u32 control; // raster control (attribute word 1)
	u32 color; // constant color when there is no texture (attribute word 2)
	bool textured; // a texture is applied
	bool perspective; // texture coordinates are perspective corrected
	bool constant_color; // all three vertices have the same color

	u32 fb; // address of the color buffer
	u32 db; // address of the depth buffer
	unsigned stride; // width of the target in pixels
	unsigned mode; // pixel mode of the color buffer
	bool linear; // target is a bitmap rather than a set of 8x8 tiles
	bool depth_enabled; // depth is tested
	bool depth_wide; // depth is a 32-bit float rather than its upper half

	u32 texbase; // address of the texture
	u32 palette; // address of the palette
	unsigned tw, th; // size of the texture in texels
	unsigned texmode; // format of the texture
	unsigned pmode; // pixel mode of the palette
	unsigned bits; // bits per texel of a paletted texture
	bool clamp_u, clamp_v; // clamp rather than wrap
	bool power2_u, power2_v; // size is a power of two
	bool swizzled; // texels are in Morton order
	bool complete_tiles; // texture consists of whole 64x64 Morton blocks
	bool alpha_weighted; // filtering weights the color of a texel by its alpha
	const u32 *decoded_palette; // palette as ARGB8888, or nullptr to read it as it is used
	const u32 *pixels; // texture as linear ARGB8888, or nullptr to read it as it is used
};

class y2_renderer : public poly_manager<float, y2_polygon, 0>
{
public:
	y2_renderer(system_board_y2_state &state);

	bool draw_triangle(const std::array<u32, 4> &attr, y2_vertex a, y2_vertex b, y2_vertex c);
	void flush(const char *reason);

private:
	// a texture decoded once for the triangles of a batch
	struct texture_snapshot
	{
		std::array<u32, 7> key;
		std::unique_ptr<u32 []> pixels;
	};

	using palette_array = poly_array<std::array<u32, 256>, 0>;

	static constexpr unsigned PIXEL_LANES = 8; // pixels interpolated together
	static constexpr unsigned MAX_ROWS = 2048; // rows of the tallest target
	static constexpr unsigned BATCH_PRIMITIVES = 1024; // triangles before the snapshots are dropped
	static constexpr u32 SNAPSHOT_BYTES = 0x1000000; // texture snapshots kept for a batch
	static constexpr float COORD_LIMIT = 524288.0f; // 2^19, so a coordinate times the widest texture stays below 2^30

	static float edge(const y2_vertex &p, const y2_vertex &q, float x, float y);
	static unsigned coord(int i, unsigned size, bool clamp, bool power2);
	static int floor_int(float value);
	static float bound_coord(float value);

	u32 fetch(const y2_polygon &poly, unsigned x, unsigned y) const;
	template <bool Filter, bool Decoded> u32 texel(const y2_polygon &poly, float u, float v) const;

	template <bool Filter, bool Decoded> void render_span(s32 scanline, const extent_t &extent, const y2_polygon &poly, int threadid);

	system_board_y2_state &m_state;
	palette_array m_palettes; // palettes decoded for queued triangles
	std::vector<texture_snapshot> m_textures; // textures decoded for queued triangles
	u32 m_texture_bytes; // size of those textures
	unsigned m_primitives; // triangles queued since the last flush
	std::unique_ptr<extent_t []> m_extents; // rows of the triangle being submitted
};

class system_board_y2_state : public driver_device
{
	friend class y2_renderer;

public:
	system_board_y2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_sound(*this, "rdac")
		, m_ram(*this, "ram")
		, m_nand(*this, "nand_u%u", 101U)
		, m_coins(*this, "COINS")
		, m_players(*this, "P%u", 1U)
		, m_volume(*this, "VOLUME")
		, m_gfx_list(*this, "gfx_list", 2 * GFX_LIST_WORDS * sizeof(u32), ENDIANNESS_LITTLE)
		, m_vint_timer(nullptr)
		, m_sound_command_timer(nullptr)
		, m_jvs_timer{}
		, m_voc{}
		, m_rdc{}
		, m_gec{}
		, m_int_select{}
		, m_irq_sources(0)
		, m_dma{}
		, m_jvs_status{}
		, m_gfx_base{}
		, m_gfx_words{}
		, m_gfx_slot(0)
		, m_mvc{}
		, m_mvc_sram{}
		, m_mvc_command_ready{}
		, m_mvc_block_ready{}
		, m_mvc_command_pos(0)
		, m_mvc_block_pos(0)
		, m_yuv{}
		, m_timer{}
		, m_timer_epoch{}
		, m_rtc{}
		, m_coin_pending{}
		, m_coin_control(0)
	{ }

	void system_board_y2(machine_config &config) ATTR_COLD;
	void init_kof2002um() ATTR_COLD;
	void init_kof2002umj() ATTR_COLD;
	void init_higurashi() ATTR_COLD;

	INPUT_CHANGED_MEMBER(coin_changed);

protected:
	// driver_device implementation
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr unsigned GFX_LIST_WORDS = 0x40000; // longest decoded GE stream

	required_device<vr5500le_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<roland_rpp324_device> m_sound;
	required_shared_ptr<u32> m_ram;
	required_region_ptr_array<u8, 2> m_nand;
	required_ioport m_coins;
	required_ioport_array<2> m_players;
	required_ioport m_volume;
	memory_share_creator<u32> m_gfx_list; // GE/SA handoff: the decoded streams, kept apart from the CPU's double-buffered lists

	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_program;
	std::unique_ptr<y2_renderer> m_poly;
	emu_timer *m_vint_timer;
	emu_timer *m_sound_command_timer;
	emu_timer *m_jvs_timer[2];

	u32 m_voc[0x1c4 / 4];
	u32 m_rdc[0x1100 / 4];
	u32 m_gec[0x80 / 4];
	u32 m_int_select[5];
	u32 m_irq_sources;
	u32 m_dma[0x80 / 4];
	u32 m_jvs_status[2];
	u32 m_gfx_base[2];
	u32 m_gfx_words[2];
	u8 m_gfx_slot;

	u32 m_mvc[0x60 / 4];
	u32 m_mvc_sram[0x940 / 4];
	bool m_mvc_command_ready[3];
	bool m_mvc_block_ready[18];
	u8 m_mvc_command_pos;
	u8 m_mvc_block_pos;
	u32 m_yuv[0x200 / 4];

	u32 m_timer[2][4]; // timers 6 and 7: count, compare, control, status
	u64 m_timer_epoch[2];
	u16 m_rtc[0x20];
	u8 m_coin_pending[2];
	u16 m_coin_control;

	void mem_map(address_map &map) ATTR_COLD;
	void decrypt_nand(u16 seed, const std::array<u8, 16> &data_bits, const std::array<u8, 16> &key_bits) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u32 coin_r();
	void coin_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void coin_control_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	template <unsigned Chip> u32 nand_r(offs_t offset);
	u32 nand_status_r(offs_t offset);
	void nand_status_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 voc_r(offs_t offset);
	void voc_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 rdc_r(offs_t offset) { return m_rdc[offset] | ((offset == 0x10 / 4 && (m_rdc[offset] & m_rdc[0x14 / 4] & 0x0f77)) ? 0x80000000 : 0); }
	void rdc_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 gec_r(offs_t offset) { return m_gec[offset]; }
	void gec_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 irq_r(offs_t offset);
	void irq_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 dma_r(offs_t offset) { return m_dma[offset]; }
	void dma_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 jvs_r(offs_t offset) { return m_jvs_status[offset] << 16; }
	void jvs_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 mvc_r(offs_t offset) { return m_mvc[offset]; }
	void mvc_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 mvc_sram_r(offs_t offset) { return m_mvc_sram[offset]; }
	void mvc_sram_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 yuv_r(offs_t offset) { return m_yuv[offset]; }
	void yuv_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	template <unsigned N> u32 timer_r(offs_t offset);
	void timer6_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void timer7_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u16 rtc_r(offs_t offset) { return (offset == 0x1f) ? 2 : (offset == 0x1e) ? 0 : m_rtc[offset]; }
	void rtc_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	TIMER_CALLBACK_MEMBER(vint);
	TIMER_CALLBACK_MEMBER(jvs_timeout);
	TIMER_CALLBACK_MEMBER(timer7_compare);

	void update_irqs();
	void schedule_vint(unsigned event);
	u64 timer_ticks() const;
	template <unsigned N> void timer_latch();
	void geometry_start();
	void render_start();
	bool execute_list(const u32 *list, unsigned words, bool render);
	bool buffer_clear(u32 control, u32 depth, u32 color);
	void mvc_process();

	static u32 texture_offset(unsigned x, unsigned y, unsigned width, unsigned height);
	static u32 buffer_offset(unsigned x, unsigned y, unsigned width, bool linear);
	static u32 pack_pixel(u32 color, unsigned mode);
	static u32 depth_encode(float z, bool wide);
	u32 depth_read(u32 base, bool wide, u32 index) const;
	void depth_write(u32 base, bool wide, u32 index, u32 value);
	u32 pixel_read(u32 address, unsigned mode) const;
	void pixel_write(u32 address, unsigned mode, u32 color);
};

void system_board_y2_state::machine_start()
{
	m_maincpu->mips3drc_set_options(MIPS3DRC_COMPATIBLE_OPTIONS);
	// fastram for the SDRAM only: the device registers stay on the memory map
	if (!(machine().debug_flags & DEBUG_FLAG_ENABLED))
	{
		m_maincpu->add_fastram(0, m_ram.bytes() - 1, false, m_ram);
	}
	m_maincpu->space(AS_PROGRAM).specific(m_program);
	m_poly = std::make_unique<y2_renderer>(*this);

	m_vint_timer = timer_alloc(FUNC(system_board_y2_state::vint), this);
	m_sound_command_timer = timer_alloc(FUNC(system_board_y2_state::timer7_compare), this);
	for (unsigned i = 0; i < 2; ++i)
	{
		m_jvs_timer[i] = timer_alloc(FUNC(system_board_y2_state::jvs_timeout), this);
	}
	m_rtc[2] = 0x80; // lost backup: the firmware initializes the calendar itself

	save_item(NAME(m_voc));
	save_item(NAME(m_rdc));
	save_item(NAME(m_gec));
	save_item(NAME(m_int_select));
	save_item(NAME(m_irq_sources));
	save_item(NAME(m_dma));
	save_item(NAME(m_jvs_status));
	save_item(NAME(m_gfx_base));
	save_item(NAME(m_gfx_words));
	save_item(NAME(m_gfx_slot));
	save_item(NAME(m_timer));
	save_item(NAME(m_timer_epoch));
	save_item(NAME(m_rtc));
	save_item(NAME(m_mvc));
	save_item(NAME(m_mvc_sram));
	save_item(NAME(m_mvc_command_ready));
	save_item(NAME(m_mvc_block_ready));
	save_item(NAME(m_mvc_command_pos));
	save_item(NAME(m_mvc_block_pos));
	save_item(NAME(m_yuv));
	save_item(NAME(m_coin_pending));
	save_item(NAME(m_coin_control));
}

void system_board_y2_state::jvs_w(offs_t offset, u32 data, u32 mem_mask)
{
	// FPGA JVS bridge: upper halfword is W1C status, bit 0 transmits the SRAM packet. TODO: JVS devices
	m_jvs_status[offset] &= ~((data & mem_mask) >> 16);
	if (data & mem_mask & 1)
	{
		m_jvs_timer[offset]->adjust(attotime::from_msec(2), offset); // approximate bridge timeout
	}
}

INPUT_CHANGED_MEMBER(system_board_y2_state::coin_changed)
{
	// the FPGA holds coin pulses until the board library consumes them; a held contact is one coin
	const unsigned chute = param;
	if (!newval && oldval && !BIT(m_coin_control, 2 + chute * 4))
	{
		m_coin_pending[chute] = std::min<unsigned>(7, m_coin_pending[chute] + 1);
	}
}

u32 system_board_y2_state::coin_r()
{
	// per chute: active-low contact at bit 0, pulse count at bits 2..4; meters complete immediately
	return (m_coins->read() & 0x0101) | (m_coin_pending[0] << 2) | (m_coin_pending[1] << 10);
}

void system_board_y2_state::coin_w(offs_t offset, u32 data, u32 mem_mask)
{
	// _BDLib_JAMMA_InputCoin: bits 0/4 acknowledge one pulse, bits 2..3/6..7 drive the coin meters
	const u16 command = (data & mem_mask) >> 16;
	for (unsigned chute = 0; chute < 2; ++chute)
	{
		if (BIT(command, chute * 4) && m_coin_pending[chute])
		{
			--m_coin_pending[chute];
		}
		if (((command >> (2 + chute * 4)) & 3) == 3)
		{
			machine().bookkeeping().coin_counter_w(chute, 1);
			machine().bookkeeping().coin_counter_w(chute, 0);
		}
	}
}

void system_board_y2_state::coin_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	// upper byte 0xbd is the write key, bits 0/4 reset the input queues (strobes), bits 2/6 are the lockouts
	const u16 value = (data & mem_mask) >> 16;
	if ((mem_mask & 0xff000000) != 0xff000000 || (value & 0xff00) != 0xbd00)
	{
		return;
	}
	m_coin_control = (m_coin_control & ~(mem_mask >> 16)) | (value & 0x0044);
	for (unsigned chute = 0; chute < 2; ++chute)
	{
		if (BIT(value, chute * 4))
		{
			m_coin_pending[chute] = 0;
		}
		machine().bookkeeping().coin_lockout_w(chute, BIT(m_coin_control, 2 + chute * 4));
	}
}

u64 system_board_y2_state::timer_ticks() const
{
	// UTTMR_GetUnit gives 8,333,333 Hz; this is the CPU clock divided by 32.
	return machine().time().as_ticks(m_maincpu->clock() / 32);
}

// Timers 6 and 7 count up from the CPU clock divided by 32 while control bit 0 is set.
template <unsigned N>
u32 system_board_y2_state::timer_r(offs_t offset)
{
	const u32 *const regs = m_timer[N - 6];
	if (offset == 0 && BIT(regs[2], 0))
	{
		return regs[0] + u32(timer_ticks() - m_timer_epoch[N - 6]);
	}
	return regs[offset];
}

// fold the elapsed count into the counter register before a register changes
template <unsigned N>
void system_board_y2_state::timer_latch()
{
	m_timer[N - 6][0] = timer_r<N>(0);
	m_timer_epoch[N - 6] = timer_ticks();
}

void system_board_y2_state::rtc_w(offs_t offset, u16 data, u16 mem_mask)
{
	// FPGA RTC bridge: 0xbd is the write key, the low byte is the register data. TODO: clock ticking
	if ((data & mem_mask & 0xff00) == 0xbd00 && offset < 0x1e)
	{
		m_rtc[offset] = data & 0xff;
	}
}

void system_board_y2_state::timer6_w(offs_t offset, u32 data, u32 mem_mask)
{
	// TODO: compare interrupts; only free-running operation is implemented
	timer_latch<6>();
	COMBINE_DATA(&m_timer[0][offset]);
}

TIMER_CALLBACK_MEMBER(system_board_y2_state::jvs_timeout)
{
	// completed + error + timeout: how BDJVS_ReceiveBuffer sees an empty bus
	m_jvs_status[param] |= 0x23;
}

void system_board_y2_state::timer7_w(offs_t offset, u32 data, u32 mem_mask)
{
	timer_latch<7>();
	u32 *const regs = m_timer[1];
	if (offset == 3)
	{
		regs[3] &= ~(data & mem_mask); // YG_TimInit / YG_TimHandr3_7 acknowledge
	}
	else
	{
		COMBINE_DATA(&regs[offset]);
	}
	if (BIT(regs[2], 0))
	{
		const u32 ticks = regs[1] - regs[0];
		m_sound_command_timer->adjust(attotime::from_ticks(ticks ? u64(ticks) : (u64(1) << 32), m_maincpu->clock() / 32));
	}
	else
	{
		m_sound_command_timer->adjust(attotime::never);
	}
	update_irqs();
}

TIMER_CALLBACK_MEMBER(system_board_y2_state::timer7_compare)
{
	m_timer[1][3] |= 1;
	m_sound_command_timer->adjust(attotime::from_ticks(u64(1) << 32, m_maincpu->clock() / 32));
	update_irqs();
}

void system_board_y2_state::update_irqs()
{
	// YG_Add_Intr routes RDC, GEC and VOC to SCC source bits 3, 4 and 5.
	m_irq_sources = ((m_rdc[0x10 / 4] & m_rdc[0x14 / 4] & 0x0f77) ? 8 : 0)
		| ((m_gec[0x10 / 4] & m_gec[0x14 / 4] & 3) ? 16 : 0)
		| ((BIT(m_voc[0x14 / 4], 31) && (m_voc[0x10 / 4] & m_voc[0x14 / 4] & 0x307)) ? 32 : 0);
	if (m_mvc[0x10 / 4] & m_mvc[0x14 / 4] & 15)
	{
		m_irq_sources |= 1U << 27;
	}
	if (m_yuv[0x10 / 4] & m_yuv[0x14 / 4] & 3)
	{
		m_irq_sources |= 1U << 28;
	}
	// Timers 3..7 share SCC interrupt 30; timer 7 is sub-source 20.
	if (BIT(m_timer[1][3], 0))
	{
		m_irq_sources |= 1U << 30;
	}
	// init_routine installs YTFDMA_Intr on SCC source 11, shared by all channels.
	if (BIT(m_dma[0x1c / 4] | m_dma[0x2c / 4] | m_dma[0x5c / 4] | m_dma[0x6c / 4], 31))
	{
		m_irq_sources |= 1U << 11;
	}
	for (unsigned i = 0; i < 5; ++i)
	{
		m_maincpu->set_input_line(MIPS3_IRQ0 + i, (m_irq_sources & m_int_select[i]) ? ASSERT_LINE : CLEAR_LINE);
	}
}

u32 system_board_y2_state::irq_r(offs_t offset)
{
	if (offset == 0)
	{
		return m_irq_sources;
	}
	if (offset == 9)
	{
		return BIT(m_timer[1][3], 0) ? (1U << 20) : 0;
	}
	if (offset >= 4 && offset < 9)
	{
		return m_irq_sources & m_int_select[offset - 4];
	}
	if (offset >= 12 && offset < 17)
	{
		return m_int_select[offset - 12];
	}
	return 0;
}

void system_board_y2_state::irq_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset >= 12 && offset < 17)
	{
		COMBINE_DATA(&m_int_select[offset - 12]);
	}
	update_irqs();
}

void system_board_y2_state::schedule_vint(unsigned event)
{
	const unsigned line = (m_voc[0x1c / 4] >> (event * 16)) & 0x7ff;
	m_vint_timer->adjust(m_screen->time_until_pos(line % m_screen->height()), event);
}

TIMER_CALLBACK_MEMBER(system_board_y2_state::vint)
{
	if (BIT(m_voc[0], 0))
	{
		m_voc[0x10 / 4] |= 1U << param;
		update_irqs();
		schedule_vint(param ^ 1);
	}
}

u32 system_board_y2_state::voc_r(offs_t offset)
{
	if (offset == 0x0c / 4)
	{
		return (m_screen->vpos() << 16) | m_screen->hpos();
	}
	return m_voc[offset] | ((offset == 0x10 / 4 && BIT(m_voc[0x14 / 4], 31)
		&& (m_voc[offset] & m_voc[0x14 / 4] & 0x307)) ? 0x80000000 : 0);
}

void system_board_y2_state::dma_w(offs_t offset, u32 data, u32 mem_mask)
{
	const unsigned reg = offset * 4;
	if (reg == 0x1c || reg == 0x2c || reg == 0x5c || reg == 0x6c)
	{
		// STT bit 31 acknowledges completion, bit 0 starts a transfer.
		m_dma[offset] &= ~(data & mem_mask & 0x80000000);
		if (data & mem_mask & 1)
		{
			const u32 source = m_dma[offset - 3];
			const u32 dest = m_dma[offset - 2];
			const u32 size = m_dma[offset - 1];
			LOGDMA("%s: YATA DMA %02x src=%08x dst=%08x size=%08x\n", machine().describe_context(), reg, source, dest, size);
			// YTFDMA_Start: physical addresses and a byte count; only linear bus-to-RAM transfers. TODO: timing
			if (dest < m_ram.bytes() && size <= m_ram.bytes() - dest && !(size & 3) && !(dest & 3) && size <= 0x2000000)
			{
				for (u32 pos = 0; pos < size; pos += 4)
				{
					m_ram[(dest + pos) / 4] = m_program.read_dword(source + pos);
				}
				m_dma[offset] = 0x80000000;
			}
			else
			{
				logerror("YATA DMA: unsupported transfer\n");
			}
		}
	}
	else
	{
		COMBINE_DATA(&m_dma[offset]);
	}
	update_irqs();
}

void system_board_y2_state::voc_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 0x10 / 4)
	{
		m_voc[offset] &= ~(data & mem_mask); // YG_BeginFrame/EndFrame acknowledge with ones
	}
	else
	{
		COMBINE_DATA(&m_voc[offset]);
	}
	if (offset == 0 && (m_voc[0] & 0xffff0000) == 0x59470000)
	{
		m_vint_timer->adjust(attotime::never);
		if (BIT(m_voc[0], 0))
		{
			schedule_vint(0);
		}
		else
		{
			m_voc[0x10 / 4] = 0;
		}
	}
	update_irqs();
}

void system_board_y2_state::rdc_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 0x10 / 4)
	{
		m_rdc[offset] &= ~(data & mem_mask);
	}
	else
	{
		COMBINE_DATA(&m_rdc[offset]);
	}
	if (offset == 0x80 / 4 || offset == 0x20 / 4)
	{
		LOGLIST("%s: YATA RDC start %02x = %08x list=%08x fb=%08x\n", machine().describe_context(), offset * 4, data, m_rdc[0x90 / 4], m_rdc[0x9c / 4]);
	}
	if (offset == 0x80 / 4)
	{
		render_start();
	}
	update_irqs();
}

void system_board_y2_state::gec_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 0x10 / 4)
	{
		m_gec[offset] &= ~(data & mem_mask);
	}
	else
	{
		COMBINE_DATA(&m_gec[offset]);
	}
	if (offset == 0x20 / 4)
	{
		LOGLIST("%s: YATA GEC start %08x list=%08x\n", machine().describe_context(), data, m_gec[0x34 / 4]);
		geometry_start();
	}
	update_irqs();
}

void system_board_y2_state::geometry_start()
{
	const unsigned slot = m_gfx_slot;
	u32 *const list = &m_gfx_list[slot * GFX_LIST_WORDS];
	m_gfx_words[slot] = 0;
	unsigned words = 0;
	u32 glt = m_gec[0x34 / 4] & 0x0ffffff8;
	if (BIT(m_gec[0x30 / 4], 4))
	{
		return; // memory-output mode not decoded
	}
	for (unsigned entry = 0; entry < 4096 && glt < m_ram.bytes() - 7; ++entry, glt += 8)
	{
		const u32 base = m_ram[glt / 4];
		const u32 length = m_ram[glt / 4 + 1];
		if (base == 0x80000000)
		{
			if (!execute_list(list, words, false))
			{
				return;
			}
			// a resubmission replaces whatever was decoded for this RC list address
			for (unsigned other = 0; other < 2; ++other)
			{
				if (m_gfx_base[other] == m_rdc[0x30 / 4])
				{
					m_gfx_words[other] = 0;
				}
			}
			m_gfx_base[slot] = m_rdc[0x30 / 4];
			m_gfx_words[slot] = words;
			m_gfx_slot ^= 1;
			m_gec[0x18 / 4] = glt + 8;
			m_gec[0x10 / 4] |= 1; // GE complete
			m_rdc[0x10 / 4] |= 0x10; // SA complete; no overflow
			if (BIT(m_rdc[0x2c / 4], 0))
			{
				render_start();
			}
			return;
		}
		const u32 count = (length + 8) / 4;
		if ((base & 0xf0000007) || (length & 7) || length >= 0x100000 || base >= m_ram.bytes()
			|| length + 8 > m_ram.bytes() - base || count > GFX_LIST_WORDS - words)
		{
			break;
		}
		std::copy_n(&m_ram[base / 4], count, &list[words]);
		words += count;
	}
	logerror("YATA GEC: unsupported/malformed GLT at %08x\n", glt);
}

void system_board_y2_state::render_start()
{
	for (unsigned slot = 0; slot < 2; ++slot)
	{
		if (m_gfx_words[slot] && m_gfx_base[slot] == m_rdc[0x90 / 4])
		{
			if (execute_list(&m_gfx_list[slot * GFX_LIST_WORDS], m_gfx_words[slot], true))
			{
				m_gfx_words[slot] = 0;
				m_rdc[0x10 / 4] |= 1;
			}
			return;
		}
	}
	logerror("YATA RC: no decoded list at %08x\n", m_rdc[0x90 / 4]);
}

bool system_board_y2_state::execute_list(const u32 *list, unsigned words, bool render)
{
	auto finish = [this, render] (bool result)
	{
		if (render)
		{
			m_poly->flush("List end");
		}
		return result;
	};
	// Command lengths from the YG_Add*/YG_*Regset routines in the SDK.
	static constexpr u8 GE_PARAM_WORDS[] = { 2, 6, 10, 0, 4, 4, 2, 0, 4, 4, 4, 4, 8 };
	std::array<u32, 4> attr{};
	y2_vertex strip[3]{};
	unsigned vertices = 0;
	for (unsigned pos = 0; pos + 1 < words; )
	{
		const u32 *p = list + pos;
		const u32 command = p[0];
		unsigned count = 0;
		switch (command >> 28)
		{
			case 0: count = 2; break; // NOP
			case 1: count = (command == 0x10000001) ? 8 : (command == 0x10000002) ? 16 : 0; break;
			case 2: count = 4; break;
			case 6: count = ((command & 0xfffffff) < std::size(GE_PARAM_WORDS)) ? GE_PARAM_WORDS[command & 0xfffffff] : 0; break;
			case 7: count = (command & 7) ? (BIT(command, 3) ? 18 : 14) : 2; break;
			case 11: count = 4; break; // attribute
			case 12: count = attr[0] ? (4 + 2 * BIT(attr[0], 17) + 2 * BIT(attr[0], 16)) : 0; break;
			case 15: return finish(true);
		}
		if (!count || count > words - pos)
		{
			logerror("YATA graphics: unsupported command %08x %08x at stream +%x (%u words)\n", command, p[1], pos * 4, words);
			return finish(false);
		}
		if (command >> 28 == 11)
		{
			std::copy_n(p, 4, attr.begin());
			vertices = 0;
		}
		if (command >> 28 == 12)
		{
			// Only pre-transformed strips with diffuse/specular colors are decoded.
			if ((attr[0] & 0x0ffcff20) || !BIT(attr[0], 17))
			{
				logerror("YATA GE: unsupported attribute %08x\n", attr[0]);
				return finish(false);
			}
			if (BIT(command, 0))
			{
				vertices = 0;
			}
			y2_vertex v{u2f(p[1]), u2f(p[2]), u2f(p[3]), 0, 0, p[4]};
			if (BIT(attr[0], 16))
			{
				v.u = u2f(p[6]);
				v.v = u2f(p[7]);
			}
			if (!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z) || !std::isfinite(v.u) || !std::isfinite(v.v)
				|| std::abs(v.x) > 65536 || std::abs(v.y) > 65536 || std::abs(v.u) > 65536 || std::abs(v.v) > 65536)
			{
				return finish(false);
			}
			strip[vertices % 3] = v;
			if (++vertices >= 3 && render && !m_poly->draw_triangle(attr, strip[0], strip[1], strip[2]))
			{
				return finish(false);
			}
		}
		if (render)
		{
			// palette and attribute state is captured per primitive; only a target change needs a barrier
			if (command >> 28 == 2 || (command == 0x10000002 && !std::equal(p + 2, p + 6, &m_rdc[0x98 / 4])))
			{
				m_poly->flush("Target change");
			}
			if (command == 0x10000001)
			{
				std::copy_n(p + 2, 6, &m_rdc[0x48 / 4]);
			}
			else if (command == 0x10000002)
			{
				std::copy_n(p + 2, 14, &m_rdc[0x98 / 4]);
			}
			else if (command >> 28 == 2 && !buffer_clear(p[1], p[2], p[3]))
			{
				return finish(false);
			}
		}
		pos += count;
	}
	logerror("YATA graphics: missing end command\n");
	return finish(false);
}

// pixel modes shared by the VOC, RC targets, palettes and texture modes 4..7
inline u32 ATTR_FORCE_INLINE system_board_y2_state::pixel_read(u32 address, unsigned mode) const
{
	if (address >= m_ram.bytes())
	{
		return 0;
	}
	const u32 p = m_ram[address / 4] >> ((mode == 3) ? 0 : ((address & 2) * 8));
	switch (mode)
	{
		case 0: return rgb_t(BIT(p, 15) ? 255 : 0, pal5bit(p >> 10), pal5bit(p >> 5), pal5bit(p));
		case 1: return rgb_t(255, pal5bit(p >> 11), pal6bit(p >> 5), pal5bit(p));
		case 2: return rgb_t(pal4bit(p >> 12), pal4bit(p >> 8), pal4bit(p >> 4), pal4bit(p));
		default: return p;
	}
}

inline u32 ATTR_FORCE_INLINE system_board_y2_state::pack_pixel(u32 color, unsigned mode)
{
	switch (mode)
	{
		case 0: return ((color >> 16) & 0x8000) | ((color >> 9) & 0x7c00) | ((color >> 6) & 0x3e0) | ((color >> 3) & 0x1f);
		case 1: return ((color >> 8) & 0xf800) | ((color >> 5) & 0x7e0) | ((color >> 3) & 0x1f);
		case 2: return ((color >> 16) & 0xf000) | ((color >> 12) & 0xf00) | ((color >> 8) & 0xf0) | ((color >> 4) & 0xf);
		default: return color;
	}
}

inline void ATTR_FORCE_INLINE system_board_y2_state::pixel_write(u32 address, unsigned mode, u32 color)
{
	if (address >= m_ram.bytes())
	{
		return;
	}
	const unsigned shift = (mode == 3) ? 0 : ((address & 2) * 8);
	const u32 mask = (mode == 3) ? ~0U : (0xffffU << shift);
	m_ram[address / 4] = (m_ram[address / 4] & ~mask) | (pack_pixel(color, mode) << shift);
}

// 8x8 tiles, shared by render targets and scanout. TODO: the physical interleaving is unverified
u32 system_board_y2_state::buffer_offset(unsigned x, unsigned y, unsigned width, bool linear)
{
	return linear ? (y * width + x) : (((y / 8) * (width / 8) + x / 8) * 64 + (y & 7) * 8 + (x & 7));
}

// Morton order in 64x64 blocks with compactly packed edge blocks. TODO: edge packing is unverified
u32 system_board_y2_state::texture_offset(unsigned x, unsigned y, unsigned width, unsigned height)
{
	const unsigned tx = x & ~63U;
	const unsigned ty = y & ~63U;
	const unsigned w = std::min(64U, width - tx);
	const unsigned h = std::min(64U, height - ty);
	u32 index = ty * width + tx * h;
	x &= 63;
	y &= 63;
	if (w == 64 && h == 64)
	{
		index += TEXTURE_MORTON[x] | (TEXTURE_MORTON[y] << 1);
	}
	else
	{
		// Rank the point among the occupied Morton positions of this edge tile.
		unsigned ox = 0, oy = 0;
		for (int bit = 5; bit >= 0; --bit)
		{
			const unsigned size = 1U << bit;
			const unsigned q = BIT(x, bit) | (BIT(y, bit) << 1);
			for (unsigned n = 0; n < q; ++n)
			{
				const unsigned xx = ox + (n & 1) * size;
				const unsigned yy = oy + (n >> 1) * size;
				index += ((xx < w) ? std::min(size, w - xx) : 0) * ((yy < h) ? std::min(size, h - yy) : 0);
			}
			ox += (q & 1) * size;
			oy += (q >> 1) * size;
		}
	}
	return index;
}

// positive reciprocal depth: 32-bit mode stores the float, 16-bit mode its upper half. TODO: 16-bit rounding
inline u32 ATTR_FORCE_INLINE system_board_y2_state::depth_encode(float z, bool wide)
{
	const u32 value = f2u(std::max(0.0f, z));
	return wide ? value : (value >> 16);
}

inline u32 ATTR_FORCE_INLINE system_board_y2_state::depth_read(u32 base, bool wide, u32 index) const
{
	const u32 address = base + index * (wide ? 4 : 2);
	if (address >= m_ram.bytes())
	{
		return 0;
	}
	return wide ? m_ram[address / 4] : ((m_ram[address / 4] >> ((address & 2) * 8)) & 0xffff);
}

inline void ATTR_FORCE_INLINE system_board_y2_state::depth_write(u32 base, bool wide, u32 index, u32 value)
{
	const u32 address = base + index * (wide ? 4 : 2);
	if (address >= m_ram.bytes())
	{
		return;
	}
	const unsigned shift = wide ? 0 : ((address & 2) * 8);
	const u32 mask = wide ? ~0U : (0xffffU << shift);
	m_ram[address / 4] = (m_ram[address / 4] & ~mask) | (value << shift);
}

//**************************************************************************
//  RENDERER
//
//  Triangles go to the polygon manager as rows of spans found with the
//  chip's own coverage rule.  The target is guest memory, which the CPU can
//  read as soon as the write that started the list returns, so every list
//  ends with a flush; anything that changes what a queued triangle reads or
//  writes flushes first.
//**************************************************************************

y2_renderer::y2_renderer(system_board_y2_state &state)
	: poly_manager<float, y2_polygon, 0>(state.machine())
	, m_state(state)
	, m_texture_bytes(0)
	, m_primitives(0)
	, m_extents(std::make_unique<extent_t []>(MAX_ROWS))
{
	// queued triangles point into the palettes, release them with everything else
	register_poly_array(m_palettes);
}

void y2_renderer::flush(const char *reason)
{
	wait(reason);
	m_textures.clear();
	m_texture_bytes = 0;
	m_primitives = 0;
}

inline float ATTR_FORCE_INLINE y2_renderer::edge(const y2_vertex &p, const y2_vertex &q, float x, float y)
{
	return (q.x - p.x) * (y - p.y) - (q.y - p.y) * (x - p.x);
}

// wrap or clamp one texel coordinate; powers of two need no division
inline unsigned ATTR_FORCE_INLINE y2_renderer::coord(int i, unsigned size, bool clamp, bool power2)
{
	if (clamp)
	{
		return std::clamp(i, 0, int(size) - 1);
	}
	if (power2)
	{
		return unsigned(i) & (size - 1);
	}
	const int remainder = i % int(size);
	return (remainder < 0) ? (remainder + int(size)) : remainder;
}

// floor for the bounded coordinate range, where int(value) is exact
inline int ATTR_FORCE_INLINE y2_renderer::floor_int(float value)
{
	const int truncated = int(value);
	if (value >= 0)
	{
		return truncated;
	}
	return (value < float(truncated)) ? (truncated - 1) : truncated;
}

// a perspective divide over depths of mixed sign can take a coordinate far outside the vertices'
// range; keep the integer texel arithmetic defined (NaN goes to the lower bound)
inline float ATTR_FORCE_INLINE y2_renderer::bound_coord(float value)
{
	if (std::abs(value) <= COORD_LIMIT)
	{
		return value;
	}
	return (value > 0) ? COORD_LIMIT : -COORD_LIMIT;
}

// one texel at integer coordinates, as ARGB8888
inline u32 ATTR_FORCE_INLINE y2_renderer::fetch(const y2_polygon &poly, unsigned x, unsigned y) const
{
	u32 index = y * poly.tw + x;
	if (poly.swizzled)
	{
		index = poly.complete_tiles
			? (y & ~63U) * poly.tw + ((x & ~63U) << 6) + TEXTURE_MORTON[x & 63] + (TEXTURE_MORTON[y & 63] << 1)
			: system_board_y2_state::texture_offset(x, y, poly.tw, poly.th);
	}
	if (poly.texmode >= 4)
	{
		return m_state.pixel_read(poly.texbase + index * ((poly.texmode == 7) ? 4 : 2), poly.texmode - 4);
	}
	const u32 address = poly.texbase + index * poly.bits / 8;
	if (address >= m_state.m_ram.bytes())
	{
		return 0;
	}

	// RAM consists of host-endian u32 words; select the logical LE byte.
	const u8 packed = reinterpret_cast<const u8 *>(&m_state.m_ram[0])[BYTE4_XOR_LE(address)];
	const u32 entry = (poly.bits == 4) ? ((packed >> ((index & 1) * 4)) & 15) : packed;
	return poly.decoded_palette ? poly.decoded_palette[entry] : m_state.pixel_read(poly.palette + entry * ((poly.pmode == 3) ? 4 : 2), poly.pmode);
}

// Texture filtering uses normalized coordinates and texel centers.
template <bool Filter, bool Decoded>
inline u32 ATTR_FORCE_INLINE y2_renderer::texel(const y2_polygon &poly, float u, float v) const
{
	const unsigned tw = poly.tw;
	const unsigned th = poly.th;
	if constexpr (!Filter)
	{
		return fetch(poly, coord(floor_int(u * tw), tw, poly.clamp_u, poly.power2_u), coord(floor_int(v * th), th, poly.clamp_v, poly.power2_v));
	}
	else
	{
		const float uu = u * tw - 0.5f, vv = v * th - 0.5f;
		const int x = floor_int(uu), y = floor_int(vv);
		const float fx = uu - x, fy = vv - y;
		const unsigned tx0 = coord(x, tw, poly.clamp_u, poly.power2_u), tx1 = coord(x + 1, tw, poly.clamp_u, poly.power2_u);
		const unsigned ty0 = coord(y, th, poly.clamp_v, poly.power2_v), ty1 = coord(y + 1, th, poly.clamp_v, poly.power2_v);
		u32 p[4];
		if constexpr (Decoded)
		{
			p[0] = poly.pixels[ty0 * tw + tx0];
			p[1] = poly.pixels[ty0 * tw + tx1];
			p[2] = poly.pixels[ty1 * tw + tx0];
			p[3] = poly.pixels[ty1 * tw + tx1];
		}
		else
		{
			p[0] = fetch(poly, tx0, ty0);
			p[1] = fetch(poly, tx1, ty0);
			p[2] = fetch(poly, tx0, ty1);
			p[3] = fetch(poly, tx1, ty1);
		}

		// transparent texels contribute no color, so they cannot darken a cell's edge
		if (poly.alpha_weighted)
		{
			const unsigned a0 = p[0] >> 24, a1 = p[1] >> 24, a2 = p[2] >> 24, a3 = p[3] >> 24;
			if (!(a0 | a1 | a2 | a3))
			{
				return 0;
			}

			// equal alphas need no weighting, which keeps the division off the opaque path
			if (a0 != a1 || a0 != a2 || a0 != a3)
			{
				const float top_alpha = a0 * (1 - fx) + a1 * fx;
				const float bottom_alpha = a2 * (1 - fx) + a3 * fx;
				const float alpha = top_alpha * (1 - fy) + bottom_alpha * fy;
				if (alpha <= 0)
				{
					return 0;
				}
				u32 result = unsigned(std::clamp(int(alpha + 0.5f), 0, 255)) << 24;
				for (unsigned shift = 0; shift < 24; shift += 8)
				{
					const float top = ((p[0] >> shift) & 255) * a0 * (1 - fx) + ((p[1] >> shift) & 255) * a1 * fx;
					const float bottom = ((p[2] >> shift) & 255) * a2 * (1 - fx) + ((p[3] >> shift) & 255) * a3 * fx;
					const float color = (top * (1 - fy) + bottom * fy) / alpha;
					result |= unsigned(std::clamp(int(color + 0.5f), 0, 255)) << shift;
				}
				return result;
			}
		}
		u32 result = 0;
		for (unsigned shift = 0; shift < 32; shift += 8)
		{
			const float top = ((p[0] >> shift) & 255) * (1 - fx) + ((p[1] >> shift) & 255) * fx;
			const float bottom = ((p[2] >> shift) & 255) * (1 - fx) + ((p[3] >> shift) & 255) * fx;
			result |= unsigned(std::clamp(int(top * (1 - fy) + bottom * fy + 0.5f), 0, 255)) << shift;
		}
		return result;
	}
}

template <bool Filter, bool Decoded>
void y2_renderer::render_span(s32 scanline, const extent_t &extent, const y2_polygon &poly, int threadid)
{
	const y2_vertex a = poly.a;
	const y2_vertex b = poly.b;
	const y2_vertex c = poly.c;
	const float area = poly.area;
	const u32 control = poly.control;
	const int y = scanline;
	const int row_max = extent.stopx - 1;

	for (int packet_x = extent.startx; packet_x <= row_max; packet_x += PIXEL_LANES)
	{
		std::array<float, PIXEL_LANES> ea, eb, ec, weights_a, weights_b, weights_c;
		for (unsigned lane = 0; lane < PIXEL_LANES; lane++)
		{
			const float x = float(packet_x + lane) + 0.5f;
			ea[lane] = edge(b, c, x, y + 0.5f);
			eb[lane] = edge(c, a, x, y + 0.5f);
			ec[lane] = edge(a, b, x, y + 0.5f);
		}
		const unsigned covered = (1U << std::min(int(PIXEL_LANES), row_max - packet_x + 1)) - 1;
		for (unsigned lane = 0; lane < PIXEL_LANES; lane++)
		{
			weights_a[lane] = ea[lane] / area;
			weights_b[lane] = eb[lane] / area;
			weights_c[lane] = ec[lane] / area;
		}
		std::array<float, PIXEL_LANES> interpolated_z, interpolated_u, interpolated_v;
		for (unsigned lane = 0; lane < PIXEL_LANES; lane++)
		{
			interpolated_z[lane] = weights_a[lane] * a.z + weights_b[lane] * b.z + weights_c[lane] * c.z;
		}
		const bool perspective_packet = poly.textured && poly.perspective
			&& std::all_of(interpolated_z.begin(), interpolated_z.end(), [] (float z) { return z > 0; });
		if (perspective_packet)
		{
			for (unsigned lane = 0; lane < PIXEL_LANES; lane++)
			{
				interpolated_u[lane] = (weights_a[lane] * a.u * a.z + weights_b[lane] * b.u * b.z + weights_c[lane] * c.u * c.z) / interpolated_z[lane];
				interpolated_v[lane] = (weights_a[lane] * a.v * a.z + weights_b[lane] * b.v * b.z + weights_c[lane] * c.v * c.z) / interpolated_z[lane];
			}
		}
		else
		{
			for (unsigned lane = 0; lane < PIXEL_LANES; lane++)
			{
				interpolated_u[lane] = weights_a[lane] * a.u + weights_b[lane] * b.u + weights_c[lane] * c.u;
				interpolated_v[lane] = weights_a[lane] * a.v + weights_b[lane] * b.v + weights_c[lane] * c.v;
			}
		}
		for (unsigned lane = 0; lane < PIXEL_LANES; lane++)
		{
			if (!BIT(covered, lane))
			{
				continue;
			}
			const int x = packet_x + lane;
			const float wa = weights_a[lane], wb = weights_b[lane], wc = weights_c[lane];
			const u32 index = system_board_y2_state::buffer_offset(x, y, poly.stride, poly.linear);
			u32 z = 0;
			if (poly.depth_enabled)
			{
				z = system_board_y2_state::depth_encode(interpolated_z[lane], poly.depth_wide);
				const u32 oldz = m_state.depth_read(poly.db, poly.depth_wide, index);
				const unsigned result = (z < oldz) ? 1 : (z == oldz) ? 2 : 4;
				if (!(((control >> 12) & 7) & result))
				{
					continue;
				}
			}

			// without a texture, attribute word 2 is the constant color; its alpha does full-screen fades
			float u = interpolated_u[lane], v = interpolated_v[lane];
			if (poly.textured && poly.perspective && !perspective_packet)
			{
				const float rhw = interpolated_z[lane];
				if (rhw > 0)
				{
					u = (wa * a.u * a.z + wb * b.u * b.z + wc * c.u * c.z) / rhw;
					v = (wa * a.v * a.z + wb * b.v * b.z + wc * c.v * c.z) / rhw;
				}
			}
			const u32 tex = poly.textured ? texel<Filter, Decoded>(poly, bound_coord(u), bound_coord(v)) : poly.color;

			// opaque textured draws keep cutout texels (KOF's paletted font). TODO: alpha threshold, TEX_ADIS
			if (poly.textured && ((control >> 16) & 7) == 1 && !BIT(control, 19) && !(tex >> 24))
			{
				continue;
			}
			if (poly.depth_enabled && BIT(control, 15))
			{
				m_state.depth_write(poly.db, poly.depth_wide, index, z);
			}
			if (!BIT(control, 11))
			{
				continue;
			}
			u32 src = 0;

			// with weights this close to normalised, interpolating equal colors is exact
			if (poly.constant_color && std::abs(wa + wb + wc - 1.0f) < (1.0f / 1024.0f))
			{
				if (a.color == 0xffffffff)
				{
					src = tex;
				}
				else
				{
					for (unsigned shift = 0; shift < 32; shift += 8)
					{
						src |= (((a.color >> shift) & 255) * ((tex >> shift) & 255) / 255) << shift;
					}
				}
			}
			else
			{
				for (unsigned shift = 0; shift < 32; shift += 8)
				{
					const unsigned col = std::clamp(int(wa * ((a.color >> shift) & 255) + wb * ((b.color >> shift) & 255) + wc * ((c.color >> shift) & 255) + 0.5f), 0, 255);
					src |= (col * ((tex >> shift) & 255) / 255) << shift;
				}
			}
			const u32 address = poly.fb + index * ((poly.mode == 3) ? 4 : 2);

			// SRC=1,DST=0 and opaque source-over pixels do not read the destination
			if ((control & 0x3f) == 8 || ((control & 0x3f) == 0x25 && (src >> 24) == 255))
			{
				m_state.pixel_write(address, poly.mode, (((control >> 6) & 3) == 3) ? 0xff000000 : (src | 0xff000000));
				continue;
			}

			// zero source alpha leaves the destination RGB alone for these factors; only the alpha bit changes
			if (!(src >> 24) && ((control & 0x3f) == 0x25 || (control & 0x3f) == 0x21))
			{
				static constexpr u32 ALPHA_MASK[] = { 0x8000, 0, 0xf000, 0xff000000 };
				if (poly.mode != 1 && address < m_state.m_ram.bytes())
				{
					m_state.m_ram[address / 4] |= ALPHA_MASK[poly.mode] << ((poly.mode == 3) ? 0 : ((address & 2) * 8));
				}
				continue;
			}
			const u32 dst = m_state.pixel_read(address, poly.mode);

			// Blend-factor encoding follows the SDK's SRC_BLEND/DST_BLEND fields.
			auto factor = [src, dst] (unsigned which, unsigned shift) -> unsigned
			{
				switch (which)
				{
					case 0: return 0;
					case 1: return 255;
					case 2: return (dst >> shift) & 255;
					case 3: return 255 - ((dst >> shift) & 255);
					case 4: return src >> 24;
					case 5: return 255 - (src >> 24);
					case 6: return dst >> 24;
					default: return 255 - (dst >> 24);
				}
			};
			u32 color = 0xff000000;
			for (unsigned shift = 0; shift < 24; shift += 8)
			{
				const int s = ((src >> shift) & 255) * factor((control >> 3) & 7, shift);
				const int d = ((dst >> shift) & 255) * factor(control & 7, shift);

				// mode 3 subtracts (Higurashi DispSpriteGs flag 4000), mode 1 adds (flag 8000)
				const int value = (((control >> 6) & 3) == 3) ? (d - s) : (d + s);
				color |= u32(std::clamp((value + 127) / 255, 0, 255)) << shift;
			}
			m_state.pixel_write(address, poly.mode, color);
		}
	}
}

bool y2_renderer::draw_triangle(const std::array<u32, 4> &attr, y2_vertex a, y2_vertex b, y2_vertex c)
{
	// Screen-space triangles only.
	// TODO: GE microcode/T&L, SA depth sorting, fog, logic ops and exact subpixel rules
	const u32 *const rdc = m_state.m_rdc;
	const u32 control = attr[1];
	const bool textured = BIT(attr[0], 0);
	const unsigned texmode = (attr[2] >> 28) & 7;
	if ((control & 0x00f00300) || ((control >> 6) & 3) == 2
		|| (((control >> 16) & 7) != 1 && ((control >> 16) & 7) != 3)
		|| (textured && (texmode == 1 || texmode == 3)))
	{
		m_state.logerror("YATA RC: unsupported raster state %08x %08x %08x %08x\n", attr[0], attr[1], attr[2], attr[3]);
		return false;
	}
	if (!BIT(control, 11) && !BIT(control, 15))
	{
		return true;
	}
	float area = edge(a, b, c.x, c.y);
	if (area == 0)
	{
		return true;
	}
	if (area < 0)
	{
		std::swap(b, c);
		area = -area;
	}
	auto top_left = [] (const y2_vertex &p, const y2_vertex &q) { return q.y < p.y || (q.y == p.y && q.x > p.x); };
	const bool ab = top_left(a, b), bc = top_left(b, c), ca = top_left(c, a);
	const unsigned stride = (rdc[0xa0 / 4] & 0x7f8) + 8;
	const unsigned mode = (rdc[0xa0 / 4] >> 16) & 3;
	const u32 fb = rdc[0x9c / 4] & 0x0fffff00;
	const u32 db = rdc[0x98 / 4] & 0x0fffff00;
	const bool linear = BIT(rdc[0xa0 / 4], 19);
	const bool depth_wide = BIT(rdc[0xa0 / 4], 22);
	const bool depth_enabled = BIT(rdc[0xa0 / 4], 23);
	const int xmax = std::min<unsigned>((rdc[0xa4 / 4] & 0x7fe) + 1, stride - 1);
	const int ymax = (rdc[0xa4 / 4] >> 16) & 0x7ff;
	const int x0 = std::max(0, int(std::ceil(std::min({ a.x, b.x, c.x }) - 0.5f)));
	const int y0 = std::max(0, int(std::ceil(std::min({ a.y, b.y, c.y }) - 0.5f)));
	const int x1 = std::min(xmax, int(std::floor(std::max({ a.x, b.x, c.x }) - 0.5f)));
	const int y1 = std::min(ymax, int(std::floor(std::max({ a.y, b.y, c.y }) - 0.5f)));
	if (x0 > x1 || y0 > y1)
	{
		return true;
	}
	const unsigned tw = ((attr[2] & 255) + 1) * 8;
	const unsigned th = (((attr[2] >> 8) & 255) + 1) * 8;
	const u32 texbase = (attr[3] & 0x7fffff) << 5;
	const bool swizzled = BIT(attr[2], 31), bilinear = BIT(attr[3], 28);
	const unsigned pmode = (rdc[0xb0 / 4] >> 16) & 3;
	const unsigned bits = (texmode == 0) ? 4 : 8;
	const u32 palette = (rdc[((texmode == 0) ? 0xb4 : 0xbc) / 4] & 0x0ffffe00)
		+ (((attr[2] >> 16) & 255) << bits) * ((pmode == 3) ? 4 : 2);

	// feedback check against the whole clipped target, since the batch's earlier triangles write it too
	struct span { u32 start, end; };
	auto word_span = [] (u32 base, u32 bytes) { return span{base & ~3U, (base + bytes + 3) & ~3U}; };
	auto overlaps = [] (span a, span b) { return a.start < b.end && b.start < a.end; };
	const u32 target_pixels = system_board_y2_state::buffer_offset(xmax, ymax, stride, linear) + 1;
	const span color_span = word_span(fb, target_pixels * ((mode == 3) ? 4 : 2));
	const span depth_span = word_span(db, target_pixels * (depth_wide ? 4 : 2));
	auto target_hits = [overlaps, color_span, depth_span, depth_enabled] (span source) { return overlaps(source, color_span) || (depth_enabled && overlaps(source, depth_span)); };
	const unsigned texture_bits = (texmode >= 4) ? ((texmode == 7) ? 32 : 16) : bits;
	const bool independent = !(depth_enabled && overlaps(color_span, depth_span))
		&& (!textured || !target_hits(word_span(texbase, tw * th * texture_bits / 8)))
		&& (!textured || texmode >= 4 || !target_hits(word_span(palette, (1U << bits) * ((pmode == 3) ? 4 : 2))));
	if (!independent)
	{
		flush("Feedback");
	}

	// each edge predicate is monotonic in x along a row: binary search its boundary with render_span's edge expression
	int first_row = -1, last_row = -1;
	for (int y = y0; y <= y1; y++)
	{
		int row_min = x0, row_max = x1;
		auto clip_edge = [&row_min, &row_max, y] (const y2_vertex &p, const y2_vertex &q, bool top)
		{
			auto inside = [&p, &q, y, top] (int x)
			{
				const float value = edge(p, q, x + 0.5f, y + 0.5f);
				return value > 0 || (value == 0 && top);
			};
			if (q.y == p.y)
			{
				return inside(row_min);
			}
			const bool increasing = q.y < p.y;
			if (!inside(increasing ? row_max : row_min))
			{
				return false;
			}
			if (inside(increasing ? row_min : row_max))
			{
				return true;
			}
			int low = row_min, high = row_max;
			while (low + 1 < high)
			{
				const int middle = low + (high - low) / 2;
				if (inside(middle) == increasing)
				{
					high = middle;
				}
				else
				{
					low = middle;
				}
			}
			if (increasing)
			{
				row_min = high;
			}
			else
			{
				row_max = low;
			}
			return true;
		};

		extent_t &extent = m_extents[y - y0];
		extent.userdata = nullptr;
		if (clip_edge(b, c, bc) && clip_edge(c, a, ca) && clip_edge(a, b, ab))
		{
			extent.startx = row_min;
			extent.stopx = row_max + 1;
			if (first_row < 0)
			{
				first_row = y;
			}
			last_row = y;
		}
		else
		{
			extent.startx = extent.stopx = 0;
		}
	}
	if (first_row < 0)
	{
		return true;
	}

	y2_polygon setup;
	setup.a = a;
	setup.b = b;
	setup.c = c;
	setup.area = area;
	setup.control = control;
	setup.color = attr[2];
	setup.textured = textured;
	setup.perspective = BIT(attr[0], 3);
	setup.constant_color = a.color == b.color && b.color == c.color;
	setup.fb = fb;
	setup.db = db;
	setup.stride = stride;
	setup.mode = mode;
	setup.linear = linear;
	setup.depth_enabled = depth_enabled;
	setup.depth_wide = depth_wide;
	setup.texbase = texbase;
	setup.palette = palette;
	setup.tw = tw;
	setup.th = th;
	setup.texmode = texmode;
	setup.pmode = pmode;
	setup.bits = bits;
	setup.clamp_u = BIT(attr[2], 26);
	setup.clamp_v = BIT(attr[2], 27);
	setup.power2_u = !(tw & (tw - 1));
	setup.power2_v = !(th & (th - 1));
	setup.swizzled = swizzled;
	setup.complete_tiles = !(tw & 63) && !(th & 63);

	// alpha-weighted filtering keeps black fringes off Higurashi's animation cells. TODO: a hypothesis for this blend state only
	setup.alpha_weighted = ((control >> 16) & 7) == 3 && ((control >> 3) & 7) == 4;
	setup.decoded_palette = nullptr;
	setup.pixels = nullptr;

	std::array<u32, 256> immediate_palette;
	if (textured && texmode < 4 && area >= float(1U << bits))
	{
		// decode the palette once unless this triangle's own writes can reach it
		const u32 first = system_board_y2_state::buffer_offset(x0, y0, stride, linear);
		const u32 last = system_board_y2_state::buffer_offset(x1, y1, stride, linear) + 1;
		const u32 palette_end = palette + (1U << bits) * ((pmode == 3) ? 4 : 2);
		auto written = [palette, palette_end, first, last] (u32 base, unsigned bytes) { return palette < base + last * bytes && palette_end > base + first * bytes; };
		if ((!BIT(control, 11) || !written(fb, (mode == 3) ? 4 : 2))
			&& (!depth_enabled || !BIT(control, 15) || !written(db, depth_wide ? 4 : 2)))
		{
			u32 *const decoded = independent ? m_palettes.next().data() : immediate_palette.data();
			for (unsigned entry = 0; entry < (1U << bits); entry++)
			{
				decoded[entry] = m_state.pixel_read(palette + entry * ((pmode == 3) ? 4 : 2), pmode);
			}
			setup.decoded_palette = decoded;
		}
	}

	// a linear ARGB8888 texture is already in the sampler's format: read it in place
	if (textured && bilinear && texmode == 7 && !swizzled && texbase + u64(tw) * th * 4 <= m_state.m_ram.bytes())
	{
		setup.pixels = &m_state.m_ram[texbase / 4];
	}
	if (!setup.pixels && textured && bilinear && independent)
	{
		// snapshots cannot overlap this batch's output, and every barrier discards them
		const std::array<u32, 7> key{texbase, (texmode < 4) ? palette : 0, tw, th, texmode, (texmode < 4) ? pmode : 0, u32(swizzled)};
		for (const texture_snapshot &entry : m_textures)
		{
			if (entry.key == key)
			{
				setup.pixels = entry.pixels.get();
				break;
			}
		}
		const u32 texels = tw * th;
		const u64 samples = std::min<u64>(u64(area * 0.5f), u64(x1 - x0 + 1) * (y1 - y0 + 1));
		if (!setup.pixels && texels <= samples * 4 && texels <= (SNAPSHOT_BYTES - m_texture_bytes) / 4)
		{
			std::unique_ptr<u32 []> decoded = std::make_unique<u32 []>(texels);
			for (unsigned y = 0; y < th; y++)
			{
				for (unsigned x = 0; x < tw; x++)
				{
					decoded[y * tw + x] = fetch(setup, x, y);
				}
			}
			setup.pixels = decoded.get();
			m_texture_bytes += texels * 4;
			m_textures.push_back({ key, std::move(decoded) });
		}
	}

	// separate loops keep the filtering branches out of the common point sampler
	const int variant = !bilinear ? 0 : (!setup.pixels ? 1 : 2);
	if (independent)
	{
		const rectangle target(0, xmax, 0, ymax);
		object_data().next() = setup;
		const extent_t *const extents = &m_extents[first_row - y0];
		const int rows = last_row - first_row + 1;
		switch (variant)
		{
			case 0: render_extents<0>(target, render_delegate(&y2_renderer::render_span<false, false>, this), first_row, rows, extents); break;
			case 1: render_extents<0>(target, render_delegate(&y2_renderer::render_span<true, false>, this), first_row, rows, extents); break;
			case 2: render_extents<0>(target, render_delegate(&y2_renderer::render_span<true, true>, this), first_row, rows, extents); break;
		}

		// the snapshots live until the next flush, so do not let a long list keep them all
		if (++m_primitives >= BATCH_PRIMITIVES)
		{
			flush("Batch");
		}
	}
	else
	{
		// this triangle reads what it or an earlier triangle writes: draw it now, in order
		for (int y = first_row; y <= last_row; y++)
		{
			switch (variant)
			{
				case 0: render_span<false, false>(y, m_extents[y - y0], setup, 0); break;
				case 1: render_span<true, false>(y, m_extents[y - y0], setup, 0); break;
				case 2: render_span<true, true>(y, m_extents[y - y0], setup, 0); break;
			}
		}
	}
	return true;
}

bool system_board_y2_state::buffer_clear(u32 control, u32 depth, u32 color)
{
	const u32 count = control & 0x007ffff8;
	const unsigned mode = (m_rdc[0xa0 / 4] >> 16) & 3;
	const u32 base = m_rdc[0x9c / 4] & 0x0ffffff0;
	const u32 bytes = count * ((mode == 3) ? 4 : 2);
	const bool clear_depth = BIT(control, 31) && BIT(m_rdc[0xa0 / 4], 23);
	const u32 db = m_rdc[0x98 / 4] & 0x0fffff00;
	const bool depth_wide = BIT(m_rdc[0xa0 / 4], 22);
	const u32 depth_bytes = count * (depth_wide ? 4 : 2);
	if ((BIT(control, 30) && bytes > m_ram.bytes() - base)
		|| (clear_depth && depth_bytes > m_ram.bytes() - db))
	{
		return false;
	}
	if (clear_depth)
	{
		const u32 value = depth_encode(u2f(depth), depth_wide);
		for (u32 i = 0; i < count; ++i)
		{
			depth_write(db, depth_wide, i, value);
		}
	}
	if (BIT(control, 30))
	{
		u32 packed = pack_pixel(color, mode);
		if (mode != 3)
		{
			packed |= packed << 16;
		}
		std::fill_n(&m_ram[base / 4], bytes / 4, packed);
	}
	return true;
}

void system_board_y2_state::mvc_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 0x10 / 4)
	{
		m_mvc[offset] = (m_mvc[offset] & ~(data & mem_mask & 15)) | (data & mem_mask & 0x1f30);
	}
	else
	{
		COMBINE_DATA(&m_mvc[offset]);
	}
	if (offset == 0x20 / 4 || (offset == 0 && BIT(data & mem_mask, 0)))
	{
		std::fill(std::begin(m_mvc_command_ready), std::end(m_mvc_command_ready), false);
		std::fill(std::begin(m_mvc_block_ready), std::end(m_mvc_block_ready), false);
		m_mvc_command_pos = m_mvc_block_pos = 0;
		m_mvc[0x28 / 4] = offset ? 1 : 0;
		m_mvc[0x10 / 4] &= ~15U;
	}
	if (offset == 0x24 / 4)
	{
		m_mvc[0x28 / 4] = 0;
		m_mvc[0x10 / 4] |= 2;
	}
	update_irqs();
}

void system_board_y2_state::mvc_sram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mvc_sram[offset]);
	// YG_MVC_SetCommand writes +0/+8/+c, SetBlock writes 64 coefficients; three commands share an 18-block ring
	if (offset < 0x30 / 4 && (offset & 3) == 3)
	{
		m_mvc_command_ready[offset / 4] = true;
	}
	if (offset >= 0x40 / 4 && ((offset - 0x40 / 4) & 31) == 31)
	{
		m_mvc_block_ready[(offset - 0x40 / 4) / 32] = true;
	}
	mvc_process();
}

void system_board_y2_state::mvc_process()
{
	if (!BIT(m_mvc[0x28 / 4], 0))
	{
		return;
	}
	while (m_mvc_command_ready[m_mvc_command_pos])
	{
		const u32 *const cmd = &m_mvc_sram[m_mvc_command_pos * 4];
		const unsigned type = cmd[0] >> 28;
		if (type > 4 || m_mvc[0x50 / 4] != 0x211)
		{
			logerror("YATA MVC: unsupported command %08x mode %08x\n", cmd[0], m_mvc[0x50 / 4]);
			return;
		}
		if (type == 4)
		{
			m_mvc_command_ready[m_mvc_command_pos] = false;
			m_mvc[0x28 / 4] = 0;
			m_mvc[0x10 / 4] |= 1;
			update_irqs();
			return;
		}
		unsigned needed = 0;
		for (unsigned b = 0; b < 6; ++b)
		{
			if (BIT(cmd[0], b))
			{
				if (!m_mvc_block_ready[(m_mvc_block_pos + needed) % 18])
				{
					return;
				}
				++needed;
			}
		}
		const unsigned width = (m_mvc[0x30 / 4] & 0x3f0) + 16;
		const unsigned height = ((m_mvc[0x30 / 4] >> 16) & 0x3f0) + 16;
		const unsigned mx = ((cmd[0] >> 16) & 63) * 16, my = ((cmd[0] >> 8) & 63) * 16;
		const u32 out = m_mvc[0x34 / 4] & 0x0fffffe0;
		if (mx >= width || my >= height || out + width * height * 3 / 2 > m_ram.bytes())
		{
			logerror("YATA MVC: invalid output rectangle/address\n");
			return;
		}
		// Standard separable MPEG IDCT. TODO: hardware rounding and precision
		static const auto BASIS = [] {
			std::array<std::array<double, 8>, 8> result{};
			for (unsigned x = 0; x < 8; ++x)
			{
				for (unsigned u = 0; u < 8; ++u)
				{
					result[x][u] = (u ? 0.5 : std::sqrt(0.125)) * std::cos((2 * x + 1) * u * std::numbers::pi / 16);
				}
			}
			return result;
		}();
		auto byte_read = [this] (u32 address) -> int { return (address < m_ram.bytes()) ? ((m_ram[address / 4] >> ((address & 3) * 8)) & 255) : 0; };
		for (unsigned block = 0; block < 6; ++block)
		{
			int residual[64]{};
			if (BIT(cmd[0], 5 - block))
			{
				const u32 *const packed = &m_mvc_sram[0x40 / 4 + m_mvc_block_pos * 32];
				int coef[64];
				bool dc_only = true;
				for (unsigned i = 0; i < 64; ++i)
				{
					coef[i] = s16(packed[i / 2] >> ((i & 1) * 16));
					if (i && coef[i])
					{
						dc_only = false;
					}
				}
				if (dc_only)
				{
					std::fill_n(residual, 64, int(std::lround(coef[0] / 8.0)));
				}
				else
				{
					double tmp[8][8]{};
					for (unsigned v = 0; v < 8; ++v)
					{
						for (unsigned x = 0; x < 8; ++x)
						{
							for (unsigned u = 0; u < 8; ++u)
							{
								tmp[v][x] += coef[v * 8 + u] * BASIS[x][u];
							}
						}
					}
					for (unsigned y = 0; y < 8; ++y)
					{
						for (unsigned x = 0; x < 8; ++x)
						{
							double sum = 0;
							for (unsigned v = 0; v < 8; ++v)
							{
								sum += tmp[v][x] * BASIS[y][v];
							}
							residual[y * 8 + x] = std::lround(sum);
						}
					}
				}
				m_mvc_block_ready[m_mvc_block_pos] = false;
				m_mvc_block_pos = (m_mvc_block_pos + 1) % 18;
			}
			const bool chroma = block >= 4;
			const unsigned pw = chroma ? (width / 2) : width;
			const unsigned ph = chroma ? (height / 2) : height;
			const unsigned bx = chroma ? (mx / 2) : (mx + (block & 1) * 8);
			const unsigned by = chroma ? (my / 2) : (my + (block >> 1) * 8);
			const u32 plane = chroma ? (width * height + (block - 4) * width / 2) : 0;
			for (unsigned y = 0; y < 8; ++y)
			{
				for (unsigned x = 0; x < 8; ++x)
				{
					auto prediction = [&byte_read, chroma, bx, by, x, y, plane, width, pw, ph] (u32 base, u32 vector)
					{
						int vx = s16(vector), vy = s16(vector >> 16);
						if (chroma)
						{
							vx /= 2;
							vy /= 2;
						}
						const int sx = int(bx + x) + (vx >> 1), sy = int(by + y) + (vy >> 1);
						auto sample = [&byte_read, base, plane, width, pw, ph] (int xx, int yy)
						{
							return byte_read((base & 0x0fffffe0) + plane + std::clamp(yy, 0, int(ph) - 1) * width + std::clamp(xx, 0, int(pw) - 1));
						};
						const int a = sample(sx, sy), b = sample(sx + (vx & 1), sy), c = sample(sx, sy + (vy & 1)), d = sample(sx + (vx & 1), sy + (vy & 1));
						return (a + b + c + d + 2) / 4;
					};
					int value = residual[y * 8 + x];
					if (type == 1)
					{
						value += prediction(m_mvc[0x38 / 4], cmd[2]);
					}
					if (type == 2)
					{
						value += prediction(m_mvc[0x3c / 4], cmd[3]);
					}
					if (type == 3)
					{
						value += (prediction(m_mvc[0x38 / 4], cmd[2]) + prediction(m_mvc[0x3c / 4], cmd[3]) + 1) / 2;
					}
					const u32 address = out + plane + (by + y) * width + bx + x;
					const unsigned shift = (address & 3) * 8;
					m_ram[address / 4] = (m_ram[address / 4] & ~(255U << shift)) | (u32(std::clamp(value, 0, 255)) << shift);
				}
			}
		}
		m_mvc_command_ready[m_mvc_command_pos] = false;
		m_mvc_command_pos = (m_mvc_command_pos + 1) % 3;
	}
}

void system_board_y2_state::yuv_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 0x10 / 4)
	{
		m_yuv[offset] &= ~(data & mem_mask & 3);
	}
	else
	{
		COMBINE_DATA(&m_yuv[offset]);
	}
	if (offset == 0x20 / 4)
	{
		// CFT_Yge: planar 4:2:0 input with interleaved U/V rows; the format bits select the framebuffer packing
		const u32 control = m_yuv[0x30 / 4];
		const unsigned width = (m_yuv[0x38 / 4] & 0x3f0) + 16;
		const unsigned height = ((m_yuv[0x38 / 4] >> 16) & 0x3f0) + 16;
		const unsigned mode = (control >> 12) & 3;
		// ARGB8888 output (CONTROL 0xb100) is always tiled, 16-bit output (0xd100) only with bit 14 set
		// TODO: verify the other CFT pixel-order selections
		const bool tiled = (mode == 3) || BIT(control, 14);
		const u32 out = m_yuv[0x3c / 4] & 0x0fffffe0;
		if ((control & ~0x7000U) != 0x8100 || out + width * height * ((mode == 3) ? 4 : 2) > m_ram.bytes())
		{
			logerror("YATA YUV: unsupported conversion %08x size %08x out %08x\n", control, m_yuv[0x38 / 4], out);
			return;
		}
		auto sample = [this, width] (unsigned reg, unsigned x, unsigned y) -> int
		{
			const u32 address = (m_yuv[reg / 4] & 0x0fffffff) + y * width + x;
			return (address < m_ram.bytes()) ? ((m_ram[address / 4] >> ((address & 3) * 8)) & 255) : 0;
		};
		for (unsigned y = 0; y < height; ++y)
		{
			for (unsigned x = 0; x < width; ++x)
			{
				const int yy = sample(0x40, x, y) - 16, u = sample(0x44, x / 2, y / 2) - 128, v = sample(0x48, x / 2, y / 2) - 128;
				const int r = std::clamp((298 * yy + 409 * v + 128) >> 8, 0, 255);
				const int g = std::clamp((298 * yy - 100 * u - 208 * v + 128) >> 8, 0, 255);
				const int b = std::clamp((298 * yy + 516 * u + 128) >> 8, 0, 255);
				const u32 index = tiled ? texture_offset(x, y, width, height) : (y * width + x);
				pixel_write(out + index * ((mode == 3) ? 4 : 2), mode, rgb_t(255, r, g, b));
			}
		}
		m_yuv[0x10 / 4] = (m_yuv[0x10 / 4] & ~0x100U) | 1;
	}
	update_irqs();
}

void system_board_y2_state::decrypt_nand(u16 seed, const std::array<u8, 16> &data_bits, const std::array<u8, 16> &key_bits)
{
	static constexpr std::array<std::array<u8, 16>, 4> LFSR_BITS = {{
		{ 1, 6, 10, 5, 7, 9, 12, 3, 13, 8, 0, 15, 14, 2, 11, 4 },
		{ 6, 8, 15, 2, 12, 5, 3, 14, 10, 1, 9, 4, 7, 11, 0, 13 },
		{ 2, 9, 5, 0, 8, 6, 15, 10, 14, 7, 3, 12, 13, 11, 4, 1 },
		{ 12, 3, 2, 14, 15, 0, 5, 9, 7, 10, 4, 1, 8, 13, 11, 6 },
	}};
	auto permute = [] (u16 value, const std::array<u8, 16> &bits)
	{
		u16 result = 0;
		for (unsigned i = 0; i < 16; ++i)
		{
			result |= BIT(value, bits[i]) << i;
		}
		return result;
	};
	const auto data_lut = std::make_unique<u16 []>(0x10000);
	const auto key_lut = std::make_unique<u16 []>(0x10000);
	for (unsigned value = 0; value < 0x10000; ++value)
	{
		data_lut[value] = permute(value, data_bits);
		key_lut[value] = permute(value, key_bits);
	}
	std::array<std::array<u16, 0x800>, 4> masks;
	for (unsigned selector = 0; selector < 4; ++selector)
	{
		u16 state = seed;
		for (u16 &mask : masks[selector])
		{
			mask = permute(state, LFSR_BITS[selector]);
			state = (state >> 1) ^ (BIT(state, 0) ? 0xb400 : 0);
		}
	}

	// compact each pair of 0x840-byte pages in place, dropping the spare area; the source stays ahead of the destination
	for (auto &region : m_nand)
	{
		u8 *const nand = region.target();
		for (unsigned record = 0; record < 0x40000; ++record)
		{
			const u8 *const src = nand + record * 0x1080;
			u8 *const dst = nand + record * 0x1000;
			if (std::all_of(src, src + 0x1080, [] (u8 v) { return v == 0xff; }))
			{
				std::fill_n(dst, 0x1000, 0xff);
				continue;
			}
			const u16 key = get_u16le(&src[0x1070]);
			const unsigned selector = BIT(key, 5) | (BIT(key, 12) << 1);
			for (unsigned i = 0; i < 0x800; ++i)
			{
				const unsigned pos = i * 2 + ((i >= 0x400) ? 0x40 : 0);
				const u16 plain = data_lut[get_u16le(&src[pos]) ^ masks[selector][i] ^ key_lut[key]];
				put_u16le(&dst[i * 2], plain);
			}
		}
	}
}

void system_board_y2_state::init_kof2002um()
{
	decrypt_nand(0x41e5,
		{ 3, 12, 8, 11, 9, 2, 6, 5, 14, 0, 13, 7, 4, 15, 1, 10 },
		{ 3, 5, 4, 10, 9, 0, 15, 6, 13, 2, 14, 1, 7, 12, 8, 11 });
}

void system_board_y2_state::init_kof2002umj()
{
	decrypt_nand(0xe9d8,
		{ 3, 12, 8, 11, 9, 2, 6, 5, 14, 0, 13, 7, 4, 15, 1, 10 },
		{ 3, 5, 4, 10, 9, 0, 15, 6, 13, 2, 14, 1, 7, 12, 8, 11 });
}

void system_board_y2_state::init_higurashi()
{
	decrypt_nand(0x1650,
		{ 2, 14, 5, 9, 8, 1, 11, 6, 13, 7, 0, 10, 3, 4, 12, 15 },
		{ 9, 14, 0, 13, 15, 3, 5, 8, 6, 11, 10, 7, 1, 4, 12, 2 });
}

void system_board_y2_state::machine_reset()
{
	std::fill(std::begin(m_voc), std::end(m_voc), 0);
	std::fill(std::begin(m_rdc), std::end(m_rdc), 0);
	std::fill(std::begin(m_gec), std::end(m_gec), 0);
	std::fill(std::begin(m_int_select), std::end(m_int_select), 0);
	std::fill(std::begin(m_dma), std::end(m_dma), 0);
	std::fill(std::begin(m_jvs_status), std::end(m_jvs_status), 0);

	std::fill(std::begin(m_gfx_words), std::end(m_gfx_words), 0);
	m_gfx_slot = 0;

	for (auto &timer : m_timer)
	{
		std::fill(std::begin(timer), std::end(timer), 0);
	}
	std::fill(std::begin(m_timer_epoch), std::end(m_timer_epoch), 0);
	m_sound_command_timer->adjust(attotime::never);

	std::fill(std::begin(m_mvc), std::end(m_mvc), 0);
	std::fill(std::begin(m_yuv), std::end(m_yuv), 0);
	std::fill(std::begin(m_mvc_command_ready), std::end(m_mvc_command_ready), false);
	std::fill(std::begin(m_mvc_block_ready), std::end(m_mvc_block_ready), false);
	m_mvc_command_pos = m_mvc_block_pos = 0;

	std::fill(std::begin(m_coin_pending), std::end(m_coin_pending), 0);
	m_coin_control = 0;
	for (unsigned chute = 0; chute < 2; ++chute)
	{
		machine().bookkeeping().coin_lockout_w(chute, 0);
	}

	for (auto &timer : m_jvs_timer)
	{
		timer->adjust(attotime::never);
	}
	m_vint_timer->adjust(attotime::never);
	update_irqs();

	const u8 *const nand = m_nand[0].target();
	if (std::memcmp(nand, "YATA_BOOT_HEADER", 16))
	{
		// The BIOS-root set only contains the FPGA bitstream, not a reset program.
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		return;
	}
	for (unsigned i = 0; i < 0x80000; i += 4)
	{
		m_ram[(0x100000 + i) / 4] = get_u32le(&nand[0x200 + i]);
	}
	m_maincpu->set_state_int(MIPS3_PC, 0x80100000);
}

template <unsigned Chip>
u32 system_board_y2_state::nand_r(offs_t offset)
{
	return get_u32le(&m_nand[Chip][offset * 4]);
}

u32 system_board_y2_state::nand_status_r(offs_t offset)
{
	// BDFNAND probes both NAND08GW3B2CN6 IDs at +10/+18; two populated sockets select the 0x70000000 window
	switch (offset * 4)
	{
		case 0x10: case 0x18: return 0x9551d320;
		case 0x14: case 0x1c: return 0x00000058;
		case 0x30: return 0; // no uncorrectable/bad-block event; ECC not modelled yet
		default:
			if (!machine().side_effects_disabled())
			{
				logerror("%s: unknown NAND status read %02x\n", machine().describe_context(), offset * 4);
			}
			return 0;
	}
}

void system_board_y2_state::nand_status_w(offs_t offset, u32 data, u32 mem_mask)
{
	// Halfword write at +32 acknowledges the two NAND error flags (0101).
	if (offset != 0x30 / 4 || (data & mem_mask & ~0x01010000U))
	{
		logerror("%s: unknown NAND status write %02x = %08x & %08x\n", machine().describe_context(), offset * 4, data, mem_mask);
	}
}

void system_board_y2_state::mem_map(address_map &map)
{
	map(0x00000000, 0x0fffffff).ram().share(m_ram); // 256 MiB, also recorded in the boot header
	map(0x17000000, 0x170010ff).rw(FUNC(system_board_y2_state::rdc_r), FUNC(system_board_y2_state::rdc_w));
	map(0x17002000, 0x1700207f).rw(FUNC(system_board_y2_state::gec_r), FUNC(system_board_y2_state::gec_w));
	map(0x17003000, 0x170031c3).rw(FUNC(system_board_y2_state::voc_r), FUNC(system_board_y2_state::voc_w));
	map(0x17006000, 0x1700605f).rw(FUNC(system_board_y2_state::mvc_r), FUNC(system_board_y2_state::mvc_w));
	map(0x17007000, 0x170071ff).rw(FUNC(system_board_y2_state::yuv_r), FUNC(system_board_y2_state::yuv_w));
	map(0x17008000, 0x1700893f).rw(FUNC(system_board_y2_state::mvc_sram_r), FUNC(system_board_y2_state::mvc_sram_w));
	map(0x18000000, 0x18000043).rw(FUNC(system_board_y2_state::irq_r), FUNC(system_board_y2_state::irq_w));
	// SYS_MODE 4 selects the full ELF header at +0x100 and the direct NAND path; the reset straps are unmeasured
	map(0x18000050, 0x18000053).lr32(NAME([] () -> u32 { return 4; }));
	map(0x18000058, 0x1800005b).ram(); // SCC SYS_DATA, firmware stores NMI hook here
	map(0x18001000, 0x18001003).ram(); // IOC PORT_WDATA (pins not modelled)
	map(0x18001010, 0x1800101b).ram(); // IOC PORT_INOUT, PORT_INVERT, PORT_CHANGE
	map(0x18002070, 0x1800207f).rw(FUNC(system_board_y2_state::timer_r<6>), FUNC(system_board_y2_state::timer6_w));
	map(0x18002080, 0x1800208f).rw(FUNC(system_board_y2_state::timer_r<7>), FUNC(system_board_y2_state::timer7_w));
	map(0x18003000, 0x1800307f).rw(FUNC(system_board_y2_state::dma_r), FUNC(system_board_y2_state::dma_w));
	map(0x80000000, 0xbfffffff).r(FUNC(system_board_y2_state::nand_r<0>));
	map(0xc0000000, 0xefffffff).r(FUNC(system_board_y2_state::nand_r<1>));
	map(0xf0000010, 0xf0000013).rw(m_sound, FUNC(roland_rpp324_device::host_r), FUNC(roland_rpp324_device::host_w));
	map(0xf0000020, 0xf0000021).rw(m_sound, FUNC(roland_rpp324_device::control_r), FUNC(roland_rpp324_device::control_w));
	map(0xf0000024, 0xf0000025).r(m_sound, FUNC(roland_rpp324_device::status_r));
	// BDLib backup memory: three 0x500-byte records, then 0x18000 bytes of user data at +0x1000. TODO: SRAM size
	map(0xf0200000, 0xf0218fff).ram().share("backup");
	map(0xf0400010, 0xf0400013).lr32(NAME([this] () -> u32 { return m_players[0]->read() | (m_players[1]->read() << 16); }));
	map(0xf0400030, 0xf0400033).rw(FUNC(system_board_y2_state::coin_r), FUNC(system_board_y2_state::coin_w));
	map(0xf0400034, 0xf0400037).lr32(NAME([this] () -> u32 { return u32(m_coin_control) << 16; })).w(FUNC(system_board_y2_state::coin_control_w));
	map(0xf0500000, 0xf050003f).rw(FUNC(system_board_y2_state::rtc_r), FUNC(system_board_y2_state::rtc_w));
	// volume potentiometer ADC: BDLib sends it to BDSND_SetMaster2Level, and zero mutes everything
	map(0xf0540000, 0xf0540000).lr8(NAME([this] () -> u8 { return u8(m_volume->read() * 255 / 100); }));
	map(0xf05c0010, 0xf05c0013).portr("DSW");
	map(0xf0600018, 0xf060001f).rw(FUNC(system_board_y2_state::jvs_r), FUNC(system_board_y2_state::jvs_w));
	map(0xf0600020, 0xf0600027).ram(); // FPGA JVS timing/control configuration
	map(0xf0601000, 0xf0604fff).ram(); // two pairs of TX/RX packet SRAM windows
	map(0xf0800000, 0xf0800033).rw(FUNC(system_board_y2_state::nand_status_r), FUNC(system_board_y2_state::nand_status_w));
}

u32 system_board_y2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_voc[0x48 / 4] | 0xff000000, cliprect);
	// primary output, packing from YG_Flip/pixtype2regmode. TODO: tiled layout, second plane, gamma
	const u32 base = m_voc[0x70 / 4];
	const u32 size = m_voc[0x74 / 4];
	const unsigned width = (size & 0x7f8) + 8;
	const unsigned height = ((size >> 16) & 0x7ff) + 1;
	const unsigned mode = (base >> 28) & 7;
	if (!BIT(m_voc[0], 0) || !BIT(m_voc[0x50 / 4], 0) || mode > 3)
	{
		return 0;
	}
	for (int y = cliprect.top(); y <= cliprect.bottom() && y < height; ++y)
	{
		u32 *const dst = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right() && x < width; ++x)
		{
			const u32 address = (base & 0x0ffffff0) + buffer_offset(x, y, width, BIT(base, 31)) * ((mode == 3) ? 4 : 2);
			if (address >= m_ram.bytes())
			{
				continue;
			}
			const u32 pixel = m_ram[address / 4] >> ((mode == 3) ? 0 : ((address & 2) * 8));
			switch (mode)
			{
				case 0: dst[x] = rgb_t(pal5bit(pixel >> 10), pal5bit(pixel >> 5), pal5bit(pixel)); break;
				case 1: dst[x] = rgb_t(pal5bit(pixel >> 11), pal6bit(pixel >> 5), pal5bit(pixel)); break;
				case 2: dst[x] = rgb_t(pal4bit(pixel >> 8), pal4bit(pixel >> 4), pal4bit(pixel)); break;
				case 3: dst[x] = 0xff000000 | pixel; break;
			}
		}
	}
	return 0;
}

static INPUT_PORTS_START(system_board_y2)
	PORT_START("VOLUME")
	PORT_ADJUSTER(100, "Board Volume")

	// BDLib uses the JVS-style switch bit assignments for the JAMMA latches.
	PORT_START("P1")
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(1)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_PLAYER(1)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(1)
	PORT_BIT(0x000f, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P2")
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_SERVICE2)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(2)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_PLAYER(2)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(2)
	PORT_BIT(0x000e, IP_ACTIVE_LOW, IPT_UNUSED)
	// P2 bit 0 becomes the common TEST bit (7) in BDLib's system byte.
	PORT_SERVICE_NO_TOGGLE(0x0001, IP_ACTIVE_LOW)

	PORT_START("COINS")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(system_board_y2_state::coin_changed), 0)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_COIN2) PORT_IMPULSE(2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(system_board_y2_state::coin_changed), 1)
	PORT_BIT(0xfefe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DSW")
	PORT_SERVICE_DIPLOC(0x0001, IP_ACTIVE_LOW, "SW:1")
	PORT_DIPNAME(0x0002, 0x0002, "Cabinet Interface") PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(0x0002, "JAMMA")
	PORT_DIPSETTING(0x0000, "JVS")
	PORT_DIPNAME(0x0004, 0x0004, "VGA Sync") PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(0x0004, "Separate")
	PORT_DIPSETTING(0x0000, "Composite")
	PORT_DIPNAME(0x0008, 0x0008, "Monitor") PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(0x0008, "31 kHz")
	PORT_DIPSETTING(0x0000, "15 kHz")
	PORT_DIPNAME(0x0010, 0x0010, "JAMMA Harness") PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(0x0010, "Standard")
	PORT_DIPSETTING(0x0000, "Mahjong")
	PORT_DIPUNKNOWN_DIPLOC(0x0020, 0x0020, "SW:6")
	PORT_BIT(0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void system_board_y2_state::system_board_y2(machine_config &config)
{
	VR5500LE(config, m_maincpu, 266'666'666); // boot header +0x84
	m_maincpu->set_icache_size(32 * 1024);
	m_maincpu->set_dcache_size(32 * 1024);
	m_maincpu->set_addrmap(AS_PROGRAM, &system_board_y2_state::mem_map);

	NVRAM(config, "backup", nvram_device::DEFAULT_ALL_0);

	// YG_Reset programs HVLOAD0=020c0319, the totals minus one.
	// FIXME: the dot clock and blanking positions are unverified; this clock gives 60 Hz
	SCREEN(config, m_screen);
	m_screen->set_raw(25'011'000, 794, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(system_board_y2_state::screen_update));
	SPEAKER(config, "speaker", 2).front();
	ROLAND_RPP324(config, m_sound, 0);
	m_sound->set_rom_tag("soundrom");
	m_sound->add_route(0, "speaker", 1.0, 0);
	m_sound->add_route(1, "speaker", 1.0, 1);
}

#define SYSTEM_Y2_FPGA_BITSTREAM \
	ROM_REGION( 0x200000, "fgpa_spi", 0) /* located on motherboard, contains a bitstream for the Xilinx Spartan 3A XC3S700A FPGA */ \
	ROM_LOAD( "system_y2_m25p40.u801", 0x00000, 0x80000, CRC(2b19f490) SHA1(b6a0482926d02fb5bb9a2c05ffdba6a2347fe658) )

ROM_START( systemy2 )
	SYSTEM_Y2_FPGA_BITSTREAM
	ROM_REGION( 0x8000000, "soundrom", ROMREGION_ERASE00 )
	ROM_REGION( 0x42000000, "nand_u101", ROMREGION_ERASE00 )
	ROM_REGION( 0x42000000, "nand_u102", ROMREGION_ERASE00 )
ROM_END

ROM_START( kof2002um ) // The King of Fighters 复仇之路/Fùchóu zhī lù/Road to Revenge
	SYSTEM_Y2_FPGA_BITSTREAM

	ROM_REGION( 0x8000000, "soundrom", 0 ) // Roland RPP-324 phrase directory and audio
	ROM_LOAD( "s29gl01gp11fcr2.u103", 0x0000000, 0x8000000, CRC(722cbad1) SHA1(0292be12255ee4bd586166a3f5cd108c5453295b) )

	ROM_REGION( 0x42000000, "nand_u101", 0 ) // presumably accessed like a filesystem (encrypted)
	ROM_LOAD( "nand08gw3b2cn6.u101", 0x00000000, 0x42000000, CRC(ddeebb49) SHA1(6907205a0e0b69e2b37528f71647c70b4dd9e0f2) )
	ROM_REGION( 0x42000000, "nand_u102", 0 )
	ROM_LOAD( "nand08gw3b2cn6.u102", 0x00000000, 0x42000000, CRC(ac2dc586) SHA1(5168b4c0c6343b6c040a206da04fa7cdbc3b35b9) )
ROM_END

ROM_START( kof2002umj )
	SYSTEM_Y2_FPGA_BITSTREAM

	ROM_REGION( 0x8000000, "soundrom", 0 ) // Roland RPP-324 phrase directory and audio
	ROM_LOAD( "s29gl01gp11fcr2.u103", 0x0000000, 0x8000000, CRC(916c9d68) SHA1(65c09f75b6a71b0d79a827c6829d1c05d8699a32) )

	ROM_REGION( 0x42000000, "nand_u101", 0 ) // presumably accessed like a filesystem (encrypted)
	ROM_LOAD( "nand08gw3b2cn6.u101", 0x00000000, 0x42000000, CRC(46e03f1d) SHA1(62d8eeb7513e851bf11a26a84b5d310270f3fcf6) )
	ROM_REGION( 0x42000000, "nand_u102", 0 )
	ROM_LOAD( "nand08gw3b2cn6.u102", 0x00000000, 0x42000000, CRC(db931dca) SHA1(1b1fc88732944e9ede09e584c7b07e28a59df3e2) )
ROM_END

ROM_START( higurashi ) //ひぐらしの哭く頃に 雀 / Higurashi no Naku Koro ni Jong - AM-Y2 ROM_V0 PCB (also has a XILINX XC2C64A etched 0012)
	SYSTEM_Y2_FPGA_BITSTREAM

	ROM_REGION( 0x10000000, "soundrom", 0 ) // Roland RPP-324 phrase directory and audio
	ROM_LOAD( "s29gl01gp11tfcr2.u103", 0x0000000, 0x8000000, CRC(1a20bb8b) SHA1(49beb0c9d3549355b55192ba4a24e3f06b750038) )
	ROM_LOAD( "s29gl01gp11tfcr2.u104", 0x8000000, 0x8000000, CRC(ea7a4ba4) SHA1(bd801eb4b21ee1bade906a4faa172cb7eb0fd5ac) )

	ROM_REGION( 0x42000000, "nand_u101", 0 )
	ROM_LOAD( "nand08gw3b2cn6.u101", 0x00000000, 0x42000000, CRC(a1968d7d) SHA1(53621cf9ebe6b428d56630705b16e00c14dc5be9) )
	ROM_REGION( 0x42000000, "nand_u102", 0 )
	ROM_LOAD( "nand08gw3b2cn6.u102", 0x00000000, 0x42000000, CRC(6ba638bb) SHA1(3f92ceb5b486787e51ea7eaaa0cf49179034210e) )
ROM_END

} // anonymous namespace


GAME( 2009, systemy2,   0,       system_board_y2, system_board_y2,  system_board_y2_state, empty_init, ROT0, "SI Electronics",           "System Board Y2", MACHINE_IS_BIOS_ROOT )

// The title screen announcer says "The King of Fighters 2002 Unlimited Match" for both versions, but
// the Chinese set only shows the Chinese title regardless of language setting in service mode
GAME( 2009, kof2002um,  systemy2,  system_board_y2, system_board_y2,  system_board_y2_state, init_kof2002um,  ROT0, "SNK Playmore / New Channel", "The King of Fighters - Fuchou Zhi Lu (Chinese/English)", 0 )
GAME( 2009, kof2002umj, kof2002um, system_board_y2, system_board_y2,  system_board_y2_state, init_kof2002umj, ROT0, "SNK Playmore",               "The King of Fighters 2002 - Unlimited Match (Japanese/English)", 0 )
GAME( 2009, higurashi,  systemy2,  system_board_y2, system_board_y2,  system_board_y2_state, init_higurashi,  ROT0, "AQ Interactive",             "Higurashi no Naku Koro ni Jong (Japan)", MACHINE_IMPERFECT_CONTROLS )

//エヌアイン完全世界/En-Eins Perfektewelt
//上海 臥龍天昇/Shanghai Ga ryū tenshō
