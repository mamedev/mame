// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Ash Wolf
/***************************************************************************

        Psion 5mx (EPOC R5) series

        Driver by Ryan Holtz, ported from work by Ash Wolf

        TODO:
        - Audio
        - UART support
        - Probably more

        More info:
            https://github.com/Treeki/WindEmu

****************************************************************************/

#include "psion5.h"

#include "utf8.h"

#define LOG_UNKNOWNS        (1U << 1)
#define LOG_IRQ             (1U << 2)
#define LOG_DISPLAY         (1U << 3)
#define LOG_DRAM_READS      (1U << 4)
#define LOG_LCD_READS       (1U << 5)
#define LOG_POWER_READS     (1U << 6)
#define LOG_EOI_READS       (1U << 7)
#define LOG_INT_READS       (1U << 8)
#define LOG_CODEC_READS     (1U << 9)
#define LOG_SSP_READS       (1U << 10)
#define LOG_TIMER_READS     (1U << 11)
#define LOG_BUZZER_READS    (1U << 12)
#define LOG_RTC_READS       (1U << 13)
#define LOG_GPIO_READS      (1U << 14)
#define LOG_KBD_READS       (1U << 15)
#define LOG_DRAM_WRITES     (1U << 16)
#define LOG_LCD_WRITES      (1U << 17)
#define LOG_POWER_WRITES    (1U << 18)
#define LOG_EOI_WRITES      (1U << 19)
#define LOG_INT_WRITES      (1U << 20)
#define LOG_CODEC_WRITES    (1U << 21)
#define LOG_SSP_WRITES      (1U << 22)
#define LOG_TIMER_WRITES    (1U << 23)
#define LOG_BUZZER_WRITES   (1U << 24)
#define LOG_RTC_WRITES      (1U << 25)
#define LOG_GPIO_WRITES     (1U << 26)
#define LOG_PIN_WRITES      (1U << 27)
#define LOG_KBD_WRITES      (1U << 28)

#define LOG_WRITES          (LOG_DRAM_WRITES | LOG_LCD_WRITES | LOG_POWER_WRITES | LOG_EOI_WRITES | LOG_INT_WRITES | LOG_CODEC_WRITES \
							| LOG_SSP_WRITES | LOG_TIMER_WRITES | LOG_BUZZER_WRITES | LOG_RTC_WRITES | LOG_GPIO_WRITES | LOG_PIN_WRITES | LOG_KBD_WRITES)
#define LOG_READS           (LOG_DRAM_READS | LOG_LCD_READS | LOG_POWER_READS | LOG_EOI_READS | LOG_INT_READS | LOG_CODEC_READS \
							| LOG_SSP_READS | LOG_TIMER_READS | LOG_BUZZER_READS | LOG_RTC_READS | LOG_GPIO_READS | LOG_KBD_READS)
#define LOG_ALL_IRQ         (LOG_IRQ | LOG_EOI_WRITES | LOG_INT_WRITES | LOG_EOI_READS | LOG_INT_READS)
#define LOG_ALL             (LOG_UNKNOWNS | LOG_IRQ | LOG_DISPLAY | LOG_WRITES | LOG_READS)

#define VERBOSE             (LOG_CODEC_READS | LOG_CODEC_WRITES | LOG_BUZZER_WRITES)
#include "logmacro.h"

void psion5mx_state::machine_start()
{
	save_item(NAME(m_memcfg));
	save_item(NAME(m_dramcfg));

	save_item(NAME(m_timer_reload));
	save_item(NAME(m_timer_value));
	save_item(NAME(m_timer_ctrl));

	save_item(NAME(m_pending_ints));
	save_item(NAME(m_int_mask));

	save_item(NAME(m_lcd_display_base_addr));

	save_item(NAME(m_rtc));
	save_item(NAME(m_pwrsr));
	save_item(NAME(m_last_ssi_request));
	save_item(NAME(m_ssi_read_counter));

	save_item(NAME(m_kbd_scan));

	save_item(NAME(m_ports));

	m_timers[0] = timer_alloc(FUNC(psion5mx_state::update_timer1), this);
	m_timers[1] = timer_alloc(FUNC(psion5mx_state::update_timer2), this);
	m_periodic = timer_alloc(FUNC(psion5mx_state::update_periodic_irq), this);
	m_rtc_ticker = timer_alloc(FUNC(psion5mx_state::update_rtc), this);
}

void psion5mx_state::machine_reset()
{
	std::fill(std::begin(m_memcfg), std::end(m_memcfg), 0);
	m_dramcfg = 0;

	m_timers[0]->adjust(attotime::never);
	m_timers[1]->adjust(attotime::never);
	m_timer_reload[0] = m_timer_reload[1] = 0;
	m_timer_ctrl[0] = m_timer_ctrl[1] = 0;

	m_pending_ints = 0;
	m_int_mask = 0;

	m_lcd_display_base_addr = 0;

	m_rtc = time(nullptr) - 946684800;
	m_pwrsr = (1 << 10) | (1 << 13);
	m_last_ssi_request = 0;
	m_ssi_read_counter = 0;
	m_kbd_scan = 0;

	std::fill(std::begin(m_ports), std::end(m_ports), 0);

	m_periodic->adjust(attotime::from_hz(64), 0, attotime::from_hz(64));

	m_rtc_ticker->adjust(attotime::from_hz(64), 0, attotime::from_hz(64));
}

void psion5mx_state::check_interrupts()
{
	LOGMASKED(LOG_IRQ, "Pending FIQs is %08x & %08x & %08x\n", m_pending_ints, m_int_mask, IRQ_FIQ_MASK);
	LOGMASKED(LOG_IRQ, "Pending IRQs is %08x & %08x & %08x\n", m_pending_ints, m_int_mask, IRQ_IRQ_MASK);
	bool any_interrupts = (m_pending_ints & m_int_mask) != 0;
	if (any_interrupts)
	{
		m_maincpu->resume(SUSPEND_REASON_HALT);
	}
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_pending_ints & m_int_mask & IRQ_FIQ_MASK ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, m_pending_ints & m_int_mask & IRQ_IRQ_MASK ? ASSERT_LINE : CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(psion5mx_state::update_timer1)
{
	update_timer(0);
	if (BIT(m_buzzer_ctrl, 1))
	{
		m_speaker->level_w(BIT(m_timer_value[0], 15));
	}
}

TIMER_CALLBACK_MEMBER(psion5mx_state::update_timer2)
{
	update_timer(1);
}

TIMER_CALLBACK_MEMBER(psion5mx_state::update_periodic_irq)
{
	LOGMASKED(LOG_IRQ, "Flagging periodic IRQ\n");
	m_pending_ints |= (1 << IRQ_TINT);
	check_interrupts();
}

TIMER_CALLBACK_MEMBER(psion5mx_state::update_rtc)
{
	if ((m_pwrsr & 0x3f) == 0x3f)
	{
		m_rtc++;
		m_pwrsr &= ~0x3f;
	}
	else
	{
		m_pwrsr++;
	}
}

void psion5mx_state::update_timer(int timer)
{
	m_timer_value[timer]--;
	if (m_timer_value[timer] == 0)
	{
		LOGMASKED(LOG_IRQ, "Flagging Timer %d IRQ\n", timer + 1);
		m_pending_ints |= (1 << (IRQ_TC1OI + timer));
		check_interrupts();
		if (BIT(m_timer_ctrl[timer], 6))
		{
			m_timer_value[timer] = m_timer_reload[timer];
		}
	}
}

void psion5mx_state::set_timer_ctrl(int timer, uint32_t value)
{
	const uint32_t old = m_timer_ctrl[timer];
	const uint32_t changed = old ^ value;
	m_timer_ctrl[timer] = value;
	if (changed != 0)
	{
		attotime interval = BIT(m_timer_ctrl[timer], 3) ? attotime::from_ticks(1, 512000) : attotime::from_ticks(1, 2000);
		if (BIT(old, 7) && BIT(value, 7))
		{
			m_timers[timer]->adjust(m_timers[timer]->remaining(), 0, interval);
		}
		else if (BIT(value, 7))
		{
			m_timers[timer]->adjust(interval, 0, interval);
		}
		else
		{
			m_timers[timer]->adjust(attotime::never);
		}
	}
}

uint32_t psion5mx_state::periphs_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t reg = offset << 2;
	uint32_t data = 0;
	switch (reg)
	{
		case REG_MEMCFG1:
			data = m_memcfg[0];
			LOGMASKED(LOG_DRAM_READS, "%s: peripheral read, MEMCFG1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_MEMCFG2:
			data = m_memcfg[1];
			LOGMASKED(LOG_DRAM_READS, "%s: peripheral read, MEMCFG2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_DRAMCFG:
			data = m_dramcfg;
			LOGMASKED(LOG_DRAM_READS, "%s: peripheral read, DRAMCFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_LCDCTL:
			LOGMASKED(LOG_LCD_READS, "%s: peripheral read, LCDCTL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDST:
			LOGMASKED(LOG_LCD_READS, "%s: peripheral read, LCDST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCD_DBAR1:
			data = m_lcd_display_base_addr;
			LOGMASKED(LOG_LCD_READS, "%s: peripheral read, LCD_DBAR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDT0:
			LOGMASKED(LOG_LCD_READS, "%s: peripheral read, LCDT0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDT1:
			LOGMASKED(LOG_LCD_READS, "%s: peripheral read, LCDT1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDT2:
			LOGMASKED(LOG_LCD_READS, "%s: peripheral read, LCDT2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PWRSR:
			data = m_pwrsr;
			LOGMASKED(LOG_POWER_READS, "%s: peripheral read, PWRSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PWRCNT:
			LOGMASKED(LOG_POWER_READS, "%s: peripheral read, PWRCNT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_HALT:
			LOGMASKED(LOG_POWER_READS, "%s: peripheral read, HALT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_STBY:
			LOGMASKED(LOG_POWER_READS, "%s: peripheral read, STBY = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_BLEOI:
			LOGMASKED(LOG_EOI_READS, "%s: peripheral read, BLEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_MCEOI:
			LOGMASKED(LOG_EOI_READS, "%s: peripheral read, MCEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TEOI:
			LOGMASKED(LOG_EOI_READS, "%s: peripheral read, TEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_STFCLR:
			LOGMASKED(LOG_POWER_READS, "%s: peripheral read, STFCLR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_E2EOI:
			LOGMASKED(LOG_EOI_READS, "%s: peripheral read, E2EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_INTSR:
			data = m_pending_ints & m_int_mask;
			LOGMASKED(LOG_INT_READS, "%s: peripheral read, INTSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTRSR:
			data = m_pending_ints;
			LOGMASKED(LOG_INT_READS, "%s: peripheral read, INTRSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTENS:
			data = m_int_mask;
			LOGMASKED(LOG_INT_READS, "%s: peripheral read, INTENS = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTENC:
			LOGMASKED(LOG_INT_READS, "%s: peripheral read, INTENC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTTEST1:
			LOGMASKED(LOG_INT_READS, "%s: peripheral read, INTTEST1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTTEST2:
			LOGMASKED(LOG_INT_READS, "%s: peripheral read, INTTEST2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PUMPCON:
			LOGMASKED(LOG_POWER_READS, "%s: peripheral read, PUMPCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_CODR:
			LOGMASKED(LOG_CODEC_READS, "%s: peripheral read, CODR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_CONFG:
			LOGMASKED(LOG_CODEC_READS, "%s: peripheral read, CONFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COLFG:
			LOGMASKED(LOG_CODEC_READS, "%s: peripheral read, COLFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COEOI:
			LOGMASKED(LOG_CODEC_READS | LOG_EOI_READS, "%s: peripheral read, COEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COTEST:
			LOGMASKED(LOG_CODEC_READS, "%s: peripheral read, COTEST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_SSCR0:
			LOGMASKED(LOG_SSP_READS, "%s: peripheral read, SSCR0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_SSCR1:
			LOGMASKED(LOG_SSP_READS, "%s: peripheral read, SSCR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_SSDR:
		{
			// TODO: Proper SSP support
			uint16_t value = 0;
			switch (m_last_ssi_request)
			{
				case 0xd0d3: // Touch X
					value = 50 + (uint16_t)(m_touchx->read() * 5.7);
					break;
				case 0x9093: // Touch Y
					value = 3834 - (uint16_t)(m_touchy->read() * 13.225);
					break;
				case 0xa4a4: // Main Battery
				case 0xe4e4: // Backup Battery
					value = 3100;
					break;
			}

			if (m_ssi_read_counter == 4) data = (value >> 5) & 0x7f;
			if (m_ssi_read_counter == 5) data = (value << 3) & 0xf8;
			m_ssi_read_counter++;
			if (m_ssi_read_counter == 6) m_ssi_read_counter = 0;

			// We should be clearing SSEOTI here, possibly,
			// but for now we just leave it on to simplify things
			LOGMASKED(LOG_SSP_READS, "%s: peripheral read, SSDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		}
		case REG_SSSR:
			data = 0;
			LOGMASKED(LOG_SSP_READS, "%s: peripheral read, SSSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_TC1LOAD:
			data = m_timer_reload[0];
			LOGMASKED(LOG_TIMER_READS, "%s: peripheral read, TC1LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC1VAL:
			data = m_timer_value[0];
			LOGMASKED(LOG_TIMER_READS, "%s: peripheral read, TC1VAL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC1CTRL:
			data = m_timer_ctrl[0];
			LOGMASKED(LOG_TIMER_READS, "%s: peripheral read, TC1CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC1EOI:
			LOGMASKED(LOG_TIMER_READS | LOG_EOI_READS, "%s: peripheral read, TC1EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2LOAD:
			data = m_timer_reload[1];
			LOGMASKED(LOG_TIMER_READS, "%s: peripheral read, TC2LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2VAL:
			data = m_timer_value[1];
			LOGMASKED(LOG_TIMER_READS, "%s: peripheral read, TC2VAL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2CTRL:
			data = m_timer_ctrl[1];
			LOGMASKED(LOG_TIMER_READS, "%s: peripheral read, TC2CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2EOI:
			LOGMASKED(LOG_EOI_READS, "%s: peripheral read, TC2EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_BZCONT:
			data = m_buzzer_ctrl;
			LOGMASKED(LOG_BUZZER_READS, "%s: peripheral read, BZCONT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_RTCDRL:
			data = (uint16_t)m_rtc;
			LOGMASKED(LOG_RTC_READS, "%s: peripheral read, RTCDRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCDRU:
			data = m_rtc >> 16;
			LOGMASKED(LOG_RTC_READS, "%s: peripheral read, RTCDRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCMRL:
			LOGMASKED(LOG_RTC_READS, "%s: peripheral read, RTCMRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCMRU:
			LOGMASKED(LOG_RTC_READS, "%s: peripheral read, RTCMRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCEOI:
			LOGMASKED(LOG_RTC_READS | LOG_EOI_READS, "%s: peripheral read, RTCEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PADR:
			data = read_keyboard();
			LOGMASKED(LOG_GPIO_READS, "%s: peripheral read, PADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PBDR:
			data = m_ports[PORTB];
			LOGMASKED(LOG_GPIO_READS, "%s: peripheral read, PBDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PCDR:
			data = m_ports[PORTC];
			LOGMASKED(LOG_GPIO_READS, "%s: peripheral read, PCDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PDDR:
			data = m_ports[PORTD];
			LOGMASKED(LOG_GPIO_READS, "%s: peripheral read, PDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PADDR:
			LOGMASKED(LOG_GPIO_READS, "%s: peripheral read, PADDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PBDDR:
			LOGMASKED(LOG_GPIO_READS, "%s: peripheral read, PBDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PCDDR:
			LOGMASKED(LOG_GPIO_READS, "%s: peripheral read, PCDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PDDDR:
			LOGMASKED(LOG_GPIO_READS, "%s: peripheral read, PDDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PEDR:
			LOGMASKED(LOG_GPIO_READS, "%s: peripheral read, PEDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PEDDR:
			LOGMASKED(LOG_GPIO_READS, "%s: peripheral read, PEDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_KSCAN:
			data = m_kbd_scan;
			LOGMASKED(LOG_KBD_READS, "%s: peripheral read, KSCAN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDMUX:
			LOGMASKED(LOG_LCD_READS, "%s: peripheral read, LCDMUX = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		default:
			LOGMASKED(LOG_UNKNOWNS, "%s: peripheral read, Unknown = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
	}
	return data;
}

void psion5mx_state::periphs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint32_t reg = offset << 2;
	switch (reg)
	{
		case REG_MEMCFG1:
			m_memcfg[0] = data;
			LOGMASKED(LOG_DRAM_WRITES, "%s: peripheral write, MEMCFG1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_MEMCFG2:
			m_memcfg[1] = data;
			LOGMASKED(LOG_DRAM_WRITES, "%s: peripheral write, MEMCFG2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_DRAMCFG:
			m_dramcfg = data;
			LOGMASKED(LOG_DRAM_WRITES, "%s: peripheral write, DRAMCFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_LCDCTL:
			LOGMASKED(LOG_LCD_WRITES, "%s: peripheral write, LCDCTL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDST:
			LOGMASKED(LOG_LCD_WRITES, "%s: peripheral write, LCDST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCD_DBAR1:
			LOGMASKED(LOG_LCD_WRITES, "%s: peripheral write, LCD_DBAR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_lcd_display_base_addr);
			break;
		case REG_LCDT0:
			LOGMASKED(LOG_LCD_WRITES, "%s: peripheral write, LCDT0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDT1:
			LOGMASKED(LOG_LCD_WRITES, "%s: peripheral write, LCDT1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDT2:
			LOGMASKED(LOG_LCD_WRITES, "%s: peripheral write, LCDT2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PWRSR:
			COMBINE_DATA(&m_pwrsr);
			LOGMASKED(LOG_POWER_WRITES, "%s: peripheral write, PWRSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PWRCNT:
			LOGMASKED(LOG_POWER_WRITES, "%s: peripheral write, PWRCNT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_HALT:
			m_maincpu->suspend(SUSPEND_REASON_HALT, 1);
			LOGMASKED(LOG_POWER_WRITES, "%s: peripheral write, HALT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_STBY:
			LOGMASKED(LOG_POWER_WRITES, "%s: peripheral write, STBY = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_BLEOI:
			LOGMASKED(LOG_EOI_WRITES, "%s: peripheral write, BLEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_MCEOI:
			LOGMASKED(LOG_EOI_WRITES, "%s: peripheral write, MCEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TEOI:
			LOGMASKED(LOG_EOI_WRITES, "%s: peripheral write, TEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_pending_ints &= ~(1 << IRQ_TINT);
			check_interrupts();
			break;
		case REG_STFCLR:
			m_pwrsr &= ~0x00003e00;
			LOGMASKED(LOG_POWER_WRITES, "%s: peripheral write, STFCLR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_E2EOI:
			LOGMASKED(LOG_EOI_WRITES, "%s: peripheral write, E2EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_INTSR:
			LOGMASKED(LOG_INT_WRITES, "%s: peripheral write, INTSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTRSR:
			LOGMASKED(LOG_INT_WRITES, "%s: peripheral write, INTRSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTENS:
			m_int_mask |= data & mem_mask;
			LOGMASKED(LOG_INT_WRITES, "%s: peripheral write, INTENS = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			check_interrupts();
			break;
		case REG_INTENC:
			m_int_mask &= ~(data & mem_mask);
			LOGMASKED(LOG_INT_WRITES, "%s: peripheral write, INTENC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			check_interrupts();
			break;
		case REG_INTTEST1:
			LOGMASKED(LOG_INT_WRITES, "%s: peripheral write, INTTEST1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTTEST2:
			LOGMASKED(LOG_INT_WRITES, "%s: peripheral write, INTTEST2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PUMPCON:
			LOGMASKED(LOG_POWER_WRITES, "%s: peripheral write, PUMPCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_CODR:
			LOGMASKED(LOG_CODEC_WRITES, "%s: peripheral write, CODR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_CONFG:
			LOGMASKED(LOG_CODEC_WRITES, "%s: peripheral write, CONFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COLFG:
			LOGMASKED(LOG_CODEC_WRITES, "%s: peripheral write, COLFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COEOI:
			LOGMASKED(LOG_CODEC_WRITES | LOG_EOI_WRITES, "%s: peripheral write, COEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COTEST:
			LOGMASKED(LOG_CODEC_WRITES, "%s: peripheral write, COTEST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_SSCR0:
			LOGMASKED(LOG_SSP_WRITES, "%s: peripheral write, SSCR0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_SSCR1:
			LOGMASKED(LOG_SSP_WRITES, "%s: peripheral write, SSCR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_SSDR:
			LOGMASKED(LOG_SSP_WRITES, "%s: peripheral write, SSDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			// TODO: Proper SPI support
			if (data != 0)
			{
				m_last_ssi_request = (m_last_ssi_request >> 8) | (data & 0xff00);
			}
			break;
		case REG_SSSR:
			LOGMASKED(LOG_SSP_WRITES, "%s: peripheral write, SSSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_TC1LOAD:
			LOGMASKED(LOG_TIMER_WRITES, "%s: peripheral write, TC1LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_timer_reload[0] = data;
			m_timer_value[0] = data;
			break;
		case REG_TC1VAL:
			LOGMASKED(LOG_TIMER_WRITES, "%s: peripheral write, TC1VAL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC1CTRL:
			LOGMASKED(LOG_TIMER_WRITES, "%s: peripheral write, TC1CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			set_timer_ctrl(0, data);
			break;
		case REG_TC1EOI:
			LOGMASKED(LOG_TIMER_WRITES | LOG_EOI_WRITES, "%s: peripheral write, TC1EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_pending_ints &= ~(1 << IRQ_TC1OI);
			check_interrupts();
			break;
		case REG_TC2LOAD:
			LOGMASKED(LOG_TIMER_WRITES, "%s: peripheral write, TC2LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_timer_reload[1] = data;
			m_timer_value[1] = data;
			break;
		case REG_TC2VAL:
			LOGMASKED(LOG_TIMER_WRITES, "%s: peripheral write, TC2VAL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2CTRL:
			LOGMASKED(LOG_TIMER_WRITES, "%s: peripheral write, TC2CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			set_timer_ctrl(1, data);
			break;
		case REG_TC2EOI:
			LOGMASKED(LOG_TIMER_WRITES | LOG_EOI_WRITES, "%s: peripheral write, TC2EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_pending_ints &= ~(1 << IRQ_TC2OI);
			check_interrupts();
			break;

		case REG_BZCONT:
			m_buzzer_ctrl = data;
			if (!BIT(m_buzzer_ctrl, 1))
			{
				m_speaker->level_w(BIT(m_buzzer_ctrl, 0));
			}
			LOGMASKED(LOG_BUZZER_WRITES, "%s: peripheral write, BZCONT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_RTCDRL:
			LOGMASKED(LOG_RTC_WRITES, "%s: peripheral write, RTCDRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_rtc &= 0xffff0000;
			m_rtc |= (uint16_t)data;
			break;
		case REG_RTCDRU:
			LOGMASKED(LOG_RTC_WRITES, "%s: peripheral write, RTCDRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_rtc &= 0x0000ffff;
			m_rtc |= data << 16;
			break;
		case REG_RTCMRL:
			LOGMASKED(LOG_RTC_WRITES, "%s: peripheral write, RTCMRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCMRU:
			LOGMASKED(LOG_RTC_WRITES, "%s: peripheral write, RTCMRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCEOI:
			LOGMASKED(LOG_RTC_WRITES | LOG_EOI_WRITES, "%s: peripheral write, RTCEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PADR:
			LOGMASKED(LOG_GPIO_WRITES, "%s: peripheral write, PADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_ports[PORTA] = data;
			break;
		case REG_PBDR:
		{
			LOGMASKED(LOG_GPIO_WRITES, "%s: peripheral write, PBDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			const uint8_t old = m_ports[PORTB];
			m_ports[PORTB] = data;
			const uint8_t diff = old ^ data;
			if (BIT(diff, 0))
				m_etna->eeprom_cs_in(BIT(data, 0));
			if (BIT(diff, 1))
				m_etna->eeprom_clk_in(BIT(data, 1));
			if (diff & 0x3c)
				LOGMASKED(LOG_PIN_WRITES, "Contrast: %d\n", (data >> 2) & 0x0f);
			if (BIT(diff, 6))
				LOGMASKED(LOG_PIN_WRITES, "Case Open: %d\n", BIT(data, 6));
			if (BIT(diff, 7))
				LOGMASKED(LOG_PIN_WRITES, "ETNA CompactFlash Power: %d\n", BIT(data, 7));
			break;
		}
		case REG_PCDR:
		{
			LOGMASKED(LOG_GPIO_WRITES, "%s: peripheral write, PCDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			const uint8_t old = m_ports[PORTC];
			m_ports[PORTC] = data;
			const uint8_t diff = old ^ data;
			if (BIT(diff, 0))
				LOGMASKED(LOG_PIN_WRITES, "RS232 RTS: %d\n", BIT(data, 0));
			if (BIT(diff, 1))
				LOGMASKED(LOG_PIN_WRITES, "RS232 DTR Toggle: %d\n", BIT(data, 1));
			if (BIT(diff, 2))
				LOGMASKED(LOG_PIN_WRITES, "Disable Power LED: %d\n", BIT(data, 2));
			if (BIT(diff, 3))
				LOGMASKED(LOG_PIN_WRITES, "Enable UART1: %d\n", BIT(data, 3));
			if (BIT(diff, 4))
				LOGMASKED(LOG_PIN_WRITES, "LCD Backlight: %d\n", BIT(data, 4));
			if (BIT(diff, 5))
				LOGMASKED(LOG_PIN_WRITES, "Enable UART0: %d\n", BIT(data, 5));
			if (BIT(diff, 6))
				LOGMASKED(LOG_PIN_WRITES, "Dictaphone: %d\n", BIT(data, 6));
			break;
		}
		case REG_PDDR:
		{
			LOGMASKED(LOG_GPIO_WRITES, "%s: peripheral write, PDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			const uint8_t old = m_ports[PORTD];
			m_ports[PORTD] = data;
			const uint8_t diff = old ^ data;
			if (BIT(diff, 0))
				LOGMASKED(LOG_PIN_WRITES, "Codec Enable: %d\n", BIT(data, 0));
			if (BIT(diff, 1))
				LOGMASKED(LOG_PIN_WRITES, "Audio Amp Enable: %d\n", BIT(data, 1));
			if (BIT(diff, 2))
				LOGMASKED(LOG_PIN_WRITES, "LCD Power: %d\n", BIT(data, 2));
			if (BIT(diff, 3))
				LOGMASKED(LOG_PIN_WRITES, "ETNA Door: %d\n", BIT(data, 3));
			if (BIT(diff, 4))
				LOGMASKED(LOG_PIN_WRITES, "Sled: %d\n", BIT(data, 4));
			if (BIT(diff, 5))
				LOGMASKED(LOG_PIN_WRITES, "Pump Power2: %d\n", BIT(data, 5));
			if (BIT(diff, 6))
				LOGMASKED(LOG_PIN_WRITES, "Pump Power1: %d\n", BIT(data, 6));
			if (BIT(diff, 7))
				LOGMASKED(LOG_PIN_WRITES, "ETNA Error: %d\n", BIT(data, 7));
			break;
		}
		case REG_PADDR:
			LOGMASKED(LOG_GPIO_WRITES, "%s: peripheral write, PADDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PBDDR:
			LOGMASKED(LOG_GPIO_WRITES, "%s: peripheral write, PBDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PCDDR:
			LOGMASKED(LOG_GPIO_WRITES, "%s: peripheral write, PCDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PDDDR:
			LOGMASKED(LOG_GPIO_WRITES, "%s: peripheral write, PDDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PEDR:
			LOGMASKED(LOG_GPIO_WRITES, "%s: peripheral write, PEDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PEDDR:
			LOGMASKED(LOG_GPIO_WRITES, "%s: peripheral write, PEDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_KSCAN:
			m_kbd_scan = data;
			LOGMASKED(LOG_KBD_WRITES, "%s: peripheral write, KSCAN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDMUX:
			LOGMASKED(LOG_LCD_WRITES, "%s: peripheral write, LCDMUX = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		default:
			LOGMASKED(LOG_UNKNOWNS, "%s: peripheral write, Unknown = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
	}
}

uint8_t psion5mx_state::read_keyboard()
{
	if (BIT(m_kbd_scan, 3))
	{
		return m_kbd_cols[m_kbd_scan & 7]->read();
	}
	else if (m_kbd_scan == 0)
	{
		uint8_t rows = 0;
		for (int i = 0; i < 8; i++)
		{
			rows |= m_kbd_cols[i]->read();
		}
		return rows;
	}
	return 0x00;
}

void psion5mx_state::main_map(address_map &map)
{
	map(0x00000000, 0x009fffff).rom().region("maincpu", 0);
	map(0x20000000, 0x20000fff).rw(m_etna, FUNC(etna_device::regs_r), FUNC(etna_device::regs_w));
	map(0x80000000, 0x80000fff).rw(FUNC(psion5mx_state::periphs_r), FUNC(psion5mx_state::periphs_w));
	map(0xc0000000, 0xc03fffff).ram().mirror(0x1fc00000).share("lcd_ram");
}

void psion5mx_state::palette_init(palette_device &palette)
{
	for (int i = 0; i < 16; i++)
	{
		const int r = (0x99 * i) / 15;
		const int g = (0xaa * i) / 15;
		const int b = (0x88 * i) / 15;
		m_palette->set_pen_color(15 - i, rgb_t(r, g, b));
	}
}

uint32_t psion5mx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint8_t *lcd_buf = (uint8_t*)&m_lcd_ram[(m_lcd_display_base_addr & 0x003fffff) >> 2];
	static const int width = 640;
	static const int height = 240;

	// fetch palette
	LOGMASKED(LOG_DISPLAY, "%02x %02x %02x %02x\n", lcd_buf[0], lcd_buf[1], lcd_buf[2], lcd_buf[3]);

	const int bpp = 1 << (lcd_buf[1] >> 4);
	const int ppb = 8 / bpp;
	LOGMASKED(LOG_DISPLAY, "bpp: %d, ppb: %d\n", bpp, ppb);
	uint16_t palette[16] = {};
	for (int i = 0; i < 16; i++)
	{
		palette[i] = lcd_buf[i*2] | ((lcd_buf[i*2+1] << 8) & 0xf00);
		LOGMASKED(LOG_DISPLAY, "palette[%d]: %04x\n", i, palette[i]);
	}

	const pen_t *pen = m_palette->pens();

	// build our image out
	const int line_width = (width * bpp) / 8;
	for (int y = 0; y < height; y++)
	{
		const int line_offs = 0x20 + (line_width * y);
		uint32_t *line = &bitmap.pix(y);
		for (int x = 0; x < width; x++)
		{
			const uint8_t byte = lcd_buf[line_offs + (x / ppb)];
			const int shift = (x & (ppb - 1)) * bpp;
			const int mask = (1 << bpp) - 1;
			const int pal_idx = (byte >> shift) & mask;

			line[x] = pen[palette[pal_idx]];
		}
	}
	return 0;
}

INPUT_CHANGED_MEMBER(psion5mx_state::touch_down)
{
	if (newval)
	{
		logerror("Flagging EINT3\n");
		m_pending_ints |= (1 << IRQ_EINT3);
	}
	else
	{
		logerror("Unflagging EINT3\n");
		m_pending_ints &= ~(1 << IRQ_EINT3);
	}
	check_interrupts();
}

/* Input ports */
INPUT_PORTS_START( psion5mx )
	PORT_START("TOUCHX")
	PORT_BIT(0x3ff, 362, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(42,681) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCHY")
	PORT_BIT(0x0ff, 125, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(5,244) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCH")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Touch") PORT_CHANGED_MEMBER(DEVICE_SELF, psion5mx_state, touch_down, 0)
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR(163) PORT_CHAR('\\')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Dictaphone Record") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("'") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('~') PORT_CHAR(':')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Del<-") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*') PORT_CHAR(']')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&') PORT_CHAR('[')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Dictaphone Play") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(128)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(';')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR('-')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Menu") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Fn") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('/')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Dictaphone Stop") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

/* basic configuration for 2 lines display */
void psion5mx_state::psion5mx(machine_config &config)
{
	/* basic machine hardware */
	ARM710T(config, m_maincpu, 36000000); // 36MHz, per wikipedia
	m_maincpu->set_addrmap(AS_PROGRAM, &psion5mx_state::main_map);

	ETNA(config, m_etna);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0)); /* not accurate */
	screen.set_screen_update(FUNC(psion5mx_state::screen_update));
	screen.set_size(640, 240);
	screen.set_visarea(0, 640-1, 0, 240-1);

	PALETTE(config, m_palette, FUNC(psion5mx_state::palette_init), 16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);
}

/* ROM definition */

ROM_START( psion5mx )
	ROM_REGION( 0xa00000, "maincpu", 0 )
	ROM_LOAD( "5mx.rom", 0x000000, 0xa00000, CRC(a1e2d038) SHA1(4c082321264e1ae7fe77699e59b8960460690fa6) )
ROM_END

/* Driver */

//    YEAR  NAME        PARENT   COMPAT  MACHINE    INPUT     CLASS           INIT        COMPANY  FULLNAME  FLAGS
COMP( 1999, psion5mx,   0,       0,      psion5mx,  psion5mx, psion5mx_state, empty_init, "Psion", "Series 5mx",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
