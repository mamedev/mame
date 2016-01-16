// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Motorola 68328 ("DragonBall") System-on-a-Chip implementation

    By Ryan Holtz

**********************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68328.h"
#include "machine/ram.h"

#define VERBOSE_LEVEL   (0)

static inline void ATTR_PRINTF(3,4) verboselog(device_t &device, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		device.logerror("%s: %s", device.machine().describe_context(), buf);
	}
}

const device_type MC68328 = &device_creator<mc68328_device>;


mc68328_device::mc68328_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, MC68328, "MC68328 (DragonBall) Integrated Processor", tag, owner, clock, "mc68328", __FILE__), m_rtc(nullptr), m_pwm(nullptr),
				m_out_port_a_cb(*this),
				m_out_port_b_cb(*this),
				m_out_port_c_cb(*this),
				m_out_port_d_cb(*this),
				m_out_port_e_cb(*this),
				m_out_port_f_cb(*this),
				m_out_port_g_cb(*this),
				m_out_port_j_cb(*this),
				m_out_port_k_cb(*this),
				m_out_port_m_cb(*this),
				m_in_port_a_cb(*this),
				m_in_port_b_cb(*this),
				m_in_port_c_cb(*this),
				m_in_port_d_cb(*this),
				m_in_port_e_cb(*this),
				m_in_port_f_cb(*this),
				m_in_port_g_cb(*this),
				m_in_port_j_cb(*this),
				m_in_port_k_cb(*this),
				m_in_port_m_cb(*this),
				m_out_pwm_cb(*this),
				m_out_spim_cb(*this),
				m_in_spim_cb(*this),
				m_spim_xch_trigger_cb(*this),
				m_cpu(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc68328_device::device_start()
{
	m_out_port_a_cb.resolve();
	m_out_port_b_cb.resolve();
	m_out_port_c_cb.resolve();
	m_out_port_d_cb.resolve();
	m_out_port_e_cb.resolve();
	m_out_port_f_cb.resolve();
	m_out_port_g_cb.resolve();
	m_out_port_j_cb.resolve();
	m_out_port_k_cb.resolve();
	m_out_port_m_cb.resolve();

	m_in_port_a_cb.resolve();
	m_in_port_b_cb.resolve();
	m_in_port_c_cb.resolve();
	m_in_port_d_cb.resolve();
	m_in_port_e_cb.resolve();
	m_in_port_f_cb.resolve();
	m_in_port_g_cb.resolve();
	m_in_port_j_cb.resolve();
	m_in_port_k_cb.resolve();
	m_in_port_m_cb.resolve();

	m_out_pwm_cb.resolve();

	m_out_spim_cb.resolve();
	m_in_spim_cb.resolve();

	m_spim_xch_trigger_cb.resolve();

	m_gptimer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc68328_device::timer1_hit),this));
	m_gptimer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc68328_device::timer2_hit),this));
	m_rtc = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc68328_device::rtc_tick),this));
	m_pwm = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc68328_device::pwm_transition),this));

	register_state_save();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc68328_device::device_reset()
{
	m_regs.scr = 0x0c;
	m_regs.grpbasea = 0x0000;
	m_regs.grpbaseb = 0x0000;
	m_regs.grpbasec = 0x0000;
	m_regs.grpbased = 0x0000;
	m_regs.grpmaska = 0x0000;
	m_regs.grpmaskb = 0x0000;
	m_regs.grpmaskc = 0x0000;
	m_regs.grpmaskd = 0x0000;
	m_regs.csa0 = 0x00010006;
	m_regs.csa1 = 0x00010006;
	m_regs.csa2 = 0x00010006;
	m_regs.csa3 = 0x00010006;
	m_regs.csb0 = 0x00010006;
	m_regs.csb1 = 0x00010006;
	m_regs.csb2 = 0x00010006;
	m_regs.csb3 = 0x00010006;
	m_regs.csc0 = 0x00010006;
	m_regs.csc1 = 0x00010006;
	m_regs.csc2 = 0x00010006;
	m_regs.csc3 = 0x00010006;
	m_regs.csd0 = 0x00010006;
	m_regs.csd1 = 0x00010006;
	m_regs.csd2 = 0x00010006;
	m_regs.csd3 = 0x00010006;

	m_regs.pllcr = 0x2400;
	m_regs.pllfsr = 0x0123;
	m_regs.pctlr = 0x1f;

	m_regs.ivr = 0x00;
	m_regs.icr = 0x0000;
	m_regs.imr = 0x00ffffff;
	m_regs.iwr = 0x00ffffff;
	m_regs.isr = 0x00000000;
	m_regs.ipr = 0x00000000;

	m_regs.padir = 0x00;
	m_regs.padata = 0x00;
	m_regs.pasel = 0x00;
	m_regs.pbdir = 0x00;
	m_regs.pbdata = 0x00;
	m_regs.pbsel = 0x00;
	m_regs.pcdir = 0x00;
	m_regs.pcdata = 0x00;
	m_regs.pcsel = 0x00;
	m_regs.pddir = 0x00;
	m_regs.pddata = 0x00;
	m_regs.pdpuen = 0xff;
	m_regs.pdpol = 0x00;
	m_regs.pdirqen = 0x00;
	m_regs.pddataedge = 0x00;
	m_regs.pdirqedge = 0x00;
	m_regs.pedir = 0x00;
	m_regs.pedata = 0x00;
	m_regs.pepuen = 0x80;
	m_regs.pesel = 0x80;
	m_regs.pfdir = 0x00;
	m_regs.pfdata = 0x00;
	m_regs.pfpuen = 0xff;
	m_regs.pfsel = 0xff;
	m_regs.pgdir = 0x00;
	m_regs.pgdata = 0x00;
	m_regs.pgpuen = 0xff;
	m_regs.pgsel = 0xff;
	m_regs.pjdir = 0x00;
	m_regs.pjdata = 0x00;
	m_regs.pjsel = 0x00;
	m_regs.pkdir = 0x00;
	m_regs.pkdata = 0x00;
	m_regs.pkpuen = 0xff;
	m_regs.pksel = 0xff;
	m_regs.pmdir = 0x00;
	m_regs.pmdata = 0x00;
	m_regs.pmpuen = 0xff;
	m_regs.pmsel = 0xff;

	m_regs.pwmc = 0x0000;
	m_regs.pwmp = 0x0000;
	m_regs.pwmw = 0x0000;
	m_regs.pwmcnt = 0x0000;

	m_regs.tctl[0] = m_regs.tctl[1] = 0x0000;
	m_regs.tprer[0] = m_regs.tprer[1] = 0x0000;
	m_regs.tcmp[0] = m_regs.tcmp[1] = 0xffff;
	m_regs.tcr[0] = m_regs.tcr[1] = 0x0000;
	m_regs.tcn[0] = m_regs.tcn[1] = 0x0000;
	m_regs.tstat[0] = m_regs.tstat[1] = 0x0000;
	m_regs.wctlr = 0x0000;
	m_regs.wcmpr = 0xffff;
	m_regs.wcn = 0x0000;

	m_regs.spisr = 0x0000;

	m_regs.spimdata = 0x0000;
	m_regs.spimcont = 0x0000;

	m_regs.ustcnt = 0x0000;
	m_regs.ubaud = 0x003f;
	m_regs.urx = 0x0000;
	m_regs.utx = 0x0000;
	m_regs.umisc = 0x0000;

	m_regs.lssa = 0x00000000;
	m_regs.lvpw = 0xff;
	m_regs.lxmax = 0x03ff;
	m_regs.lymax = 0x01ff;
	m_regs.lcxp = 0x0000;
	m_regs.lcyp = 0x0000;
	m_regs.lcwch = 0x0101;
	m_regs.lblkc = 0x7f;
	m_regs.lpicf = 0x00;
	m_regs.lpolcf = 0x00;
	m_regs.lacdrc = 0x00;
	m_regs.lpxcd = 0x00;
	m_regs.lckcon = 0x40;
	m_regs.llbar = 0x3e;
	m_regs.lotcr = 0x3f;
	m_regs.lposr = 0x00;
	m_regs.lfrcm = 0xb9;
	m_regs.lgpmr = 0x1073;

	m_regs.hmsr = 0x00000000;
	m_regs.alarm = 0x00000000;
	m_regs.rtcctl = 0x00;
	m_regs.rtcisr = 0x00;
	m_regs.rtcienr = 0x00;
	m_regs.stpwtch = 0x00;

	m_rtc->adjust(attotime::from_hz(1), 0, attotime::from_hz(1));
}


void mc68328_device::set_interrupt_line(UINT32 line, UINT32 active)
{
	if (active)
	{
		m_regs.ipr |= line;

		if (!(m_regs.imr & line) && !(m_regs.isr & line))
		{
			m_regs.isr |= line;

			if (m_regs.isr & INT_M68K_LINE7)
			{
				m_cpu->set_input_line_and_vector(M68K_IRQ_7, ASSERT_LINE, m_regs.ivr | 0x07);
			}
			else if (m_regs.isr & INT_M68K_LINE6)
			{
				m_cpu->set_input_line_and_vector(M68K_IRQ_6, ASSERT_LINE, m_regs.ivr | 0x06);
			}
			else if (m_regs.isr & INT_M68K_LINE5)
			{
				m_cpu->set_input_line_and_vector(M68K_IRQ_5, ASSERT_LINE, m_regs.ivr | 0x05);
			}
			else if (m_regs.isr & INT_M68K_LINE4)
			{
				m_cpu->set_input_line_and_vector(M68K_IRQ_4, ASSERT_LINE, m_regs.ivr | 0x04);
			}
			else if (m_regs.isr & INT_M68K_LINE3)
			{
				m_cpu->set_input_line_and_vector(M68K_IRQ_3, ASSERT_LINE, m_regs.ivr | 0x03);
			}
			else if (m_regs.isr & INT_M68K_LINE2)
			{
				m_cpu->set_input_line_and_vector(M68K_IRQ_2, ASSERT_LINE, m_regs.ivr | 0x02);
			}
			else if (m_regs.isr & INT_M68K_LINE1)
			{
				m_cpu->set_input_line_and_vector(M68K_IRQ_1, ASSERT_LINE, m_regs.ivr | 0x01);
			}
		}
	}
	else
	{
		m_regs.isr &= ~line;

		if ((line & INT_M68K_LINE7) && !(m_regs.isr & INT_M68K_LINE7))
		{
			m_cpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE6) && !(m_regs.isr & INT_M68K_LINE6))
		{
			m_cpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE5) && !(m_regs.isr & INT_M68K_LINE5))
		{
			m_cpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE4) && !(m_regs.isr & INT_M68K_LINE4))
		{
			m_cpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE3) && !(m_regs.isr & INT_M68K_LINE3))
		{
			m_cpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE2) && !(m_regs.isr & INT_M68K_LINE2))
		{
			m_cpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE1) && !(m_regs.isr & INT_M68K_LINE1))
		{
			m_cpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
		}
	}
}

void mc68328_device::poll_port_d_interrupts()
{
	UINT8 line_transitions = m_regs.pddataedge & m_regs.pdirqedge;
	UINT8 line_holds = m_regs.pddata &~ m_regs.pdirqedge;
	UINT8 line_interrupts = (line_transitions | line_holds) & m_regs.pdirqen;

	if (line_interrupts)
	{
		set_interrupt_line(line_interrupts << 8, 1);
	}
	else
	{
		set_interrupt_line(INT_KBDINTS, 0);
	}
}

WRITE_LINE_MEMBER( mc68328_device::set_penirq_line )
{
	if (state)
	{
		set_interrupt_line(INT_PEN, 1);
	}
	else
	{
		m_regs.ipr &= ~INT_PEN;
		set_interrupt_line(INT_PEN, 0);
	}
}

void mc68328_device::set_port_d_lines(UINT8 state, int bit)
{
	UINT8 old_button_state = m_regs.pddata;

	if (state & (1 << bit))
	{
		m_regs.pddata |= (1 << bit);
	}
	else
	{
		m_regs.pddata &= ~(1 << bit);
	}

	m_regs.pddataedge |= ~old_button_state & m_regs.pddata;

	poll_port_d_interrupts();
}

UINT32 mc68328_device::get_timer_frequency(UINT32 index)
{
	UINT32 frequency = 0;

	switch (m_regs.tctl[index] & TCTL_CLKSOURCE)
	{
		case TCTL_CLKSOURCE_SYSCLK:
			frequency = 32768 * 506;
			break;

		case TCTL_CLKSOURCE_SYSCLK16:
			frequency = (32768 * 506) / 16;
			break;

		case TCTL_CLKSOURCE_32KHZ4:
		case TCTL_CLKSOURCE_32KHZ5:
		case TCTL_CLKSOURCE_32KHZ6:
		case TCTL_CLKSOURCE_32KHZ7:
			frequency = 32768;
			break;
	}
	frequency /= (m_regs.tprer[index] + 1);

	return frequency;
}

void mc68328_device::maybe_start_timer(UINT32 index, UINT32 new_enable)
{
	if ((m_regs.tctl[index] & TCTL_TEN) == TCTL_TEN_ENABLE && (m_regs.tctl[index] & TCTL_CLKSOURCE) > TCTL_CLKSOURCE_STOP)
	{
		if ((m_regs.tctl[index] & TCTL_CLKSOURCE) == TCTL_CLKSOURCE_TIN)
		{
			m_gptimer[index]->adjust(attotime::never);
		}
		else if (m_regs.tcmp[index] == 0)
		{
			m_gptimer[index]->adjust(attotime::never);
		}
		else
		{
			UINT32 frequency = get_timer_frequency(index);
			attotime period = (attotime::from_hz(frequency) *  m_regs.tcmp[index]);

			if (new_enable)
			{
				m_regs.tcn[index] = 0x0000;
			}

			m_gptimer[index]->adjust(period);
		}
	}
	else
	{
		m_gptimer[index]->adjust(attotime::never);
	}
}

void mc68328_device::timer_compare_event(UINT32 index)
{
	m_regs.tcn[index] = m_regs.tcmp[index];
	m_regs.tstat[index] |= TSTAT_COMP;

	if ((m_regs.tctl[index] & TCTL_FRR) == TCTL_FRR_RESTART)
	{
		UINT32 frequency = get_timer_frequency(index);

		if (frequency > 0)
		{
			attotime period = attotime::from_hz(frequency) * m_regs.tcmp[index];

			m_regs.tcn[index] = 0x0000;

			m_gptimer[index]->adjust(period);
		}
		else
		{
			m_gptimer[index]->adjust(attotime::never);
		}
	}
	else
	{
		UINT32 frequency = get_timer_frequency(index);

		if (frequency > 0)
		{
			attotime period = attotime::from_hz(frequency) * 0x10000;

			m_gptimer[index]->adjust(period);
		}
		else
		{
			m_gptimer[index]->adjust(attotime::never);
		}
	}
	if ((m_regs.tctl[index] & TCTL_IRQEN) == TCTL_IRQEN_ENABLE)
	{
		set_interrupt_line((index == 0) ? INT_TIMER1 : INT_TIMER2, 1);
	}
}

TIMER_CALLBACK_MEMBER( mc68328_device::timer1_hit )
{
	timer_compare_event(0);
}

TIMER_CALLBACK_MEMBER( mc68328_device::timer2_hit )
{
	timer_compare_event(1);
}

TIMER_CALLBACK_MEMBER( mc68328_device::pwm_transition )
{
	if (m_regs.pwmw >= m_regs.pwmp || m_regs.pwmw == 0 || m_regs.pwmp == 0)
	{
		m_pwm->adjust(attotime::never);
		return;
	}

	if (((m_regs.pwmc & PWMC_POL) == 0 && (m_regs.pwmc & PWMC_PIN) != 0) ||
		((m_regs.pwmc & PWMC_POL) != 0 && (m_regs.pwmc & PWMC_PIN) == 0))
	{
		UINT32 frequency = 32768 * 506;
		UINT32 divisor = 4 << (m_regs.pwmc & PWMC_CLKSEL); // ?? Datasheet says 2 <<, but then we're an octave higher than CoPilot.
		attotime period;

		frequency /= divisor;
		period = attotime::from_hz(frequency) * (m_regs.pwmp - m_regs.pwmw);

		m_pwm->adjust(period);

		if (m_regs.pwmc & PWMC_IRQEN)
		{
			set_interrupt_line(INT_PWM, 1);
		}
	}
	else
	{
		UINT32 frequency = 32768 * 506;
		UINT32 divisor = 4 << (m_regs.pwmc & PWMC_CLKSEL); // ?? Datasheet says 2 <<, but then we're an octave higher than CoPilot.
		attotime period;

		frequency /= divisor;
		period = attotime::from_hz(frequency) * m_regs.pwmw;

		m_pwm->adjust(period);
	}

	m_regs.pwmc ^= PWMC_PIN;

	if (!m_out_pwm_cb.isnull())
	{
		m_out_pwm_cb((offs_t)0, (m_regs.pwmc & PWMC_PIN) ? 1 : 0);
	}
}

TIMER_CALLBACK_MEMBER( mc68328_device::rtc_tick )
{
	if (m_regs.rtcctl & RTCCTL_ENABLE)
	{
		UINT32 set_int = 0;

		m_regs.hmsr++;

		if (m_regs.rtcienr & RTCINT_SECOND)
		{
			set_int = 1;
			m_regs.rtcisr |= RTCINT_SECOND;
		}

		if ((m_regs.hmsr & 0x0000003f) == 0x0000003c)
		{
			m_regs.hmsr &= 0xffffffc0;
			m_regs.hmsr += 0x00010000;

			if (m_regs.rtcienr & RTCINT_MINUTE)
			{
				set_int = 1;
				m_regs.rtcisr |= RTCINT_MINUTE;
			}

			if ((m_regs.hmsr & 0x003f0000) == 0x003c0000)
			{
				m_regs.hmsr &= 0xffc0ffff;
				m_regs.hmsr += 0x0100000;

				if ((m_regs.hmsr & 0x1f000000) == 0x18000000)
				{
					m_regs.hmsr &= 0xe0ffffff;

					if (m_regs.rtcienr & RTCINT_DAY)
					{
						set_int = 1;
						m_regs.rtcisr |= RTCINT_DAY;
					}
				}
			}

			if (m_regs.stpwtch != 0x003f)
			{
				m_regs.stpwtch--;
				m_regs.stpwtch &= 0x003f;

				if (m_regs.stpwtch == 0x003f)
				{
					if (m_regs.rtcienr & RTCINT_STOPWATCH)
					{
						set_int = 1;
						m_regs.rtcisr |= RTCINT_STOPWATCH;
					}
				}
			}
		}

		if (m_regs.hmsr == m_regs.alarm)
		{
			if (m_regs.rtcienr & RTCINT_ALARM)
			{
				set_int = 1;
				m_regs.rtcisr |= RTCINT_STOPWATCH;
			}
		}

		if (set_int)
		{
			set_interrupt_line(INT_RTC, 1);
		}
		else
		{
			set_interrupt_line(INT_RTC, 0);
		}
	}
}

WRITE16_MEMBER( mc68328_device::write )
{
	UINT32 address = offset << 1;
	UINT16 temp16[4] = { 0 };
	UINT32 imr_old = m_regs.imr, imr_diff;

	switch (address)
	{
		case 0x000:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfff001) = %02x\n", data & 0x00ff);
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: SCR = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0x100:
			verboselog( *this, 2, "mc68328_w: GRPBASEA = %04x\n", data);
			m_regs.grpbasea = data;
			break;

		case 0x102:
			verboselog( *this, 2, "mc68328_w: GRPBASEB = %04x\n", data);
			m_regs.grpbaseb = data;
			break;

		case 0x104:
			verboselog( *this, 2, "mc68328_w: GRPBASEC = %04x\n", data);
			m_regs.grpbasec = data;
			break;

		case 0x106:
			verboselog( *this, 2, "mc68328_w: GRPBASED = %04x\n", data);
			m_regs.grpbased = data;
			break;

		case 0x108:
			verboselog( *this, 2, "mc68328_w: GRPMASKA = %04x\n", data);
			m_regs.grpmaska = data;
			break;

		case 0x10a:
			verboselog( *this, 2, "mc68328_w: GRPMASKB = %04x\n", data);
			m_regs.grpmaskb = data;
			break;

		case 0x10c:
			verboselog( *this, 2, "mc68328_w: GRPMASKC = %04x\n", data);
			m_regs.grpmaskc = data;
			break;

		case 0x10e:
			verboselog( *this, 2, "mc68328_w: GRPMASKD = %04x\n", data);
			m_regs.grpmaskd = data;
			break;

		case 0x110:
			verboselog( *this, 5, "mc68328_w: CSA0(0) = %04x\n", data);
			m_regs.csa0 &= 0xffff0000 | (~mem_mask);
			m_regs.csa0 |= data & mem_mask;
			break;

		case 0x112:
			verboselog( *this, 5, "mc68328_w: CSA0(16) = %04x\n", data);
			m_regs.csa0 &= ~(mem_mask << 16);
			m_regs.csa0 |= (data & mem_mask) << 16;
			break;

		case 0x114:
			verboselog( *this, 5, "mc68328_w: CSA1(0) = %04x\n", data);
			m_regs.csa1 &= 0xffff0000 | (~mem_mask);
			m_regs.csa1 |= data & mem_mask;
			break;

		case 0x116:
			verboselog( *this, 5, "mc68328_w: CSA1(16) = %04x\n", data);
			m_regs.csa1 &= ~(mem_mask << 16);
			m_regs.csa1 |= (data & mem_mask) << 16;
			break;

		case 0x118:
			verboselog( *this, 5, "mc68328_w: CSA2(0) = %04x\n", data);
			m_regs.csa2 &= 0xffff0000 | (~mem_mask);
			m_regs.csa2 |= data & mem_mask;
			break;

		case 0x11a:
			verboselog( *this, 5, "mc68328_w: CSA2(16) = %04x\n", data);
			m_regs.csa2 &= ~(mem_mask << 16);
			m_regs.csa2 |= (data & mem_mask) << 16;
			break;

		case 0x11c:
			verboselog( *this, 5, "mc68328_w: CSA3(0) = %04x\n", data);
			m_regs.csa3 &= 0xffff0000 | (~mem_mask);
			m_regs.csa3 |= data & mem_mask;
			break;

		case 0x11e:
			verboselog( *this, 5, "mc68328_w: CSA3(16) = %04x\n", data);
			m_regs.csa3 &= ~(mem_mask << 16);
			m_regs.csa3 |= (data & mem_mask) << 16;
			break;

		case 0x120:
			verboselog( *this, 5, "mc68328_w: CSB0(0) = %04x\n", data);
			m_regs.csb0 &= 0xffff0000 | (~mem_mask);
			m_regs.csb0 |= data & mem_mask;
			break;

		case 0x122:
			verboselog( *this, 5, "mc68328_w: CSB0(16) = %04x\n", data);
			m_regs.csb0 &= ~(mem_mask << 16);
			m_regs.csb0 |= (data & mem_mask) << 16;
			break;

		case 0x124:
			verboselog( *this, 5, "mc68328_w: CSB1(0) = %04x\n", data);
			m_regs.csb1 &= 0xffff0000 | (~mem_mask);
			m_regs.csb1 |= data & mem_mask;
			break;

		case 0x126:
			verboselog( *this, 5, "mc68328_w: CSB1(16) = %04x\n", data);
			m_regs.csb1 &= ~(mem_mask << 16);
			m_regs.csb1 |= (data & mem_mask) << 16;
			break;

		case 0x128:
			verboselog( *this, 5, "mc68328_w: CSB2(0) = %04x\n", data);
			m_regs.csb2 &= 0xffff0000 | (~mem_mask);
			m_regs.csb2 |= data & mem_mask;
			break;

		case 0x12a:
			verboselog( *this, 5, "mc68328_w: CSB2(16) = %04x\n", data);
			m_regs.csb2 &= ~(mem_mask << 16);
			m_regs.csb2 |= (data & mem_mask) << 16;
			break;

		case 0x12c:
			verboselog( *this, 5, "mc68328_w: CSB3(0) = %04x\n", data);
			m_regs.csb3 &= 0xffff0000 | (~mem_mask);
			m_regs.csb3 |= data & mem_mask;
			break;

		case 0x12e:
			verboselog( *this, 5, "mc68328_w: CSB3(16) = %04x\n", data);
			m_regs.csb3 &= ~(mem_mask << 16);
			m_regs.csb3 |= (data & mem_mask) << 16;
			break;

		case 0x130:
			verboselog( *this, 5, "mc68328_w: CSC0(0) = %04x\n", data);
			m_regs.csc0 &= 0xffff0000 | (~mem_mask);
			m_regs.csc0 |= data & mem_mask;
			break;

		case 0x132:
			verboselog( *this, 5, "mc68328_w: CSC0(16) = %04x\n", data);
			m_regs.csc0 &= ~(mem_mask << 16);
			m_regs.csc0 |= (data & mem_mask) << 16;
			break;

		case 0x134:
			verboselog( *this, 5, "mc68328_w: CSC1(0) = %04x\n", data);
			m_regs.csc1 &= 0xffff0000 | (~mem_mask);
			m_regs.csc1 |= data & mem_mask;
			break;

		case 0x136:
			verboselog( *this, 5, "mc68328_w: CSC1(16) = %04x\n", data);
			m_regs.csc1 &= ~(mem_mask << 16);
			m_regs.csc1 |= (data & mem_mask) << 16;
			break;

		case 0x138:
			verboselog( *this, 5, "mc68328_w: CSC2(0) = %04x\n", data);
			m_regs.csc2 &= 0xffff0000 | (~mem_mask);
			m_regs.csc2 |= data & mem_mask;
			break;

		case 0x13a:
			verboselog( *this, 5, "mc68328_w: CSC2(16) = %04x\n", data);
			m_regs.csc2 &= ~(mem_mask << 16);
			m_regs.csc2 |= (data & mem_mask) << 16;
			break;

		case 0x13c:
			verboselog( *this, 5, "mc68328_w: CSC3(0) = %04x\n", data);
			m_regs.csc3 &= 0xffff0000 | (~mem_mask);
			m_regs.csc3 |= data & mem_mask;
			break;

		case 0x13e:
			verboselog( *this, 5, "mc68328_w: CSC3(16) = %04x\n", data);
			m_regs.csc3 &= ~(mem_mask << 16);
			m_regs.csc3 |= (data & mem_mask) << 16;
			break;

		case 0x140:
			verboselog( *this, 5, "mc68328_w: CSD0(0) = %04x\n", data);
			m_regs.csd0 &= 0xffff0000 | (~mem_mask);
			m_regs.csd0 |= data & mem_mask;
			break;

		case 0x142:
			verboselog( *this, 5, "mc68328_w: CSD0(16) = %04x\n", data);
			m_regs.csd0 &= ~(mem_mask << 16);
			m_regs.csd0 |= (data & mem_mask) << 16;
			break;

		case 0x144:
			verboselog( *this, 5, "mc68328_w: CSD1(0) = %04x\n", data);
			m_regs.csd1 &= 0xffff0000 | (~mem_mask);
			m_regs.csd1 |= data & mem_mask;
			break;

		case 0x146:
			verboselog( *this, 5, "mc68328_w: CSD1(16) = %04x\n", data);
			m_regs.csd1 &= ~(mem_mask << 16);
			m_regs.csd1 |= (data & mem_mask) << 16;
			break;

		case 0x148:
			verboselog( *this, 5, "mc68328_w: CSD2(0) = %04x\n", data);
			m_regs.csd2 &= 0xffff0000 | (~mem_mask);
			m_regs.csd2 |= data & mem_mask;
			break;

		case 0x14a:
			verboselog( *this, 5, "mc68328_w: CSD2(16) = %04x\n", data);
			m_regs.csd2 &= ~(mem_mask << 16);
			m_regs.csd2 |= (data & mem_mask) << 16;
			break;

		case 0x14c:
			verboselog( *this, 5, "mc68328_w: CSD3(0) = %04x\n", data);
			m_regs.csd3 &= 0xffff0000 | (~mem_mask);
			m_regs.csd3 |= data & mem_mask;
			break;

		case 0x14e:
			verboselog( *this, 5, "mc68328_w: CSD3(16) = %04x\n", data);
			m_regs.csd3 &= ~(mem_mask << 16);
			m_regs.csd3 |= (data & mem_mask) << 16;
			break;

		case 0x200:
			verboselog( *this, 2, "mc68328_w: PLLCR = %04x\n", data);
			m_regs.pllcr = data;
			break;

		case 0x202:
			verboselog( *this, 2, "mc68328_w: PLLFSR = %04x\n", data);
			m_regs.pllfsr = data;
			break;

		case 0x206:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PCTLR = %02x\n", data & 0x00ff);
				m_regs.pctlr = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfff206) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0x300:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfff301) = %02x\n", data & 0x00ff);
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: IVR = %02x\n", (data >> 8) & 0x00ff);
				m_regs.ivr = (data >> 8) & 0x00ff;
			}
			break;

		case 0x302:
			verboselog( *this, 2, "mc68328_w: ICR = %04x\n", data);
			m_regs.icr = data;
			break;

		case 0x304:
			verboselog( *this, 2, "mc68328_w: IMR(16) = %04x\n", data);
			m_regs.imr &= ~(mem_mask << 16);
			m_regs.imr |= (data & mem_mask) << 16;
			m_regs.isr &= ~((data & mem_mask) << 16);

			imr_diff = imr_old ^ m_regs.imr;
			set_interrupt_line(imr_diff, 0);
			break;

		case 0x306:
			verboselog( *this, 2, "mc68328_w: IMR(0) = %04x\n", data);
			m_regs.imr &= 0xffff0000 | (~mem_mask);
			m_regs.imr |= data & mem_mask;
			m_regs.isr &= ~(data & mem_mask);

			imr_diff = imr_old ^ m_regs.imr;
			set_interrupt_line(imr_diff, 0);
			break;

		case 0x308:
			verboselog( *this, 2, "mc68328_w: IWR(16) = %04x\n", data);
			m_regs.iwr &= ~(mem_mask << 16);
			m_regs.iwr |= (data & mem_mask) << 16;
			break;

		case 0x30a:
			verboselog( *this, 2, "mc68328_w: IWR(0) = %04x\n", data);
			m_regs.iwr &= 0xffff0000 | (~mem_mask);
			m_regs.iwr |= data & mem_mask;
			break;

		case 0x30c:
			verboselog( *this, 2, "mc68328_w: ISR(16) = %04x\n", data);
			// Clear edge-triggered IRQ1
			if ((m_regs.icr & ICR_ET1) == ICR_ET1 && (data & INT_IRQ1_SHIFT) == INT_IRQ1_SHIFT)
			{
				m_regs.isr &= ~INT_IRQ1;
			}

			// Clear edge-triggered IRQ2
			if ((m_regs.icr & ICR_ET2) == ICR_ET2 && (data & INT_IRQ2_SHIFT) == INT_IRQ2_SHIFT)
			{
				m_regs.isr &= ~INT_IRQ2;
			}

			// Clear edge-triggered IRQ3
			if ((m_regs.icr & ICR_ET3) == ICR_ET3 && (data & INT_IRQ3_SHIFT) == INT_IRQ3_SHIFT)
			{
				m_regs.isr &= ~INT_IRQ3;
			}

			// Clear edge-triggered IRQ6
			if ((m_regs.icr & ICR_ET6) == ICR_ET6 && (data & INT_IRQ6_SHIFT) == INT_IRQ6_SHIFT)
			{
				m_regs.isr &= ~INT_IRQ6;
			}

			// Clear edge-triggered IRQ7
			if ((data & INT_IRQ7_SHIFT) == INT_IRQ7_SHIFT)
			{
				m_regs.isr &= ~INT_IRQ7;
			}
			break;

		case 0x30e:
			verboselog( *this, 2, "mc68328_w: ISR(0) = %04x (Ignored)\n", data);
			break;

		case 0x310:
			verboselog( *this, 2, "mc68328_w: IPR(16) = %04x (Ignored)\n", data);
			break;

		case 0x312:
			verboselog( *this, 2, "mc68328_w: IPR(0) = %04x (Ignored)\n", data);
			break;

		case 0x400:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PADATA = %02x\n", data & 0x00ff);
				m_regs.padata = data & 0x00ff;
				if (!m_out_port_a_cb.isnull())
				{
					m_out_port_a_cb((offs_t)0, data & 0x00ff);
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PADIR = %02x\n", (data >> 8) & 0x00ff);
				m_regs.padir = (data >> 8) & 0x00ff;
			}
			break;

		case 0x402:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PASEL = %02x\n", data & 0x00ff);
				m_regs.pasel = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfff402) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0x408:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PBDATA = %02x\n", data & 0x00ff);
				m_regs.pbdata = data & 0x00ff;
				if (!m_out_port_b_cb.isnull())
				{
					m_out_port_b_cb((offs_t)0, data & 0x00ff);
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PBDIR = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pbdir = (data >> 8) & 0x00ff;
			}
			break;

		case 0x40a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PBSEL = %02x\n", data & 0x00ff);
				m_regs.pbsel = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfff40a) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0x410:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PCDATA = %02x\n", data & 0x00ff);
				m_regs.pcdata = data & 0x00ff;
				if (!m_out_port_c_cb.isnull())
				{
					m_out_port_c_cb((offs_t)0, data & 0x00ff);
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PCDIR = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pcdir = (data >> 8) & 0x00ff;
			}
			break;

		case 0x412:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PCSEL = %02x\n", data & 0x00ff);
				m_regs.pcsel = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfff412) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0x418:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PDDATA = %02x\n", data & 0x00ff);

				m_regs.pddataedge &= ~(data & 0x00ff);
				poll_port_d_interrupts();
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PDDIR = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pddir = (data >> 8) & 0x00ff;
			}
			break;

		case 0x41a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfff41b) = %02x\n", data & 0x00ff);
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PDPUEN = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pdpuen = (data >> 8) & 0x00ff;
			}
			break;

		case 0x41c:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PDIRQEN = %02x\n", data & 0x00ff);
				m_regs.pdirqen = data & 0x00ff;

				poll_port_d_interrupts();
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PDPOL = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pdpol = (data >> 8) & 0x00ff;
			}
			break;

		case 0x41e:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PDIRQEDGE = %02x\n", data & 0x00ff);
				m_regs.pdirqedge = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfff41e) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0x420:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PEDATA = %02x\n", data & 0x00ff);
				m_regs.pedata = data & 0x00ff;
				if (!m_out_port_e_cb.isnull())
				{
					m_out_port_e_cb((offs_t)0, data & 0x00ff);
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PEDIR = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pedir = (data >> 8) & 0x00ff;
			}
			break;

		case 0x422:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PESEL = %02x\n", data & 0x00ff);
				m_regs.pesel = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PEPUEN = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pepuen = (data >> 8) & 0x00ff;
				m_regs.pedata |= m_regs.pepuen;
			}
			break;

		case 0x428:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PFDATA = %02x\n", data & 0x00ff);
				m_regs.pfdata = data & 0x00ff;
				if (!m_out_port_f_cb.isnull())
				{
					m_out_port_f_cb((offs_t)0, data & 0x00ff);
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PFDIR = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pfdir = (data >> 8) & 0x00ff;
			}
			break;

		case 0x42a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PFSEL = %02x\n", data & 0x00ff);
				m_regs.pfsel = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PFPUEN = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pfpuen = (data >> 8) & 0x00ff;
			}
			break;

		case 0x430:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PGDATA = %02x\n", data & 0x00ff);
				m_regs.pgdata = data & 0x00ff;
				if (!m_out_port_g_cb.isnull())
				{
					m_out_port_g_cb((offs_t)0, data & 0x00ff);
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PGDIR = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pgdir = (data >> 8) & 0x00ff;
			}
			break;

		case 0x432:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PGSEL = %02x\n", data & 0x00ff);
				m_regs.pgsel = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PGPUEN = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pgpuen = (data >> 8) & 0x00ff;
			}
			break;

		case 0x438:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PJDATA = %02x\n", data & 0x00ff);
				m_regs.pjdata = data & 0x00ff;
				if (!m_out_port_j_cb.isnull())
				{
					m_out_port_j_cb((offs_t)0, data & 0x00ff);
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PJDIR = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pjdir = (data >> 8) & 0x00ff;
			}
			break;

		case 0x43a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PJSEL = %02x\n", data & 0x00ff);
				m_regs.pjsel = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfff43a) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0x440:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PKDATA = %02x\n", data & 0x00ff);
				m_regs.pkdata = data & 0x00ff;
				if (!m_out_port_k_cb.isnull())
				{
					m_out_port_k_cb((offs_t)0, data & 0x00ff);
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PKDIR = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pkdir = (data >> 8) & 0x00ff;
			}
			break;

		case 0x442:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PKSEL = %02x\n", data & 0x00ff);
				m_regs.pksel = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PKPUEN = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pgpuen = (data >> 8) & 0x00ff;
			}
			break;

		case 0x448:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PMDATA = %02x\n", data & 0x00ff);
				m_regs.pmdata = data & 0x00ff;
				if (!m_out_port_m_cb.isnull())
				{
					m_out_port_m_cb((offs_t)0, data & 0x00ff);
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PMDIR = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pmdir = (data >> 8) & 0x00ff;
			}
			break;

		case 0x44a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: PMSEL = %02x\n", data & 0x00ff);
				m_regs.pmsel = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: PMPUEN = %02x\n", (data >> 8) & 0x00ff);
				m_regs.pmpuen = (data >> 8) & 0x00ff;
			}
			break;

		case 0x500:
			verboselog( *this, 2, "mc68328_w: PWMC = %04x\n", data);

			m_regs.pwmc = data;

			if (m_regs.pwmc & PWMC_PWMIRQ)
			{
				set_interrupt_line(INT_PWM, 1);
			}

			m_regs.pwmc &= ~PWMC_LOAD;

			if ((m_regs.pwmc & PWMC_PWMEN) != 0 && m_regs.pwmw != 0 && m_regs.pwmp != 0)
			{
				UINT32 frequency = 32768 * 506;
				UINT32 divisor = 4 << (m_regs.pwmc & PWMC_CLKSEL); // ?? Datasheet says 2 <<, but then we're an octave higher than CoPilot.
				attotime period;
				frequency /= divisor;
				period = attotime::from_hz(frequency) * m_regs.pwmw;
				m_pwm->adjust(period);
				if (m_regs.pwmc & PWMC_IRQEN)
				{
					set_interrupt_line(INT_PWM, 1);
				}
				m_regs.pwmc ^= PWMC_PIN;
			}
			else
			{
				m_pwm->adjust(attotime::never);
			}
			break;

		case 0x502:
			verboselog( *this, 2, "mc68328_w: PWMP = %04x\n", data);
			m_regs.pwmp = data;
			break;

		case 0x504:
			verboselog( *this, 2, "mc68328_w: PWMW = %04x\n", data);
			m_regs.pwmw = data;
			break;

		case 0x506:
			verboselog( *this, 2, "mc68328_w: PWMCNT = %04x\n", data);
			m_regs.pwmcnt = 0;
			break;

		case 0x600:
			verboselog( *this, 2, "mc68328_w: TCTL1 = %04x\n", data);
			temp16[0] = m_regs.tctl[0];
			m_regs.tctl[0] = data;
			if ((temp16[0] & TCTL_TEN) == (m_regs.tctl[0] & TCTL_TEN))
			{
				maybe_start_timer(0, 0);
			}
			else if ((temp16[0] & TCTL_TEN) != TCTL_TEN_ENABLE && (m_regs.tctl[0] & TCTL_TEN) == TCTL_TEN_ENABLE)
			{
				maybe_start_timer(0, 1);
			}
			break;

		case 0x602:
			verboselog( *this, 2, "mc68328_w: TPRER1 = %04x\n", data);
			m_regs.tprer[0] = data;
			maybe_start_timer(0, 0);
			break;

		case 0x604:
			verboselog( *this, 2, "mc68328_w: TCMP1 = %04x\n", data);
			m_regs.tcmp[0] = data;
			maybe_start_timer(0, 0);
			break;

		case 0x606:
			verboselog( *this, 2, "mc68328_w: TCR1 = %04x (Ignored)\n", data);
			break;

		case 0x608:
			verboselog( *this, 2, "mc68328_w: TCN1 = %04x (Ignored)\n", data);
			break;

		case 0x60a:
			verboselog( *this, 5, "mc68328_w: TSTAT1 = %04x\n", data);
			m_regs.tstat[0] &= ~m_regs.tclear[0];
			if (!(m_regs.tstat[0] & TSTAT_COMP))
			{
				set_interrupt_line(INT_TIMER1, 0);
			}
			break;

		case 0x60c:
			verboselog( *this, 2, "mc68328_w: TCTL2 = %04x\n", data);
			temp16[0] = m_regs.tctl[1];
			m_regs.tctl[1] = data;
			if ((temp16[0] & TCTL_TEN) == (m_regs.tctl[1] & TCTL_TEN))
			{
				maybe_start_timer(1, 0);
			}
			else if ((temp16[0] & TCTL_TEN) != TCTL_TEN_ENABLE && (m_regs.tctl[1] & TCTL_TEN) == TCTL_TEN_ENABLE)
			{
				maybe_start_timer(1, 1);
			}
			break;

		case 0x60e:
			verboselog( *this, 2, "mc68328_w: TPRER2 = %04x\n", data);
			m_regs.tprer[1] = data;
			maybe_start_timer(1, 0);
			break;

		case 0x610:
			verboselog( *this, 2, "mc68328_w: TCMP2 = %04x\n", data);
			m_regs.tcmp[1] = data;
			maybe_start_timer(1, 0);
			break;

		case 0x612:
			verboselog( *this, 2, "mc68328_w: TCR2 = %04x (Ignored)\n", data);
			break;

		case 0x614:
			verboselog( *this, 2, "mc68328_w: TCN2 = %04x (Ignored)\n", data);
			break;

		case 0x616:
			verboselog( *this, 2, "mc68328_w: TSTAT2 = %04x\n", data);
			m_regs.tstat[1] &= ~m_regs.tclear[1];
			if (!(m_regs.tstat[1] & TSTAT_COMP))
			{
				set_interrupt_line(INT_TIMER2, 0);
			}
			break;

		case 0x618:
			verboselog( *this, 2, "mc68328_w: WCTLR = %04x\n", data);
			m_regs.wctlr = data;
			break;

		case 0x61a:
			verboselog( *this, 2, "mc68328_w: WCMPR = %04x\n", data);
			m_regs.wcmpr = data;
			break;

		case 0x61c:
			verboselog( *this, 2, "mc68328_w: WCN = %04x (Ignored)\n", data);
			break;

		case 0x700:
			verboselog( *this, 2, "mc68328_w: SPISR = %04x\n", data);
			m_regs.spisr = data;
			break;

		case 0x800:
			verboselog( *this, 2, "mc68328_w: SPIMDATA = %04x\n", data);
			if (!m_out_spim_cb.isnull())
			{
				m_out_spim_cb(0, data, 0xffff);
			}
			else
			{
				m_regs.spimdata = data;
			}
			break;

		case 0x802:
			verboselog( *this, 2, "mc68328_w: SPIMCONT = %04x\n", data);
			verboselog( *this, 3, "           Count = %d\n", data & SPIM_CLOCK_COUNT);
			verboselog( *this, 3, "           Polarity = %s\n", (data & SPIM_POL) ? "Inverted" : "Active-high");
			verboselog( *this, 3, "           Phase = %s\n", (data & SPIM_PHA) ? "Opposite" : "Normal");
			verboselog( *this, 3, "           IRQ Enable = %s\n", (data & SPIM_IRQEN) ? "Enable" : "Disable");
			verboselog( *this, 3, "           IRQ Pending = %s\n", (data & SPIM_SPIMIRQ) ? "Yes" : "No");
			verboselog( *this, 3, "           Exchange = %s\n", (data & SPIM_XCH) ? "Initiate" : "Idle");
			verboselog( *this, 3, "           SPIM Enable = %s\n", (data & SPIM_SPMEN) ? "Enable" : "Disable");
			verboselog( *this, 3, "           Data Rate = Divide By %d\n", 1 << ((((data & SPIM_RATE) >> 13) & 0x0007) + 2) );
			m_regs.spimcont = data;
			// $$HACK$$ We should probably emulate the ADS7843 A/D device properly.
			if (data & SPIM_XCH)
			{
				m_regs.spimcont &= ~SPIM_XCH;
				if (!m_spim_xch_trigger_cb.isnull())
				{
					m_spim_xch_trigger_cb(0);
				}
				if (data & SPIM_IRQEN)
				{
					m_regs.spimcont |= SPIM_SPIMIRQ;
					verboselog( *this, 3, "Triggering SPIM Interrupt\n" );
					set_interrupt_line(INT_SPIM, 1);
				}
			}
			if (!(data & SPIM_IRQEN))
			{
				set_interrupt_line(INT_SPIM, 0);
			}
			break;

		case 0x900:
			verboselog( *this, 2, "mc68328_w: USTCNT = %04x\n", data);
			m_regs.ustcnt = data;
			break;

		case 0x902:
			verboselog( *this, 2, "mc68328_w: UBAUD = %04x\n", data);
			m_regs.ubaud = data;
			break;

		case 0x904:
			verboselog( *this, 2, "mc68328_w: URX = %04x\n", data);
			break;

		case 0x906:
			verboselog( *this, 2, "mc68328_w: UTX = %04x\n", data);
			break;

		case 0x908:
			verboselog( *this, 2, "mc68328_w: UMISC = %04x\n", data);
			m_regs.umisc = data;
			break;

		case 0xa00:
			verboselog( *this, 2, "mc68328_w: LSSA(16) = %04x\n", data);
			m_regs.lssa &= ~(mem_mask << 16);
			m_regs.lssa |= (data & mem_mask) << 16;
			verboselog( *this, 3, "              Address: %08x\n", m_regs.lssa);
			break;

		case 0xa02:
			verboselog( *this, 2, "mc68328_w: LSSA(0) = %04x\n", data);
			m_regs.lssa &= 0xffff0000 | (~mem_mask);
			m_regs.lssa |= data & mem_mask;
			verboselog( *this, 3, "              Address: %08x\n", m_regs.lssa);
			break;

		case 0xa04:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: LVPW = %02x\n", data & 0x00ff);
				m_regs.lvpw = data & 0x00ff;
				verboselog( *this, 3, "              Page Width: %d\n", (m_regs.lvpw + 1) * ((m_regs.lpicf & 0x01) ? 8 : 16));
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfffa04) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0xa08:
			verboselog( *this, 2, "mc68328_w: LXMAX = %04x\n", data);
			m_regs.lxmax = data;
			verboselog( *this, 3, "              Width: %d\n", (data & 0x03ff) + 1);
			break;

		case 0xa0a:
			verboselog( *this, 2, "mc68328_w: LYMAX = %04x\n", data);
			m_regs.lymax = data;
			verboselog( *this, 3, "              Height: %d\n", (data & 0x03ff) + 1);
			break;

		case 0xa18:
			verboselog( *this, 2, "mc68328_w: LCXP = %04x\n", data);
			m_regs.lcxp = data;
			verboselog( *this, 3, "              X Position: %d\n", data & 0x03ff);
			switch (m_regs.lcxp >> 14)
			{
				case 0:
					verboselog( *this, 3, "              Cursor Control: Transparent\n");
					break;

				case 1:
					verboselog( *this, 3, "              Cursor Control: Black\n");
					break;

				case 2:
					verboselog( *this, 3, "              Cursor Control: Reverse\n");
					break;

				case 3:
					verboselog( *this, 3, "              Cursor Control: Invalid\n");
					break;
			}
			break;

		case 0xa1a:
			verboselog( *this, 2, "mc68328_w: LCYP = %04x\n", data);
			m_regs.lcyp = data;
			verboselog( *this, 3, "              Y Position: %d\n", data & 0x01ff);
			break;

		case 0xa1c:
			verboselog( *this, 2, "mc68328_w: LCWCH = %04x\n", data);
			m_regs.lcwch = data;
			verboselog( *this, 3, "              Width:  %d\n", (data >> 8) & 0x1f);
			verboselog( *this, 3, "              Height: %d\n", data & 0x1f);
			break;

		case 0xa1e:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: LBLKC = %02x\n", data & 0x00ff);
				m_regs.lblkc = data & 0x00ff;
				verboselog( *this, 3, "              Blink Enable:  %d\n", m_regs.lblkc >> 7);
				verboselog( *this, 3, "              Blink Divisor: %d\n", m_regs.lblkc & 0x7f);
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfffa1e) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0xa20:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: LPOLCF = %02x\n", data & 0x00ff);
				m_regs.lpolcf = data & 0x00ff;
				verboselog( *this, 3, "              LCD Shift Clock Polarity: %s\n", (m_regs.lpicf & 0x08) ? "Active positive edge of LCLK" : "Active negative edge of LCLK");
				verboselog( *this, 3, "              First-line marker polarity: %s\n", (m_regs.lpicf & 0x04) ? "Active Low" : "Active High");
				verboselog( *this, 3, "              Line-pulse polarity: %s\n", (m_regs.lpicf & 0x02) ? "Active Low" : "Active High");
				verboselog( *this, 3, "              Pixel polarity: %s\n", (m_regs.lpicf & 0x01) ? "Active Low" : "Active High");
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: LPICF = %02x\n", (data >> 8) & 0x00ff);
				m_regs.lpicf = (data >> 8) & 0x00ff;
				switch((m_regs.lpicf >> 1) & 0x03)
				{
					case 0:
						verboselog( *this, 3, "              Bus Size: 1-bit\n");
						break;

					case 1:
						verboselog( *this, 3, "              Bus Size: 2-bit\n");
						break;

					case 2:
						verboselog( *this, 3, "              Bus Size: 4-bit\n");
						break;

					case 3:
						verboselog( *this, 3, "              Bus Size: unused\n");
						break;
				}
				verboselog( *this, 3, "              Gray scale enable: %d\n", m_regs.lpicf & 0x01);
			}
			break;

		case 0xa22:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: LACDRC = %02x\n", data & 0x00ff);
				m_regs.lacdrc = data & 0x00ff;
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfffa22) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0xa24:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: LPXCD = %02x\n", data & 0x00ff);
				m_regs.lpxcd = data & 0x00ff;
				verboselog( *this, 3, "              Clock Divisor: %d\n", m_regs.lpxcd + 1);
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfffa24) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0xa26:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: LCKCON = %02x\n", data & 0x00ff);
				m_regs.lckcon = data & 0x00ff;
				verboselog( *this, 3, "              LCDC Enable: %d\n", (m_regs.lckcon >> 7) & 0x01);
				verboselog( *this, 3, "              DMA Burst Length: %d\n", ((m_regs.lckcon >> 6) & 0x01) ? 16 : 8);
				verboselog( *this, 3, "              DMA Bursting Clock Control: %d\n", ((m_regs.lckcon >> 4) & 0x03) + 1);
				verboselog( *this, 3, "              Bus Width: %d\n", ((m_regs.lckcon >> 1) & 0x01) ? 8 : 16);
				verboselog( *this, 3, "              Pixel Clock Divider Source: %s\n", (m_regs.lckcon & 0x01) ? "PIX" : "SYS");
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfffa26) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0xa28:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: LLBAR = %02x\n", data & 0x00ff);
				m_regs.llbar = data & 0x00ff;
				verboselog( *this, 3, "              Address: %d\n", (m_regs.llbar & 0x7f) * ((m_regs.lpicf & 0x01) ? 8 : 16));
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfffa28) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0xa2a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: LOTCR = %02x\n", data & 0x00ff);
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfffa2a) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0xa2c:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: LPOSR = %02x\n", data & 0x00ff);
				m_regs.lposr = data & 0x00ff;
				verboselog( *this, 3, "              Byte Offset: %d\n", (m_regs.lposr >> 3) & 0x01);
				verboselog( *this, 3, "              Pixel Offset: %d\n", m_regs.lposr & 0x07);
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfffa2c) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0xa30:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_w: LFRCM = %02x\n", data & 0x00ff);
				m_regs.lfrcm = data & 0x00ff;
				verboselog( *this, 3, "              X Modulation: %d\n", (m_regs.lfrcm >> 4) & 0x0f);
				verboselog( *this, 3, "              Y Modulation: %d\n", m_regs.lfrcm & 0x0f);
			}
			else
			{
				verboselog( *this, 2, "mc68328_w: Unknown address (0xfffa30) = %02x\n", (data >> 8) & 0x00ff);
			}
			break;

		case 0xa32:
			verboselog( *this, 2, "mc68328_w: LGPMR = %04x\n", data);
			m_regs.lgpmr = data;
			verboselog( *this, 3, "              Palette 0: %d\n", (m_regs.lgpmr >>  8) & 0x07);
			verboselog( *this, 3, "              Palette 1: %d\n", (m_regs.lgpmr >> 12) & 0x07);
			verboselog( *this, 3, "              Palette 2: %d\n", (m_regs.lgpmr >>  0) & 0x07);
			verboselog( *this, 3, "              Palette 3: %d\n", (m_regs.lgpmr >>  4) & 0x07);
			break;

		case 0xb00:
			verboselog( *this, 2, "mc68328_w: HMSR(0) = %04x\n", data);
			m_regs.hmsr &= ~(mem_mask << 16);
			m_regs.hmsr |= (data & mem_mask) << 16;
			m_regs.hmsr &= 0x1f3f003f;
			break;

		case 0xb02:
			verboselog( *this, 2, "mc68328_w: HMSR(16) = %04x\n", data);
			m_regs.hmsr &= 0xffff0000 | (~mem_mask);
			m_regs.hmsr |= data & mem_mask;
			m_regs.hmsr &= 0x1f3f003f;
			break;

		case 0xb04:
			verboselog( *this, 2, "mc68328_w: ALARM(0) = %04x\n", data);
			m_regs.alarm &= ~(mem_mask << 16);
			m_regs.alarm |= (data & mem_mask) << 16;
			m_regs.alarm &= 0x1f3f003f;
			break;

		case 0xb06:
			verboselog( *this, 2, "mc68328_w: ALARM(16) = %04x\n", data);
			m_regs.alarm &= 0xffff0000 | (~mem_mask);
			m_regs.alarm |= data & mem_mask;
			m_regs.alarm &= 0x1f3f003f;
			break;

		case 0xb0c:
			verboselog( *this, 2, "mc68328_w: RTCCTL = %04x\n", data);
			m_regs.rtcctl = data & 0x00a0;
			break;

		case 0xb0e:
			verboselog( *this, 2, "mc68328_w: RTCISR = %04x\n", data);
			m_regs.rtcisr &= ~data;
			if (m_regs.rtcisr == 0)
			{
				set_interrupt_line(INT_RTC, 0);
			}
			break;

		case 0xb10:
			verboselog( *this, 2, "mc68328_w: RTCIENR = %04x\n", data);
			m_regs.rtcienr = data & 0x001f;
			break;

		case 0xb12:
			verboselog( *this, 2, "mc68328_w: STPWTCH = %04x\n", data);
			m_regs.stpwtch = data & 0x003f;
			break;

		default:
			verboselog( *this, 0, "mc68328_w: Unknown address (0x%06x) = %04x (%04x)\n", 0xfff000 + address, data, mem_mask);
			break;
	}
}

READ16_MEMBER( mc68328_device::read )
{
	UINT16 temp16;
	UINT32 address = offset << 1;

	switch (address)
	{
		case 0x000:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfff001)\n", mem_mask);
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): SCR = %02x\n", mem_mask, m_regs.scr);
				return m_regs.scr << 8;
			}
			break;

		case 0x100:
			verboselog( *this, 2, "mc68328_r (%04x): GRPBASEA = %04x\n", mem_mask, m_regs.grpbasea);
			return m_regs.grpbasea;

		case 0x102:
			verboselog( *this, 2, "mc68328_r (%04x): GRPBASEB = %04x\n", mem_mask, m_regs.grpbaseb);
			return m_regs.grpbaseb;

		case 0x104:
			verboselog( *this, 2, "mc68328_r (%04x): GRPBASEC = %04x\n", mem_mask, m_regs.grpbasec);
			return m_regs.grpbasec;

		case 0x106:
			verboselog( *this, 2, "mc68328_r (%04x): GRPBASED = %04x\n", mem_mask, m_regs.grpbased);
			return m_regs.grpbased;

		case 0x108:
			verboselog( *this, 2, "mc68328_r (%04x): GRPMASKA = %04x\n", mem_mask, m_regs.grpmaska);
			return m_regs.grpmaska;

		case 0x10a:
			verboselog( *this, 2, "mc68328_r (%04x): GRPMASKB = %04x\n", mem_mask, m_regs.grpmaskb);
			return m_regs.grpmaskb;

		case 0x10c:
			verboselog( *this, 2, "mc68328_r (%04x): GRPMASKC = %04x\n", mem_mask, m_regs.grpmaskc);
			return m_regs.grpmaskc;

		case 0x10e:
			verboselog( *this, 2, "mc68328_r (%04x): GRPMASKD = %04x\n", mem_mask, m_regs.grpmaskd);
			return m_regs.grpmaskd;

		case 0x110:
			verboselog( *this, 5, "mc68328_r (%04x): CSA0(0) = %04x\n", mem_mask, m_regs.csa0 & 0x0000ffff);
			return m_regs.csa0 & 0x0000ffff;

		case 0x112:
			verboselog( *this, 5, "mc68328_r (%04x): CSA0(16) = %04x\n", mem_mask, m_regs.csa0 >> 16);
			return m_regs.csa0 >> 16;

		case 0x114:
			verboselog( *this, 5, "mc68328_r (%04x): CSA1(0) = %04x\n", mem_mask, m_regs.csa1 & 0x0000ffff);
			return m_regs.csa1 & 0x0000ffff;

		case 0x116:
			verboselog( *this, 5, "mc68328_r (%04x): CSA1(16) = %04x\n", mem_mask, m_regs.csa1 >> 16);
			return m_regs.csa1 >> 16;

		case 0x118:
			verboselog( *this, 5, "mc68328_r (%04x): CSA2(0) = %04x\n", mem_mask, m_regs.csa2 & 0x0000ffff);
			return m_regs.csa2 & 0x0000ffff;

		case 0x11a:
			verboselog( *this, 5, "mc68328_r (%04x): CSA2(16) = %04x\n", mem_mask, m_regs.csa2 >> 16);
			return m_regs.csa2 >> 16;

		case 0x11c:
			verboselog( *this, 5, "mc68328_r (%04x): CSA3(0) = %04x\n", mem_mask, m_regs.csa3 & 0x0000ffff);
			return m_regs.csa3 & 0x0000ffff;

		case 0x11e:
			verboselog( *this, 5, "mc68328_r (%04x): CSA3(16) = %04x\n", mem_mask, m_regs.csa3 >> 16);
			return m_regs.csa3 >> 16;

		case 0x120:
			verboselog( *this, 5, "mc68328_r (%04x): CSB0(0) = %04x\n", mem_mask, m_regs.csb0 & 0x0000ffff);
			return m_regs.csb0 & 0x0000ffff;

		case 0x122:
			verboselog( *this, 5, "mc68328_r (%04x): CSB0(16) = %04x\n", mem_mask, m_regs.csb0 >> 16);
			return m_regs.csb0 >> 16;

		case 0x124:
			verboselog( *this, 5, "mc68328_r (%04x): CSB1(0) = %04x\n", mem_mask, m_regs.csb1 & 0x0000ffff);
			return m_regs.csb1 & 0x0000ffff;

		case 0x126:
			verboselog( *this, 5, "mc68328_r (%04x): CSB1(16) = %04x\n", mem_mask, m_regs.csb1 >> 16);
			return m_regs.csb1 >> 16;

		case 0x128:
			verboselog( *this, 5, "mc68328_r (%04x): CSB2(0) = %04x\n", mem_mask, m_regs.csb2 & 0x0000ffff);
			return m_regs.csb2 & 0x0000ffff;

		case 0x12a:
			verboselog( *this, 5, "mc68328_r (%04x): CSB2(16) = %04x\n", mem_mask, m_regs.csb2 >> 16);
			return m_regs.csb2 >> 16;

		case 0x12c:
			verboselog( *this, 5, "mc68328_r (%04x): CSB3(0) = %04x\n", mem_mask, m_regs.csb3 & 0x0000ffff);
			return m_regs.csb3 & 0x0000ffff;

		case 0x12e:
			verboselog( *this, 5, "mc68328_r (%04x): CSB3(16) = %04x\n", mem_mask, m_regs.csb3 >> 16);
			return m_regs.csb3 >> 16;

		case 0x130:
			verboselog( *this, 5, "mc68328_r (%04x): CSC0(0) = %04x\n", mem_mask, m_regs.csc0 & 0x0000ffff);
			return m_regs.csc0 & 0x0000ffff;

		case 0x132:
			verboselog( *this, 5, "mc68328_r (%04x): CSC0(16) = %04x\n", mem_mask, m_regs.csc0 >> 16);
			return m_regs.csc0 >> 16;

		case 0x134:
			verboselog( *this, 5, "mc68328_r (%04x): CSC1(0) = %04x\n", mem_mask, m_regs.csc1 & 0x0000ffff);
			return m_regs.csc1 & 0x0000ffff;

		case 0x136:
			verboselog( *this, 5, "mc68328_r (%04x): CSC1(16) = %04x\n", mem_mask, m_regs.csc1 >> 16);
			return m_regs.csc1 >> 16;

		case 0x138:
			verboselog( *this, 5, "mc68328_r (%04x): CSC2(0) = %04x\n", mem_mask, m_regs.csc2 & 0x0000ffff);
			return m_regs.csc2 & 0x0000ffff;

		case 0x13a:
			verboselog( *this, 5, "mc68328_r (%04x): CSC2(16) = %04x\n", mem_mask, m_regs.csc2 >> 16);
			return m_regs.csc2 >> 16;

		case 0x13c:
			verboselog( *this, 5, "mc68328_r (%04x): CSC3(0) = %04x\n", mem_mask, m_regs.csc3 & 0x0000ffff);
			return m_regs.csc3 & 0x0000ffff;

		case 0x13e:
			verboselog( *this, 5, "mc68328_r (%04x): CSC3(16) = %04x\n", mem_mask, m_regs.csc3 >> 16);
			return m_regs.csc3 >> 16;

		case 0x140:
			verboselog( *this, 5, "mc68328_r (%04x): CSD0(0) = %04x\n", mem_mask, m_regs.csd0 & 0x0000ffff);
			return m_regs.csd0 & 0x0000ffff;

		case 0x142:
			verboselog( *this, 5, "mc68328_r (%04x): CSD0(16) = %04x\n", mem_mask, m_regs.csd0 >> 16);
			return m_regs.csd0 >> 16;

		case 0x144:
			verboselog( *this, 5, "mc68328_r (%04x): CSD1(0) = %04x\n", mem_mask, m_regs.csd1 & 0x0000ffff);
			return m_regs.csd1 & 0x0000ffff;

		case 0x146:
			verboselog( *this, 5, "mc68328_r (%04x): CSD1(16) = %04x\n", mem_mask, m_regs.csd1 >> 16);
			return m_regs.csd1 >> 16;

		case 0x148:
			verboselog( *this, 5, "mc68328_r (%04x): CSD2(0) = %04x\n", mem_mask, m_regs.csd2 & 0x0000ffff);
			return m_regs.csd2 & 0x0000ffff;

		case 0x14a:
			verboselog( *this, 5, "mc68328_r (%04x): CSD2(16) = %04x\n", mem_mask, m_regs.csd2 >> 16);
			return m_regs.csd2 >> 16;

		case 0x14c:
			verboselog( *this, 5, "mc68328_r (%04x): CSD3(0) = %04x\n", mem_mask, m_regs.csd3 & 0x0000ffff);
			return m_regs.csd3 & 0x0000ffff;

		case 0x14e:
			verboselog( *this, 5, "mc68328_r (%04x): CSD3(16) = %04x\n", mem_mask, m_regs.csd3 >> 16);
			return m_regs.csd3 >> 16;

		case 0x200:
			verboselog( *this, 2, "mc68328_r (%04x): PLLCR = %04x\n", mem_mask, m_regs.pllcr);
			return m_regs.pllcr;

		case 0x202:
			verboselog( *this, 2, "mc68328_r (%04x): PLLFSR = %04x\n", mem_mask, m_regs.pllfsr);
			m_regs.pllfsr ^= 0x8000;
			return m_regs.pllfsr;

		case 0x206:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfff206)\n", mem_mask);
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PCTLR = %02x\n", mem_mask, m_regs.pctlr);
				return m_regs.pctlr << 8;
			}
			break;

		case 0x300:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfff301)\n", mem_mask);
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): IVR = %02x\n", mem_mask, m_regs.ivr);
				return m_regs.ivr << 8;
			}
			break;

		case 0x302:
			verboselog( *this, 2, "mc68328_r (%04x): ICR = %04x\n", mem_mask, m_regs.icr);
			return m_regs.icr;

		case 0x304:
			verboselog( *this, 2, "mc68328_r (%04x): IMR(16) = %04x\n", mem_mask, m_regs.imr >> 16);
			return m_regs.imr >> 16;

		case 0x306:
			verboselog( *this, 2, "mc68328_r (%04x): IMR(0) = %04x\n", mem_mask, m_regs.imr & 0x0000ffff);
			return m_regs.imr & 0x0000ffff;

		case 0x308:
			verboselog( *this, 2, "mc68328_r (%04x): IWR(16) = %04x\n", mem_mask, m_regs.iwr >> 16);
			return m_regs.iwr >> 16;

		case 0x30a:
			verboselog( *this, 2, "mc68328_r (%04x): IWR(0) = %04x\n", mem_mask, m_regs.iwr & 0x0000ffff);
			return m_regs.iwr & 0x0000ffff;

		case 0x30c:
			verboselog( *this, 2, "mc68328_r (%04x): ISR(16) = %04x\n", mem_mask, m_regs.isr >> 16);
			return m_regs.isr >> 16;

		case 0x30e:
			verboselog( *this, 2, "mc68328_r (%04x): ISR(0) = %04x\n", mem_mask, m_regs.isr & 0x0000ffff);
			return m_regs.isr & 0x0000ffff;

		case 0x310:
			verboselog( *this, 2, "mc68328_r (%04x): IPR(16) = %04x\n", mem_mask, m_regs.ipr >> 16);
			return m_regs.ipr >> 16;

		case 0x312:
			verboselog( *this, 2, "mc68328_r (%04x): IPR(0) = %04x\n", mem_mask, m_regs.ipr & 0x0000ffff);
			return m_regs.ipr & 0x0000ffff;

		case 0x400:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PADATA = %02x\n", mem_mask, m_regs.padata);
				if (!m_in_port_a_cb.isnull())
				{
					return m_in_port_a_cb(0);
				}
				else
				{
					return m_regs.padata;
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PADIR = %02x\n", mem_mask, m_regs.padir);
				return m_regs.padir << 8;
			}

		case 0x402:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PASEL = %02x\n", mem_mask, m_regs.pasel);
				return m_regs.pasel;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfff402)\n", mem_mask);
			}
			break;

		case 0x408:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PBDATA = %02x\n", mem_mask, m_regs.pbdata);
				if (!m_in_port_b_cb.isnull())
				{
					return m_in_port_b_cb(0);
				}
				else
				{
					return m_regs.pbdata;
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PBDIR = %02x\n", mem_mask, m_regs.pbdir);
				return m_regs.pbdir << 8;
			}

		case 0x40a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PBSEL = %02x\n", mem_mask, m_regs.pbsel);
				return m_regs.pbsel;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfff40a)\n", mem_mask);
			}
			break;

		case 0x410:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PCDATA = %02x\n", mem_mask, m_regs.pcdata);
				if (!m_in_port_c_cb.isnull())
				{
					return m_in_port_c_cb(0);
				}
				else
				{
					return m_regs.pcdata;
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PCDIR = %02x\n", mem_mask, m_regs.pcdir);
				return m_regs.pcdir << 8;
			}

		case 0x412:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PCSEL = %02x\n", mem_mask, m_regs.pcsel);
				return m_regs.pcsel;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfff412)\n", mem_mask);
			}
			break;

		case 0x418:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PDDATA = %02x\n", mem_mask, m_regs.pddata);
				if (!m_in_port_d_cb.isnull())
				{
					return m_in_port_d_cb(0);
				}
				else
				{
					return m_regs.pddata;
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PDDIR = %02x\n", mem_mask, m_regs.pddir);
				return m_regs.pddir << 8;
			}

		case 0x41a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfff41b)\n", mem_mask);
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PDPUEN = %02x\n", mem_mask, m_regs.pdpuen);
				return m_regs.pdpuen << 8;
			}
			break;

		case 0x41c:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PDIRQEN = %02x\n", mem_mask, m_regs.pdirqen);
				return m_regs.pdirqen;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PDPOL = %02x\n", mem_mask, m_regs.pdpol);
				return m_regs.pdpol << 8;
			}

		case 0x41e:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PDIRQEDGE = %02x\n", mem_mask, m_regs.pdirqedge);
				return m_regs.pdirqedge;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfff41e)\n", mem_mask);
			}
			break;

		case 0x420:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PEDATA = %02x\n", mem_mask, m_regs.pedata);
				if (!m_in_port_e_cb.isnull())
				{
					return m_in_port_e_cb(0);
				}
				else
				{
					return m_regs.pedata;
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PEDIR = %02x\n", mem_mask, m_regs.pedir);
				return m_regs.pedir << 8;
			}

		case 0x422:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PESEL = %02x\n", mem_mask, m_regs.pesel);
				return m_regs.pesel;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PEPUEN = %02x\n", mem_mask, m_regs.pepuen);
				return m_regs.pepuen << 8;
			}

		case 0x428:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PFDATA = %02x\n", mem_mask, m_regs.pfdata);
				if (!m_in_port_f_cb.isnull())
				{
					return m_in_port_f_cb(0);
				}
				else
				{
					return m_regs.pfdata;
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PFDIR = %02x\n", mem_mask, m_regs.pfdir);
				return m_regs.pfdir << 8;
			}

		case 0x42a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PFSEL = %02x\n", mem_mask, m_regs.pfsel);
				return m_regs.pfsel;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PFPUEN = %02x\n", mem_mask, m_regs.pfpuen);
				return m_regs.pfpuen << 8;
			}

		case 0x430:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PGDATA = %02x\n", mem_mask, m_regs.pgdata);
				if (!m_in_port_g_cb.isnull())
				{
					return m_in_port_g_cb(0);
				}
				else
				{
					return m_regs.pgdata;
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PGDIR = %02x\n", mem_mask, m_regs.pgdir);
				return m_regs.pgdir << 8;
			}

		case 0x432:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PGSEL = %02x\n", mem_mask, m_regs.pgsel);
				return m_regs.pgsel;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PGPUEN = %02x\n", mem_mask, m_regs.pgpuen);
				return m_regs.pgpuen << 8;
			}

		case 0x438:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PJDATA = %02x\n", mem_mask, m_regs.pjdata);
				if (!m_in_port_j_cb.isnull())
				{
					return m_in_port_j_cb(0);
				}
				else
				{
					return m_regs.pjdata;
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PJDIR = %02x\n", mem_mask, m_regs.pjdir);
				return m_regs.pjdir << 8;
			}

		case 0x43a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PJSEL = %02x\n", mem_mask, m_regs.pjsel);
				return m_regs.pjsel;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfff43a)\n", mem_mask);
			}
			break;

		case 0x440:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PKDATA = %02x\n", mem_mask, m_regs.pkdata);
				if (!m_in_port_k_cb.isnull())
				{
					return m_in_port_k_cb(0);
				}
				else
				{
					return m_regs.pkdata;
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PKDIR = %02x\n", mem_mask, m_regs.pkdir);
				return m_regs.pkdir << 8;
			}

		case 0x442:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PKSEL = %02x\n", mem_mask, m_regs.pksel);
				return m_regs.pksel;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PKPUEN = %02x\n", mem_mask, m_regs.pkpuen);
				return m_regs.pkpuen << 8;
			}

		case 0x448:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PMDATA = %02x\n", mem_mask, m_regs.pmdata);
				if (!m_in_port_m_cb.isnull())
				{
					return m_in_port_m_cb(0);
				}
				else
				{
					return m_regs.pmdata;
				}
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PMDIR = %02x\n", mem_mask, m_regs.pmdir);
				return m_regs.pmdir << 8;
			}

		case 0x44a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): PMSEL = %02x\n", mem_mask, m_regs.pmsel);
				return m_regs.pmsel;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): PMPUEN = %02x\n", mem_mask, m_regs.pmpuen);
				return m_regs.pmpuen << 8;
			}

		case 0x500:
			verboselog( *this, 2, "mc68328_r (%04x): PWMC = %04x\n", mem_mask, m_regs.pwmc);
			temp16 = m_regs.pwmc;
			if (m_regs.pwmc & PWMC_PWMIRQ)
			{
				m_regs.pwmc &= ~PWMC_PWMIRQ;
				set_interrupt_line(INT_PWM, 0);
			}
			return temp16;

		case 0x502:
			verboselog( *this, 2, "mc68328_r (%04x): PWMP = %04x\n", mem_mask, m_regs.pwmp);
			return m_regs.pwmp;

		case 0x504:
			verboselog( *this, 2, "mc68328_r (%04x): PWMW = %04x\n", mem_mask, m_regs.pwmw);
			return m_regs.pwmw;

		case 0x506:
			verboselog( *this, 2, "mc68328_r (%04x): PWMCNT = %04x\n", mem_mask, m_regs.pwmcnt);
			return m_regs.pwmcnt;

		case 0x600:
			verboselog( *this, 2, "mc68328_r (%04x): TCTL1 = %04x\n", mem_mask, m_regs.tctl[0]);
			return m_regs.tctl[0];

		case 0x602:
			verboselog( *this, 2, "mc68328_r (%04x): TPRER1 = %04x\n", mem_mask, m_regs.tprer[0]);
			return m_regs.tprer[0];

		case 0x604:
			verboselog( *this, 2, "mc68328_r (%04x): TCMP1 = %04x\n", mem_mask, m_regs.tcmp[0]);
			return m_regs.tcmp[0];

		case 0x606:
			verboselog( *this, 2, "mc68328_r (%04x): TCR1 = %04x\n", mem_mask, m_regs.tcr[0]);
			return m_regs.tcr[0];

		case 0x608:
			verboselog( *this, 2, "mc68328_r (%04x): TCN1 = %04x\n", mem_mask, m_regs.tcn[0]);
			return m_regs.tcn[0];

		case 0x60a:
			verboselog( *this, 5, "mc68328_r (%04x): TSTAT1 = %04x\n", mem_mask, m_regs.tstat[0]);
			m_regs.tclear[0] |= m_regs.tstat[0];
			return m_regs.tstat[0];

		case 0x60c:
			verboselog( *this, 2, "mc68328_r (%04x): TCTL2 = %04x\n", mem_mask, m_regs.tctl[1]);
			return m_regs.tctl[1];

		case 0x60e:
			verboselog( *this, 2, "mc68328_r (%04x): TPREP2 = %04x\n", mem_mask, m_regs.tprer[1]);
			return m_regs.tprer[1];

		case 0x610:
			verboselog( *this, 2, "mc68328_r (%04x): TCMP2 = %04x\n", mem_mask, m_regs.tcmp[1]);
			return m_regs.tcmp[1];

		case 0x612:
			verboselog( *this, 2, "mc68328_r (%04x): TCR2 = %04x\n", mem_mask, m_regs.tcr[1]);
			return m_regs.tcr[1];

		case 0x614:
			verboselog( *this, 2, "mc68328_r (%04x): TCN2 = %04x\n", mem_mask, m_regs.tcn[1]);
			return m_regs.tcn[1];

		case 0x616:
			verboselog( *this, 2, "mc68328_r (%04x): TSTAT2 = %04x\n", mem_mask, m_regs.tstat[1]);
			m_regs.tclear[1] |= m_regs.tstat[1];
			return m_regs.tstat[1];

		case 0x618:
			verboselog( *this, 2, "mc68328_r (%04x): WCTLR = %04x\n", mem_mask, m_regs.wctlr);
			return m_regs.wctlr;

		case 0x61a:
			verboselog( *this, 2, "mc68328_r (%04x): WCMPR = %04x\n", mem_mask, m_regs.wcmpr);
			return m_regs.wcmpr;

		case 0x61c:
			verboselog( *this, 2, "mc68328_r (%04x): WCN = %04x\n", mem_mask, m_regs.wcn);
			return m_regs.wcn;

		case 0x700:
			verboselog( *this, 2, "mc68328_r (%04x): SPISR = %04x\n", mem_mask, m_regs.spisr);
			return m_regs.spisr;

		case 0x800:
			verboselog( *this, 2, "mc68328_r (%04x): SPIMDATA = %04x\n", mem_mask, m_regs.spimdata);
			if (!m_in_spim_cb.isnull())
			{
				return m_in_spim_cb(0, 0xffff);
			}
			return m_regs.spimdata;

		case 0x802:
			verboselog( *this, 2, "mc68328_r (%04x): SPIMCONT = %04x\n", mem_mask, m_regs.spimcont);
			if (m_regs.spimcont & SPIM_XCH)
			{
				m_regs.spimcont &= ~SPIM_XCH;
				m_regs.spimcont |= SPIM_SPIMIRQ;
				return ((m_regs.spimcont | SPIM_XCH) &~ SPIM_SPIMIRQ);
			}
			return m_regs.spimcont;

		case 0x900:
			verboselog( *this, 2, "mc68328_r (%04x): USTCNT = %04x\n", mem_mask, m_regs.ustcnt);
			return m_regs.ustcnt;

		case 0x902:
			verboselog( *this, 2, "mc68328_r (%04x): UBAUD = %04x\n", mem_mask, m_regs.ubaud);
			return m_regs.ubaud;

		case 0x904:
			verboselog( *this, 5, "mc68328_r (%04x): URX = %04x\n", mem_mask, m_regs.urx);
			return m_regs.urx;

		case 0x906:
			verboselog( *this, 5, "mc68328_r (%04x): UTX = %04x\n", mem_mask, m_regs.utx);
			return m_regs.utx | UTX_FIFO_EMPTY | UTX_FIFO_HALF | UTX_TX_AVAIL;

		case 0x908:
			verboselog( *this, 2, "mc68328_r (%04x): UMISC = %04x\n", mem_mask, m_regs.umisc);
			return m_regs.umisc;

		case 0xa00:
			verboselog( *this, 2, "mc68328_r (%04x): LSSA(16) = %04x\n", mem_mask, m_regs.lssa >> 16);
			return m_regs.lssa >> 16;

		case 0xa02:
			verboselog( *this, 2, "mc68328_r (%04x): LSSA(0) = %04x\n", mem_mask, m_regs.lssa & 0x0000ffff);
			return m_regs.lssa & 0x0000ffff;

		case 0xa04:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): LVPW = %02x\n", mem_mask, m_regs.lvpw);
				return m_regs.lvpw;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfffa04)\n", mem_mask);
			}
			break;

		case 0xa08:
			verboselog( *this, 2, "mc68328_r (%04x): LXMAX = %04x\n", mem_mask, m_regs.lxmax);
			return m_regs.lxmax;

		case 0xa0a:
			verboselog( *this, 2, "mc68328_r (%04x): LYMAX = %04x\n", mem_mask, m_regs.lymax);
			return m_regs.lymax;

		case 0xa18:
			verboselog( *this, 2, "mc68328_r (%04x): LCXP = %04x\n", mem_mask, m_regs.lcxp);
			return m_regs.lcxp;

		case 0xa1a:
			verboselog( *this, 2, "mc68328_r (%04x): LCYP = %04x\n", mem_mask, m_regs.lcyp);
			return m_regs.lcyp;

		case 0xa1c:
			verboselog( *this, 2, "mc68328_r (%04x): LCWCH = %04x\n", mem_mask, m_regs.lcwch);
			return m_regs.lcwch;

		case 0xa1e:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): LBLKC = %02x\n", mem_mask, m_regs.lblkc);
				return m_regs.lblkc;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfffa1e)\n", mem_mask);
			}
			break;

		case 0xa20:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): LPOLCF = %02x\n", mem_mask, m_regs.lpolcf);
				return m_regs.lpolcf;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): LPICF = %02x\n", mem_mask, m_regs.lpicf);
				return m_regs.lpicf << 8;
			}

		case 0xa22:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): LACDRC = %02x\n", mem_mask, m_regs.lacdrc);
				return m_regs.lacdrc;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfffa22)\n", mem_mask);
			}
			break;

		case 0xa24:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): LPXCD = %02x\n", mem_mask, m_regs.lpxcd);
				return m_regs.lpxcd;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfffa24)\n", mem_mask);
			}
			break;

		case 0xa26:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): LCKCON = %02x\n", mem_mask, m_regs.lckcon);
				return m_regs.lckcon;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfffa26)\n", mem_mask);
			}
			break;

		case 0xa28:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): LLBAR = %02x\n", mem_mask, m_regs.llbar);
				return m_regs.llbar;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfffa28)\n", mem_mask);
			}
			break;

		case 0xa2a:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): LOTCR = %02x\n", mem_mask, m_regs.lotcr);
				return m_regs.lotcr;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfffa2a)\n", mem_mask);
			}
			break;

		case 0xa2c:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): LPOSR = %02x\n", mem_mask, m_regs.lposr);
				return m_regs.lposr;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfffa2c)\n", mem_mask);
			}
			break;

		case 0xa30:
			if (mem_mask & 0x00ff)
			{
				verboselog( *this, 2, "mc68328_r (%04x): LFRCM = %02x\n", mem_mask, m_regs.lfrcm);
				return m_regs.lfrcm;
			}
			else
			{
				verboselog( *this, 2, "mc68328_r (%04x): Unknown address (0xfffa30)\n", mem_mask);
			}
			break;

		case 0xa32:
			verboselog( *this, 2, "mc68328_r (%04x): LGPMR = %04x\n", mem_mask, m_regs.lgpmr);
			return m_regs.lgpmr;

		case 0xb00:
			verboselog( *this, 2, "mc68328_r (%04x): HMSR(0) = %04x\n", mem_mask, m_regs.hmsr & 0x0000ffff);
			return m_regs.hmsr & 0x0000ffff;

		case 0xb02:
			verboselog( *this, 2, "mc68328_r (%04x): HMSR(16) = %04x\n", mem_mask, m_regs.hmsr >> 16);
			return m_regs.hmsr >> 16;

		case 0xb04:
			verboselog( *this, 2, "mc68328_r (%04x): ALARM(0) = %04x\n", mem_mask, m_regs.alarm & 0x0000ffff);
			return m_regs.alarm & 0x0000ffff;

		case 0xb06:
			verboselog( *this, 2, "mc68328_r (%04x): ALARM(16) = %04x\n", mem_mask, m_regs.alarm >> 16);
			return m_regs.alarm >> 16;

		case 0xb0c:
			verboselog( *this, 2, "mc68328_r (%04x): RTCCTL = %04x\n", mem_mask, m_regs.rtcctl);
			return m_regs.rtcctl;

		case 0xb0e:
			verboselog( *this, 2, "mc68328_r (%04x): RTCISR = %04x\n", mem_mask, m_regs.rtcisr);
			return m_regs.rtcisr;

		case 0xb10:
			verboselog( *this, 2, "mc68328_r (%04x): RTCIENR = %04x\n", mem_mask, m_regs.rtcienr);
			return m_regs.rtcienr;

		case 0xb12:
			verboselog( *this, 2, "mc68328_r (%04x): STPWTCH = %04x\n", mem_mask, m_regs.stpwtch);
			return m_regs.stpwtch;

		default:
			verboselog( *this, 0, "mc68328_r (%04x): Unknown address (0x%06x)\n", mem_mask, 0xfff000 + address);
			break;
	}
	return 0;
}

/* THIS IS PRETTY MUCH TOTALLY WRONG AND DOESN'T REFLECT THE MC68328'S INTERNAL FUNCTIONALITY AT ALL! */
UINT32 mc68328_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *video_ram = (UINT16 *)(machine().device<ram_device>(RAM_TAG)->pointer() + (m_regs.lssa & 0x00ffffff));
	UINT16 word;
	UINT16 *line;
	int y, x, b;

	if (m_regs.lckcon & LCKCON_LCDC_EN)
	{
		for (y = 0; y < 160; y++)
		{
			line = &bitmap.pix16(y);

			for (x = 0; x < 160; x += 16)
			{
				word = *(video_ram++);
				for (b = 0; b < 16; b++)
				{
					line[x + b] = (word >> (15 - b)) & 0x0001;
				}
			}
		}
	}
	else
	{
		for (y = 0; y < 160; y++)
		{
			line = &bitmap.pix16(y);

			for (x = 0; x < 160; x++)
			{
				line[x] = 0;
			}
		}
	}
	return 0;
}


void mc68328_device::register_state_save()
{
	save_item(NAME(m_regs.scr));
	save_item(NAME(m_regs.grpbasea));
	save_item(NAME(m_regs.grpbaseb));
	save_item(NAME(m_regs.grpbasec));
	save_item(NAME(m_regs.grpbased));
	save_item(NAME(m_regs.grpmaska));
	save_item(NAME(m_regs.grpmaskb));
	save_item(NAME(m_regs.grpmaskc));
	save_item(NAME(m_regs.grpmaskd));
	save_item(NAME(m_regs.csa0));
	save_item(NAME(m_regs.csa1));
	save_item(NAME(m_regs.csa2));
	save_item(NAME(m_regs.csa3));
	save_item(NAME(m_regs.csb0));
	save_item(NAME(m_regs.csb1));
	save_item(NAME(m_regs.csb2));
	save_item(NAME(m_regs.csb3));
	save_item(NAME(m_regs.csc0));
	save_item(NAME(m_regs.csc1));
	save_item(NAME(m_regs.csc2));
	save_item(NAME(m_regs.csc3));
	save_item(NAME(m_regs.csd0));
	save_item(NAME(m_regs.csd1));
	save_item(NAME(m_regs.csd2));
	save_item(NAME(m_regs.csd3));

	save_item(NAME(m_regs.pllcr));
	save_item(NAME(m_regs.pllfsr));
	save_item(NAME(m_regs.pctlr));

	save_item(NAME(m_regs.ivr));
	save_item(NAME(m_regs.icr));
	save_item(NAME(m_regs.imr));
	save_item(NAME(m_regs.iwr));
	save_item(NAME(m_regs.isr));
	save_item(NAME(m_regs.ipr));

	save_item(NAME(m_regs.padir));
	save_item(NAME(m_regs.padata));
	save_item(NAME(m_regs.pasel));
	save_item(NAME(m_regs.pbdir));
	save_item(NAME(m_regs.pbdata));
	save_item(NAME(m_regs.pbsel));
	save_item(NAME(m_regs.pcdir));
	save_item(NAME(m_regs.pcdata));
	save_item(NAME(m_regs.pcsel));
	save_item(NAME(m_regs.pddir));
	save_item(NAME(m_regs.pddata));
	save_item(NAME(m_regs.pdpuen));
	save_item(NAME(m_regs.pdpol));
	save_item(NAME(m_regs.pdirqen));
	save_item(NAME(m_regs.pddataedge));
	save_item(NAME(m_regs.pdirqedge));
	save_item(NAME(m_regs.pedir));
	save_item(NAME(m_regs.pedata));
	save_item(NAME(m_regs.pepuen));
	save_item(NAME(m_regs.pesel));
	save_item(NAME(m_regs.pfdir));
	save_item(NAME(m_regs.pfdata));
	save_item(NAME(m_regs.pfpuen));
	save_item(NAME(m_regs.pfsel));
	save_item(NAME(m_regs.pgdir));
	save_item(NAME(m_regs.pgdata));
	save_item(NAME(m_regs.pgpuen));
	save_item(NAME(m_regs.pgsel));
	save_item(NAME(m_regs.pjdir));
	save_item(NAME(m_regs.pjdata));
	save_item(NAME(m_regs.pjsel));
	save_item(NAME(m_regs.pkdir));
	save_item(NAME(m_regs.pkdata));
	save_item(NAME(m_regs.pkpuen));
	save_item(NAME(m_regs.pksel));
	save_item(NAME(m_regs.pmdir));
	save_item(NAME(m_regs.pmdata));
	save_item(NAME(m_regs.pmpuen));
	save_item(NAME(m_regs.pmsel));

	save_item(NAME(m_regs.pwmc));
	save_item(NAME(m_regs.pwmp));
	save_item(NAME(m_regs.pwmw));
	save_item(NAME(m_regs.pwmcnt));

	save_item(NAME(m_regs.tctl[0]));
	save_item(NAME(m_regs.tctl[1]));
	save_item(NAME(m_regs.tprer[0]));
	save_item(NAME(m_regs.tprer[1]));
	save_item(NAME(m_regs.tcmp[0]));
	save_item(NAME(m_regs.tcmp[1]));
	save_item(NAME(m_regs.tcr[0]));
	save_item(NAME(m_regs.tcr[1]));
	save_item(NAME(m_regs.tcn[0]));
	save_item(NAME(m_regs.tcn[1]));
	save_item(NAME(m_regs.tstat[0]));
	save_item(NAME(m_regs.tstat[1]));
	save_item(NAME(m_regs.wctlr));
	save_item(NAME(m_regs.wcmpr));
	save_item(NAME(m_regs.wcn));

	save_item(NAME(m_regs.spisr));

	save_item(NAME(m_regs.spimdata));
	save_item(NAME(m_regs.spimcont));

	save_item(NAME(m_regs.ustcnt));
	save_item(NAME(m_regs.ubaud));
	save_item(NAME(m_regs.urx));
	save_item(NAME(m_regs.utx));
	save_item(NAME(m_regs.umisc));

	save_item(NAME(m_regs.lssa));
	save_item(NAME(m_regs.lvpw));
	save_item(NAME(m_regs.lxmax));
	save_item(NAME(m_regs.lymax));
	save_item(NAME(m_regs.lcxp));
	save_item(NAME(m_regs.lcyp));
	save_item(NAME(m_regs.lcwch));
	save_item(NAME(m_regs.lblkc));
	save_item(NAME(m_regs.lpicf));
	save_item(NAME(m_regs.lpolcf));
	save_item(NAME(m_regs.lacdrc));
	save_item(NAME(m_regs.lpxcd));
	save_item(NAME(m_regs.lckcon));
	save_item(NAME(m_regs.llbar));
	save_item(NAME(m_regs.lotcr));
	save_item(NAME(m_regs.lposr));
	save_item(NAME(m_regs.lfrcm));
	save_item(NAME(m_regs.lgpmr));

	save_item(NAME(m_regs.hmsr));
	save_item(NAME(m_regs.alarm));
	save_item(NAME(m_regs.rtcctl));
	save_item(NAME(m_regs.rtcisr));
	save_item(NAME(m_regs.rtcienr));
	save_item(NAME(m_regs.stpwtch));
}
