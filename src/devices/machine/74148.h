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
    H   X  X  X  X  X  X  X  X  | H  H  H  | H  H
    L   H  H  H  H  H  H  H  H  | H  H  H  | H  L
    L   X  X  X  X  X  X  X  L  | L  L  L  | L  H
    L   X  X  X  X  X  X  L  H  | L  L  H  | L  H
    L   X  X  X  X  X  L  H  H  | L  H  L  | L  H
    L   X  X  X  X  L  H  H  H  | L  H  H  | L  H
    L   X  X  X  L  H  H  H  H  | H  L  L  | L  H
    L   X  X  L  H  H  H  H  H  | H  L  H  | L  H
    L   X  L  H  H  H  H  H  H  | H  H  L  | L  H
    L   L  H  H  H  H  H  H  H  | H  H  H  | L  H
    ----------------------------+----------+------
    L   = lo (0)
    H   = hi (1)
    X   = any state

*****************************************************************************/

#ifndef MAME_MACHINE_74148_H
#define MAME_MACHINE_74148_H

#pragma once

class ttl74148_device : public device_t
{
public:
	ttl74148_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~ttl74148_device() {}

	auto out_cb() { return m_output_cb.bind(); }

	/* must call update() after setting the inputs */
	void update();

	void input_line_w(int input_line, int data);
	void enable_input_w(int data);
	int  output_r();
	int  output_valid_r();
	int  enable_output_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	// internal state
	devcb_write8 m_output_cb;

	/* inputs */
	int m_input_lines[8]; /* pins 1-4,10-13 */
	int m_enable_input;   /* pin 5 */

	/* outputs */
	int m_output;         /* pins 6,7,9 */
	int m_output_valid;   /* pin 14 */
	int m_enable_output;  /* pin 15 */

	/* internals */
	int m_last_output;
	int m_last_output_valid;
	int m_last_enable_output;
};

DECLARE_DEVICE_TYPE(TTL74148, ttl74148_device)

#endif // MAME_DEVICES_MACHINE_74148_H
