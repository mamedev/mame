/**********************************************************************

    Fairchild F3853 SRAM interface with integrated interrupt
    controller and timer (SMI)

    This chip is a timer shift register, basically the same as in the
    F3851.

    Based on a datasheet obtained from www.freetradezone.com

    The SMI does not have DC0 and DC1, only DC0; as a result, it does
    not respond to the main CPU's DC0/DC1 swap instruction.  This may
    lead to two devices responding to the same DC0 address and
    attempting to place their bytes on the data bus simultaneously!

    8-bit shift register:
    Feedback in0 = !((out3 ^ out4) ^ (out5 ^ out7))
    Interrupts are at 0xfe
    0xff stops the register (0xfe is never reached)

**********************************************************************/

#include "emu.h"
#include "f3853.h"
#include "devhelpr.h"

/***************************************************************************
    MACROS
***************************************************************************/

#define INTERRUPT_VECTOR(external) ( external ? m_low | ( m_high << 8 ) | 0x80 \
: ( m_low | ( m_high << 8 ) ) & ~0x80 )



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type F3853 = &device_creator<f3853_device>;

//-------------------------------------------------
//  f3853_device - constructor
//-------------------------------------------------

f3853_device::f3853_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, F3853, "F3853", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void f3853_device::device_config_complete()
{
	// inherit a copy of the static data
	const f3853_interface *intf = reinterpret_cast<const f3853_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<f3853_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_interrupt_request, 0, sizeof(m_interrupt_request));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void f3853_device::device_start()
{
	UINT8 reg = 0xfe;
	for(INT32 i=254 /* Known to get 0xfe after 255 cycles */; i >= 0; i--)
	{
		INT32 o7 = (reg & 0x80) ? TRUE : FALSE;
		INT32 o5 = (reg & 0x20) ? TRUE : FALSE;
		INT32 o4 = (reg & 0x10) ? TRUE : FALSE;
		INT32 o3 = (reg & 0x08) ? TRUE : FALSE;
		m_value_to_cycle[reg] = i;
		reg <<= 1;
		if(!((o7 != o5) != (o4 != o3)))
		{
			reg |= 1;
		}
	}

	m_timer = machine().scheduler().timer_alloc(FUNC(f3853_timer_callback), (void *)this );

	save_item(NAME(m_high) );
	save_item(NAME(m_low) );
	save_item(NAME(m_external_enable) );
	save_item(NAME(m_timer_enable) );
	save_item(NAME(m_request_flipflop) );
	save_item(NAME(m_priority_line) );
	save_item(NAME(m_external_interrupt_line) );
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void f3853_device::device_reset()
{
	m_high = 0;
	m_low = 0;
	m_external_enable = 0;
	m_timer_enable = 0;
	m_request_flipflop = 0;
	m_priority_line = FALSE;
	m_external_interrupt_line = TRUE;

	m_timer->enable(false);
}


void f3853_device::f3853_set_interrupt_request_line()
{
	if(!m_interrupt_request)
	{
		return;
	}

	if(m_external_enable && !m_priority_line)
	{
		m_interrupt_request(this, INTERRUPT_VECTOR(TRUE), TRUE);
	}
	else if( m_timer_enable && !m_priority_line && m_request_flipflop)
	{
		m_interrupt_request(this, INTERRUPT_VECTOR(FALSE), TRUE);
	}
	else
	{
		m_interrupt_request(this, 0, FALSE);
	}
}


void f3853_device::f3853_timer_start(UINT8 value)
{
	attotime period = (value != 0xff) ? attotime::from_hz(clock()) * (m_value_to_cycle[value]*31) : attotime::never;

	m_timer->adjust(period);
}


TIMER_CALLBACK( f3853_device::f3853_timer_callback )
{
	reinterpret_cast<f3853_device*>(ptr)->f3853_timer();
}

void f3853_device::f3853_timer()
{
	if(m_timer_enable)
	{
		m_request_flipflop = TRUE;
		f3853_set_interrupt_request_line();
	}
	f3853_timer_start(0xfe);
}


void f3853_set_external_interrupt_in_line(device_t *device, int level)
{
	downcast<f3853_device*>(device)->f3853_set_external_interrupt_in_line(level);
}

void f3853_device::f3853_set_external_interrupt_in_line(int level)
{
	if(m_external_interrupt_line && !level && m_external_enable)
	{
		m_request_flipflop = TRUE;
	}
	m_external_interrupt_line = level;
	f3853_set_interrupt_request_line();
}


void f3853_set_priority_in_line(device_t *device, int level)
{
	downcast<f3853_device*>(device)->f3853_set_priority_in_line(level);
}

void f3853_device::f3853_set_priority_in_line(int level)
{
	m_priority_line = level;
	f3853_set_interrupt_request_line();
}


READ8_DEVICE_HANDLER_TRAMPOLINE(f3853, f3853_r)
{
	UINT8 data = 0;

	switch (offset)
	{
	case 0:
		data = m_high;
		break;

	case 1:
		data = m_low;
		break;

	case 2: // Interrupt control; not readable
	case 3: // Timer; not readable
		break;
	}

	return data;
}


WRITE8_DEVICE_HANDLER_TRAMPOLINE(f3853, f3853_w)
{
	switch(offset)
	{
	case 0:
		m_high = data;
		break;

	case 1:
		m_low = data;
		break;

	case 2: //interrupt control
		m_external_enable = ((data & 3) == 1);
		m_timer_enable = ((data & 3) == 3);
		f3853_set_interrupt_request_line();
		break;

	case 3: //timer
		m_request_flipflop = FALSE;
		f3853_set_interrupt_request_line();
		f3853_timer_start(data);
		break;
	}
}
