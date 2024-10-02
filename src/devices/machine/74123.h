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

#ifndef MAME_MACHINE_74123_H
#define MAME_MACHINE_74123_H

#pragma once

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
	ttl74123_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	ttl74123_device(const machine_config &mconfig, const char *tag, device_t *owner, double res, double cap)
		: ttl74123_device(mconfig, tag, owner, 0U)
	{
		set_resistor_value(res);
		set_capacitor_value(cap);
	}

	void set_connection_type(int type) { m_connection_type = type; }
	void set_resistor_value(double value) { m_res = value; }
	void set_capacitor_value(double value) { m_cap = value; }
	void set_a_pin_value(int value) { m_a = value; }
	void set_b_pin_value(int value) { m_b = value; }
	void set_clear_pin_value(int value) { m_clear = value; }

	auto out_cb() { return m_output_changed_cb.bind(); }

	void a_w(int state);
	void b_w(int state);
	void clear_w(int state);
	void reset_w(int state);

	int q_r() { return timer_running(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

	TIMER_CALLBACK_MEMBER( output_callback );
	TIMER_CALLBACK_MEMBER( clear_callback );

private:
	int timer_running();
	void start_pulse();
	void set_output();
	attotime compute_duration();

	emu_timer *m_clear_timer;
	emu_timer *m_output_timer;
	int m_connection_type;  /* the hook up type - one of the constants above */
	double m_res;           /* resistor connected to RCext */
	double m_cap;           /* capacitor connected to Cext and RCext */
	int m_a;                /* initial/constant value of the A pin */
	int m_b;                /* initial/constant value of the B pin */
	int m_clear;            /* initial/constant value of the Clear pin */
	devcb_write_line  m_output_changed_cb;
};


// device type definition
DECLARE_DEVICE_TYPE(TTL74123, ttl74123_device)

#endif // MAME_MACHINE_74123_H
