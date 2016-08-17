// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*****************************************************************************

  7474 positive-edge-triggered D-type flip-flop with preset, clear and
       complementary outputs.  There are 2 flip-flops per chips


  Pin layout and functions to access pins:

  clear_w        [1] /1CLR         VCC [14]
  d_w            [2]  1D         /2CLR [13]  clear_w
  clock_w        [3]  1CLK          2D [12]  d_w
  preset_w       [4] /1PR         2CLK [11]  clock_w
  output_r       [5]  1Q          /2PR [10]  preset_w
  output_comp_r  [6] /1Q            2Q [9]   output_r
                 [7]  GND          /2Q [8]   output_comp_r


  Truth table (all logic levels indicate the actual voltage on the line):

        INPUTS    | OUTPUTS
                  |
    PR  CLR CLK D | Q  /Q
    --------------+-------
 1  L   H   X   X | H   L
 2  H   L   X   X | L   H
 3  L   L   X   X | H   H  (Note 1)
 4  H   H  _-   X | D  /D
 5  H   H   L   X | Q0 /Q0
    --------------+-------
    L   = lo (0)
    H   = hi (1)
    X   = any state
    _-  = raising edge
    Q0  = previous state

    Note 1: Non-stable configuration

*****************************************************************************/

#include "emu.h"
#include "7474.h"



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type TTL7474 = &device_creator<ttl7474_device>;

//-------------------------------------------------
//  ttl7474_device - constructor
//-------------------------------------------------

ttl7474_device::ttl7474_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TTL7474, "7474 TTL", tag, owner, clock, "7474", __FILE__),
		m_output_func(*this),
		m_comp_output_func(*this)
{
	init();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl7474_device::device_start()
{
	save_item(NAME(m_clear));
	save_item(NAME(m_preset));
	save_item(NAME(m_clk));
	save_item(NAME(m_d));
	save_item(NAME(m_output));
	save_item(NAME(m_output_comp));
	save_item(NAME(m_last_clock));
	save_item(NAME(m_last_output));
	save_item(NAME(m_last_output_comp));

	m_output_func.resolve_safe();
	m_comp_output_func.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ttl7474_device::device_reset()
{
	init();
}


//-------------------------------------------------
//  update - update internal state
//-------------------------------------------------

void ttl7474_device::update()
{
	if (!m_preset && m_clear)           // line 1 in truth table
	{
		m_output    = 1;
		m_output_comp = 0;
	}
	else if (m_preset && !m_clear)      // line 2 in truth table
	{
		m_output    = 0;
		m_output_comp = 1;
	}
	else if (!m_preset && !m_clear)     // line 3 in truth table
	{
		m_output    = 1;
		m_output_comp = 1;
	}
	else if (!m_last_clock && m_clk)    // line 4 in truth table
	{
		m_output    =  m_d;
		m_output_comp = !m_d;
	}

	m_last_clock = m_clk;


	// call callback if any of the outputs changed
	if (m_output != m_last_output)
	{
		m_last_output = m_output;
		m_output_func(m_output);
	}
	// call callback if any of the outputs changed
	if (m_output_comp != m_last_output_comp)
	{
		m_last_output_comp = m_output_comp;
		m_comp_output_func(m_output_comp);
	}
}


//-------------------------------------------------
//  clear_w - set the clear line state
//-------------------------------------------------

WRITE_LINE_MEMBER( ttl7474_device::clear_w )
{
	m_clear = state & 1;
	update();
}


//-------------------------------------------------
//  clear_w - set the clear line state
//-------------------------------------------------

WRITE_LINE_MEMBER( ttl7474_device::preset_w )
{
	m_preset = state & 1;
	update();
}


//-------------------------------------------------
//  clock_w - set the clock line state
//-------------------------------------------------

WRITE_LINE_MEMBER( ttl7474_device::clock_w )
{
	m_clk = state & 1;
	update();
}


//-------------------------------------------------
//  d_w - set the d line state
//-------------------------------------------------

WRITE_LINE_MEMBER( ttl7474_device::d_w )
{
	m_d = state & 1;
	update();
}


//-------------------------------------------------
//  output_r - get the output line state
//-------------------------------------------------

READ_LINE_MEMBER( ttl7474_device::output_r )
{
	return m_output;
}


//-----------------------------------------------------
//  output_comp_r - get the output-compare line state
//-----------------------------------------------------

READ_LINE_MEMBER( ttl7474_device::output_comp_r )
{
	return m_output_comp;
}

void ttl7474_device::init()
{
	m_clear = 1;
	m_preset = 1;
	m_clk = 1;
	m_d = 1;

	m_output = -1;
	m_last_clock = 1;
	m_last_output = -1;
	m_last_output_comp = -1;
}
