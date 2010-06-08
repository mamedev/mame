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

#ifndef TTL74123_H
#define TTL74123_H

#include "devlegcy.h"

DECLARE_LEGACY_DEVICE(TTL74123, ttl74123);

#define MDRV_TTL74123_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, TTL74123, 0) \
	MDRV_DEVICE_CONFIG(_config)


/* constants for the different ways the cap/res can be connected.
   This determines the formula for calculating the pulse width */
#define TTL74123_NOT_GROUNDED_NO_DIODE		(1)
#define TTL74123_NOT_GROUNDED_DIODE			(2)
#define TTL74123_GROUNDED					(3)


typedef struct _ttl74123_config ttl74123_config;
struct _ttl74123_config
{
	int connection_type;	/* the hook up type - one of the constants above */
	double res;				/* resistor connected to RCext */
	double cap;				/* capacitor connected to Cext and RCext */
	int a;					/* initial/constant value of the A pin */
	int b;					/* initial/constant value of the B pin */
	int clear;				/* initial/constant value of the Clear pin */
	write8_device_func	output_changed_cb;
};

/* write inputs */

WRITE8_DEVICE_HANDLER( ttl74123_a_w );
WRITE8_DEVICE_HANDLER( ttl74123_b_w );
WRITE8_DEVICE_HANDLER( ttl74123_clear_w );

/* reset the latch */

WRITE8_DEVICE_HANDLER( ttl74123_reset_w );

#endif
