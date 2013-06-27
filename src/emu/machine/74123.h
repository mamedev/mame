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

#define MCFG_TTL74123_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, TTL74123, 0) \
	MCFG_DEVICE_CONFIG(_config)

/* constants for the different ways the cap/res can be connected.
   This determines the formula for calculating the pulse width */
#define TTL74123_NOT_GROUNDED_NO_DIODE      (1)
#define TTL74123_NOT_GROUNDED_DIODE         (2)
#define TTL74123_GROUNDED                   (3)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> ttl74123_interface

struct ttl74123_interface
{
	int m_connection_type;  /* the hook up type - one of the constants above */
	double m_res;           /* resistor connected to RCext */
	double m_cap;           /* capacitor connected to Cext and RCext */
	int m_a;                /* initial/constant value of the A pin */
	int m_b;                /* initial/constant value of the B pin */
	int m_clear;            /* initial/constant value of the Clear pin */
	devcb_write8  m_output_changed_cb;
};



// ======================> ttl74123_device

class ttl74123_device :  public device_t,
							public ttl74123_interface
{
public:
	// construction/destruction
	ttl74123_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void a_w(UINT8 data);
	void b_w(UINT8 data);
	void clear_w(UINT8 data);
	void reset_w();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load() { }
	virtual void device_clock_changed() { }

	static TIMER_CALLBACK( output_callback );
	static TIMER_CALLBACK( clear_callback );

private:

	int timer_running();
	void start_pulse();
	void output(INT32 param);
	void set_output();
	attotime compute_duration();
	void clear();

	emu_timer *m_timer;
	devcb_resolved_write8 m_output_changed;
};


// device type definition
extern const device_type TTL74123;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

DECLARE_WRITE8_DEVICE_HANDLER( ttl74123_a_w );
DECLARE_WRITE8_DEVICE_HANDLER( ttl74123_b_w );
DECLARE_WRITE8_DEVICE_HANDLER( ttl74123_clear_w );
DECLARE_WRITE8_DEVICE_HANDLER( ttl74123_reset_w ); /* reset the latch */

#endif
