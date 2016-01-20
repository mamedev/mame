// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*****************************************************************************

  74153 Dual 4-line to 1-line data selectors/multiplexers


  Pin layout and functions to access pins:

  enable_w(0)        [1] /1G   VCC [16]
  b_w                [2] B     /2G [15]  enable_w(1)
  input_line_w(0,3)  [3] 1C3     A [14]  a_w
  input_line_w(0,2)  [4] 1C2   2C3 [13]  input_line_w(1,3)
  input_line_w(0,1)  [5] 1C1   2C2 [12]  input_line_w(1,2)
  input_line_w(0,0)  [6] 1C0   2C1 [11]  input_line_w(1,1)
  output_r(0)        [7] 1Y    2C0 [10]  input_line_w(1,0)
                     [8] GND    2Y [9]   output_r(1)


  Truth table (all logic levels indicate the actual voltage on the line):

            INPUTS         | OUTPUT
                           |
    G | B  A | C0 C1 C2 C3 | Y
    --+------+-------------+---
1   H | X  X | X  X  X  X  | L
2   L | L  L | X  X  X  X  | C0
3   L | L  H | X  X  X  X  | C1
4   L | H  L | X  X  X  X  | C2
5   L | H  H | X  X  X  X  | C3
    --+------+-------------+---
    L   = lo (0)
    H   = hi (1)
    X   = any state

*****************************************************************************/

#include "emu.h"
#include "machine/74153.h"


const device_type TTL74153 = &device_creator<ttl74153_device>;

ttl74153_device::ttl74153_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, TTL74153, "74153 TTL", tag, owner, clock, "74153", __FILE__),
					m_a(0),
					m_b(0)
{
	m_input_lines[0][0] = 0;
	m_input_lines[0][1] = 0;
	m_input_lines[0][2] = 0;
	m_input_lines[0][3] = 0;
	m_input_lines[1][0] = 0;
	m_input_lines[1][1] = 0;
	m_input_lines[1][2] = 0;
	m_input_lines[1][3] = 0;

	for (auto & elem : m_enable)
		elem = 0;

	for (auto & elem : m_output)
		elem = 0;

	for (auto & elem : m_last_output)
		elem = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl74153_device::device_start()
{
	m_output_cb.bind_relative_to(*owner());

	save_item(NAME(m_enable));
	save_item(NAME(m_last_output));
	save_item(NAME(m_input_lines[0][0]));
	save_item(NAME(m_input_lines[0][1]));
	save_item(NAME(m_input_lines[0][2]));
	save_item(NAME(m_input_lines[0][3]));
	save_item(NAME(m_input_lines[1][0]));
	save_item(NAME(m_input_lines[1][1]));
	save_item(NAME(m_input_lines[1][2]));
	save_item(NAME(m_input_lines[1][3]));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ttl74153_device::device_reset()
{
	m_a = 1;
	m_b = 1;
	m_enable[0] = 1;
	m_enable[1] = 1;
	m_input_lines[0][0] = 1;
	m_input_lines[0][1] = 1;
	m_input_lines[0][2] = 1;
	m_input_lines[0][3] = 1;
	m_input_lines[1][0] = 1;
	m_input_lines[1][1] = 1;
	m_input_lines[1][2] = 1;
	m_input_lines[1][3] = 1;

	m_last_output[0] = -1;
	m_last_output[1] = -1;
}


void ttl74153_device::update()
{
	int sel;
	int section;


	sel = (m_b << 1) | m_a;


	/* process both sections */
	for (section = 0; section < 2; section++)
	{
		if (m_enable[section])
			m_output[section] = 0; // row 1 in truth table
		else
			m_output[section] = m_input_lines[section][sel];
	}


	/* call callback if either of the outputs changed */
	if (!m_output_cb.isnull() &&
		((m_output[0] != m_last_output[0]) || (m_output[1] != m_last_output[1])))
	{
		m_last_output[0] = m_output[0];
		m_last_output[1] = m_output[1];

		m_output_cb();
	}
}


void ttl74153_device::a_w(int data)
{
	m_a = data ? 1 : 0;
}


void ttl74153_device::b_w(int data)
{
	m_b = data ? 1 : 0;
}


void ttl74153_device::input_line_w(int section, int input_line, int data)
{
	m_input_lines[section][input_line] = data ? 1 : 0;
}


void ttl74153_device::enable_w(int section, int data)
{
	m_enable[section] = data ? 1 : 0;
}


int ttl74153_device::output_r(int section)
{
	return m_output[section];
}
