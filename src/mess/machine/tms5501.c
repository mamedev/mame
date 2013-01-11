/*********************************************************************

    tms5501.c

    TMS5501 input/output controller

    Krzysztof Strzecha, Nathan Woods, 2003
    Based on TMS9901 emulator by Raphael Nabet

    21-May-2004 -   Fixed interrupt queue overflow bug (not really fixed
            previously).
    06-Mar-2004 -   Fixed bug in sensor input.
    01-Mar-2004 -   Interrupt queue overrun problem fixed.
    19-Oct-2003 -   Status register added. Reset fixed. Some cleanups.
            INTA enable/disable.

    TODO:
    - SIO

*********************************************************************/

#include "emu.h"
#include "tms5501.h"


/***************************************************************************
    PARAMETERS/MACROS
***************************************************************************/

#define DEBUG_TMS5501   0

#define LOG_TMS5501(message, data) do { if (DEBUG_TMS5501) logerror ("\nTMS5501 %s: %s %02x", tag(), message, data); } while (0)

/* status register */
#define TMS5501_FRAME_ERROR     0x01
#define TMS5501_OVERRUN_ERROR       0x02
#define TMS5501_SERIAL_RCVD     0x04
#define TMS5501_RCV_BUFFER_LOADED   0x08
#define TMS5501_XMIT_BUFFER_EMPTY   0x10
#define TMS5501_INTERRUPT_PENDING   0x20
#define TMS5501_FULL_BIT_DETECT     0x40
#define TMS5501_START_BIT_DETECT    0x80

/* command */
#define TMS5501_RESET           0x01
#define TMS5501_BREAK           0x02
#define TMS5501_INT_7_SELECT        0x04
#define TMS5501_INT_ACK_ENABLE      0x08
#define TMS5501_TEST_BIT_1      0x10
#define TMS5501_TEST_BIT_2      0x20
#define TMS5501_COMMAND_LATCHED_BITS    0x3e

/* interrupt mask register */
#define TMS5501_TIMER_0_INT     0x01
#define TMS5501_TIMER_1_INT     0x02
#define TMS5501_SENSOR_INT      0x04
#define TMS5501_TIMER_2_INT     0x08
#define TMS5501_SERIAL_RCV_LOADED_INT   0x10
#define TMS5501_SERIAL_XMIT_EMPTY_INT   0x20
#define TMS5501_TIMER_3_INT     0x40
#define TMS5501_TIMER_4_INT     0x80
#define TMS5501_INT_7_INT       0x80

#define TMS5501_PIO_INT_7       0x80

// device type definition
const device_type TMS5501 = &device_creator<tms5501_device>;


static const UINT8 timer_name[] = { TMS5501_TIMER_0_INT, TMS5501_TIMER_1_INT, TMS5501_TIMER_2_INT, TMS5501_TIMER_3_INT, TMS5501_TIMER_4_INT };

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  tms5501_device - constructor
//-------------------------------------------------

tms5501_device::tms5501_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS5501, "TMS5501", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tms5501_device::device_config_complete()
{
	// inherit a copy of the static data
	const tms5501_interface *intf = reinterpret_cast<const tms5501_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<tms5501_interface *>(this) = *intf;
	}
	// or initialize to defaults if none provided
	else
	{
		m_interrupt_cb = NULL;
		memset(&m_pio_read_cb, 0, sizeof(m_pio_read_cb));
		memset(&m_pio_write_cb, 0, sizeof(m_pio_write_cb));
	}
}

/*-------------------------------------------------
    find_first_bit
-------------------------------------------------*/

int tms5501_device::find_first_bit(int value)
{
	int bit = 0;

	if (! value)
		return -1;

	while (! (value & 1))
	{
		value >>= 1;    /* try next bit */
		bit++;
	}
	return bit;
}


/*-------------------------------------------------
    tms5501_device::field_interrupts()
-------------------------------------------------*/

void tms5501_device::field_interrupts()
{
	static const UINT8 int_vectors[] = { 0xc7, 0xcf, 0xd7, 0xdf, 0xe7, 0xef, 0xf7, 0xff };
	UINT8 current_ints = m_pending_interrupts;

	/* disabling masked interrupts */
	current_ints &= m_interrupt_mask;

	LOG_TMS5501("Pending interrupts", m_pending_interrupts);
	LOG_TMS5501("Interrupt mask", m_interrupt_mask);
	LOG_TMS5501("Current interrupts", current_ints);

	if (current_ints)
	{
		/* selecting interrupt with highest priority */
		int level = find_first_bit(current_ints);
		LOG_TMS5501("Interrupt level", level);

		/* resetting proper bit in pending interrupts register */
		m_pending_interrupts &= ~(1<<level);

		/* selecting  interrupt vector */
		m_interrupt_address = int_vectors[level];
		LOG_TMS5501("Interrupt vector", int_vectors[level]);

		if ((m_command & TMS5501_INT_ACK_ENABLE))
		{
			if (m_interrupt_cb)
				(m_interrupt_cb)(*this, 1, int_vectors[level]);
		}
		else
			m_status |= TMS5501_INTERRUPT_PENDING;
	}
	else
	{
		if ((m_command & TMS5501_INT_ACK_ENABLE))
		{
			if (m_interrupt_cb)
				(m_interrupt_cb)(*this, 0, 0);
		}
		else
			m_status &= ~TMS5501_INTERRUPT_PENDING;
	}
}


/*-------------------------------------------------
    tms5501_device::timer_decrementer
-------------------------------------------------*/

void tms5501_device::timer_decrementer(UINT8 mask)
{
	if ((mask != TMS5501_TIMER_4_INT) || ((mask == TMS5501_TIMER_4_INT) && (!(m_command & TMS5501_INT_7_SELECT))))
		m_pending_interrupts |= mask;

	field_interrupts();
}


/*-------------------------------------------------
    tms5501_device::timer_reload
-------------------------------------------------*/

void tms5501_device::timer_reload(int timer)
{
	if (m_timer_counter[timer])
	{   /* reset clock interval */
		m_timer[timer]->adjust(attotime::from_double((double) m_timer_counter[timer] / (clock() / 128.)), timer_name[timer], attotime::from_double((double) m_timer_counter[timer] / (clock() / 128.)));
	}
	else
	{   /* clock interval == 0 -> no timer */
		switch (timer)
		{
			case 0: timer_decrementer(TMS5501_TIMER_0_INT); break;
			case 1: timer_decrementer(TMS5501_TIMER_1_INT); break;
			case 2: timer_decrementer(TMS5501_TIMER_2_INT); break;
			case 3: timer_decrementer(TMS5501_TIMER_3_INT); break;
			case 4: timer_decrementer(TMS5501_TIMER_4_INT); break;
		}
		m_timer[timer]->enable(0);
	}
}


/*-------------------------------------------------
    DEVICE_RESET( tms5501 )
-------------------------------------------------*/

void tms5501_device::device_reset()
{
	m_status &= ~(TMS5501_RCV_BUFFER_LOADED|TMS5501_FULL_BIT_DETECT|TMS5501_START_BIT_DETECT|TMS5501_OVERRUN_ERROR);
	m_status |= TMS5501_XMIT_BUFFER_EMPTY|TMS5501_SERIAL_RCVD;

	m_pending_interrupts = TMS5501_SERIAL_XMIT_EMPTY_INT;

	for (int i=0; i<5; i++)
	{
		m_timer_counter[i] = 0;
		m_timer[i]->enable(0);
	}

	LOG_TMS5501("Reset", 0);
}


/*-------------------------------------------------
    DEVICE_START( tms5501 )
-------------------------------------------------*/

void tms5501_device::device_start()
{
	// resolve callbacks
	m_pio_read_func.resolve(m_pio_read_cb, *this);
	m_pio_write_func.resolve(m_pio_write_cb, *this);

	// allocate timers
	for (int i = 0; i < 5; i++)
	{
		m_timer[i] = timer_alloc(TIMER_DECREMENTER);
		m_timer[i]->set_param(i);
	}

	m_interrupt_mask = 0;
	m_interrupt_address = 0;

	m_sensor = 0;
	m_sio_rate = 0;
	m_sio_input_buffer = 0;
	m_sio_output_buffer = 0;
	m_pio_input_buffer = 0;
	m_pio_output_buffer = 0;

	m_command = 0;
	LOG_TMS5501("Init", 0);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void tms5501_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_DECREMENTER:
		timer_decrementer((UINT8)param);
		break;
	}
}


/*-------------------------------------------------
    tms5501_device::set_pio_bit_7
-------------------------------------------------*/

DECLARE_WRITE_LINE_MEMBER( tms5501_device::set_pio_bit_7 )
{
	if (m_command & TMS5501_INT_7_SELECT)
	{
		if (!(m_pio_input_buffer & TMS5501_PIO_INT_7) && state)
			m_pending_interrupts |= TMS5501_INT_7_INT;
		else
			m_pending_interrupts &= ~TMS5501_INT_7_INT;
	}

	m_pio_input_buffer &= ~TMS5501_PIO_INT_7;
	if (state)
		m_pio_input_buffer |= TMS5501_PIO_INT_7;

	if (m_pending_interrupts & TMS5501_INT_7_INT)
		field_interrupts();
}


/*-------------------------------------------------
    tms5501_device::set_sensor
-------------------------------------------------*/

WRITE_LINE_MEMBER(tms5501_device::set_sensor)
{
	if (!(m_sensor) && state)
		m_pending_interrupts |= TMS5501_SENSOR_INT;
	else
		m_pending_interrupts &= ~TMS5501_SENSOR_INT;

	m_sensor = state;

	if (m_pending_interrupts &= TMS5501_SENSOR_INT)
		field_interrupts();
}


/*-------------------------------------------------
    READ8_DEVICE_HANDLER( tms5501_r )
-------------------------------------------------*/

READ8_MEMBER( tms5501_device::read )
{
	UINT8 data = 0x00;
	offset &= 0x0f;

	switch (offset)
	{
		case 0x00:  /* Serial input buffer */
			data = m_sio_input_buffer;
			m_status &= ~TMS5501_RCV_BUFFER_LOADED;
			LOG_TMS5501("Reading from serial input buffer", data);
			break;
		case 0x01:  /* PIO input port */
			if (!m_pio_read_func.isnull())
				data = m_pio_read_func(0);
			LOG_TMS5501("Reading from PIO", data);
			break;
		case 0x02:  /* Interrupt address register */
			data = m_interrupt_address;
			m_status &= ~TMS5501_INTERRUPT_PENDING;
			break;
		case 0x03:  /* Status register */
			data = m_status;
			break;
		case 0x04:  /* Command register */
			data = m_command;
			LOG_TMS5501("Command register read", data);
			break;
		case 0x05:  /* Serial rate register */
			data = m_sio_rate;
			LOG_TMS5501("Serial rate read", data);
			break;
		case 0x06:  /* Serial output buffer */
		case 0x07:  /* PIO output */
			break;
		case 0x08:  /* Interrupt mask register */
			data = m_interrupt_mask;
			LOG_TMS5501("Interrupt mask read", data);
			break;
		case 0x09:  /* Timer 0 address */
		case 0x0a:  /* Timer 1 address */
		case 0x0b:  /* Timer 2 address */
		case 0x0c:  /* Timer 3 address */
		case 0x0d:  /* Timer 4 address */
			break;
	}
	return data;
}


/*-------------------------------------------------
    WRITE8_DEVICE_HANDLER( tms5501_w )
-------------------------------------------------*/

WRITE8_MEMBER( tms5501_device::write )
{
	offset &= 0x0f;

	switch (offset)
	{
		case 0x00:  /* Serial input buffer */
		case 0x01:  /* Keyboard input port, Page blanking signal */
		case 0x02:  /* Interrupt address register */
		case 0x03:  /* Status register */
			LOG_TMS5501("Writing to read only port", offset&0x000f);
			LOG_TMS5501("Data", data);
			break;
		case 0x04:
			/* Command register
			    bit 0: reset
			    bit 1: send break, '1' - serial output is high impedance
			    bit 2: int 7 select: '0' - timer 5, '1' - IN7 of the DCE-bus
			    bit 3: int ack enable, '0' - disabled, '1' - enabled
			    bits 4-5: test bits, normally '0'
			    bits 6-7: not used, normally '0'
			   bits 1-5 are latched */

			m_command = data & TMS5501_COMMAND_LATCHED_BITS;
			LOG_TMS5501("Command register write", data);

			if (data & TMS5501_RESET)
				reset();
			break;
		case 0x05:
			/* Serial rate register
			    bit 0: 110 baud
			    bit 1: 150 baud
			    bit 2: 300 baud
			    bit 3: 1200 baud
			    bit 4: 2400 baud
			    bit 5: 4800 baud
			    bit 6: 9600 baud
			    bit 7: '0' - two stop bits, '1' - one stop bit */

			m_sio_rate = data;
			LOG_TMS5501("Serial rate write", data);
			break;
		case 0x06:  /* Serial output buffer */
			m_sio_output_buffer = data;
			LOG_TMS5501("Serial output data", data);
			break;
		case 0x07:  /* PIO output */
			m_pio_output_buffer = data;
			if (!m_pio_write_func.isnull())
				m_pio_write_func(0, m_pio_output_buffer);
			LOG_TMS5501("Writing to PIO", data);
			break;
		case 0x08:
			/* Interrupt mask register
			    bit 0: Timer 1 has expired (UTIM)
			    bit 1: Timer 2 has expired
			    bit 2: External interrupt (STKIM)
			    bit 3: Timer 3 has expired (SNDIM)
			    bit 4: Serial receiver loaded
			    bit 5: Serial transmitter empty
			    bit 6: Timer 4 has expired (KBIM)
			    bit 7: Timer 5 has expired or IN7 (CLKIM) */

			m_interrupt_mask = data;
			LOG_TMS5501("Interrupt mask write", data);
			break;
		case 0x09:  /* Timer 0 counter */
		case 0x0a:  /* Timer 1 counter */
		case 0x0b:  /* Timer 2 counter */
		case 0x0c:  /* Timer 3 counter */
		case 0x0d:  /* Timer 4 counter */
			offset -= 9;
			m_timer_counter[offset] = data;
			timer_reload(offset);
			LOG_TMS5501("Write timer", offset);
			LOG_TMS5501("Timer counter set", data);
			break;
	}
}
