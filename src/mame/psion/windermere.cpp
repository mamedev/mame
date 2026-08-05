// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Nigel Barnes
// thanks-to:Ash Wolf
/***************************************************************************

    Psion Windermere peripheral emulation

    Windermere is a SoC based around the ARM710T processor core.

    Functionality includes:
    - ARM7TDMI CPU core
    - Interrupt controller
    - CODEC interface
    - LCD controller
    - 2 x UART's
    - Timer counters
    - RTC

***************************************************************************/

#include "emu.h"
#include "windermere.h"
#include "screen.h"


#define LOG_UNKN  (1U << 1)
#define LOG_IRQ   (1U << 2)
#define LOG_DRAM  (1U << 4)
#define LOG_LCD   (1U << 5)
#define LOG_PWR   (1U << 6)
#define LOG_EOI   (1U << 7)
#define LOG_INT   (1U << 8)
#define LOG_CODEC (1U << 9)
#define LOG_SSP   (1U << 10)
#define LOG_TIMER (1U << 11)
#define LOG_BUZ   (1U << 12)
#define LOG_RTC   (1U << 13)
#define LOG_GPIO  (1U << 14)
#define LOG_KBD   (1U << 15)
#define LOG_UART  (1U << 16)

#define LOG_ALL   (LOG_UNKN | LOG_DRAM | LOG_LCD | LOG_PWR | LOG_EOI | LOG_CODEC \
				| LOG_SSP | LOG_TIMER | LOG_BUZ | LOG_RTC | LOG_GPIO | LOG_KBD | LOG_UART)
#define LOG_ALL_IRQ  (LOG_IRQ | LOG_EOI | LOG_INT)

#define VERBOSE   (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(WINDERMERE, windermere_device, "windermere", "Psion Windermere")

windermere_device::windermere_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, WINDERMERE, tag, owner, clock)
	, device_video_interface(mconfig, *this, false)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_lcd_dma_cb(*this, 0x00)
	, m_buz_cb(*this)
	, m_col_cb(*this)
	, m_ssp_r(*this, 0)
	, m_port_r(*this, 0x00)
	, m_port_w(*this)
	, m_pcm_in(*this, 0)
	, m_pcm_out(*this)
{
}


void windermere_device::device_start()
{
	m_timer[0] = timer_alloc(FUNC(windermere_device::update_timer), this);
	m_timer[1] = timer_alloc(FUNC(windermere_device::update_timer), this);
	m_rtc_ticker = timer_alloc(FUNC(windermere_device::update_rtc), this);
	m_codec_timer = timer_alloc(FUNC(windermere_device::update_codec), this);

	m_buzzer_tog = 0;

	save_item(NAME(m_memcfg));
	save_item(NAME(m_dramcfg));

	save_item(NAME(m_timer_load));
	save_item(NAME(m_timer_value));
	save_item(NAME(m_timer_ctrl));

	save_item(NAME(m_pending_ints));
	save_item(NAME(m_int_mask));

	save_item(NAME(m_uartfcr));
	save_item(NAME(m_uartcon));

	save_item(NAME(m_lcd_base_addr));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_lcd_timing));

	save_item(NAME(m_rtc));
	save_item(NAME(m_pwrsr));
	save_item(NAME(m_pwrcnt));
	save_item(NAME(m_pumpcon));
	save_item(NAME(m_kscan));
	save_item(NAME(m_last_ssi_request));
	save_item(NAME(m_ssi_read_counter));
	save_item(NAME(m_confg));

	save_item(NAME(m_port_data));
	save_item(NAME(m_port_ddr));
}

void windermere_device::device_reset()
{
	m_memcfg[0] = 0;
	m_memcfg[1] = 0;
	m_dramcfg = 0;

	m_timer_ctrl[0] = m_timer_value[0] = m_timer_load[0] = 0;
	m_timer_ctrl[1] = m_timer_value[1] = m_timer_load[1] = 0;

	m_timer[0]->adjust(attotime::never);
	m_timer[1]->adjust(attotime::never);

	m_pending_ints = 0;
	m_int_mask = 0;

	m_lcd_base_addr[0] = 0;
	m_lcd_base_addr[1] = 0;
	m_lcd_control = 0;

	m_maincpu->set_unscaled_clock(clock() * 5);

	m_rtc = time(nullptr) - 946684800;
	m_pwrsr = (1 << 7) | (1 << 12); // DCDET, PFFLG
	m_pwrcnt = 0;
	m_kscan = 0;
	m_last_ssi_request = 0;
	m_ssi_read_counter = 0;
	m_confg = 0;

	std::fill(std::begin(m_port_data), std::end(m_port_data), 0);
	std::fill(std::begin(m_port_ddr), std::end(m_port_ddr), 0);

	m_rtc_ticker->adjust(attotime::from_hz(64), 0, attotime::from_hz(64));
	m_codec_timer->adjust(attotime::from_hz(8000), 0, attotime::from_hz(8000));
}

void windermere_device::extfiq_w(int state)
{
	// level triggered, active low
	if (state)
		m_pending_ints |= (1 << IRQ_EXTFIQ);
	else
		m_pending_ints &= ~(1 << IRQ_EXTFIQ);

	check_interrupts();
}

void windermere_device::eint1_w(int state)
{
	// level triggered, active low
	if (state)
		m_pending_ints |= (1 << IRQ_EINT1);
	else
		m_pending_ints &= ~(1 << IRQ_EINT1);

	check_interrupts();
}

void windermere_device::eint2_w(int state)
{
	// edge triggered, active low
	if (state)
		m_pending_ints |= (1 << IRQ_EINT2);

	check_interrupts();
}

void windermere_device::eint3_w(int state)
{
	// level triggered, active high
	if (state)
		m_pending_ints |= (1 << IRQ_EINT3);
	else
		m_pending_ints &= ~(1 << IRQ_EINT3);

	check_interrupts();
}

void windermere_device::check_interrupts()
{
	LOGMASKED(LOG_IRQ, "Pending FIQs is %04x & %04x & %04x\n", m_pending_ints, m_int_mask, IRQ_FIQ_MASK);
	LOGMASKED(LOG_IRQ, "Pending IRQs is %04x & %04x & %04x\n", m_pending_ints, m_int_mask, IRQ_IRQ_MASK);

	if (m_pending_ints & m_int_mask)
	{
		m_maincpu->resume(SUSPEND_REASON_HALT);
	}

	m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_pending_ints & m_int_mask & IRQ_FIQ_MASK ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, m_pending_ints & m_int_mask & IRQ_IRQ_MASK ? ASSERT_LINE : CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(windermere_device::update_timer)
{
	if (--m_timer_value[param] == 0)
	{
		if ((param == 0) && BIT(m_buzzer_ctrl, 1))
		{
			m_buz_cb(m_buzzer_tog ^= 1);
		}

		LOGMASKED(LOG_IRQ, "Flagging Timer %d IRQ\n", param + 1);
		m_pending_ints |= (1 << (IRQ_TC1OI + param));
		check_interrupts();

		if (BIT(m_timer_ctrl[param], 6))
		{
			m_timer_value[param] = m_timer_load[param];
		}
	}
}

TIMER_CALLBACK_MEMBER(windermere_device::update_rtc)
{
	LOGMASKED(LOG_IRQ, "Flagging periodic IRQ\n");
	if (m_pending_ints & (1 << IRQ_TINT))
		m_pending_ints |= 1 << IRQ_WEINT;
	else
		m_pending_ints |= 1 << IRQ_TINT;

	if ((m_pwrsr & 0x3f) == 0x3f)
	{
		m_rtc++;
		m_pwrsr &= ~0x3f;
	}
	else
	{
		m_pwrsr++;
	}

	check_interrupts();
}

TIMER_CALLBACK_MEMBER(windermere_device::update_codec)
{
	if (BIT(m_confg, 0)) // Enable CODEC data output
	{
		if (!m_codec_tx.empty())
			m_pcm_out(m_codec_tx.dequeue());

		if (m_codec_tx.queue_length() == 8)
		{
			m_pending_ints |= 1 << IRQ_CSINT;
			check_interrupts();
		}
	}

	if (BIT(m_confg, 1)) // Enable CODEC data input
	{
		if (!m_codec_rx.full())
			m_codec_rx.enqueue(m_pcm_in());

		if (m_codec_rx.queue_length() == 8)
		{
			m_pending_ints |= 1 << IRQ_CSINT;
			check_interrupts();
		}
	}
}


void windermere_device::set_timer_ctrl(int timer, uint8_t data)
{
	const uint16_t old = m_timer_ctrl[timer];
	const uint16_t changed = old ^ data;

	m_timer_ctrl[timer] = data;

	if (changed != 0)
	{
		attotime interval = BIT(m_timer_ctrl[timer], 3) ? attotime::from_ticks(1, 512000) : attotime::from_ticks(1, 2000);
		if (BIT(old, 7) && BIT(data, 7))
		{
			m_timer[timer]->adjust(m_timer[timer]->remaining(), timer, interval);
		}
		else if (BIT(data, 7))
		{
			m_timer[timer]->adjust(interval, timer, interval);
		}
		else
		{
			m_timer[timer]->adjust(attotime::never);
		}
	}
}


uint32_t windermere_device::periphs_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0xffffffff;

	switch (offset << 2)
	{
	case REG_MEMCFG1:
		data = m_memcfg[0];
		LOGMASKED(LOG_DRAM, "%s: peripheral read  MEMCFG1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_MEMCFG2:
		data = m_memcfg[1];
		LOGMASKED(LOG_DRAM, "%s: peripheral read  MEMCFG2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_DRAMCFG:
		data = m_dramcfg;
		LOGMASKED(LOG_DRAM, "%s: peripheral read  DRAMCFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_LCDCTL:
		data = m_lcd_control;
		LOGMASKED(LOG_LCD, "%s: peripheral read  LCDCTL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_LCDST:
		data = 0;
		LOGMASKED(LOG_LCD, "%s: peripheral read  LCDST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_LCD_DBAR1:
		data = m_lcd_base_addr[0];
		LOGMASKED(LOG_LCD, "%s: peripheral read  DBAR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_LCD_DBAR2:
		data = m_lcd_base_addr[1];
		LOGMASKED(LOG_LCD, "%s: peripheral read  DBAR2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_LCDT0:
		data = m_lcd_timing[0];
		LOGMASKED(LOG_LCD, "%s: peripheral read  LCDT0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_LCDT1:
		data = m_lcd_timing[1];
		LOGMASKED(LOG_LCD, "%s: peripheral read  LCDT1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_LCDT2:
		data = m_lcd_timing[2];
		LOGMASKED(LOG_LCD, "%s: peripheral read  LCDT2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_PWRSR:
		data = m_pwrsr;
		LOGMASKED(LOG_PWR, "%s: peripheral read  PWRSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PWRCNT:
		data = m_pwrcnt;
		LOGMASKED(LOG_PWR, "%s: peripheral read  PWRCNT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_INTSR:
		data = m_pending_ints & m_int_mask;
		LOGMASKED(LOG_INT, "%s: peripheral read  INTSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_INTRSR:
		data = m_pending_ints;
		LOGMASKED(LOG_INT, "%s: peripheral read  INTRSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_INTENS:
		data = m_int_mask;
		LOGMASKED(LOG_INT, "%s: peripheral read  INTENS = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_INTTEST1:
		LOGMASKED(LOG_INT, "%s: peripheral read  INTTEST1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_INTTEST2:
		LOGMASKED(LOG_INT, "%s: peripheral read  INTTEST2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_UART1DR:
		LOGMASKED(LOG_UART, "%s: peripheral read  UART1DR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART1FCR:
		data = m_uartfcr[0];
		LOGMASKED(LOG_UART, "%s: peripheral read  UART1FCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART1BR:
		LOGMASKED(LOG_UART, "%s: peripheral read  UART1BR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART1CON:
		data = m_uartcon[0];
		LOGMASKED(LOG_UART, "%s: peripheral read  UART1CON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART1FLG:
		data = 0x10;
		LOGMASKED(LOG_UART, "%s: peripheral read  UART1FLG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART1INT:
		LOGMASKED(LOG_UART, "%s: peripheral read  UART1INT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART1INTM:
		LOGMASKED(LOG_UART, "%s: peripheral read  UART1INTM = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART1INTR:
		LOGMASKED(LOG_UART, "%s: peripheral read  UART1INTR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_UART2DR:
		LOGMASKED(LOG_UART, "%s: peripheral read  UART2DR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART2FCR:
		data = m_uartfcr[1];
		LOGMASKED(LOG_UART, "%s: peripheral read  UART2FCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART2BR:
		LOGMASKED(LOG_UART, "%s: peripheral read  UART2BR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART2CON:
		data = m_uartcon[1];
		LOGMASKED(LOG_UART, "%s: peripheral read  UART2CON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART2FLG:
		data = 0x10;
		LOGMASKED(LOG_UART, "%s: peripheral read  UART2FLG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART2INT:
		LOGMASKED(LOG_UART, "%s: peripheral read  UART2INT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART2INTM:
		LOGMASKED(LOG_UART, "%s: peripheral read  UART2INTM = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART2INTR:
		LOGMASKED(LOG_UART, "%s: peripheral read  UART2INTR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_PUMPCON:
		data = m_pumpcon;
		LOGMASKED(LOG_PWR, "%s: peripheral read  PUMPCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_CODR:
		data = m_codec_rx.dequeue();
		LOGMASKED(LOG_CODEC, "%s: peripheral read  CODR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_CONFG:
		data = m_confg;
		LOGMASKED(LOG_CODEC, "%s: peripheral read  CONFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_COLFG:
		data = (m_codec_tx.full() ? 0x02 : 0x00) | (m_codec_rx.empty() ? 0x01 : 0x00);
		LOGMASKED(LOG_CODEC, "%s: peripheral read  COLFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_COTEST:
		LOGMASKED(LOG_CODEC, "%s: peripheral read  COTEST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_SSCR0:
		data = 0;
		LOGMASKED(LOG_SSP, "%s: peripheral read  SSCR0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_SSCR1:
		data = 0;
		LOGMASKED(LOG_SSP, "%s: peripheral read  SSCR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_SSDR:
	{
		// TODO: Proper SSP support
		uint16_t value = m_ssp_r(m_last_ssi_request);
		data = 0;
		switch (m_ssi_read_counter)
		{
		case 4: data = (value >> 5) & 0x7f; break;
		case 5: data = (value << 3) & 0xf8; break;
		}
		if (++m_ssi_read_counter == 6) m_ssi_read_counter = 0;

		// We should be clearing SSEOTI here, possibly,
		// but for now we just leave it on to simplify things
		LOGMASKED(LOG_SSP, "%s: peripheral read  SSDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	}
	case REG_SSSR:
		data = 0;
		LOGMASKED(LOG_SSP, "%s: peripheral read  SSSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_TC1LOAD:
		data = m_timer_load[0];
		LOGMASKED(LOG_TIMER, "%s: peripheral read  TC1LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_TC1VAL:
		data = m_timer_value[0];
		LOGMASKED(LOG_TIMER, "%s: peripheral read  TC1VAL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_TC1CTRL:
		data = m_timer_ctrl[0];
		LOGMASKED(LOG_TIMER, "%s: peripheral read  TC1CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_TC2LOAD:
		data = m_timer_load[1];
		LOGMASKED(LOG_TIMER, "%s: peripheral read  TC2LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_TC2VAL:
		data = m_timer_value[1];
		LOGMASKED(LOG_TIMER, "%s: peripheral read  TC2VAL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_TC2CTRL:
		data = m_timer_ctrl[1];
		LOGMASKED(LOG_TIMER, "%s: peripheral read  TC2CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_RTCDRL:
		data = m_rtc & 0xffff;
		LOGMASKED(LOG_RTC, "%s: peripheral read  RTCDRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_RTCDRU:
		data = m_rtc >> 16;
		LOGMASKED(LOG_RTC, "%s: peripheral read  RTCDRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_RTCMRL:
		LOGMASKED(LOG_RTC, "%s: peripheral read  RTCMRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_RTCMRU:
		LOGMASKED(LOG_RTC, "%s: peripheral read  RTCMRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_PADR:
		data = (m_port_data[PORTA] & m_port_ddr[PORTA]) | (m_port_r[PORTA]() & ~m_port_ddr[PORTA]);
		LOGMASKED(LOG_GPIO, "%s: peripheral read  PADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PBDR:
		data = (m_port_data[PORTB] & m_port_ddr[PORTB]) | (m_port_r[PORTB]() & ~m_port_ddr[PORTB]);
		LOGMASKED(LOG_GPIO, "%s: peripheral read  PBDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PCDR:
		data = (m_port_data[PORTC] & ~m_port_ddr[PORTC]) | (m_port_r[PORTC]() & m_port_ddr[PORTC]);
		LOGMASKED(LOG_GPIO, "%s: peripheral read  PCDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PDDR:
		data = (m_port_data[PORTD] & ~m_port_ddr[PORTD]) | (m_port_r[PORTD]() & m_port_ddr[PORTD]);
		LOGMASKED(LOG_GPIO, "%s: peripheral read  PDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PADDR:
		data = m_port_ddr[PORTA];
		LOGMASKED(LOG_GPIO, "%s: peripheral read  PADDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PBDDR:
		data = m_port_ddr[PORTB];
		LOGMASKED(LOG_GPIO, "%s: peripheral read  PBDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PCDDR:
		data = m_port_ddr[PORTC];
		LOGMASKED(LOG_GPIO, "%s: peripheral read  PCDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PDDDR:
		data = m_port_ddr[PORTD];
		LOGMASKED(LOG_GPIO, "%s: peripheral read  PDDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PEDR:
		data = (m_port_data[PORTE] & m_port_ddr[PORTE]) | (m_port_r[PORTE]() & ~m_port_ddr[PORTE]);
		LOGMASKED(LOG_GPIO, "%s: peripheral read  PEDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PEDDR:
		data = m_port_ddr[PORTE];
		LOGMASKED(LOG_GPIO, "%s: peripheral read  PEDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_KSCAN:
		data = m_kscan;
		LOGMASKED(LOG_KBD, "%s: peripheral read  KSCAN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_LCDMUX:
		LOGMASKED(LOG_LCD, "%s: peripheral read  LCDMUX = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	default:
		LOGMASKED(LOG_UNKN, "%s: peripheral read  Unknown %04x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
		break;
	}
	return data;
}

void windermere_device::periphs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset << 2)
	{
	case REG_MEMCFG1:
		m_memcfg[0] = data;
		LOGMASKED(LOG_DRAM, "%s: peripheral write MEMCFG1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_MEMCFG2:
		m_memcfg[1] = data;
		LOGMASKED(LOG_DRAM, "%s: peripheral write MEMCFG2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_DRAMCFG:
		m_dramcfg = data;
		LOGMASKED(LOG_DRAM, "%s: peripheral write DRAMCFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_LCDCTL:
		LOGMASKED(LOG_LCD, "%s: peripheral write LCDCTL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_lcd_control = data;
		break;
	case REG_LCDST:
		LOGMASKED(LOG_LCD, "%s: peripheral write LCDST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_LCD_DBAR1:
		LOGMASKED(LOG_LCD, "%s: peripheral write DBAR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_lcd_base_addr[0]);
		break;
	case REG_LCD_DBAR2:
		LOGMASKED(LOG_LCD, "%s: peripheral write DBAR2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_lcd_base_addr[1]);
		break;
	case REG_LCDT0:
		LOGMASKED(LOG_LCD, "%s: peripheral write LCDT0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_lcd_timing[0] = data;
		break;
	case REG_LCDT1:
		LOGMASKED(LOG_LCD, "%s: peripheral write LCDT1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_lcd_timing[1] = data;
		break;
	case REG_LCDT2:
		LOGMASKED(LOG_LCD, "%s: peripheral write LCDT2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_lcd_timing[2] = data;
		break;

	case REG_PWRSR:
		LOGMASKED(LOG_PWR, "%s: peripheral write PWRSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_pwrsr);
		break;
	case REG_PWRCNT:
		LOGMASKED(LOG_PWR, "%s: peripheral write PWRCNT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pwrcnt = data;
		m_maincpu->set_unscaled_clock(clock() * (BIT(data, 2) ? 10 : 5));
		break;
	case REG_HALT:
		LOGMASKED(LOG_PWR, "%s: peripheral write HALT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_maincpu->suspend(SUSPEND_REASON_HALT, true);
		m_pwrsr &= ~0x0200;
		break;
	case REG_STBY:
		LOGMASKED(LOG_PWR, "%s: peripheral write STBY = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pwrsr &= ~0x0200;
		break;
	case REG_BLEOI:
		LOGMASKED(LOG_EOI, "%s: peripheral write BLEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pending_ints &= ~(1 << IRQ_BLINT);
		check_interrupts();
		break;
	case REG_MCEOI:
		LOGMASKED(LOG_EOI, "%s: peripheral write MCEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pending_ints &= ~(1 << IRQ_MCINT);
		check_interrupts();
		break;
	case REG_TEOI:
		LOGMASKED(LOG_EOI, "%s: peripheral write TEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pending_ints &= ~(1 << IRQ_TINT);
		m_pending_ints &= ~(1 << IRQ_WEINT);
		check_interrupts();
		break;
	case REG_STFCLR:
		LOGMASKED(LOG_PWR, "%s: peripheral write STFCLR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pwrsr &= ~0x3e00;
		break;
	case REG_E2EOI:
		LOGMASKED(LOG_EOI, "%s: peripheral write E2EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pending_ints &= ~(1 << IRQ_EINT2);
		check_interrupts();
		break;

	case REG_INTENS:
		LOGMASKED(LOG_INT, "%s: peripheral write INTENS = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_mask |= data & mem_mask;
		check_interrupts();
		break;
	case REG_INTENC:
		LOGMASKED(LOG_INT, "%s: peripheral write INTENC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_mask &= ~(data & mem_mask);
		check_interrupts();
		break;
	case REG_INTTEST1:
		LOGMASKED(LOG_INT, "%s: peripheral write INTTEST1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_INTTEST2:
		LOGMASKED(LOG_INT, "%s: peripheral write INTTEST2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_UART1DR:
		LOGMASKED(LOG_UART, "%s: peripheral write UART1DR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART1FCR:
		LOGMASKED(LOG_UART, "%s: peripheral write UART1FCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_uartfcr[0] = data;
		break;
	case REG_UART1BR:
		LOGMASKED(LOG_UART, "%s: peripheral write UART1BR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART1CON:
		LOGMASKED(LOG_UART, "%s: peripheral write UART1CON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_uartcon[0] = data;
		break;
	case REG_UART1INTM:
		LOGMASKED(LOG_UART, "%s: peripheral write UART1INTM = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_UART2DR:
		LOGMASKED(LOG_UART, "%s: peripheral write UART2DR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART2FCR:
		LOGMASKED(LOG_UART, "%s: peripheral write UART2FCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_uartfcr[1] = data;
		break;
	case REG_UART2BR:
		LOGMASKED(LOG_UART, "%s: peripheral write UART2BR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART2CON:
		LOGMASKED(LOG_UART, "%s: peripheral write UART2CON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_uartcon[1] = data;
		break;
	case REG_UART2INT:
		LOGMASKED(LOG_UART, "%s: peripheral write UART2INT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UART2INTM:
		LOGMASKED(LOG_UART, "%s: peripheral write UART2INTM = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_PUMPCON:
		LOGMASKED(LOG_PWR, "%s: peripheral write PUMPCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pumpcon = data;
		break;

	case REG_CODR:
		LOGMASKED(LOG_CODEC, "%s: peripheral write CODR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_codec_tx.enqueue(data & 0xff);
		break;
	case REG_CONFG:
		LOGMASKED(LOG_CODEC, "%s: peripheral write CONFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_confg = data;
		break;
	case REG_COEOI:
		LOGMASKED(LOG_CODEC | LOG_EOI, "%s: peripheral write COEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pending_ints &= ~(1 << IRQ_CSINT);
		check_interrupts();
		break;

	case REG_SSCR0:
		LOGMASKED(LOG_SSP, "%s: peripheral write SSCR0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_SSCR1:
		LOGMASKED(LOG_SSP, "%s: peripheral write SSCR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_SSDR:
		LOGMASKED(LOG_SSP, "%s: peripheral write SSDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		// TODO: Proper SPI support
		if (data != 0)
		{
			m_last_ssi_request = (m_last_ssi_request >> 8) | (data & 0xff00);
		}
		break;
	case REG_SSSR:
		LOGMASKED(LOG_SSP, "%s: peripheral write SSSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_TC1LOAD:
		LOGMASKED(LOG_TIMER, "%s: peripheral write TC1LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_timer_load[0] = data;
		m_timer_value[0] = data;
		break;
	case REG_TC1CTRL:
		LOGMASKED(LOG_TIMER, "%s: peripheral write TC1CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		set_timer_ctrl(0, data);
		break;
	case REG_TC1EOI:
		LOGMASKED(LOG_TIMER | LOG_EOI, "%s: peripheral write TC1EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pending_ints &= ~(1 << IRQ_TC1OI);
		check_interrupts();
		break;
	case REG_TC2LOAD:
		LOGMASKED(LOG_TIMER, "%s: peripheral write TC2LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_timer_load[1] = data;
		m_timer_value[1] = data;
		break;
	case REG_TC2CTRL:
		LOGMASKED(LOG_TIMER, "%s: peripheral write TC2CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		set_timer_ctrl(1, data);
		break;
	case REG_TC2EOI:
		LOGMASKED(LOG_TIMER | LOG_EOI, "%s: peripheral write TC2EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pending_ints &= ~(1 << IRQ_TC2OI);
		check_interrupts();
		break;

	case REG_BZCONT:
		LOGMASKED(LOG_BUZ, "%s: peripheral write BZCONT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_buzzer_ctrl = data;
		if (!BIT(m_buzzer_ctrl, 1))
		{
			m_buz_cb(BIT(m_buzzer_ctrl, 0));
		}
		break;

	case REG_RTCDRL:
		LOGMASKED(LOG_RTC, "%s: peripheral write RTCDRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_rtc &= 0xffff0000;
		m_rtc |= (data & 0xffff);
		break;
	case REG_RTCDRU:
		LOGMASKED(LOG_RTC, "%s: peripheral write RTCDRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_rtc &= 0x0000ffff;
		m_rtc |= (data & 0xffff) << 16;
		break;
	case REG_RTCMRL:
		LOGMASKED(LOG_RTC, "%s: peripheral write RTCMRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_RTCMRU:
		LOGMASKED(LOG_RTC, "%s: peripheral write RTCMRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_RTCEOI:
		LOGMASKED(LOG_RTC | LOG_EOI, "%s: peripheral write RTCEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_pending_ints &= ~(1 << IRQ_RTCMI);
		check_interrupts();
		break;

	case REG_PADR:
		LOGMASKED(LOG_GPIO, "%s: peripheral write PADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (data != m_port_data[PORTA])
		{
			m_port_data[PORTA] = data;
			m_port_w[PORTA](PORTA, data | ~m_port_ddr[PORTA], m_port_ddr[PORTA]);
		}
		break;
	case REG_PBDR:
		LOGMASKED(LOG_GPIO, "%s: peripheral write PBDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (data != m_port_data[PORTB])
		{
			m_port_data[PORTB] = data;
			m_port_w[PORTB](PORTB, data | ~m_port_ddr[PORTB], m_port_ddr[PORTB]);
		}
#if 0
		const uint8_t old = m_port_data[PORTB];
		const uint8_t diff = old ^ data;
		if (diff & 0x3c)
			LOGMASKED(LOG_PIN, "Contrast: %d\n", (data >> 2) & 0x0f);
		if (BIT(diff, 6))
			LOGMASKED(LOG_PIN, "Case Open: %d\n", BIT(data, 6));
		if (BIT(diff, 7))
			LOGMASKED(LOG_PIN, "ETNA CompactFlash Power: %d\n", BIT(data, 7));
#endif
		break;
	case REG_PCDR:
		LOGMASKED(LOG_GPIO, "%s: peripheral write PCDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (data != m_port_data[PORTC])
		{
			m_port_data[PORTC] = data;
			m_port_w[PORTC](PORTC, data | m_port_ddr[PORTC], ~m_port_ddr[PORTC]);
		}
#if 0
		const uint8_t old = m_port_data[PORTC];
		const uint8_t diff = old ^ data;
		if (BIT(diff, 0))
			LOGMASKED(LOG_PIN, "RS232 RTS: %d\n", BIT(data, 0));
		if (BIT(diff, 1))
			LOGMASKED(LOG_PIN, "RS232 DTR Toggle: %d\n", BIT(data, 1));
		if (BIT(diff, 2))
			LOGMASKED(LOG_PIN, "Disable Power LED: %d\n", BIT(data, 2));
		if (BIT(diff, 3))
			LOGMASKED(LOG_PIN, "Enable UART1: %d\n", BIT(data, 3));
		if (BIT(diff, 4))
			LOGMASKED(LOG_PIN, "LCD Backlight: %d\n", BIT(data, 4));
		if (BIT(diff, 5))
			LOGMASKED(LOG_PIN, "Enable UART0: %d\n", BIT(data, 5));
		if (BIT(diff, 6))
			LOGMASKED(LOG_PIN, "Dictaphone: %d\n", BIT(data, 6));
#endif
		break;
	case REG_PDDR:
		LOGMASKED(LOG_GPIO, "%s: peripheral write PDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (data != m_port_data[PORTD])
		{
			m_port_data[PORTD] = data;
			m_port_w[PORTD](PORTD, data | m_port_ddr[PORTD], ~m_port_ddr[PORTD]);
		}
#if 0
		const uint8_t old = m_port_data[PORTD];
		const uint8_t diff = old ^ data;
		if (BIT(diff, 0))
			LOGMASKED(LOG_PIN, "Codec Enable: %d\n", BIT(data, 0));
		if (BIT(diff, 1))
			LOGMASKED(LOG_PIN, "Audio Amp Enable: %d\n", BIT(data, 1));
		if (BIT(diff, 2))
			LOGMASKED(LOG_PIN, "LCD Power: %d\n", BIT(data, 2));
		if (BIT(diff, 3))
			LOGMASKED(LOG_PIN, "ETNA Door: %d\n", BIT(data, 3));
		if (BIT(diff, 4))
			LOGMASKED(LOG_PIN, "Sled: %d\n", BIT(data, 4));
		if (BIT(diff, 5))
			LOGMASKED(LOG_PIN, "Pump Power2: %d\n", BIT(data, 5));
		if (BIT(diff, 6))
			LOGMASKED(LOG_PIN, "Pump Power1: %d\n", BIT(data, 6));
		if (BIT(diff, 7))
			LOGMASKED(LOG_PIN, "ETNA Error: %d\n", BIT(data, 7));
#endif
		break;
	case REG_PADDR:
		LOGMASKED(LOG_GPIO, "%s: peripheral write PADDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_port_ddr[PORTA] = data;
		break;
	case REG_PBDDR:
		LOGMASKED(LOG_GPIO, "%s: peripheral write PBDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_port_ddr[PORTB] = data;
		break;
	case REG_PCDDR:
		LOGMASKED(LOG_GPIO, "%s: peripheral write PCDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_port_ddr[PORTC] = data;
		break;
	case REG_PDDDR:
		LOGMASKED(LOG_GPIO, "%s: peripheral write PDDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_port_ddr[PORTD] = data;
		break;
	case REG_PEDR:
		LOGMASKED(LOG_GPIO, "%s: peripheral write PEDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (data != m_port_data[PORTE])
		{
			m_port_data[PORTE] = data;
			m_port_w[PORTE](PORTE, data | ~m_port_ddr[PORTE], m_port_ddr[PORTE]);
		}
		break;
	case REG_PEDDR:
		LOGMASKED(LOG_GPIO, "%s: peripheral write PEDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_port_ddr[PORTE] = data;
		break;

	case REG_KSCAN:
		LOGMASKED(LOG_KBD, "%s: peripheral write KSCAN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_kscan = data;
		if (BIT(data, 3))
			m_col_cb(1 << (data & 7));
		else if (data & 0x0f)
			m_col_cb(0x00);
		else
			m_col_cb(0xff);
		break;
	case REG_LCDMUX:
		LOGMASKED(LOG_LCD, "%s: peripheral write LCDMUX = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	default:
		LOGMASKED(LOG_UNKN, "%s: peripheral write Unknown %04x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
		break;
	}
}


//-------------------------------------------------
//  LCD Controller
//-------------------------------------------------

uint32_t windermere_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_lcd_control, 0)) // LCD Enable
	{
		const uint32_t base_addr = m_lcd_base_addr[0] & 0x3ffffff;

		const int width  = (m_lcd_timing[0] & 0x3f0) + 16;
		const int height = (m_lcd_timing[1] & 0x3ff) + 1;

		const int bpp = 1 << (m_lcd_dma_cb(base_addr + 1) >> 4);
		const int ppb = 8 / bpp;

		LOGMASKED(LOG_LCD, "bpp: %d, ppb: %d\n", bpp, ppb);
		uint16_t palette[16] = {};
		for (int i = 0; i < 16; i++)
		{
			palette[i] = m_lcd_dma_cb(base_addr +  (i * 2)) | ((m_lcd_dma_cb(base_addr + (i * 2) + 1) << 8) & 0xf00);
			LOGMASKED(LOG_LCD, "palette[%d]: %04x\n", i, palette[i]);
		}

		const pen_t *pen = screen.palette().pens();

		// build our image out
		const int line_width = (width * bpp) / 8;
		for (int y = 0; y < height; y++)
		{
			const int line_offs = 0x20 + (line_width * y);
			uint16_t *line = &bitmap.pix(y + m_lcd_y_offset);
			for (int x = 0; x < width; x++)
			{
				const uint8_t byte = m_lcd_dma_cb(base_addr + line_offs + (x / ppb));
				const int shift = (x & (ppb - 1)) * bpp;
				const int mask  = (1 << bpp) - 1;
				const int pal_idx = (byte >> shift) & mask;

				line[x + m_lcd_x_offset] = pen[palette[pal_idx]];
			}
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}
	return 0;
}
