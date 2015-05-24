// license:BSD-3-Clause
// copyright-holders:Couriersud
/*****************************************************************************

    74123 monoflop emulator

    There are 2 monoflops per chips.

    Pin out:

              +--------+
          B1  |1 | | 16|  Vcc
          A1 o|2  -  15|  RCext1
      Clear1 o|3     14|  Cext1
    *Output1 o|4     13|  Output1
     Output2  |5     12|o *Output2
       Cext2  |6     11|o Clear2
      RCext2  |7     10|  B2
         GND  |8      9|o A2
              +--------+

    All resistor values in Ohms.
    All capacitor values in Farads.


    Truth table:

    C   A   B | Q  /Q
    ----------|-------
    L   X   X | L   H
    X   H   X | L   H
    X   X   L | L   H
    H   L  _- |_-_ -_-
    H  -_   H |_-_ -_-
    _-  L   H |_-_ -_-
    ------------------
    C   = clear
    L   = LO (0)
    H   = HI (1)
    X   = any state
    _-  = raising edge
    -_  = falling edge
    _-_ = positive pulse
    -_- = negative pulse

*****************************************************************************/

#pragma once

#ifndef __TTL74123_H__
#define __TTL74123_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TTL74123_CONNECTION_TYPE(_ctype) \
	ttl74123_device::set_connection_type(*device, _ctype);

#define MCFG_TTL74123_RESISTOR_VALUE(_value) \
	ttl74123_device::set_resistor_value(*device, _value);

#define MCFG_TTL74123_CAPACITOR_VALUE(_value) \
	ttl74123_device::set_capacitor_value(*device, _value);

#define MCFG_TTL74123_A_PIN_VALUE(_value) \
	ttl74123_device::set_a_pin_value(*device, _value);

#define MCFG_TTL74123_B_PIN_VALUE(_value) \
	ttl74123_device::set_b_pin_value(*device, _value);

#define MCFG_TTL74123_CLEAR_PIN_VALUE(_value) \
	ttl74123_device::set_clear_pin_value(*device, _value);

#define MCFG_TTL74123_OUTPUT_CHANGED_CB(_devcb) \
	devcb = &ttl74123_device::set_output_changed_callback(*device, DEVCB_##_devcb);

/* constants for the different ways the cap/res can be connected.
   This determines the formula for calculating the pulse width */
#define TTL74123_NOT_GROUNDED_NO_DIODE      (1)
#define TTL74123_NOT_GROUNDED_DIODE         (2)
#define TTL74123_GROUNDED                   (3)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> ttl74123_device

class ttl74123_device :  public device_t
{
public:
	// construction/destruction
	ttl74123_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_connection_type(device_t &device, int type) { downcast<ttl74123_device &>(device).m_connection_type = type; }
	static void set_resistor_value(device_t &device, double value) { downcast<ttl74123_device &>(device).m_res = value; }
	static void set_capacitor_value(device_t &device, double value) { downcast<ttl74123_device &>(device).m_cap = value; }
	static void set_a_pin_value(device_t &device, int value) { downcast<ttl74123_device &>(device).m_a = value; }
	static void set_b_pin_value(device_t &device, int value) { downcast<ttl74123_device &>(device).m_b = value; }
	static void set_clear_pin_value(device_t &device, int value) { downcast<ttl74123_device &>(device).m_clear = value; }
	template<class _Object> static devcb_base &set_output_changed_callback(device_t &device, _Object object) { return downcast<ttl74123_device &>(device).m_output_changed_cb.set_callback(object); }

	DECLARE_WRITE8_MEMBER(a_w);
	DECLARE_WRITE8_MEMBER(b_w);
	DECLARE_WRITE8_MEMBER(clear_w);
	DECLARE_WRITE8_MEMBER(reset_w);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load() { }
	virtual void device_clock_changed() { }

	TIMER_CALLBACK_MEMBER( output_callback );
	TIMER_CALLBACK_MEMBER( clear_callback );

private:

	int timer_running();
	void start_pulse();
	void output(INT32 param);
	void set_output();
	attotime compute_duration();
	void clear();

	emu_timer *m_timer;
	int m_connection_type;  /* the hook up type - one of the constants above */
	double m_res;           /* resistor connected to RCext */
	double m_cap;           /* capacitor connected to Cext and RCext */
	int m_a;                /* initial/constant value of the A pin */
	int m_b;                /* initial/constant value of the B pin */
	int m_clear;            /* initial/constant value of the Clear pin */
	devcb_write8  m_output_changed_cb;
};


// device type definition
extern const device_type TTL74123;

#endif
