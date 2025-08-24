// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6S46 family

E0C6S46: 6144x12 ROM, 640x4 RAM, 2*80x4 VRAM, LCD has 16 commons and 40 segments
E0C6S48: 8192x12 ROM, 768x4 RAM, 2*102x4 VRAM, LCD has 16 commons and 51 segments

TODO:
- finish i/o ports
- serial interface
- what happens if OSC3 is selected while OSCC (bit 2) is low?
- add mask options for ports (eg. buzzer on output port R4x is optional)

*/

#include "emu.h"
#include "e0c6s46.h"

enum
{
	IRQREG_CLKTIMER = 0,
	IRQREG_STOPWATCH,
	IRQREG_PRGTIMER,
	IRQREG_SERIAL,
	IRQREG_INPUT0,
	IRQREG_INPUT1
};

// device definitions
DEFINE_DEVICE_TYPE(E0C6S46, e0c6s46_device, "e0c6s46", "Seiko Epson E0C6S46")
DEFINE_DEVICE_TYPE(E0C6S48, e0c6s48_device, "e0c6s48", "Seiko Epson E0C6S48")


// internal memory maps
void e0c6s46_device::program_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();
}

void e0c6s48_device::program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void e0c6s46_device::data_map(address_map &map)
{
	map(0x0000, 0x027f).ram();
	map(0x0e00, 0x0e4f).ram().share(m_vram[0]);
	map(0x0e80, 0x0ecf).ram().share(m_vram[1]);
	map(0x0f00, 0x0f7f).rw(FUNC(e0c6s46_device::io_r), FUNC(e0c6s46_device::io_w));
}

void e0c6s48_device::data_map(address_map &map)
{
	map(0x0000, 0x02ff).ram();
	map(0x0e00, 0x0e65).ram().share(m_vram[0]);
	map(0x0e80, 0x0ee5).ram().share(m_vram[1]);
	map(0x0f00, 0x0f7f).rw(FUNC(e0c6s48_device::io_r), FUNC(e0c6s48_device::io_w));
}


// constructor
e0c6s46_device::e0c6s46_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor program, address_map_constructor data) :
	e0c6200_cpu_device(mconfig, type, tag, owner, clock, program, data),
	m_vram(*this, "vram%u", 1U),
	m_write_segs(*this),
	m_write_contrast(*this),
	m_pixel_cb(*this),
	m_write_r(*this),
	m_read_p(*this, 0),
	m_write_p(*this),
	m_osc3(0)
{ }

e0c6s46_device::e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	e0c6s46_device(mconfig, E0C6S46, tag, owner, clock, address_map_constructor(FUNC(e0c6s46_device::program_map), this), address_map_constructor(FUNC(e0c6s46_device::data_map), this))
{ }

e0c6s48_device::e0c6s48_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	e0c6s46_device(mconfig, E0C6S48, tag, owner, clock, address_map_constructor(FUNC(e0c6s48_device::program_map), this), address_map_constructor(FUNC(e0c6s48_device::data_map), this))
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e0c6s46_device::device_start()
{
	e0c6200_cpu_device::device_start();

	// misc init
	m_osc1 = clock();
	if (m_osc3 == 0)
		m_osc3 = m_osc1;

	m_core_256_handle = timer_alloc(FUNC(e0c6s46_device::core_256_cb), this);
	m_core_256_handle->adjust(attotime::from_ticks(64, m_osc1));
	m_prgtimer_handle = timer_alloc(FUNC(e0c6s46_device::prgtimer_cb), this);
	m_buzzer_handle = timer_alloc(FUNC(e0c6s46_device::buzzer_cb), this);
	m_osc_change = timer_alloc(FUNC(e0c6s46_device::osc_change), this);
	m_lcd_driver = timer_alloc(FUNC(e0c6s46_device::lcd_driver_cb), this);
	m_lcd_driver->adjust(attotime::from_ticks(1024, m_osc1));

	m_pixel_cb.resolve();

	const u32 render_buf_size = m_vram[0].bytes() * 2 * 4;
	m_render_buf = make_unique_clear<u8[]>(render_buf_size);
	save_pointer(NAME(m_render_buf), render_buf_size);

	// zerofill
	memset(m_port_r, 0x0, sizeof(m_port_r));
	m_r_dir = 0;
	memset(m_port_p, 0x0, sizeof(m_port_p));
	m_p_dir = 0;
	m_p_pullup = 0;
	memset(m_port_k, 0xf, sizeof(m_port_k));
	m_dfk0 = 0xf;

	memset(m_irqflag, 0, sizeof(m_irqflag));
	memset(m_irqmask, 0, sizeof(m_irqmask));
	m_osc = 0;
	m_svd = 0;
	m_lcd_control = 0;
	m_lcd_contrast = 0;

	m_256_src_pulse = 0;
	m_watchdog_count = 0;
	m_clktimer_count = 0;

	m_stopwatch_on = 0;
	m_swl_cur_pulse = 0;
	m_swl_slice = 0;
	m_swl_count = 0;
	m_swh_count = 0;

	m_prgtimer_select = 0;
	m_prgtimer_on = 0;
	m_prgtimer_src_pulse = 0;
	m_prgtimer_cur_pulse = 0;
	m_prgtimer_count = 0;
	m_prgtimer_reload = 0;

	m_bz_43_on = 0;
	m_bz_freq = 0;
	m_bz_envelope = 0;
	m_bz_envelope_count = 0;
	m_bz_duty_ratio = 0;
	m_bz_1shot_on = 0;
	m_bz_1shot_running = false;
	m_bz_1shot_count = 0;
	m_bz_pulse = 0;

	// register for savestates
	save_item(NAME(m_port_r));
	save_item(NAME(m_r_dir));
	save_item(NAME(m_port_p));
	save_item(NAME(m_p_dir));
	save_item(NAME(m_p_pullup));
	save_item(NAME(m_port_k));
	save_item(NAME(m_dfk0));

	save_item(NAME(m_irqflag));
	save_item(NAME(m_irqmask));
	save_item(NAME(m_osc));
	save_item(NAME(m_svd));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_lcd_contrast));

	save_item(NAME(m_256_src_pulse));
	save_item(NAME(m_watchdog_count));
	save_item(NAME(m_clktimer_count));

	save_item(NAME(m_stopwatch_on));
	save_item(NAME(m_swl_cur_pulse));
	save_item(NAME(m_swl_slice));
	save_item(NAME(m_swl_count));
	save_item(NAME(m_swh_count));

	save_item(NAME(m_prgtimer_select));
	save_item(NAME(m_prgtimer_on));
	save_item(NAME(m_prgtimer_src_pulse));
	save_item(NAME(m_prgtimer_cur_pulse));
	save_item(NAME(m_prgtimer_count));
	save_item(NAME(m_prgtimer_reload));

	save_item(NAME(m_bz_43_on));
	save_item(NAME(m_bz_freq));
	save_item(NAME(m_bz_envelope));
	save_item(NAME(m_bz_envelope_count));
	save_item(NAME(m_bz_duty_ratio));
	save_item(NAME(m_bz_1shot_on));
	save_item(NAME(m_bz_1shot_running));
	save_item(NAME(m_bz_1shot_count));
	save_item(NAME(m_bz_pulse));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void e0c6s46_device::device_reset()
{
	e0c6200_cpu_device::device_reset();

	// reset interrupts
	memset(m_irqflag, 0, sizeof(m_irqflag));
	memset(m_irqmask, 0, sizeof(m_irqmask));

	// reset other I/O
	m_data->write_byte(0xf41, 0xf);
	m_data->write_byte(0xf54, 0xf);
	m_data->write_byte(0xf70, 0x0);
	m_data->write_byte(0xf71, 0x8);
	m_data->write_byte(0xf73, m_svd & 3);

	m_data->write_byte(0xf74, 0x0);
	m_data->write_byte(0xf75, 0x4);
	m_data->write_byte(0xf76, 0x3);
	m_data->write_byte(0xf77, 0x2);
	m_data->write_byte(0xf78, 0x2);
	m_data->write_byte(0xf79, 0x0);
	m_data->write_byte(0xf7a, 0x0);
	m_data->write_byte(0xf7b, 0x0);
	m_data->write_byte(0xf7d, 0x0);
	m_data->write_byte(0xf7e, 0x0);

	m_write_contrast(m_lcd_contrast);

	// reset ports
	for (int i = 0; i < 5; i++)
		write_r(i, m_port_r[i]);
	for (int i = 0; i < 4; i++)
		write_p(i, m_port_p[i]);
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void e0c6s46_device::execute_one()
{
	// E0C6S46 has no support for SLP opcode
	if (m_op == 0xff9)
		op_illegal();
	else
		e0c6200_cpu_device::execute_one();
}



//-------------------------------------------------
//  interrupts
//-------------------------------------------------

bool e0c6s46_device::check_interrupt()
{
	// priority order is not the same as register order
	static const int priorder[6] =
	{
		IRQREG_CLKTIMER,
		IRQREG_STOPWATCH,
		IRQREG_INPUT0,
		IRQREG_INPUT1,
		IRQREG_SERIAL,
		IRQREG_PRGTIMER,
	};

	// check interrupts from high to low priority
	for (int pri = 5; pri >= 0; pri--)
	{
		// hw glitch note, not emulated: if a new interrupt is requested in the
		// middle of handling this interrupt, IRQ vector may be an OR of 2 vectors
		m_irq_vector = 2 * pri + 2;
		int reg = priorder[pri];
		m_irq_id = reg;

		switch (reg)
		{
			// IK0/IK1 only have 1 flag bit
			case IRQREG_INPUT0: case IRQREG_INPUT1:
				if (m_irqflag[reg])
					return true;
				break;

			// other: mask vs flag
			default:
				if (m_irqflag[reg] & m_irqmask[reg])
					return true;
				break;
		}
	}

	return false;
}

void e0c6s46_device::execute_set_input(int line, int state)
{
	// only support 8 K input lines at the moment
	if (line < 0 || line > 7)
		return;

	state = (state) ? 1 : 0;
	u8 port = line >> 2 & 1;
	u8 bit = 1 << (line & 3);

	u8 prev = m_port_k[port];
	m_port_k[port] = (m_port_k[port] & ~bit) | (state ? bit : 0);

	// set IRQ flag on falling/rising edge of input
	u8 dfk = (port == 0) ? m_dfk0 : 0xf;
	u8 edge = ~(prev ^ dfk) & (m_port_k[port] ^ dfk) & bit;
	if (m_irqmask[IRQREG_INPUT0 + port] & edge)
	{
		m_irqflag[IRQREG_INPUT0 + port] |= 1;
		m_possible_irq = true;
	}

	// clock programmable timer on falling edge of K03
	if ((prev & ~m_port_k[port] & bit) && m_prgtimer_on && (m_prgtimer_select & 7) < 2)
		clock_prgtimer();
}



//-------------------------------------------------
//  ports
//-------------------------------------------------

// R output ports

void e0c6s46_device::write_r(u8 port, u8 data)
{
	data &= 0xf;
	m_port_r[port] = data;

	if (port < 4)
	{
		// ports R0-R3 can be high-impedance
		u8 mask = BIT(m_r_dir, port) ? 0xf : 0;
		m_write_r[port](port, data | (mask ^ 0xf), mask);
	}
	else
	{
		// R43: buzzer on: direct output or 1-shot output
		if ((data & 8) != m_bz_43_on)
		{
			m_bz_43_on = data & 8;
			reset_bz_envelope();
			reset_buzzer();
		}
		write_r4_out();
	}
}

void e0c6s46_device::write_r4_out()
{
	// R40: _FOUT(clock inverted output)
	// R42: FOUT or _BZ
	// R43: BZ(buzzer)
	u8 data = (m_port_r[4] & 2) | (m_bz_pulse << 3) | (m_bz_pulse << 2 ^ 4);
	m_write_r[4](4, data, 0xf);
}


// P I/O ports

void e0c6s46_device::write_p(u8 port, u8 data)
{
	data &= 0xf;
	m_port_p[port] = data;

	// high-impedance if port is set to input
	u8 mask = BIT(m_p_dir, port) ? 0xf : 0;
	m_write_p[port](port, data | (mask ^ 0xf), mask);
}

u8 e0c6s46_device::read_p(u8 port)
{
	// return written value if port direction is set to output
	if (BIT(m_p_dir, port))
		return m_port_p[port];

	return m_read_p[port](port);
}



//-------------------------------------------------
//  timers
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(e0c6s46_device::core_256_cb)
{
	// clock-timer, stopwatch timer, and some features of the buzzer all run
	// from the same internal 256hz timer (64 ticks high+low at default clock of 32768hz)
	m_256_src_pulse ^= 1;
	m_core_256_handle->adjust(attotime::from_ticks(64, m_osc1));

	// clock stopwatch on falling edge of pulse+on
	m_swl_cur_pulse = m_256_src_pulse | (m_stopwatch_on ^ 1);
	if (m_swl_cur_pulse == 0)
		clock_stopwatch();

	// clock 1-shot buzzer on rising edge if it's on
	if (m_bz_1shot_on != 0 && m_256_src_pulse == 1)
		clock_bz_1shot();

	// clock buzzer envelope on rising edge if it's on
	if (m_bz_envelope & 1 && m_256_src_pulse == 1)
		clock_bz_envelope();

	// clock-timer is always running, advance it on falling edge
	// (handle clock_clktimer last in case of watchdog reset)
	if (m_256_src_pulse == 0)
		clock_clktimer();
}

TIMER_CALLBACK_MEMBER(e0c6s46_device::osc_change)
{
	// set MCU instruction clock on CLKCHG change
	set_clock((m_osc & 8) ? m_osc3 : m_osc1);
}


// clock-timer

void e0c6s46_device::clock_watchdog()
{
	// initial reset after 3 to 4 seconds
	if (++m_watchdog_count == 4)
	{
		logerror("watchdog reset\n");
		m_watchdog_count = 0;
		device_reset();
	}
}

void e0c6s46_device::clock_clktimer()
{
	m_clktimer_count++;

	// IRQ on falling edge of 32, 8, 2, 1hz
	u8 flag = 0;
	if ((m_clktimer_count & 0x07) == 0)
		flag |= 1;
	if ((m_clktimer_count & 0x1f) == 0)
		flag |= 2;
	if ((m_clktimer_count & 0x7f) == 0)
		flag |= 4;
	if (m_clktimer_count == 0)
		flag |= 8;

	m_irqflag[IRQREG_CLKTIMER] |= flag;
	if (m_irqflag[IRQREG_CLKTIMER] & m_irqmask[IRQREG_CLKTIMER])
		m_possible_irq = true;

	// 1hz falling edge also clocks the watchdog timer
	if (m_clktimer_count == 0)
		clock_watchdog();
}


// stopwatch timer

void e0c6s46_device::clock_stopwatch()
{
	m_swl_slice++;

	// 1 slice is 3 ticks(256hz) on even and 2 ticks on uneven counts,
	// but from count 1 to 2 it's 3 ticks, 6 out of 100 times, to make
	// exactly 26/256hz * 6 + 25/256hz * 4 = 1 second
	int swl_next = 3 - (m_swl_count & 1);
	if (m_swl_count == 1 && !(m_swh_count >> 1 & 1))
		swl_next = 3;

	if (m_swl_slice == swl_next)
	{
		m_swl_slice = 0;

		// bcd counter, IRQ on falling edge of 10 and 1hz
		m_swl_count = (m_swl_count + 1) % 10;
		if (m_swl_count == 0)
		{
			m_irqflag[IRQREG_STOPWATCH] |= 1;
			m_swh_count = (m_swh_count + 1) % 10;
			if (m_swh_count == 0)
				m_irqflag[IRQREG_STOPWATCH] |= 2;

			if (m_irqflag[IRQREG_STOPWATCH] & m_irqmask[IRQREG_STOPWATCH])
				m_possible_irq = true;
		}
	}
}


// programmable timer

void e0c6s46_device::clock_prgtimer()
{
	// IRQ and reload when it reaches zero
	if (--m_prgtimer_count == 0)
	{
		m_irqflag[IRQREG_PRGTIMER] |= 1;
		if (m_irqflag[IRQREG_PRGTIMER] & m_irqmask[IRQREG_PRGTIMER])
			m_possible_irq = true;

		// note: a reload of 0 indicates a 256-counter
		m_prgtimer_count = m_prgtimer_reload;
	}
}

bool e0c6s46_device::prgtimer_reset_prescaler()
{
	// only 2 to 7 are clock dividers
	u8 sel = m_prgtimer_select & 7;
	if (sel >= 2)
		m_prgtimer_handle->adjust(attotime::from_ticks(2 << (sel ^ 7), m_osc1));

	return (sel >= 2);
}

TIMER_CALLBACK_MEMBER(e0c6s46_device::prgtimer_cb)
{
	// check if it's clocked by osc1, schedule next timeout
	if (!prgtimer_reset_prescaler())
		return;

	m_prgtimer_src_pulse ^= 1;
	m_prgtimer_cur_pulse = m_prgtimer_src_pulse | (m_prgtimer_on ^ 1);

	// clock prgtimer on falling edge of pulse+on
	if (m_prgtimer_cur_pulse == 0)
		clock_prgtimer();
}


// buzzer

void e0c6s46_device::schedule_buzzer()
{
	// only schedule next buzzer timeout if it's on
	if (m_bz_43_on && !m_bz_1shot_running)
		return;

	// pulse width differs per frequency selection
	int mul = (m_bz_freq & 4) ? 1 : 2;
	int high = (m_bz_freq & 2) ? 12 : 8;
	int low = 16 + (m_bz_freq << 2 & 0xc);

	// pulse width envelope if it's on
	if (m_bz_envelope & 1)
		high -= m_bz_duty_ratio;
	low -= high;

	m_buzzer_handle->adjust(attotime::from_ticks(m_bz_pulse ? high : low, mul * m_osc1));
}

TIMER_CALLBACK_MEMBER(e0c6s46_device::buzzer_cb)
{
	// invert pulse wave if buzzer is on
	if (m_bz_43_on && !m_bz_1shot_running)
		m_bz_pulse = 0;
	else
		m_bz_pulse ^= 1;

	write_r4_out();
	schedule_buzzer();
}

void e0c6s46_device::reset_buzzer()
{
	// don't reset if the timer is running
	if (m_buzzer_handle->remaining() == attotime::never)
		schedule_buzzer();
}

void e0c6s46_device::clock_bz_1shot()
{
	m_bz_1shot_running = true;

	// reload counter the 1st time
	if (m_bz_1shot_count == 0)
	{
		m_bz_1shot_count = (m_bz_freq & 8) ? 16 : 8;
		reset_buzzer();
	}

	// stop ringing when counter reaches 0
	else if (--m_bz_1shot_count == 0)
	{
		m_bz_1shot_on = 0;
		m_bz_1shot_running = false;
	}
}

void e0c6s46_device::clock_bz_envelope()
{
	if (--m_bz_envelope_count == 0)
	{
		m_bz_envelope_count = (m_bz_envelope & 2) ? 32 : 16;

		// maximum duty addition is 7
		if (m_bz_duty_ratio < 7)
			m_bz_duty_ratio++;
	}
}

void e0c6s46_device::reset_bz_envelope()
{
	m_bz_envelope_count = (m_bz_envelope & 2) ? 32 : 16;
	m_bz_duty_ratio = 0;
}



//-------------------------------------------------
//  LCD Driver
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(e0c6s46_device::lcd_driver_cb)
{
	// this gets called 32 times per second (osc1/1024: 32hz at default clock of 32768hz)
	for (int bank = 0; bank < 2; bank++)
	{
		// determine operating mode
		bool lcd_on = false;
		int pixel = 0;
		if (m_lcd_control & 8 || (bank == 1 && m_lcd_control & 2))
			pixel = 0;
		else if (m_lcd_control & 4)
			pixel = 1;
		else
			lcd_on = true;

		// draw pixels
		for (int offset = 0; offset < m_vram[0].bytes(); offset++)
		{
			for (int c = 0; c < 4; c++)
			{
				if (lcd_on)
					pixel = m_vram[bank][offset] >> c & 1;

				// 16 COM(common) pins, 40 SEG(segment) pins
				int seg = offset / 2;
				int com = bank * 8 + (offset & 1) * 4 + c;

				const u8 width = m_vram[0].bytes() * 4 / 8;
				m_render_buf[com * width + seg] = pixel;
				m_write_segs(seg << 4 | com, pixel);
			}
		}
	}

	m_lcd_driver->adjust(attotime::from_ticks(1024, m_osc1));
}

u32 e0c6s46_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u8 *src = lcd_buffer();
	const u8 width = m_vram[0].bytes() * 4 / 8;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			int dx = x;
			int dy = y;

			if (!m_pixel_cb.isnull())
				m_pixel_cb(dx, dy);

			if (cliprect.contains(dx, dy))
				bitmap.pix(dy, dx) = src[y * width + x];
		}
	}

	return 0;
}



//-------------------------------------------------
//  internal I/O
//-------------------------------------------------

u8 e0c6s46_device::io_r(offs_t offset)
{
	switch (offset)
	{
		// IRQ flags, masks
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05:
		{
			// IRQ flags are reset(acked) when read
			u8 flag = m_irqflag[offset];
			if (!machine().side_effects_disabled())
				m_irqflag[offset] = 0;
			return flag;
		}
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
			return m_irqmask[offset & 7];

		// K input ports
		case 0x40: case 0x42:
			return m_port_k[offset >> 1 & 1];
		case 0x41:
			return m_dfk0;

		// R output ports
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54:
			return m_port_r[offset & 7];
		case 0x7b:
			return m_r_dir;

		// P I/O ports
		case 0x60: case 0x61: case 0x62: case 0x63:
			return read_p(offset & 3);
		case 0x7d:
			return m_p_dir;
		case 0x7e:
			return m_p_pullup;

		// clock-timer (lo, hi)
		case 0x20: case 0x21:
			return m_clktimer_count >> (4 * (offset & 1)) & 0xf;

		// stopwatch timer
		case 0x22:
			return m_swl_count;
		case 0x23:
			return m_swh_count;
		case 0x77:
			return m_stopwatch_on;

		// programmable timer
		case 0x24: case 0x25:
			return m_prgtimer_count >> (4 * (offset & 1)) & 0xf;
		case 0x26: case 0x27:
			return m_prgtimer_reload >> (4 * (offset & 1)) & 0xf;
		case 0x78:
			return m_prgtimer_on;
		case 0x79:
			return m_prgtimer_select;

		// buzzer
		case 0x74:
			return m_bz_freq;
		case 0x75:
			// d3: 1-shot buzzer is on
			return m_bz_1shot_on | m_bz_envelope;

		// OSC circuit
		case 0x70:
			return m_osc;

		// LCD driver
		case 0x71:
			return m_lcd_control;
		case 0x72:
			return m_lcd_contrast;

		// SVD circuit (supply voltage detection)
		case 0x73:
			// d3: criteria voltage* is 0: <=, 1: > source voltage (Vdd-Vss)
			// *0,1,2,3: -2.2V, -2.5V, -3.1V, -4.2V, 1 when off
			return m_svd | ((m_svd & 4 && m_svd != 7) ? 0 : 8);

		// write-only registers
		case 0x76:
			break;

		default:
			if (!machine().side_effects_disabled())
				logerror("unknown io_r from $0F%02X at $%04X\n", offset, m_prev_pc);
			break;
	}

	return 0;
}

void e0c6s46_device::io_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		// IRQ masks
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
		{
			static const u8 maskmask[6] = { 0xf, 3, 1, 1, 0xf, 0xf };
			u8 prev = m_irqmask[offset & 7];
			m_irqmask[offset & 7] = data & maskmask[offset & 7];
			u8 rise = ~prev & m_irqmask[offset & 7];

			if (rise)
			{
				m_possible_irq = true;

				if (offset >= 0x14)
				{
					u8 port = offset & 1;
					u8 dfk = (port == 0) ? m_dfk0 : 0xf;

					// IK flag gets set if input is active at rising edge of mask bit
					if (rise & (m_port_k[port] ^ dfk))
						m_irqflag[offset & 7] |= 1;
				}
			}
			break;
		}

		// K input ports
		case 0x41:
			// d0-d3: K0x IRQ on 0: rising edge, 1: falling edge
			m_dfk0 = data;
			break;

		// R output ports
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54:
			write_r(offset & 7, data);
			break;
		case 0x7b:
			// d0-d3: Rx* direction 0: high-impedance, 1: output
			if (data != m_r_dir)
			{
				m_r_dir = data;

				// refresh outputs
				for (int i = 0; i < 5; i++)
					write_r(i, m_port_r[i]);
			}
			break;

		// P I/O ports
		case 0x60: case 0x61: case 0x62: case 0x63:
			write_p(offset & 3, data);
			break;
		case 0x7d:
			// d0-d3: Px* direction 0: input, 1: output
			if (data != m_p_dir)
			{
				m_p_dir = data;

				// refresh outputs
				for (int i = 0; i < 4; i++)
					write_p(i, m_port_p[i]);
			}
			break;
		case 0x7e:
			// d0-d3: Px* pull up resistor on/off
			m_p_pullup = data;
			break;

		// OSC circuit
		case 0x70:
			// d0,d1: CPU operating voltage
			// d2: OSC3 on (high freq)
			// d3: clock source OSC1 or OSC3
			if ((m_osc ^ data) & 8)
			{
				if (m_osc1 == m_osc3)
					logerror("io_w unhandled OSC change, PC=$%04X\n", m_prev_pc);
				else
					m_osc_change->adjust(attotime::zero);
			}
			m_osc = data;
			break;

		// LCD driver
		case 0x71:
			// d0: heavy load protection
			// d1: duty 1/16 or 1/8
			// d2,d3: all pixels on,off
			m_lcd_control = data;
			break;
		case 0x72:
			// contrast adjustment (0=light, 15=dark)
			m_lcd_contrast = data;
			m_write_contrast(data);
			break;

		// SVD circuit (supply voltage detection)
		case 0x73:
			// d0,d1: criteria voltage
			// d2: on
			m_svd = data & 7;
			break;

		// clock-timer
		case 0x76:
			// d0: reset watchdog
			// d1: reset clktimer (hw glitch note, not emulated: this also "sometimes"(when??)
			// sets the clktimer interrupt and clocks the watchdog)
			if (data & 1)
				m_watchdog_count = 0;
			if (data & 2)
				m_clktimer_count = 0;
			break;

		// stopwatch timer
		case 0x77:
			// d0: run/stop counter
			// d1: reset stopwatch
			m_stopwatch_on = data & 1;
			if (data & 2)
			{
				m_swh_count = 0;
				m_swl_count = 0;
				m_swl_slice = 0;
			}
			if (m_stopwatch_on && m_swl_cur_pulse && !m_256_src_pulse)
			{
				// clock stopwatch on falling edge of pulse+on
				m_swl_cur_pulse = 0;
				clock_stopwatch();
			}
			break;

		// programmable timer
		case 0x26:
			m_prgtimer_reload = (m_prgtimer_reload & 0xf0) | data;
			break;
		case 0x27:
			m_prgtimer_reload = (m_prgtimer_reload & 0x0f) | data << 4;
			break;

		case 0x78:
			// d0: run/stop counter
			// d1: reset timer
			m_prgtimer_on = data & 1;
			if (data & 2)
			{
				m_prgtimer_count = m_prgtimer_reload;
			}
			if (m_prgtimer_on && (m_prgtimer_select & 7) >= 2 && m_prgtimer_cur_pulse && !m_prgtimer_src_pulse)
			{
				// if input clock is osc1, clock timer on falling edge of pulse+on
				m_prgtimer_cur_pulse = 0;
				clock_prgtimer();
			}
			break;

		case 0x79:
			// d0-d2: input clock select: 2-7 = osc1 divider 256hz-8192hz,
			// 0/1 = K03 input (0 enables noise rejector, no need to emulate that)
			// d3: output input clock to output port R33
			if ((data & 7) != (m_prgtimer_select & 7))
			{
				m_prgtimer_src_pulse = 0;
				m_prgtimer_cur_pulse = m_prgtimer_on ^ 1;
				m_prgtimer_select = data;
				prgtimer_reset_prescaler();
			}
			m_prgtimer_select = data;
			break;

		// buzzer
		case 0x74:
			// d0-d2: frequency (8 steps, 4096hz to ~1170hz)
			// d3: 1-shot buzzer duration 31.25ms or 62.5ms
			m_bz_freq = data;
			break;
		case 0x75:
			// d0: envelope on/off
			// d1: envelope cycle selection
			// d2: reset envelope
			// d3: trigger one-shot buzzer
			if (~m_bz_envelope & data & 1 || data & 4)
				reset_bz_envelope();
			m_bz_envelope = data & 3;
			m_bz_1shot_on |= data & 8;
			break;

		// read-only registers
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05:
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25:
		case 0x40: case 0x42:
			break;

		default:
			if (machine().phase() > machine_phase::RESET)
				logerror("unknown io_w $%X to $0F%02X at $%04X\n", data, offset, m_prev_pc);
			break;
	}
}
