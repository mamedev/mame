// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*
    Mitsubishi M5074x/5075x 8-bit microcontroller family
*/

#include "emu.h"
#include "m5074x.h"

#define LOG_ADC         (1U << 1)
#define LOG_PORTS       (1U << 2)
#define LOG_TIMER       (1U << 3)

#define VERBOSE (0)
#include "logmacro.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

static constexpr u8 IRQ_CNTRREQ = 0x80;
static constexpr u8 IRQ_CNTRENA = 0x40;
static constexpr u8 IRQ_TMR1REQ = 0x20;
static constexpr u8 IRQ_TMR1ENA = 0x10;
static constexpr u8 IRQ_TMR2REQ = 0x08;
static constexpr u8 IRQ_TMR2ENA = 0x04;
static constexpr u8 IRQ_INTREQ  = 0x02;
static constexpr u8 IRQ_INTENA  = 0x01;

static constexpr u8 TMRC_TMRXREQ = 0x80;
static constexpr u8 TMRC_TMRXENA = 0x40;
static constexpr u8 TMRC_TMRXHLT = 0x20;
static constexpr u8 TMRC_TMRXMDE = 0x0c;

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(M50740, m50740_device, "m50740", "Mitsubishi M50740")
DEFINE_DEVICE_TYPE(M50741, m50741_device, "m50741", "Mitsubishi M50741")
DEFINE_DEVICE_TYPE(M50753, m50753_device, "m50753", "Mitsubishi M50753")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m5074x_device - constructor
//-------------------------------------------------
m5074x_device::m5074x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int addrbits, address_map_constructor internal_map) :
	m740_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, addrbits, 0, internal_map),
	m_read_p(*this, 0),
	m_write_p(*this),
	m_intctrl(0),
	m_tmrctrl(0),
	m_tmr12pre(0),
	m_tmr1(0),
	m_tmr2(0),
	m_tmrxpre(0),
	m_tmrx(0),
	m_tmr1latch(0),
	m_tmr2latch(0),
	m_tmrxlatch(0),
	m_last_all_ints(0)
{
	std::fill(std::begin(m_pullups), std::end(m_pullups), 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m5074x_device::device_start()
{
	m_timers[TIMER_1] = timer_alloc(FUNC(m5074x_device::timer1_tick), this);
	m_timers[TIMER_2] = timer_alloc(FUNC(m5074x_device::timer2_tick), this);
	m_timers[TIMER_X] = timer_alloc(FUNC(m5074x_device::timerx_tick), this);
	m_timers[TIMER_ADC] = timer_alloc(FUNC(m5074x_device::adc_complete), this);

	m740_device::device_start();

	save_item(NAME(m_ports));
	save_item(NAME(m_ddrs));
	save_item(NAME(m_intctrl));
	save_item(NAME(m_tmrctrl));
	save_item(NAME(m_tmr12pre));
	save_item(NAME(m_tmr1));
	save_item(NAME(m_tmr2));
	save_item(NAME(m_tmrxpre));
	save_item(NAME(m_tmrx));
	save_item(NAME(m_tmr1latch));
	save_item(NAME(m_tmr2latch));
	save_item(NAME(m_tmrxlatch));
	save_item(NAME(m_last_all_ints));

	memset(m_ports, 0, sizeof(m_ports));
	memset(m_ddrs, 0, sizeof(m_ddrs));
	m_intctrl = m_tmrctrl = 0;
	m_tmr12pre = m_tmrxpre = 0;
	m_tmr1 = m_tmr2 = m_tmrx = 0;
	m_last_all_ints = 0;
}


device_memory_interface::space_config_vector m5074x_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m5074x_device::device_reset()
{
	m740_device::device_reset();

	// all ports reset to input on startup
	memset(m_ports, 0, sizeof(m_ports));
	memset(m_ddrs, 0, sizeof(m_ddrs));
	m_intctrl = m_tmrctrl = 0;
	m_tmr12pre = m_tmrxpre = 0;
	m_tmr1 = m_tmr2 = m_tmrx = 0;
}

TIMER_CALLBACK_MEMBER(m5074x_device::timer1_tick)
{
	m_tmr1--;

	if (m_tmr1 <= 0)
	{
		m_intctrl |= IRQ_TMR1REQ;
		m_tmr1 = m_tmr1latch;
		recalc_irqs();
	}
}

TIMER_CALLBACK_MEMBER(m5074x_device::timer2_tick)
{
	m_tmr2--;

	if (m_tmr2 <= 0)
	{
		m_intctrl |= IRQ_TMR2REQ;
		m_tmr2 = m_tmr2latch;
		recalc_irqs();
	}
}

TIMER_CALLBACK_MEMBER(m5074x_device::timerx_tick)
{
	m_tmrx--;

	if (m_tmrx == 0)
	{
		m_tmrctrl |= TMRC_TMRXREQ;
		m_tmrx = m_tmrxlatch;
		recalc_irqs();
	}
}

void m5074x_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case M5074X_INT1_LINE:
		// FIXME: edge-triggered
		if (state == ASSERT_LINE)
		{
			m_intctrl |= IRQ_INTREQ;
		}
		break;
	}

	recalc_irqs();
}

void m5074x_device::recalc_irqs()
{
	uint8_t all_ints = 0;

	if ((m_intctrl & (IRQ_CNTRREQ|IRQ_CNTRENA)) == (IRQ_CNTRREQ|IRQ_CNTRENA))
	{
		all_ints |= 0x01;
	}
	if ((m_tmrctrl & (TMRC_TMRXREQ|TMRC_TMRXENA)) == (TMRC_TMRXREQ|TMRC_TMRXENA))
	{
		all_ints |= 0x02;
	}
	if ((m_intctrl & (IRQ_TMR1REQ|IRQ_TMR1ENA)) == (IRQ_TMR1REQ|IRQ_TMR1ENA))
	{
		all_ints |= 0x04;
	}
	if ((m_intctrl & (IRQ_TMR2REQ|IRQ_TMR2ENA)) == (IRQ_TMR2REQ|IRQ_TMR2ENA))
	{
		all_ints |= 0x08;
	}
	if ((m_intctrl & (IRQ_INTREQ|IRQ_INTENA)) == (IRQ_INTREQ|IRQ_INTENA))
	{
		all_ints |= 0x10;
	}

	// check all 5 IRQ bits for changes
	for (int i = 0; i < 5; i++)
	{
		// if bit is set now
		if (all_ints & (1 << i))
		{
			// and wasn't last time
			if (!(m_last_all_ints & (1 << i)))
			{
				m740_device::execute_set_input(M740_INT0_LINE + i, ASSERT_LINE);
			}
		}
		else    // bit is clear now
		{
			// ...and wasn't clear last time
			if (m_last_all_ints & (1 << i))
			{
				m740_device::execute_set_input(M740_INT0_LINE + i, CLEAR_LINE);
			}
		}
	}

	m_last_all_ints = all_ints;
}

void m5074x_device::recalc_timer(int timer)
{
	int hz;

	switch (timer)
	{
		case 0:
			hz = clock() / 16;
			hz /= (m_tmr12pre + 2);
			LOGMASKED(LOG_TIMER, "%s: timer 1, prescale %02x, fire at %d Hz\n", machine().describe_context(), m_tmr12pre, hz);
			m_timers[TIMER_1]->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
			break;

		case 1:
			hz = clock() / 16;
			hz /= (m_tmr12pre + 2);
			LOGMASKED(LOG_TIMER, "%s: timer 2, prescale %02x, fire at %d Hz\n", machine().describe_context(), m_tmr12pre, hz);
			m_timers[TIMER_2]->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
			break;

		case 2:
			// Timer X modes: 00 = free run countdown, 01 = invert CNTR pin each time expires,
			// 10 = count each time CNTR pin inverts, 11 = count when CNTR pin low
			if ((m_tmrctrl & TMRC_TMRXMDE) == 0)
			{
				// stop bit?
				if (m_tmrctrl & TMRC_TMRXHLT)
				{
					LOGMASKED(LOG_TIMER, "%s: timer X halted\n", machine().describe_context());
					m_timers[TIMER_X]->adjust(attotime::never, 0, attotime::never);
				}
				else
				{
					hz = clock() / 16;
					hz /= (m_tmrxpre + 2);
					LOGMASKED(LOG_TIMER, "%s: timer X, prescale %02x, fire at %d Hz\n", machine().describe_context(), m_tmrxpre, hz);
					m_timers[TIMER_X]->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
				}
			}
			else
			{
				fatalerror("M5074x: Unhandled timer X mode %d\n", (m_tmrctrl&TMRC_TMRXMDE)>>2);
			}
			break;
	}
}

void m5074x_device::send_port(uint8_t offset, uint8_t data)
{
	LOGMASKED(LOG_PORTS, "%s: Write port %d, data %02x DDR %02x pull-ups %02x\n", machine().describe_context(), offset,
			  data, m_ddrs[offset], m_pullups[offset]);

	m_write_p[offset](data);
}

uint8_t m5074x_device::read_port(uint8_t offset)
{
	uint8_t incoming = m_read_p[offset]();

	// apply data direction registers
	incoming &= (m_ddrs[offset] ^ 0xff);
	// OR in ddr-masked version of port writes
	incoming |= (m_ports[offset] & m_ddrs[offset]);

	LOGMASKED(LOG_PORTS, "%s: Read port %d, incoming %02x DDR %02x output latch %02x\n", machine().describe_context(), offset,
			m_read_p[offset](), m_ddrs[offset], m_ports[offset]);

	return incoming;
}

uint8_t m5074x_device::ports_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return read_port(0);

		case 1:
			return m_ddrs[0];

		case 2:
			return read_port(1);

		case 3:
			return m_ddrs[1];

		case 4:
			return read_port(2);

		case 5:
			return m_ddrs[2];

		case 8:
			return read_port(3);

		case 9:
			return m_ddrs[3];

		case 0xa:
			return read_port(4) & 0x0f;

		case 0xb:
			return m_ddrs[4];
	}

	return 0xff;
}

void m5074x_device::ports_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // p0
			send_port(0, (data & m_ddrs[0]) | (m_pullups[0] & ~m_ddrs[0]));
			m_ports[0] = data;
			break;

		case 1: // p0 ddr
			send_port(0, (m_ports[0] & data) | (m_pullups[0] & ~data));
			m_ddrs[0] = data;
			break;

		case 2: // p1
			send_port(1, (data & m_ddrs[1]) | (m_pullups[1] & ~m_ddrs[1]));
			m_ports[1] = data;
			break;

		case 3: // p1 ddr
			send_port(1, (m_ports[1] & data) | (m_pullups[1] & ~data));
			m_ddrs[1] = data;
			break;

		case 4: // p2
			send_port(2, (data & m_ddrs[2]) | (m_pullups[2] & ~m_ddrs[2]));
			m_ports[2] = data;
			break;

		case 5: // p2 ddr
			send_port(2, (m_ports[2] & data) | (m_pullups[2] & ~data));
			m_ddrs[2] = data;
			break;

		case 8: // p3
			send_port(3, (data & m_ddrs[3]) | (m_pullups[3] & ~m_ddrs[3]));
			m_ports[3] = data;
			break;

		case 9: // p3 ddr
			send_port(3, (m_ports[3] & data) | (m_pullups[3] & ~data));
			m_ddrs[3] = data;
			break;

		case 0xa: // p4 (4-bit open drain)
			send_port(4, (data & m_ddrs[4] & 0x0f) | (m_pullups[4] & ~m_ddrs[4]));
			m_ports[4] = data & 0x0f;
			break;

		case 0xb: // p4 ddr
			send_port(4, (m_ports[4] & data & 0x0f) | (m_pullups[4] & ~data));
			m_ddrs[4] = data & 0x0f;
			break;
	}
}

uint8_t m5074x_device::tmrirq_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return m_tmr12pre;

		case 1:
			return m_tmr1;

		case 2:
			return m_tmr2;

		case 3:
			return m_tmrxpre;

		case 4:
			return m_tmrx;

		case 5:
			return m_intctrl;

		case 6:
			return m_tmrctrl;
	}

	return 0xff;
}

void m5074x_device::tmrirq_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_tmr12pre = data;
			LOGMASKED(LOG_TIMER, "%s: timer 1/2 prescale %02x\n", machine().describe_context(), data);
			recalc_timer(0);
			recalc_timer(1);
			break;

		case 1:
			m_tmr1 = m_tmr1latch = data;
			LOGMASKED(LOG_TIMER, "%s: timer 1 latch %02x\n", machine().describe_context(), data);
			break;

		case 2:
			m_tmr2 = m_tmr2latch = data;
			LOGMASKED(LOG_TIMER, "%s: timer 2 latch %02x\n", machine().describe_context(), data);
			break;

		case 3:
			m_tmrxpre = data;
			LOGMASKED(LOG_TIMER, "%s: timer X prescale %02x\n", machine().describe_context(), data);
			recalc_timer(2);
			break;

		case 4:
			m_tmrx = m_tmrxlatch = data;
			LOGMASKED(LOG_TIMER, "%s: timer X latch %02x\n", machine().describe_context(), data);
			break;

		case 5:
			// Interrupt request bits can only be reset
			m_intctrl = data & (m_intctrl | ~(IRQ_CNTRREQ | IRQ_INTREQ));
			recalc_irqs();
			break;

		case 6:
			m_tmrctrl = data & (m_tmrctrl | ~TMRC_TMRXREQ);
			recalc_irqs();
			break;
	}
}

// M50740 - baseline for this family
void m50740_device::m50740_map(address_map &map)
{
	map(0x0000, 0x005f).ram();
	map(0x00e0, 0x00e9).rw(FUNC(m50740_device::ports_r), FUNC(m50740_device::ports_w));
	map(0x00f9, 0x00ff).rw(FUNC(m50740_device::tmrirq_r), FUNC(m50740_device::tmrirq_w));
	map(0x1400, 0x1fff).rom().region(DEVICE_SELF, 0);
}

m50740_device::m50740_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m50740_device(mconfig, M50740, tag, owner, clock)
{
}

m50740_device::m50740_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m5074x_device(mconfig, type, tag, owner, clock, 13, address_map_constructor(FUNC(m50740_device::m50740_map), this))
{
}

// M50741 - 50740 with a larger internal ROM
void m50741_device::m50741_map(address_map &map)
{
	map(0x0000, 0x005f).ram();
	map(0x00e0, 0x00e9).rw(FUNC(m50741_device::ports_r), FUNC(m50741_device::ports_w));
	map(0x00f9, 0x00ff).rw(FUNC(m50741_device::tmrirq_r), FUNC(m50741_device::tmrirq_w));
	map(0x1000, 0x1fff).rom().region(DEVICE_SELF, 0);
}

m50741_device::m50741_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m50741_device(mconfig, M50741, tag, owner, clock)
{
}

m50741_device::m50741_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m5074x_device(mconfig, type, tag, owner, clock, 13, address_map_constructor(FUNC(m50741_device::m50741_map), this))
{
}

// M50753 - M5074X with more pins, more RAM, more ROM, A-D, PWM, serial I/O (TODO)
void m50753_device::m50753_map(address_map &map)
{
	map(0x0000, 0x00bf).ram();
	map(0x00e0, 0x00eb).rw(FUNC(m50753_device::ports_r), FUNC(m50753_device::ports_w));
	map(0x00ee, 0x00ee).r(FUNC(m50753_device::in_r));
	map(0x00ef, 0x00ef).r(FUNC(m50753_device::ad_r));
	map(0x00f2, 0x00f2).nopr().w(FUNC(m50753_device::ad_start_w));
	map(0x00f3, 0x00f3).rw(FUNC(m50753_device::ad_control_r), FUNC(m50753_device::ad_control_w));
	map(0x00f5, 0x00f5).rw(FUNC(m50753_device::pwm_control_r), FUNC(m50753_device::pwm_control_w));
	map(0x00f9, 0x00ff).rw(FUNC(m50753_device::tmrirq_r), FUNC(m50753_device::tmrirq_w));
	map(0xe800, 0xffff).rom().region(DEVICE_SELF, 0);
}

// interrupt bits on 50753 are slightly different from the 740/741.
static constexpr u8 IRQ_50753_INT1REQ = 0x80;
static constexpr u8 IRQ_50753_INTADC = 0x20;
static constexpr u8 IRQ_50753_INT2REQ = 0x02;

m50753_device::m50753_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m50753_device(mconfig, M50753, tag, owner, clock)
{
}

m50753_device::m50753_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m5074x_device(mconfig, type, tag, owner, clock, 16, address_map_constructor(FUNC(m50753_device::m50753_map), this)),
	m_ad_in(*this, 0),
	m_in_p(*this, 0),
	m_ad_control(0),
	m_pwm_enabled(false)
{
}

void m50753_device::device_start()
{
	m5074x_device::device_start();

	save_item(NAME(m_ad_control));
	save_item(NAME(m_pwm_enabled));
}

void m50753_device::device_reset()
{
	m5074x_device::device_reset();

	m_ad_control = 0;
	m_pwm_enabled = false;
}

uint8_t m50753_device::in_r()
{
	return m_in_p();
}

uint8_t m50753_device::ad_r()
{
	m_intctrl &= ~IRQ_50753_INTADC;
	recalc_irqs();

	return m_ad_in[m_ad_control & 0x07]();
}

void m50753_device::ad_start_w(uint8_t data)
{
	LOGMASKED(LOG_ADC, "%s: A-D start (IN%d)\n", machine().describe_context(), m_ad_control & 0x07);

	// starting a conversion.  M50753 documentation says conversion time is 72 microseconds.
	m_timers[TIMER_ADC]->adjust(attotime::from_usec(72));
}

uint8_t m50753_device::ad_control_r()
{
	return m_ad_control;
}

void m50753_device::ad_control_w(uint8_t data)
{
	LOGMASKED(LOG_ADC, "%s: %02x to A-D control\n", machine().describe_context(), data);
	m_ad_control = data & 0x0f;
}

uint8_t m50753_device::pwm_control_r()
{
	return m_pwm_enabled ? 0x01 : 0x00;
}

void m50753_device::pwm_control_w(uint8_t data)
{
	m_pwm_enabled = BIT(data, 0);
}

void m50753_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case M50753_INT1_LINE:
		// FIXME: edge-triggered
		if (state == ASSERT_LINE)
		{
			m_intctrl |= IRQ_50753_INT1REQ;
		}
		break;

	case M50753_INT2_LINE:
		// FIXME: edge-triggered
		if (state == ASSERT_LINE)
		{
			m_intctrl |= IRQ_50753_INT2REQ;
		}
		break;
	}

	recalc_irqs();
}

TIMER_CALLBACK_MEMBER(m50753_device::adc_complete)
{
	m_timers[TIMER_ADC]->adjust(attotime::never);

	// if interrupt source is the ADC, do it.
	if (m_ad_control & 4)
	{
		m_intctrl |= IRQ_50753_INTADC;
		recalc_irqs();
	}
}
