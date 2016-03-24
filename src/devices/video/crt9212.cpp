// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SMC CRT9212 Double Row Buffer (DRB) emulation

**********************************************************************/

#include "crt9212.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CRT9212 = &device_creator<crt9212_t>;



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  crt9212_t - constructor
//-------------------------------------------------

crt9212_t::crt9212_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CRT9212, "SMC CRT9212", tag, owner, clock, "crt9212", __FILE__),
	m_write_dout(*this),
	m_write_rof(*this),
	m_write_wof(*this),
	m_data(0),
	m_clrcnt(0),
	m_tog(0),
	m_ren(0),
	m_wen1(1),
	m_wen2(0),
	m_oe(0),
	m_rclk(0),
	m_wclk(0),
	m_clrcnt_edge(false),
	m_data_latch(0),
	m_ren_int(0),
	m_wen_int(0),
	m_buffer(0),
	m_rac(0),
	m_wac(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void crt9212_t::device_start()
{
	// resolve callbacks
	m_write_dout.resolve_safe();
	m_write_rof.resolve_safe();
	m_write_wof.resolve_safe();

	// state saving
	save_item(NAME(m_data));
	save_item(NAME(m_clrcnt));
	save_item(NAME(m_tog));
	save_item(NAME(m_ren));
	save_item(NAME(m_wen1));
	save_item(NAME(m_wen2));
	save_item(NAME(m_oe));
	save_item(NAME(m_rclk));
	save_item(NAME(m_wclk));
	save_item(NAME(m_clrcnt_edge));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_ren_int));
	save_item(NAME(m_wen_int));
	save_item(NAME(m_ram[0]));
	save_item(NAME(m_ram[1]));
	save_item(NAME(m_buffer));
	save_item(NAME(m_rac));
	save_item(NAME(m_wac));
}


//-------------------------------------------------
//  clrcnt_w - clear counter
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9212_t::clrcnt_w )
{
	if (m_clrcnt && !state)
	{
		m_clrcnt_edge = true;
	}

	m_clrcnt = state;
}


//-------------------------------------------------
//  rclk_w - read clock
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9212_t::rclk_w )
{
	if (!m_rclk && state)
	{
		if (m_clrcnt_edge)
		{
			// reset read address counter
			m_rac = 0;

			// reset read overflow
			m_write_rof(0);

			if (!m_tog)
			{
				// switch buffers
				m_buffer = !m_buffer;

				// reset write address counter
				m_wac = 0;

				// reset write overflow
				m_write_wof(0);
			}

			m_clrcnt_edge = false;
		}

		if (m_ren_int && (m_rac < CRT9212_RAM_SIZE))
		{
			// output data
			m_write_dout(m_ram[m_rac][!m_buffer]);

			// increment read address counter
			m_rac++;

			if (m_rac == CRT9212_RAM_SIZE - 1)
			{
				// set read overflow
				m_write_rof(1);
			}
		}

		m_ren_int = m_ren;
	}

	m_rclk = state;
}


//-------------------------------------------------
//  wclk_w - write clock
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9212_t::wclk_w )
{
	if (!m_wclk && state)
	{
		if (m_wen_int && (m_wac < CRT9212_RAM_SIZE))
		{
			// input data
			m_ram[m_rac][m_buffer] = m_data_latch;

			// increment write address counter
			m_wac++;

			if (m_wac == CRT9212_RAM_SIZE - 1)
			{
				// set write overflow
				m_write_wof(1);
			}
		}

		if (m_wen1 && m_wen2)
		{
			m_data_latch = m_data;
		}

		m_wen_int = m_wen1 && m_wen2;
	}

	m_wclk = state;
}
