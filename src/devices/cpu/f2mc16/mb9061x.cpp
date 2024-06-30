// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Fujitsu Micro MB9061x Microcontroller Family
    Emulation by R. Belmont
*/

#include "emu.h"
#include "mb9061x.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MB90610A, mb90610_device, "mb90610a", "Fujitsu MB90610A")
DEFINE_DEVICE_TYPE(MB90611A, mb90611_device, "mb90611a", "Fujitsu MB90611A")
DEFINE_DEVICE_TYPE(MB90641A, mb90641_device, "mb90641a", "Fujitsu MB90641A")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mb9061x_device - constructor
//-------------------------------------------------
mb9061x_device::mb9061x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map) :
	f2mc16_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 24, 0, internal_map),
	m_read_port(*this, 0xff),
	m_write_port(*this),
	m_read_adcs(*this, 0xff),
	m_write_adcs(*this),
	m_read_adcr(*this, 0xff),
	m_write_adcr(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb9061x_device::device_start()
{
	f2mc16_device::device_start();

	m_tbtc_timer = timer_alloc(FUNC(mb9061x_device::tbtc_tick), this);
	m_tbtc_timer->adjust(attotime::never);
	m_timer[0] = timer_alloc(FUNC(mb9061x_device::timer0_tick), this);
	m_timer[0]->adjust(attotime::never);
	m_timer[1] = timer_alloc(FUNC(mb9061x_device::timer1_tick), this);
	m_timer[1]->adjust(attotime::never);
}


device_memory_interface::space_config_vector mb9061x_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb9061x_device::device_reset()
{
	f2mc16_device::device_reset();
	m_tbtc = 0;
	memset(m_timer_regs, 0, sizeof(m_timer_regs));
	memset(m_event_count, 0, sizeof(m_event_count));
	m_event_state[0] = m_event_state[1] = CLEAR_LINE;
}

void mb9061x_device::execute_set_input(int inputnum, int state)
{
}

/* MB90610 - "Evaluation device" with extra RAM */
void mb90610_device::mb90610_map(address_map &map)
{
	map(0x0001, 0x000a).rw(FUNC(mb9061x_device::port_r), FUNC(mb9061x_device::port_w));
	map(0x002c, 0x002d).rw(FUNC(mb9061x_device::adcs_r), FUNC(mb9061x_device::adcs_w));
	map(0x002e, 0x002f).rw(FUNC(mb9061x_device::adcr_r), FUNC(mb9061x_device::adcr_w));
	map(0x00a9, 0x00a9).rw(FUNC(mb9061x_device::tbtc_r), FUNC(mb9061x_device::tbtc_w));
	map(0x00b0, 0x00bf).rw(FUNC(mb9061x_device::intc_r), FUNC(mb9061x_device::intc_w));
	map(0x0100, 0x10ff).ram();  // 4K of internal RAM from 0x100 to 0x1100
}

mb90610_device::mb90610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mb90610_device(mconfig, MB90610A, tag, owner, clock)
{
}

mb90610_device::mb90610_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mb9061x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(mb90610_device::mb90610_map), this))
{
}

/* 16-bit preload timers with event count function */
enum
{
		TCTH_CSL1 = 0x08,       // clock source select bit 1
		TCTH_CSL0 = 0x04,       // clock source select bit 0
		TCTH_MOD2 = 0x02,       // mode bit 2
		TCTH_MOD1 = 0x01,       // mode bit 1

		TCTL_MOD0 = 0x80,   // mode bit 0
		TCTL_OUTE = 0x40,   // output enable
		TCTL_OUTL = 0x20,   // output level
		TCTL_RELD = 0x10,   // reload
		TCTL_INTE = 0x08,   // IRQ enable
		TCTL_UF   = 0x04,   // expire flag
		TCTL_CNTE = 0x02,   // enable counting
		TCTL_TRG  = 0x01    // force a reload and start counting
};

TIMER_CALLBACK_MEMBER(mb9061x_device::timer0_tick)
{
	u8 ctl = m_timer_regs[0];
	m_timer_regs[0] |= TCTL_UF;
	if (ctl & TCTL_INTE)
	{
//      printf("timer 0 IRQ\n");
		intc_trigger_irq(9, 0x1d);
	}

	if (ctl & TCTL_RELD)
	{
		recalc_timer(0);
		m_timer[0]->adjust(attotime::from_hz(m_timer_hz[0]));
	}
}

TIMER_CALLBACK_MEMBER(mb9061x_device::timer1_tick)
{
	u8 ctl = m_timer_regs[4];
	m_timer_regs[4] |= TCTL_UF;
	if (ctl & TCTL_INTE)
	{
 //       printf("timer 1 IRQ\n");
		intc_trigger_irq(9, 0x1e);
	}

	if (ctl & TCTL_RELD)
	{
		recalc_timer(0);
		m_timer[1]->adjust(attotime::from_hz(m_timer_hz[1]));
	}
}

u8 mb9061x_device::timer_r(offs_t offset)
{
	//printf("timer_r: offset %d = %02x\n", offset, m_timer_regs[offset]);
	return m_timer_regs[offset];
}

void mb9061x_device::timer_w(offs_t offset, u8 data)
{
	int timer = offset / 4;
	int reg = offset % 4;

	//printf("timer_w: %x to %d\n", data, offset);
#if 0
	switch (reg)
	{
		case 0: // control/status, lower
			printf("%02x to timer %d lower control\n", data, timer);
			break;

		case 1: // control/status, upper
			printf("%02x to timer %d upper control\n", data, timer);
			break;

		case 2: // timer/reload, lower
			printf("%02x to timer %d lower reload\n", data, timer);
			break;

		case 3: //  timer/reload, upper
			printf("%02x to timer %d upper reload\n", data, timer);
			break;
	}
#endif
	if (reg == 0)
	{
		m_timer_regs[offset] &= (TCTL_TRG|TCTL_UF);
		m_timer_regs[offset] |= data;
	}
	else
	{
		m_timer_regs[offset] = data;
	}

	int rbase = timer << 2;
	u8 csl = (m_timer_regs[rbase+1] >> 2) & 3;
	if (reg == 0)
	{
		if (data & TCTL_TRG)
		{
			//printf("Got TRG\n");
			recalc_timer(timer);
			if ((m_timer_regs[rbase] & TCTL_CNTE) && (csl != 3))
			{
				m_timer[timer]->adjust(attotime::from_hz(m_timer_hz[timer]));
			}
		}

		if ((data & TCTL_CNTE) && (csl != 3))
		{
			//printf("CNTE set, starting timer at %d Hz\n", m_timer_hz[timer]);
			m_timer[timer]->adjust(attotime::from_hz(m_timer_hz[timer]));
		}

		if ((data & TCTL_CNTE) && (csl == 3))
		{
			m_event_count[timer] = m_timer_regs[2] | (m_timer_regs[3]<<8);
			//printf("CNTE set in event counter mode, reseeding count to %04x\n", m_event_count[timer]);
		}

		if (!(data & TCTL_UF))
		{
			intc_clear_irq(9, 0x1d + timer);
		}
	}
}

void mb9061x_device::tin0_w(int state)
{
	tin_common(0, 0, state);
}

void mb9061x_device::tin1_w(int state)
{
	tin_common(1, 4, state);
}

void mb9061x_device::tin_common(int timer, int base, int state)
{
	// emsure event counter mode
	if ((m_timer_regs[base + 1] & (TCTH_CSL0|TCTH_CSL1)) == (TCTH_CSL0|TCTH_CSL1) &&
		(m_timer_regs[base] & TCTL_CNTE))
	{
		bool bTickIt = false;

		// rising edge
		if ((state && !m_event_state[base/4]) && (m_timer_regs[base] & TCTL_MOD0) && !(m_timer_regs[base+1] & TCTH_MOD1))
		{
			bTickIt = true;
		}

		// falling edge
		if ((!state && m_event_state[base/4]) && !(m_timer_regs[base] & TCTL_MOD0) && (m_timer_regs[base+1] & TCTH_MOD1))
		{
			bTickIt = true;
		}

		// any edge
		if ((state != m_event_state[base/4]) && !(m_timer_regs[base] & TCTL_MOD0) && (m_timer_regs[base+1] & TCTH_MOD1))
		{
			bTickIt = true;
		}

		if (bTickIt)
		{
			m_event_count[timer]--;
			//printf("Tick timer %d, new value %04x\n", timer, m_event_count[timer]);

			if (m_event_count[timer] == 0xffff)
			{
				u8 ctl = m_timer_regs[base];
				m_timer_regs[base] |= TCTL_UF;
				//printf("Timer %d exp, CL %02x\n", timer, m_timer_regs[base]);
				if (ctl & TCTL_INTE)
				{
					//printf("timer %d IRQ\n", timer);
					intc_trigger_irq(9, 0x1d + timer);
				}

				if (m_timer_regs[timer * 4] & TCTL_RELD)
				{
					m_event_count[timer] = m_timer_regs[2] | (m_timer_regs[3]<<8);
					//printf("timer %d reload to %04x\n", timer, m_event_count[timer]);
				}
			}
		}

		m_event_state[timer] = state;
	}
}

void mb9061x_device::recalc_timer(int tnum)
{
	int rbase = tnum << 2;
	u32 divider = 1;
	u8 csl = (m_timer_regs[rbase+1] >> 2) & 3;

	// check clock select
	switch (csl)
	{
		case 0: divider = 2; break;
		case 1: divider = 16; break;
		case 2: divider = 64; break;

		case 3: // event counter mode
			return;
	}

	u32 tclk = clock() / divider;
	u32 tval = m_timer_regs[rbase+2] | (m_timer_regs[rbase+3]<<8);
	//printf("CSL %d, tclk %d, tval %d\n", csl, tclk, tval);
	//printf("Timer is %d Hz\n", tclk / tval);
	m_timer_hz[tnum] = tclk / tval;
}

/* TBTC: timebase counter */
enum
{
	TBTC_TEST = 0x80,   // test, must always write 1
	TBTC_TBIE = 0x10,   // interrupt enable
	TBTC_TBOF = 0x08,   // expire flag
	TBTC_TBR  = 0x04,   // write 0 to restart
	TBTC_TBC1 = 0x02,   // rate select bit 1
	TBTC_TBC0 = 0x01    // rate select bit 0
};

u8 mb9061x_device::tbtc_r()
{
	return m_tbtc;
}

void mb9061x_device::tbtc_w(u8 data)
{
	static const float periods[4] = { 1.024f, 4.096f, 16.384f, 131.072f };

	//printf("%02x to TBTC\n", data);
//  if ((!(data & TBTC_TBR)) || ((data & (TBTC_TBC1|TBTC_TBC0)) != (m_tbtc & (TBTC_TBC1|TBTC_TBC0))))
	{
		m_tbtc_timer->adjust(attotime(0, ATTOSECONDS_IN_MSEC(periods[data & (TBTC_TBC1|TBTC_TBC0)])));

		if (!(data & TBTC_TBR))
		{
			intc_clear_irq(11, 0x22);
		}
	}

	if (!(data & TBTC_TBOF) && (m_tbtc & TBTC_TBOF))
	{
		intc_clear_irq(11, 0x22);
	}

	m_tbtc = data;
}

/* INTC: Interrupt controller */
TIMER_CALLBACK_MEMBER(mb9061x_device::tbtc_tick)
{
	//printf("TBTC tick\n");
	m_tbtc_timer->adjust(attotime::never);
	m_tbtc |= TBTC_TBOF;
	if (m_tbtc & TBTC_TBIE)
	{
		intc_trigger_irq(11, 0x22);
	}
}

u8 mb9061x_device::intc_r(offs_t offset)
{
	return m_intc[offset];
}

void mb9061x_device::intc_w(offs_t offset, u8 data)
{
	//printf("INTC ICR %d to %02x\n", offset, data);
	m_intc[offset] = data;
}

void mb9061x_device::intc_trigger_irq(int icr, int vector)
{
	int level = m_intc[icr] & 7;

	//printf("trigger_irq: ICR %d, level %d\n", icr, level);

	// Extended Intelligent I/O Service?
	if (m_intc[icr] & 8)
	{
		fatalerror("MB9061X: ICR %d (vector %x) calls for EI2OS, not implemented\n", icr, vector);
	}
	else
	{
		if (level != 7)
		{
			set_irq(vector, level);
		}
	}
}

void mb9061x_device::intc_clear_irq(int icr, int vector)
{
	int level = m_intc[icr] & 7;

	// Make sure this isn't the Extended Intelligent I/O Service
	if (!(m_intc[icr] & 8))
	{
		if (level != 7)
		{
			clear_irq(vector);
		}
	}
}

/* MB90611 - Production version of this series */
void mb90611_device::mb90611_map(address_map &map)
{
	map(0x0001, 0x000a).rw(FUNC(mb9061x_device::port_r), FUNC(mb9061x_device::port_w));
	map(0x002c, 0x002d).rw(FUNC(mb9061x_device::adcs_r), FUNC(mb9061x_device::adcs_w));
	map(0x002e, 0x002f).rw(FUNC(mb9061x_device::adcr_r), FUNC(mb9061x_device::adcr_w));
	map(0x0038, 0x003f).rw(FUNC(mb9061x_device::timer_r), FUNC(mb9061x_device::timer_w));
	map(0x00a9, 0x00a9).rw(FUNC(mb9061x_device::tbtc_r), FUNC(mb9061x_device::tbtc_w));
	map(0x00b0, 0x00bf).rw(FUNC(mb9061x_device::intc_r), FUNC(mb9061x_device::intc_w));
	map(0x0100, 0x04ff).ram();  // 1K of internal RAM from 0x100 to 0x500
}

mb90611_device::mb90611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mb90611_device(mconfig, MB90611A, tag, owner, clock)
{
}

mb90611_device::mb90611_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mb9061x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(mb90611_device::mb90611_map), this))
{
}

/* MB90641 - no A/D, extra UART and timers, optional internal ROM */
void mb90641_device::mb90641_map(address_map &map)
{
	map(0x0001, 0x000a).rw(FUNC(mb9061x_device::port_r), FUNC(mb9061x_device::port_w));
	map(0x002c, 0x002d).rw(FUNC(mb9061x_device::adcs_r), FUNC(mb9061x_device::adcs_w));
	map(0x002e, 0x002f).rw(FUNC(mb9061x_device::adcr_r), FUNC(mb9061x_device::adcr_w));
	map(0x0038, 0x003f).rw(FUNC(mb9061x_device::timer_r), FUNC(mb9061x_device::timer_w));
	map(0x00a9, 0x00a9).rw(FUNC(mb9061x_device::tbtc_r), FUNC(mb9061x_device::tbtc_w));
	map(0x00b0, 0x00bf).rw(FUNC(mb9061x_device::intc_r), FUNC(mb9061x_device::intc_w));
	map(0x0100, 0x08ff).ram();  // 2K of internal RAM
}

mb90641_device::mb90641_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mb90641_device(mconfig, MB90641A, tag, owner, clock)
{
}

mb90641_device::mb90641_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mb9061x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(mb90641_device::mb90641_map), this))
{
}
