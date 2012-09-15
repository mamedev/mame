/****************************************************************************

    ay31015.c by Robbbert, May 2008. Bugs fixed by Judge.

    Code for the AY-3-1014A, AY-3-1015(D), AY-5-1013(A), and AY-6-1013 UARTs
    The HD6402 UART is compatible with the AY-3-1015 UART.

    This is cycle-accurate according to the specifications.

    It supports independent receive and transmit clocks,
    and transmission and reception can occur simultaneously if desired.

*****************************************************************************

Differences between the chip types:
- All units have pull-up resistors on the inputs, except for the AY-3-1014A which is CMOS-compatible.
- AY-3-1014A and AY-3-1015 - 1.5 stop bits mode available.
- Max baud rate of 30k, except AY-5-1013 which has 20k.
- AY-5-1013 has extended temperature ratings.
- AY-5-1013 and AY-6-1013 require a -12 volt supply on pin 2. Pin is not used otherwise.
- AY-5-1013 and AY-6-1013 do not reset the received data register when XR pin is used.

******************************************************************************

It is not clear in the documentation as to which settings will reset the device.
    To be safe, we will always reset whenever the control register changes.

    Also, it not clear exactly what happens under various error conditions.

********************************************************************************

Device Data:

* Common Controls:
-- Pin 1 - Vcc - 5 volts
-- Pin 2 - not used (on AY-5-1013 and AY-6-1013 this is Voo = -12 volts)
-- Pin 3 - Gnd - 0 volts
-- Pin 21 - XR - External Reset - resets all registers to initial state except for the control register
-- Pin 35 - NP - No Parity - "1" will kill any parity processing
-- Pin 36 - TSB - Number of Stop Bits - "0" = 1 stop bit; "1" = 2 stop bits. If "1", and 5 bits per character, then we have 1.5 stop bits
-- pin 37 - NB1
-- pin 38 - NB2 - Number of bits per character = NB1 + (NB2 * 2) + 5
-- pin 39 - EPS - Odd or Even Parity Select - "0" = Odd parity; "1" = Even parity. Has no effect if NP is high.
-- Pin 34 - CS - Control Strobe - Read NP, TSB, EPS, NB1, NB2 into the control register.

Format of data stream:
Start bit (low), Bit 0, Bit 1... highest bit, Parity bit (if enabled), 1-2 stop bits (high)


* Receiver Controls:
-- Pin 17 - RCP - Clock which is 16x the desired baud rate
-- Pin 20 - SI - Serial input stream - "1" = Mark (waiting for input), "0" = Space (Start bit) initiates the transfer of a byte
-- Pin 4 - RDE - "0" causes the received data to appear on RD1 to RD8.
-- Pins 5 to 12 - RD8 to RD1 - These are the data lines (bits 7 to 0). Data is right-justified.
-- Pin 16 - SWE - Status word enable - causes the status bits (PE, FE, OR, DAV, TBMT) to appear at the pins.
-- Pin 19 - DAV - "1" indicates that a byte has been received by the UART, and should now be accepted by the computer
-- Pin 18 - RDAV - "0" will force DAV low.
-- Pin 13 - PE - Parity error - "1" indicates that a parity error occurred
-- Pin 14 - FE - Framing error - "1" Indicates that the stop bit was missing
-- Pin 15 - OR - overrun - "1" indicates that a new character has become available before the computer had accepted the previous character

* Transmitter controls:
-- Pin 40 - TCP - Clock which is 16x the desired baud rate
-- Pin 25 - SO - Serial output stream - it will stay at "1" while no data is being transmitted
-- Pins 26 to 33 - DB1 to DB8 - These are the data lines containing the byte to be sent
-- Pin 23 - DS - Data Strobe - "0" will copy DB1 to DB8 into the transmit buffer
-- Pin 22 - TBMT - Transmit buffer Empty - "1" indicates to the computer that another byte may be sent to the UART
-- Pin 24 - EOC - End of Character - "0" means that a character is being sent.

******************************************* COMMON CONTROLS ********************************************************/

#include "emu.h"
#include "ay31015.h"

typedef enum
{
	IDLE,
	START_BIT,
	PROCESSING,
	PARITY_BIT,
	FIRST_STOP_BIT,
	SECOND_STOP_BIT,
	PREP_TIME
} state_t;


struct ay31015_t
{
	const ay31015_config	*config;

	int	pins[41];

	UINT8 control_reg;
	UINT8 status_reg;
	UINT16 second_stop_bit;	// 0, 8, 16
	UINT16 total_pulses;	// bits * 16
	UINT8 internal_sample;

	state_t rx_state;
	UINT8 rx_data;		// byte being received
	UINT8 rx_buffer;	// received byte waiting to be accepted by computer
	UINT8 rx_bit_count;
	UINT8 rx_parity;
	UINT16 rx_pulses;	// total pulses left
	double rx_clock;
	emu_timer *rx_timer;

	state_t tx_state;
	UINT8 tx_data;		// byte being sent
	UINT8 tx_buffer;	// next byte to send
	UINT8 tx_parity;
	UINT16 tx_pulses;	// total pulses left
	double tx_clock;
	emu_timer *tx_timer;
};


/* control reg */
#define CONTROL_NB1		0x01
#define CONTROL_NB2		0x02
#define CONTROL_TSB		0x04
#define CONTROL_EPS		0x08
#define CONTROL_NP		0x10


/* status reg */
#define STATUS_TBMT		0x01
#define STATUS_DAV		0x02
#define STATUS_OR		0x04
#define STATUS_FE		0x08
#define STATUS_PE		0x10
#define STATUS_EOC		0x20


/*-------------------------------------------------
    get_safe_token - safely gets the data
-------------------------------------------------*/

INLINE ay31015_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == AY31015);
	return (ay31015_t *) downcast<ay31015_device *>(device)->token();
}


INLINE UINT8 ay31015_get_si( device_t *device )
{
	ay31015_t	*ay31015 = get_safe_token( device );

	if ( ay31015->config->read_si )
		ay31015->pins[AY31015_SI] = (ay31015->config->read_si)( device, 0 ) ? 1 : 0;

	return ay31015->pins[AY31015_SI];
}


INLINE void ay31015_set_so( device_t *device, int data )
{
	ay31015_t	*ay31015 = get_safe_token( device );

	ay31015->pins[AY31015_SO] = data ? 1 : 0;

	if ( ay31015->config->write_so )
		(ay31015->config->write_so)( device, 0, ay31015->pins[AY31015_SO] );
}


INLINE int ay31015_update_status_pin( ay31015_t *ay31015, UINT8 reg_bit, ay31015_output_pin_t pin )
{
	int new_value = ( ay31015->status_reg & reg_bit ) ? 1 : 0;

	if ( new_value == ay31015->pins[pin] )
		return 0;

	ay31015->pins[pin] = new_value;
	return 1;
}


/*-------------------------------------------------
    ay31015_update_status_pins - Update the status pins
-------------------------------------------------*/
static void ay31015_update_status_pins( device_t *device )
{
	ay31015_t	*ay31015 = get_safe_token( device );
	int status_pins_changed = 0;

	/* Should status pins be updated? */
	if ( ! ay31015->pins[AY31015_SWE] )
	{
		status_pins_changed += ay31015_update_status_pin( ay31015, STATUS_PE, AY31015_PE );
		status_pins_changed += ay31015_update_status_pin( ay31015, STATUS_FE, AY31015_FE );
		status_pins_changed += ay31015_update_status_pin( ay31015, STATUS_OR, AY31015_OR );
		status_pins_changed += ay31015_update_status_pin( ay31015, STATUS_DAV, AY31015_DAV );
		status_pins_changed += ay31015_update_status_pin( ay31015, STATUS_TBMT, AY31015_TBMT );
	}
	status_pins_changed += ay31015_update_status_pin( ay31015, STATUS_EOC, AY31015_EOC );

	if ( status_pins_changed && ay31015->config->status_changed )
	{
		(ay31015->config->status_changed)( device, 0, status_pins_changed );
	}
}


/*************************************************** RECEIVE CONTROLS *************************************************/


/*-------------------------------------------------
    ay31015_rx_process - convert serial to parallel
-------------------------------------------------*/
static TIMER_CALLBACK( ay31015_rx_process )
{
	device_t *device = (device_t *)ptr;
	ay31015_t			*ay31015 = get_safe_token( device );

	switch (ay31015->rx_state)
	{
		case PREP_TIME:							// assist sync by ensuring high bit occurs
			ay31015->rx_pulses--;
			if (ay31015_get_si( device ))
				ay31015->rx_state = IDLE;
			return;

		case IDLE:
			ay31015->rx_pulses--;
			if (!ay31015_get_si( device ))
			{
				ay31015->rx_state = START_BIT;
				ay31015->rx_pulses = 16;
			}
			return;

		case START_BIT:
			ay31015->rx_pulses--;
			if (ay31015->rx_pulses == 8)			// start bit must be low at sample time
			{
				if ( ay31015_get_si( device ) )
					ay31015->rx_state = IDLE;
			}
			else
			if (!ay31015->rx_pulses)					// end of start bit
			{
				ay31015->rx_state = PROCESSING;
				ay31015->rx_pulses = ay31015->total_pulses;
				ay31015->rx_bit_count = 0;
				ay31015->rx_parity = 0;
				ay31015->rx_data = 0;
			}
			return;

		case PROCESSING:
			ay31015->rx_pulses--;
			if (!ay31015->rx_pulses)					// end of a byte
			{
				ay31015->rx_pulses = 16;
				if (ay31015->control_reg & CONTROL_NP)		// see if we need to get a parity bit
					ay31015->rx_state = FIRST_STOP_BIT;
				else
					ay31015->rx_state = PARITY_BIT;
			}
			else
			if (!(ay31015->rx_pulses & 15))				// end of a bit
				ay31015->rx_bit_count++;
			else
			if ((ay31015->rx_pulses & 15) == 8)				// sample input stream
			{
				ay31015->internal_sample = ay31015_get_si( device );
				ay31015->rx_parity ^= ay31015->internal_sample;		// calculate cumulative parity
				ay31015->rx_data |= ay31015->internal_sample << ay31015->rx_bit_count;
			}
			return;

		case PARITY_BIT:
			ay31015->rx_pulses--;

			if (ay31015->rx_pulses == 8)					// sample input stream
			{
				ay31015->rx_parity ^= ay31015_get_si( device );				// calculate cumulative parity
			}
			else
			if (!ay31015->rx_pulses)					// end of a byte
			{
				ay31015->rx_pulses = 16;
				ay31015->rx_state = FIRST_STOP_BIT;

				if ((!(ay31015->control_reg & CONTROL_EPS)) && (ay31015->rx_parity))
					ay31015->rx_parity = 0;			// odd parity, ok
				else
				if ((ay31015->control_reg & CONTROL_EPS) && (!ay31015->rx_parity))
					ay31015->rx_parity = 0;			// even parity, ok
				else
					ay31015->rx_parity = 1;			// parity error
			}
			return;

		case FIRST_STOP_BIT:
			ay31015->rx_pulses--;
			if (ay31015->rx_pulses == 8)				// sample input stream
				ay31015->internal_sample = ay31015_get_si( device );
			else
			if (ay31015->rx_pulses == 7)				// set error flags
			{
				if (!ay31015->internal_sample)
				{
					ay31015->status_reg |= STATUS_FE;		// framing error - the stop bit not high
					ay31015->rx_state = PREP_TIME;		// lost sync - start over
			//      return;
				}
				else
					ay31015->status_reg &= ~STATUS_FE;

				if ((ay31015->rx_parity) && (!(ay31015->control_reg & CONTROL_NP)))
					ay31015->status_reg |= STATUS_PE;		// parity error
				else
					ay31015->status_reg &= ~STATUS_PE;

				if (ay31015->status_reg & STATUS_DAV)
					ay31015->status_reg |= STATUS_OR;		// overrun error - previous byte still in buffer
				else
					ay31015->status_reg &= ~STATUS_OR;

				ay31015->rx_buffer = ay31015->rx_data;		// bring received byte out for computer to read

				ay31015_update_status_pins( device );
			}
			else
			if (ay31015->rx_pulses == 6)
			{
				ay31015->status_reg |= STATUS_DAV;		// tell computer that new byte is ready
				ay31015_update_status_pins( device );
			}
			else
			if (ay31015->rx_pulses == 4)
			{
				if (ay31015->second_stop_bit)
				{
					/* We should wait for the full first stop bit and
                       the beginning of the second stop bit */
					ay31015->rx_state = SECOND_STOP_BIT;
					ay31015->rx_pulses += ay31015->second_stop_bit - 7;
				}
				else
				{
					/* We have seen a STOP bit, go back to PREP_TIME */
					ay31015->rx_state = PREP_TIME;
				}
			}
			return;

		case SECOND_STOP_BIT:
			ay31015->rx_pulses--;
			if (!ay31015->rx_pulses)
				ay31015->rx_state = PREP_TIME;
			return;

	}
}


/*************************************************** TRANSMIT CONTROLS *************************************************/


/*-------------------------------------------------
    ay31015_tx_process - convert parallel to serial
-------------------------------------------------*/
static TIMER_CALLBACK( ay31015_tx_process )
{
	device_t *device = (device_t *)ptr;
	ay31015_t			*ay31015 = get_safe_token( device );

	UINT8 t1;
	switch (ay31015->tx_state)
	{
		case IDLE:
			if (!(ay31015->status_reg & STATUS_TBMT))
			{
				ay31015->tx_state = PREP_TIME;		// When idle, see if a byte has been sent to us
				ay31015->tx_pulses = 1;
			}
			return;

		case PREP_TIME:						// This phase lets the transmitter regain sync after an idle period
			ay31015->tx_pulses--;
			if (!ay31015->tx_pulses)
			{
				ay31015->tx_state = START_BIT;
				ay31015->tx_pulses = 16;
			}
			return;

		case START_BIT:
			if (ay31015->tx_pulses == 16)				// beginning of start bit
			{
				ay31015->tx_data = ay31015->tx_buffer;			// load the shift register
				ay31015->status_reg |= STATUS_TBMT;			// tell computer that another byte can be sent to uart
				ay31015_set_so( device, 0 );				/* start bit begins now (we are "spacing") */
				ay31015->status_reg &= ~STATUS_EOC;			// we are no longer idle
				ay31015->tx_parity = 0;
				ay31015_update_status_pins( device );
			}

			ay31015->tx_pulses--;
			if (!ay31015->tx_pulses)					// end of start bit
			{
				ay31015->tx_state = PROCESSING;
				ay31015->tx_pulses = ay31015->total_pulses;
			}
			return;

		case PROCESSING:
			if (!(ay31015->tx_pulses & 15))				// beginning of a data bit
			{
				if (ay31015->tx_data & 1)
				{
					ay31015_set_so( device, 1 );
					ay31015->tx_parity++;				// calculate cumulative parity
				}
				else
					ay31015_set_so( device, 0 );

				ay31015->tx_data >>= 1;				// adjust the shift register
			}

			ay31015->tx_pulses--;
			if (!ay31015->tx_pulses)					// all data bits sent
			{
				ay31015->tx_pulses = 16;
				if (ay31015->control_reg & CONTROL_NP)		// see if we need to make a parity bit
					ay31015->tx_state = FIRST_STOP_BIT;
				else
					ay31015->tx_state = PARITY_BIT;
			}

			return;

		case PARITY_BIT:
			if (ay31015->tx_pulses == 16)
			{
				t1 = (ay31015->control_reg & CONTROL_EPS) ? 0 : 1;
				t1 ^= (ay31015->tx_parity & 1);
				if (t1)
					ay31015_set_so( device, 1 );			/* extra bit to set the correct parity */
				else
					ay31015_set_so( device, 0 );			/* it was already correct */
			}

			ay31015->tx_pulses--;
			if (!ay31015->tx_pulses)
			{
				ay31015->tx_state = FIRST_STOP_BIT;
				ay31015->tx_pulses = 16;
			}
			return;

		case FIRST_STOP_BIT:
			if (ay31015->tx_pulses == 16)
				ay31015_set_so( device, 1 );				/* create a stop bit (marking and soon idle) */
			ay31015->tx_pulses--;
			if (!ay31015->tx_pulses)
			{
				ay31015->status_reg |= STATUS_EOC;			// character is completely sent
				if (ay31015->second_stop_bit)
				{
					ay31015->tx_state = SECOND_STOP_BIT;
					ay31015->tx_pulses = ay31015->second_stop_bit;
				}
				else
				if (ay31015->status_reg & STATUS_TBMT)
					ay31015->tx_state = IDLE;			// if nothing to send, go idle
				else
				{
					ay31015->tx_pulses = 16;
					ay31015->tx_state = START_BIT;		// otherwise immediately start next byte
				}
				ay31015_update_status_pins( device );
			}
			return;

		case SECOND_STOP_BIT:
			ay31015->tx_pulses--;
			if (!ay31015->tx_pulses)
			{
				if (ay31015->status_reg & STATUS_TBMT)
					ay31015->tx_state = IDLE;			// if nothing to send, go idle
				else
				{
					ay31015->tx_pulses = 16;
					ay31015->tx_state = START_BIT;		// otherwise immediately start next byte
				}
			}
			return;

	}
}


/*-------------------------------------------------
    ay31015_reset - reset internal state
-------------------------------------------------*/
static void ay31015_reset( device_t *device )
{
	ay31015_t	*ay31015 = get_safe_token( device );

	/* total pulses = 16 * data-bits */
	UINT8 t1;

	if ( ay31015->control_reg & CONTROL_NB2 )
		t1 = ( ay31015->control_reg & CONTROL_NB1 ) ? 8 : 7;
	else
		t1 = ( ay31015->control_reg & CONTROL_NB1 ) ? 6 : 5;

	ay31015->total_pulses = t1 << 4;					/* total clock pulses to load a byte */
	ay31015->second_stop_bit = ((ay31015->control_reg & CONTROL_TSB) ? 16 : 0);		/* 2nd stop bit */
	if ((t1 == 5) && (ay31015->second_stop_bit == 16))
		ay31015->second_stop_bit = 8;				/* 5 data bits and 2 stop bits = 1.5 stop bits */
	ay31015->status_reg = STATUS_EOC | STATUS_TBMT;
	ay31015->tx_data = 0;
	ay31015->rx_state = PREP_TIME;
	ay31015->tx_state = IDLE;
	ay31015->pins[AY31015_SI] = 1;
	ay31015_set_so( device, 1 );

	if ( ay31015->config->type == AY_3_1015 )
		ay31015->rx_data = 0;

}


/*-------------------------------------------------
    ay31015_transfer_control_pins - transfers contents of controls pins to the control register
-------------------------------------------------*/
static void ay31015_transfer_control_pins( device_t *device )
{
	ay31015_t	*ay31015 = get_safe_token( device );
	UINT8 control = 0;

	control |= ay31015->pins[AY31015_NP ] ? CONTROL_NP  : 0;
	control |= ay31015->pins[AY31015_TSB] ? CONTROL_TSB : 0;
	control |= ay31015->pins[AY31015_NB1] ? CONTROL_NB1 : 0;
	control |= ay31015->pins[AY31015_NB2] ? CONTROL_NB2 : 0;
	control |= ay31015->pins[AY31015_EPS] ? CONTROL_EPS : 0;

	if ( ay31015->control_reg != control )
	{
		ay31015->control_reg = control;
		ay31015_reset( device );
	}
}


/*-------------------------------------------------
    ay31015_set_input_pin - set an input pin
-------------------------------------------------*/
void ay31015_set_input_pin( device_t *device, ay31015_input_pin_t pin, int data )
{
	ay31015_t	*ay31015 = get_safe_token(device);

	data = data ? 1 : 0;

	switch ( pin )
	{
	case AY31015_SWE:
		ay31015->pins[pin] = data;
		ay31015_update_status_pins( device );
		break;
	case AY31015_RDAV:
		ay31015->pins[pin] = data;
		if ( ! data )
		{
			ay31015->status_reg &= ~STATUS_DAV;
			ay31015->pins[AY31015_DAV] = 0;
		}
		break;
	case AY31015_SI:
		ay31015->pins[pin] = data;
		break;
	case AY31015_XR:
		ay31015->pins[pin] = data;
		if ( data )
			ay31015_reset( device );
		break;
	case AY31015_CS:
	case AY31015_NP:
	case AY31015_TSB:
	case AY31015_NB1:
	case AY31015_NB2:
	case AY31015_EPS:
		ay31015->pins[pin] = data;
		if ( ay31015->pins[AY31015_CS] )
			ay31015_transfer_control_pins( device );
		break;
	}
}


/*-------------------------------------------------
    ay31015_get_output_pin - get the status of an output pin
-------------------------------------------------*/
int ay31015_get_output_pin( device_t *device, ay31015_output_pin_t pin )
{
	ay31015_t	*ay31015 = get_safe_token(device);

	return ay31015->pins[pin];
}


INLINE void ay31015_update_rx_timer( device_t *device )
{
	ay31015_t	*ay31015 = get_safe_token( device );

	if ( ay31015->rx_clock > 0.0 )
	{
		ay31015->rx_timer->adjust( attotime::from_hz( ay31015->rx_clock ), 0, attotime::from_hz( ay31015->rx_clock ) );
	}
	else
	{
		ay31015->rx_timer->enable( 0 );
	}
}


INLINE void ay31015_update_tx_timer( device_t *device )
{
	ay31015_t	*ay31015 = get_safe_token( device );

	if ( ay31015->tx_clock > 0.0 )
	{
		ay31015->tx_timer->adjust( attotime::from_hz( ay31015->tx_clock ), 0, attotime::from_hz( ay31015->tx_clock ) );
	}
	else
	{
		ay31015->tx_timer->enable( 0 );
	}
}


/*-------------------------------------------------
    ay31015_set_receiver_clock - set receive clock
-------------------------------------------------*/
void ay31015_set_receiver_clock( device_t *device, double new_clock )
{
	ay31015_t	*ay31015 = get_safe_token(device);

	ay31015->rx_clock = new_clock;
	ay31015_update_rx_timer( device );
}


/*-------------------------------------------------
    ay31015_set_transmitter_clock - set transmit clock
-------------------------------------------------*/
void ay31015_set_transmitter_clock( device_t *device, double new_clock )
{
	ay31015_t	*ay31015 = get_safe_token(device);

	ay31015->tx_clock = new_clock;
	ay31015_update_tx_timer( device );
}


/*-------------------------------------------------
    ay31015_get_received_data - return a byte to the computer
-------------------------------------------------*/
UINT8 ay31015_get_received_data( device_t *device )
{
	ay31015_t	*ay31015 = get_safe_token(device);

	return ay31015->rx_buffer;
}


/*-------------------------------------------------
    ay31015_set_transmit_data - accept a byte to transmit, if able
-------------------------------------------------*/
void ay31015_set_transmit_data( device_t *device, UINT8 data )
{
	ay31015_t	*ay31015 = get_safe_token(device);

	if (ay31015->status_reg & STATUS_TBMT)
	{
		ay31015->tx_buffer = data;
		ay31015->status_reg &= ~STATUS_TBMT;
		ay31015_update_status_pins( device );
	}
}


static DEVICE_START(ay31015)
{
	ay31015_t	*ay31015 = get_safe_token(device);

	ay31015->config = (const ay31015_config*)device->static_config();

	ay31015->tx_clock = ay31015->config->transmitter_clock;
	ay31015->rx_clock = ay31015->config->receiver_clock;

	ay31015->rx_timer = device->machine().scheduler().timer_alloc(FUNC(ay31015_rx_process), (void *)device );
	ay31015->tx_timer = device->machine().scheduler().timer_alloc(FUNC(ay31015_tx_process), (void *)device );

	ay31015_update_rx_timer( device );
	ay31015_update_tx_timer( device );
}


static DEVICE_RESET( ay31015 )
{
	ay31015_t	*ay31015 = get_safe_token(device);

	ay31015->control_reg = 0;
	ay31015->rx_data = 0;

	ay31015_reset( device );
}


const device_type AY31015 = &device_creator<ay31015_device>;

ay31015_device::ay31015_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AY31015, "AY-3-1015", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(ay31015_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ay31015_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ay31015_device::device_start()
{
	DEVICE_START_NAME( ay31015 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ay31015_device::device_reset()
{
	DEVICE_RESET_NAME( ay31015 )(this);
}


