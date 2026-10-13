// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    MOS 6560/6561 Video Interface Chip (VIC) emulation

***************************************************************************/

#include "emu.h"
#include "mos6560.h"

#include <algorithm>
#include <cmath>


namespace {

constexpr rgb_t PALETTE_6561[16] =
{
	rgb_t(0x00, 0x00, 0x00), rgb_t(0xff, 0xff, 0xff), rgb_t(0xb6, 0x1f, 0x21), rgb_t(0x4d, 0xf0, 0xff),
	rgb_t(0xb4, 0x3f, 0xff), rgb_t(0x44, 0xe2, 0x37), rgb_t(0x1a, 0x34, 0xff), rgb_t(0xdc, 0xd7, 0x1b),
	rgb_t(0xca, 0x54, 0x00), rgb_t(0xe9, 0xb0, 0x72), rgb_t(0xe7, 0x92, 0x93), rgb_t(0x9a, 0xf7, 0xfd),
	rgb_t(0xe0, 0x9f, 0xff), rgb_t(0x8f, 0xe4, 0x93), rgb_t(0x82, 0x90, 0xff), rgb_t(0xe5, 0xde, 0x85)
};

constexpr rgb_t PALETTE_6560[16] =
{
	rgb_t(0x00, 0x00, 0x00), rgb_t(0xff, 0xff, 0xff), rgb_t(0xf9, 0x11, 0x37), rgb_t(0x35, 0xf9, 0xf6),
	rgb_t(0xff, 0x3c, 0xc6), rgb_t(0x3c, 0xed, 0xa9), rgb_t(0x0f, 0x57, 0xf7), rgb_t(0xfe, 0xe9, 0x63),
	rgb_t(0xfb, 0x62, 0x44), rgb_t(0xfb, 0xbf, 0xde), rgb_t(0xf3, 0xac, 0xe5), rgb_t(0xa8, 0xea, 0xdd),
	rgb_t(0xe6, 0xb8, 0xf7), rgb_t(0xab, 0xdd, 0xa4), rgb_t(0x6a, 0xb3, 0xe7), rgb_t(0xf7, 0xda, 0xa5)
};

constexpr int OUTPUT_DELAY = 7;

} // anonymous namespace


DEFINE_DEVICE_TYPE(MOS6560,            mos6560_device,            "mos6560",            "MOS 6560 VIC")
DEFINE_DEVICE_TYPE(MOS6561,            mos6561_device,            "mos6561",            "MOS 6561 VIC")
DEFINE_DEVICE_TYPE(MOS656X_ATTACK_UFO, mos656x_attack_ufo_device, "mos656x_attack_ufo", "MOS 656X VIC (Attack UFO)")


void mos6560_device::mos6560_videoram_map(address_map &map)
{
	if (!has_configured_map(0))
		map(0x0000, 0x3fff).ram();
}

void mos6560_device::mos6560_colorram_map(address_map &map)
{
	if (!has_configured_map(1))
		map(0x000, 0x3ff).ram();
}

mos6560_device::mos6560_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cycles_per_line, int lines, int raster_offset, const rgb_t *palette, bool ntsc, bool interlace, bool attack_ufo) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_sound_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_videoram_space_config("videoram", ENDIANNESS_LITTLE, 8, 14, 0, address_map_constructor(FUNC(mos6560_device::mos6560_videoram_map), this)),
	m_colorram_space_config("colorram", ENDIANNESS_LITTLE, 8, 10, 0, address_map_constructor(FUNC(mos6560_device::mos6560_colorram_map), this)),
	m_cycles_per_line(cycles_per_line),
	m_lines(lines),
	m_raster_offset(raster_offset),
	m_palette(palette),
	m_ntsc(ntsc),
	m_interlace_capable(interlace),
	m_attack_ufo(attack_ufo),
	m_read_potx(*this, 0xff),
	m_read_poty(*this, 0xff),
	m_stream(nullptr),
	m_line_timer(nullptr),
	m_lp_state(false)
{
}

mos6560_device::mos6560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mos6560_device(mconfig, MOS6560, tag, owner, clock, CYCLES, LINES, 37, PALETTE_6560, true, true, false)
{
}

mos6561_device::mos6561_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mos6560_device(mconfig, MOS6561, tag, owner, clock, CYCLES, LINES, 0, PALETTE_6561, false, false, false)
{
}

mos656x_attack_ufo_device::mos656x_attack_ufo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mos6560_device(mconfig, MOS656X_ATTACK_UFO, tag, owner, clock, CYCLES, LINES, 37, PALETTE_6560, true, false, true)
{
}

device_memory_interface::space_config_vector mos6560_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_videoram_space_config),
		std::make_pair(1, &m_colorram_space_config)
	};
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6560_device::device_start()
{
	screen().register_screen_bitmap(m_bitmap[0]);
	screen().register_screen_bitmap(m_bitmap[1]);
	m_bitmap[0].fill(rgb_t::black());
	m_bitmap[1].fill(rgb_t::black());
	m_draw = 0;

	m_period = HZ_TO_ATTOSECONDS(clock());
	m_base_time = attotime::zero;
	m_base_cycle = 0;
	m_cycle_count = 0;
	m_frame_start = 0;
	m_frame_lines = m_lines;

	std::fill(std::begin(m_reg), std::end(m_reg), 0);

	m_line = 0;
	m_cyc = 0;
	m_area = AREA_IDLE;
	m_fetch_state = FETCH_IDLE;
	m_start_delay = 0;
	m_buf_offset = 0;
	m_text_cols = 0;
	m_pending_text_cols = 0;
	m_text_lines = 0;
	m_char_height = 8;
	m_ycounter = 0;
	m_row_counter = 0;
	m_vmc = 0;
	m_vmb = 0;
	m_vmc_left = 0;
	m_vmc_inc = false;
	m_vmc_wrap_inc = false;
	m_blank_this_line = true;
	m_line_was_blank = true;
	m_interlace = false;
	m_field = 0;

	m_vbuf = 0;
	m_cbuf = 0;
	std::fill(std::begin(m_gload), std::end(m_gload), 0);
	std::fill(std::begin(m_cload), std::end(m_cload), 0);
	std::fill(std::begin(m_load), std::end(m_load), false);
	m_gdata = 0;
	m_cdata = 0;
	m_shift_pos = 8;
	m_vbus = 0;
	m_vbus_high = 0;

	m_out_bg = 0;
	m_out_border = 0;
	m_out_aux = 0;
	m_out_reverse = false;
	m_pend_reg = -1;
	m_pend_data = 0;

	m_lp_triggered = false;
	m_lp_trigger = ~uint64_t(0);
	m_lp_x = 0;
	m_lp_y = 0;
	m_lp_extra = 1;

	m_snd_div = 0;
	std::fill(std::begin(m_snd_cnt), std::end(m_snd_cnt), 0);
	std::fill(std::begin(m_snd_sreg), std::end(m_snd_sreg), 0);
	m_noise_lfsr = 0;
	m_noise_lfsr0 = false;

	for (unsigned i = 0; i < std::size(m_dac); i++)
		m_dac[i] = 0.888 * (1.0 - std::exp(-double(i) / 140.0));

	m_stream = stream_alloc(0, 1, clock() / 2);

	m_line_timer = timer_alloc(FUNC(mos6560_device::line_tick), this);
	attotime const line_period(0, m_period * m_cycles_per_line);
	m_line_timer->adjust(line_period, 0, line_period);

	save_item(NAME(m_draw));
	save_item(NAME(m_base_time));
	save_item(NAME(m_base_cycle));
	save_item(NAME(m_cycle_count));
	save_item(NAME(m_frame_start));
	save_item(NAME(m_frame_lines));
	save_item(NAME(m_reg));
	save_item(NAME(m_line));
	save_item(NAME(m_cyc));
	save_item(NAME(m_area));
	save_item(NAME(m_fetch_state));
	save_item(NAME(m_start_delay));
	save_item(NAME(m_buf_offset));
	save_item(NAME(m_text_cols));
	save_item(NAME(m_pending_text_cols));
	save_item(NAME(m_text_lines));
	save_item(NAME(m_char_height));
	save_item(NAME(m_ycounter));
	save_item(NAME(m_row_counter));
	save_item(NAME(m_vmc));
	save_item(NAME(m_vmb));
	save_item(NAME(m_vmc_left));
	save_item(NAME(m_vmc_inc));
	save_item(NAME(m_vmc_wrap_inc));
	save_item(NAME(m_blank_this_line));
	save_item(NAME(m_line_was_blank));
	save_item(NAME(m_interlace));
	save_item(NAME(m_field));
	save_item(NAME(m_vbuf));
	save_item(NAME(m_cbuf));
	save_item(NAME(m_gload));
	save_item(NAME(m_cload));
	save_item(NAME(m_load));
	save_item(NAME(m_gdata));
	save_item(NAME(m_cdata));
	save_item(NAME(m_shift_pos));
	save_item(NAME(m_vbus));
	save_item(NAME(m_vbus_high));
	save_item(NAME(m_out_bg));
	save_item(NAME(m_out_border));
	save_item(NAME(m_out_aux));
	save_item(NAME(m_out_reverse));
	save_item(NAME(m_pend_reg));
	save_item(NAME(m_pend_data));
	save_item(NAME(m_lp_state));
	save_item(NAME(m_lp_triggered));
	save_item(NAME(m_lp_trigger));
	save_item(NAME(m_lp_x));
	save_item(NAME(m_lp_y));
	save_item(NAME(m_lp_extra));
	save_item(NAME(m_snd_div));
	save_item(NAME(m_snd_cnt));
	save_item(NAME(m_snd_sreg));
	save_item(NAME(m_noise_lfsr));
	save_item(NAME(m_noise_lfsr0));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6560_device::device_reset()
{
	m_lp_triggered = false;
	m_lp_trigger = ~uint64_t(0);
}


//-------------------------------------------------
//  cycle bookkeeping
//-------------------------------------------------

uint64_t mos6560_device::cycle_at(const attotime &time) const
{
	if (time >= m_base_time)
		return m_base_cycle + uint64_t((time - m_base_time).as_attoseconds() / m_period);

	attotime const delta = m_base_time - time;
	if (delta.seconds() > 0)
		return 0;

	uint64_t const cycles = uint64_t((delta.as_attoseconds() + m_period - 1) / m_period);
	return (cycles > m_base_cycle) ? 0 : (m_base_cycle - cycles);
}

uint64_t mos6560_device::current_cycle()
{
	attotime const now = machine().time();
	if (now <= m_base_time)
		return m_base_cycle;

	return cycle_at(now);
}

void mos6560_device::update_cycles()
{
	if (!started())
		return;

	uint64_t const target = current_cycle();

	while (m_cycle_count < target)
	{
		cycle();
		m_cycle_count++;
	}
}

TIMER_CALLBACK_MEMBER(mos6560_device::line_tick)
{
	update_cycles();

	uint64_t const cycles = m_cycle_count - m_base_cycle;
	m_base_time += attotime(0, cycles * m_period);
	m_base_cycle = m_cycle_count;
}

int mos6560_device::raster_line(uint64_t cycle, int *pos) const
{
	int64_t rel = int64_t(cycle) + m_raster_offset - int64_t(m_frame_start);
	int64_t const frame = int64_t(m_frame_lines) * m_cycles_per_line;
	int field = m_field;
	int prev_lines = m_interlace ? (m_field ? m_lines + 2 : m_lines + 1) : m_lines;

	if (rel >= frame)
	{
		rel -= frame;
		field = m_interlace ? (m_field ^ 1) : 0;
		prev_lines = m_frame_lines;
	}
	else if (rel < 0)
	{
		rel += frame;
	}

	int line = int(rel / m_cycles_per_line);
	int cyc = int(rel % m_cycles_per_line);

	if (m_ntsc && !field && (line == 0))
	{
		if (cyc < 33)
			line = prev_lines;
		else
			cyc -= 33;
	}

	if (pos)
		*pos = cyc;

	return line;
}

int mos6560_device::last_line_cycles(int line) const
{
	if (m_ntsc && (line == m_frame_lines))
		return 33;

	return m_cycles_per_line;
}


//-------------------------------------------------
//  raster state machine
//-------------------------------------------------

void mos6560_device::open_v()
{
	m_area = AREA_PENDING;

	if (m_text_lines == 0)
		close_v();
}

void mos6560_device::close_v()
{
	m_area = AREA_DONE;
}

void mos6560_device::open_h()
{
	m_fetch_state = FETCH_START;
	m_start_delay = 4;
	m_buf_offset = 0;

	if (m_area == AREA_PENDING)
		m_area = AREA_DISPLAY;

	m_text_cols = m_pending_text_cols;
}

void mos6560_device::handle_vmc()
{
	int const vmc = m_vmc;

	if ((m_ycounter == m_char_height) || (m_ycounter == 2 * m_char_height))
	{
		m_ycounter = 0;

		if (!m_line_was_blank)
			m_vmb = vmc;

		m_row_counter++;
		if (m_row_counter == m_text_lines)
			close_v();
	}

	m_vmc = m_vmb;

	if (m_vmc_wrap_inc)
	{
		if (!(vmc & 0x0f))
			m_vmc += 0x10;
		if (!(vmc & 0xff))
			m_vmc += 0x100;
		m_vmc &= 0xfff;
	}
}

void mos6560_device::count_vmc()
{
	m_vmc_inc = m_vmc_left > 0;

	if (m_vmc_inc)
	{
		m_vmc = (m_vmc + 1) & 0xfff;
		m_vmc_left--;
	}
}

void mos6560_device::end_of_line()
{
	m_line_was_blank = m_blank_this_line;
	m_cyc = 0;

	if (m_area == AREA_DISPLAY)
		m_ycounter++;

	m_vmc_wrap_inc = m_vmc_inc;
	m_vmc_inc = false;
	m_vmc_left = 0;
	m_fetch_state = FETCH_IDLE;
	m_blank_this_line = true;
	m_line++;
}

void mos6560_device::end_of_frame()
{
	m_row_counter = 0;
	m_line = 0;
	m_area = AREA_IDLE;
	m_ycounter = 0;
	m_vmc = 0;
	m_vmb = 0;
	m_vmc_wrap_inc = false;
	m_frame_start = m_cycle_count + 1;
	m_draw ^= 1;

	bool const lace = m_interlace_capable && BIT(m_reg[0], 7);

	if (m_interlace)
	{
		m_field ^= 1;
		if (!lace)
		{
			m_interlace = false;
			m_field = 0;
		}
	}
	else if (lace)
	{
		m_interlace = true;
		m_field = 0;
	}

	if (m_interlace)
		m_frame_lines = m_field ? m_lines + 1 : m_lines + 2;
	else
		m_frame_lines = m_lines;
}

uint8_t mos6560_device::fetch_byte(offs_t addr)
{
	m_vbus = space(0).read_byte(addr & 0x3fff);
	m_vbus_high = space(1).read_byte(addr & 0x3ff) & 0x0f;

	return m_vbus;
}

void mos6560_device::fetch()
{
	switch (m_fetch_state)
	{
	case FETCH_START:
		if (--m_start_delay == 0)
		{
			m_text_cols = m_pending_text_cols;

			if (m_text_cols > 0)
			{
				m_blank_this_line = false;
				m_vmc_left = 2 * m_text_cols;
				m_fetch_state = FETCH_MATRIX;
			}
			else
			{
				m_fetch_state = FETCH_DONE;
			}
		}
		break;

	case FETCH_MATRIX:
		{
			offs_t const addr = (((m_reg[5] & 0xf0) << 6) | ((m_reg[2] & 0x80) << 2)) + (m_vmc >> 1);
			m_vbuf = fetch_byte(addr);
			m_cbuf = m_vbus_high;
			m_fetch_state = FETCH_CHARGEN;
		}
		break;

	case FETCH_CHARGEN:
		{
			offs_t const addr = ((m_reg[5] & 0x0f) << 10) + (m_vbuf * m_char_height) + (m_ycounter & ((m_char_height >> 1) | 7));
			m_gload[2] = fetch_byte(addr);
			m_cload[2] = m_cbuf;
			m_load[2] = true;

			m_buf_offset++;

			if (m_buf_offset >= m_text_cols)
				m_fetch_state = FETCH_DONE;
			else
				m_fetch_state = FETCH_MATRIX;
		}
		break;

	default:
		break;
	}
}

void mos6560_device::output_pixels()
{
	if (m_load[0])
	{
		m_gdata = m_gload[0];
		m_cdata = m_cload[0];
		m_shift_pos = 0;
	}

	m_load[0] = m_load[1];
	m_gload[0] = m_gload[1];
	m_cload[0] = m_cload[1];
	m_load[1] = m_load[2];
	m_gload[1] = m_gload[2];
	m_cload[1] = m_cload[2];
	m_load[2] = false;

	int row = m_line;
	int x = 4 * (m_cyc - OUTPUT_DELAY);
	bitmap_rgb32 *bitmap = &m_bitmap[m_draw];

	if (m_cyc < OUTPUT_DELAY)
	{
		x += 4 * m_cycles_per_line;

		if (row == 0)
		{
			bitmap = &m_bitmap[m_draw ^ 1];
			row = m_lines - 1;
		}
		else
		{
			row--;
		}
	}

	bool const visible = (row < bitmap->height()) && (x >= 0) && (x + 3 < bitmap->width());
	uint32_t *dest = visible ? &bitmap->pix(row, x) : nullptr;

	for (int i = 0; i < 4; i++)
	{
		if (m_pend_reg >= 0)
		{
			if (i == 1)
			{
				if (m_pend_reg == 0x0e)
				{
					m_out_aux = m_pend_data >> 4;
					m_pend_reg = -1;
				}
				else
				{
					m_out_bg = m_pend_data >> 4;
					m_out_border = m_pend_data & 0x07;
				}
			}
			else if (i == 3)
			{
				m_out_reverse = !BIT(m_pend_data, 3);
				m_pend_reg = -1;
			}
		}

		uint8_t color = m_out_border;

		if (m_shift_pos < 8)
		{
			if (m_attack_ufo)
			{
				color = BIT(m_gdata, 7 - m_shift_pos) ? (m_cdata & 0x0f) : m_out_bg;
			}
			else if (BIT(m_cdata, 3))
			{
				switch ((m_gdata >> (6 - (m_shift_pos & 6))) & 3)
				{
				case 0: color = m_out_bg; break;
				case 1: color = m_out_border; break;
				case 2: color = m_cdata & 0x07; break;
				case 3: color = m_out_aux; break;
				}
			}
			else
			{
				bool const bit = BIT(m_gdata, 7 - m_shift_pos) ^ m_out_reverse;
				color = bit ? (m_cdata & 0x07) : m_out_bg;
			}

			m_shift_pos++;
		}

		if (dest)
			dest[i] = m_palette[color];
	}
}

void mos6560_device::trigger_light_pen()
{
	m_lp_trigger = ~uint64_t(0);

	if (m_lp_triggered)
		return;

	m_lp_triggered = true;

	int pos;
	int const line = raster_line(m_cycle_count + 1, &pos);
	int const last = m_ntsc ? m_frame_lines : (m_frame_lines - 1);

	if ((line == last) && (pos > 1))
		return;

	m_lp_x = 2 * ((m_cyc + 1) % m_cycles_per_line) + m_lp_extra;
	m_lp_y = line >> 1;
	m_lp_extra = 1;
}

void mos6560_device::cycle()
{
	if ((m_area == AREA_IDLE) && ((m_attack_ufo ? 14 : m_reg[1]) == (m_line >> 1)))
		open_v();

	m_cyc++;

	if (m_cyc == m_cycles_per_line)
	{
		end_of_line();

		if (m_line >= m_frame_lines)
			end_of_frame();
	}

	if ((m_area == AREA_DISPLAY) || (m_area == AREA_PENDING))
	{
		if ((m_fetch_state == FETCH_IDLE) && ((m_attack_ufo ? 8 : (m_reg[0] & 0x7f)) == m_cyc))
			open_h();

		if ((m_area == AREA_DISPLAY) && (m_cyc == 0))
			handle_vmc();
	}

	if ((m_line == 0) && (m_cyc == 2))
		m_text_lines = (m_reg[3] & 0x7e) >> 1;

	int lp_pos;
	if (((m_line == 0) || (m_line >= (m_frame_lines - 1))) && (raster_line(m_cycle_count + 1, &lp_pos) == 0) && (lp_pos == 1))
	{
		m_lp_triggered = false;

		if (m_lp_state)
			m_lp_trigger = m_cycle_count + 2;
	}

	if (m_cyc == 1)
		m_pending_text_cols = std::min(m_reg[2] & 0x7f, m_ntsc ? 31 : 32);

	if (m_lp_trigger == m_cycle_count + 1)
		trigger_light_pen();

	fetch();
	count_vmc();
	output_pixels();
}


//-------------------------------------------------
//  register access
//-------------------------------------------------

uint8_t mos6560_device::read(offs_t offset)
{
	update_cycles();

	offset &= 0xf;

	switch (offset)
	{
	case 3:
		return (BIT(raster_line(m_cycle_count), 0) << 7) | (m_reg[3] & 0x7f);

	case 4:
		return raster_line(m_cycle_count) >> 1;

	case 6:
		return m_lp_x;

	case 7:
		return m_lp_y;

	case 8:
		return m_read_potx(0);

	case 9:
		return m_read_poty(0);

	default:
		return m_reg[offset];
	}
}

void mos6560_device::write(offs_t offset, uint8_t data)
{
	update_cycles();

	offset &= 0xf;

	if (offset >= 0x0a && offset <= 0x0e)
		m_stream->update();

	m_reg[offset] = data;

	switch (offset)
	{
	case 3:
		if (!m_attack_ufo)
			m_char_height = BIT(data, 0) ? 16 : 8;
		break;

	case 0xe:
	case 0xf:
		m_pend_reg = offset;
		m_pend_data = data;
		break;
	}
}

uint8_t mos6560_device::bus_r()
{
	update_cycles();

	return m_vbus;
}

void mos6560_device::bus_w(uint8_t data)
{
	update_cycles();

	m_vbus = data;
}

void mos6560_device::bus_w(uint8_t data, uint8_t high)
{
	update_cycles();

	m_vbus = data;
	m_vbus_high = high & 0x0f;
}

void mos6560_device::lp_w(int state)
{
	if (!started())
	{
		m_lp_state = !state;
		return;
	}

	update_cycles();

	bool const active = !state;

	if (active && !m_lp_state)
	{
		m_lp_trigger = m_cycle_count + 1;

		int pos;
		int const line = raster_line(m_cycle_count, &pos);
		int const last = m_ntsc ? m_frame_lines : (m_frame_lines - 1);

		if ((line == last) && (pos == (last_line_cycles(line) - 2)))
			m_lp_extra = 0;
	}

	m_lp_state = active;
}


//-------------------------------------------------
//  screen_update
//-------------------------------------------------

uint32_t mos6560_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap[m_draw ^ 1], 0, 0, 0, 0, cliprect);

	return 0;
}


//-------------------------------------------------
//  sound_stream_update
//-------------------------------------------------

void mos6560_device::sound_stream_update(sound_stream &stream)
{
	static constexpr uint8_t PRESCALE[4] = { 0x0f, 0x07, 0x03, 0x01 };

	int const volume = m_reg[0x0e] & 0x0f;

	for (int i = 0; i < stream.samples(); i++)
	{
		for (int ch = 0; ch < 4; ch++)
		{
			if (m_snd_div & PRESCALE[ch])
				continue;

			uint8_t const reg = m_reg[0x0a + ch];
			bool const enabled = BIT(reg, 7);

			if (m_snd_cnt[ch] != 0x7f)
			{
				m_snd_cnt[ch]++;
				continue;
			}

			m_snd_cnt[ch] = (reg + 1) & 0x7f;

			if (ch == 3)
			{
				bool const edge = BIT(m_noise_lfsr, 0) && !m_noise_lfsr0;
				if (edge)
					m_snd_sreg[ch] = (m_snd_sreg[ch] << 1) | (!BIT(m_snd_sreg[ch], 7) && enabled);

				bool const fb = BIT(m_noise_lfsr, 3) ^ BIT(m_noise_lfsr, 12) ^ BIT(m_noise_lfsr, 14) ^ BIT(m_noise_lfsr, 15);
				m_noise_lfsr0 = BIT(m_noise_lfsr, 0);
				m_noise_lfsr = (m_noise_lfsr << 1) | !(!fb && enabled);
			}
			else
			{
				m_snd_sreg[ch] = (m_snd_sreg[ch] << 1) | (!BIT(m_snd_sreg[ch], 7) && enabled);
			}
		}

		m_snd_div += 2;

		int bits = BIT(m_snd_sreg[0], 0) + BIT(m_snd_sreg[1], 0) + BIT(m_snd_sreg[2], 0);
		if (BIT(m_reg[0x0d], 7))
			bits += BIT(m_snd_sreg[3], 0);

		stream.put(0, i, m_dac[(bits * 7 + 1) * volume]);
	}
}
