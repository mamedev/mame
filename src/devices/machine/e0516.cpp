// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Microelectronic-Marin E050-16 Real Time Clock emulation

**********************************************************************/

#include "emu.h"
#include "e0516.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


// states
enum
{
	STATE_ADDRESS = 0,
	STATE_DATA
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type E0516 = &device_creator<e0516_device>;

//-------------------------------------------------
//  e0516_device - constructor
//-------------------------------------------------

e0516_device::e0516_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, E0516, "E05-16", tag, owner, clock, "e0516", __FILE__),
		device_rtc_interface(mconfig, *this), m_cs(0), m_clk(0), m_data_latch(0), m_reg_latch(0), m_read_write(0), m_state(0), m_bits(0), m_dio(0), m_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e0516_device::device_start()
{
	// allocate timers
	m_timer = timer_alloc();
	m_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_clk));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_reg_latch));
	save_item(NAME(m_read_write));
	save_item(NAME(m_state));
	save_item(NAME(m_bits));
	save_item(NAME(m_dio));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void e0516_device::device_reset()
{
	set_current_time(machine());
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void e0516_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	advance_seconds();
}


//-------------------------------------------------
//  cs_w - chip select input
//-------------------------------------------------

WRITE_LINE_MEMBER( e0516_device::cs_w )
{
	if (LOG) logerror("E05-16 '%s' CS %u\n", tag().c_str(), state);

	m_cs = state;

	if (m_cs)
	{
		m_data_latch = 0;
		m_reg_latch = 0;
		m_bits = 0;
		m_state = STATE_ADDRESS;
	}
}


//-------------------------------------------------
//  clk_w - serial clock input
//-------------------------------------------------

WRITE_LINE_MEMBER( e0516_device::clk_w )
{
	if (LOG) logerror("E05-16 '%s' CLK %u\n", tag().c_str(), state);

	m_clk = state;

	if (m_cs || m_clk) return;

	m_bits++;

	if (m_state == STATE_ADDRESS)
	{
		if (LOG) logerror("E05-16 '%s' Command Bit %u\n", tag().c_str(), m_dio);

		// command
		m_reg_latch |= m_dio << 3;
		m_reg_latch >>= 1;

		if (m_bits == 4)
		{
			m_state = STATE_DATA;
			m_bits = 0;

			if (BIT(m_reg_latch, 0))
			{
				// load register value to data latch
				m_data_latch = convert_to_bcd(get_clock_register(m_reg_latch >> 1));
			}
		}
	}
	else
	{
		// data
		if (BIT(m_reg_latch, 0))
		{
			// read
			if (LOG) logerror("E05-16 '%s' Data Bit OUT %u\n", tag().c_str(), m_dio);

			m_dio = BIT(m_data_latch, 0);
			m_data_latch >>= 1;
		}
		else
		{
			// write
			if (LOG) logerror("E05-16 '%s' Data Bit IN %u\n", tag().c_str(), m_dio);

			m_data_latch |= m_dio << 7;
			m_data_latch >>= 1;
		}

		if (m_bits == 8)
		{
			m_state = STATE_ADDRESS;
			m_bits = 0;

			if (!BIT(m_reg_latch, 0))
			{
				// write latched data to register
				set_clock_register(m_reg_latch >> 1, bcd_to_integer(m_data_latch));
			}
		}
	}
}


//-------------------------------------------------
//  dio_w - serial data input
//-------------------------------------------------

WRITE_LINE_MEMBER( e0516_device::dio_w )
{
	if (LOG) logerror("E05-16 '%s' DIO %u\n", tag().c_str(), state);

	m_dio = state;
}


//-------------------------------------------------
//  do_r - serial data output
//-------------------------------------------------

READ_LINE_MEMBER( e0516_device::dio_r )
{
	return m_dio;
}
