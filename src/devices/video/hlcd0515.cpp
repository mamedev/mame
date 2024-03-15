// license:BSD-3-Clause
// copyright-holders:hap
/*

Hughes HLCD 0515 family LCD Driver

0515: 25 columns(also size of buffer/ram)
0569: 24 columns, display blank has no effect(instead it's external with VDRIVE?)
0530: specifications unknown, pinout seems similar to 0569
0601: specifications unknown, pinout seems similar to 0569

TODO:
- Does DATA OUT pin function the same on each chip? The 0515 datasheet says that
  the 25th column is output first, but on 0569(no datasheet available) it's reversed.

*/

#include "emu.h"
#include "hlcd0515.h"

#include <tuple>


DEFINE_DEVICE_TYPE(HLCD0515, hlcd0515_device, "hlcd0515", "Hughes HLCD 0515 LCD Driver")
DEFINE_DEVICE_TYPE(HLCD0569, hlcd0569_device, "hlcd0569", "Hughes HLCD 0569 LCD Driver")
DEFINE_DEVICE_TYPE(HLCD0530, hlcd0530_device, "hlcd0530", "Hughes HLCD 0530 LCD Driver")
DEFINE_DEVICE_TYPE(HLCD0601, hlcd0601_device, "hlcd0601", "Hughes HLCD 0601 LCD Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

hlcd0515_device::hlcd0515_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 colmax) :
	device_t(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_colmax(colmax),
	m_write_cols(*this), m_write_data(*this)
{
	// disable nvram by default
	nvram_enable_backup(false);
}

hlcd0515_device::hlcd0515_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hlcd0515_device(mconfig, HLCD0515, tag, owner, clock, 25)
{ }

hlcd0569_device::hlcd0569_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hlcd0515_device(mconfig, HLCD0569, tag, owner, clock, 24)
{ }

hlcd0530_device::hlcd0530_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hlcd0515_device(mconfig, HLCD0530, tag, owner, clock, 24)
{ }

hlcd0601_device::hlcd0601_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hlcd0515_device(mconfig, HLCD0601, tag, owner, clock, 24)
{ }



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hlcd0515_device::internal_clear()
{
	// clear internal registers and RAM
	m_count = 0;
	m_control = 0;
	m_blank = false;
	m_rowmax = 0;
	m_rowout = 0;
	m_rowsel = 0;
	m_buffer = 0;
	memset(m_ram, 0, sizeof(m_ram));
}

void hlcd0515_device::device_start()
{
	// timer
	m_lcd_timer = timer_alloc(FUNC(hlcd0515_device::scan_lcd), this);
	attotime period = attotime::from_hz(clock() / 2);
	m_lcd_timer->adjust(period, 0, period);

	// zerofill
	internal_clear();
	m_cs = 0;
	m_clk = 0;
	m_data = 0;
	m_dataout = 0;

	// register for savestates
	save_item(NAME(m_cs));
	save_item(NAME(m_clk));
	save_item(NAME(m_data));
	save_item(NAME(m_dataout));
	save_item(NAME(m_count));
	save_item(NAME(m_control));
	save_item(NAME(m_blank));
	save_item(NAME(m_rowmax));
	save_item(NAME(m_rowout));
	save_item(NAME(m_rowsel));
	save_item(NAME(m_buffer));
	save_item(NAME(m_ram));
}



//-------------------------------------------------
//  nvram (when VDD is battery-backed)
//-------------------------------------------------

bool hlcd0515_device::nvram_write(util::write_stream &file)
{
	std::error_condition err;
	size_t actual;

	// misc internal registers
	u8 buf[6];
	buf[0] = m_count;
	buf[1] = m_control;
	buf[2] = m_blank ? 1 : 0;
	buf[3] = m_rowmax;
	buf[4] = m_rowout;
	buf[5] = m_rowsel;

	std::tie(err, actual) = write(file, &buf, sizeof(buf));
	if (err)
		return false;

	// shift register and RAM
	std::tie(err, actual) = write(file, &m_buffer, sizeof(m_buffer));
	if (err)
		return false;
	std::tie(err, actual) = write(file, &m_ram, sizeof(m_ram));
	if (err)
		return false;

	return true;
}

bool hlcd0515_device::nvram_read(util::read_stream &file)
{
	std::error_condition err;
	size_t actual;

	// misc internal registers
	u8 buf[6];
	std::tie(err, actual) = read(file, &buf, sizeof(buf));
	if (err || (sizeof(buf) != actual))
		return false;

	m_count = buf[0];
	m_control = buf[1];
	m_blank = bool(buf[2]);
	m_rowmax = buf[3] & 7;
	m_rowout = buf[4] & 7;
	m_rowsel = buf[5] & 7;

	// shift register and RAM
	std::tie(err, actual) = read(file, &m_buffer, sizeof(m_buffer));
	if (err || (sizeof(m_buffer) != actual))
		return false;
	std::tie(err, actual) = read(file, &m_ram, sizeof(m_ram));
	if (err || (sizeof(m_ram) != actual))
		return false;

	return true;
}



//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(hlcd0515_device::scan_lcd)
{
	if (m_rowout > m_rowmax)
		m_rowout = 0;

	// write to COL/ROW pins
	m_write_cols(m_rowout, m_blank ? 0 : m_ram[m_rowout], ~0);
	m_rowout++;
}


void hlcd0515_device::set_control()
{
	// clock 0,1,2: row select
	m_rowsel = m_control >> 2 & 7;

	// clock 3(,4): initialize
	if (m_control & 2)
	{
		m_rowmax = m_rowsel;
		m_blank = bool(~m_control & 1);
	}

	// clock 4: read/write mode
	if (m_control & 1)
	{
		m_buffer = m_ram[m_rowsel];
		clock_data();
	}
	else
		m_buffer = 0;
}

void hlcd0569_device::set_control()
{
	hlcd0515_device::set_control();
	m_blank = false; // 0569 doesn't support display blanking
}


void hlcd0515_device::clock_data(int col)
{
	if (m_control & 1)
	{
		m_dataout = m_buffer & 1;
		m_write_data(m_dataout);

		m_buffer >>= 1;
	}
	else
	{
		if (col < m_colmax)
			m_buffer >>= 1;

		// always write last column
		u32 mask = 1 << (m_colmax - 1);
		m_buffer = (m_buffer & ~mask) | (m_data ? mask : 0);
	}
}


void hlcd0515_device::clock_w(int state)
{
	state = (state) ? 1 : 0;

	// clock/shift data on falling edge
	if (!m_cs && !state && m_clk)
	{
		if (m_count < 5)
		{
			// 5-bit mode/control
			m_control = m_control << 1 | m_data;
			if (m_count == 4)
				set_control();
		}
		else
			clock_data(m_count - 5);

		if (m_count < (m_colmax + 5))
			m_count++;
	}

	m_clk = state;
}


void hlcd0515_device::cs_w(int state)
{
	state = (state) ? 1 : 0;

	// finish serial sequence on rising edge
	if (state && !m_cs)
	{
		// transfer to ram
		if (~m_control & 1)
			m_ram[m_rowsel] = m_buffer;

		m_count = 0;
		m_control = 0;
	}

	m_cs = state;
}
