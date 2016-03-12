// license:BSD-3-Clause
// copyright-holders:Robbbert
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
-- pin 37 - NB2
-- pin 38 - NB1 - Number of bits per character = NB1 + (NB2 * 2) + 5
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



/* control reg */
#define CONTROL_NB1     0x01
#define CONTROL_NB2     0x02
#define CONTROL_TSB     0x04
#define CONTROL_EPS     0x08
#define CONTROL_NP      0x10


/* status reg */
#define STATUS_TBMT     0x01
#define STATUS_DAV      0x02
#define STATUS_OR       0x04
#define STATUS_FE       0x08
#define STATUS_PE       0x10
#define STATUS_EOC      0x20



const device_type AY31015 = &device_creator<ay31015_device>;
const device_type AY51013 = &device_creator<ay51013_device>;

ay31015_device::ay31015_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
				: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
				m_control_reg(0),
				m_status_reg(0),
				m_second_stop_bit(0),
				m_total_pulses(0),
				m_internal_sample(0),
				m_rx_data(0),
				m_rx_buffer(0),
				m_rx_bit_count(0),
				m_rx_parity(0),
				m_rx_pulses(0),
				m_rx_clock(0),
				m_rx_timer(nullptr),
				m_tx_data(0),
				m_tx_buffer(0),
				m_tx_parity(0),
				m_tx_pulses(0),
				m_tx_clock(0),
				m_tx_timer(nullptr),
				m_read_si_cb(*this),
				m_write_so_cb(*this),
				m_status_changed_cb(*this)
{
	for (auto & elem : m_pins)
		elem = 0;
}

ay31015_device::ay31015_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, AY31015, "AY-3-1015", tag, owner, clock, "ay31015", __FILE__),
				m_control_reg(0),
				m_status_reg(0),
				m_second_stop_bit(0),
				m_total_pulses(0),
				m_internal_sample(0),
				m_rx_data(0),
				m_rx_buffer(0),
				m_rx_bit_count(0),
				m_rx_parity(0),
				m_rx_pulses(0),
				m_rx_clock(0),
				m_rx_timer(nullptr),
				m_tx_data(0),
				m_tx_buffer(0),
				m_tx_parity(0),
				m_tx_pulses(0),
				m_tx_clock(0),
				m_tx_timer(nullptr),
				m_read_si_cb(*this),
				m_write_so_cb(*this),
				m_status_changed_cb(*this)
{
	for (auto & elem : m_pins)
		elem = 0;
}

ay51013_device::ay51013_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: ay31015_device(mconfig, AY31015, "AY-5-1013", tag, owner, clock, "ay51013", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ay31015_device::device_start()
{
	m_read_si_cb.resolve();
	m_write_so_cb.resolve();
	m_status_changed_cb.resolve();

	m_rx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ay31015_device::rx_process),this));
	m_tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ay31015_device::tx_process),this));

	update_rx_timer();
	update_tx_timer();

	save_item(NAME(m_pins));
	save_item(NAME(m_control_reg));
	save_item(NAME(m_status_reg));
	save_item(NAME(m_second_stop_bit));
	save_item(NAME(m_total_pulses));
	save_item(NAME(m_internal_sample));

	save_item(NAME(m_rx_state));
	save_item(NAME(m_rx_data));
	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_rx_bit_count));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_rx_pulses));
	save_item(NAME(m_rx_clock));

	save_item(NAME(m_tx_state));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_tx_pulses));
	save_item(NAME(m_tx_clock));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ay31015_device::device_reset()
{
	m_control_reg = 0;
	m_rx_data = 0;

	internal_reset();
}


inline UINT8 ay31015_device::get_si()
{
	if (!m_read_si_cb.isnull())
		m_pins[AY31015_SI] = m_read_si_cb(0) ? 1 : 0;

	return m_pins[AY31015_SI];
}


inline void ay31015_device::set_so( int data )
{
	m_pins[AY31015_SO] = data ? 1 : 0;

	if (!m_write_so_cb.isnull())
		m_write_so_cb((offs_t)0, m_pins[AY31015_SO]);
}


inline int ay31015_device::update_status_pin( UINT8 reg_bit, ay31015_output_pin_t pin )
{
	int new_value = (m_status_reg & reg_bit) ? 1 : 0;

	if (new_value == m_pins[pin])
		return 0;

	m_pins[pin] = new_value;
	return 1;
}


/*-------------------------------------------------
 ay31015_update_status_pins - Update the status pins
-------------------------------------------------*/

void ay31015_device::update_status_pins()
{
	int status_pins_changed = 0;

	/* Should status pins be updated? */
	if (!m_pins[AY31015_SWE])
	{
		status_pins_changed += update_status_pin(STATUS_PE, AY31015_PE);
		status_pins_changed += update_status_pin(STATUS_FE, AY31015_FE);
		status_pins_changed += update_status_pin(STATUS_OR, AY31015_OR);
		status_pins_changed += update_status_pin(STATUS_DAV, AY31015_DAV);
		status_pins_changed += update_status_pin(STATUS_TBMT, AY31015_TBMT);
	}
	status_pins_changed += update_status_pin(STATUS_EOC, AY31015_EOC);

	if (status_pins_changed && !m_status_changed_cb.isnull())
	{
		m_status_changed_cb((offs_t)0, status_pins_changed);
	}
}


/*************************************************** RECEIVE CONTROLS *************************************************/


/*-------------------------------------------------
 ay31015_rx_process - convert serial to parallel
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( ay31015_device::rx_process )
{
	switch (m_rx_state)
	{
		case PREP_TIME:                         // assist sync by ensuring high bit occurs
			m_rx_pulses--;
			if (get_si())
				m_rx_state = IDLE;
			return;

		case IDLE:
			m_rx_pulses--;
			if (!get_si())
			{
				m_rx_state = START_BIT;
				m_rx_pulses = 16;
			}
			return;

		case START_BIT:
			m_rx_pulses--;
			if (m_rx_pulses == 8)            // start bit must be low at sample time
			{
				if (get_si())
					m_rx_state = IDLE;
			}
			else
			if (!m_rx_pulses)                    // end of start bit
			{
				m_rx_state = PROCESSING;
				m_rx_pulses = m_total_pulses;
				m_rx_bit_count = 0;
				m_rx_parity = 0;
				m_rx_data = 0;
			}
			return;

		case PROCESSING:
			m_rx_pulses--;
			if (!m_rx_pulses)                    // end of a byte
			{
				m_rx_pulses = 16;
				if (m_control_reg & CONTROL_NP)      // see if we need to get a parity bit
					m_rx_state = FIRST_STOP_BIT;
				else
					m_rx_state = PARITY_BIT;
			}
			else
			if (!(m_rx_pulses & 15))             // end of a bit
				m_rx_bit_count++;
			else
			if ((m_rx_pulses & 15) == 8)             // sample input stream
			{
				m_internal_sample = get_si();
				m_rx_parity ^= m_internal_sample;     // calculate cumulative parity
				m_rx_data |= m_internal_sample << m_rx_bit_count;
			}
			return;

		case PARITY_BIT:
			m_rx_pulses--;

			if (m_rx_pulses == 8)                    // sample input stream
			{
				m_rx_parity ^= get_si();             // calculate cumulative parity
			}
			else
			if (!m_rx_pulses)                    // end of a byte
			{
				m_rx_pulses = 16;
				m_rx_state = FIRST_STOP_BIT;

				if ((!(m_control_reg & CONTROL_EPS)) && (m_rx_parity))
					m_rx_parity = 0;         // odd parity, ok
				else
				if ((m_control_reg & CONTROL_EPS) && (!m_rx_parity))
					m_rx_parity = 0;         // even parity, ok
				else
					m_rx_parity = 1;         // parity error
			}
			return;

		case FIRST_STOP_BIT:
			m_rx_pulses--;
			if (m_rx_pulses == 8)                // sample input stream
				m_internal_sample = get_si();
			else
			if (m_rx_pulses == 7)                // set error flags
			{
				if (!m_internal_sample)
				{
					m_status_reg |= STATUS_FE;       // framing error - the stop bit not high
					m_rx_state = PREP_TIME;      // lost sync - start over
			//      return;
				}
				else
					m_status_reg &= ~STATUS_FE;

				if ((m_rx_parity) && (!(m_control_reg & CONTROL_NP)))
					m_status_reg |= STATUS_PE;       // parity error
				else
					m_status_reg &= ~STATUS_PE;

				if (m_status_reg & STATUS_DAV)
					m_status_reg |= STATUS_OR;       // overrun error - previous byte still in buffer
				else
					m_status_reg &= ~STATUS_OR;

				m_rx_buffer = m_rx_data;      // bring received byte out for computer to read

				update_status_pins();
			}
			else
			if (m_rx_pulses == 6)
			{
				m_status_reg |= STATUS_DAV;      // tell computer that new byte is ready
				update_status_pins();
			}
			else
			if (m_rx_pulses == 4)
			{
				if (m_second_stop_bit)
				{
					/* We should wait for the full first stop bit and
					   the beginning of the second stop bit */
					m_rx_state = SECOND_STOP_BIT;
					m_rx_pulses += m_second_stop_bit - 7;
				}
				else
				{
					/* We have seen a STOP bit, go back to PREP_TIME */
					m_rx_state = PREP_TIME;
				}
			}
			return;

		case SECOND_STOP_BIT:
			m_rx_pulses--;
			if (!m_rx_pulses)
				m_rx_state = PREP_TIME;
			return;

	}
}


/*************************************************** TRANSMIT CONTROLS *************************************************/


/*-------------------------------------------------
 ay31015_tx_process - convert parallel to serial
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( ay31015_device::tx_process )
{
	UINT8 t1;
	switch (m_tx_state)
	{
		case IDLE:
			if (!(m_status_reg & STATUS_TBMT))
			{
				m_tx_state = PREP_TIME;      // When idle, see if a byte has been sent to us
				m_tx_pulses = 1;
			}
			return;

		case PREP_TIME:                     // This phase lets the transmitter regain sync after an idle period
			m_tx_pulses--;
			if (!m_tx_pulses)
			{
				m_tx_state = START_BIT;
				m_tx_pulses = 16;
			}
			return;

		case START_BIT:
			if (m_tx_pulses == 16)               // beginning of start bit
			{
				m_tx_data = m_tx_buffer;          // load the shift register
				m_status_reg |= STATUS_TBMT;         // tell computer that another byte can be sent to uart
				set_so(0);                /* start bit begins now (we are "spacing") */
				m_status_reg &= ~STATUS_EOC;         // we are no longer idle
				m_tx_parity = 0;
				update_status_pins();
			}

			m_tx_pulses--;
			if (!m_tx_pulses)                    // end of start bit
			{
				m_tx_state = PROCESSING;
				m_tx_pulses = m_total_pulses;
			}
			return;

		case PROCESSING:
			if (!(m_tx_pulses & 15))             // beginning of a data bit
			{
				if (m_tx_data & 1)
				{
					set_so(1);
					m_tx_parity++;               // calculate cumulative parity
				}
				else
					set_so(0);

				m_tx_data >>= 1;             // adjust the shift register
			}

			m_tx_pulses--;
			if (!m_tx_pulses)                    // all data bits sent
			{
				m_tx_pulses = 16;
				if (m_control_reg & CONTROL_NP)      // see if we need to make a parity bit
					m_tx_state = FIRST_STOP_BIT;
				else
					m_tx_state = PARITY_BIT;
			}

			return;

		case PARITY_BIT:
			if (m_tx_pulses == 16)
			{
				t1 = (m_control_reg & CONTROL_EPS) ? 0 : 1;
				t1 ^= (m_tx_parity & 1);
				if (t1)
					set_so(1);            /* extra bit to set the correct parity */
				else
					set_so(0);            /* it was already correct */
			}

			m_tx_pulses--;
			if (!m_tx_pulses)
			{
				m_tx_state = FIRST_STOP_BIT;
				m_tx_pulses = 16;
			}
			return;

		case FIRST_STOP_BIT:
			if (m_tx_pulses == 16)
				set_so(1);                /* create a stop bit (marking and soon idle) */
			m_tx_pulses--;
			if (!m_tx_pulses)
			{
				m_status_reg |= STATUS_EOC;          // character is completely sent
				if (m_second_stop_bit)
				{
					m_tx_state = SECOND_STOP_BIT;
					m_tx_pulses = m_second_stop_bit;
				}
				else
				if (m_status_reg & STATUS_TBMT)
					m_tx_state = IDLE;           // if nothing to send, go idle
				else
				{
					m_tx_pulses = 16;
					m_tx_state = START_BIT;      // otherwise immediately start next byte
				}
				update_status_pins();
			}
			return;

		case SECOND_STOP_BIT:
			m_tx_pulses--;
			if (!m_tx_pulses)
			{
				if (m_status_reg & STATUS_TBMT)
					m_tx_state = IDLE;           // if nothing to send, go idle
				else
				{
					m_tx_pulses = 16;
					m_tx_state = START_BIT;      // otherwise immediately start next byte
				}
			}
			return;

	}
}


/*-------------------------------------------------
 ay31015_reset - reset internal state
-------------------------------------------------*/

void ay31015_device::internal_reset()
{
	/* total pulses = 16 * data-bits */
	UINT8 t1;

	if (m_control_reg & CONTROL_NB2)
		t1 = (m_control_reg & CONTROL_NB1) ? 8 : 7;
	else
		t1 = (m_control_reg & CONTROL_NB1) ? 6 : 5;

	m_total_pulses = t1 << 4;                    /* total clock pulses to load a byte */
	m_second_stop_bit = ((m_control_reg & CONTROL_TSB) ? 16 : 0);     /* 2nd stop bit */
	if ((t1 == 5) && (m_second_stop_bit == 16))
		m_second_stop_bit = 8;               /* 5 data bits and 2 stop bits = 1.5 stop bits */
	m_status_reg = STATUS_EOC | STATUS_TBMT;
	m_tx_data = 0;
	m_rx_state = PREP_TIME;
	m_tx_state = IDLE;
	m_pins[AY31015_SI] = 1;
	set_so(1);

	m_rx_data = 0;
}


void ay51013_device::internal_reset()
{
	/* total pulses = 16 * data-bits */
	UINT8 t1;

	if (m_control_reg & CONTROL_NB2)
		t1 = (m_control_reg & CONTROL_NB1) ? 8 : 7;
	else
		t1 = (m_control_reg & CONTROL_NB1) ? 6 : 5;

	m_total_pulses = t1 << 4;                    /* total clock pulses to load a byte */
	m_second_stop_bit = ((m_control_reg & CONTROL_TSB) ? 16 : 0);     /* 2nd stop bit */
	if ((t1 == 5) && (m_second_stop_bit == 16))
		m_second_stop_bit = 8;               /* 5 data bits and 2 stop bits = 1.5 stop bits */
	m_status_reg = STATUS_EOC | STATUS_TBMT;
	m_tx_data = 0;
	m_rx_state = PREP_TIME;
	m_tx_state = IDLE;
	m_pins[AY31015_SI] = 1;
	set_so(1);
	// no m_rx_data = 0 in this case
}

/*-------------------------------------------------
 ay31015_transfer_control_pins - transfers contents of controls pins to the control register
-------------------------------------------------*/

void ay31015_device::transfer_control_pins()
{
	UINT8 control = 0;

	control |= m_pins[AY31015_NP ] ? CONTROL_NP  : 0;
	control |= m_pins[AY31015_TSB] ? CONTROL_TSB : 0;
	control |= m_pins[AY31015_NB1] ? CONTROL_NB1 : 0;
	control |= m_pins[AY31015_NB2] ? CONTROL_NB2 : 0;
	control |= m_pins[AY31015_EPS] ? CONTROL_EPS : 0;

	if (m_control_reg != control)
	{
		m_control_reg = control;
		internal_reset();
	}
}


/*-------------------------------------------------
 ay31015_set_input_pin - set an input pin
-------------------------------------------------*/
void ay31015_device::set_input_pin( ay31015_input_pin_t pin, int data )
{
	data = data ? 1 : 0;

	switch (pin)
	{
	case AY31015_SWE:
		m_pins[pin] = data;
		update_status_pins();
		break;
	case AY31015_RDAV:
		m_pins[pin] = data;
		if (!data)
		{
			m_status_reg &= ~STATUS_DAV;
			m_pins[AY31015_DAV] = 0;
		}
		break;
	case AY31015_SI:
		m_pins[pin] = data;
		break;
	case AY31015_XR:
		m_pins[pin] = data;
		if (data)
			internal_reset();
		break;
	case AY31015_CS:
	case AY31015_NP:
	case AY31015_TSB:
	case AY31015_NB1:
	case AY31015_NB2:
	case AY31015_EPS:
		m_pins[pin] = data;
		if (m_pins[AY31015_CS])
			transfer_control_pins();
		break;
	}
}


/*-------------------------------------------------
 ay31015_get_output_pin - get the status of an output pin
-------------------------------------------------*/

int ay31015_device::get_output_pin( ay31015_output_pin_t pin )
{
	return m_pins[pin];
}


inline void ay31015_device::update_rx_timer()
{
	if (m_rx_clock > 0.0)
	{
		m_rx_timer->adjust(attotime::from_hz(m_rx_clock), 0, attotime::from_hz(m_rx_clock));
	}
	else
	{
		m_rx_timer->enable(0);
	}
}


inline void ay31015_device::update_tx_timer()
{
	if (m_tx_clock > 0.0)
	{
		m_tx_timer->adjust(attotime::from_hz(m_tx_clock), 0, attotime::from_hz(m_tx_clock));
	}
	else
	{
		m_tx_timer->enable(0);
	}
}


/*-------------------------------------------------
 ay31015_set_receiver_clock - set receive clock
-------------------------------------------------*/

void ay31015_device::set_receiver_clock( double new_clock )
{
	m_rx_clock = new_clock;
	update_rx_timer();
}


/*-------------------------------------------------
 ay31015_set_transmitter_clock - set transmit clock
-------------------------------------------------*/

void ay31015_device::set_transmitter_clock( double new_clock )
{
	m_tx_clock = new_clock;
	update_tx_timer();
}


/*-------------------------------------------------
 ay31015_get_received_data - return a byte to the computer
-------------------------------------------------*/

UINT8 ay31015_device::get_received_data()
{
	return m_rx_buffer;
}


/*-------------------------------------------------
    ay31015_set_transmit_data - accept a byte to transmit, if able
-------------------------------------------------*/
void ay31015_device::set_transmit_data( UINT8 data )
{
	if (m_status_reg & STATUS_TBMT)
	{
		m_tx_buffer = data;
		m_status_reg &= ~STATUS_TBMT;
		update_status_pins();
	}
}
