// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    Picture timing and interrupt scheduling for the XaviX video devices.
    Maintains raster counters, arena boundaries and video-field timing,
    schedules NMI and raster/arena IRQ events, and implements the picture
    position and timing registers used by the renderer.

    TODO:
    - Align partial updates with the retained picture counters.
    - Confirm original-chip vertical counter endpoints and PAL-M/PAL-N
      selection/status encoding.

***************************************************************************/

#include "emu.h"
#include "xavix_video.h"

#include "util/multibyte.h"

#include <algorithm>
#include <array>

#define LOG_IRQ (1U << 5)

#define VERBOSE 0
#include "logmacro.h"

//-------------------------------------------------
//  Screen Timing and Geometry
//-------------------------------------------------

constexpr std::array<u8, 0x1a> superxavix_video_device::TIMING_MASK =
{
	0x7f, 0x00, 0x7f, 0xff, 0xff, 0x01, 0xff, 0x01,
	0xff, 0x00, 0xff, 0x03, 0xff, 0x07, 0x7f, 0x00,
	0x7f, 0x00, 0x0f, 0x0f, 0x0f, 0x1f, 0x1f, 0x1f,
	0x3f, 0x7f
};

constexpr std::array<u8, 8> superxavix_video_device::POSITION_MASK =
{
	0xff, 0x0f, 0xff, 0x01, 0xff, 0x0f, 0xff, 0x01
};

constexpr std::array<u8, 0x1a> superxavix_video_device::TIMING_NTSC_DEFAULT =
{
	0x29, 0x00, 0x40, 0xa3, 0x0a, 0x01, 0x24, 0x01,
	0x90, 0x00, 0xe3, 0x01, 0x03, 0x04, 0x4a, 0x00,
	0x0d, 0x00, 0x03, 0x03, 0x03, 0x09, 0x0f, 0x03,
	0x2b, 0x2b
};

constexpr std::array<u8, 0x1a> superxavix_video_device::TIMING_PAL_DEFAULT =
{
	0x23, 0x00, 0x3f, 0xa3, 0x07, 0x01, 0x2d, 0x01,
	0x8d, 0x00, 0x3e, 0x02, 0x01, 0x04, 0x51, 0x00,
	0x73, 0x00, 0x03, 0x02, 0x03, 0x08, 0x0d, 0x03,
	0x14, 0x35
};

void xavix_video_device::configure_raster(int width, int height, rectangle const &visible, attotime period)
{
	m_raster_visible = visible;
	rectangle picture = picture_area() & visible;
	if (picture.empty())
		picture = rectangle(0, 0, 0, 0);
	if ((screen().width() != width) || (screen().height() != height)
			|| (screen().visible_area() != picture) || (screen().frame_period() != period))
	{
		// Changing the visible bottom shifts the screen's beam coordinates.
		// Retain the beam position while selecting the displayed picture.
		int const row = screen().vpos();
		int const pixel = screen().hpos();
		screen().configure(width, height, picture, period);
		screen().reset_origin(row - picture.top(), pixel);
	}
	m_vblank_start_timer->adjust(screen().time_until_pos(0), 1);
	m_vblank_end_timer->adjust(screen().time_until_pos(m_raster_visible.top()), 0);
}

void xavix_video_device::reset_raster_origin(int row)
{
	screen().reset_origin(row - screen().visible_area().top(), 0);
	m_vblank_start_timer->adjust(screen().time_until_pos(0), 1);
	m_vblank_end_timer->adjust(screen().time_until_pos(m_raster_visible.top()), 0);
}

TIMER_CALLBACK_MEMBER(xavix_video_device::raster_vblank)
{
	if (param)
	{
		raster_end();
		configure_raster(screen().width(), screen().height(), m_raster_visible, screen().frame_period());
		// A full-height field has coincident electrical blanking edges.
		m_vblank_end_timer->adjust(m_raster_visible.top() ? screen().time_until_pos(m_raster_visible.top()) : attotime::zero, 0);
	}
	m_vblank_cb(param);
}

void superxavix_video_device::configure_screen(screen_device &screen, u32 master_clock, video_standard standard)
{
	auto const &defaults = (standard == video_standard::PAL) ? TIMING_PAL_DEFAULT : TIMING_NTSC_DEFAULT;
	// The reset picture origin is even, so field-left parity determines the initial output phase.
	video_timing const timing = calculate_video_timing(defaults.data(), BIT(defaults[TIMING_FIELD_LEFT], 0));
	rectangle const visible = timing.visible_area();
	screen.set_raw(master_clock / OUTPUT_PIXEL_CLOCKS, timing.width(), visible.left(), visible.right() + 1,
			timing.height(), visible.top(), visible.bottom() + 1);
	// Both reset profiles contain whole lines; retain the master-clock field period.
	screen.set_refresh(attotime::from_ticks(u64(timing.horizontal_total) * timing.height(), master_clock));
}

void superxavix_video_device::video_timing_regs_w(offs_t offset, u8 data)
{
	data &= TIMING_MASK[offset];
	LOG("%s video_timing_regs_w %02x %02x\n", machine().describe_context(), offset, data);
	if (((offset == TIMING_HORIZONTAL_TOTAL) || (offset == TIMING_FIELD_LEFT)
			|| (offset == TIMING_FIELD_LEFT + 1) || (offset == TIMING_FIELD_RIGHT)) && (data != m_video_timing_regs[offset]))
		screen().update_partial(screen().vpos());
	sync_video_counters();
	m_video_timing_regs[offset] = data;
	if ((offset == TIMING_HORIZONTAL_TOTAL) || (offset == TIMING_HALF_LINES))
		schedule_picture_irqs();
}

void xavix_video_device::reset_video_timing()
{
	counter_config const config = counter_parameters();
	m_video_counters = {};
	m_video_counters.picture_x = config.left;
	m_video_counters.picture_y = (config.top + (config.vertical_period + 2) / 2 - config.vertical_reference - 1) & 0x01ff;
	m_video_counter_ticks = attotime_to_clocks(machine().time());
	schedule_picture_irqs();
}

void superxavix_video_device::reset_video_timing()
{
	bool const pal = m_video_standard == video_standard::PAL;
	auto const &defaults = pal ? TIMING_PAL_DEFAULT : TIMING_NTSC_DEFAULT;
	std::copy(defaults.cbegin(), defaults.cend(), m_video_timing_regs.begin());

	// The horizontal reference selects the end of the second half-line.
	put_u16le(&m_video_position_regs[0], 0x0800 | ((0x0a80 | m_video_timing_regs[TIMING_HORIZONTAL_TOTAL]) / 2));
	counter_config const picture_defaults = xavix_video_device::counter_parameters();
	put_u16le(&m_video_position_regs[2], picture_defaults.vertical_reference);
	// Independent reset preloads for the 12-bit X and 9-bit Y picture coordinates.
	put_u16le(&m_video_position_regs[4], picture_defaults.left);
	put_u16le(&m_video_position_regs[6], picture_defaults.top);
	m_video_timing = calculate_video_timing();
	apply_video_timing();
	xavix_video_device::reset_video_timing();
	reset_raster_origin(screen().height() - m_video_timing.vertical_origin);
}

superxavix_video_device::video_timing superxavix_video_device::calculate_video_timing() const
{
	return calculate_video_timing(&m_video_timing_regs[0], output_sample_phase_clocks());
}

superxavix_video_device::video_timing superxavix_video_device::calculate_video_timing(u8 const *registers, int output_sample_phase_clocks)
{
	video_timing timing{
		.horizontal_total = u16((0x0a80 | registers[TIMING_HORIZONTAL_TOTAL]) + 1),
		.half_lines = u16((0x0200 | registers[TIMING_HALF_LINES]) + 1),
		.active_width = MAX_ACTIVE_WIDTH
	};

	u16 const half_clocks = (timing.horizontal_total + 1) / 2;
	u16 const half_limit = (timing.horizontal_total - 1) / 2;
	u16 const field_left = get_u16le(&registers[TIMING_FIELD_LEFT]);
	u16 const field_right = 0x0500 | registers[TIMING_FIELD_RIGHT];

	if ((field_left <= half_limit) && (field_right <= half_limit))
	{
		// Display blanking counts output samples independently of the picture-column layout.
		int const active_clocks = half_clocks + field_right - field_left - output_sample_phase_clocks;
		timing.active_width = std::max(1, (active_clocks + OUTPUT_PIXEL_CLOCKS - 1) / OUTPUT_PIXEL_CLOCKS);
		timing.horizontal_start = (field_left + output_sample_phase_clocks + OUTPUT_PIXEL_CLOCKS - 1) / OUTPUT_PIXEL_CLOCKS;
	}

	u16 const field_bottom = registers[TIMING_FIELD_BOTTOM];
	u16 const field_top = registers[TIMING_FIELD_TOP];
	u16 const active_half_lines = (field_top == field_bottom)
			? timing.half_lines
			: (field_bottom + timing.half_lines - field_top) % timing.half_lines;
	timing.active_height = std::max<u16>(1, (active_half_lines + 1) / 2);
	timing.vertical_origin = (field_bottom + 1) / 2;

	return timing;
}

rectangle superxavix_video_device::video_timing::visible_area() const
{
	int const visible_left = std::min<int>(width() - 1, horizontal_start);
	int const visible_right = std::min<int>(width() - 1, visible_left + active_width - 1);
	int const visible_top = std::max<int>(0, height() - active_height);
	return rectangle(visible_left, visible_right, visible_top, height() - 1);
}

void superxavix_video_device::apply_video_timing()
{
	// Odd half-line totals retain the exact field period on a rounded-up host raster.
	attotime const period = attotime::from_ticks(
			u64(m_video_timing.horizontal_total) * m_video_timing.half_lines,
			clock() * 2);

	configure_raster(m_video_timing.width(), m_video_timing.height(), m_video_timing.visible_area(), period);
	if (m_arena_irq_timer && (m_arena_ctrl & ARENA_IRQ_ENABLE))
		schedule_arena_irq();
}

//-------------------------------------------------
//  Picture Counters
//-------------------------------------------------

auto xavix_video_device::counter_parameters() const -> counter_config
{
	bool const pal = m_video_standard == video_standard::PAL;
	// Fixed timing on earlier chips. Retain the configured 525/625-half-line
	// cadence pending confirmation of the original vertical counter endpoints.
	u16 const period = (pal ? PAL_LINE_CLOCKS : NTSC_LINE_CLOCKS) - 1;
	return { period, u16((pal ? PAL_FIELD_HALF_LINES : NTSC_FIELD_HALF_LINES) - 1), u16(0x0800 | (period / 2)), 3,
			u16((pal ? -586 : -574) & 0x0fff), u16((pal ? -41 : -13) & 0x01ff) };
}

auto superxavix_video_device::counter_parameters() const -> counter_config
{
	return { u16(0x0a80 | m_video_timing_regs[TIMING_HORIZONTAL_TOTAL]),
			u16(0x0200 | m_video_timing_regs[TIMING_HALF_LINES]),
			get_u16le(&m_video_position_regs[0]), get_u16le(&m_video_position_regs[2]),
			get_u16le(&m_video_position_regs[4]), get_u16le(&m_video_position_regs[6]) };
}

u32 xavix_video_device::counter_interval(counter_config const &config, video_counters const &counters)
{
	u32 ticks = (((config.horizontal_period >> 1) - ((counters.horizontal & 0x0fff) >> 1)) & 0x07ff) + 1;
	if (BIT(counters.horizontal, 12) == BIT(config.horizontal_reference, 11))
		ticks = std::min<u32>(ticks, (((config.horizontal_reference & 0x07ff) - ((counters.horizontal & 0x0fff) >> 1)) & 0x07ff) + 1);
	return ticks;
}

void xavix_video_device::advance_counters(counter_config const &config, video_counters &counters, u64 ticks)
{
	while (ticks)
	{
		u32 const elapsed = std::min<u64>(ticks, counter_interval(config, counters));
		u16 const horizontal = (counters.horizontal & 0x1000) | ((counters.horizontal + 2 * (elapsed - 1)) & 0x0fff);
		u16 const vertical = counters.vertical;
		counters.picture_x = (counters.picture_x + elapsed) & 0x0fff;
		if ((horizontal >> 1) == config.horizontal_reference)
		{
			counters.picture_x = config.left;
			counters.picture_y = ((vertical >> 1) == config.vertical_reference)
					? config.top
					: (counters.picture_y + 1) & 0x01ff;
		}

		if (((horizontal & 0x0fff) >> 1) == (config.horizontal_period >> 1))
		{
			counters.horizontal = (horizontal ^ 0x1000) & 0x1000;
			counters.horizontal |= BIT(horizontal, 0) + 1 - BIT(config.horizontal_period, 0);
			counters.vertical = (vertical == config.vertical_period) ? 0 : (vertical + 1) & 0x03ff;
		}
		else
		{
			counters.horizontal = (horizontal & 0x1000) | ((horizontal + 2) & 0x0fff);
		}
		ticks -= elapsed;
	}
}

void xavix_video_device::sync_video_counters()
{
	u64 const ticks = attotime_to_clocks(machine().time());
	if (ticks <= m_video_counter_ticks)
		return;
	bool const edge = advance_arena(counter_parameters(), m_video_counters, m_arena, ticks - m_video_counter_ticks);
	m_video_counter_ticks = ticks;
	if (edge && (m_arena_ctrl & ARENA_IRQ_ENABLE) && !m_arena_irq_pending)
	{
		m_arena_irq_pending = true;
		update_video_irq_source();
	}
}

u32 xavix_video_device::arena_interval(counter_config const &config, video_counters const &counters, arena_state const &arena) const
{
	unsigned const x = counters.picture_x;
	unsigned const boundary = 8 * (arena.flag ? m_arena_end : m_arena_start);
	bool const active_line = counters.picture_y >= PICTURE_FIRST_ROW && counters.picture_y < PICTURE_END_ROW;
	bool const match = active_line && !BIT(x, 11) && (x / PICTURE_PIXEL_CLOCKS == boundary / PICTURE_PIXEL_CLOCKS);
	if (arena.compare || match)
		return 1;

	// Skip only stable stretches, stopping at a counter reload, selected boundary
	// or output-latch update. Blanking presets the flag from the side control.
	u32 ticks = counter_interval(config, counters);
	if (active_line)
		ticks = std::min<u32>(ticks, ((boundary - x) & 0x0fff) + 1);
	bool const side = bool(m_arena_ctrl & ARENA_SIDE);
	if ((arena.output != arena.flag) || (BIT(x, 11) && (arena.flag != side)))
		ticks = std::min<u32>(ticks, ((x & 7) == 7) ? 1 : ((6 - x) & 7) + 1);
	else if (arena.flag != side)
		ticks = std::min<u32>(ticks, ((0x0806 - x) & 0x0fff) + 1);
	return ticks;
}

bool xavix_video_device::advance_arena(counter_config const &config, video_counters &counters, arena_state &arena, u64 ticks) const
{
	bool edge = false;
	while (ticks)
	{
		u32 const elapsed = std::min<u64>(ticks, arena_interval(config, counters, arena));
		advance_counters(config, counters, elapsed - 1);
		unsigned const x = counters.picture_x;
		bool const match = (counters.picture_y >= PICTURE_FIRST_ROW) && (counters.picture_y < PICTURE_END_ROW)
				&& !BIT(x, 11) && ((x >> 3) == (arena.flag ? m_arena_end : m_arena_start));
		edge |= arena.compare && !match;
		arena.compare = match;
		if ((x & 7) >= 6)
		{
			// The colour-effects gate takes the previous flag, not the new comparison result.
			arena.output = arena.flag;
			if (BIT(x, 11))
				arena.flag = bool(m_arena_ctrl & ARENA_SIDE);
			else if (match)
				arena.flag = !arena.flag;
		}
		advance_counters(config, counters, 1);
		ticks -= elapsed;
	}
	return edge;
}

xavix_video_device::picture_event_delays xavix_video_device::predict_picture_delays(unsigned events) const
{
	counter_config const config = counter_parameters();
	u64 const now = std::max(m_video_counter_ticks, attotime_to_clocks(machine().time()));
	video_counters counters = m_video_counters;
	arena_state arena = m_arena;
	if (events & PREDICT_ARENA)
		advance_arena(config, counters, arena, now - m_video_counter_ticks);
	else
		advance_counters(config, counters, now - m_video_counter_ticks);
	picture_event_delays result;
	u64 elapsed = 0;
	while (true)
	{
		u32 const interval = (events & PREDICT_ARENA) ? arena_interval(config, counters, arena) : counter_interval(config, counters);
		auto const compare = [this, &events, &counters, interval, now, &elapsed] (unsigned event, unsigned x, unsigned y, attotime &delay)
		{
			if (events & event)
			{
				u32 const target = (counters.picture_x == 8 * x + 7) ? 1 : ((8 * x + 6 - counters.picture_x) & 0x0fff) + 1;
				if ((counters.picture_y == y) && (target <= interval))
				{
					delay = clocks_to_attotime(now + elapsed + target) - machine().time();
					events &= ~event;
				}
			}
		};

		compare(PREDICT_NMI, 256, PICTURE_END_ROW - 1, result.nmi);
		compare(PREDICT_RASTER, m_raster_irq_x[0], m_raster_irq_y[0], result.raster);
		if (!events)
			return result;

		u16 const vertical = counters.vertical;
		if (events & PREDICT_ARENA)
		{
			if (advance_arena(config, counters, arena, interval))
			{
				result.arena = clocks_to_attotime(now + elapsed + interval) - machine().time();
				events &= ~PREDICT_ARENA;
				if (!events)
					return result;
			}
		}
		else
		{
			advance_counters(config, counters, interval);
		}
		elapsed += interval;
		if (vertical && !counters.vertical)
		{
			result.field = clocks_to_attotime(now + elapsed) - machine().time();
			return result;
		}
	}
}

//-------------------------------------------------
//  Picture Timing and Interrupt Scheduling
//-------------------------------------------------

void xavix_video_device::schedule_picture_irqs()
{
	// Earlier chips retain raster-based IRQ/NMI timing and partial updates.
	// Only the arena has been moved to the retained picture counters here.
	unsigned const arena = ((m_arena_ctrl & ARENA_IRQ_ENABLE) && !m_arena_irq_pending) ? PREDICT_ARENA : 0;
	picture_event_delays const next = predict_picture_delays(PREDICT_FIELD | arena);
	m_video_counter_timer->adjust(next.field);
	if (!m_arena_irq_timer->enabled() || !m_arena_irq_timer->remaining().is_zero())
		m_arena_irq_timer->adjust(next.arena);
}

void superxavix_video_device::schedule_picture_irqs()
{
	unsigned const arena = ((m_arena_ctrl & ARENA_IRQ_ENABLE) && !m_arena_irq_pending) ? PREDICT_ARENA : 0;
	picture_event_delays const next = predict_picture_delays(PREDICT_FIELD | PREDICT_NMI | PREDICT_RASTER | arena);

	// Reconsider unreachable comparisons at the next field boundary, without
	// cancelling a comparison that occurs on the boundary itself.
	m_video_counter_timer->adjust(next.field);
	if (!m_picture_nmi_timer->enabled() || !m_picture_nmi_timer->remaining().is_zero())
		m_picture_nmi_timer->adjust(next.nmi);
	if (!m_video_field_timer->enabled() || !m_video_field_timer->remaining().is_zero())
		m_video_field_timer->adjust(next.field);
	if (!m_raster_irq_timer->enabled() || !m_raster_irq_timer->remaining().is_zero())
		m_raster_irq_timer->adjust(next.raster, 0);
	if (!m_arena_irq_timer->enabled() || !m_arena_irq_timer->remaining().is_zero())
		m_arena_irq_timer->adjust(next.arena);
}

TIMER_CALLBACK_MEMBER(xavix_video_device::counter_field_start)
{
	sync_video_counters();
	schedule_picture_irqs();
}

TIMER_CALLBACK_MEMBER(superxavix_video_device::picture_nmi)
{
	sync_video_counters();
	// Finish this native field before software replaces its picture data.
	// TODO: Align partial updates with the picture counters rather than the approximate raster origin.
	screen().update_partial(screen().visible_area().bottom());
	// Retain the full-raster tail of a rendered field, including after its
	// cropped picture was published. Skipped fields advance at blanking.
	if (m_bitmap_rendered)
		update_bitmap_rows(screen().height() - 1);
	xavix_video_device::vblank();
	m_picture_nmi_timer->adjust(predict_picture_delays(PREDICT_NMI).nmi);
}

void superxavix_video_device::schedule_raster_irq()
{
	m_raster_irq_timer->adjust(predict_picture_delays(PREDICT_RASTER).raster, 0);
}

TIMER_CALLBACK_MEMBER(superxavix_video_device::video_field_start)
{
	sync_video_counters();
	m_video_field_phase = BIT(m_video_counters.horizontal, 12) ^ BIT(m_video_counters.vertical, 0);
	m_video_field_timer->adjust(predict_picture_delays(PREDICT_FIELD).field);

	// Sample presentation geometry at the native field boundary. An independent
	// screen callback can catch a temporary blanking-line period adjustment and
	// incorrectly apply it to every line, resizing the displayed picture.
	video_timing const timing = calculate_video_timing();
	if (timing != m_video_timing)
	{
		m_video_timing = timing;
		apply_video_timing();
	}
}

void superxavix_video_device::video_position_regs_w(offs_t offset, u8 data)
{
	data &= POSITION_MASK[offset];
	LOG("%s video_position_regs_w %02x %02x\n", machine().describe_context(), offset, data);
	if (data != m_video_position_regs[offset])
		screen().update_partial(screen().vpos());
	sync_video_counters();
	m_video_position_regs[offset] = data;
	schedule_picture_irqs();
}

//-------------------------------------------------
//  Video Status and Control
//-------------------------------------------------

u8 xavix_video_device::video_status_r()
{
	// Bit 4 is the original one-bit video-system selector: clear for NTSC and set for PAL.
	// Monster Truck selects between separate PAL and NTSC test-screen graphics at 0x1854c;
	// Card Night likewise has separate regional tilesets.  Both test bit 4.
	return (m_video_standard == video_standard::PAL) ? 0x10 : 0x00;
}

u8 superxavix_video_device::video_status_r()
{
	// Bit 4 reports the video-system input. Bit 7 independently selects PAL colour
	// mode and initially matches bit 4; only these default values are returned here.
	// Super XaviX PAL startup code checks both bits together.
	return (m_video_standard == video_standard::PAL) ? 0x90 : 0x00;
}

void xavix_video_device::vblank()
{
	if (m_video_ctrl & VIDEO_VBLANK_NMI_ENABLE)
	{
		m_nmi_cb(1);
		m_video_ctrl |= VIDEO_VBLANK_NMI_PENDING;
	}

}

u8 xavix_video_device::video_ctrl_r()
{
	// Bits 7 and 6 report pending vertical and raster interrupts;
	// bits 5 and 4 enable the respective sources. Bits 3-0 read as zero.

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_IRQ, "%s: video_ctrl_r %02x\n", machine().describe_context(), m_video_ctrl);
	return m_video_ctrl;
}

void xavix_video_device::video_ctrl_w(u8 data)
{
	u8 const old_ctrl = m_video_ctrl;

	// Pending flags are read-only status with write-one-to-clear semantics.
	// Preserve enabled requests across writes that do not acknowledge them.
	m_video_ctrl = (m_video_ctrl & (VIDEO_RASTER_IRQ_PENDING | VIDEO_VBLANK_NMI_PENDING))
			| (data & (VIDEO_RASTER_IRQ_ENABLE | VIDEO_VBLANK_NMI_ENABLE));

	// Acknowledging or disabling either source clears its pending request.
	if ((data & VIDEO_RASTER_IRQ_PENDING) || !(data & VIDEO_RASTER_IRQ_ENABLE))
		m_video_ctrl &= ~VIDEO_RASTER_IRQ_PENDING;

	if ((data & VIDEO_VBLANK_NMI_PENDING) || !(data & VIDEO_VBLANK_NMI_ENABLE))
	{
		m_video_ctrl &= ~VIDEO_VBLANK_NMI_PENDING;
		m_nmi_cb(0);
	}

	update_video_irq_source();

	LOGMASKED(LOG_IRQ, "[irq] display control %02x -> %02x beam=%d,%d\n",
			old_ctrl, m_video_ctrl, screen().vpos(), screen().hpos());
}

//-------------------------------------------------
//  Video IRQ Output and Raster Interrupts
//-------------------------------------------------

void xavix_video_device::update_video_irq_source()
{
	m_irq_cb((m_video_ctrl & VIDEO_RASTER_IRQ_PENDING)
			|| (m_arena_irq_pending && (m_arena_ctrl & ARENA_IRQ_ENABLE))
			|| (m_sprite_dma_ctrl & SPRITE_DMA_COMPLETE));
}

TIMER_CALLBACK_MEMBER(xavix_video_device::raster_irq_gen)
{
	if (m_video_ctrl & VIDEO_RASTER_IRQ_ENABLE)
	{
		LOGMASKED(LOG_IRQ, "[irq] raster compare beam=%d,%d target=%02x,%02x\n",
				screen().vpos(), screen().hpos(), m_raster_irq_y[0], m_raster_irq_x[0]);
		m_video_ctrl |= VIDEO_RASTER_IRQ_PENDING;
		update_video_irq_source();

		// The comparison uses picture rows, which need not share the screen's
		// scanline origin. Finish that row before software changes the tilemaps.
		screen().update_partial(m_raster_irq_y[0] - counter_offset().second + render_offset().second);
	}
	schedule_raster_irq();
}

void xavix_video_device::schedule_raster_irq()
{
	auto const [xoffset, yoffset] = counter_offset();
	int const row = m_raster_irq_y[0] - yoffset;
	// Off-field comparisons must not wrap to row zero and render the next field during sprite DMA.
	m_raster_irq_timer->adjust((row >= 0 && row < screen().height())
			? screen().time_until_pos(row, m_raster_irq_x[0] - xoffset) : attotime::never, 0);
}

void xavix_video_device::raster_irq_x_w(u8 data)
{
	LOG("%s: raster_irq_x_w %02x\n", machine().describe_context(), data);
	m_raster_irq_x[0] = data;
	schedule_raster_irq();
}

void xavix_video_device::raster_irq_y_w(u8 data)
{
	LOG("%s: raster_irq_y_w %02x\n", machine().describe_context(), data);
	m_raster_irq_y[0] = data;

	schedule_raster_irq();
}

//-------------------------------------------------
//  Arena Interrupts
//-------------------------------------------------

u8 xavix_video_device::arena_start_r()
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_IRQ, "%s: arena_start_r %02x\n", machine().describe_context(), m_arena_start);
	return m_arena_start;
}

void xavix_video_device::arena_start_w(u8 data)
{
	LOG("%s: arena_start_w %02x\n", machine().describe_context(), data);
	sync_video_counters();
	if (data != m_arena_start)
	{
		screen().update_partial(screen().vpos());
		m_arena_start = data;
	}
	schedule_arena_irq();
}

u8 xavix_video_device::arena_end_r()
{
	if (!machine().side_effects_disabled())
		LOG("%s: arena_end_r\n", machine().describe_context());
	return m_arena_end;
}

void xavix_video_device::arena_end_w(u8 data)
{
	LOG("%s: arena_end_w %02x\n", machine().describe_context(), data);
	sync_video_counters();
	if (data != m_arena_end)
	{
		screen().update_partial(screen().vpos());
		m_arena_end = data;
	}
	schedule_arena_irq();
}

u8 xavix_video_device::arena_ctrl_r()
{
	// Debugger reads predict the current state without advancing IRQs or saved counters.
	video_counters counters = m_video_counters;
	arena_state arena = m_arena;
	bool pending = m_arena_irq_pending;
	if (machine().side_effects_disabled())
	{
		u64 const ticks = attotime_to_clocks(machine().time());
		if (ticks > m_video_counter_ticks)
			pending |= advance_arena(counter_parameters(), counters, arena, ticks - m_video_counter_ticks) && (m_arena_ctrl & ARENA_IRQ_ENABLE);
	}
	else
	{
		sync_video_counters();
		arena = m_arena;
		pending = m_arena_irq_pending;
	}
	return m_arena_ctrl | (arena.flag ? ARENA_STATUS_ACTIVE : 0) | (pending ? ARENA_STATUS_PENDING : 0);
}

void xavix_video_device::arena_ctrl_w(u8 data)
{
	sync_video_counters();
	u8 const control = data & (ARENA_SIDE | ARENA_IRQ_ENABLE);
	if ((control ^ m_arena_ctrl) & ARENA_SIDE)
		screen().update_partial(screen().vpos());
	m_arena_ctrl = control;
	if ((data & ARENA_STATUS_PENDING) || !(control & ARENA_IRQ_ENABLE))
		m_arena_irq_pending = false;
	update_video_irq_source();
	schedule_arena_irq();
}

TIMER_CALLBACK_MEMBER(xavix_video_device::arena_irq_gen)
{
	sync_video_counters();
	schedule_arena_irq();
}

void xavix_video_device::schedule_arena_irq()
{
	// Preserve a comparison due now when presentation timing changes or a state is loaded.
	sync_video_counters();
	m_arena_irq_timer->adjust(((m_arena_ctrl & ARENA_IRQ_ENABLE) && !m_arena_irq_pending)
			? predict_picture_delays(PREDICT_ARENA).arena : attotime::never);
}
