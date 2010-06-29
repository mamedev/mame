/***************************************************************************

    Z80 CTC (Z8430) implementation

    based on original version (c) 1997, Tatsuyuki Satoh

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "z80ctc.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE		0

#define VPRINTF(x) do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// these are the bits of the incoming commands to the CTC
const int INTERRUPT			= 0x80;
const int INTERRUPT_ON		= 0x80;
const int INTERRUPT_OFF		= 0x00;

const int MODE				= 0x40;
const int MODE_TIMER		= 0x00;
const int MODE_COUNTER		= 0x40;

const int PRESCALER			= 0x20;
const int PRESCALER_256		= 0x20;
const int PRESCALER_16		= 0x00;

const int EDGE				= 0x10;
const int EDGE_FALLING		= 0x00;
const int EDGE_RISING		= 0x10;

const int TRIGGER			= 0x08;
const int TRIGGER_AUTO		= 0x00;
const int TRIGGER_CLOCK		= 0x08;

const int CONSTANT			= 0x04;
const int CONSTANT_LOAD		= 0x04;
const int CONSTANT_NONE		= 0x00;

const int RESET				= 0x02;
const int RESET_CONTINUE	= 0x00;
const int RESET_ACTIVE		= 0x02;

const int CONTROL			= 0x01;
const int CONTROL_VECTOR	= 0x00;
const int CONTROL_WORD		= 0x01;

// these extra bits help us keep things accurate
const int WAITING_FOR_TRIG	= 0x100;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  z80ctc_device_config - constructor
//-------------------------------------------------

z80ctc_device_config::z80ctc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "Zilog Z80 CTC", tag, owner, clock),
	  device_config_z80daisy_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *z80ctc_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(z80ctc_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *z80ctc_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, z80ctc_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void z80ctc_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const z80ctc_interface *intf = reinterpret_cast<const z80ctc_interface *>(static_config());
	if (intf != NULL)
		*static_cast<z80ctc_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_notimer = 0;
		memset(&m_intr, 0, sizeof(m_intr));
		memset(&m_zc0, 0, sizeof(m_zc0));
		memset(&m_zc1, 0, sizeof(m_zc1));
		memset(&m_zc2, 0, sizeof(m_zc2));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80ctc_device - constructor
//-------------------------------------------------

z80ctc_device::z80ctc_device(running_machine &_machine, const z80ctc_device_config &_config)
	: device_t(_machine, _config),
	  device_z80daisy_interface(_machine, _config, *this),
	  m_config(_config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80ctc_device::device_start()
{
	m_period16 = attotime_mul(ATTOTIME_IN_HZ(m_clock), 16);
	m_period256 = attotime_mul(ATTOTIME_IN_HZ(m_clock), 256);

	// resolve callbacks
	devcb_resolve_write_line(&m_intr, &m_config.m_intr, this);

	// start each channel
	m_channel[0].start(this, 0, (m_config.m_notimer & NOTIMER_0) != 0, &m_config.m_zc0);
	m_channel[1].start(this, 1, (m_config.m_notimer & NOTIMER_1) != 0, &m_config.m_zc1);
	m_channel[2].start(this, 2, (m_config.m_notimer & NOTIMER_2) != 0, &m_config.m_zc2);
	m_channel[3].start(this, 3, (m_config.m_notimer & NOTIMER_3) != 0, NULL);

	// register for save states
    state_save_register_device_item(this, 0, m_vector);
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

	logerror("z80ctc_irq_ack: failed to find an interrupt to ack!\n");
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

	logerror("z80ctc_irq_reti: failed to find an interrupt to clear IEO on!\n");
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
	devcb_call_write_line(&m_intr, state);
}



//*************************************************************************
//  CTC CHANNELS
//**************************************************************************

//-------------------------------------------------
//  ctc_channel - constructor
//-------------------------------------------------

z80ctc_device::ctc_channel::ctc_channel()
	: m_notimer(false),
	  m_mode(0),
	  m_tconst(0),
	  m_down(0),
	  m_extclk(0),
	  m_timer(NULL),
	  m_int_state(0)
{
	memset(&m_zc, 0, sizeof(m_zc));
}


//-------------------------------------------------
//  start - set up at device start time
//-------------------------------------------------

void z80ctc_device::ctc_channel::start(z80ctc_device *device, int index, bool notimer, const devcb_write_line *write_line)
{
	// initialize state
	m_device = device;
	m_index = index;
	if (write_line != NULL)
		devcb_resolve_write_line(&m_zc, write_line, m_device);
	m_notimer = notimer;
	m_timer = timer_alloc(&m_device->m_machine, static_timer_callback, this);

	// register for save states
    state_save_register_device_item(m_device, m_index, m_mode);
    state_save_register_device_item(m_device, m_index, m_tconst);
    state_save_register_device_item(m_device, m_index, m_down);
    state_save_register_device_item(m_device, m_index, m_extclk);
    state_save_register_device_item(m_device, m_index, m_int_state);
}


//-------------------------------------------------
//  reset - reset the channel
//-------------------------------------------------

void z80ctc_device::ctc_channel::reset()
{
	m_mode = RESET_ACTIVE;
	m_tconst = 0x100;
	timer_adjust_oneshot(m_timer, attotime_never, 0);
	m_int_state = 0;
}


//-------------------------------------------------
//  period - return the current channel's period
//-------------------------------------------------

attotime z80ctc_device::ctc_channel::period() const
{
	// if reset active, no period
	if ((m_mode & RESET) == RESET_ACTIVE)
		return attotime_zero;

	// if counter mode, no real period
	if ((m_mode & MODE) == MODE_COUNTER)
	{
		logerror("CTC %d is CounterMode : Can't calculate period\n", m_index);
		return attotime_zero;
	}

	// compute the period
	attotime period = ((m_mode & PRESCALER) == PRESCALER_16) ? m_device->m_period16 : m_device->m_period256;
	return attotime_mul(period, m_tconst);
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

		VPRINTF(("CTC clock %f\n",ATTOSECONDS_TO_HZ(period.attoseconds)));

		if (m_timer != NULL)
			return ((int)(attotime_to_double(timer_timeleft(m_timer)) / attotime_to_double(period)) + 1) & 0xff;
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
		VPRINTF(("CTC ch.%d constant = %02x\n", m_index, data));

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
				if (!m_notimer)
				{
					attotime curperiod = period();
					timer_adjust_periodic(m_timer, curperiod, m_index, curperiod);
				}
				else
					timer_adjust_oneshot(m_timer, attotime_never, 0);
			}

			// else set the bit indicating that we're waiting for the appropriate trigger
			else
				m_mode |= WAITING_FOR_TRIG;
		}

		// also set the down counter in case we're clocking externally
		m_down = m_tconst;
	}

	// if we're writing the interrupt vector, handle it specially
#if 0	/* Tatsuyuki Satoh changes */
	// The 'Z80family handbook' wrote,
	// interrupt vector is able to set for even channel (0 or 2)
	else if ((data & CONTROL) == CONTROL_VECTOR && (m_index & 1) == 0)
#else
	else if ((data & CONTROL) == CONTROL_VECTOR && m_index == 0)
#endif
	{
		m_device->m_vector = data & 0xf8;
		logerror("CTC Vector = %02x\n", m_device->m_vector);
	}

	// this must be a control word
	else if ((data & CONTROL) == CONTROL_WORD)
	{
		// set the new mode
		m_mode = data;
		VPRINTF(("CTC ch.%d mode = %02x\n", m_index, data));

		// if we're being reset, clear out any pending timers for this channel
		if ((data & RESET) == RESET_ACTIVE)
		{
			timer_adjust_oneshot(m_timer, attotime_never, 0);
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
				if (!m_notimer)
				{
					attotime curperiod = period();
					VPRINTF(("CTC period %s\n", attotime_string(curperiod, 9)));
					timer_adjust_periodic(m_timer, curperiod, m_index, curperiod);
				}
				else
				{
					VPRINTF(("CTC disabled\n"));
					timer_adjust_oneshot(m_timer, attotime_never, 0);
				}
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
		VPRINTF(("CTC timer ch%d\n", m_index));
		m_device->interrupt_check();
	}

	// generate the clock pulse
	devcb_call_write_line(&m_zc, 1);
	devcb_call_write_line(&m_zc, 0);

	// reset the down counter
	m_down = m_tconst;
}



//**************************************************************************
//  GLOBAL STUBS
//**************************************************************************

WRITE8_DEVICE_HANDLER( z80ctc_w ) { downcast<z80ctc_device *>(device)->write(offset & 3, data); }
READ8_DEVICE_HANDLER( z80ctc_r ) { return downcast<z80ctc_device *>(device)->read(offset & 3); }

WRITE_LINE_DEVICE_HANDLER( z80ctc_trg0_w ) { downcast<z80ctc_device *>(device)->trigger(0, state); }
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg1_w ) { downcast<z80ctc_device *>(device)->trigger(1, state); }
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg2_w ) { downcast<z80ctc_device *>(device)->trigger(2, state); }
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg3_w ) { downcast<z80ctc_device *>(device)->trigger(3, state); }

const device_type Z80CTC = z80ctc_device_config::static_alloc_device_config;
