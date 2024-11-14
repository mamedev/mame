// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Z80 CTC (Z8430) implementation

    based on original version (c) 1997, Tatsuyuki Satoh

***************************************************************************/

#include "emu.h"
#include "z80ctc.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE     0
#include "logmacro.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// these are the bits of the incoming commands to the CTC
constexpr u16 INTERRUPT         = 0x80;
constexpr u16 INTERRUPT_ON      = 0x80;
constexpr u16 INTERRUPT_OFF     = 0x00;

constexpr u16 MODE              = 0x40;
constexpr u16 MODE_TIMER        = 0x00;
constexpr u16 MODE_COUNTER      = 0x40;

constexpr u16 PRESCALER         = 0x20;
//constexpr u16 PRESCALER_256     = 0x20;
constexpr u16 PRESCALER_16      = 0x00;

constexpr u16 EDGE              = 0x10;
constexpr u16 EDGE_FALLING      = 0x00;
constexpr u16 EDGE_RISING       = 0x10;

constexpr u16 TRIGGER           = 0x08;
constexpr u16 TRIGGER_AUTO      = 0x00;
//constexpr u16 TRIGGER_CLOCK     = 0x08;

constexpr u16 CONSTANT          = 0x04;
constexpr u16 CONSTANT_LOAD     = 0x04;
//constexpr u16 CONSTANT_NONE     = 0x00;

constexpr u16 RESET             = 0x02;
//constexpr u16 RESET_CONTINUE    = 0x00;
constexpr u16 RESET_ACTIVE      = 0x02;

constexpr u16 CONTROL           = 0x01;
constexpr u16 CONTROL_VECTOR    = 0x00;
constexpr u16 CONTROL_WORD      = 0x01;

// these extra bits help us keep things accurate
constexpr u16 WAITING_FOR_TRIG  = 0x100;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(Z80CTC, z80ctc_device, "z80ctc", "Z80 CTC")
DEFINE_DEVICE_TYPE(Z80CTC_CHANNEL, z80ctc_channel_device, "z80ctc_channel", "Z80 CTC Channel")

//-------------------------------------------------
//  z80ctc_device - constructor
//-------------------------------------------------

z80ctc_device::z80ctc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80ctc_device(mconfig, Z80CTC, tag, owner, clock)
{
}

z80ctc_device::z80ctc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, m_channel(*this, "ch%u", 0U)
	, m_intr_cb(*this)
	, m_zc_cb(*this)
	, m_vector(0)
{
}


//-------------------------------------------------
//  read - standard handler for reading
//-------------------------------------------------

uint8_t z80ctc_device::read(offs_t offset)
{
	return m_channel[offset & 3]->read();
}


//-------------------------------------------------
//  write - standard handler for writing
//-------------------------------------------------

void z80ctc_device::write(offs_t offset, uint8_t data)
{
	m_channel[offset & 3]->write(data);
}


//-------------------------------------------------
//  trg0-3 - standard write line handlers for each
//  trigger
//-------------------------------------------------

void z80ctc_device::trg0(int state) { m_channel[0]->trigger(state != 0); }
void z80ctc_device::trg1(int state) { m_channel[1]->trigger(state != 0); }
void z80ctc_device::trg2(int state) { m_channel[2]->trigger(state != 0); }
void z80ctc_device::trg3(int state) { m_channel[3]->trigger(state != 0); }


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void z80ctc_device::device_add_mconfig(machine_config &config)
{
	for (int ch = 0; ch < 4; ch++)
	{
		Z80CTC_CHANNEL(config, m_channel[ch]);

		// assign channel index
		m_channel[ch]->m_index = ch;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80ctc_device::device_start()
{
	// register for save states
	save_item(NAME(m_vector));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z80ctc_device::device_reset_after_children()
{
	// check for interrupts
	interrupt_check();
	LOG("CTC Reset\n");
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
	LOG("CTC IRQ state = %d%d%d%d\n", m_channel[0]->m_int_state, m_channel[1]->m_int_state, m_channel[2]->m_int_state, m_channel[3]->m_int_state);

	// loop over all channels
	int state = 0;
	for (int ch = 0; ch < 4; ch++)
	{
		// if we're servicing a request, don't indicate more interrupts
		if (m_channel[ch]->m_int_state & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= m_channel[ch]->m_int_state;
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
		z80ctc_channel_device &channel = *m_channel[ch];

		// find the first channel with an interrupt requested
		if (channel.m_int_state & Z80_DAISY_INT)
		{
			LOG("CTC IRQAck ch%d\n", ch);

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
		z80ctc_channel_device &channel = *m_channel[ch];

		// find the first channel with an IEO pending
		if (channel.m_int_state & Z80_DAISY_IEO)
		{
			LOG("CTC IRQReti ch%d\n", ch);

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
//  z80ctc_channel_device - constructor
//-------------------------------------------------

z80ctc_channel_device::z80ctc_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, Z80CTC_CHANNEL, tag, owner, clock),
		m_device(*this, DEVICE_SELF_OWNER),
		m_index(0),
		m_mode(0),
		m_tconst(0),
		m_down(0),
		m_extclk(0),
		m_timer(nullptr),
		m_int_state(0),
		m_zc_to_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - set up at device start time
//-------------------------------------------------

void z80ctc_channel_device::device_start()
{
	// initialize state
	m_timer = timer_alloc(FUNC(z80ctc_channel_device::timer_callback), this);
	m_zc_to_timer = timer_alloc(FUNC(z80ctc_channel_device::zc_to_callback), this);

	// register for save states
	save_item(NAME(m_mode));
	save_item(NAME(m_tconst));
	save_item(NAME(m_down));
	save_item(NAME(m_extclk));
	save_item(NAME(m_int_state));
}


//-------------------------------------------------
//  device_reset - reset the channel
//-------------------------------------------------

void z80ctc_channel_device::device_reset()
{
	m_mode = RESET_ACTIVE;
	m_tconst = 0x100;
	m_timer->adjust(attotime::never);
	m_int_state = 0;
}


//-------------------------------------------------
//  period - return the current channel's period
//-------------------------------------------------

attotime z80ctc_channel_device::period() const
{
	// if reset active, no period
	if ((m_mode & RESET) == RESET_ACTIVE)
		return attotime::never;

	// if counter mode, no real period unless the channel clock is specifically configured
	if ((m_mode & MODE) == MODE_COUNTER)
		return clocks_to_attotime(m_tconst);

	// compute the period
	attotime period = m_device->clocks_to_attotime((m_mode & PRESCALER) == PRESCALER_16 ? 16 : 256);
	return period * m_tconst;
}


//-------------------------------------------------
//  read - read the channel's state
//-------------------------------------------------

u8 z80ctc_channel_device::read()
{
	// if we're in counter mode, just return the count
	if (!m_timer->enabled() || (m_mode & WAITING_FOR_TRIG))
		return m_down;

	// else compute the down counter value
	else
	{
		attotime period;
		if ((m_mode & MODE) == MODE_COUNTER)
			period = clocks_to_attotime(1);
		else
			period = m_device->clocks_to_attotime((m_mode & PRESCALER) == PRESCALER_16 ? 16 : 256);

		LOG("CTC clock %f\n", period.as_hz());

		if(!m_timer->remaining().is_never())
			return u8((m_timer->remaining().as_double() / period.as_double()) + 1.0);
		else
		{
			// value read-back is required by x1turbo for YM internal board detection.
			// cfr. x1turbo40 argus wpiset 0x704,1,rw
			return m_down;
		}
	}
}


//-------------------------------------------------
//  write - handle writes to a channel
//-------------------------------------------------

void z80ctc_channel_device::write(u8 data)
{
	// if we're waiting for a time constant, this is it
	if ((m_mode & CONSTANT) == CONSTANT_LOAD)
	{
		LOG("Time constant = %02x\n", data);

		// set the time constant (0 -> 0x100)
		m_tconst = data ? data : 0x100;

		// clear the internal mode -- we're no longer waiting
		m_mode &= ~CONSTANT;

		// also clear the reset, since the constant gets it going again
		m_mode &= ~RESET;

		// if we're triggering on the time constant, reset the down counter now
		if ((m_mode & MODE) == MODE_COUNTER || (m_mode & TRIGGER) == TRIGGER_AUTO)
		{
			attotime curperiod = period();
			m_timer->adjust(curperiod, 0, curperiod);
		}

		// else set the bit indicating that we're waiting for the appropriate trigger
		else
		{
			m_mode |= WAITING_FOR_TRIG;
			m_timer->adjust(clocks_to_attotime(1));
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
		LOG("Vector = %02x\n", m_device->m_vector);
	}

	// this must be a control word
	else if ((data & CONTROL) == CONTROL_WORD)
	{
		// (mode change without reset?)
		if ((m_mode & MODE) == MODE_TIMER && (data & MODE) == MODE_COUNTER && (data & RESET) == 0)
		{
			m_timer->adjust(attotime::never);
		}

		// if we're being reset, clear out any pending timers for this channel
		if ((data & RESET) == RESET_ACTIVE)
		{
			// remember the present count
			m_down = read();
			m_timer->adjust(attotime::never);
			// note that we don't clear the interrupt state here!
		}

		// set the new mode
		m_mode = data;
		LOG("Channel mode = %02x\n", data);

		// clearing this bit resets the interrupt state regardless of M1 activity (or lack thereof)
		if ((data & INTERRUPT) == INTERRUPT_OFF && (m_int_state & Z80_DAISY_INT))
		{
			m_int_state &= ~Z80_DAISY_INT;
			LOG("Interrupt forced off\n");
			m_device->interrupt_check();
		}
	}
}


//-------------------------------------------------
//  trigger - clock this channel and handle any
//  side-effects
//-------------------------------------------------

void z80ctc_channel_device::trigger(bool state)
{
	// see if the trigger value has changed
	if (state != m_extclk)
	{
		m_extclk = state;

		// see if this is the active edge of the trigger
		if (((m_mode & EDGE) == EDGE_RISING && state) || ((m_mode & EDGE) == EDGE_FALLING && !state))
		{
			// if we're waiting for a trigger, start the timer
			if ((m_mode & WAITING_FOR_TRIG) && (m_mode & MODE) == MODE_TIMER)
			{
				attotime curperiod = period();
				LOG("Period = %s\n", curperiod.as_string());
				m_timer->adjust(curperiod, 0, curperiod);
			}

			// we're no longer waiting
			m_mode &= ~WAITING_FOR_TRIG;

			// if we're clocking externally, decrement the count
			if ((m_mode & MODE) == MODE_COUNTER)
			{
				// if we hit zero, do the same thing as for a timer interrupt
				if (--m_down == 0)
					timer_callback(0);
			}
		}
	}
}


//-------------------------------------------------
//  trigger - clock this channel and handle any
//  side-effects
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(z80ctc_channel_device::timer_callback)
{
	if (m_mode & WAITING_FOR_TRIG)
	{
		attotime curperiod = period();
		LOG("Period = %s\n", curperiod.as_string());
		m_timer->adjust(curperiod, 0, curperiod);

		// we're no longer waiting
		m_mode &= ~WAITING_FOR_TRIG;

		return;
	}

	// down counter has reached zero - see if we should interrupt
	if ((m_mode & INTERRUPT) == INTERRUPT_ON)
	{
		m_int_state |= Z80_DAISY_INT;
		LOG("Timer interrupt\n");
		m_device->interrupt_check();
	}

	// generate the clock pulse
	m_device->m_zc_cb[m_index](1);
	m_zc_to_timer->adjust(m_device->clocks_to_attotime(1));

	// reset the down counter
	m_down = m_tconst;
}

TIMER_CALLBACK_MEMBER(z80ctc_channel_device::zc_to_callback)
{
	m_device->m_zc_cb[m_index](0);
}
