// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain UV201/UV202 video chip emulation

**********************************************************************/

#include "emu.h"
#include "uv201.h"

#include <algorithm>

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

namespace {

// screen parameters
constexpr int HTOTAL = 228;
constexpr int HBLANK_END = 33;
constexpr int HBLANK_START = 222;
constexpr int ODD_FIELD_LINES = 263;
constexpr int EVEN_FIELD_LINES = 262;
constexpr int VBLANK_LINES = 21;

// X=0 leaves the earliest first fetch (cycle 18) time to reach the FIFO even after losing the bus to a CPU access
constexpr int RENDER_START = HBLANK_END + 27;

} // anonymous namespace


// write-only registers
#define REGISTER_COMMAND            0xf7
#define REGISTER_BACKGROUND         0xf5
#define REGISTER_FINAL_MODIFIER     0xf2
#define REGISTER_Y_INTERRUPT        0xf0


// read-only registers
#define REGISTER_X_FREEZE           0xf8
#define REGISTER_Y_FREEZE_LOW       0xf9
#define REGISTER_Y_FREEZE_HIGH      0xfa
#define REGISTER_CURRENT_Y_LOW      0xfb


// read/write registers - RAM memory
#define RAM_RP_LO                   0x00    // cartridge pointer low order
#define RAM_RP_HI_COLOR             0x10    // cartridge pointer high order and color
#define RAM_DX_INT_XCOPY            0x20    // dX, intensity, X-copy
#define RAM_DY                      0x30    // dY
#define RAM_X                       0x40    // X value
#define RAM_Y_LO_A                  0x50    // Y value low order list A
#define RAM_Y_LO_B                  0x60    // Y value low order list B
#define RAM_XY_HI_A                 0x70    // Y value high order and X order list A
#define RAM_XY_HI_B                 0x80    // Y value high order and X order list B


// command register bits
#define COMMAND_YINT_H_O            0x80
#define COMMAND_A_B                 0x40
#define COMMAND_Y_ZM                0x20
#define COMMAND_KBD                 0x10
#define COMMAND_INT                 0x08
#define COMMAND_ENB                 0x04
#define COMMAND_FRZ                 0x02
#define COMMAND_X_ZM                0x01


#define IS_CHANGED(_bit) \
	((m_cmd & _bit) != (data & _bit))



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(UV201, uv201_device, "uv201", "UV201")


//-------------------------------------------------
//  uv201_device - constructor
//-------------------------------------------------

uv201_device::uv201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, UV201, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_write_ext_int(*this),
	m_write_hblank(*this),
	m_read_db(*this, 0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void uv201_device::device_start()
{
	// allocate timers
	m_timer_y_odd = timer_alloc(FUNC(uv201_device::y_update_tick), this);
	m_timer_y_even = timer_alloc(FUNC(uv201_device::y_update_tick), this);
	m_timer_hblank_on = timer_alloc(FUNC(uv201_device::hblank_on), this);
	m_timer_hblank_off = timer_alloc(FUNC(uv201_device::hblank_off), this);

	for (auto &bitmap : m_bitmap)
	{
		bitmap.allocate(HTOTAL, ODD_FIELD_LINES);
		bitmap.fill(rgb_t::black());
	}

	initialize_palette();

	memset(m_ram, 0x00, sizeof(m_ram));
	m_y_int = 0;
	m_fmod = 0;
	m_bg = 0;
	m_cmd = 0;
	m_freeze_x = 0;
	m_freeze_y = 0;
	m_ext_int = 1;
	m_vpos = 0;
	m_hpos = 0;
	m_cycle = 0;
	m_in_range = 0;
	m_height = 0;
	m_fetch_state = FETCH_IDLE;
	m_fetch_sprite = 0;
	m_fetch_byte = 0;
	m_fetch_ptr = 0;
	m_fetch_delay = 0;
	m_fetch_total = 0;
	m_fetch_start = 0;
	m_fetch_ws = 0;
	m_fetch_pending = 0;
	m_fetch_cpu_at = 0;
	m_dma_count = 0;
	m_bus_free = 0;
	m_xdelta = 0;
	std::fill(std::begin(m_fifo_count), std::end(m_fifo_count), 0);
	std::fill(std::begin(m_fifo_data), std::end(m_fifo_data), 0);
	std::fill(std::begin(m_fifo_color), std::end(m_fifo_color), 0);
	m_fifo_head = 0;
	m_fifo_level = 0;
	m_fifo_full = false;
	m_render_left = 0;
	m_render_pos = 0;
	m_render_count = 0;
	m_render_data = 0;
	m_render_color = 0;

	// state saving
	save_item(NAME(m_ram));
	save_item(NAME(m_y_int));
	save_item(NAME(m_fmod));
	save_item(NAME(m_bg));
	save_item(NAME(m_cmd));
	save_item(NAME(m_freeze_x));
	save_item(NAME(m_freeze_y));
	save_item(NAME(m_ext_int));
	save_item(NAME(m_vpos));
	save_item(NAME(m_hpos));
	save_item(NAME(m_cycle));
	save_item(NAME(m_in_range));
	save_item(NAME(m_height));
	save_item(NAME(m_fetch_state));
	save_item(NAME(m_fetch_sprite));
	save_item(NAME(m_fetch_byte));
	save_item(NAME(m_fetch_ptr));
	save_item(NAME(m_fetch_delay));
	save_item(NAME(m_fetch_total));
	save_item(NAME(m_fetch_start));
	save_item(NAME(m_fetch_ws));
	save_item(NAME(m_fetch_pending));
	save_item(NAME(m_fetch_cpu_at));
	save_item(NAME(m_dma_count));
	save_item(NAME(m_bus_free));
	save_item(NAME(m_xdelta));
	save_item(NAME(m_fifo_count));
	save_item(NAME(m_fifo_data));
	save_item(NAME(m_fifo_color));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_level));
	save_item(NAME(m_fifo_full));
	save_item(NAME(m_render_left));
	save_item(NAME(m_render_pos));
	save_item(NAME(m_render_count));
	save_item(NAME(m_render_data));
	save_item(NAME(m_render_color));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void uv201_device::device_reset()
{
	update();
	reset_state();

	m_vpos = screen().vpos();
	m_hpos = screen().hpos();
	m_cycle = machine().time().as_ticks(clock());

	int vpos = m_vpos;

	if (m_hpos >= HBLANK_START)
		vpos = (vpos + 1) % screen().height();

	m_timer_hblank_on->adjust(screen().time_until_pos(vpos, HBLANK_START), vpos);
}


//-------------------------------------------------
//  reset_state - state cleared by the RESET pin
//-------------------------------------------------

void uv201_device::reset_state()
{
	m_write_ext_int(CLEAR_LINE);

	m_in_range = 0;
	m_fetch_state = FETCH_IDLE;
	fifo_clear();
}


//-------------------------------------------------
//  reset_w - RESET pin
//-------------------------------------------------

void uv201_device::reset_w(int state)
{
	if (state)
	{
		update();
		reset_state();
	}
}


//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(uv201_device::y_update_tick)
{
	if ((m_cmd & COMMAND_INT) && !(m_cmd & COMMAND_FRZ))
	{
		int scanline = get_field_vpos();
		LOG("Y-Interrupt at scanline %u\n", scanline);

		m_freeze_y = scanline;

		m_write_ext_int(ASSERT_LINE);
		m_write_ext_int(CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(uv201_device::hblank_on)
{
	update();

	m_write_hblank(1);

	int vpos = (param + 1) % screen().height();
	m_timer_hblank_off->adjust(screen().time_until_pos(vpos, HBLANK_END), vpos);
}

TIMER_CALLBACK_MEMBER(uv201_device::hblank_off)
{
	update();

	m_write_hblank(0);
	m_timer_hblank_on->adjust(screen().time_until_pos(param, HBLANK_START), param);
}


//-------------------------------------------------
//  initialize_palette -
//-------------------------------------------------

void uv201_device::initialize_palette()
{
	// index bits: 0 = red, 1 = green, 2 = blue, 3 = intensity
	for (int i = 0; i < 16; i++)
	{
		uint8_t const off = BIT(i, 3) ? 0x55 : 0x00;
		uint8_t const on = BIT(i, 3) ? 0xff : 0xaa;

		m_palette_val[i] = rgb_t(BIT(i, 0) ? on : off, BIT(i, 1) ? on : off, BIT(i, 2) ? on : off);
	}
}


//-------------------------------------------------
//  get_field_vpos - get scanline within field
//-------------------------------------------------

int uv201_device::get_field_vpos()
{
	int vpos = screen().vpos();

	return (vpos < ODD_FIELD_LINES) ? vpos : (vpos - ODD_FIELD_LINES);
}


//-------------------------------------------------
//  get_field - get video field
//-------------------------------------------------

int uv201_device::get_field()
{
	return screen().vpos() < ODD_FIELD_LINES;
}


//-------------------------------------------------
//  set_y_interrupt - set Y interrupt timer
//-------------------------------------------------

void uv201_device::set_y_interrupt()
{
	int scanline = ((m_cmd & COMMAND_YINT_H_O) << 1) | m_y_int;

	if (scanline < ODD_FIELD_LINES)
		m_timer_y_odd->adjust(screen().time_until_pos(scanline), 0, screen().frame_period());
	else
		m_timer_y_odd->adjust(attotime::never);

	if (scanline < EVEN_FIELD_LINES)
		m_timer_y_even->adjust(screen().time_until_pos(ODD_FIELD_LINES + scanline), 0, screen().frame_period());
	else
		m_timer_y_even->adjust(attotime::never);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t uv201_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	if (!machine().side_effects_disabled())
		update();

	switch (offset)
	{
	case REGISTER_X_FREEZE:
		data = m_freeze_x;

		LOG("X-Freeze %02x\n", data);
		break;

	case REGISTER_Y_FREEZE_LOW:
		data = m_freeze_y & 0xff;

		LOG("Y-Freeze Low %02x\n", data);
		break;

	case REGISTER_Y_FREEZE_HIGH:
		/*

		    bit     signal      description

		    0       Y-F8        Y freeze high order (MSB) bit
		    1       Y-C8        current Y counter high order (MSB) bit
		    2
		    3
		    4
		    5
		    6
		    7       O/_E        odd/even field

		*/

		data = (get_field() << 7) | (BIT(get_field_vpos(), 8) << 1) | BIT(m_freeze_y, 8);

		LOG("Y-Freeze High %02x\n", data);
		break;

	case REGISTER_CURRENT_Y_LOW:
		data = get_field_vpos() & 0xff;

		LOG("Current-Y Low %02x\n", data);
		break;

	default:
		if (offset < 0x90)
			data = m_ram[offset];
		else
			LOG("Unknown VLSI read from %02x!\n", offset);
		break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void uv201_device::write(offs_t offset, uint8_t data)
{
	update();

	switch (offset)
	{
	case REGISTER_Y_INTERRUPT:
		LOG("Y-Interrupt %02x\n", data);

		if (m_y_int != data)
		{
			m_y_int = data;
			set_y_interrupt();
		}
		break;

	case REGISTER_FINAL_MODIFIER:
		/*

		    bit     signal      description

		    0       RED         red
		    1       GREEN       green
		    2       BLUE        blue
		    3       INT 0       intensity 0 (no output pin)
		    4       INT 1       intensity 1
		    5       not used
		    6       not used
		    7       not used

		*/

		LOG("Final Modifier %02x\n", data);

		m_fmod = (data & 0x07) | (BIT(data, 4) << 3);
		break;

	case REGISTER_BACKGROUND:
		/*

		    bit     signal      description

		    0       RED         red
		    1       GREEN       green
		    2       BLUE        blue
		    3       INT 0       intensity 0 (no output pin)
		    4       INT 1       intensity 1
		    5       not used
		    6       not used
		    7       not used

		*/

		LOG("Background %02x\n", data);

		m_bg = (data & 0x07) | (BIT(data, 4) << 3);
		break;

	case REGISTER_COMMAND:
		/*

		    bit     signal      description

		    0       X-ZM        X zoom
		    1       FRZ         freeze
		    2       ENB         video enable
		    3       INT         interrupt enable
		    4       KBD         general purpose output
		    5       Y-ZM        Y zoom
		    6       A/_B        list selection
		    7       YINT H.O.   Y COMMAND_INT register high order bit

		*/

		LOG("Command %02x\n", data);

		if (IS_CHANGED(COMMAND_YINT_H_O))
		{
			set_y_interrupt();
		}

		m_cmd = data;
		break;

	default:
		if (offset < 0x90)
			m_ram[offset] = data;
		else
			logerror("Unknown VLSI write %02x to %02x!\n", data, offset);
		break;
	}
}


//-------------------------------------------------
//  bus_request - CPU buffered bus cycle, returns
//  the number of BRCLKs the CPU is held
//-------------------------------------------------

int uv201_device::bus_request(bool registers)
{
	update();

	// register accesses are stretched until the free running divide by 4 wraps
	auto const stretch = [registers] (int hpos) { return registers ? (1 + 3 - ((hpos + 1) & 3)) : 0; };

	if ((m_fetch_state == FETCH_DMA) && (m_fetch_delay || !m_fifo_full))
	{
		// the CPU gets the bus after at least 2 DMA reads, replacing the next one
		int const left = sprite_width(m_fetch_sprite) - m_fetch_byte;
		int const reads = std::max(1, 2 - m_dma_count);
		int const remain = (m_fetch_delay + 1) + 3 * (std::min(reads, left) - 1);

		if (reads < left)
		{
			int const length = 3 + stretch(m_hpos + remain);

			m_fetch_cpu_at = reads;
			m_fetch_pending = length + 1;
			m_fetch_ws += length + 1;

			return remain + length;
		}

		int const length = 1 + 3 + stretch(m_hpos + remain + 1);

		m_bus_free = std::max(m_bus_free, m_cycle + remain + length);

		return remain + length;
	}

	int const length = 1 + 3 + stretch(m_hpos + 1);

	m_bus_free = std::max(m_bus_free, m_cycle + length);

	return length;
}


//-------------------------------------------------
//  ext_int_w - external interrupt write
//-------------------------------------------------

void uv201_device::ext_int_w(int state)
{
	bool const falling = m_ext_int && !state;

	m_ext_int = state;

	if (falling && (m_cmd & COMMAND_FRZ))
	{
		m_freeze_y = get_field_vpos();
		m_freeze_x = screen().hpos();
	}
}


//-------------------------------------------------
//  kbd_r - keyboard select read
//-------------------------------------------------

int uv201_device::kbd_r()
{
	return (m_cmd & COMMAND_KBD) ? 1 : 0;
}


//-------------------------------------------------
//  sprite field accessors
//-------------------------------------------------

int uv201_device::sprite_bank(int i) const
{
	return m_ram[((m_cmd & COMMAND_A_B) ? RAM_XY_HI_A : RAM_XY_HI_B) + i] & 0x0f;
}

int uv201_device::sprite_y(int i) const
{
	uint8_t const y_lo = m_ram[((m_cmd & COMMAND_A_B) ? RAM_Y_LO_A : RAM_Y_LO_B) + i];
	uint8_t const xy_hi = m_ram[((m_cmd & COMMAND_A_B) ? RAM_XY_HI_A : RAM_XY_HI_B) + i];

	return (BIT(xy_hi, 7) << 8) | y_lo;
}

int uv201_device::sprite_width(int i) const
{
	int const width = m_ram[RAM_DX_INT_XCOPY + sprite_bank(i)] & 0x1f;

	return width ? width : 32;
}

int uv201_device::next_sprite(int i) const
{
	while ((i < 16) && !BIT(m_in_range, i))
		i++;

	return i;
}


//-------------------------------------------------
//  fifo
//-------------------------------------------------

void uv201_device::fifo_clear()
{
	m_fifo_head = 0;
	m_fifo_level = 0;
	m_fifo_full = false;
	m_render_left = 0;
	m_render_pos = 0;
}

void uv201_device::fifo_push(uint16_t count, uint8_t data, uint8_t color)
{
	int const index = (m_fifo_head + m_fifo_level) % FIFO_DEPTH;

	m_fifo_count[index] = count;
	m_fifo_data[index] = data;
	m_fifo_color[index] = color;

	if (++m_fifo_level == FIFO_DEPTH)
		m_fifo_full = true;
}

bool uv201_device::fifo_pop()
{
	if (!m_fifo_level)
		return false;

	m_render_count = m_fifo_count[m_fifo_head];
	m_render_data = m_fifo_data[m_fifo_head];
	m_render_color = m_fifo_color[m_fifo_head];

	m_fifo_head = (m_fifo_head + 1) % FIFO_DEPTH;

	if (--m_fifo_level <= FIFO_DEPTH - 2)
		m_fifo_full = false;

	return true;
}


//-------------------------------------------------
//  fetch_start - start fetching a scanline
//-------------------------------------------------

void uv201_device::fetch_start()
{
	m_fetch_sprite = next_sprite(0);

	if (m_fetch_sprite == 16)
		return;

	int const bank = sprite_bank(m_fetch_sprite);
	bool const gap = m_ram[RAM_X + bank] > m_xdelta;
	int cycles = (m_fetch_sprite & 7) + 17;

	if (cycles & 1)
		cycles++;
	if (gap)
		cycles += 2;

	m_fetch_total = cycles;

	if ((sprite_width(m_fetch_sprite) == 1) && !gap)
		m_fetch_total += 2;

	m_fetch_state = FETCH_DELAY;
	m_fetch_delay = cycles;
}


//-------------------------------------------------
//  fetch_next - schedule the next sprite fetch
//-------------------------------------------------

void uv201_device::fetch_next(int width)
{
	m_fetch_sprite = next_sprite(m_fetch_sprite + 1);

	if (m_fetch_sprite == 16)
	{
		m_fetch_state = FETCH_IDLE;
		return;
	}

	int cycles = (3 * width) + m_fetch_ws + 15;

	if (cycles & 1)
		cycles++;

	m_fetch_total += cycles;

	if (m_fetch_total == 46)
		m_fetch_total += 2;

	int const offset = (m_fetch_total > 48) ? (m_fetch_total & 2) : 0;

	cycles += offset;
	m_fetch_total += offset;

	if (m_ram[RAM_X + sprite_bank(m_fetch_sprite)] > m_xdelta)
	{
		cycles += 2;
		m_fetch_total += 2;
	}

	// the delay counts from the start of the previous fetch
	m_fetch_state = FETCH_DELAY;
	m_fetch_delay = std::max<int64_t>(0, int64_t(m_fetch_start + cycles - m_cycle) - 1);
}


//-------------------------------------------------
//  fetch_end - end of sprite fetch
//-------------------------------------------------

void uv201_device::fetch_end(int y)
{
	int const i = m_fetch_sprite;
	int const bank = sprite_bank(i);

	if (BIT(y, 0) ^ BIT(m_height, i) ^ BIT(m_cmd, 5))
	{
		m_ram[RAM_RP_LO + bank] = m_fetch_ptr & 0xff;
		m_ram[RAM_RP_HI_COLOR + bank] = (m_ram[RAM_RP_HI_COLOR + bank] & 0xe0) | ((m_fetch_ptr >> 8) & 0x1f);

		m_height = (m_height & ~(1 << i)) | (BIT(y, 0) << i);

		uint8_t const height = (m_ram[RAM_DY + bank] - 1) & 0x3f;
		m_ram[RAM_DY + bank] = (m_ram[RAM_DY + bank] & 0xc0) | height;

		if (!height)
			m_in_range &= ~(1 << i);
	}
}


//-------------------------------------------------
//  fetch_clock - advance the fetcher one cycle
//-------------------------------------------------

void uv201_device::fetch_clock(int y)
{
	switch (m_fetch_state)
	{
	case FETCH_IDLE:
		break;

	case FETCH_DELAY:
		// the background gap is computed internally and does not need the bus
		if ((m_fetch_delay <= 2) && !m_fifo_full)
		{
			uint8_t const x = m_ram[RAM_X + sprite_bank(m_fetch_sprite)];

			if (x > m_xdelta)
			{
				fifo_push(x, 0, 0xff);
				m_xdelta = x;
			}
		}

		if (m_fetch_delay)
		{
			m_fetch_delay--;
			break;
		}

		if (m_fifo_full || (m_cycle < m_bus_free))
			break;

		{
			int const bank = sprite_bank(m_fetch_sprite);
			uint8_t const x = m_ram[RAM_X + bank];

			if (x > m_xdelta)
			{
				fifo_push(x, 0, 0xff);
				m_xdelta = x;
			}

			m_fetch_ptr = ((m_ram[RAM_RP_HI_COLOR + bank] << 8) | m_ram[RAM_RP_LO + bank]) & 0x1fff;
			m_fetch_byte = 0;
			m_fetch_delay = 2;
			m_fetch_start = m_cycle;
			m_fetch_ws = 0;
			m_fetch_pending = 0;
			m_fetch_cpu_at = 0;
			m_dma_count = 2;
			m_fetch_state = FETCH_DMA;
		}
		break;

	case FETCH_DMA:
		if (m_fetch_delay)
		{
			m_fetch_delay--;
			break;
		}

		if (m_fifo_full || (m_cycle < m_bus_free))
			break;

		{
			int const i = m_fetch_sprite;
			int const bank = sprite_bank(i);
			int const width = sprite_width(i);
			uint8_t const rp_hi_color = m_ram[RAM_RP_HI_COLOR + bank];
			uint8_t const dx_int_xcopy = m_ram[RAM_DX_INT_XCOPY + bank];
			uint8_t const color = (BIT(dx_int_xcopy, 6) << 3) | (BIT(rp_hi_color, 5) << 2) | (BIT(rp_hi_color, 6) << 1) | BIT(rp_hi_color, 7);
			bool const last = (++m_fetch_byte == width);

			fifo_push(8, m_read_db(m_fetch_ptr), color);
			m_xdelta += 8;

			if (!BIT(dx_int_xcopy, 7) || last)
				m_fetch_ptr = (m_fetch_ptr + 1) & 0x1fff;

			if (last)
			{
				fetch_end(y);
				fetch_next(width);
			}
			else
			{
				m_fetch_delay = 2;
				m_dma_count++;

				if (m_fetch_cpu_at && !--m_fetch_cpu_at)
				{
					m_fetch_delay += m_fetch_pending;
					m_dma_count = 0;
				}
			}
		}
		break;
	}
}


//-------------------------------------------------
//  render_clock - output one pixel
//-------------------------------------------------

uint8_t uv201_device::render_clock()
{
	int const zoom = (m_cmd & COMMAND_X_ZM) ? 2 : 1;

	while (!m_render_left)
	{
		if (!fifo_pop())
		{
			m_render_pos++;
			return m_bg;
		}

		// background runs end at an absolute position, so pixels sent while the FIFO was empty count towards them
		if (m_render_color == 0xff)
			m_render_left = std::max(0, (m_render_count * zoom) - m_render_pos);
		else
			m_render_left = 8 * zoom;
	}

	uint8_t pixel = m_bg;

	if (m_render_color != 0xff)
	{
		int const bit = 7 - ((8 * zoom) - m_render_left) / zoom;

		if (BIT(m_render_data, bit))
			pixel = m_render_color;
	}

	m_render_left--;
	m_render_pos++;

	return pixel;
}


//-------------------------------------------------
//  update - run the video chip up to the beam
//-------------------------------------------------

void uv201_device::update()
{
	uint64_t const cycle = machine().time().as_ticks(clock());

	for (; m_cycle < cycle; m_cycle++)
	{
		int const field = (m_vpos < ODD_FIELD_LINES) ? 0 : 1;
		int const y = field ? (m_vpos - ODD_FIELD_LINES) : m_vpos;
		bool const vblank = y >= ((field ? EVEN_FIELD_LINES : ODD_FIELD_LINES) - VBLANK_LINES);

		if (m_hpos == HBLANK_START)
		{
			fifo_clear();
			m_xdelta = 0;

			int const next_vpos = (m_vpos + 1) % screen().height();
			int const next_y = (next_vpos < ODD_FIELD_LINES) ? next_vpos : (next_vpos - ODD_FIELD_LINES);

			if (!next_y)
				m_in_range = 0;

			for (int i = 0; i < 16; i++)
			{
				if (sprite_y(i) == next_y)
				{
					m_in_range |= 1 << i;
					m_height = (m_height & ~(1 << i)) | (BIT(y, 0) << i);
				}
			}
		}
		else if (m_hpos == HBLANK_END)
		{
			if (vblank)
				m_fetch_state = FETCH_IDLE;
			else if ((m_fetch_state == FETCH_IDLE) && (m_cmd & COMMAND_ENB))
				fetch_start();
		}

		fetch_clock(y);

		if (!vblank && (m_hpos >= HBLANK_END) && (m_hpos < HBLANK_START))
		{
			uint8_t const pixel = (m_hpos >= RENDER_START) ? render_clock() : m_bg;

			m_bitmap[field].pix(y, m_hpos) = (m_cmd & COMMAND_ENB) ? m_palette_val[pixel ^ m_fmod] : rgb_t::black();
		}
		else
		{
			m_bitmap[field].pix(y, m_hpos) = rgb_t::black();
		}

		if (++m_hpos == HTOTAL)
		{
			m_hpos = 0;
			m_vpos = (m_vpos + 1) % screen().height();
		}
	}
}


//-------------------------------------------------
//  screen_update - weave both fields
//-------------------------------------------------

uint32_t uv201_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	update();

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		bitmap_rgb32 const &field = m_bitmap[y & 1];

		std::copy_n(&field.pix(y >> 1, cliprect.min_x), cliprect.width(), &bitmap.pix(y, cliprect.min_x));
	}

	return 0;
}
