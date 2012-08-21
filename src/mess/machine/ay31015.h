/* ay31015.h

    Written for MESS by Robbbert on May 29th, 2008.

*/

#ifndef __AY31015_H_
#define __AY31015_H_

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


typedef enum
{
	/* For AY-3-1014A, AY-3-1015(D) and HD6402 variants */
	AY_3_1015,

	/* For AY-3-1014, AY-5-1013 and AY-6-1013 variants */
	AY_5_1013
} ay31015_type_t;


typedef enum
{
	AY31015_SWE=16,			/* -SWE  - Pin 16 - Status word enable */
	AY31015_RDAV=18,		/* -RDAV - Pin 18 - Reset data available */
	AY31015_SI=20,			/*  SI   - Pin 20 - Serial input */
	AY31015_XR=21,			/*  XR   - Pin 21 - External reset */
	AY31015_CS=34,			/*  CS   - Pin 34 - Control strobe */
	AY31015_NP=35,			/*  NP   - Pin 35 - No parity */
	AY31015_TSB=36,			/*  TSB  - Pin 36 - Number of stop bits */
	AY31015_NB1=37,			/*  NB1  - Pin 37 - Number of bits #1 */
	AY31015_NB2=38,			/*  NB2  - Pin 38 - Number of bits #2 */
	AY31015_EPS=39			/*  EPS  - Pin 39 - Odd/Even parity select */
} ay31015_input_pin_t;


typedef enum
{
	AY31015_PE=13,			/* PE   - Pin 13 - Parity error */
	AY31015_FE=14,			/* FE   - Pin 14 - Framing error */
	AY31015_OR=15,			/* OR   - Pin 15 - Over-run */
	AY31015_DAV=19,			/* DAV  - Pin 19 - Data available */
	AY31015_TBMT=22,		/* TBMT - Pin 22 - Transmit buffer empty */
	AY31015_EOC=24,			/* EOC  - Pin 24 - End of character */
	AY31015_SO=25			/* SO   - Pin 25 - Serial output */
} ay31015_output_pin_t;


typedef struct _ay31015_config	ay31015_config;
struct _ay31015_config
{
	ay31015_type_t		type;					/* Type of chip */
	double				transmitter_clock;		/* TCP - pin 40 */
	double				receiver_clock;			/* RCP - pin 17 */
	read8_device_func	read_si;				/* SI - pin 20 - This will be called whenever the SI pin is sampled. Optional */
	write8_device_func	write_so;				/* SO - pin 25 - This will be called whenever data is put on the SO pin. Optional */
	write8_device_func	status_changed;			/* This will be called whenever one of the status pins may have changed. Optional */
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_AY31015_ADD(_tag, _config)	\
    MCFG_DEVICE_ADD(_tag, AY31015, 0)		\
    MCFG_DEVICE_CONFIG(_config)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* Set an input pin */
void ay31015_set_input_pin( device_t *device, ay31015_input_pin_t pin, int data );


/* Get an output pin */
int ay31015_get_output_pin( device_t *device, ay31015_output_pin_t pin );


/* Set a new transmitter clock (new_clock is in Hz) */
void ay31015_set_transmitter_clock( device_t *device, double new_clock );


/* Set a new receiver clock (new_clock is in Hz) */
void ay31015_set_receiver_clock( device_t *device, double new_clock );


/* Reead the received data */
/* The received data is available on RD8-RD1 (pins 5-12) */
UINT8 ay31015_get_received_data( device_t *device );


/* Set the transmitter buffer */
/* The data to transmit is set on DB1-DB8 (pins 26-33) */
void ay31015_set_transmit_data( device_t *device, UINT8 data );


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

DECLARE_LEGACY_DEVICE(AY31015, ay31015);
#endif
