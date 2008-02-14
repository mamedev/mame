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

#define MAX_TTL74123 4


/* constants for the different ways the cap/res can be connected.
   This determines the formula for calculating the pulse width */
#define TTL74123_NOT_GROUNDED_NO_DIODE		(1)
#define TTL74123_NOT_GROUNDED_DIODE			(2)
#define TTL74123_GROUNDED					(3)


typedef struct _TTL74123_interface TTL74123_interface;
struct _TTL74123_interface
{
	int connection_type;	/* the hook up type - one of the constants above */
	double res;				/* resistor connected to RCext */
	double cap;				/* capacitor connected to Cext and RCext */
	int A;					/* initial/constant value of the A pin */
	int B;					/* initial/constant value of the B pin */
	int clear;				/* initial/constant value of the Clear pin */
	void (*output_changed_cb)(int output);
};


void TTL74123_config(int which, const TTL74123_interface *intf);
void TTL74123_reset(int which);

void TTL74123_A_w(int which, int data);
void TTL74123_B_w(int which, int data);
void TTL74123_clear_w(int which, int data);


#endif
