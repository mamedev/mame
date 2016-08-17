// license:BSD-3-Clause
// copyright-holders:Peter Trauner, Mathis Rosenhauer
/**********************************************************************

    Rockwell 6522 VIA interface and emulation

    This function emulates the functionality of up to 8 6522
    versatile interface adapters.

    This is based on the M6821 emulation in MAME.

    To do:

    T2 pulse counting mode
    Pulse mode handshake output
    More shift register

**********************************************************************/

/*
  1999-Dec-22 PeT
   vc20 random number generation only partly working
   (reads (uninitialized) timer 1 and timer 2 counter)
   timer init, reset, read changed
 */

#include "emu.h"
#include "6522via.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define TRACE_VIA       0


/***************************************************************************
    MACROS
***************************************************************************/

/* Macros for PCR */
#define CA1_LOW_TO_HIGH(c)      (c & 0x01)
#define CA1_HIGH_TO_LOW(c)      (!(c & 0x01))

#define CB1_LOW_TO_HIGH(c)      (c & 0x10)
#define CB1_HIGH_TO_LOW(c)      (!(c & 0x10))

#define CA2_INPUT(c)            (!(c & 0x08))
#define CA2_LOW_TO_HIGH(c)      ((c & 0x0c) == 0x04)
#define CA2_HIGH_TO_LOW(c)      ((c & 0x0c) == 0x00)
#define CA2_IND_IRQ(c)          ((c & 0x0a) == 0x02)

#define CA2_OUTPUT(c)           (c & 0x08)
#define CA2_AUTO_HS(c)          ((c & 0x0c) == 0x08)
#define CA2_HS_OUTPUT(c)        ((c & 0x0e) == 0x08)
#define CA2_PULSE_OUTPUT(c)     ((c & 0x0e) == 0x0a)
#define CA2_FIX_OUTPUT(c)       ((c & 0x0c) == 0x0c)
#define CA2_OUTPUT_LEVEL(c)     ((c & 0x02) >> 1)

#define CB2_INPUT(c)            (!(c & 0x80))
#define CB2_LOW_TO_HIGH(c)      ((c & 0xc0) == 0x40)
#define CB2_HIGH_TO_LOW(c)      ((c & 0xc0) == 0x00)
#define CB2_IND_IRQ(c)          ((c & 0xa0) == 0x20)

#define CB2_OUTPUT(c)           (c & 0x80)
#define CB2_AUTO_HS(c)          ((c & 0xc0) == 0x80)
#define CB2_HS_OUTPUT(c)        ((c & 0xe0) == 0x80)
#define CB2_PULSE_OUTPUT(c)     ((c & 0xe0) == 0xa0)
#define CB2_FIX_OUTPUT(c)       ((c & 0xc0) == 0xc0)
#define CB2_OUTPUT_LEVEL(c)     ((c & 0x20) >> 5)

/* Macros for ACR */
#define PA_LATCH_ENABLE(c)      (c & 0x01)
#define PB_LATCH_ENABLE(c)      (c & 0x02)

#define SR_DISABLED(c)          (!(c & 0x1c))
#define SI_T2_CONTROL(c)        ((c & 0x1c) == 0x04)
#define SI_O2_CONTROL(c)        ((c & 0x1c) == 0x08)
#define SI_EXT_CONTROL(c)       ((c & 0x1c) == 0x0c)
#define SO_T2_RATE(c)           ((c & 0x1c) == 0x10)
#define SO_T2_CONTROL(c)        ((c & 0x1c) == 0x14)
#define SO_O2_CONTROL(c)        ((c & 0x1c) == 0x18)
#define SO_EXT_CONTROL(c)       ((c & 0x1c) == 0x1c)

#define T1_SET_PB7(c)           (c & 0x80)
#define T1_CONTINUOUS(c)        (c & 0x40)
#define T2_COUNT_PB6(c)         (c & 0x20)

/* Interrupt flags */
#define INT_CA2 0x01
#define INT_CA1 0x02
#define INT_SR  0x04
#define INT_CB2 0x08
#define INT_CB1 0x10
#define INT_T2  0x20
#define INT_T1  0x40
#define INT_ANY 0x80

#define CLR_PA_INT()    clear_int(INT_CA1 | ((!CA2_IND_IRQ(m_pcr)) ? INT_CA2: 0))
#define CLR_PB_INT()    clear_int(INT_CB1 | ((!CB2_IND_IRQ(m_pcr)) ? INT_CB2: 0))

#define IFR_DELAY 3

#define TIMER1_VALUE    (m_t1ll+(m_t1lh<<8))
#define TIMER2_VALUE    (m_t2ll+(m_t2lh<<8))



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

UINT16 via6522_device::get_counter1_value()
{
	UINT16 val;

	if(m_t1_active)
	{
		val = attotime_to_clocks(m_t1->remaining()) - IFR_DELAY;
	}
	else
	{
		val = 0xffff - attotime_to_clocks(machine().time() - m_time1);
	}

	return val;
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type VIA6522 = &device_creator<via6522_device>;

DEVICE_ADDRESS_MAP_START( map, 8, via6522_device )
	AM_RANGE(0x00, 0x0f) AM_READWRITE(read, write)
ADDRESS_MAP_END

//-------------------------------------------------
//  via6522_device - constructor
//-------------------------------------------------

via6522_device::via6522_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VIA6522, "6522 VIA", tag, owner, clock, "via6522", __FILE__),
		m_in_a_handler(*this),
		m_in_b_handler(*this),
		m_out_a_handler(*this),
		m_out_b_handler(*this),
		m_ca2_handler(*this),
		m_cb1_handler(*this),
		m_cb2_handler(*this),
		m_irq_handler(*this),
		m_in_a(0xff),
		m_in_ca1(0),
		m_in_ca2(0),
		m_out_ca2(0),
		m_in_b(0),
		m_in_cb1(0),
		m_in_cb2(0),
		m_pcr(0),
		m_acr(0),
		m_ier(0),
		m_ifr(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void via6522_device::device_start()
{
	m_in_a_handler.resolve();
	m_in_b_handler.resolve();
	m_out_a_handler.resolve_safe();
	m_out_b_handler.resolve_safe();
	m_cb1_handler.resolve_safe();
	m_ca2_handler.resolve_safe();
	m_cb2_handler.resolve_safe();
	m_irq_handler.resolve_safe();

	m_t1ll = 0xf3; /* via at 0x9110 in vic20 show these values */
	m_t1lh = 0xb5; /* ports are not written by kernel! */
	m_t2ll = 0xff; /* taken from vice */
	m_t2lh = 0xff;
	m_sr = 0;

	m_time2 = m_time1 = machine().time();
	m_t1 = timer_alloc(TIMER_T1);
	m_t2 = timer_alloc(TIMER_T2);
	m_ca2_timer = timer_alloc(TIMER_CA2);
	m_shift_timer = timer_alloc(TIMER_SHIFT);

	/* Default clock is from CPU1 */
	if (clock() == 0)
	{
		set_unscaled_clock(machine().firstcpu->clock());
	}

	/* save state register */
	save_item(NAME(m_in_a));
	save_item(NAME(m_in_ca1));
	save_item(NAME(m_in_ca2));
	save_item(NAME(m_out_a));
	save_item(NAME(m_out_ca2));
	save_item(NAME(m_ddr_a));
	save_item(NAME(m_latch_a));
	save_item(NAME(m_in_b));
	save_item(NAME(m_in_cb1));
	save_item(NAME(m_in_cb2));
	save_item(NAME(m_out_b));
	save_item(NAME(m_out_cb1));
	save_item(NAME(m_out_cb2));
	save_item(NAME(m_ddr_b));
	save_item(NAME(m_latch_b));
	save_item(NAME(m_t1cl));
	save_item(NAME(m_t1ch));
	save_item(NAME(m_t1ll));
	save_item(NAME(m_t1lh));
	save_item(NAME(m_t2cl));
	save_item(NAME(m_t2ch));
	save_item(NAME(m_t2ll));
	save_item(NAME(m_t2lh));
	save_item(NAME(m_sr));
	save_item(NAME(m_pcr));
	save_item(NAME(m_acr));
	save_item(NAME(m_ier));
	save_item(NAME(m_ifr));
	save_item(NAME(m_time1));
	save_item(NAME(m_t1_active));
	save_item(NAME(m_t1_pb7));
	save_item(NAME(m_time2));
	save_item(NAME(m_t2_active));
	save_item(NAME(m_shift_counter));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void via6522_device::device_reset()
{
	m_out_a = 0;
	m_out_ca2 = 1;
	m_ddr_a = 0;
	m_latch_a = 0;

	m_out_b = 0;
	m_out_cb1 = 1;
	m_out_cb2 = 1;
	m_ddr_b = 0;
	m_latch_b = 0;

	m_t1cl = 0;
	m_t1ch = 0;
	m_t2cl = 0;
	m_t2ch = 0;

	m_pcr = 0;
	m_acr = 0;
	m_ier = 0;
	m_ifr = 0;
	m_t1_active = 0;
	m_t1_pb7 = 1;
	m_t2_active = 0;
	m_shift_counter = 0;

	output_pa();
	output_pb();
	m_ca2_handler(m_out_ca2);
	m_cb1_handler(m_out_cb1);
	m_cb2_handler(m_out_cb2);
}


void via6522_device::output_irq()
{
	if (m_ier & m_ifr & 0x7f)
	{
		if ((m_ifr & INT_ANY) == 0)
		{
			m_ifr |= INT_ANY;
			m_irq_handler(ASSERT_LINE);
		}
	}
	else
	{
		if (m_ifr & INT_ANY)
		{
			m_ifr &= ~INT_ANY;
			m_irq_handler(CLEAR_LINE);
		}
	}
}


/*-------------------------------------------------
    via_set_int - external interrupt check
-------------------------------------------------*/

void via6522_device::set_int(int data)
{
	if (!(m_ifr & data))
	{
		m_ifr |= data;

		output_irq();

		if (TRACE_VIA)
		{
			logerror("%s:6522VIA chip %s: IFR = %02X\n", machine().describe_context(), tag(), m_ifr);
		}
	}
}


/*-------------------------------------------------
    via_clear_int - external interrupt check
-------------------------------------------------*/

void via6522_device::clear_int(int data)
{
	if (m_ifr & data)
	{
		m_ifr &= ~data;

		output_irq();

		if (TRACE_VIA)
		{
			logerror("%s:6522VIA chip %s: IFR = %02X\n", machine().describe_context(), tag(), m_ifr);
		}
	}
}


/*-------------------------------------------------
    via_shift
-------------------------------------------------*/

void via6522_device::shift_out()
{
	m_out_cb2 = (m_sr >> 7) & 1;
	m_sr =  (m_sr << 1) | m_out_cb2;

	m_cb2_handler(m_out_cb2);

	if (!SO_T2_RATE(m_acr))
	{
		m_shift_counter = (m_shift_counter + 1) % 8;

		if (m_shift_counter == 0)
		{
			set_int(INT_SR);
		}
	}
}

void via6522_device::shift_in()
{
	m_sr =  (m_sr << 1) | (m_in_cb2 & 1);

	m_shift_counter = (m_shift_counter + 1) % 8;

	if (m_shift_counter == 0)
	{
		set_int(INT_SR);
	}
}


void via6522_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_SHIFT:
			m_out_cb1 = 0;
			m_cb1_handler(m_out_cb1);

			if (SO_T2_RATE(m_acr) || SO_T2_CONTROL(m_acr) || SO_O2_CONTROL(m_acr))
			{
				shift_out();
			}

			m_out_cb1 = 1;
			m_cb1_handler(m_out_cb1);

			if (SI_T2_CONTROL(m_acr) || SI_O2_CONTROL(m_acr))
			{
				shift_in();
			}

			if (SO_T2_RATE(m_acr) || m_shift_counter)
			{
				if (SI_O2_CONTROL(m_acr) || SO_O2_CONTROL(m_acr))
				{
					m_shift_timer->adjust(clocks_to_attotime(2));
				}
				else
				{
					m_shift_timer->adjust(clocks_to_attotime((m_t2ll + 2)*2));
				}
			}
			break;

		case TIMER_T1:
			if (T1_CONTINUOUS (m_acr))
			{
				m_t1_pb7 = !m_t1_pb7;
				m_t1->adjust(clocks_to_attotime(TIMER1_VALUE + IFR_DELAY));
			}
			else
			{
				m_t1_pb7 = 1;
				m_t1_active = 0;
				m_time1 = machine().time();
			}

			if (T1_SET_PB7(m_acr))
			{
				output_pb();
			}

			set_int(INT_T1);
			break;

		case TIMER_T2:
			m_t2_active = 0;
			m_time2 = machine().time();

			set_int(INT_T2);
			break;

		case TIMER_CA2:
			m_out_ca2 = 1;
			m_ca2_handler(m_out_ca2);
			break;
	}
}

UINT8 via6522_device::input_pa()
{
	/// TODO: REMOVE THIS
	if (!m_in_a_handler.isnull())
	{
		if (m_ddr_a != 0xff)
			m_in_a = m_in_a_handler(0);

		return (m_out_a & m_ddr_a) + (m_in_a & ~m_ddr_a);
	}

	return m_in_a & (m_out_a | ~m_ddr_a);
}

void via6522_device::output_pa()
{
	UINT8 pa = (m_out_a & m_ddr_a) | ~m_ddr_a;
	m_out_a_handler(pa);
}

UINT8 via6522_device::input_pb()
{
	/// TODO: REMOVE THIS
	if (m_ddr_b != 0xff && !m_in_b_handler.isnull())
	{
		m_in_b = m_in_b_handler(0);
	}

	UINT8 pb = (m_out_b & m_ddr_b) + (m_in_b & ~m_ddr_b);

	if (T1_SET_PB7(m_acr))
		pb = (pb & 0x7f) | (m_t1_pb7 << 7);

	return pb;
}

void via6522_device::output_pb()
{
	UINT8 pb = (m_out_b & m_ddr_b) | ~m_ddr_b;

	if (T1_SET_PB7(m_acr))
		pb = (pb & 0x7f) | (m_t1_pb7 << 7);

	m_out_b_handler(pb);
}

/*-------------------------------------------------
    via_r - CPU interface for VIA read
-------------------------------------------------*/

READ8_MEMBER( via6522_device::read )
{
	int val = 0;
	if (space.debugger_access())
		return 0;

	offset &= 0xf;

	switch (offset)
	{
	case VIA_PB:
		/* update the input */
		if (PB_LATCH_ENABLE(m_acr) == 0)
		{
			val = input_pb();
		}
		else
		{
			val = m_latch_b;
		}

		CLR_PB_INT();
		break;

	case VIA_PA:
		/* update the input */
		if (PA_LATCH_ENABLE(m_acr) == 0)
		{
			val = input_pa();
		}
		else
		{
			val = m_latch_a;
		}

		CLR_PA_INT();

		if (m_out_ca2 && (CA2_PULSE_OUTPUT(m_pcr) || CA2_AUTO_HS(m_pcr)))
		{
			m_out_ca2 = 0;
			m_ca2_handler(m_out_ca2);
		}

		if (CA2_PULSE_OUTPUT(m_pcr))
			m_ca2_timer->adjust(clocks_to_attotime(1));

		break;

	case VIA_PANH:
		/* update the input */
		if (PA_LATCH_ENABLE(m_acr) == 0)
		{
			val = input_pa();
		}
		else
		{
			val = m_latch_a;
		}
		break;

	case VIA_DDRB:
		val = m_ddr_b;
		break;

	case VIA_DDRA:
		val = m_ddr_a;
		break;

	case VIA_T1CL:
		clear_int(INT_T1);
		val = get_counter1_value() & 0xFF;
		break;

	case VIA_T1CH:
		val = get_counter1_value() >> 8;
		break;

	case VIA_T1LL:
		val = m_t1ll;
		break;

	case VIA_T1LH:
		val = m_t1lh;
		break;

	case VIA_T2CL:
		clear_int(INT_T2);
		if (m_t2_active)
		{
			val = attotime_to_clocks(m_t2->remaining()) & 0xff;
		}
		else
		{
			if (T2_COUNT_PB6(m_acr))
			{
				val = m_t2cl;
			}
			else
			{
				val = (0x10000 - (attotime_to_clocks(machine().time() - m_time2) & 0xffff) - 1) & 0xff;
			}
		}
		break;

	case VIA_T2CH:
		if (m_t2_active)
		{
			val = attotime_to_clocks(m_t2->remaining()) >> 8;
		}
		else
		{
			if (T2_COUNT_PB6(m_acr))
			{
				val = m_t2ch;
			}
			else
			{
				val = (0x10000 - (attotime_to_clocks(machine().time() - m_time2) & 0xffff) - 1) >> 8;
			}
		}
		break;

	case VIA_SR:
		val = m_sr;
		m_shift_counter=0;
		clear_int(INT_SR);
		if (SI_O2_CONTROL(m_acr))
		{
			m_shift_timer->adjust(clocks_to_attotime(2));
		}
		if (SI_T2_CONTROL(m_acr))
		{
			m_shift_timer->adjust(clocks_to_attotime((m_t2ll + 2)*2));
		}
		break;

	case VIA_PCR:
		val = m_pcr;
		break;

	case VIA_ACR:
		val = m_acr;
		break;

	case VIA_IER:
		val = m_ier | 0x80;
		break;

	case VIA_IFR:
		val = m_ifr;
		break;
	}
	return val;
}


/*-------------------------------------------------
    via_w - CPU interface for VIA write
-------------------------------------------------*/

WRITE8_MEMBER( via6522_device::write )
{
	offset &=0x0f;

	switch (offset)
	{
	case VIA_PB:
		m_out_b = data;

		if (m_ddr_b != 0)
		{
			output_pb();
		}

		CLR_PB_INT();

		if (m_out_cb2 && CB2_AUTO_HS(m_pcr))
		{
			m_out_cb2 = 0;
			m_cb2_handler(m_out_cb2);
		}
		break;

	case VIA_PA:
		m_out_a = data;

		if (m_ddr_a != 0)
		{
			output_pa();
		}

		CLR_PA_INT();

		if (m_out_ca2 && (CA2_PULSE_OUTPUT(m_pcr) || CA2_AUTO_HS(m_pcr)))
		{
			m_out_ca2 = 0;
			m_ca2_handler(m_out_ca2);
		}

		if (CA2_PULSE_OUTPUT(m_pcr))
			m_ca2_timer->adjust(clocks_to_attotime(1));

		break;

	case VIA_PANH:
		m_out_a = data;

		if (m_ddr_a != 0)
		{
			output_pa();
		}

		break;

	case VIA_DDRB:
		if ( data != m_ddr_b )
		{
			m_ddr_b = data;

			output_pb();
		}
		break;

	case VIA_DDRA:
		if (m_ddr_a != data)
		{
			m_ddr_a = data;

			output_pa();
		}
		break;

	case VIA_T1CL:
	case VIA_T1LL:
		m_t1ll = data;
		break;

	case VIA_T1LH:
		m_t1lh = data;
		clear_int(INT_T1);
		break;

	case VIA_T1CH:
		m_t1ch = m_t1lh = data;
		m_t1cl = m_t1ll;

		clear_int(INT_T1);

		m_t1_pb7 = 0;

		if (T1_SET_PB7(m_acr))
		{
			output_pb();
		}

		m_t1->adjust(clocks_to_attotime(TIMER1_VALUE + IFR_DELAY));
		m_t1_active = 1;
		break;

	case VIA_T2CL:
		m_t2ll = data;
		break;

	case VIA_T2CH:
		m_t2ch = m_t2lh = data;
		m_t2cl = m_t2ll;

		clear_int(INT_T2);

		if (!T2_COUNT_PB6(m_acr))
		{
			m_t2->adjust(clocks_to_attotime(TIMER2_VALUE + IFR_DELAY));
			m_t2_active = 1;
		}
		else
		{
			m_t2->adjust(clocks_to_attotime(TIMER2_VALUE));
			m_t2_active = 1;
			m_time2 = machine().time();
		}
		break;

	case VIA_SR:
		m_sr = data;
		m_shift_counter=0;
		clear_int(INT_SR);
		if (SO_O2_CONTROL(m_acr))
		{
			m_shift_timer->adjust(clocks_to_attotime(2));
		}
		if (SO_T2_RATE(m_acr) || SO_T2_CONTROL(m_acr))
		{
			m_shift_timer->adjust(clocks_to_attotime((m_t2ll + 2)*2));
		}
		break;

	case VIA_PCR:
		m_pcr = data;

		if (TRACE_VIA)
		{
			logerror("%s:6522VIA chip %s: PCR = %02X\n", machine().describe_context(), tag(), data);
		}

		if (CA2_FIX_OUTPUT(data) && m_out_ca2 != CA2_OUTPUT_LEVEL(data))
		{
			m_out_ca2 = CA2_OUTPUT_LEVEL(data);
			m_ca2_handler(m_out_ca2);
		}

		if (CB2_FIX_OUTPUT(data) && m_out_cb2 != CB2_OUTPUT_LEVEL(data))
		{
			m_out_cb2 = CB2_OUTPUT_LEVEL(data);
			m_cb2_handler(m_out_cb2);
		}
		break;

	case VIA_ACR:
		{
			UINT16 counter1 = get_counter1_value();
			m_acr = data;

			output_pb();

			if (T1_CONTINUOUS(data))
			{
				m_t1->adjust(clocks_to_attotime(counter1 + IFR_DELAY));
				m_t1_active = 1;
			}
		}
		break;

	case VIA_IER:
		if (data & 0x80)
		{
			m_ier |= data & 0x7f;
		}
		else
		{
			m_ier &= ~(data & 0x7f);
		}

		output_irq();
		break;

	case VIA_IFR:
		if (data & INT_ANY)
		{
			data = 0x7f;
		}
		clear_int(data);
		break;
	}
}

void via6522_device::write_pa(int line, int state)
{
	if (state)
		m_in_a |= (1 << line);
	else
		m_in_a &= ~(1 << line);
}

WRITE8_MEMBER( via6522_device::write_pa )
{
	m_in_a = data;
}

/*-------------------------------------------------
    ca1_w - interface setting VIA port CA1 input
-------------------------------------------------*/

WRITE_LINE_MEMBER( via6522_device::write_ca1 )
{
	if (m_in_ca1 != state)
	{
		m_in_ca1 = state;

		if (TRACE_VIA)
			logerror("%s:6522VIA chip %s: CA1 = %02X\n", machine().describe_context(), tag(), m_in_ca1);

		if ((m_in_ca1 && CA1_LOW_TO_HIGH(m_pcr)) || (!m_in_ca1 && CA1_HIGH_TO_LOW(m_pcr)))
		{
			if (PA_LATCH_ENABLE(m_acr))
			{
				m_latch_a = input_pa();
			}

			set_int(INT_CA1);

			if (!m_out_ca2 && CA2_AUTO_HS(m_pcr))
			{
				m_out_ca2 = 1;
				m_ca2_handler(m_out_ca2);
			}
		}
	}
}


/*-------------------------------------------------
    ca2_w - interface setting VIA port CA2 input
-------------------------------------------------*/

WRITE_LINE_MEMBER( via6522_device::write_ca2 )
{
	if (m_in_ca2 != state)
	{
		m_in_ca2 = state;

		if (CA2_INPUT(m_pcr))
		{
			if ((m_in_ca2 && CA2_LOW_TO_HIGH(m_pcr)) || (!m_in_ca2 && CA2_HIGH_TO_LOW(m_pcr)))
			{
				set_int(INT_CA2);
			}
		}
	}
}

void via6522_device::write_pb(int line, int state)
{
	if (state)
		m_in_b |= (1 << line);
	else
		m_in_b &= ~(1 << line);
}

WRITE8_MEMBER( via6522_device::write_pb )
{
	m_in_b = data;
}

/*-------------------------------------------------
    cb1_w - interface setting VIA port CB1 input
-------------------------------------------------*/

WRITE_LINE_MEMBER( via6522_device::write_cb1 )
{
	if (m_in_cb1 != state)
	{
		m_in_cb1 = state;

		if ((m_in_cb1 && CB1_LOW_TO_HIGH(m_pcr)) || (!m_in_cb1 && CB1_HIGH_TO_LOW(m_pcr)))
		{
			if (PB_LATCH_ENABLE(m_acr))
			{
				m_latch_b = input_pb();
			}

			if (SO_EXT_CONTROL(m_acr))
			{
				shift_out();
			}

			if (SI_EXT_CONTROL(m_acr))
			{
				shift_in();
			}

			set_int(INT_CB1);

			if (!m_out_cb2 && CB2_AUTO_HS(m_pcr))
			{
				m_out_cb2 = 1;
				m_cb2_handler(1);
			}
		}
	}
}


/*-------------------------------------------------
    cb2_w - interface setting VIA port CB2 input
-------------------------------------------------*/

WRITE_LINE_MEMBER( via6522_device::write_cb2 )
{
	if (m_in_cb2 != state)
	{
		m_in_cb2 = state;

		if (CB2_INPUT(m_pcr))
		{
			if ((m_in_cb2 && CB2_LOW_TO_HIGH(m_pcr)) || (!m_in_cb2 && CB2_HIGH_TO_LOW(m_pcr)))
			{
				set_int(INT_CB2);
			}
		}
	}
}
