// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    MOS 6566 Video Interface Chip II (VIC-II) emulation

***************************************************************************/

#include "emu.h"
#include "mos6566.h"

#include "screen.h"

#include <algorithm>
#include <bit>
#include <iterator>


namespace {

constexpr rgb_t PALETTE_PAL[] =
{
	rgb_t(0x00, 0x00, 0x00),
	rgb_t(0xff, 0xff, 0xff),
	rgb_t(0x96, 0x28, 0x2e),
	rgb_t(0x5b, 0xd6, 0xce),
	rgb_t(0x9f, 0x2d, 0xad),
	rgb_t(0x41, 0xb9, 0x36),
	rgb_t(0x27, 0x24, 0xc4),
	rgb_t(0xef, 0xf3, 0x47),
	rgb_t(0x9f, 0x48, 0x15),
	rgb_t(0x5e, 0x35, 0x00),
	rgb_t(0xda, 0x5f, 0x66),
	rgb_t(0x47, 0x47, 0x47),
	rgb_t(0x78, 0x78, 0x78),
	rgb_t(0x91, 0xff, 0x84),
	rgb_t(0x68, 0x64, 0xff),
	rgb_t(0xae, 0xae, 0xae)
};

constexpr rgb_t PALETTE_NTSC[] =
{
	rgb_t(0x00, 0x00, 0x00),
	rgb_t(0xff, 0xff, 0xff),
	rgb_t(0x7c, 0x35, 0x2b),
	rgb_t(0x5a, 0xa6, 0xb1),
	rgb_t(0x69, 0x41, 0x85),
	rgb_t(0x5d, 0x86, 0x43),
	rgb_t(0x21, 0x2e, 0x78),
	rgb_t(0xcf, 0xbe, 0x6f),
	rgb_t(0x89, 0x4a, 0x26),
	rgb_t(0x5b, 0x33, 0x00),
	rgb_t(0xaf, 0x64, 0x59),
	rgb_t(0x43, 0x43, 0x43),
	rgb_t(0x6b, 0x6b, 0x6b),
	rgb_t(0xa0, 0xcb, 0x84),
	rgb_t(0x56, 0x65, 0xb3),
	rgb_t(0x95, 0x95, 0x95)
};

constexpr rgb_t PALETTE_NTSC_OLD[] =
{
	rgb_t(0x00, 0x00, 0x00),
	rgb_t(0xff, 0xff, 0xff),
	rgb_t(0x6b, 0x24, 0x1a),
	rgb_t(0x87, 0xd3, 0xde),
	rgb_t(0x91, 0x69, 0xad),
	rgb_t(0x68, 0x91, 0x4e),
	rgb_t(0x27, 0x34, 0x7e),
	rgb_t(0xd2, 0xc1, 0x72),
	rgb_t(0xad, 0x6e, 0x4a),
	rgb_t(0x5a, 0x32, 0x00),
	rgb_t(0xb3, 0x68, 0x5d),
	rgb_t(0x38, 0x38, 0x38),
	rgb_t(0x7d, 0x7d, 0x7d),
	rgb_t(0xa7, 0xd2, 0x8b),
	rgb_t(0x6a, 0x79, 0xc7),
	rgb_t(0xbd, 0xbd, 0xbd)
};

constexpr int FIRST_BAD_LINE = 0x30;
constexpr int LAST_BAD_LINE = 0xf7;

constexpr uint8_t IRQ_RST = 0x01;
constexpr uint8_t IRQ_MBC = 0x02;
constexpr uint8_t IRQ_MMC = 0x04;
constexpr uint8_t IRQ_LP  = 0x08;

constexpr int REGISTER_LPX = 0x13;
constexpr int REGISTER_LPY = 0x14;
constexpr int REGISTER_CR1 = 0x11;
constexpr int REGISTER_RASTER = 0x12;
constexpr int REGISTER_CR2 = 0x16;
constexpr int REGISTER_MEMORY = 0x18;
constexpr int REGISTER_MSE = 0x15;
constexpr int REGISTER_MYE = 0x17;
constexpr int REGISTER_MDP = 0x1b;
constexpr int REGISTER_MMC = 0x1c;
constexpr int REGISTER_MXE = 0x1d;
constexpr int REGISTER_MM = 0x1e;
constexpr int REGISTER_MD = 0x1f;
constexpr int REGISTER_EC = 0x20;
constexpr int REGISTER_B0C = 0x21;
constexpr int REGISTER_MM0 = 0x25;
constexpr int REGISTER_MM1 = 0x26;
constexpr int REGISTER_M0C = 0x27;

constexpr uint8_t UNUSED_BITS[0x40] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x01, 0x70, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

constexpr int CYCLE_DOTS = 8;
constexpr int PHI2_START = 4;

// dot of the write cycle from which a register write is visible to the logic that consumes it
constexpr int SEEN_LIVE = 0;
constexpr int SEEN_SPRITE_MUX = 2;
constexpr int SEEN_SPRITE_DECODE = 3;
constexpr int SEEN_PHI2 = PHI2_START;
constexpr int SEEN_HMOS_FALL = PHI2_START + 1;
constexpr int SEEN_CYCLE_END = CYCLE_DOTS;

constexpr int NMOS_FALL_LAG_BMM = 2;
constexpr int NMOS_FALL_LAG_ECM = 2;
constexpr int NMOS_FALL_LAG_MCM_IN_ECM = 1;

constexpr int register_seen(int reg)
{
	if (reg <= 0x10)
		return SEEN_PHI2;
	if (reg == REGISTER_MDP || reg == REGISTER_MXE)
		return SEEN_SPRITE_MUX;
	if (reg >= REGISTER_EC && reg < 0x2f)
		return SEEN_LIVE;
	return SEEN_CYCLE_END;
}

constexpr int MCM_FLOP_DOT = 3;

// dots by which the per-cell hires/multicolor latch sees BMM late
constexpr int CELL_MC_BMM_DELAY = 2;

// dots between the visible column a light pen is aimed at and the beam position that triggers it
constexpr int LIGHTPEN_LATENCY_PAL = 39;
constexpr int LIGHTPEN_LATENCY_NTSC = 53;

// dots between a sprite pixel and its collision reaching $d01e/$d01f
constexpr int COLLISION_LAG = 8;

// dots after a collision register read during which new collisions are lost
constexpr int COLLISION_CLEAR_HOLD = COLLISION_LAG + PHI2_START;

// dots after the start of phi2 that a read of the collision registers can still see
constexpr int COLLISION_READ_DELAY = 0;

// dots after the start of a sprite's 16 dot slot
constexpr int SPRITE_SLOT_FROZEN_HIRES = 2;
constexpr int SPRITE_SLOT_HALT = 3;
constexpr int SPRITE_SLOT_ACTIVE_CLEAR = 10;
constexpr int SPRITE_SLOT_RELOAD = 12;
constexpr int SPRITE_SLOT_RESUME = 15;
constexpr int SPRITE_SLOT_DOTS = 16;

constexpr int SPRITE_BA_LEAD = 3;
constexpr int SPRITE_BA_CYCLES = 5;

constexpr int CYCLE_VC_LOAD = 14;
constexpr int CYCLE_REFRESH_FIRST = 11;
constexpr int CYCLE_REFRESH_LAST = 15;
constexpr int CYCLE_MATRIX_FIRST = 15;
constexpr int CYCLE_MATRIX_LAST = 54;
constexpr int CYCLE_BA_FIRST = 12;
constexpr int CYCLE_BA_LAST = 54;
constexpr int CYCLE_GRAPHICS_FIRST = 16;
constexpr int CYCLE_GRAPHICS_LAST = 55;
constexpr int CYCLE_SPRITE_MCBASE = 16;
constexpr int CYCLE_SPRITE_YEXP = 56;
constexpr int CYCLE_ROW_END = 58;

constexpr int BORDER_LEFT_X_40 = 24;
constexpr int BORDER_LEFT_X_38 = 31;
constexpr int BORDER_RIGHT_X_40 = 344;
constexpr int BORDER_RIGHT_X_38 = 335;

constexpr int BORDER_TOP_LINE_25 = 51;
constexpr int BORDER_TOP_LINE_24 = 55;
constexpr int BORDER_BOTTOM_LINE_25 = 251;
constexpr int BORDER_BOTTOM_LINE_24 = 247;

enum : uint16_t
{
	DECODE_LINE_START = 0x0001,
	DECODE_FRAME_START = 0x0002,
	DECODE_REFRESH = 0x0004,
	DECODE_BADLINE_BA = 0x0008,
	DECODE_VC_LOAD = 0x0010,
	DECODE_MATRIX = 0x0020,
	DECODE_GRAPHICS = 0x0040,
	DECODE_SPR_MCBASE = 0x0080,
	DECODE_SPR_DMA = 0x0100,
	DECODE_SPR_YEXP = 0x0200,
	DECODE_ROW_END = 0x0400,
	DECODE_SPR_DISPLAY = 0x0800
};

} // anonymous namespace


struct mos6566_device::raster_timing
{
	int cycles_per_line;
	int lines;
	int line_pixels;
	int first_x;
	int x_stall_phase;
	int x_stall_length;
	int spr_first_cycle;
	int spr_dma_cycle;
	int spr_disp_cycle;
	int row_shift;
	int blank_end;
	int burst_length;
};

struct mos6566_device::process_traits
{
	bool asymmetric_mode_edges;
	bool hmos_sprite_mc_flop;
	bool grey_dot;
	int output_lag;
	int mmc_seen;
	int lightpen_x_offset;
	bool lightpen_frame_irq_only;
	offs_t dma_delay_idle_address;
};

const mos6566_device::raster_timing &mos6566_device::raster_timing_for(uint32_t variant)
{
	static constexpr raster_timing PAL{
			.cycles_per_line = 63, .lines = 312, .line_pixels = 504, .first_x = 0x194,
			.x_stall_phase = 2 * 63, .x_stall_length = 0,
			.spr_first_cycle = 58, .spr_dma_cycle = 55, .spr_disp_cycle = 58,
			.row_shift = 0, .blank_end = 0x1e0, .burst_length = 2 };
	static constexpr raster_timing NTSC{
			.cycles_per_line = 65, .lines = 263, .line_pixels = 512, .first_x = 0x19c,
			.x_stall_phase = 123, .x_stall_length = 2,
			.spr_first_cycle = 59, .spr_dma_cycle = 56, .spr_disp_cycle = 59,
			.row_shift = 263 - VIC6567_FIRST_DISP_LINE, .blank_end = 0x1db, .burst_length = 0 };
	static constexpr raster_timing NTSC_64{
			.cycles_per_line = 64, .lines = 262, .line_pixels = 512, .first_x = 0x19c,
			.x_stall_phase = 2 * 64, .x_stall_length = 0,
			.spr_first_cycle = 59, .spr_dma_cycle = 56, .spr_disp_cycle = 58,
			.row_shift = 262 - VIC6567_FIRST_DISP_LINE, .blank_end = 0x1db, .burst_length = 0 };

	switch (variant)
	{
	case TYPE_6566:
	case TYPE_6567R56A:
		return NTSC_64;

	case TYPE_6567:
	case TYPE_8562:
	case TYPE_8564:
		return NTSC;

	default:
		return PAL;
	}
}

const mos6566_device::process_traits &mos6566_device::process_traits_for(uint32_t variant)
{
	static constexpr process_traits NMOS{
			.asymmetric_mode_edges = true, .hmos_sprite_mc_flop = false, .grey_dot = false,
			.output_lag = 3, .mmc_seen = SEEN_SPRITE_DECODE, .lightpen_x_offset = 2,
			.lightpen_frame_irq_only = false, .dma_delay_idle_address = 0x38ff };
	static constexpr process_traits NMOS_OLD{
			.asymmetric_mode_edges = true, .hmos_sprite_mc_flop = false, .grey_dot = false,
			.output_lag = 3, .mmc_seen = SEEN_SPRITE_DECODE, .lightpen_x_offset = 2,
			.lightpen_frame_irq_only = true, .dma_delay_idle_address = 0x38ff };
	static constexpr process_traits HMOS{
			.asymmetric_mode_edges = false, .hmos_sprite_mc_flop = true, .grey_dot = true,
			.output_lag = 4, .mmc_seen = SEEN_SPRITE_MUX, .lightpen_x_offset = 1,
			.lightpen_frame_irq_only = false, .dma_delay_idle_address = 0x3807 };

	switch (variant)
	{
	case TYPE_6567R56A:
		return NMOS_OLD;

	case TYPE_8562:
	case TYPE_8564:
	case TYPE_8565:
	case TYPE_8566:
		return HMOS;

	default:
		return NMOS;
	}
}


DEFINE_DEVICE_TYPE(MOS6566, mos6566_device, "mos6566", "MOS 6566 VIC-II")
DEFINE_DEVICE_TYPE(MOS6567, mos6567_device, "mos6567", "MOS 6567 VIC-II")
DEFINE_DEVICE_TYPE(MOS6567R56A, mos6567r56a_device, "mos6567r56a", "MOS 6567R56A VIC-II")
DEFINE_DEVICE_TYPE(MOS8562, mos8562_device, "mos8562", "MOS 8562 VIC-II")
DEFINE_DEVICE_TYPE(MOS8564, mos8564_device, "mos8564", "MOS 8564 VIC-IIe")
DEFINE_DEVICE_TYPE(MOS6569, mos6569_device, "mos6569", "MOS 6569 VIC-II")
DEFINE_DEVICE_TYPE(MOS8565, mos8565_device, "mos8565", "MOS 8565 VIC-II")
DEFINE_DEVICE_TYPE(MOS8566, mos8566_device, "mos8566", "MOS 8566 VIC-IIe")


mos6566_device::mos6566_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, device_execute_interface(mconfig, *this)
	, m_icount(0)
	, m_variant(variant)
	, m_timing(raster_timing_for(variant))
	, m_process(process_traits_for(variant))
	, m_space_config{
		{ "videoram", ENDIANNESS_LITTLE, 8, 14, 0, address_map_constructor(FUNC(mos6566_device::default_map<0>), this) },
		{ "colorram", ENDIANNESS_LITTLE, 8, 10, 0, address_map_constructor(FUNC(mos6566_device::default_map<1>), this) } }
	, m_write_irq(*this)
	, m_write_ba(*this)
	, m_write_aec(*this)
	, m_write_k(*this)
	, m_read_charrom(*this, 0)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_palette((variant == TYPE_6567R56A) ? PALETTE_NTSC_OLD : is_ntsc() ? PALETTE_NTSC : PALETTE_PAL)
	, m_phi0(1)
	, m_ba(1)
	, m_aec(1)
	, m_lp(1)
{
}

template <int Space>
void mos6566_device::default_map(address_map &map)
{
	if (!has_configured_map(Space))
		map(0x0000, Space ? 0x03ff : 0x3fff).ram();
}

device_memory_interface::space_config_vector mos6566_device::memory_space_config() const
{
	return space_config_vector{ { 0, &m_space_config[0] }, { 1, &m_space_config[1] } };
}

TIMER_CALLBACK_MEMBER(mos6566_device::fast_changed)
{
	m_cpu->set_unscaled_clock((clock() / 8) << param, !param);
}

void mos6566_device::cpu_access(int ioacc)
{
	if (!fast_mode())
		return;

	// the processor runs at twice the cycle rate: work out in which half cycle of the VIC the access falls
	attoseconds_t const half = cycles_to_attotime(1).as_attoseconds() / 2;
	attotime const now = machine().time();
	attotime const vic = local_time();
	attoseconds_t const ahead = (now >= vic) ? (now - vic).as_attoseconds() : -(vic - now).as_attoseconds();
	attoseconds_t const shifted = ahead + half / 2;
	int64_t const halves = (shifted >= 0) ? (shifted / half) : -((half - 1 - shifted) / half);

	if (BIT(halves, 0))
		return;

	int const offset = int(halves >> 1) % m_timing.cycles_per_line;
	int const cycle = (m_cycle - 1 + offset + m_timing.cycles_per_line) % m_timing.cycles_per_line + 1;

	if (ioacc || (m_decode[cycle].strobes & DECODE_REFRESH))
		m_cpu->adjust_icount(-1);
}

int mos6566_device::sprite_cycle(int sprite) const
{
	return (m_timing.spr_first_cycle - 1 + 2 * sprite) % m_timing.cycles_per_line + 1;
}

void mos6566_device::build_decode()
{
	int const cycles = m_timing.cycles_per_line;

	for (int phase = 0; phase < 2 * cycles; phase++)
		m_phase_x[phase] = phase_x(phase);

	for (int cycle = 1; cycle <= cycles; cycle++)
	{
		auto const within = [cycle] (int first, int last) { return cycle >= first && cycle <= last; };
		cycle_decode &decode = m_decode[cycle];

		decode.strobes = 0;
		if (cycle == 1)
			decode.strobes |= DECODE_LINE_START;
		if (cycle == 2)
			decode.strobes |= DECODE_FRAME_START;
		if (within(CYCLE_REFRESH_FIRST, CYCLE_REFRESH_LAST))
			decode.strobes |= DECODE_REFRESH;
		if (within(CYCLE_BA_FIRST, CYCLE_BA_LAST))
			decode.strobes |= DECODE_BADLINE_BA;
		if (cycle == CYCLE_VC_LOAD)
			decode.strobes |= DECODE_VC_LOAD;
		if (within(CYCLE_MATRIX_FIRST, CYCLE_MATRIX_LAST))
			decode.strobes |= DECODE_MATRIX;
		if (within(CYCLE_GRAPHICS_FIRST, CYCLE_GRAPHICS_LAST))
			decode.strobes |= DECODE_GRAPHICS;
		if (cycle == CYCLE_SPRITE_MCBASE)
			decode.strobes |= DECODE_SPR_MCBASE;
		if (within(m_timing.spr_dma_cycle, m_timing.spr_dma_cycle + 1))
			decode.strobes |= DECODE_SPR_DMA;
		if (cycle == CYCLE_SPRITE_YEXP)
			decode.strobes |= DECODE_SPR_YEXP;
		if (cycle == CYCLE_ROW_END)
			decode.strobes |= DECODE_ROW_END;
		if (cycle == m_timing.spr_disp_cycle)
			decode.strobes |= DECODE_SPR_DISPLAY;

		decode.spr_pointer = -1;
		decode.spr_data = -1;
		decode.spr_ba = 0;
		for (int i = 0; i < 8; i++)
		{
			int const pointer = sprite_cycle(i);

			if (cycle == pointer)
				decode.spr_pointer = i;
			else if (cycle == pointer % cycles + 1)
				decode.spr_data = i;

			if (((cycle - (pointer - SPRITE_BA_LEAD) + 2 * cycles) % cycles) < SPRITE_BA_CYCLES)
				decode.spr_ba |= 1 << i;
		}
	}
}

int mos6566_device::phase_x(int phase) const
{
	if (phase >= m_timing.x_stall_phase)
		phase = std::max(m_timing.x_stall_phase - 1, phase - m_timing.x_stall_length);

	return (m_timing.first_x + 4 * phase) % m_timing.line_pixels;
}

mos6566_device::mos6566_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6566_device(mconfig, MOS6566, tag, owner, clock, TYPE_6566)
{
}

mos6567_device::mos6567_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant)
	: mos6566_device(mconfig, type, tag, owner, clock, variant)
{
}

mos6567_device::mos6567_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6567_device(mconfig, MOS6567, tag, owner, clock, TYPE_6567)
{
}

mos6567r56a_device::mos6567r56a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6567_device(mconfig, MOS6567R56A, tag, owner, clock, TYPE_6567R56A)
{
}

mos8562_device::mos8562_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6567_device(mconfig, MOS8562, tag, owner, clock, TYPE_8562)
{
}

mos8564_device::mos8564_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6567_device(mconfig, MOS8564, tag, owner, clock, TYPE_8564)
{
}

mos6569_device::mos6569_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant)
	: mos6566_device(mconfig, type, tag, owner, clock, variant)
{
}

mos6569_device::mos6569_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6569_device(mconfig, MOS6569, tag, owner, clock, TYPE_6569)
{
}

mos8565_device::mos8565_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6569_device(mconfig, MOS8565, tag, owner, clock, TYPE_8565)
{
}


mos8566_device::mos8566_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6569_device(mconfig, MOS8566, tag, owner, clock, TYPE_8566)
{
}

void mos6566_device::device_start()
{
	set_icountptr(m_icount);

	build_decode();

	if (is_viciie())
		m_fast_timer = timer_alloc(FUNC(mos6566_device::fast_changed), this);

	screen().register_screen_bitmap(m_bitmap);

	save_item(NAME(m_reg));
	save_item(NAME(m_rasterline));
	save_item(NAME(m_cycle));
	save_item(NAME(m_raster_x));
	save_item(NAME(m_last_data));
	save_item(NAME(m_bus_slot));
	save_item(NAME(m_lp));
	save_item(NAME(m_lp_latched_this_frame));
	save_item(NAME(m_lp_pending));
	save_item(NAME(m_aec_delay));
	save_item(NAME(m_phi0));
	save_item(NAME(m_ba));
	save_item(NAME(m_aec));
	save_item(NAME(m_ba_out));
	save_item(NAME(m_aec_out));
	save_item(NAME(m_beam_line));
	save_item(NAME(m_irq_flags));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_out));
	save_item(NAME(m_raster_irq_done));
	save_item(NAME(m_badline));
	save_item(NAME(m_badlines_enabled));
	save_item(NAME(m_display_state));
	save_item(NAME(m_fetch_cr1));
	save_item(NAME(m_vc));
	save_item(NAME(m_vcbase));
	save_item(NAME(m_vmli));
	save_item(NAME(m_rc));
	save_item(NAME(m_refresh));
	save_item(NAME(m_matrix));
	save_item(NAME(m_color));
	save_item(NAME(m_spr_dma));
	save_item(NAME(m_spr_disp));
	save_item(NAME(m_spr_yff));
	save_item(NAME(m_spr_mcbase));
	save_item(NAME(m_spr_mc));
	save_item(NAME(m_spr_pointer));
	save_item(NAME(m_spr_byte));
	save_item(NAME(m_spr_pending));
	save_item(NAME(m_spr_active));
	save_item(NAME(m_spr_halt));
	save_item(NAME(m_spr_shift_data));
	save_item(NAME(m_spr_xphase));
	save_item(NAME(m_spr_mcphase));
	save_item(NAME(m_spr_pixel));
	save_item(NAME(m_dreg));
	save_item(NAME(m_pixel_color));
	save_item(NAME(m_pixel_row));
	save_item(NAME(m_pixel_col));
	save_item(NAME(m_pixel_index));
	save_item(NAME(m_pixel_count));
	save_item(NAME(m_color_write));
	save_item(NAME(m_coll_mm));
	save_item(NAME(m_coll_md));
	save_item(NAME(m_coll_index));
	save_item(NAME(m_coll_clear));
	save_item(STRUCT_MEMBER(m_write_queue, reg));
	save_item(STRUCT_MEMBER(m_write_queue, data));
	save_item(STRUCT_MEMBER(m_write_queue, mask));
	save_item(STRUCT_MEMBER(m_write_queue, when));
	save_item(NAME(m_write_count));
	save_item(NAME(m_write_last));
	save_item(NAME(m_dot));
	save_item(NAME(m_cycle_dot));
	save_item(NAME(m_draw_row));
	save_item(NAME(m_draw_col));
	save_item(NAME(m_draw_pos));
	save_item(NAME(m_latch_gfx));
	save_item(NAME(m_latch_vbuf));
	save_item(NAME(m_seq_shift));
	save_item(NAME(m_seq_vbuf));
	save_item(NAME(m_seq_count));
	save_item(NAME(m_seq_mc));
	save_item(NAME(m_seq_mcm));
	save_item(NAME(m_seq_cell_mc));
	save_item(NAME(m_seq_bmm));
	save_item(NAME(m_seq_xscroll));
	save_item(NAME(m_main_border));
	save_item(NAME(m_vert_border));
	save_item(NAME(m_vert_ff));
}


void mos6566_device::device_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);

	m_rasterline = 0;
	m_beam_line = 0;
	m_cycle = 14;
	m_raster_x = phase_x(2 * (m_cycle - 1));
	m_last_data = 0;
	m_bus_slot = -1;
	m_lp_latched_this_frame = false;
	m_lp_pending = false;

	m_phi0 = 1;
	m_ba = 1;
	m_aec = 1;
	m_aec_delay = 0xff;
	m_ba_out = -1;
	m_aec_out = -1;

	m_irq_flags = 0;
	m_irq_enable = 0;
	m_irq_out = 0;
	m_raster_irq_done = false;

	m_badline = false;
	m_badlines_enabled = false;
	m_display_state = false;
	m_fetch_cr1 = 0;
	m_vc = 0;
	m_vcbase = 0;
	m_vmli = 0;
	m_rc = 7;
	m_refresh = 0xff;
	std::fill(std::begin(m_matrix), std::end(m_matrix), 0);
	std::fill(std::begin(m_color), std::end(m_color), 0);

	m_spr_dma = 0;
	m_spr_disp = 0;
	m_spr_yff = 0xff;
	std::fill(std::begin(m_spr_mcbase), std::end(m_spr_mcbase), 0);
	std::fill(std::begin(m_spr_mc), std::end(m_spr_mc), 0);
	std::fill(std::begin(m_spr_pointer), std::end(m_spr_pointer), 0);
	for (auto &bytes : m_spr_byte)
		std::fill(std::begin(bytes), std::end(bytes), 0);
	m_spr_pending = 0;
	m_spr_active = 0;
	m_spr_halt = 0;
	std::fill(std::begin(m_spr_shift_data), std::end(m_spr_shift_data), 0);
	m_spr_xphase = 0;
	m_spr_mcphase = 0;
	std::fill(std::begin(m_spr_pixel), std::end(m_spr_pixel), 0);

	std::fill(std::begin(m_dreg), std::end(m_dreg), 0);
	std::fill(std::begin(m_pixel_color), std::end(m_pixel_color), 0);
	std::fill(std::begin(m_pixel_row), std::end(m_pixel_row), 0);
	std::fill(std::begin(m_pixel_col), std::end(m_pixel_col), 0);
	m_pixel_index = 0;
	m_pixel_count = 0;
	m_color_write = 0xff;
	std::fill(std::begin(m_coll_mm), std::end(m_coll_mm), 0);
	std::fill(std::begin(m_coll_md), std::end(m_coll_md), 0);
	m_coll_index = 0;
	std::fill(std::begin(m_coll_clear), std::end(m_coll_clear), 0);
	std::fill(std::begin(m_write_last), std::end(m_write_last), 0);
	m_write_count = 0;
	m_dot = 0;
	m_cycle_dot = 0;
	m_draw_row = 0;
	m_draw_col = 0;
	m_draw_pos = 8;
	std::fill(std::begin(m_latch_gfx), std::end(m_latch_gfx), 0);
	std::fill(std::begin(m_latch_vbuf), std::end(m_latch_vbuf), 0);
	m_seq_shift = 0;
	m_seq_vbuf = 0;
	m_seq_count = 0;
	m_seq_mc = 0;
	m_seq_mcm = false;
	m_seq_cell_mc = false;
	m_seq_bmm = 0;
	m_seq_xscroll = 0;

	m_main_border = true;
	m_vert_border = true;
	m_vert_ff = true;

	if (is_viciie())
	{
		m_reg[0x2f] = 0xff;
		m_reg[0x30] = 0xfc;
		m_write_k(0, 7);
		m_fast_timer->adjust(attotime::zero, 0);
	}
}


void mos6566_device::set_ba(int state)
{
	if (fast_mode())
		state = 1;

	if (m_ba != state)
	{
		m_ba = state;

		if (m_ba)
			m_aec_delay = 0xff;
	}
}

void mos6566_device::update_irq()
{
	int const state = ((m_irq_flags & m_irq_enable) != 0) ? 1 : 0;

	if (state != m_irq_out)
	{
		m_irq_out = state;
		m_write_irq(state);
	}
}

void mos6566_device::raise_irq(uint8_t mask)
{
	m_irq_flags |= mask;
	update_irq();
}

void mos6566_device::check_raster_irq()
{
	int const compare = (BIT(m_reg[REGISTER_CR1], 7) << 8) | m_reg[REGISTER_RASTER];

	if (!m_raster_irq_done && m_rasterline == compare)
	{
		m_raster_irq_done = true;
		raise_irq(IRQ_RST);
	}
}

uint8_t mos6566_device::fetch(offs_t address)
{
	m_last_data = space(0).read_byte(address & 0x3fff);
	return m_last_data;
}

uint8_t mos6566_device::fetch_phi2(offs_t address)
{
	if (m_aec)
		return 0xff;

	return fetch(address);
}

uint8_t mos6566_device::read_color(offs_t offset)
{
	return space(1).read_byte(offset & 0x3ff) & 0x0f;
}


void mos6566_device::line_start()
{
	m_beam_line = (m_beam_line == m_timing.lines - 1) ? 0 : (m_beam_line + 1);

	if (m_beam_line != 0)
		m_rasterline = m_beam_line;
}

void mos6566_device::update_badline()
{
	if (m_rasterline == FIRST_BAD_LINE && BIT(m_reg[REGISTER_CR1], 4))
		m_badlines_enabled = true;

	m_badline = m_badlines_enabled
			&& m_rasterline >= FIRST_BAD_LINE
			&& m_rasterline <= LAST_BAD_LINE
			&& (m_rasterline & 7) == (m_reg[REGISTER_CR1] & 7);

	if (m_badline)
		m_display_state = true;
}

void mos6566_device::sprite_dma_check()
{
	for (int i = 0; i < 8; i++)
	{
		uint8_t const mask = 1 << i;

		if ((m_reg[REGISTER_MSE] & mask) && !(m_spr_dma & mask) && m_reg[1 + 2 * i] == (m_rasterline & 0xff))
		{
			m_spr_dma |= mask;
			m_spr_mcbase[i] = 0;
			m_spr_yff |= mask;
		}
	}
}

bool mos6566_device::sprite_phase1(const cycle_decode &decode)
{
	if (decode.spr_pointer >= 0)
	{
		int const i = decode.spr_pointer;
		m_spr_pointer[i] = fetch(((m_reg[REGISTER_MEMORY] & 0xf0) << 6) | 0x3f8 | i);
		return true;
	}

	if (decode.spr_data >= 0)
	{
		int const i = decode.spr_data;
		if (BIT(m_spr_dma, i))
		{
			m_spr_byte[i][1] = fetch((m_spr_pointer[i] << 6) | (m_spr_mc[i] & 0x3f));
			m_spr_mc[i] = (m_spr_mc[i] + 1) & 0x3f;
		}
		else
		{
			m_spr_byte[i][1] = fetch(0x3fff);
		}
		return true;
	}

	return false;
}

void mos6566_device::sprite_phase2(const cycle_decode &decode)
{
	m_bus_slot = -1;

	bool const pointer_cycle = decode.spr_pointer >= 0;
	int const i = pointer_cycle ? decode.spr_pointer : decode.spr_data;
	if (i < 0)
		return;

	int const byte = pointer_cycle ? 0 : 2;

	if (BIT(m_spr_dma, i))
	{
		m_spr_byte[i][byte] = fetch_phi2((m_spr_pointer[i] << 6) | (m_spr_mc[i] & 0x3f));
		m_spr_mc[i] = (m_spr_mc[i] + 1) & 0x3f;
	}
	else
	{
		m_spr_byte[i][byte] = 0xff;
		m_bus_slot = 3 * i + byte;
	}
}

void mos6566_device::graphics_fetch(bool display, bool dma_delay)
{
	uint8_t const mode = fetch_mode();
	bool const ecm = BIT(mode, 6);

	m_latch_gfx[1] = m_latch_gfx[0];
	m_latch_vbuf[1] = m_latch_vbuf[0];

	if (display)
	{
		uint8_t const chr = m_matrix[m_vmli];
		auto const address_for_mode = [this, chr] (uint8_t cr1)
		{
			offs_t address;
			if (BIT(cr1, 5))
				address = ((m_reg[REGISTER_MEMORY] & 0x08) << 10) | ((m_vc & 0x3ff) << 3) | m_rc;
			else
				address = ((m_reg[REGISTER_MEMORY] & 0x0e) << 10) | (chr << 3) | m_rc;
			return BIT(cr1, 6) ? address & 0x39ff : address;
		};

		offs_t address = address_for_mode(mode);
		if (m_process.asymmetric_mode_edges && ((m_reg[REGISTER_CR1] ^ m_fetch_cr1) & 0x20))
		{
			offs_t const from = address_for_mode(m_fetch_cr1);
			offs_t const to = address_for_mode(m_reg[REGISTER_CR1]);
			if (!m_read_charrom(from) && m_read_charrom(to))
				address = (from & 0xff) | (to & 0x3f00);
		}

		m_latch_gfx[0] = fetch(address);
		m_latch_vbuf[0] = (m_color[m_vmli] << 8) | chr;

		m_vmli = (m_vmli + 1) & 0x3f;
		m_vc = (m_vc + 1) & 0x3ff;
	}
	else
	{
		m_latch_gfx[0] = fetch(ecm ? 0x39ff : dma_delay ? m_process.dma_delay_idle_address : 0x3fff);
		m_latch_vbuf[0] = 0;
	}

	if (m_vert_border)
	{
		m_latch_gfx[0] = 0;
		m_latch_vbuf[0] = m_latch_vbuf[1];
	}
}

void mos6566_device::matrix_fetch()
{
	offs_t const address = ((m_reg[REGISTER_MEMORY] & 0xf0) << 6) | (m_vc & 0x3ff);

	m_matrix[m_vmli] = fetch_phi2(address);
	m_color[m_vmli] = read_color(address);
}


void mos6566_device::queue_register(uint8_t reg, uint8_t data)
{
	if (reg == REGISTER_CR1)
	{
		queue_register(reg, data, 0x97, SEEN_CYCLE_END);
		queue_register(reg, data, 0x08, SEEN_LIVE);
		if (m_process.asymmetric_mode_edges)
		{
			queue_edges(reg, data, 0x20, SEEN_LIVE, SEEN_LIVE + NMOS_FALL_LAG_BMM);
			queue_edges(reg, data, 0x40, SEEN_LIVE, SEEN_LIVE + NMOS_FALL_LAG_ECM);
		}
		else
		{
			queue_edges(reg, data, 0x60, SEEN_PHI2, SEEN_HMOS_FALL);
		}
	}
	else if (reg == REGISTER_CR2)
	{
		queue_register(reg, data, 0xe0, SEEN_CYCLE_END);
		queue_register(reg, data, 0x08, SEEN_LIVE);
		queue_register(reg, data, 0x07, SEEN_PHI2);
		queue_edges(reg, data, 0x10, SEEN_LIVE, SEEN_LIVE + mcm_fall_lag(m_reg[REGISTER_CR1]));
	}
	else if (reg == REGISTER_MMC)
	{
		queue_register(reg, data, 0xff, m_process.mmc_seen);
	}
	else
	{
		queue_register(reg, data, 0xff, register_seen(reg));
	}
}

void mos6566_device::queue_edges(uint8_t reg, uint8_t data, uint8_t mask, int rise_seen, int fall_seen)
{
	if (rise_seen == fall_seen)
	{
		queue_register(reg, data, mask, rise_seen);
	}
	else
	{
		queue_register(reg, data, data & mask, rise_seen);
		queue_register(reg, data, ~data & mask, fall_seen);
	}
}

int mos6566_device::mcm_fall_lag(uint8_t cr1) const
{
	return (m_process.asymmetric_mode_edges && BIT(cr1, 6)) ? NMOS_FALL_LAG_MCM_IN_ECM : 0;
}

uint8_t mos6566_device::fetch_mode() const
{
	return m_process.asymmetric_mode_edges ? (m_reg[REGISTER_CR1] | (m_fetch_cr1 & 0x60)) : m_fetch_cr1;
}

void mos6566_device::queue_register(uint8_t reg, uint8_t data, uint8_t mask, int seen)
{
	if (!mask)
		return;

	if (m_write_count == std::size(m_write_queue))
	{
		apply_register(m_write_queue[0].reg, m_write_queue[0].data, m_write_queue[0].mask);
		std::copy(std::begin(m_write_queue) + 1, std::end(m_write_queue), std::begin(m_write_queue));
		m_write_count--;
	}

	uint64_t when = std::max<int64_t>(int64_t(m_cycle_dot) + seen, int64_t(m_dot));
	if (mask == 0xff)
	{
		when = std::max(when, m_write_last[reg]);
		m_write_last[reg] = when;
	}
	m_write_queue[m_write_count++] = { reg, data, mask, when };
}

void mos6566_device::apply_register(uint8_t reg, uint8_t data, uint8_t mask)
{
	if (reg == REGISTER_MMC)
	{
		uint8_t const toggled = m_dreg[reg] ^ data;
		if (m_process.hmos_sprite_mc_flop)
		{
			m_spr_mcphase ^= toggled & m_spr_xphase;
			m_spr_mcphase |= toggled & m_spr_xphase & ~data;
		}
		else
		{
			m_spr_mcphase &= ~toggled;
		}
	}

	if (reg >= REGISTER_EC && reg < 0x2f)
		m_color_write = reg;

	m_dreg[reg] = (m_dreg[reg] & ~mask) | (data & mask);
}

void mos6566_device::apply_registers(uint64_t dot)
{
	int kept = 0;

	for (int i = 0; i < m_write_count; i++)
	{
		if (m_write_queue[i].when <= dot)
			apply_register(m_write_queue[i].reg, m_write_queue[i].data, m_write_queue[i].mask);
		else
			m_write_queue[kept++] = m_write_queue[i];
	}

	m_write_count = kept;
}

void mos6566_device::draw_until(uint64_t dot)
{
	while (m_dot < dot && m_draw_pos < 8)
	{
		if (m_write_count)
			apply_registers(m_dot);

		draw_dot();
		m_draw_pos++;
		m_dot++;
	}
}

void mos6566_device::draw_dot()
{
	int const x = m_phase_x[m_draw_col / 4 + (m_draw_pos >> 2)] + (m_draw_pos & 3);
	uint8_t const cr1 = m_dreg[REGISTER_CR1];
	uint8_t const cr2 = m_dreg[REGISTER_CR2];

	border_unit(x, cr1, cr2);

	int color;
	bool const fg = graphics_sequencer(cr1, cr2, color);

	int sprite_color[8];
	uint8_t const sprite_mask = sprite_sequencer(x, sprite_color);

	collision_unit(sprite_mask, fg);

	if (sprite_mask)
	{
		int const lowest = std::countr_zero(sprite_mask);
		if (!(fg && BIT(m_dreg[REGISTER_MDP], lowest)))
			color = sprite_color[lowest];
	}

	if (m_main_border || fast_mode())
		color = REGISTER_EC;

	if (x >= m_timing.blank_end && x < m_timing.blank_end + m_timing.burst_length)
		color = 0x01;
	else if (x > 0x17c && x < m_timing.blank_end)
		color = 0x00;

	output_stage(color);
}

void mos6566_device::border_unit(int x, uint8_t cr1, uint8_t cr2)
{
	bool const csel = BIT(cr2, 3);
	bool const rsel = BIT(cr1, 3);

	if (x == (csel ? BORDER_RIGHT_X_40 : BORDER_RIGHT_X_38))
		m_main_border = true;

	if (x == (csel ? BORDER_LEFT_X_40 : BORDER_LEFT_X_38))
	{
		if (m_rasterline == (rsel ? BORDER_BOTTOM_LINE_25 : BORDER_BOTTOM_LINE_24))
			m_vert_ff = true;

		m_vert_border = m_vert_ff;
		if (!m_vert_border)
			m_main_border = false;
	}
}

bool mos6566_device::graphics_sequencer(uint8_t cr1, uint8_t cr2, int &color)
{
	int const slot = m_draw_pos;
	bool const next_mcm = BIT(cr2, 4);

	if (slot == MCM_FLOP_DOT + (next_mcm ? 0 : mcm_fall_lag(cr1)))
	{
		if (next_mcm && !m_seq_mcm)
			m_seq_count |= 1;
		m_seq_mcm = next_mcm;
	}

	if ((m_decode[m_draw_col / CYCLE_DOTS + 1].strobes & DECODE_GRAPHICS) && !m_vert_border)
		m_seq_xscroll = cr2 & 7;

	if (((slot - PHI2_START) & 7) == m_seq_xscroll)
	{
		int const latch = (slot >= PHI2_START) ? 0 : 1;

		m_seq_shift = m_latch_gfx[latch];
		m_seq_vbuf = m_latch_vbuf[latch];
		m_seq_count = 0;
		m_seq_cell_mc = BIT(m_seq_bmm, CELL_MC_BMM_DELAY - 1) || BIT(m_seq_vbuf, 11);
	}

	uint16_t const v = m_seq_vbuf;
	if (m_seq_mcm && m_seq_cell_mc)
	{
		if (!(m_seq_count & 1))
			m_seq_mc = (m_seq_shift >> 6) & 3;
	}
	else
	{
		m_seq_mc = BIT(m_seq_shift, 7) << 1;
	}
	m_seq_count++;
	m_seq_shift <<= 1;
	m_seq_bmm = (m_seq_bmm << 1) | BIT(cr1, 5);

	int const bit = BIT(m_seq_mc, 1);
	int const pair = m_seq_mc;
	int const mode = (BIT(cr1, 6) << 2) | (BIT(cr1, 5) << 1) | BIT(cr2, 4);
	bool fg = false;
	color = 0;

	switch (mode)
	{
	case 0:
		fg = bit;
		color = bit ? (v >> 8) & 0x0f : REGISTER_B0C;
		break;

	case 1:
		if (BIT(v, 11))
		{
			fg = BIT(pair, 1);
			color = (pair == 3) ? (v >> 8) & 0x07 : REGISTER_B0C + pair;
		}
		else
		{
			fg = bit;
			color = bit ? (v >> 8) & 0x07 : REGISTER_B0C;
		}
		break;

	case 2:
		fg = bit;
		color = bit ? (v >> 4) & 0x0f : v & 0x0f;
		break;

	case 3:
		fg = BIT(pair, 1);
		switch (pair)
		{
		case 0: color = REGISTER_B0C; break;
		case 1: color = (v >> 4) & 0x0f; break;
		case 2: color = v & 0x0f; break;
		case 3: color = (v >> 8) & 0x0f; break;
		}
		break;

	case 4:
		fg = bit;
		color = bit ? (v >> 8) & 0x0f : REGISTER_B0C + ((v >> 6) & 3);
		break;

	case 5:
		fg = BIT(v, 11) ? BIT(pair, 1) : bit;
		break;

	case 6:
		fg = bit;
		break;

	case 7:
		fg = BIT(pair, 1);
		break;
	}

	return fg;
}

uint8_t mos6566_device::sprite_sequencer(int x, int (&color)[8])
{
	int const dot = m_draw_col + m_draw_pos;
	if (m_draw_pos == 0 && (m_decode[m_draw_col / CYCLE_DOTS + 1].strobes & DECODE_SPR_DISPLAY))
		m_spr_pending = m_spr_disp;

	int slot_dot = dot + PHI2_START - CYCLE_DOTS * (m_timing.spr_first_cycle - 1);
	if (slot_dot < 0)
		slot_dot += CYCLE_DOTS * m_timing.cycles_per_line;
	if (slot_dot < 8 * SPRITE_SLOT_DOTS)
	{
		int const i = slot_dot / SPRITE_SLOT_DOTS;
		uint8_t const mask = 1 << i;

		switch (slot_dot % SPRITE_SLOT_DOTS)
		{
		case SPRITE_SLOT_HALT:
			m_spr_halt |= mask;
			break;
		case SPRITE_SLOT_ACTIVE_CLEAR:
			m_spr_active &= ~mask;
			break;
		case SPRITE_SLOT_RELOAD:
			m_spr_shift_data[i] = (m_spr_byte[i][0] << 16) | (m_spr_byte[i][1] << 8) | m_spr_byte[i][2];
			break;
		case SPRITE_SLOT_RESUME:
			m_spr_halt &= ~mask;
			break;
		}
	}

	uint8_t sprite_mask = 0;

	if (m_spr_pending || m_spr_active)
	{
		for (int i = 0; i < 8; i++)
		{
			uint8_t const mask = 1 << i;

			if ((m_spr_pending & ~m_spr_active & ~m_spr_halt) & mask)
			{
				int const sx = m_dreg[2 * i] | ((m_dreg[0x10] & mask) ? 0x100 : 0);
				if (sx == x)
				{
					m_spr_active |= mask;
					m_spr_xphase &= ~mask;
					m_spr_mcphase |= mask;
				}
			}

			if (!(m_spr_active & mask))
				continue;

			if (!m_spr_shift_data[i] && !m_spr_pixel[i])
			{
				m_spr_active &= ~mask;
				continue;
			}

			if (!(m_spr_halt & mask))
			{
				uint32_t const data = m_spr_shift_data[i];

				if (!(m_spr_xphase & mask))
				{
					if (m_dreg[REGISTER_MMC] & mask)
					{
						if (m_spr_mcphase & mask)
							m_spr_pixel[i] = (data >> 22) & 3;

						m_spr_mcphase ^= mask;
					}
					else if (!m_process.hmos_sprite_mc_flop || (m_spr_mcphase & mask))
					{
						m_spr_pixel[i] = BIT(data, 23) << 1;
					}
					else
					{
						m_spr_mcphase |= mask;
					}

					m_spr_shift_data[i] = (data << 1) & 0xffffff;
				}

				if (m_dreg[REGISTER_MXE] & mask)
					m_spr_xphase ^= mask;
				else
					m_spr_xphase &= ~mask;
			}

			// a halted sprite outputs its frozen pixel as hires, as seen on real 6569 and 8565 (test-136-2a)
			int const sp = ((m_spr_halt & mask) || (slot_dot == SPRITE_SLOT_DOTS * i + SPRITE_SLOT_FROZEN_HIRES)) ? (m_spr_pixel[i] & 2) : m_spr_pixel[i];
			if (sp)
			{
				sprite_mask |= mask;
				color[i] = (sp == 1) ? REGISTER_MM0 : (sp == 3) ? REGISTER_MM1 : REGISTER_M0C + i;
			}
		}
	}

	return sprite_mask;
}

void mos6566_device::collision_unit(uint8_t sprite_mask, bool fg)
{
	uint8_t const mm = (sprite_mask & (sprite_mask - 1)) ? sprite_mask : 0;
	uint8_t const md = (sprite_mask && fg) ? sprite_mask : 0;
	uint8_t &ring_mm = m_coll_mm[m_coll_index];
	uint8_t &ring_md = m_coll_md[m_coll_index];

	if (ring_mm && m_dot >= m_coll_clear[0])
	{
		if (!m_reg[REGISTER_MM])
			raise_irq(IRQ_MMC);

		m_reg[REGISTER_MM] |= ring_mm;
	}

	if (ring_md && m_dot >= m_coll_clear[1])
	{
		if (!m_reg[REGISTER_MD])
			raise_irq(IRQ_MBC);

		m_reg[REGISTER_MD] |= ring_md;
	}

	ring_mm = mm;
	ring_md = md;
	m_coll_index = (m_coll_index + 1) & (COLLISION_LAG - 1);
}

void mos6566_device::output_stage(int color)
{
	int const lag = m_process.output_lag;
	if (m_pixel_count == lag)
	{
		uint8_t const selected = m_pixel_color[m_pixel_index];
		int const resolved = (m_process.grey_dot && selected == m_color_write) ? 0x0f
				: (selected < 0x10) ? selected : m_dreg[selected] & 0x0f;
		m_bitmap.pix(m_pixel_row[m_pixel_index], m_pixel_col[m_pixel_index]) = m_palette[resolved];
	}
	else
	{
		m_pixel_count++;
	}

	m_pixel_color[m_pixel_index] = color;
	m_pixel_row[m_pixel_index] = m_draw_row;
	m_pixel_col[m_pixel_index] = m_draw_col + m_draw_pos;
	if (++m_pixel_index == lag)
		m_pixel_index = 0;
	m_color_write = 0xff;
}

void mos6566_device::execute_run()
{
	do
	{
		int const cycle = m_cycle;
		cycle_decode const &decode = m_decode[cycle];

		draw_until(m_cycle_dot + CYCLE_DOTS);

		m_phi0 = 0;
		m_aec_delay = (m_aec_delay << 1) | m_ba;
		m_aec = 0;

		if (decode.strobes & DECODE_LINE_START)
		{
			line_start();
		}
		else if (decode.strobes & DECODE_FRAME_START)
		{
			if (m_beam_line == 0)
			{
				m_rasterline = 0;
				m_vcbase = 0;
				m_refresh = 0xff;
				m_badlines_enabled = false;
				m_lp_latched_this_frame = false;

				if (!m_lp)
				{
					m_lp_pending = false;
					m_lp_latched_this_frame = true;
					m_reg[REGISTER_LPX] = 0xd1;
					m_reg[REGISTER_LPY] = 0;
					raise_irq(IRQ_LP);
				}
			}
		}

		if (m_rasterline != ((BIT(m_reg[REGISTER_CR1], 7) << 8) | m_reg[REGISTER_RASTER]))
			m_raster_irq_done = false;
		else if (!(decode.strobes & DECODE_LINE_START))
			check_raster_irq();

		bool const display = m_display_state;
		update_badline();
		bool const dma_delay = !display && m_display_state;

		bool const rsel = BIT(m_reg[REGISTER_CR1], 3);
		if (m_rasterline == (rsel ? BORDER_TOP_LINE_25 : BORDER_TOP_LINE_24) && BIT(m_reg[REGISTER_CR1], 4))
			m_vert_ff = m_vert_border = false;
		if (m_rasterline == (rsel ? BORDER_BOTTOM_LINE_25 : BORDER_BOTTOM_LINE_24))
			m_vert_ff = true;
		if (decode.strobes & DECODE_LINE_START)
			m_vert_border = m_vert_ff;

		if (decode.strobes & DECODE_VC_LOAD)
		{
			m_vc = m_vcbase;
			m_vmli = 0;

			if (m_badline)
				m_rc = 0;
		}
		else if (decode.strobes & DECODE_SPR_MCBASE)
		{
			for (int i = 0; i < 8; i++)
			{
				uint8_t const mask = 1 << i;

				m_spr_yff |= ~m_reg[REGISTER_MYE] & mask;

				if (m_spr_yff & mask)
				{
					m_spr_mcbase[i] = m_spr_mc[i];

					if (m_spr_mcbase[i] == 63)
						m_spr_dma &= ~mask;
				}
			}
		}
		else if (decode.strobes & DECODE_SPR_DMA)
		{
			sprite_dma_check();
		}
		else if (decode.strobes & DECODE_ROW_END)
		{
			if (m_rc == 7)
			{
				m_vcbase = m_vc;
				m_display_state = false;
			}

			if (m_badline || m_display_state)
			{
				m_display_state = true;
				m_rc = (m_rc + 1) & 7;
			}
		}

		if (decode.strobes & DECODE_SPR_DISPLAY)
		{
			for (int i = 0; i < 8; i++)
			{
				uint8_t const mask = 1 << i;

				m_spr_mc[i] = m_spr_mcbase[i];

				if (m_spr_dma & mask)
				{
					if ((m_reg[REGISTER_MSE] & mask) && m_reg[1 + 2 * i] == (m_rasterline & 0xff))
						m_spr_disp |= mask;
				}
				else
				{
					m_spr_disp &= ~mask;
				}
			}
		}

		if (decode.strobes & DECODE_GRAPHICS)
		{
			graphics_fetch(display, dma_delay);
		}
		else
		{
			m_latch_gfx[1] = m_latch_gfx[0];
			m_latch_vbuf[1] = m_latch_vbuf[0];
			m_latch_gfx[0] = 0;

			if (decode.strobes & DECODE_REFRESH)
				fetch(0x3f00 | m_refresh--);
			else if (!sprite_phase1(decode))
				fetch(0x3fff);
		}

		bool const ba_low = (m_badline && (decode.strobes & DECODE_BADLINE_BA)) || (m_spr_dma & decode.spr_ba);
		set_ba(!ba_low);

		m_fetch_cr1 = m_reg[REGISTER_CR1];
		m_phi0 = 1;
		m_aec = BIT(m_aec_delay, 2);

		if (decode.strobes & DECODE_SPR_YEXP)
		{
			for (int i = 0; i < 8; i++)
			{
				uint8_t const mask = 1 << i;

				if ((m_reg[REGISTER_MYE] & mask) && (m_spr_dma & mask))
					m_spr_yff ^= mask;
			}
		}

		if (m_badline && (decode.strobes & DECODE_MATRIX))
			matrix_fetch();

		sprite_phase2(decode);

		if (m_ba_out != m_ba)
		{
			m_ba_out = m_ba;
			m_write_ba(m_ba);
		}

		if (m_aec_out != m_aec)
		{
			m_aec_out = m_aec;
			m_write_aec(m_aec);
		}

		m_cycle_dot = m_dot;
		m_draw_row = (m_beam_line + m_timing.row_shift) % m_timing.lines;
		m_draw_col = (cycle - 1) * CYCLE_DOTS;
		m_draw_pos = 0;

		if (m_lp_pending)
			trigger_lightpen(cycle);

		m_cycle = (cycle == m_timing.cycles_per_line) ? 1 : (cycle + 1);
		m_raster_x = phase_x(2 * (m_cycle - 1));

		m_icount--;
	} while (m_icount > 0);
}


uint32_t mos6566_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


uint8_t mos6566_device::read(offs_t offset)
{
	offset &= 0x3f;

	if (is_viciie() && (offset == 0x2f || offset == 0x30))
		return m_reg[offset];

	uint8_t val;

	switch (offset)
	{
	case REGISTER_CR1:
		val = (m_reg[offset] & 0x7f) | (BIT(m_rasterline, 8) << 7);
		break;

	case REGISTER_RASTER:
		val = m_rasterline & 0xff;
		break;

	case REGISTER_CR2:
		val = m_reg[offset] | 0xc0;
		break;

	case REGISTER_MEMORY:
		val = m_reg[offset] | 0x01;
		break;

	case 0x19:
		if (!machine().side_effects_disabled())
			draw_until(m_cycle_dot + PHI2_START + COLLISION_READ_DELAY);
		val = m_irq_flags | 0x70 | (m_irq_out ? 0x80 : 0x00);
		break;

	case 0x1a:
		val = m_irq_enable | 0xf0;
		break;

	case REGISTER_MM:
	case REGISTER_MD:
		if (!machine().side_effects_disabled())
			draw_until(m_cycle_dot + PHI2_START + COLLISION_READ_DELAY);
		val = m_reg[offset];
		if (!machine().side_effects_disabled())
		{
			m_reg[offset] = 0;
			m_coll_clear[offset - REGISTER_MM] = m_dot + COLLISION_CLEAR_HOLD;
		}
		break;

	default:
		val = (offset < 0x2f) ? m_reg[offset] : 0;
		break;
	}

	val |= UNUSED_BITS[offset];
	if (m_bus_slot >= 0 && !machine().side_effects_disabled())
		m_spr_byte[m_bus_slot / 3][m_bus_slot % 3] = val;

	return val;
}


void mos6566_device::write(offs_t offset, uint8_t data)
{
	offset &= 0x3f;

	if (m_bus_slot >= 0)
		m_spr_byte[m_bus_slot / 3][m_bus_slot % 3] = data;

	if (is_viciie())
	{
		if (offset == 0x2f)
		{
			m_reg[offset] = data | 0xf8;
			m_write_k(0, data & 7);
			return;
		}
		if (offset == 0x30)
		{
			if (BIT(m_reg[offset], 0) != BIT(data, 0))
			{
				m_cpu->abort_timeslice();
				m_fast_timer->adjust(attotime::zero, BIT(data, 0));
			}
			m_reg[offset] = data | 0xfc;
			if (fast_mode())
				set_ba(1);
			return;
		}
	}

	switch (offset)
	{
	case 0x19:
		m_irq_flags &= ~(data & 0x0f);
		update_irq();
		break;

	case 0x1a:
		m_irq_enable = data & 0x0f;
		update_irq();
		break;

	case REGISTER_CR1:
	case REGISTER_RASTER:
		m_reg[offset] = data;
		queue_register(offset, data);
		check_raster_irq();
		if (m_rasterline == FIRST_BAD_LINE && BIT(m_reg[REGISTER_CR1], 4))
			m_badlines_enabled = true;
		break;

	case REGISTER_MYE:
		if (m_decode[m_cycle].strobes & DECODE_SPR_MCBASE)
		{
			for (int i = 0; i < 8; i++)
			{
				if (!BIT(data, i) && !BIT(m_spr_yff, i))
					m_spr_mc[i] = (0x2a & (m_spr_mcbase[i] & m_spr_mc[i])) | (0x15 & (m_spr_mcbase[i] | m_spr_mc[i]));
			}
		}
		m_reg[offset] = data;
		queue_register(offset, data);
		m_spr_yff |= ~data;
		break;

	case REGISTER_MM:
	case REGISTER_MD:
	case REGISTER_LPX:
	case REGISTER_LPY:
		break;

	default:
		if (offset < 0x2f)
		{
			m_reg[offset] = data;
			queue_register(offset, data);
		}
		break;
	}
}


void mos6566_device::lp_w(int state)
{
	if (m_lp && !state)
		m_lp_pending = true;

	m_lp = state;
}

void mos6566_device::trigger_lightpen(int cycle)
{
	m_lp_pending = false;

	if (m_lp_latched_this_frame)
		return;

	m_lp_latched_this_frame = true;

	if (m_rasterline == m_timing.lines - 1 && cycle != 1)
		return;

	m_reg[REGISTER_LPX] = ((m_raster_x & ~7) >> 1) + m_process.lightpen_x_offset;
	m_reg[REGISTER_LPY] = m_rasterline & 0xff;

	if (!m_process.lightpen_frame_irq_only)
		raise_irq(IRQ_LP);
}


attotime mos6566_device::time_until_pos(int rasterline, int raster_x) const
{
	int target = 1;
	for (int cycle = 1; cycle <= m_timing.cycles_per_line; cycle++)
	{
		if ((phase_x(2 * (cycle - 1)) >> 3) == ((raster_x % m_timing.line_pixels) >> 3))
		{
			target = cycle;
			break;
		}
	}

	int lines_to_advance = (rasterline - m_beam_line + m_timing.lines) % m_timing.lines;
	if (lines_to_advance == 0)
		lines_to_advance = m_timing.lines;

	uint64_t const cycles = uint64_t(m_timing.cycles_per_line - m_cycle + 1) + uint64_t(lines_to_advance - 1) * m_timing.cycles_per_line + (target - 1);

	return cycles_to_attotime(cycles);
}

attotime mos6566_device::time_until_lightpen_pos(int x255, int y255) const
{
	int const first_line = is_ntsc() ? VIC6567_FIRST_DISP_LINE : VIC6569_FIRST_DISP_LINE;
	int const first_column = is_ntsc() ? VIC6567_FIRST_COLUMN : VIC6569_FIRST_COLUMN;
	int const visible_lines = is_ntsc() ? VIC6567_VISIBLELINES : VIC6569_VISIBLELINES;
	int const visible_columns = is_ntsc() ? VIC6567_VISIBLECOLUMNS : VIC6569_VISIBLECOLUMNS;

	int const line = (first_line + (y255 * visible_lines) / 256) % m_timing.lines;
	int const column = first_column + (x255 * visible_columns) / 256;
	int const latency = is_ntsc() ? LIGHTPEN_LATENCY_NTSC : LIGHTPEN_LATENCY_PAL;
	int const x = (phase_x(column >> 2) + (column & 3) + latency) % m_timing.line_pixels;

	return time_until_pos(line, x);
}
