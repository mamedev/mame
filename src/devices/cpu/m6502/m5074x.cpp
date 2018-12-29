// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*
    Mitsubishi M5074x 8-bit microcontroller family
*/

#include "emu.h"
#include "m5074x.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define IRQ_CNTRREQ (0x80)
#define IRQ_CNTRENA (0x40)
#define IRQ_TMR1REQ (0x20)
#define IRQ_TMR1ENA (0x10)
#define IRQ_TMR2REQ (0x08)
#define IRQ_TMR2ENA (0x04)
#define IRQ_INTREQ  (0x02)
#define IRQ_INTENA  (0x01)

#define TMRC_TMRXREQ (0x80)
#define TMRC_TMRXENA (0x40)
#define TMRC_TMRXHLT (0x20)
#define TMRC_TMRXMDE (0x0c)

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(M50740, m50740_device, "m50740", "Mitsubishi M50740")
DEFINE_DEVICE_TYPE(M50741, m50741_device, "m50741", "Mitsubishi M50741")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m5074x_device - constructor
//-------------------------------------------------
m5074x_device::m5074x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map) :
	m740_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 13, 0, internal_map),
	m_read_p{{*this}, {*this}, {*this}, {*this}},
	m_write_p{{*this}, {*this}, {*this}, {*this}},
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
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m5074x_device::device_start()
{
	for (int i = 0; i < 4; i++)
	{
		m_read_p[i].resolve_safe(0);
		m_write_p[i].resolve_safe();
	}

	for (int i = 0; i < NUM_TIMERS; i++)
	{
		m_timers[i] = timer_alloc(i, nullptr);
	}

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

void m5074x_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_1:
			m_tmr1--;

			if (m_tmr1 <= 0)
			{
				m_intctrl |= IRQ_TMR1REQ;
				m_tmr1 = m_tmr1latch;
				recalc_irqs();
			}
			break;

		case TIMER_2:
			m_tmr2--;

			if (m_tmr2 <= 0)
			{
				m_intctrl |= IRQ_TMR2REQ;
				m_tmr2 = m_tmr2latch;
				recalc_irqs();
			}
			break;

		case TIMER_X:
			m_tmrx--;

			if (m_tmrx <= 0)
			{
				m_tmrctrl |= TMRC_TMRXREQ;
				m_tmrx = m_tmrxlatch;
				recalc_irqs();
			}
			break;
	}
}

void m5074x_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
		case M5074X_INT1_LINE:
			if (state == ASSERT_LINE)
			{
				m_intctrl |= IRQ_INTREQ;
			}
			else
			{
				m_intctrl &= ~IRQ_INTREQ;
			}
			break;

		case M5074X_SET_OVERFLOW:   // the base 740 class can handle this
			m740_device::execute_set_input(M740_SET_OVERFLOW, state);
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
			m_timers[TIMER_1]->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
			break;

		case 1:
			hz = clock() / 16;
			hz /= (m_tmr12pre + 2);
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
					m_timers[TIMER_X]->adjust(attotime::never, 0, attotime::never);
				}
				else
				{
					hz = clock() / 16;
					hz /= (m_tmrxpre + 2);
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

void m5074x_device::send_port(address_space &space, uint8_t offset, uint8_t data)
{
	m_write_p[offset](data);
}

uint8_t m5074x_device::read_port(uint8_t offset)
{
	uint8_t incoming = m_read_p[offset]();

	// apply data direction registers
	incoming &= (m_ddrs[offset] ^ 0xff);
	// OR in ddr-masked version of port writes
	incoming |= (m_ports[offset] & m_ddrs[offset]);

	return incoming;
}

READ8_MEMBER(m5074x_device::ports_r)
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
	}

	return 0xff;
}

WRITE8_MEMBER(m5074x_device::ports_w)
{
	switch (offset)
	{
		case 0: // p0
			send_port(space, 0, data & m_ddrs[0]);
			m_ports[0] = data;
			break;

		case 1: // p0 ddr
			send_port(space, 0, m_ports[0] & data);
			m_ddrs[0] = data;
			break;

		case 2: // p1
			send_port(space, 1, data & m_ddrs[1]);
			m_ports[1] = data;
			break;

		case 3: // p1 ddr
			send_port(space, 1, m_ports[1] & data);
			m_ddrs[1] = data;
			break;

		case 4: // p2
			send_port(space, 2, data & m_ddrs[2]);
			m_ports[2] = data;
			break;

		case 5: // p2 ddr
			send_port(space, 2, m_ports[2] & data);
			m_ddrs[2] = data;
			break;

		case 8: // p3
			send_port(space, 3, data & m_ddrs[3]);
			m_ports[3] = data;
			break;

		case 9: // p3 ddr
			send_port(space, 3, m_ports[3] & data);
			m_ddrs[3] = data;
			break;
	}
}

READ8_MEMBER(m5074x_device::tmrirq_r)
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

WRITE8_MEMBER(m5074x_device::tmrirq_w)
{
//  printf("%02x to tmrirq @ %d\n", data, offset);

	switch (offset)
	{
		case 0:
			m_tmr12pre = data;
			recalc_timer(0);
			recalc_timer(1);
			break;

		case 1:
			m_tmr1 = m_tmr1latch = data;
			break;

		case 2:
			m_tmr2 = m_tmr2latch = data;
			break;

		case 3:
			m_tmrxpre = m_tmrxlatch = data;
			recalc_timer(2);
			break;

		case 4:
			m_tmrx = data;
			break;

		case 5:
			m_intctrl = data;
			recalc_irqs();
			break;

		case 6:
			m_tmrctrl = data;
			recalc_irqs();
			break;
	}
}

/* M50740 - baseline for this familiy */
void m50740_device::m50740_map(address_map &map)
{
	map(0x0000, 0x005f).ram();
	map(0x00e0, 0x00e9).rw(FUNC(m50740_device::ports_r), FUNC(m50740_device::ports_w));
	map(0x00f9, 0x00ff).rw(FUNC(m50740_device::tmrirq_r), FUNC(m50740_device::tmrirq_w));
	map(0x1400, 0x1fff).rom().region(M5074X_INTERNAL_ROM_REGION, 0);
}

m50740_device::m50740_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m50740_device(mconfig, M50740, tag, owner, clock)
{
}

m50740_device::m50740_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m5074x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(m50740_device::m50740_map), this))
{
}

/* M50741 - 50740 with a larger internal ROM */
void m50741_device::m50741_map(address_map &map)
{
	map(0x0000, 0x005f).ram();
	map(0x00e0, 0x00e9).rw(FUNC(m50741_device::ports_r), FUNC(m50741_device::ports_w));
	map(0x00f9, 0x00ff).rw(FUNC(m50741_device::tmrirq_r), FUNC(m50741_device::tmrirq_w));
	map(0x1000, 0x1fff).rom().region("internal", 0);
}

m50741_device::m50741_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m50741_device(mconfig, M50741, tag, owner, clock)
{
}

m50741_device::m50741_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m5074x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(m50741_device::m50741_map), this))
{
}
