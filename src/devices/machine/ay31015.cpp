// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************

    ay31015.c by Robbbert, May 2008. Bugs fixed by Judge.

    Code for the General Instruments AY-3-1014A, AY-3-1015(D), AY-5-1013(A),
    and AY-6-1013 UARTs (Universal Asynchronous Receiver/Transmitters).

    Compatible UARTs were produced by Harris (HD6402), TI (TMS6011),
    Western Digital (TR1602/TR1402/TR1863/TR1865), AMI (S1883), Signetics
    (2536), National (MM5303), Standard Microsystems (COM2502/COM2017),
    Tesla (MHB1012) and other companies.

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

//#define VERBOSE 1
#include "logmacro.h"


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



DEFINE_DEVICE_TYPE(AY31015, ay31015_device, "ay31015", "AY-3-1015 UART")
DEFINE_DEVICE_TYPE(AY51013, ay51013_device, "ay51013", "AY-5-1013 UART")

ay31015_device::ay31015_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
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
	m_tx_data(0),
	m_tx_buffer(0),
	m_tx_parity(0),
	m_tx_pulses(0),
	m_read_si_cb(*this),
	m_write_so_cb(*this),
	m_write_pe_cb(*this),
	m_write_fe_cb(*this),
	m_write_or_cb(*this),
	m_write_dav_cb(*this),
	m_write_tbmt_cb(*this),
	m_write_eoc_cb(*this),
	m_auto_rdav(false)
{
	for (auto & elem : m_pins)
		elem = 0;
}

ay31015_device::ay31015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ay31015_device(mconfig, AY31015, tag, owner, clock)
{
}

ay51013_device::ay51013_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ay31015_device(mconfig, AY51013, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void ay31015_device::device_resolve_objects()
{
	m_read_si_cb.resolve();
	m_write_so_cb.resolve();

	m_write_tbmt_cb.resolve();
	m_write_dav_cb.resolve();
	m_write_or_cb.resolve();
	m_write_fe_cb.resolve();
	m_write_pe_cb.resolve();
	m_write_eoc_cb.resolve();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ay31015_device::device_start()
{
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

	save_item(NAME(m_tx_state));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_tx_pulses));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ay31015_device::device_reset()
{
	m_rx_data = 0;

	if (!m_pins[CS])
	{
		m_control_reg = 0;
		internal_reset();
	}
}


inline uint8_t ay31015_device::get_si()
{
	if (!m_read_si_cb.isnull())
		m_pins[SI] = m_read_si_cb();

	return m_pins[SI];
}


inline void ay31015_device::set_so( int data )
{
	m_pins[SO] = data ? 1 : 0;

	if (!m_write_so_cb.isnull())
		m_write_so_cb(m_pins[SO]);
}


inline void ay31015_device::update_status_pin(uint8_t reg_bit, ay31015_device::output_pin pin, devcb_write_line &write_cb)
{
	int new_value = (m_status_reg & reg_bit) ? 1 : 0;

	if (new_value != m_pins[pin])
	{
		m_pins[pin] = new_value;
		if (!write_cb.isnull())
			write_cb(new_value);
	}
}


/*-------------------------------------------------
 ay31015_update_status_pins - Update the status pins
-------------------------------------------------*/

void ay31015_device::update_status_pins()
{
	/* Should status pins be updated? */
	if (!m_pins[SWE])
	{
		update_status_pin(STATUS_PE, PE, m_write_pe_cb);
		update_status_pin(STATUS_FE, FE, m_write_fe_cb);
		update_status_pin(STATUS_OR, OR, m_write_or_cb);
		update_status_pin(STATUS_DAV, DAV, m_write_dav_cb);
		update_status_pin(STATUS_TBMT, TBMT, m_write_tbmt_cb);
	}

	update_status_pin(STATUS_EOC, EOC, m_write_eoc_cb);
}

/*************************************************** RECEIVE CONTROLS *************************************************/


/*-------------------------------------------------
 ay31015_rx_process - convert serial to parallel
-------------------------------------------------*/

void ay31015_device::rx_process()
{
	switch (m_rx_state)
	{
		case PREP_TIME:                         // assist sync by ensuring high bit occurs
			m_rx_pulses--;
			if (get_si())
			{
				LOG("Receiver idle\n");
				m_rx_state = IDLE;
			}
			return;

		case IDLE:
			m_rx_pulses--;
			if (!get_si())
			{
				m_rx_state = START_BIT;
				m_rx_pulses = 15;
			}
			return;

		case START_BIT:
			m_rx_pulses--;
			if (m_rx_pulses == 8)            // start bit must be low at sample time
			{
				if (get_si())
				{
					LOG("Receive false start bit\n");
					m_rx_state = IDLE;
				}
				else
					LOG("Receive start bit\n");
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
				LOG("Receive data bit #%d: %d\n", m_rx_bit_count + 1, m_internal_sample);
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
			{
				m_internal_sample = get_si();
				LOG("Receive stop bit: %d\n", m_internal_sample);
			}
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
			if (m_rx_pulses == 4 && m_second_stop_bit)
			{
				/* We should wait for the full first stop bit and
				   the beginning of the second stop bit */
				m_rx_state = SECOND_STOP_BIT;
				m_rx_pulses += m_second_stop_bit - 7;
			}
			else
			if (!m_rx_pulses)
			{
				/* We have seen a STOP bit, go back to IDLE */
				m_rx_state = IDLE;
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

void ay31015_device::tx_process()
{
	uint8_t t1;
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
				LOG("Transmit start bit\n");
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
				LOG("Transmit data bit #%d: %d\n", 9 - (m_tx_pulses >> 4), m_tx_data & 1);

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
				LOG("Transmit parity bit: %d\n", t1);
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
			{
				set_so(1);                /* create a stop bit (marking and soon idle) */
				LOG("Transmit stop bit\n");
			}
			m_tx_pulses--;
			if (!m_tx_pulses)
			{
				m_status_reg |= STATUS_EOC;          // character is completely sent
				if (m_second_stop_bit)
				{
					m_tx_state = SECOND_STOP_BIT;
					m_tx_pulses = m_second_stop_bit;
					LOG("Transmit second stop bit\n");
				}
				else
				if (m_status_reg & STATUS_TBMT)
				{
					m_tx_state = IDLE;           // if nothing to send, go idle
					LOG("Transmitter idle\n");
				}
				else
				{
					m_tx_pulses = 16;
					m_tx_state = START_BIT;      // otherwise immediately start next byte
				}
				update_status_pins();
			}
			return;

		case SECOND_STOP_BIT:
			if (m_tx_pulses == 16)
				LOG("Transmit second stop bit\n");
			m_tx_pulses--;
			if (!m_tx_pulses)
			{
				if (m_status_reg & STATUS_TBMT)
				{
					m_tx_state = IDLE;           // if nothing to send, go idle
					LOG("Transmitter idle\n");
				}
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
	uint8_t t1;

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
	m_pins[SI] = 1;
	m_pins[EOC] = 1;
	m_pins[TBMT] = 1;
	set_so(1);

	m_rx_data = 0;
}


void ay51013_device::internal_reset()
{
	/* total pulses = 16 * data-bits */
	uint8_t t1;

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
	m_pins[SI] = 1;
	m_pins[EOC] = 1;
	m_pins[TBMT] = 1;
	set_so(1);
	// no m_rx_data = 0 in this case
}

/*-------------------------------------------------
 ay31015_transfer_control_pins - transfers contents of controls pins to the control register
-------------------------------------------------*/

void ay31015_device::transfer_control_pins()
{
	uint8_t control = 0;

	control |= m_pins[NP ] ? CONTROL_NP  : 0;
	control |= m_pins[TSB] ? CONTROL_TSB : 0;
	control |= m_pins[NB1] ? CONTROL_NB1 : 0;
	control |= m_pins[NB2] ? CONTROL_NB2 : 0;
	control |= m_pins[EPS] ? CONTROL_EPS : 0;

	if (m_control_reg != control)
	{
		m_control_reg = control;
		internal_reset();
	}
}


/*-------------------------------------------------
 ay31015_set_input_pin - set an input pin
-------------------------------------------------*/
void ay31015_device::set_input_pin( ay31015_device::input_pin pin, int data )
{
	data = data ? 1 : 0;

	switch (pin)
	{
	case RCP:
		if (!m_pins[pin] && data)
			rx_process();
		m_pins[pin] = data;
		break;
	case TCP:
		if (m_pins[pin] && !data)
			tx_process();
		m_pins[pin] = data;
		break;
	case SWE:
		m_pins[pin] = data;
		update_status_pins();
		break;
	case RDAV:
		m_pins[pin] = data;
		if (!data)
		{
			m_status_reg &= ~STATUS_DAV;
			update_status_pins();
		}
		break;
	case SI:
		m_pins[pin] = data;
		break;
	case XR:
		m_pins[pin] = data;
		if (data)
			internal_reset();
		break;
	case CS:
	case NP:
	case TSB:
	case NB1:
	case NB2:
	case EPS:
		m_pins[pin] = data;
		if (m_pins[CS])
			transfer_control_pins();
		break;
	}
}


/*-------------------------------------------------
 ay31015_get_output_pin - get the status of an output pin
-------------------------------------------------*/

int ay31015_device::get_output_pin( ay31015_device::output_pin pin )
{
	return m_pins[pin];
}


/*-------------------------------------------------
 ay31015_get_received_data - return a byte to the computer
-------------------------------------------------*/

uint8_t ay31015_device::get_received_data()
{
	if (m_auto_rdav && !machine().side_effects_disabled())
	{
		m_status_reg &= ~STATUS_DAV;
		update_status_pins();
	}

	return m_rx_buffer;
}

READ8_MEMBER(ay31015_device::receive)
{
	return get_received_data();
}


/*-------------------------------------------------
    ay31015_set_transmit_data - accept a byte to transmit, if able
-------------------------------------------------*/
void ay31015_device::set_transmit_data( uint8_t data )
{
	if (m_status_reg & STATUS_TBMT)
	{
		m_tx_buffer = data;
		m_status_reg &= ~STATUS_TBMT;
		update_status_pins();
	}
}

WRITE8_MEMBER(ay31015_device::transmit)
{
	set_transmit_data(data);
}
