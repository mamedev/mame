// license:BSD-3-Clause
// copyright-holders:Peter Trauner, Mathis Rosenhauer
/**********************************************************************

    Rockwell 6522 VIA interface and emulation

    This is based on the M6821 emulation in MAME.

    To do:
    Pulse mode handshake output

**********************************************************************/

#include "emu.h"
#include "6522via.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_SETUP   (1U << 1)
#define LOG_SHIFT   (1U << 2)
#define LOG_READ    (1U << 3)
#define LOG_INT     (1U << 4)

//#define VERBOSE (LOG_SHIFT|LOG_INT|LOG_SETUP)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGSHIFT(...) LOGMASKED(LOG_SHIFT,   __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,     __VA_ARGS__)


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

#define TIMER1_VALUE    (m_t1ll+(m_t1lh<<8))
#define TIMER2_VALUE    (m_t2ll+(m_t2lh<<8))

namespace {

// T2 runs as an 8-bit timer while it clocks the shift register
constexpr bool t2_shifter_clocked(u8 acr)
{
	return ((acr & 0x0c) == 0x04) || ((acr & 0x1c) == 0x10);
}

} // anonymous namespace


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

// CPU time advances by the truncated clock period, so a second holds slightly
// more than clock() periods; attotime_to_clocks() would drop one per second
uint64_t via6522_device::clocks_since(const attotime &start) const
{
	const attotime duration = machine().time() - start;
	const u64 period = clocks_to_attotime(1).attoseconds();
	const u64 slack = ATTOSECONDS_PER_SECOND - clock() * period;

	return u64(duration.seconds()) * clock() + (u64(duration.seconds()) * slack + duration.attoseconds()) / period;
}

uint16_t via6522_device::get_counter1_value() const
{
	// reloads in one-shot mode too: latch .. 1, 0, $ffff, latch
	// hold at 0 until t1_tick has set the interrupt flag
	if (m_t1_active && (m_t1->expire() <= machine().time()))
		return 0;

	const u32 period = TIMER1_VALUE + 2;
	const u64 elapsed = clocks_since(m_time1);
	const s64 e = elapsed ? s64(elapsed - 1) : 0;
	const s64 v = (m_t1_value == 0xffff) ? -1 : s64(m_t1_value);

	if (e <= v)
		return u16(v - e);
	if (e == v + 1)
		return 0xffff;

	const u32 phase = u32(u64(e - v - 2) % period);

	return (phase <= TIMER1_VALUE) ? (TIMER1_VALUE - phase) : 0xffff;
}

void via6522_device::reanchor_counter1()
{
	const bool underflow_due = m_t1_active && (m_t1->expire() <= machine().time());
	const uint16_t value = underflow_due ? 0xffff : get_counter1_value();

	m_time1 = machine().time() - clocks_to_attotime(1);
	m_t1_value = value;
}

uint32_t via6522_device::t2_underflow_delay() const
{
	if (!t2_shifter_clocked(m_acr))
		return TIMER2_VALUE + 2;

	return (m_t2_start & 0xff) + (m_t2_start >> 8) * (m_t2ll + 2) + 2;
}

uint16_t via6522_device::get_counter2_value() const
{
	if (T2_COUNT_PB6(m_acr))
	{
		return m_t2cl | (m_t2ch << 8);
	}

	// 8-bit mode: the low byte reloads from T2LL via $ff, the high byte borrows
	if (t2_shifter_clocked(m_acr))
	{
		const u32 lo0 = m_t2_start & 0xff;
		const u32 hi0 = m_t2_start >> 8;
		const u64 elapsed = clocks_since(m_t2_load);
		const u64 t = elapsed ? elapsed - 1 : 0;

		if (t <= lo0)
			return (hi0 << 8) | u32(lo0 - t);

		const u32 period = m_t2ll + 2;
		const u64 since = t - (lo0 + 1);
		const u32 hi = (hi0 - 1 - u32(since / period)) & 0xff;
		const u32 r = u32(since % period);

		return (hi << 8) | (r ? ((m_t2ll - (r - 1)) & 0xff) : 0xff);
	}

	if (m_t2_active && !m_t2->expire().is_never())
	{
		// hold at 0 until t2_tick sets IFR: Mac OS reads T2CH, then IFR, and
		// skips T2CL if the high byte is zero
		const u64 remaining = attotime_to_clocks(m_t2->remaining());
		return remaining ? u16(remaining - 1) : 0;
	}

	return (0x10000 - (clocks_since(m_time2) & 0xffff) - 1);
}

void via6522_device::counter2_decrement()
{
	if (!T2_COUNT_PB6(m_acr))
		return;

	// count down on T2CL
	if (m_t2cl-- != 0)
		return;

	// borrow from T2CH
	if (m_t2ch-- != 0)
		return;

	// underflow causes only one interrupt between T2CH writes
	if (m_t2_active)
	{
		m_t2_active = 0;

		LOGINT("T2 INT request ");
		set_int(INT_T2);
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(MOS6522, mos6522_device, "mos6522", "MOS 6522 VIA")
DEFINE_DEVICE_TYPE(R65C22, r65c22_device, "r65c22", "Rockwell R65C22 VIA")
DEFINE_DEVICE_TYPE(R65NC22, r65nc22_device, "r65nc22", "Rockwell R65NC22 VIA")
DEFINE_DEVICE_TYPE(W65C22S, w65c22s_device, "w65c22s", "WDC W65C22S VIA")

void via6522_device::map(address_map &map)
{
	map(0x00, 0x0f).rw(FUNC(via6522_device::read), FUNC(via6522_device::write));
}

//-------------------------------------------------
//  via6522_device - constructor
//-------------------------------------------------

via6522_device::via6522_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_in_cb1(0),
	m_in_cb2(0),
	m_acr(0),
	m_in_a_handler(*this, 0xff),
	m_in_b_handler(*this, 0xff),
	m_out_a_handler(*this),
	m_out_b_handler(*this),
	m_ca2_handler(*this),
	m_cb1_handler(*this),
	m_cb2_handler(*this),
	m_irq_handler(*this),
	m_in_a(0xff),
	m_in_ca1(0),
	m_in_ca2(0),
	m_in_b(0xff),
	m_pcr(0)
{
}


//-------------------------------------------------
//  mos6522_device - constructor
//-------------------------------------------------

mos6522_device::mos6522_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	via6522_device(mconfig, MOS6522, tag, owner, clock)
{
}


//-------------------------------------------------
//  r65c22_device - constructor
//-------------------------------------------------

r65c22_device::r65c22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	via6522_device(mconfig, R65C22, tag, owner, clock)
{
}


//-------------------------------------------------
//  r65c22_device - constructor
//-------------------------------------------------

r65nc22_device::r65nc22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	via6522_device(mconfig, R65NC22, tag, owner, clock)
{
}


//-------------------------------------------------
//  w65c22s_device - constructor
//-------------------------------------------------

w65c22s_device::w65c22s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	via6522_device(mconfig, W65C22S, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void via6522_device::device_start()
{
	m_t1ll = 0xf3; /* via at 0x9110 in vic20 show these values */
	m_t1lh = 0xb5; /* ports are not written by kernel! */
	m_t2ll = 0xff; /* taken from vice */
	m_t2lh = 0xff;

	m_time1 = machine().time();
	m_time2 = machine().time();
	m_t2_load = machine().time();

	m_t1 = timer_alloc(FUNC(via6522_device::t1_tick), this);
	m_t2 = timer_alloc(FUNC(via6522_device::t2_tick), this);
	m_ca2_timer = timer_alloc(FUNC(via6522_device::ca2_tick), this);
	m_cb2_timer = machine().scheduler().timer_alloc(timer_expired_delegate());
	m_shift_timer = timer_alloc(FUNC(via6522_device::shift_tick), this);
	m_shift_irq_timer = timer_alloc(FUNC(via6522_device::shift_irq_tick), this);

	// zerofill other
	m_out_a = 0;
	m_out_ca2 = 0;
	m_ddr_a = 0;
	m_latch_a = 0;
	m_out_b = 0;
	m_out_cb1 = 0;
	m_out_cb2 = 0;
	m_ddr_b = 0;
	m_latch_b = 0;

	// TODO: initial counter state unknown, but definitely not zero.
	m_t1cl = 0xff;
	m_t1ch = 0xff;
	m_t2cl = 0xff;
	m_t2ch = 0xff;
	m_shift_done = true;
	m_t1_value = 0;
	m_t2_start = 0xffff;

	m_sr = 0;
	m_pcr = 0;
	m_acr = 0;
	m_ier = 0;
	m_ifr = 0;

	m_t1_active = 0;
	m_t1_pb7 = 0;
	m_t2_active = 0;
	m_shift_counter = 0;

	// save state register
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
	save_item(NAME(m_shift_done));
	save_item(NAME(m_t1_value));
	save_item(NAME(m_t2_start));
	save_item(NAME(m_t2_load));
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

	m_pcr = 0;
	m_acr = 0;
	m_ier = 0;
	m_ifr = 0;
	m_t1_active = 0;
	m_t1_pb7 = 1;
	m_t2_active = 0;
	m_shift_done = true;

	output_pa();
	output_pb();
	m_ca2_handler(m_out_ca2);
	m_cb1_handler(m_out_cb1);
	m_cb2_handler(m_out_cb2);

	m_t1->adjust(attotime::never);
	m_t2->adjust(attotime::never);
	m_ca2_timer->adjust(attotime::never);
	m_cb2_timer->adjust(attotime::never);
	m_shift_timer->adjust(attotime::never);
	m_shift_irq_timer->adjust(attotime::never);
}


void via6522_device::output_irq()
{
	if (m_ier & m_ifr & 0x7f)
	{
		if ((m_ifr & INT_ANY) == 0)
		{
			LOGINT("INT asserted\n");
			m_ifr |= INT_ANY;
			m_irq_handler(ASSERT_LINE);
		}
	}
	else
	{
		if (m_ifr & INT_ANY)
		{
			LOGINT("INT cleared\n");
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

		LOGINT("granted\n");
		LOG("%s:6522VIA chip %s: IFR = %02X\n", machine().describe_context(), tag(), m_ifr);
	}
	else
	{
		LOGINT("denied\n");
	}
}


/*-------------------------------------------------
    via_clear_int - external interrupt check
-------------------------------------------------*/

void via6522_device::clear_int(int data)
{
	if (m_ifr & data)
	{
		LOGINT("cleared\n");
		m_ifr &= ~data;

		output_irq();

		LOG("%s:6522VIA chip %s: IFR = %02X\n", machine().describe_context(), tag(), m_ifr);
	}
	else
	{
		LOGINT("not cleared\n");
	}
}


/*-------------------------------------------------
    shift_blocked
-------------------------------------------------*/

bool via6522_device::shift_blocked() const
{
	return m_shift_done && !(SO_T2_RATE(m_acr) || SI_EXT_CONTROL(m_acr) || SO_EXT_CONTROL(m_acr));
}


/*-------------------------------------------------
    shift_clock_level
-------------------------------------------------*/

bool via6522_device::shift_clock_level() const
{
	return (SI_EXT_CONTROL(m_acr) || SO_EXT_CONTROL(m_acr) || SR_DISABLED(m_acr)) ? bool(m_in_cb1) : bool(m_out_cb1);
}


/*-------------------------------------------------
    shift_out
-------------------------------------------------*/

void via6522_device::shift_out()
{
	// Only shift out msb on falling edge
	if (!shift_clock_level())
	{
		uint8_t old_sr = m_sr;
		m_out_cb2 = (m_sr >> 7) & 1;
		m_sr =  (m_sr << 1) | m_out_cb2;
		m_shift_counter = (m_shift_counter - 1) & 7;
		LOGSHIFT("Shift Out SR bit %d (%d): %02x->%02x\n", m_shift_counter, m_out_cb2, old_sr, m_sr);

		m_cb2_handler(m_out_cb2);

		if (m_shift_counter == 0 && SO_EXT_CONTROL(m_acr))
		{
			m_shift_done = true;
			LOGINT("SHIFT EXT out INT request ");
			set_int(INT_SR); // IRQ on last falling edge for external clock (mode 7)
		}
	}
	else // Check for INT condition, eg the last and raising edge of the 15-0 falling/raising edges
	{
		if (!SO_T2_RATE(m_acr)) // The T2 continuous shifter doesn't do interrupts (mode 4)
		{
			if (m_shift_counter == 0 && (SO_O2_CONTROL(m_acr) || SO_T2_CONTROL(m_acr)))
			{
				m_shift_done = true;
				LOGINT("SHIFT O2/T2 out INT request ");
				set_int(INT_SR); // IRQ on last raising edge for internal clock (mode 5-6)
			}
		}
	}
}

void via6522_device::shift_in()
{
	// Only shift in data on raising edge
	if (shift_clock_level())
	{
		uint8_t old_sr = m_sr;
		m_sr =  (m_sr << 1) | (m_in_cb2 & 1);
		m_shift_counter = (m_shift_counter - 1) & 7;
		LOGSHIFT("Shift In SR bit %d (%d): %02x->%02x\n", m_shift_counter, m_in_cb2 & 1, old_sr, m_sr);

		if (m_shift_counter == 0 && !SR_DISABLED(m_acr))
		{
			m_shift_done = true;
			LOGINT("SHIFT in INT request ");
			if (SI_EXT_CONTROL(m_acr))
			{
				// Set IRQ immediately for external shifter clock.  PCI PowerMacs rely on this timing,
				// including the Pippin.
				set_int(INT_SR);
			}
			else
			{
				m_shift_irq_timer->adjust(clocks_to_attotime(2)/2); // Delay IRQ 2 edges for internal shift INs (mode 1-2)
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(via6522_device::shift_irq_tick)
{
	// This timer event is a delayed IRQ for improved cycle accuracy
	set_int(INT_SR);  // triggered from shift_in or shift_out on the last rising edge
	m_shift_irq_timer->adjust(attotime::never); // Not needed really...
}

TIMER_CALLBACK_MEMBER(via6522_device::shift_tick)
{
	// CB1 parks once the eight bits are done while T2 keeps running
	if (!shift_blocked())
	{
		LOGSHIFT("SHIFT timer event CB1 %s\n", m_out_cb1 & 1 ? "falling" : "raising");
		m_out_cb1 ^= 1;
		m_cb1_handler(m_out_cb1);

		if (SO_T2_RATE(m_acr) || SO_T2_CONTROL(m_acr) || SO_O2_CONTROL(m_acr))
		{
			shift_out();
		}
		else if (SI_T2_CONTROL(m_acr) || SI_O2_CONTROL(m_acr))
		{
			shift_in();
		}
	}

	if (SO_T2_RATE(m_acr) || SO_T2_CONTROL(m_acr) || SI_T2_CONTROL(m_acr))
	{
		m_shift_timer->adjust(clocks_to_attotime(m_t2ll + 2));
	}
	else if ((SI_O2_CONTROL(m_acr) || SO_O2_CONTROL(m_acr)) && !m_shift_done)
	{
		m_shift_timer->adjust(clocks_to_attotime(1));
	}
	else
	{
		m_shift_timer->adjust(attotime::never);
	}
}

TIMER_CALLBACK_MEMBER(via6522_device::t1_tick)
{
	// PB7 toggles on timeout in one-shot mode too, only while PB7 output is enabled
	if (T1_CONTINUOUS(m_acr))
	{
		if (TIMER1_VALUE > 0 && T1_SET_PB7(m_acr))
			m_t1_pb7 = !m_t1_pb7;
		m_t1->adjust(clocks_to_attotime(TIMER1_VALUE + 2));
	}
	else
	{
		if (T1_SET_PB7(m_acr))
			m_t1_pb7 = !m_t1_pb7;
		m_t1_active = 0;
	}

	if (T1_SET_PB7(m_acr))
	{
		output_pb();
	}

	LOGINT("T1 INT request ");
	set_int(INT_T1);
}

TIMER_CALLBACK_MEMBER(via6522_device::t2_tick)
{
	m_t2_active = 0;
	m_time2 = machine().time();

	LOGINT("T2 INT request ");
	set_int(INT_T2);
}

TIMER_CALLBACK_MEMBER(via6522_device::ca2_tick)
{
	m_out_ca2 = 1;
	m_ca2_handler(m_out_ca2);
}

uint8_t via6522_device::input_pa()
{
	// HACK: port a in the real 6522 does not mask off the output pins, but you can't trust handlers.
	if (!m_in_a_handler.isunset())
		return (m_in_a & ~m_ddr_a & m_in_a_handler()) | (m_out_a & m_ddr_a);
	else
		return (m_out_a | ~m_ddr_a) & m_in_a;
}

void via6522_device::output_pa()
{
	uint8_t pa = (m_out_a & m_ddr_a) | ~m_ddr_a;
	m_out_a_handler(pa);
}

uint8_t via6522_device::read_pa() const
{
	return (m_out_a & m_ddr_a) | ~m_ddr_a;
}

uint8_t via6522_device::input_pb()
{
	uint8_t pb = m_in_b & ~m_ddr_b;

	/// TODO: REMOVE THIS
	if (m_ddr_b != 0xff && !m_in_b_handler.isunset())
	{
		pb &= m_in_b_handler();
	}

	pb |= m_out_b & m_ddr_b;

	if (T1_SET_PB7(m_acr))
		pb = (pb & 0x7f) | (m_t1_pb7 << 7);

	return pb;
}

void via6522_device::output_pb()
{
	uint8_t pb = (m_out_b & m_ddr_b) | ~m_ddr_b;

	if (T1_SET_PB7(m_acr))
		pb = (pb & 0x7f) | (m_t1_pb7 << 7);

	m_out_b_handler(pb);
}

uint8_t via6522_device::read_pb() const
{
	uint8_t pb = (m_out_b & m_ddr_b) | ~m_ddr_b;

	if (T1_SET_PB7(m_acr))
		pb = (pb & 0x7f) | (m_t1_pb7 << 7);

	return pb;
}

/*-------------------------------------------------
    via_r - CPU interface for VIA read
-------------------------------------------------*/

u8 via6522_device::read(offs_t offset)
{
	int val = 0;
	offset &= 0xf;

	switch (offset)
	{
	case VIA_PB:
		/* update the input */
		if ((PB_LATCH_ENABLE(m_acr) != 0) && ((m_ifr & INT_CB1) != 0))
		{
			val = m_latch_b;
		}
		else
		{
			val = input_pb();
		}

		if (!machine().side_effects_disabled())
		{
			LOGINT("PB INT ");
			CLR_PB_INT();
		}
		break;

	case VIA_PA:
		/* update the input */
		if ((PA_LATCH_ENABLE(m_acr) != 0) && ((m_ifr & INT_CA1) != 0))
		{
			val = m_latch_a;
		}
		else
		{
			val = input_pa();
		}

		if (!machine().side_effects_disabled())
		{
			LOGINT("PA INT ");
			CLR_PA_INT();

			if (m_out_ca2 && (CA2_PULSE_OUTPUT(m_pcr) || CA2_AUTO_HS(m_pcr)))
			{
				m_out_ca2 = 0;
				m_ca2_handler(m_out_ca2);
			}

			if (CA2_PULSE_OUTPUT(m_pcr))
				m_ca2_timer->adjust(clocks_to_attotime(1));
		}

		break;

	case VIA_PANH:
		/* update the input */
		if ((PA_LATCH_ENABLE(m_acr) != 0) && ((m_ifr & INT_CA1) != 0))
		{
			val = m_latch_a;
		}
		else
		{
			val = input_pa();
		}
		break;

	case VIA_DDRB:
		val = m_ddr_b;
		break;

	case VIA_DDRA:
		val = m_ddr_a;
		break;

	case VIA_T1CL:
		if (!machine().side_effects_disabled())
		{
			LOGINT("T1CL INT ");
			clear_int(INT_T1);
		}
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
		if (!machine().side_effects_disabled())
		{
			LOGINT("T2CL INT ");
			clear_int(INT_T2);
		}
		val = get_counter2_value() & 0xff;
		break;

	case VIA_T2CH:
		val = get_counter2_value() >> 8;
		break;

	case VIA_SR:
		LOGSHIFT("Read SR: %02x ", m_sr);
		val = m_sr;
		if (!machine().side_effects_disabled())
		{
			// an access mid-shift neither restarts the count nor moves the clock
			if (!(SI_EXT_CONTROL(m_acr) || SO_EXT_CONTROL(m_acr)))
			{
				if ((m_ifr & INT_SR) || (m_shift_done && !SR_DISABLED(m_acr)))
				{
					m_shift_counter = 8;
					m_shift_done = false;
				}
			}
			else
			{
				m_shift_counter = 8;
				m_shift_done = false;
			}

			LOGINT("SR INT ");
			clear_int(INT_SR);
			LOGSHIFT(" - ACR: %02x ", m_acr);
			if (SI_O2_CONTROL(m_acr) || SO_O2_CONTROL(m_acr))
			{
				if (m_shift_timer->expire().is_never())
					m_shift_timer->adjust(clocks_to_attotime(2));
				LOGSHIFT(" - read SR starts O2 timer ");
			}
			else if (SI_T2_CONTROL(m_acr) || SO_T2_CONTROL(m_acr))
			{
				if (m_shift_timer->expire().is_never())
					m_shift_timer->adjust(clocks_to_attotime(m_t2ll + 2));
				LOGSHIFT(" - read SR starts T2 timer ");
			}
			else if (!SO_T2_RATE(m_acr))
			{
				m_shift_timer->adjust(attotime::never);
				LOGSHIFT("Timer stops");
			}
			LOGSHIFT("\n");
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
	LOGR(" * %s Reg %02x -> %02x - %s\n", tag(), offset, val, std::array<char const *, 16>
		 {{"IRB", "IRA", "DDRB", "DDRA", "T1CL","T1CH","T1LL","T1LH","T2CL","T2CH","SR","ACR","PCR","IFR","IER","IRA (nh)"}}[offset]);

	return val;
}


/*-------------------------------------------------
    via_w - CPU interface for VIA write
-------------------------------------------------*/

void via6522_device::write(offs_t offset, u8 data)
{
	offset &= 0x0f;

	LOGSETUP(" * %s Reg %02x <- %02x - %s\n", tag(), offset, data, std::array<char const *, 16>
		 {{"ORB", "ORA", "DDRB", "DDRA", "T1CL","T1CH","T1LL","T1LH","T2CL","T2CH","SR","ACR","PCR","IFR","IER","ORA (nh)"}}[offset]);

	switch (offset)
	{
	case VIA_PB:
		m_out_b = data;

		if (m_ddr_b != 0)
		{
			output_pb();
		}

		LOGINT("PB INT ");
		CLR_PB_INT();

		if (m_out_cb2 && (CB2_PULSE_OUTPUT(m_pcr) || CB2_AUTO_HS(m_pcr)))
		{
			m_out_cb2 = 0;
			m_cb2_handler(m_out_cb2);
		}

		if (CB2_PULSE_OUTPUT(m_pcr))
		{
			m_cb2_timer->adjust(clocks_to_attotime(1));
		}
		break;

	case VIA_PA:
		m_out_a = data;

		if (m_ddr_a != 0)
		{
			output_pa();
		}

		LOGINT("PA INT ");
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
		if (data != m_ddr_b)
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
		reanchor_counter1();
		m_t1ll = data;
		break;

	case VIA_T1LH:
		reanchor_counter1();
		m_t1lh = data;
		LOGINT("T1LH INT ");
		clear_int(INT_T1);
		break;

	case VIA_T1CH:
		m_t1ch = m_t1lh = data;
		m_t1cl = m_t1ll;

		LOGINT("T1CH INT ");
		clear_int(INT_T1);

		m_t1_pb7 = T1_SET_PB7(m_acr) ? 0 : 1;

		if (T1_SET_PB7(m_acr))
		{
			output_pb();
		}

		m_time1 = machine().time();
		m_t1_value = TIMER1_VALUE;
		m_t1->adjust(clocks_to_attotime(TIMER1_VALUE + 2));
		m_t1_active = 1;
		break;

	case VIA_T2CL:
		m_t2ll = data;
		break;

	case VIA_T2CH:
		m_t2ch = m_t2lh = data;
		m_t2cl = m_t2ll;

		LOGINT("T2 INT ");
		clear_int(INT_T2);

		if (!T2_COUNT_PB6(m_acr))
		{
			m_t2_start = TIMER2_VALUE;
			m_t2_load = machine().time();
			m_t2->adjust(clocks_to_attotime(t2_underflow_delay()));
			m_t2_active = 1;
		}
		else
		{
			m_t2->adjust(attotime::never);
			m_t2_active = 1;
			m_time2 = machine().time();
		}
		break;

	case VIA_SR:
		m_sr = data;
		LOGSHIFT("Write SR: %02x\n", m_sr);

		m_shift_counter = 8;
		m_shift_done = SR_DISABLED(m_acr);

		LOGINT("SR INT ");
		clear_int(INT_SR);
		LOGSHIFT(" - ACR is: %02x ", m_acr);
		if (SO_O2_CONTROL(m_acr) || SI_O2_CONTROL(m_acr))
		{
			if (m_shift_timer->expire().is_never())
				m_shift_timer->adjust(clocks_to_attotime(2));
			LOGSHIFT(" - write SR starts O2 timer");
		}
		else if (SO_T2_RATE(m_acr) || SO_T2_CONTROL(m_acr) || SI_T2_CONTROL(m_acr))
		{
			if (m_shift_timer->expire().is_never())
				m_shift_timer->adjust(clocks_to_attotime(m_t2ll + 2));
			LOGSHIFT(" - write starts T2 timer");
		}
		else
		{
			m_shift_timer->adjust(attotime::never); // In case we change mode before counter expire
			LOGSHIFT(" - timer stops");
		}
		LOGSHIFT("\n");
		break;

	case VIA_PCR:
		m_pcr = data;

		LOG("%s:6522VIA chip %s: PCR = %02X\n", machine().describe_context(), tag(), data);

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
			uint16_t counter2 = get_counter2_value();
			bool t2_was_pb6 = bool(T2_COUNT_PB6(m_acr));
			m_acr = data;
			LOGSHIFT("Write ACR: %02x ", m_acr);

			output_pb();

			LOGSHIFT("Shift mode [%02x]: ", (m_acr >> 2) & 7);
			if (SR_DISABLED(m_acr))    LOGSHIFT("Disabled");
			if (SI_T2_CONTROL(m_acr))  LOGSHIFT("IN on T2");
			if (SI_O2_CONTROL(m_acr))  LOGSHIFT("IN on O2");
			if (SI_EXT_CONTROL(m_acr)) LOGSHIFT("IN on EXT");
			if (SO_T2_RATE(m_acr))     LOGSHIFT("OUT on continuous T2");
			if (SO_T2_CONTROL(m_acr))  LOGSHIFT("OUT on T2");
			if (SO_O2_CONTROL(m_acr))  LOGSHIFT("OUT on O2");
			if (SO_EXT_CONTROL(m_acr)) LOGSHIFT("OUT on EXT");

			if (SR_DISABLED(m_acr) || SI_EXT_CONTROL(m_acr) || SO_EXT_CONTROL(m_acr))
			{
				m_shift_timer->adjust(attotime::never);
				if (SR_DISABLED(m_acr) && !m_out_cb1)
				{
					m_out_cb1 = 1;
					m_cb1_handler(m_out_cb1);
				}
				LOGSHIFT(" Timer stops");
			}
			else if ((SO_T2_RATE(m_acr) || SO_T2_CONTROL(m_acr) || SI_T2_CONTROL(m_acr))
					&& m_shift_timer->expire().is_never())
			{
				// CB1 edges follow the low byte's underflows, starting one period late
				m_shift_timer->adjust(clocks_to_attotime((counter2 & 0xff) + 3));
			}

			// the counter carries its value across a clock source change
			if (bool(T2_COUNT_PB6(m_acr)) != t2_was_pb6)
			{
				if (T2_COUNT_PB6(m_acr))
				{
					counter2 = (counter2 - 1) & 0xffff;
					m_t2cl = counter2 & 0xff;
					m_t2ch = counter2 >> 8;
					m_t2->adjust(attotime::never);
				}
				else if (m_t2_active)
				{
					// the new clock source takes effect one cycle after the write
					m_t2->adjust(clocks_to_attotime(counter2 + 2));
				}
				else
				{
					m_time2 = machine().time() - clocks_to_attotime(0xfffe - counter2);
				}
			}

			if (SI_T2_CONTROL(m_acr) || SI_O2_CONTROL(m_acr) || SI_EXT_CONTROL(m_acr))
			{
				m_out_cb2 = 1;
				m_cb2_handler(m_out_cb2);
			}

			LOGSHIFT("\n");
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
		LOGINT("IFR INT ");
		clear_int(data & 0x7f);
		break;
	}
}

void via6522_device::set_pa_line(int line, int state)
{
	if (state)
		m_in_a |= (1 << line);
	else
		m_in_a &= ~(1 << line);
}

void via6522_device::write_pa(u8 data)
{
	m_in_a = data;
}

/*-------------------------------------------------
    ca1_w - interface setting VIA port CA1 input
-------------------------------------------------*/

void via6522_device::write_ca1(int state)
{
	if (m_in_ca1 != state)
	{
		m_in_ca1 = state;

		LOG("%s:6522VIA chip %s: CA1 = %02X\n", machine().describe_context(), tag(), m_in_ca1);

		if ((m_in_ca1 && CA1_LOW_TO_HIGH(m_pcr)) || (!m_in_ca1 && CA1_HIGH_TO_LOW(m_pcr)))
		{
			if (PA_LATCH_ENABLE(m_acr))
			{
				m_latch_a = input_pa();
			}

			LOGINT("CA1 INT request ");
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

void via6522_device::write_ca2(int state)
{
	if (m_in_ca2 != state)
	{
		m_in_ca2 = state;

		if (CA2_INPUT(m_pcr))
		{
			if ((m_in_ca2 && CA2_LOW_TO_HIGH(m_pcr)) || (!m_in_ca2 && CA2_HIGH_TO_LOW(m_pcr)))
			{
				LOGINT("CA2 INT request ");
				set_int(INT_CA2);
			}
		}
	}
}

void via6522_device::set_pb_line(int line, int state)
{
	if (state)
		m_in_b |= (1 << line);
	else
	{
		if (line == 6 && BIT(m_in_b, 6))
			counter2_decrement();

		m_in_b &= ~(1 << line);
	}
}

void via6522_device::write_pb(u8 data)
{
	if (!BIT(data, 6) && BIT(m_in_b, 6))
		counter2_decrement();

	m_in_b = data;
}

/*-------------------------------------------------
    write_cb1 - interface setting VIA port CB1 input
-------------------------------------------------*/

void via6522_device::write_cb1(int state)
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
			LOGINT("CB1 INT request ");
			set_int(INT_CB1);

			if (!m_out_cb2 && CB2_AUTO_HS(m_pcr))
			{
				m_out_cb2 = 1;
				m_cb2_handler(1);
			}
		}

		// The shifter shift is not controlled by PCR
		if (SO_EXT_CONTROL(m_acr))
		{
			LOGSHIFT("SHIFT OUT EXT/CB1 falling edge, %d (CB1: %d)\n", m_shift_counter, m_in_cb1);
			shift_out();
		}
		else if (SI_EXT_CONTROL(m_acr) || SR_DISABLED(m_acr))
		{
			LOGSHIFT("SHIFT IN EXT/CB1 raising edge, %d (CB1: %d)\n", m_shift_counter, m_in_cb1);
			shift_in();
		}
	}
}


/*-------------------------------------------------
    write_cb2 - interface setting VIA port CB2 input
-------------------------------------------------*/

void via6522_device::write_cb2(int state)
{
	if (m_in_cb2 != state)
	{
		m_in_cb2 = state;
		LOGSHIFT("CB2 IN: %d\n", m_in_cb2);

		if (CB2_INPUT(m_pcr))
		{
			if ((m_in_cb2 && CB2_LOW_TO_HIGH(m_pcr)) || (!m_in_cb2 && CB2_HIGH_TO_LOW(m_pcr)))
			{
				LOGINT("CB2 INT request ");
				set_int(INT_CB2);
			}
		}
	}
}
