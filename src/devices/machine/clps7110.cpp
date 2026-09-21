// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    CL-PS7110 - Low-Power System-on-a-Chip

***************************************************************************/

#include "emu.h"
#include "clps7110.h"
#include "screen.h"


#define LOG_UNKN   (1U << 1)
#define LOG_IRQ    (1U << 2)
#define LOG_DRAM   (1U << 3)
#define LOG_LCD    (1U << 4)
#define LOG_PWR    (1U << 5)
#define LOG_INT    (1U << 6)
#define LOG_CODEC  (1U << 7)
#define LOG_SSP    (1U << 8)
#define LOG_TMR    (1U << 9)
#define LOG_BUZ    (1U << 10)
#define LOG_RTC    (1U << 11)
#define LOG_GPIO   (1U << 12)
#define LOG_UART   (1U << 13)

#define LOG_ALL    (LOG_DRAM | LOG_LCD | LOG_PWR | LOG_INT | LOG_CODEC | LOG_UART | LOG_GPIO \
					| LOG_SSP | LOG_TMR | LOG_BUZ | LOG_RTC | LOG_GPIO | LOG_UNKN)
#define LOG_ALL_IRQ  (LOG_IRQ | LOG_INT)

#define VERBOSE      (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(CLPS7110, clps7110_device, "clps7110", "CL-PS7110")
DEFINE_DEVICE_TYPE(CLPS7111, clps7111_device, "clps7111", "CL-PS7111")

clps711x_device::clps711x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t id)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this, false)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_lcd_dma_cb(*this, 0x00)
	, m_buz_cb(*this)
	, m_col_cb(*this)
	, m_adc_r(*this, 0)
	, m_port_r(*this, 0x00)
	, m_port_w(*this)
	, m_pcm_in(*this, 0)
	, m_pcm_out(*this)
	, m_id(id)
{
}

clps7110_device::clps7110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: clps711x_device(mconfig, CLPS7110, tag, owner, clock, 0)
{
}

clps7111_device::clps7111_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: clps711x_device(mconfig, CLPS7111, tag, owner, clock, 1)
{
}


void clps711x_device::device_start()
{
	m_timer[0] = timer_alloc(FUNC(clps711x_device::update_timer), this);
	m_timer[1] = timer_alloc(FUNC(clps711x_device::update_timer), this);
	m_rtc_ticker = timer_alloc(FUNC(clps711x_device::update_rtc), this);

	m_buzzer_tog = 0;

	save_item(NAME(m_memcfg));
	save_item(NAME(m_dramcfg));
	save_item(NAME(m_syscon));
	save_item(NAME(m_sysflg1));
	save_item(NAME(m_syncio));
	save_item(NAME(m_pmpcon));

	save_item(NAME(m_timer_reload));
	save_item(NAME(m_timer_value));

	save_item(NAME(m_int_status));
	save_item(NAME(m_int_mask));

	save_item(NAME(m_lcd_base_addr));
	save_item(NAME(m_lcdcon));
	save_item(NAME(m_lcdpal));

	save_item(NAME(m_rtc));
	save_item(NAME(m_rtcdiv));

	save_item(NAME(m_port_data));
	save_item(NAME(m_port_ddr));
}

void clps711x_device::device_reset()
{
	m_memcfg[0] = 0xffff0ff;
	m_memcfg[1] = 0xffff0ff;
	m_dramcfg   = 0;
	m_syscon[0] = 0;
	m_syscon[1] = 0;
	m_sysflg1 = (1 << 1) | (1 << 14) | (m_id << 29); // DCDET, PFFLG, ID

	m_syncio = 0;

	m_timer_value[0] = m_timer_reload[0] = 0;
	m_timer_value[1] = m_timer_reload[1] = 0;

	m_timer[0]->adjust(attotime::from_ticks(1, 2000), 0, attotime::from_ticks(1, 2000));
	m_timer[1]->adjust(attotime::from_ticks(1, 2000), 1, attotime::from_ticks(1, 2000));

	m_int_status = 0;
	m_int_mask = 0;

	m_lcd_base_addr = 0x0c;
	m_lcdcon = 0;
	m_lcdpal = 0;

	m_rtc = time(nullptr) - 946684800;
	m_rtcdiv = 0;

	std::fill(std::begin(m_port_data), std::end(m_port_data), 0);
	std::fill(std::begin(m_port_ddr), std::end(m_port_ddr), 0);

	m_rtc_ticker->adjust(attotime::from_hz(64), 0, attotime::from_hz(64));
}

void clps711x_device::extfiq_w(int state)
{
	// level triggered, active low
	if (state)
		m_int_status |= (1 << IRQ_EXTFIQ);
	else
		m_int_status &= ~(1 << IRQ_EXTFIQ);

	check_interrupts();
}

void clps711x_device::eint1_w(int state)
{
	// level triggered, active low
	if (state)
		m_int_status |= (1 << IRQ_EINT1);
	else
		m_int_status &= ~(1 << IRQ_EINT1);

	check_interrupts();
}

void clps711x_device::eint2_w(int state)
{
	// level triggered, active low
	if (state)
		m_int_status |= (1 << IRQ_EINT2);
	else
		m_int_status &= ~(1 << IRQ_EINT2);

	check_interrupts();
}

void clps711x_device::eint3_w(int state)
{
	// level triggered, active high
	if (state)
		m_int_status |= (1 << IRQ_EINT3);
	else
		m_int_status &= ~(1 << IRQ_EINT3);

	check_interrupts();
}

void clps711x_device::check_interrupts()
{
	LOGMASKED(LOG_IRQ, "Pending FIQs is %08x & %08x & %08x\n", m_int_status, m_int_mask, IRQ_FIQ_MASK);
	LOGMASKED(LOG_IRQ, "Pending IRQs is %08x & %08x & %08x\n", m_int_status, m_int_mask, IRQ_IRQ_MASK);

	if (m_int_status & m_int_mask)
	{
		m_maincpu->resume(SUSPEND_REASON_HALT);
	}
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_int_status & m_int_mask & IRQ_FIQ_MASK ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, m_int_status & m_int_mask & IRQ_IRQ_MASK ? ASSERT_LINE : CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(clps711x_device::update_timer)
{
	if (--m_timer_value[param] == 0)
	{
		if ((param == 0) && BIT(m_syscon[0], 10))
		{
			m_buz_cb(m_buzzer_tog ^= 1);
		}

		if (BIT(m_syscon[0], 4 + (param * 2)))
		{
			m_timer_value[param] = m_timer_reload[param];
		}

		LOGMASKED(LOG_IRQ, "Flagging Timer %d IRQ\n", param + 1);
		m_int_status |= (1 << (IRQ_TC1OI + param));
		check_interrupts();
	}
}

TIMER_CALLBACK_MEMBER(clps711x_device::update_rtc)
{
	LOGMASKED(LOG_IRQ, "Flagging periodic IRQ\n");
	if (m_int_status & (1 << IRQ_TINT))
		m_int_status |= (1 << IRQ_WEINT);
	else
		m_int_status |= (1 << IRQ_TINT);

	if ((++m_rtcdiv & 0x40) == 0x40)
	{
		m_rtc++;
		m_rtcdiv &= 0x3f;
	}

	check_interrupts();
}


uint32_t clps711x_device::periphs_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0xffffffff;

	switch (offset << 2)
	{
	case REG_PADR:
		if (ACCESSING_BITS_0_7) // REG_PADR
		{
			data = (m_port_data[PORTA] & m_port_ddr[PORTA]) | (m_port_r[PORTA]() & ~m_port_ddr[PORTA]);
			LOGMASKED(LOG_GPIO, "%s: read  PADR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_8_15) // REG_PBDR
		{
			data = ((m_port_data[PORTB] & m_port_ddr[PORTB]) | (m_port_r[PORTB]() & ~m_port_ddr[PORTB])) << 8;
			LOGMASKED(LOG_GPIO, "%s: read  PBDR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_16_23) // REG_PCDR
		{
			data = ((m_port_data[PORTC] & ~m_port_ddr[PORTC]) | (m_port_r[PORTC]() & m_port_ddr[PORTC])) << 16;
			LOGMASKED(LOG_GPIO, "%s: read  PCDR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_24_31) // REG_PDDR
		{
			data = ((m_port_data[PORTD] & ~m_port_ddr[PORTD]) | (m_port_r[PORTD]() & m_port_ddr[PORTD])) << 24;
			LOGMASKED(LOG_GPIO, "%s: read  PDDR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		break;
	case REG_PADDR:
		if (ACCESSING_BITS_0_7) // REG_PADDR
		{
			data = m_port_ddr[PORTA];
			LOGMASKED(LOG_GPIO, "%s: read  PADDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_8_15) // REG_PBDDR
		{
			data = m_port_ddr[PORTB] << 8;
			LOGMASKED(LOG_GPIO, "%s: read  PBDDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_16_23) // REG_PCDDR
		{
			data = m_port_ddr[PORTC] << 16;
			LOGMASKED(LOG_GPIO, "%s: read  PCDDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_24_31)  // REG_PDDDR
		{
			data = m_port_ddr[PORTD] << 24;
			LOGMASKED(LOG_GPIO, "%s: read  PDDDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		break;
	case REG_PEDR:
		data = (m_port_data[PORTE] & m_port_ddr[PORTE]) | (m_port_r[PORTE]() & ~m_port_ddr[PORTE]);
		LOGMASKED(LOG_GPIO, "%s: read  PEDR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PEDDR:
		data = m_port_ddr[PORTE];
		LOGMASKED(LOG_GPIO, "%s: read  PEDDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_SYSCON1:
		data = m_syscon[0];
		LOGMASKED(LOG_LCD, "%s: read  SYSCON1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	case REG_SYSFLG1:
		data = m_sysflg1 | (m_rtcdiv << 16);
		//data |= (1 << 1);
		//data |= (m_rtcdiv << 16);
		//data |= (1 << 8);
		//data |= (1 << 22);
		//data |= (1 << 24);
		LOGMASKED(LOG_PWR, "%s: read  SYSFLG1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_MEMCFG1:
		data = m_memcfg[0];
		LOGMASKED(LOG_DRAM, "%s: read  MEMCFG1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_MEMCFG2:
		data = m_memcfg[1];
		LOGMASKED(LOG_DRAM, "%s: read  MEMCFG2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_DRFPR:
		LOGMASKED(LOG_DRAM, "%s: read  DRFPR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_INTSR1:
		data = m_int_status & 0xffff;
		LOGMASKED(LOG_INT, "%s: read  INTSR1  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_INTMR1:
		data = m_int_mask & 0xffff;
		LOGMASKED(LOG_INT, "%s: read  INTMR1  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_LCDCON:
		data = m_lcdcon;
		LOGMASKED(LOG_LCD, "%s: read  LCDCON  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_TC1D:
		data = m_timer_value[0];
		LOGMASKED(LOG_TMR, "%s: read  TC1D    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_TC2D:
		data = m_timer_value[1];
		LOGMASKED(LOG_TMR, "%s: read  TC2D    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_RTCDR:
		data = m_rtc;
		LOGMASKED(LOG_RTC, "%s: read  RTCDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_RTCMR:
		LOGMASKED(LOG_RTC, "%s: read  RTCMR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PMPCON:
		data = m_pmpcon;
		LOGMASKED(LOG_INT, "%s: read  PMPCON  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_CODR:
		LOGMASKED(LOG_CODEC, "%s: read  CODR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UARTDR1:
		data = 0;
		LOGMASKED(LOG_UART, "%s: read  UARTDR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UBRLCR1:
		LOGMASKED(LOG_UART, "%s: read  UBRLCR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_SYNCIO:
		data = m_adc_r(m_syncio);
		//m_int_status &= ~(1 << IRQ_SSEOTI);
		//check_interrupts();
		LOGMASKED(LOG_PWR, "%s: read  SYNCIO  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PALLSW:
		data = m_lcdpal & 0xffffffff;
		LOGMASKED(LOG_LCD, "%s: read  PALLSW  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PALMSW:
		data = m_lcdpal >> 32;
		LOGMASKED(LOG_LCD, "%s: read  PALMSW  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_FRBADDR:
		data = m_lcd_base_addr;
		LOGMASKED(LOG_LCD, "%s: read  FRBADDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_SYSCON2:
		data = m_syscon[1];
		LOGMASKED(LOG_TMR, "%s: read  SYSCON2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_SYSFLG2:
		LOGMASKED(LOG_TMR, "%s: read  SYSFLG2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		data = 0;
		break;
	case REG_INTSR2:
		data = m_int_status >> 16;
		LOGMASKED(LOG_INT, "%s: read  INTSR2  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_INTMR2:
		data = m_int_mask >> 16;
		LOGMASKED(LOG_INT, "%s: read  INTMR2  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UARTDR2:
		data = 0;
		LOGMASKED(LOG_UART, "%s: read  UARTDR2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UBRLCR2:
		LOGMASKED(LOG_UART, "%s: read  UBRLCR2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;

	default:
		LOGMASKED(LOG_UNKN, "%s: read  Unknown = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	}
	return data;
}

void clps711x_device::periphs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset << 2)
	{
	case REG_PADR:
		if (ACCESSING_BITS_0_7) // REG_PADR
		{
			if (data != m_port_data[PORTA])
			{
				m_port_data[PORTA] = data;
				m_port_w[PORTA](PORTA, m_port_data[PORTA] | ~m_port_ddr[PORTA], m_port_ddr[PORTA]);
			}
			LOGMASKED(LOG_GPIO, "%s: write PADR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_8_15) // REG_PBDR
		{
			if (data >> 8 != m_port_data[PORTB])
			{
				m_port_data[PORTB] = data >> 8;
				m_port_w[PORTB](PORTB, m_port_data[PORTB] | ~m_port_ddr[PORTB], m_port_ddr[PORTB]);
			}
			LOGMASKED(LOG_GPIO, "%s: write PBDR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_16_23) // REG_PCDR
		{
			if (data >> 16 != m_port_data[PORTC])
			{
				m_port_data[PORTC] = data >> 16;
				m_port_w[PORTC](PORTC, m_port_data[PORTC] | m_port_ddr[PORTC], ~m_port_ddr[PORTC]);
			}
			LOGMASKED(LOG_GPIO, "%s: write PCDR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_24_31) // REG_PDDR
		{
			if (data >> 24 != m_port_data[PORTD])
			{
				m_port_data[PORTD] = data >> 24;
				m_port_w[PORTD](PORTD, m_port_data[PORTD] | m_port_ddr[PORTD], ~m_port_ddr[PORTD]);
			}
			LOGMASKED(LOG_GPIO, "%s: write PDDR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		break;
	case REG_PADDR:
		if (ACCESSING_BITS_0_7) // REG_PADDR
		{
			m_port_ddr[PORTA] = data & 0xff;
			LOGMASKED(LOG_GPIO, "%s: write PADDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_8_15) // REG_PBDDR
		{
			m_port_ddr[PORTB] = (data >> 8) & 0xff;
			LOGMASKED(LOG_GPIO, "%s: write PBDDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_16_23) // REG_PCDDR
		{
			m_port_ddr[PORTC] = (data >> 16) & 0xff;
			LOGMASKED(LOG_GPIO, "%s: write PCDDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		if (ACCESSING_BITS_24_31) // REG_PDDDR
		{
			m_port_ddr[PORTD] = (data >> 24) & 0xff;
			LOGMASKED(LOG_GPIO, "%s: write PDDDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		break;
	case REG_PEDR:
		if (ACCESSING_BITS_0_7)
		{
			if (data != m_port_data[PORTE])
			{
				m_port_data[PORTE] = data & 0xff;
				m_port_w[PORTE](PORTE, m_port_data[PORTE] | ~m_port_ddr[PORTE], m_port_ddr[PORTE]);
			}
			LOGMASKED(LOG_GPIO, "%s: write PEDR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		break;
	case REG_PEDDR:
		if (ACCESSING_BITS_0_7)
		{
			m_port_ddr[PORTE] = data & 0xff;
			LOGMASKED(LOG_GPIO, "%s: write PEDDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		}
		break;
	case REG_SYSCON1:
		if (BIT(data, 3))
			m_col_cb(1 << (data & 7));
		else if (data & 0x0f)
			m_col_cb(0x00);
		else
			m_col_cb(0xff);

		if (BIT(data, 5) != BIT(m_syscon[0], 5))
		{
			attotime interval = BIT(data, 5) ? attotime::from_ticks(1, 512000) : attotime::from_ticks(1, 2000);
			m_timer[0]->adjust(m_timer[0]->remaining(), 0, interval);
		}
		if (BIT(data, 7) != BIT(m_syscon[0], 7))
		{
			attotime interval = BIT(data, 7) ? attotime::from_ticks(1, 512000) : attotime::from_ticks(1, 2000);
			m_timer[1]->adjust(m_timer[1]->remaining(), 1, interval);
		}
		if (!BIT(data, 10))
		{
			m_buz_cb(BIT(data, 9));
		}
		m_syscon[0] = data;
		LOGMASKED(LOG_LCD, "%s: write SYSCON1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_MEMCFG1:
		LOGMASKED(LOG_DRAM, "%s: write MEMCFG1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_memcfg[0] = data;
		break;
	case REG_MEMCFG2:
		LOGMASKED(LOG_DRAM, "%s: write MEMCFG2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_memcfg[1] = data;
		break;
	case REG_DRFPR:
		LOGMASKED(LOG_PWR, "%s: write DRFPR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_INTMR1:
		LOGMASKED(LOG_INT, "%s: write INTMR1  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_mask &= 0xffff0000;
		m_int_mask |= data & 0xffff;
		check_interrupts();
		break;
	case REG_LCDCON:
		LOGMASKED(LOG_LCD, "%s: write LCDCON  = %08x & %08x, Buffer = %04x, Line = %d\n",
			machine().describe_context(), data, mem_mask, (BIT(data, 0, 13) + 1) * 128 / 8, (BIT(data, 13, 6) + 1) * 16);
		m_lcdcon = data;;
		break;
	case REG_TC1D:
		LOGMASKED(LOG_TMR, "%s: write TC1D    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_timer_reload[0] = data;
		m_timer_value[0] = data;
		break;
	case REG_TC2D:
		LOGMASKED(LOG_TMR, "%s: write TC2D    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_timer_reload[1] = data;
		m_timer_value[1] = data;
		break;
	case REG_RTCDR:
		LOGMASKED(LOG_RTC, "%s: write RTCDR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_rtc = data;
		break;
	case REG_RTCMR:
		LOGMASKED(LOG_RTC, "%s: write RTCMR   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_PMPCON:
		m_pmpcon = data;
		LOGMASKED(LOG_INT, "%s: write PMPCON  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_CODR:
		LOGMASKED(LOG_CODEC, "%s: write CODR    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UARTDR1:
		LOGMASKED(LOG_UART, "%s: write UARTDR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UBRLCR1:
		LOGMASKED(LOG_UART, "%s: write UBRLCR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_SYNCIO:
		LOGMASKED(LOG_PWR, "%s: write SYNCIO  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_syncio = data & 0xffff;
		//m_int_status |= (1 << IRQ_SSEOTI);
		//check_interrupts();
		break;
	case REG_PALLSW:
		LOGMASKED(LOG_LCD, "%s: write PALLSW  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_lcdpal &= 0xffffffff00000000;
		m_lcdpal |= data;
		break;
	case REG_PALMSW:
		LOGMASKED(LOG_LCD, "%s: write PALMSW  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_lcdpal &= 0x00000000ffffffff;
		m_lcdpal |= (uint64_t)data << 32;
		break;
	case REG_STFCLR:
		LOGMASKED(LOG_PWR, "%s: write STFCLR  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_sysflg1 &= 0xffff0fff;
		break;
	case REG_BLEOI:
		LOGMASKED(LOG_INT, "%s: write BLEOI   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_status &= ~(1 << IRQ_BLINT);
		check_interrupts();
		break;
	case REG_MCEOI:
		LOGMASKED(LOG_INT, "%s: write MCEOI   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_status &= ~(1 << IRQ_MCINT);
		check_interrupts();
		break;
	case REG_TEOI:
		LOGMASKED(LOG_INT, "%s: write TEOI    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_status &= ~(1 << IRQ_TINT);
		m_int_status &= ~(1 << IRQ_WEINT);
		check_interrupts();
		break;
	case REG_TC1EOI:
		LOGMASKED(LOG_INT, "%s: write TC1EOI  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_status &= ~(1 << IRQ_TC1OI);
		check_interrupts();
		break;
	case REG_TC2EOI:
		LOGMASKED(LOG_INT, "%s: write TC2EOI  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_status &= ~(1 << IRQ_TC2OI);
		check_interrupts();
		break;
	case REG_RTCEOI:
		LOGMASKED(LOG_INT, "%s: write RTCEOI  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_status &= ~(1 << IRQ_RTCMI);
		check_interrupts();
		break;
	case REG_UMSEOI:
		LOGMASKED(LOG_INT, "%s: write UMSEOI  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_status &= ~(1 << IRQ_UMSINT);
		check_interrupts();
		break;
	case REG_COEOI:
		LOGMASKED(LOG_INT, "%s: write COEOI   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_status &= ~(1 << IRQ_CSINT);
		check_interrupts();
		break;
	case REG_HALT:
		LOGMASKED(LOG_PWR, "%s: write HALT    = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_maincpu->suspend(SUSPEND_REASON_HALT, true);
		break;
	case REG_STDBY:
		LOGMASKED(LOG_PWR, "%s: write STDBY   = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_FRBADDR:
		LOGMASKED(LOG_LCD, "%s: write FRBADDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_lcd_base_addr);
		break;
	case REG_SYSCON2:
		m_syscon[1] = data;
		LOGMASKED(LOG_TMR, "%s: write SYSCON2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_INTMR2:
		m_int_mask &= 0x0000ffff;
		m_int_mask |= data << 16;
		check_interrupts();
		LOGMASKED(LOG_INT, "%s: write INTMR2  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UARTDR2:
		LOGMASKED(LOG_UART, "%s: write UARTDR2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UBRLCR2:
		LOGMASKED(LOG_UART, "%s: write UBRLCR2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_KBDEOI:
		LOGMASKED(LOG_INT, "%s: write KBDEOI  = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_int_status &= ~(1 << IRQ_KBDINT);
		check_interrupts();
		break;

	default:
		LOGMASKED(LOG_UNKN, "%s: write Unknown %04x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
		break;
	}
}


//-------------------------------------------------
//  LCD Controller
//-------------------------------------------------

uint32_t clps711x_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_syscon[0], 12)) // LCD enable
	{
		const uint32_t base_addr = 0;

		const int bpp = BIT(m_lcdcon, 30) ? (BIT(m_lcdcon, 31) ? 4 : 2) : 1;
		const int ppb = 8 / bpp;
		LOGMASKED(LOG_LCD, "bpp: %d, ppb: %d\n", bpp, ppb);

		const int width  = (BIT(m_lcdcon, 13, 6) + 1) * 16;
		const int buffer = (BIT(m_lcdcon, 0, 13) + 1) * 128;
		const int height = buffer / bpp / width;
		LOGMASKED(LOG_LCD, "buffer: %04x, width: %d height: %d\n", buffer/8, width, height);

		const pen_t *pen = screen.palette().pens();

		// build our image out
		const int line_width = (width * bpp) / 8;
		for (int y = 0; y < height; y++)
		{
			const int line_offs = line_width * y;
			uint16_t *line = &bitmap.pix(y + m_lcd_y_offset);
			for (int x = 0; x < width; x++)
			{
				const uint8_t byte = m_lcd_dma_cb(base_addr + line_offs + (x / ppb));
				const int shift = (x & (ppb - 1)) * bpp;
				const int mask  = (1 << bpp) - 1;
				const int pal_idx = (byte >> shift) & mask;

				line[x + m_lcd_x_offset] = pen[BIT(m_lcdpal, pal_idx * 4, 4)];
			}
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}
	return 0;
}
