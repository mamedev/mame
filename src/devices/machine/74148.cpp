// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*****************************************************************************

  74148 8-line-to-3-line priority encoder


  Pin layout and functions to access pins:

  input_line_w(4)  [1] /IN4   VCC [16]
  input_line_w(5)  [2] /IN5   /EO [15]  enable_output_r
  input_line_w(6)  [3] /IN6   /GS [14]  output_valid_r
  input_line_w(7)  [4] /IN7  /IN3 [13]  input_line_w(3)
  enable_input_w   [5] /EI   /IN2 [12]  input_line_w(2)
  output_r         [6] /A2   /IN1 [11]  input_line_w(1)
  output_r         [7] /A1   /IN0 [10]  input_line_w(0)
                   [8] GND   /A0  [9]   output_r


  Truth table (all logic levels indicate the actual voltage on the line):

              INPUTS            |     OUTPUTS
                                |
    EI  I0 I1 I2 I3 I4 I5 I6 I7 | A2 A1 A0 | GS EO
    ----------------------------+----------+------
 1  H   X  X  X  X  X  X  X  X  | H  H  H  | H  H
 2  L   H  H  H  H  H  H  H  H  | H  H  H  | H  L
 3  L   X  X  X  X  X  X  X  L  | L  L  L  | L  H
 4  L   X  X  X  X  X  X  L  H  | L  L  H  | L  H
 5  L   X  X  X  X  X  L  H  H  | L  H  L  | L  H
 6  L   X  X  X  X  L  H  H  H  | L  H  H  | L  H
 7  L   X  X  X  L  H  H  H  H  | H  L  L  | L  H
 8  L   X  X  L  H  H  H  H  H  | H  L  H  | L  H
 9  L   X  L  H  H  H  H  H  H  | H  H  L  | L  H
 10 L   L  H  H  H  H  H  H  H  | H  H  H  | L  H
    ----------------------------+----------+------
    L   = lo (0)
    H   = hi (1)
    X   = any state

*****************************************************************************/

#include "emu.h"
#include "machine/74148.h"


const device_type TTL74148 = &device_creator<ttl74148_device>;

ttl74148_device::ttl74148_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, TTL74148, "74148 TTL", tag, owner, clock, "74148", __FILE__),
					m_enable_input(0),
					m_output(0),
					m_output_valid(0),
					m_enable_output(0),
					m_last_output(0),
					m_last_output_valid(0),
					m_last_enable_output(0)
{
	for (auto & elem : m_input_lines)
		elem = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl74148_device::device_start()
{
	m_output_cb.bind_relative_to(*owner());

	save_item(NAME(m_input_lines));
	save_item(NAME(m_enable_input));
	save_item(NAME(m_output));
	save_item(NAME(m_output_valid));
	save_item(NAME(m_enable_output));
	save_item(NAME(m_last_output));
	save_item(NAME(m_last_output_valid));
	save_item(NAME(m_last_enable_output));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ttl74148_device::device_reset()
{
	m_enable_input = 1;
	m_input_lines[0] = 1;
	m_input_lines[1] = 1;
	m_input_lines[2] = 1;
	m_input_lines[3] = 1;
	m_input_lines[4] = 1;
	m_input_lines[5] = 1;
	m_input_lines[6] = 1;
	m_input_lines[7] = 1;

	m_last_output = -1;
	m_last_output_valid = -1;
	m_last_enable_output = -1;
}


void ttl74148_device::update()
{
	if (m_enable_input)
	{
		// row 1 in truth table
		m_output = 0x07;
		m_output_valid = 1;
		m_enable_output = 1;
	}
	else
	{
		int bit0, bit1, bit2;

		/* this comes straight off the data sheet schematics */
		bit0 = !(((!m_input_lines[1]) &
					m_input_lines[2] &
					m_input_lines[4] &
					m_input_lines[6])  |
					((!m_input_lines[3]) &
					m_input_lines[4] &
					m_input_lines[6])  |
					((!m_input_lines[5]) &
					m_input_lines[6])  |
					(!m_input_lines[7]));

		bit1 = !(((!m_input_lines[2]) &
					m_input_lines[4] &
					m_input_lines[5])  |
					((!m_input_lines[3]) &
					m_input_lines[4] &
					m_input_lines[5])  |
					(!m_input_lines[6])  |
					(!m_input_lines[7]));

		bit2 = !((!m_input_lines[4])  |
					(!m_input_lines[5])  |
					(!m_input_lines[6])  |
					(!m_input_lines[7]));

		m_output = (bit2 << 2) | (bit1 << 1) | bit0;

		m_output_valid = (m_input_lines[0] &
										m_input_lines[1] &
										m_input_lines[2] &
										m_input_lines[3] &
										m_input_lines[4] &
										m_input_lines[5] &
										m_input_lines[6] &
										m_input_lines[7]);

		m_enable_output = !m_output_valid;
	}


	/* call callback if any of the outputs changed */
	if (!m_output_cb.isnull() &&
		((m_output != m_last_output) ||
			(m_output_valid != m_last_output_valid) || (m_enable_output != m_last_enable_output)))
	{
		m_last_output = m_output;
		m_last_output_valid = m_output_valid;
		m_last_enable_output = m_enable_output;

		m_output_cb();
	}
}


void ttl74148_device::input_line_w(int input_line, int data)
{
	m_input_lines[input_line] = data ? 1 : 0;
}


void ttl74148_device::enable_input_w(int data)
{
	m_enable_input = data ? 1 : 0;
}


int ttl74148_device::output_r()
{
	return m_output;
}


int ttl74148_device::output_valid_r()
{
	return m_output_valid;
}


int ttl74148_device::enable_output_r()
{
	return m_enable_output;
}
