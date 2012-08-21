/**********************************************************************

    SMC KR2376 Keyboard Encoder emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   Vcc   1 |*    \_/     | 40  Frequency Control A
   Frequency Control B   2 |             | 39  X0
   Frequency Control C   3 |             | 38  X1
           Shift Input   4 |             | 37  X2
         Control Input   5 |             | 36  X3
   Parity Invert Input   6 |             | 35  X4
         Parity Output   7 |             | 34  X5
        Data Output B8   8 |             | 33  X6
        Data Output B7   9 |             | 32  X7
        Data Output B6  10 |   KR2376    | 31  Y0
        Data Output B5  11 |             | 30  Y1
        Data Output B4  12 |             | 29  Y2
        Data Output B3  13 |             | 28  Y3
        Data Output B2  14 |             | 27  Y4
        Data Output B1  15 |             | 26  Y5
         Strobe Output  16 |             | 25  Y6
                Ground  17 |             | 24  Y7
                   Vgg  18 |             | 23  Y8
  Strobe Control Input  19 |             | 22  Y9
          Invert Input  20 |_____________| 21  Y10

**********************************************************************/

#ifndef __KR2376__
#define __KR2376__

typedef void (*kr2376_on_strobe_changed_func) (device_t *device, int level);
#define KR2376_ON_STROBE_CHANGED(name) void name(device_t *device, int level)

DECLARE_LEGACY_DEVICE(KR2376, kr2376);

#define MCFG_KR2376_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, KR2376, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_KR2376_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag, KR2376)

/*
 * Input pins
 */
typedef enum
{
	KR2376_DSII=20,			/* DSII  - Pin 20 - Data & Strobe Invert Input */
	KR2376_PII=6			/* PII   - Pin  6 - Parity Invert Input */
} kr2376_input_pin_t;

typedef enum
{
	KR2376_SO=16,			/* SO    - Pin 16 - Strobe Output */
	KR2376_PO=7			/* PO    - Pin  7 - Parity Output */
} kr2376_output_pin_t;

/* interface */
typedef struct _kr2376_interface kr2376_interface;
struct _kr2376_interface
{
	/* The clock of the chip (Typical 50 kHz) */
	int clock;

	/* This will be called for every change of the strobe pin (pin 16). Optional */
//  kr2376_on_strobe_changed_func       on_strobe_changed;
	write8_device_func	on_strobe_changed;
};
#define KR2376_INTERFACE(name) const kr2376_interface (name)=
/* keyboard matrix */
INPUT_PORTS_EXTERN( kr2376 );

/* keyboard data */
READ8_DEVICE_HANDLER( kr2376_data_r );

/* Set an input pin */
void kr2376_set_input_pin( device_t *device, kr2376_input_pin_t pin, int data );


/* Get an output pin */
int kr2376_get_output_pin( device_t *device, kr2376_output_pin_t pin );

#endif
