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

#pragma once

#ifndef __TTL7474_H__
#define __TTL7474_H__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_7474_OUTPUT_CB(_devcb) \
	devcb = &ttl7474_device::set_output_cb(*device, DEVCB_##_devcb);

#define MCFG_7474_COMP_OUTPUT_CB(_devcb) \
	devcb = &ttl7474_device::set_comp_output_cb(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ttl7474_device

class ttl7474_device : public device_t
{
public:
	// construction/destruction
	ttl7474_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_output_cb(device_t &device, _Object object) { return downcast<ttl7474_device &>(device).m_output_func.set_callback(object); }
	template<class _Object> static devcb_base &set_comp_output_cb(device_t &device, _Object object) { return downcast<ttl7474_device &>(device).m_comp_output_func.set_callback(object); }

	// public interfaces
	DECLARE_WRITE_LINE_MEMBER( clear_w );
	DECLARE_WRITE_LINE_MEMBER( preset_w );
	DECLARE_WRITE_LINE_MEMBER( clock_w );
	DECLARE_WRITE_LINE_MEMBER( d_w );
	DECLARE_READ_LINE_MEMBER( output_r );
	DECLARE_READ_LINE_MEMBER( output_comp_r );    // NOT strictly the same as !output_r()

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load() { }
	virtual void device_clock_changed() { }
private:
	// callbacks
	devcb_write_line m_output_func;
	devcb_write_line m_comp_output_func;

	// inputs
	UINT8 m_clear;              // pin 1/13
	UINT8 m_preset;             // pin 4/10
	UINT8 m_clk;                // pin 3/11
	UINT8 m_d;                  // pin 2/12

	// outputs
	UINT8 m_output;             // pin 5/9
	UINT8 m_output_comp;        // pin 6/8

	// internal
	UINT8 m_last_clock;
	UINT8 m_last_output;
	UINT8 m_last_output_comp;

	void update();
	void init();
};


// device type definition
extern const device_type TTL7474;


#endif /* __TTL7474_H__ */
