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
    L   H   X   X | H   L
    H   L   X   X | L   H
    L   L   X   X | H   H  (Note 1)
    H   H  _-   X | D  /D
    H   H   L   X | Q0 /Q01
    --------------+-------
    L   = lo (0)
    H   = hi (1)
    X   = any state
    _-  = raising edge
    Q0  = previous state

    Note 1: Non-stable configuration

*****************************************************************************/

#ifndef MAME_MACHINE_TTL7474_H
#define MAME_MACHINE_TTL7474_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ttl7474_device

class ttl7474_device : public device_t
{
public:
	// construction/destruction
	ttl7474_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	auto output_cb() { return m_output_func.bind(); }
	auto comp_output_cb() { return m_comp_output_func.bind(); }

	// public interfaces
	void clear_w(int state);
	void preset_w(int state);
	void clock_w(int state);
	void d_w(int state);
	int output_r();
	int output_comp_r();    // NOT strictly the same as !output_r()

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// callbacks
	devcb_write_line m_output_func;
	devcb_write_line m_comp_output_func;

	// inputs
	uint8_t m_clear;              // pin 1/13
	uint8_t m_preset;             // pin 4/10
	uint8_t m_clk;                // pin 3/11
	uint8_t m_d;                  // pin 2/12

	// outputs
	uint8_t m_output;             // pin 5/9
	uint8_t m_output_comp;        // pin 6/8

	// internal
	uint8_t m_last_clock;
	uint8_t m_last_output;
	uint8_t m_last_output_comp;

	void update();
	void init();
};


// device type definition
DECLARE_DEVICE_TYPE(TTL7474, ttl7474_device)

#endif // MAME_MACHINE_TTL7474_H
