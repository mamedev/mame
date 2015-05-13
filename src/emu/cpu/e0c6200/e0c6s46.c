// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6S46 MCU

*/

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

const device_type E0C6S46 = &device_creator<e0c6s46_device>;


// internal memory maps
static ADDRESS_MAP_START(e0c6s46_program, AS_PROGRAM, 16, e0c6s46_device)
	AM_RANGE(0x0000, 0x17ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(e0c6s46_data, AS_DATA, 8, e0c6s46_device)
	AM_RANGE(0x0000, 0x027f) AM_RAM
	AM_RANGE(0x0e00, 0x0e4f) AM_RAM AM_SHARE("vram1")
	AM_RANGE(0x0e80, 0x0ecf) AM_RAM AM_SHARE("vram2")
	AM_RANGE(0x0f00, 0x0f7f) AM_READWRITE(io_r, io_w)
ADDRESS_MAP_END


// device definitions
e0c6s46_device::e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: e0c6200_cpu_device(mconfig, E0C6S46, "E0C6S46", tag, owner, clock, ADDRESS_MAP_NAME(e0c6s46_program), ADDRESS_MAP_NAME(e0c6s46_data), "e0c6s46", __FILE__)
	, m_vram1(*this, "vram1")
	, m_vram2(*this, "vram2")
	, m_pixel_update_handler(NULL)
{ }





//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e0c6s46_device::device_start()
{
	e0c6200_cpu_device::device_start();
	
	// create timers
	m_clktimer_handle = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(e0c6s46_device::clktimer_cb), this));
	m_clktimer_handle->adjust(attotime::from_ticks(128, unscaled_clock()));
	m_stopwatch_handle = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(e0c6s46_device::stopwatch_cb), this));
	m_stopwatch_handle->adjust(attotime::from_ticks(64, unscaled_clock()));
	m_prgtimer_handle = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(e0c6s46_device::prgtimer_cb), this));
	m_prgtimer_handle->adjust(attotime::never);
	
	// zerofill
	memset(m_port_k, 0xf, sizeof(m_port_k));
	m_dfk0 = 0xf;
	
	memset(m_irqflag, 0, sizeof(m_irqflag));
	memset(m_irqmask, 0, sizeof(m_irqmask));
	m_osc = 0;
	m_svd = 0;
	m_lcd_control = 0;
	m_lcd_contrast = 0;

	m_watchdog_count = 0;
	m_clktimer_count = 0;

	m_stopwatch_on = 0;
	m_swl_src_pulse = 0;
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
	
	// register for savestates
	save_item(NAME(m_port_k));
	save_item(NAME(m_dfk0));
	
	save_item(NAME(m_irqflag));
	save_item(NAME(m_irqmask));
	save_item(NAME(m_osc));
	save_item(NAME(m_svd));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_lcd_contrast));

	save_item(NAME(m_watchdog_count));
	save_item(NAME(m_clktimer_count));

	save_item(NAME(m_stopwatch_on));
	save_item(NAME(m_swl_src_pulse));
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
	
	// reset other i/o
	m_data->write_byte(0xf41, 0xf);
	m_data->write_byte(0xf54, 0xf);
	m_data->write_byte(0xf70, 0x0);
	m_data->write_byte(0xf71, 0x8);
	m_data->write_byte(0xf73, m_svd & 0xc0);

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
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void e0c6s46_device::execute_one()
{
	// E0C6S46 has no support for SLP opcode
	if (m_op == 0xff9)
		return;
	
	e0c6200_cpu_device::execute_one();
}



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
		// middle of handling this interrupt, irq vector may be an OR of 2 vectors
		m_irq_vector = 2*pri + 2;
		int reg = priorder[pri];
		m_irq_id = reg;
		
		switch (reg)
		{
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
	int port = line >> 3 & 1;
	UINT8 bit = 1 << (line & 3);
	
	m_port_k[port] = (m_port_k[port] & ~bit) | (state ? bit : 0);
}


void e0c6s46_device::clock_watchdog()
{
	// initial reset after 3 to 4 seconds
	if (++m_watchdog_count == 4)
	{
		logerror("%s watchdog reset\n", tag());
		m_watchdog_count = 0;
		device_reset();
	}
}

TIMER_CALLBACK_MEMBER(e0c6s46_device::clktimer_cb)
{
	m_clktimer_count++;
	
	// irq on falling edge of 32, 8, 2, 1hz
	UINT8 flag = 0;
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
	
	// schedule next timeout (256hz at default clock of 32768hz)
	m_clktimer_handle->adjust(attotime::from_ticks(128, unscaled_clock()));

	// 1hz falling edge also clocks the watchdog timer
	if (m_clktimer_count == 0)
		clock_watchdog();
}

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

		// bcd counter, irq on falling edge of 10 and 1hz
		m_swl_count = (m_swl_count + 1) % 10;
		if (m_swl_count == 0)
		{
			m_irqflag[IRQREG_STOPWATCH] |= 1;
			m_swh_count = (m_swh_count + 1) % 10;
			if (m_swh_count == 0)
				m_irqflag[IRQREG_STOPWATCH] |= 2;
		}

		if (m_irqflag[IRQREG_STOPWATCH] & m_irqmask[IRQREG_STOPWATCH])
			m_possible_irq = true;
	}
}


TIMER_CALLBACK_MEMBER(e0c6s46_device::stopwatch_cb)
{
	m_swl_src_pulse ^= 1;
	m_swl_cur_pulse = m_swl_src_pulse | (m_stopwatch_on ^ 1);
	
	// clock stopwatch on falling edge of pulse+on
	if (m_swl_cur_pulse == 0)
		clock_stopwatch();
	
	// schedule next timeout (256hz high+low at default clock of 32768hz)
	m_stopwatch_handle->adjust(attotime::from_ticks(64, unscaled_clock()));
}

void e0c6s46_device::clock_prgtimer()
{
	// irq and reload when it reaches zero
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
	UINT8 sel = m_prgtimer_select & 7;
	if (sel >= 2)
		m_prgtimer_handle->adjust(attotime::from_ticks(2 << (sel ^ 7), unscaled_clock()));
	
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


UINT32 e0c6s46_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int bank = 0; bank < 2; bank++)
	{
		const UINT8* vram = bank ? m_vram2 : m_vram1;

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
		for (int offset = 0; offset < 0x50; offset++)
		{
			for (int c = 0; c < 4; c++)
			{
				if (lcd_on)
					pixel = vram[offset] >> c & 1;
				
				// 16 COM(common) pins, 40 SEG(segment) pins
				int seg = offset / 2;
				int com = bank * 8 + (offset & 1) * 4 + c;
				
				if (m_pixel_update_handler != NULL)
					m_pixel_update_handler(*this, bitmap, cliprect, m_lcd_contrast, seg, com, pixel);
				else if (cliprect.contains(seg, com))
					bitmap.pix16(com, seg) = pixel;
			}
		}
	}

	return 0;
}

READ8_MEMBER(e0c6s46_device::io_r)
{
	switch (offset)
	{
		// irq flags, masks
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05:
		{
			// irq flags are reset(acked) when read
			UINT8 flag = m_irqflag[offset];
			if (!space.debugger_access())
				m_irqflag[offset] = 0;
			return flag;
		}
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
			return m_irqmask[offset-0x10];
		
		// k input ports
		case 0x40: case 0x42:
			return m_port_k[offset >> 1 & 1];
		case 0x41:
			return m_dfk0;
		
		// clock timer (lo, hi)
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
			if (!space.debugger_access())
				logerror("%s unknown io_r from $0F%02X at $%04X\n", tag(), offset, m_prev_pc);
			break;
	}

	return 0;
}

WRITE8_MEMBER(e0c6s46_device::io_w)
{
	switch (offset)
	{
		// irq masks
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
		{
			static const UINT8 maskmask[6] = { 0xf, 3, 1, 1, 0xf, 0xf };
			m_irqmask[offset-0x10] = data & maskmask[offset-0x10];
			m_possible_irq = true;
			break;
		}
		
		// k input ports
		case 0x41:
			// d0-d3: K0x input port irq on 0: rising edge, 1: falling edge, 
			m_dfk0 = data;
			break;
		
		// OSC circuit
		case 0x70:
			// d0,d1: CPU operating voltage
			// d2: OSC3 on (high freq)
			// d3: clock source OSC1 or OSC3
			if (data & 8)
				logerror("%s io_w selected OSC3! PC=$%04X\n", tag(), m_prev_pc);
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
			break;

		// SVD circuit (supply voltage detection)
		case 0x73:
			// d0,d1: criteria voltage
			// d2: on
			m_svd = data & 7;
			break;
		
		// clock timer
		case 0x76:
			// d0: reset watchdog
			// d1: reset clock timer (hw glitch note, not emulated: this also "sometimes"(when??)
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
			if (m_stopwatch_on && m_swl_cur_pulse && !m_swl_src_pulse)
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
		
		// read-only registers
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05:
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25:
		case 0x40: case 0x42:
			break;

		default:
			if (machine().phase() > MACHINE_PHASE_RESET)
				logerror("%s unknown io_w $%X to $0F%02X at $%04X\n", tag(), data, offset, m_prev_pc);
			break;
	}
}
