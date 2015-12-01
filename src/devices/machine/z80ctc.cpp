// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Z80 CTC (Z8430) implementation

    based on original version (c) 1997, Tatsuyuki Satoh

***************************************************************************/

#include "emu.h"
#include "z80ctc.h"
#include "cpu/z80/z80daisy.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE     0

#define VPRINTF(x) do { if (VERBOSE) logerror x; } while (0)
#define VPRINTF_CHANNEL(x) do { if (VERBOSE) m_device->logerror x; } while (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// these are the bits of the incoming commands to the CTC
const int INTERRUPT         = 0x80;
const int INTERRUPT_ON      = 0x80;
//const int INTERRUPT_OFF     = 0x00;

const int MODE              = 0x40;
const int MODE_TIMER        = 0x00;
const int MODE_COUNTER      = 0x40;

const int PRESCALER         = 0x20;
//const int PRESCALER_256     = 0x20;
const int PRESCALER_16      = 0x00;

const int EDGE              = 0x10;
const int EDGE_FALLING      = 0x00;
const int EDGE_RISING       = 0x10;

const int TRIGGER           = 0x08;
const int TRIGGER_AUTO      = 0x00;
//const int TRIGGER_CLOCK     = 0x08;

const int CONSTANT          = 0x04;
const int CONSTANT_LOAD     = 0x04;
//const int CONSTANT_NONE     = 0x00;

const int RESET             = 0x02;
//const int RESET_CONTINUE    = 0x00;
const int RESET_ACTIVE      = 0x02;

const int CONTROL           = 0x01;
const int CONTROL_VECTOR    = 0x00;
const int CONTROL_WORD      = 0x01;

// these extra bits help us keep things accurate
const int WAITING_FOR_TRIG  = 0x100;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type Z80CTC = &device_creator<z80ctc_device>;

//-------------------------------------------------
//  z80ctc_device - constructor
//-------------------------------------------------

z80ctc_device::z80ctc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Z80CTC, "Z80 CTC", tag, owner, clock, "z80ctc", __FILE__),
		device_z80daisy_interface(mconfig, *this),
		m_intr_cb(*this),
		m_zc0_cb(*this),
		m_zc1_cb(*this),
		m_zc2_cb(*this),
		m_zc3_cb(*this),
		m_vector(0)
{
}


//-------------------------------------------------
//  read - standard handler for reading
//-------------------------------------------------

READ8_MEMBER( z80ctc_device::read )
{
	return m_channel[offset & 3].read();
}


//-------------------------------------------------
//  write - standard handler for writing
//-------------------------------------------------

WRITE8_MEMBER( z80ctc_device::write )
{
	m_channel[offset & 3].write(data);
}


//-------------------------------------------------
//  trg0-3 - standard write line handlers for each
//  trigger
//-------------------------------------------------

WRITE_LINE_MEMBER( z80ctc_device::trg0 ) { m_channel[0].trigger(state); }
WRITE_LINE_MEMBER( z80ctc_device::trg1 ) { m_channel[1].trigger(state); }
WRITE_LINE_MEMBER( z80ctc_device::trg2 ) { m_channel[2].trigger(state); }
WRITE_LINE_MEMBER( z80ctc_device::trg3 ) { m_channel[3].trigger(state); }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80ctc_device::device_start()
{
	m_period16 = attotime::from_hz(m_clock) * 16;
	m_period256 = attotime::from_hz(m_clock) * 256;

	// resolve callbacks
	m_intr_cb.resolve_safe();
	m_zc0_cb.resolve_safe();
	m_zc1_cb.resolve_safe();
	m_zc2_cb.resolve_safe();
	m_zc3_cb.resolve_safe();

	// start each channel
	m_channel[0].start(this, 0);
	m_channel[1].start(this, 1);
	m_channel[2].start(this, 2);
	m_channel[3].start(this, 3);

	// register for save states
	save_item(NAME(m_vector));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z80ctc_device::device_reset()
{
	// reset each channel
	m_channel[0].reset();
	m_channel[1].reset();
	m_channel[2].reset();
	m_channel[3].reset();

	// check for interrupts
	interrupt_check();
	VPRINTF(("CTC Reset\n"));
}



//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int z80ctc_device::z80daisy_irq_state()
{
	VPRINTF(("CTC IRQ state = %d%d%d%d\n", m_channel[0].m_int_state, m_channel[1].m_int_state, m_channel[2].m_int_state, m_channel[3].m_int_state));

	// loop over all channels
	int state = 0;
	for (int ch = 0; ch < 4; ch++)
	{
		ctc_channel &channel = m_channel[ch];

		// if we're servicing a request, don't indicate more interrupts
		if (channel.m_int_state & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= channel.m_int_state;
	}

	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int z80ctc_device::z80daisy_irq_ack()
{
	// loop over all channels
	for (int ch = 0; ch < 4; ch++)
	{
		ctc_channel &channel = m_channel[ch];

		// find the first channel with an interrupt requested
		if (channel.m_int_state & Z80_DAISY_INT)
		{
			VPRINTF(("CTC IRQAck ch%d\n", ch));

			// clear interrupt, switch to the IEO state, and update the IRQs
			channel.m_int_state = Z80_DAISY_IEO;
			interrupt_check();
			return m_vector + ch * 2;
		}
	}

	//logerror("z80ctc_irq_ack: failed to find an interrupt to ack!\n");
	return m_vector;
}


//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void z80ctc_device::z80daisy_irq_reti()
{
	// loop over all channels
	for (int ch = 0; ch < 4; ch++)
	{
		ctc_channel &channel = m_channel[ch];

		// find the first channel with an IEO pending
		if (channel.m_int_state & Z80_DAISY_IEO)
		{
			VPRINTF(("CTC IRQReti ch%d\n", ch));

			// clear the IEO state and update the IRQs
			channel.m_int_state &= ~Z80_DAISY_IEO;
			interrupt_check();
			return;
		}
	}

	//logerror("z80ctc_irq_reti: failed to find an interrupt to clear IEO on!\n");
}



//**************************************************************************
//  INTERNAL STATE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  interrupt_check - look for pending interrupts
//  and update the line
//-------------------------------------------------

void z80ctc_device::interrupt_check()
{
	int state = (z80daisy_irq_state() & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE;
	m_intr_cb(state);
}



//*************************************************************************
//  CTC CHANNELS
//**************************************************************************

//-------------------------------------------------
//  ctc_channel - constructor
//-------------------------------------------------

z80ctc_device::ctc_channel::ctc_channel()
	: m_device(NULL),
		m_index(0),
		m_mode(0),
		m_tconst(0),
		m_down(0),
		m_extclk(0),
		m_timer(NULL),
		m_int_state(0)
{
}


//-------------------------------------------------
//  start - set up at device start time
//-------------------------------------------------

void z80ctc_device::ctc_channel::start(z80ctc_device *device, int index)
{
	// initialize state
	m_device = device;
	m_index = index;
	m_timer = m_device->machine().scheduler().timer_alloc(FUNC(static_timer_callback), this);

	// register for save states
	m_device->save_item(NAME(m_mode), m_index);
	m_device->save_item(NAME(m_tconst), m_index);
	m_device->save_item(NAME(m_down), m_index);
	m_device->save_item(NAME(m_extclk), m_index);
	m_device->save_item(NAME(m_int_state), m_index);
}


//-------------------------------------------------
//  reset - reset the channel
//-------------------------------------------------

void z80ctc_device::ctc_channel::reset()
{
	m_mode = RESET_ACTIVE;
	m_tconst = 0x100;
	m_timer->adjust(attotime::never);
	m_int_state = 0;
}


//-------------------------------------------------
//  period - return the current channel's period
//-------------------------------------------------

attotime z80ctc_device::ctc_channel::period() const
{
	// if reset active, no period
	if ((m_mode & RESET) == RESET_ACTIVE)
		return attotime::zero;

	// if counter mode, no real period
	if ((m_mode & MODE) == MODE_COUNTER)
	{
		m_device->logerror("CTC %d is CounterMode : Can't calculate period\n", m_index);
		return attotime::zero;
	}

	// compute the period
	attotime period = ((m_mode & PRESCALER) == PRESCALER_16) ? m_device->m_period16 : m_device->m_period256;
	return period * m_tconst;
}


//-------------------------------------------------
//  read - read the channel's state
//-------------------------------------------------

UINT8 z80ctc_device::ctc_channel::read()
{
	// if we're in counter mode, just return the count
	if ((m_mode & MODE) == MODE_COUNTER || (m_mode & WAITING_FOR_TRIG))
		return m_down;

	// else compute the down counter value
	else
	{
		attotime period = ((m_mode & PRESCALER) == PRESCALER_16) ? m_device->m_period16 : m_device->m_period256;

		VPRINTF_CHANNEL(("CTC clock %f\n",ATTOSECONDS_TO_HZ(period.attoseconds())));

		if (m_timer != NULL)
			return ((int)(m_timer->remaining().as_double() / period.as_double()) + 1) & 0xff;
		else
			return 0;
	}
}


//-------------------------------------------------
//  write - handle writes to a channel
//-------------------------------------------------

void z80ctc_device::ctc_channel::write(UINT8 data)
{
	// if we're waiting for a time constant, this is it
	if ((m_mode & CONSTANT) == CONSTANT_LOAD)
	{
		VPRINTF_CHANNEL(("CTC ch.%d constant = %02x\n", m_index, data));

		// set the time constant (0 -> 0x100)
		m_tconst = data ? data : 0x100;

		// clear the internal mode -- we're no longer waiting
		m_mode &= ~CONSTANT;

		// also clear the reset, since the constant gets it going again
		m_mode &= ~RESET;

		// if we're in timer mode....
		if ((m_mode & MODE) == MODE_TIMER)
		{
			// if we're triggering on the time constant, reset the down counter now
			if ((m_mode & TRIGGER) == TRIGGER_AUTO)
			{
				attotime curperiod = period();
				m_timer->adjust(curperiod, m_index, curperiod);
			}

			// else set the bit indicating that we're waiting for the appropriate trigger
			else
				m_mode |= WAITING_FOR_TRIG;
		}

		// also set the down counter in case we're clocking externally
		m_down = m_tconst;
	}

	// if we're writing the interrupt vector, handle it specially
#if 0   /* Tatsuyuki Satoh changes */
	// The 'Z80family handbook' wrote,
	// interrupt vector is able to set for even channel (0 or 2)
	else if ((data & CONTROL) == CONTROL_VECTOR && (m_index & 1) == 0)
#else
	else if ((data & CONTROL) == CONTROL_VECTOR && m_index == 0)
#endif
	{
		m_device->m_vector = data & 0xf8;
		VPRINTF_CHANNEL(("CTC Vector = %02x\n", m_device->m_vector));
	}

	// this must be a control word
	else if ((data & CONTROL) == CONTROL_WORD)
	{
		// set the new mode
		m_mode = data;
		VPRINTF_CHANNEL(("CTC ch.%d mode = %02x\n", m_index, data));

		// if we're being reset, clear out any pending timers for this channel
		if ((data & RESET) == RESET_ACTIVE)
		{
			m_timer->adjust(attotime::never);
			// note that we don't clear the interrupt state here!
		}
	}
}


//-------------------------------------------------
//  trigger - clock this channel and handle any
//  side-effects
//-------------------------------------------------

void z80ctc_device::ctc_channel::trigger(UINT8 data)
{
	// normalize data
	data = data ? 1 : 0;

	// see if the trigger value has changed
	if (data != m_extclk)
	{
		m_extclk = data;

		// see if this is the active edge of the trigger
		if (((m_mode & EDGE) == EDGE_RISING && data) || ((m_mode & EDGE) == EDGE_FALLING && !data))
		{
			// if we're waiting for a trigger, start the timer
			if ((m_mode & WAITING_FOR_TRIG) && (m_mode & MODE) == MODE_TIMER)
			{
				attotime curperiod = period();
				VPRINTF_CHANNEL(("CTC period %s\n", curperiod.as_string()));
				m_timer->adjust(curperiod, m_index, curperiod);
			}

			// we're no longer waiting
			m_mode &= ~WAITING_FOR_TRIG;

			// if we're clocking externally, decrement the count
			if ((m_mode & MODE) == MODE_COUNTER)
			{
				// if we hit zero, do the same thing as for a timer interrupt
				if (--m_down == 0)
					timer_callback();
			}
		}
	}
}


//-------------------------------------------------
//  trigger - clock this channel and handle any
//  side-effects
//-------------------------------------------------

void z80ctc_device::ctc_channel::timer_callback()
{
	// down counter has reached zero - see if we should interrupt
	if ((m_mode & INTERRUPT) == INTERRUPT_ON)
	{
		m_int_state |= Z80_DAISY_INT;
		VPRINTF_CHANNEL(("CTC timer ch%d\n", m_index));
		m_device->interrupt_check();
	}

	// generate the clock pulse
	switch (m_index)
	{
		case 0:
			m_device->m_zc0_cb(1);
			m_device->m_zc0_cb(0);
			break;
		case 1:
			m_device->m_zc1_cb(1);
			m_device->m_zc1_cb(0);
			break;
		case 2:
			m_device->m_zc2_cb(1);
			m_device->m_zc2_cb(0);
			break;
		case 3:
			m_device->m_zc3_cb(1);
			m_device->m_zc3_cb(0);
			break;
	}

	// reset the down counter
	m_down = m_tconst;
}
