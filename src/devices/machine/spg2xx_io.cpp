// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

/* this is used by the SPG110, SPG24x and SPG28x
   basic I/O behavior is definitely the same on the SPG110 but
   the rest needs verifying */

#include "emu.h"
#include "spg2xx_io.h"

DEFINE_DEVICE_TYPE(SPG24X_IO, spg24x_io_device, "spg24x_io", "SPG240-series System-on-a-Chip I/O")
DEFINE_DEVICE_TYPE(SPG28X_IO, spg28x_io_device, "spg28x_io", "SPG280-series System-on-a-Chip I/O")

#define LOG_IO_READS        (1U << 1)
#define LOG_IO_WRITES       (1U << 2)
#define LOG_UNKNOWN_IO      (1U << 3)
#define LOG_IRQS            (1U << 4)
#define LOG_VLINES          (1U << 5)
#define LOG_GPIO            (1U << 6)
#define LOG_UART            (1U << 7)
#define LOG_I2C             (1U << 8)
#define LOG_SEGMENT         (1U << 10)
#define LOG_WATCHDOG        (1U << 11)
#define LOG_TIMERS          (1U << 12)
#define LOG_FIQ             (1U << 25)
#define LOG_SIO             (1U << 26)
#define LOG_EXT_MEM         (1U << 27)
#define LOG_EXTINT          (1U << 28)
#define LOG_SPI             (1U << 29)
#define LOG_ADC             (1U << 30)
#define LOG_IO              (LOG_IO_READS | LOG_IO_WRITES | LOG_IRQS | LOG_GPIO | LOG_UART | LOG_I2C | LOG_TIMERS | LOG_EXTINT | LOG_UNKNOWN_IO | LOG_SPI | LOG_ADC)
#define LOG_ALL             (LOG_IO | LOG_VLINES | LOG_SEGMENT | LOG_WATCHDOG | LOG_FIQ | LOG_SIO | LOG_EXT_MEM | LOG_ADC)

#define VERBOSE             (0)
#include "logmacro.h"


#define IO_IRQ_ENABLE       m_io_regs[REG_INT_CTRL]
#define IO_IRQ_STATUS       m_io_regs[REG_INT_CLEAR]

spg2xx_io_device::spg2xx_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_porta_out(*this),
	m_portb_out(*this),
	m_portc_out(*this),
	m_porta_in(*this),
	m_portb_in(*this),
	m_portc_in(*this),
	m_adc_in(*this),
	m_i2c_w(*this),
	m_i2c_r(*this),
	m_uart_tx(*this),
	m_spi_tx(*this),
	m_chip_sel(*this),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_pal_read_cb(*this),
	m_timer_irq_cb(*this),
	m_uart_adc_irq_cb(*this),
	m_external_irq_cb(*this),
	m_ffreq_tmr1_irq_cb(*this),
	m_ffreq_tmr2_irq_cb(*this),
	m_fiq_vector_w(*this)
{
}

spg24x_io_device::spg24x_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spg2xx_io_device(mconfig, SPG24X_IO, tag, owner, clock, 256)
{
}

spg28x_io_device::spg28x_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spg2xx_io_device(mconfig, SPG28X_IO, tag, owner, clock, 64)
{
}


void spg2xx_io_device::device_start()
{
	m_porta_out.resolve_safe();
	m_portb_out.resolve_safe();
	m_portc_out.resolve_safe();
	m_porta_in.resolve_safe(0);
	m_portb_in.resolve_safe(0);
	m_portc_in.resolve_safe(0);
	m_adc_in.resolve_all_safe(0x0fff);
	m_i2c_w.resolve_safe();
	m_i2c_r.resolve_safe(0);
	m_uart_tx.resolve_safe();
	m_spi_tx.resolve_safe();
	m_chip_sel.resolve_safe();
	m_pal_read_cb.resolve_safe(0);

	m_timer_irq_cb.resolve_safe();
	m_uart_adc_irq_cb.resolve_safe();
	m_external_irq_cb.resolve_safe();
	m_ffreq_tmr1_irq_cb.resolve_safe();
	m_ffreq_tmr2_irq_cb.resolve_safe();

	m_fiq_vector_w.resolve_safe();

	m_tmb1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::tmb_timer_tick<0>), this));
	m_tmb2 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::tmb_timer_tick<1>), this));

	m_uart_tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::uart_transmit_tick), this));
	m_uart_rx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::uart_receive_tick), this));

	m_4khz_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::system_timer_tick), this));

	m_timer_src_ab = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::timer_ab_tick), this));
	m_timer_src_c = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::timer_c_tick), this));
	m_rng_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::rng_clock_tick), this));
	m_watchdog_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::watchdog_tick), this));
	m_spi_tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::spi_tx_tick), this));

	m_adc_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::adc_convert_tick<0>), this));
	m_adc_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::adc_convert_tick<1>), this));
	m_adc_timer[2] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::adc_convert_tick<2>), this));
	m_adc_timer[3] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_io_device::adc_convert_tick<3>), this));

	save_item(NAME(m_io_regs));

	save_item(NAME(m_uart_rx_fifo));
	save_item(NAME(m_uart_rx_fifo_start));
	save_item(NAME(m_uart_rx_fifo_end));
	save_item(NAME(m_uart_rx_fifo_count));
	save_item(NAME(m_uart_rx_available));
	save_item(NAME(m_uart_rx_irq));
	save_item(NAME(m_uart_tx_irq));

	save_item(NAME(m_spi_tx_fifo));
	save_item(NAME(m_spi_tx_fifo_start));
	save_item(NAME(m_spi_tx_fifo_end));
	save_item(NAME(m_spi_tx_fifo_count));
	save_item(NAME(m_spi_tx_buf));
	save_item(NAME(m_spi_tx_bit));

	save_item(NAME(m_spi_rx_fifo));
	save_item(NAME(m_spi_rx_fifo_start));
	save_item(NAME(m_spi_rx_fifo_end));
	save_item(NAME(m_spi_rx_fifo_count));
	save_item(NAME(m_spi_rx_buf));
	save_item(NAME(m_spi_rx_bit));

	save_item(NAME(m_extint));

	save_item(NAME(m_timer_a_preload));
	save_item(NAME(m_timer_b_preload));
	save_item(NAME(m_timer_b_divisor));
	save_item(NAME(m_timer_b_tick_rate));

	save_item(NAME(m_2khz_divider));
	save_item(NAME(m_1khz_divider));
	save_item(NAME(m_4hz_divider));

	save_item(NAME(m_uart_baud_rate));

	save_item(NAME(m_spi_rate));

	save_item(NAME(m_sio_bits_remaining));
	save_item(NAME(m_sio_writing));
}

void spg2xx_io_device::device_reset()
{
	std::fill_n(&m_io_regs[0], 0x100, 0);

	m_timer_a_preload = 0;
	m_timer_b_preload = 0;
	m_timer_b_divisor = 0;
	m_timer_b_tick_rate = 0;

	m_io_regs[REG_EXT_MEMORY_CTRL] = 0x0028;
	m_io_regs[REG_PRNG1] = 0x1418;
	m_io_regs[REG_PRNG2] = 0x1658;

	std::fill(std::begin(m_uart_rx_fifo), std::end(m_uart_rx_fifo), 0);
	m_uart_rx_fifo_start = 0;
	m_uart_rx_fifo_end = 0;
	m_uart_rx_fifo_count = 0;
	m_uart_rx_available = false;
	m_uart_tx_irq = false;
	m_uart_rx_irq = false;

	std::fill(std::begin(m_spi_tx_fifo), std::end(m_spi_tx_fifo), 0);
	m_spi_tx_fifo_start = 0;
	m_spi_tx_fifo_end = 0;
	m_spi_tx_fifo_count = 0;
	m_spi_tx_buf = 0x00;
	m_spi_tx_bit = 8;

	std::fill(std::begin(m_spi_rx_fifo), std::end(m_spi_rx_fifo), 0);
	m_spi_rx_fifo_start = 0;
	m_spi_rx_fifo_end = 0;
	m_spi_rx_fifo_count = 0;
	m_spi_rx_buf = 0x00;
	m_spi_rx_bit = 7;

	m_spi_rate = 0;

	std::fill_n(&m_extint[0], 2, false);

	m_4khz_timer->adjust(attotime::from_hz(4096), 0, attotime::from_hz(4096));

	m_rng_timer->adjust(attotime::from_hz(1234), 0, attotime::from_hz(1234)); // timer value is arbitrary, maybe should match system clock, but that would result in heavy switching

	m_tmb1->adjust(attotime::never);
	m_tmb2->adjust(attotime::never);
	m_uart_tx_timer->adjust(attotime::never);
	m_uart_rx_timer->adjust(attotime::never);
	m_timer_src_ab->adjust(attotime::never);
	m_timer_src_c->adjust(attotime::never);
	m_watchdog_timer->adjust(attotime::never);
	m_spi_tx_timer->adjust(attotime::never);

	for (int i = 0; i < 4; i++)
	{
		m_adc_timer[i]->adjust(attotime::never);
	}

	m_2khz_divider = 0;
	m_1khz_divider = 0;
	m_4hz_divider = 0;

	m_sio_bits_remaining = 0;
	m_sio_writing = false;
}

/*************************
*    Machine Hardware    *
*************************/

void spg2xx_io_device::uart_rx(uint8_t data)
{
	LOGMASKED(LOG_UART, "uart_rx: Pulling %02x into receive FIFO\n", data);
	if (BIT(m_io_regs[REG_UART_CTRL], 6))
	{
		m_uart_rx_fifo[m_uart_rx_fifo_end] = data;
		m_uart_rx_fifo_end = (m_uart_rx_fifo_end + 1) % std::size(m_uart_rx_fifo);
		m_uart_rx_fifo_count++;
		if (m_uart_rx_timer->remaining() == attotime::never)
			m_uart_rx_timer->adjust(attotime::from_ticks(BIT(m_io_regs[REG_UART_CTRL], 5) ? 11 : 10, m_uart_baud_rate));
	}
}

void spg2xx_io_device::set_spi_irq(bool set)
{
	const uint16_t old = IO_IRQ_STATUS;
	if (set)
	{
		LOGMASKED(LOG_SPI, "Raising SPI IRQ\n");
		IO_IRQ_STATUS |= 0x4000;
	}
	else
	{
		LOGMASKED(LOG_SPI, "Lowering SPI IRQ\n");
		IO_IRQ_STATUS &= ~0x4000;
	}

	const uint16_t changed = (old & IO_IRQ_ENABLE) ^ (IO_IRQ_STATUS & IO_IRQ_ENABLE);
	if (changed)
		check_data_irq();
}

void spg2xx_io_device::update_spi_irqs()
{
	bool ovf_set = BIT(m_io_regs[REG_SPI_RXSTATUS], 8);
	bool rxi_set = BIT(m_io_regs[REG_SPI_RXSTATUS], 15) && BIT(m_io_regs[REG_SPI_RXSTATUS], 14);
	bool txi_set = BIT(m_io_regs[REG_SPI_TXSTATUS], 15) && BIT(m_io_regs[REG_SPI_TXSTATUS], 14);
	set_spi_irq(ovf_set || rxi_set || txi_set);
}

TIMER_CALLBACK_MEMBER(spg2xx_io_device::spi_tx_tick)
{
	if (!BIT(m_io_regs[REG_SPI_CTRL], 15) || m_spi_tx_fifo_count == 0)
	{
		LOGMASKED(LOG_SPI, "Nothing to transmit or SPI disabled, bailing.\n");
		return;
	}

	if (m_spi_tx_bit == 8)
	{
		m_spi_tx_bit--;
		m_spi_tx_buf = m_spi_tx_fifo[m_spi_tx_fifo_end];
		m_spi_tx_fifo_end = (m_spi_tx_fifo_end + 1) & 0x0f;
		LOGMASKED(LOG_SPI, "Peeling off byte %02x and putting it in the Tx buffer.\n", m_spi_tx_buf);
	}

	m_spi_tx(BIT(m_spi_tx_buf, m_spi_tx_bit));

	if (m_spi_tx_bit == 0)
	{
		m_spi_tx_bit = 8;
		m_spi_tx_fifo_count--;
		LOGMASKED(LOG_SPI, "Done transmitting byte, new FIFO count is %d.\n", m_spi_tx_fifo_count);
		if (m_spi_tx_fifo_count == 0)
		{
			LOGMASKED(LOG_SPI, "Stopping Tx timer.\n");
			m_spi_tx_timer->adjust(attotime::never);
		}

		m_io_regs[REG_SPI_TXSTATUS] &= ~0x000f;
		m_io_regs[REG_SPI_TXSTATUS] |= m_spi_tx_fifo_count;

		if ((m_io_regs[REG_SPI_TXSTATUS] & 0x000f) < ((m_io_regs[REG_SPI_TXSTATUS] >> 4) & 0x000f))
		{
			LOGMASKED(LOG_SPI, "We're below the Tx IRQ threshold, flagging IRQ.\n");
			m_io_regs[REG_SPI_TXSTATUS] |= 0x8000;
			update_spi_irqs();
		}
	}
	else
	{
		m_spi_tx_bit--;
	}
}

void spg2xx_io_device::spi_rx(int state)
{
	if (!BIT(m_io_regs[REG_SPI_CTRL], 15))
	{
		LOGMASKED(LOG_SPI, "SPI Rx, but SPI is disabled, bailing.\n");
		return;
	}

	m_spi_rx_buf |= state << (m_spi_rx_bit);
	if (m_spi_rx_bit == 0)
	{
		LOGMASKED(LOG_SPI, "Done receiving byte: %02x\n", m_spi_rx_buf);
		m_spi_rx_bit = 7;
		if (m_spi_rx_fifo_count == 16)
		{
			LOGMASKED(LOG_SPI, "Rx FIFO overflow.\n");
			m_io_regs[REG_SPI_RXSTATUS] |= 0x0100; // Set RFOV flag

			update_spi_irqs();

			if (BIT(m_io_regs[REG_SPI_MISC], 9))
				m_spi_rx_fifo[(m_spi_rx_fifo_start - 1) & 0x0f] = m_spi_rx_buf;

			m_spi_rx_buf = 0x00;
			return;
		}

		m_spi_rx_fifo[m_spi_rx_fifo_start] = m_spi_rx_buf;
		m_spi_rx_fifo_start = (m_spi_rx_fifo_start + 1) & 0x0f;
		LOGMASKED(LOG_SPI, "Putting byte into Rx buffer.\n");

		m_io_regs[REG_SPI_MISC] |= 0x0004; // Set RNE flag

		if (m_spi_rx_fifo_count == 0)
			m_io_regs[REG_SPI_RXDATA] = m_spi_rx_buf;

		m_spi_rx_buf = 0x00;
		m_spi_rx_fifo_count++;
		LOGMASKED(LOG_SPI, "New Rx FIFO count: %d\n", m_spi_rx_fifo_count);

		m_io_regs[REG_SPI_RXSTATUS] &= ~0x000f;
		m_io_regs[REG_SPI_RXSTATUS] |= m_spi_rx_fifo_count; // Update RXFFLAG bits

		if (m_spi_rx_fifo_count >= ((m_io_regs[REG_SPI_RXSTATUS] >> 4) & 0x000f))
		{
			LOGMASKED(LOG_SPI, "Rx buffer is at or above threshold, flagging IRQ\n");
			m_io_regs[REG_SPI_RXSTATUS] |= 0x8000; // Set SPIRXIF
			update_spi_irqs();
		}
	}
	else
	{
		m_spi_rx_bit--;
	}
}

TIMER_CALLBACK_MEMBER(spg2xx_io_device::rng_clock_tick)
{
	clock_rng(0);
	clock_rng(1);
}

uint16_t spg2xx_io_device::clock_rng(int which)
{
	const uint16_t value = m_io_regs[REG_PRNG1 + which];
	m_io_regs[REG_PRNG1 + which] = ((value << 1) | (BIT(value, 14) ^ BIT(value, 13))) & 0x7fff;
	return value;
}



uint16_t spg2xx_io_device::io_r(offs_t offset)
{
	static const char *const gpioregs[] = { "GPIO Data Port", "GPIO Buffer Port", "GPIO Direction Port", "GPIO Attribute Port", "GPIO IRQ/Latch Port" };
	static const char gpioports[] = { 'A', 'B', 'C' };

	uint16_t val = m_io_regs[offset];

	switch (offset)
	{
	case REG_IOA_DATA: case REG_IOB_DATA: case REG_IOC_DATA:
		if (!machine().side_effects_disabled())
		{
			do_gpio(offset, false);
			LOGMASKED(LOG_GPIO, "%s: io_r: %s %c = %04x\n", machine().describe_context(), gpioregs[(offset - REG_IOA_DATA) % 5], gpioports[(offset - REG_IOA_DATA) / 5], m_io_regs[offset]);
		}
		val = m_io_regs[offset];
		break;

	case REG_IOA_BUFFER: case REG_IOA_DIR: case REG_IOA_ATTRIB: case REG_IOA_MASK:
	case REG_IOB_BUFFER: case REG_IOB_DIR: case REG_IOB_ATTRIB: case REG_IOB_MASK:
	case REG_IOC_BUFFER: case REG_IOC_DIR: case REG_IOC_ATTRIB: case REG_IOC_MASK:
		LOGMASKED(LOG_GPIO, "%s: io_r: %s %c = %04x\n", machine().describe_context(), gpioregs[(offset - REG_IOA_DATA) % 5], gpioports[(offset - REG_IOA_DATA) / 5], m_io_regs[offset]);
		break;

	case REG_TIMEBASE_SETUP:
		LOGMASKED(LOG_IO_READS, "%s: io_r: Timebase Setup = %04x\n", machine().describe_context(), val);
		break;

	case REG_TIMERA_DATA:
		LOGMASKED(LOG_IO_WRITES, "%s: io_r: Timer A Data = %04x\n", machine().describe_context(), val);
		break;

	case REG_VERT_LINE:
		val = m_screen->vpos();
		LOGMASKED(LOG_VLINES, "%s: io_r: Video Line = %04x\n", machine().describe_context(), val);
		break;

	case REG_SYSTEM_CTRL:
		LOGMASKED(LOG_IO_READS, "%s: io_r: System Control = %04x\n", machine().describe_context(), val);
		break;

	case REG_INT_CTRL:
		LOGMASKED(LOG_IRQS, "%s: io_r: I/O IRQ Control = %04x\n", machine().describe_context(), val);
		break;

	case REG_INT_CLEAR:
		LOGMASKED(LOG_IRQS, "%s: io_r: I/O IRQ Status = %04x\n", machine().describe_context(), val);
		break;

	case REG_EXT_MEMORY_CTRL:
		LOGMASKED(LOG_IO_READS, "%s: io_r: Ext. Memory Control = %04x\n", machine().describe_context(), val);
		break;

	case REG_ADC_CTRL:
		LOGMASKED(LOG_IO_READS | LOG_ADC, "%s: io_r: ADC Control = %04x\n", machine().describe_context(), val);
		break;

	case REG_ADC_PAD:
		LOGMASKED(LOG_IO_READS | LOG_ADC, "%s: io_r: ADC Pad Control = %04x\n", machine().describe_context(), val);
		break;

	case REG_ADC_DATA:
		LOGMASKED(LOG_IO_READS | LOG_ADC, "%s: io_r: ADC Data = %04x\n", machine().describe_context(), val);
		break;

	case REG_WAKEUP_SOURCE:
		LOGMASKED(LOG_IO_READS, "%s: io_r: Wakeup Source = %04x\n", machine().describe_context(), val);
		break;

	case REG_NTSC_PAL:
		val = m_pal_read_cb();
		LOGMASKED(LOG_IO_READS, "%s: io_r: NTSC/PAL = %04x\n", machine().describe_context(), val);
		break;

	case REG_PRNG1:
		if (!machine().side_effects_disabled())
			return clock_rng(0);
		return m_io_regs[REG_PRNG1];

	case REG_PRNG2:
		if (!machine().side_effects_disabled())
			return clock_rng(1);
		return m_io_regs[REG_PRNG2];

	case REG_FIQ_SEL:
		LOGMASKED(LOG_FIQ, "%s: io_r: FIQ Source Select = %04x\n", machine().describe_context(), val);
		break;

	case REG_DATA_SEGMENT:
		val = m_cpu->get_ds();
		LOGMASKED(LOG_SEGMENT, "%s: io_r: Data Segment = %04x\n", machine().describe_context(), val);
		break;

	default:
		LOGMASKED(LOG_UNKNOWN_IO, "%s: io_r: Unknown register %04x\n", machine().describe_context(), 0x3d00 + offset);
		break;
	}

	return val;
}

uint16_t spg2xx_io_device::io_extended_r(offs_t offset)
{
	// this set of registers might only be on the 24x not the 11x

	offset += 0x30;

	uint16_t val = m_io_regs[offset];

	switch (offset)
	{
	case REG_UART_CTRL:
		LOGMASKED(LOG_UART, "%s: io_r: UART Control = %04x\n", machine().describe_context(), val);
		break;

	case REG_UART_STATUS:
		LOGMASKED(LOG_UART, "%s: io_r: UART Status = %04x\n", machine().describe_context(), val);
		break;

	case REG_UART_RXBUF:
		if (!machine().side_effects_disabled())
		{
			if (m_uart_rx_available)
			{
				m_io_regs[REG_UART_STATUS] &= ~0x0081;
				LOGMASKED(LOG_UART, "UART Rx data is available, clearing bits\n");
				if (m_uart_rx_fifo_count)
				{
					LOGMASKED(LOG_UART, "%s: Remaining count %d, value %02x\n", machine().describe_context(), m_uart_rx_fifo_count, m_uart_rx_fifo[m_uart_rx_fifo_start]);
					m_io_regs[REG_UART_RXBUF] = m_uart_rx_fifo[m_uart_rx_fifo_start];
					val = m_io_regs[REG_UART_RXBUF];
					m_uart_rx_fifo_start = (m_uart_rx_fifo_start + 1) % std::size(m_uart_rx_fifo);
					m_uart_rx_fifo_count--;

					if (m_uart_rx_fifo_count == 0)
					{
						m_uart_rx_available = false;
					}
					else
					{
						LOGMASKED(LOG_UART, "Remaining count %d, setting up timer\n", m_uart_rx_fifo_count);
						if (m_uart_rx_timer->remaining() == attotime::never)
							m_uart_rx_timer->adjust(attotime::from_ticks(BIT(m_io_regs[REG_UART_CTRL], 5) ? 11 : 10, m_uart_baud_rate));
					}
				}
				else
				{
					m_uart_rx_available = false;
				}
			}
			else
			{
				m_io_regs[REG_UART_RXFIFO] |= 0x2000;
			}
			LOGMASKED(LOG_UART, "%s: io_r: UART Rx Data = %04x\n", machine().describe_context(), val);
		}
		break;

	case REG_UART_RXFIFO:
		val &= ~0x0070;
		val |= (m_uart_rx_available ? 7 : 0) << 4;
		LOGMASKED(LOG_UART, "%s: io_r: UART Rx FIFO Control = %04x\n", machine().describe_context(), val);
		break;

	case REG_SPI_CTRL:
		LOGMASKED(LOG_SPI, "%s: io_r: SPI Control = %04x\n", machine().describe_context(), val);
		break;

	case REG_SPI_TXSTATUS:
		LOGMASKED(LOG_SPI, "%s: io_r: SPI Tx Status = %04x\n", machine().describe_context(), val);
		break;

	case REG_SPI_TXDATA:
		LOGMASKED(LOG_SPI, "%s: io_r: SPI Tx Data = %04x\n", machine().describe_context(), val);
		break;

	case REG_SPI_RXSTATUS:
		LOGMASKED(LOG_SPI, "%s: io_r: SPI Rx Status = %04x\n", machine().describe_context(), val);
		break;

	case REG_SPI_RXDATA:
		LOGMASKED(LOG_SPI, "%s: io_r: SPI Rx Data = %04x, FIFO count %d\n", machine().describe_context(), val, m_spi_rx_fifo_count);
		if (m_spi_rx_fifo_count > 0 && !machine().side_effects_disabled())
		{
			m_spi_rx_fifo_count--;
			if (m_spi_rx_fifo_count > 0)
			{
				m_spi_rx_fifo_end = (m_spi_rx_fifo_end + 1) & 0x0f;
				m_io_regs[REG_SPI_RXDATA] = m_spi_rx_fifo[m_spi_rx_fifo_end];
			}

			m_io_regs[REG_SPI_RXSTATUS] &= ~(0x0200); // Clear RXFULL
			m_io_regs[REG_SPI_MISC] &= ~(0x0008); // Clear RFF
			if (m_spi_rx_fifo_count == 0)
			{
				m_io_regs[REG_SPI_MISC] &= ~(0x0004); // Clear RNE
			}
		}
		break;

	case REG_SPI_MISC:
		LOGMASKED(LOG_SPI, "%s: io_r: SPI Misc. = %04x\n", machine().describe_context(), val);
		break;

	case REG_SIO_SETUP:
		LOGMASKED(LOG_SIO, "%s: io_r: SIO Setup = %04x\n", machine().describe_context(), val);
		break;

	case REG_SIO_STATUS:
		LOGMASKED(LOG_SIO, "%s: io_r: SIO Status = %04x\n", machine().describe_context(), val);
		break;

	case REG_SIO_DATA:
		LOGMASKED(LOG_SIO, "%s: io_r: SIO Data = %04x\n", machine().describe_context(), val);
		if ((m_io_regs[REG_SIO_STATUS] & 0x8000) && !m_sio_writing && !machine().side_effects_disabled())
		{
			m_sio_bits_remaining--;
			if (m_sio_bits_remaining == 0)
			{
				m_io_regs[REG_SIO_STATUS] &= ~0x8000;
			}
		}
		break;

	case REG_I2C_CMD:
		LOGMASKED(LOG_I2C, "%s: io_r: I2C Command = %04x\n", machine().describe_context(), val);
		break;

	case REG_I2C_STATUS:
		LOGMASKED(LOG_I2C, "%s: io_r: I2C Status = %04x\n", machine().describe_context(), val);
		break;

	case REG_I2C_DATA_IN: // I2C Data In
		LOGMASKED(LOG_I2C, "%s: io_r: I2C Data In = %04x\n", machine().describe_context(), val);
		break;

	default:
		LOGMASKED(LOG_UNKNOWN_IO, "%s: io_r: Unknown register %04x\n", machine().describe_context(), 0x3d00 + offset);
		break;
	}

	return val;
}

void spg2xx_io_device::update_porta_special_modes()
{
	static const char* const s_pa_special[4][16] =
	{
		// Input,  Special 0
		// Input,  Special 1
		// Output, Special 0
		// Output, Special 1

		{ "LP",   "ExtClk2", "ExtClk1", "-",   "SDA", "SlvRDY", "-",     "-",       "SPICLK", "-",   "RxD", "SPISSB", "-",     "-",     "-",     "-"     },
		{ "-",    "-",       "-",       "SCK", "-",   "SWS",    "-",     "-",       "-",      "-",   "-",   "-",      "IRQ2B", "-",     "-",     "IRQ1B" },
		{ "-",    "-",       "-",       "SCK", "SDA", "SWS",    "-",     "-",       "SPICLK", "TxD", "-",   "SPISSB", "TAPWM", "TM1",   "TBPWM", "TM2"   },
		{ "CSB3", "CSB2",    "CSB1",    "SCK", "SDA", "VSYNC",  "HSYNC", "SYSCLK3", "SPICLK", "TxD", "SWS", "SPISSB", "-",     "VSYNC", "HSYNC", "CSYNC" },
	};
	for (int bit = 15; bit >= 0; bit--)
	{
		if (!BIT(m_io_regs[REG_IOA_MASK], bit))
			continue;
		uint8_t type = (BIT(m_io_regs[REG_IOA_DIR], bit) << 1) | BIT(m_io_regs[REG_IO_MODE], 0);
		LOGMASKED(LOG_GPIO, "      Bit %2d: %s\n", bit, s_pa_special[type][bit]);
	}
}

void spg2xx_io_device::update_portb_special_modes()
{
	static const char* const s_pb_special[4][8] =
	{
		// Input,  Special 0
		// Input,  Special 1
		// Output, Special 0
		// Output, Special 1

		{ "-",    "-",      "-",     "-",     "-",   "-",   "SDA", "SlvRDY"  },
		{ "-",    "-",      "-",     "-",     "-",   "-",   "SDA", "SlvRDY"  },
		{ "VSYNC", "HSYNC", "CSYNC", "-",     "-",   "SCK", "SDA", "SWS"     },
		{ "CSB3",  "CSB2",  "CSB1",  "TBPWM", "TM2", "-",   "-",   "SYSCLK2" },
	};
	for (int bit = 7; bit >= 0; bit--)
	{
		if (!BIT(m_io_regs[REG_IOB_MASK], bit))
			continue;
		uint8_t type = (BIT(m_io_regs[REG_IOB_DIR], bit) << 1) | BIT(m_io_regs[REG_IO_MODE], 1);
		LOGMASKED(LOG_GPIO, "      Bit %2d: %s\n", bit, s_pb_special[type][bit]);
	}
}

void spg2xx_io_device::update_timer_b_rate()
{
	switch (m_io_regs[REG_TIMERB_CTRL] & 7)
	{
		case 0:
		case 1:
		case 5:
		case 6:
		case 7:
			m_timer_src_c->adjust(attotime::never);
			break;
		case 2:
			m_timer_src_c->adjust(attotime::from_hz(32768), 0, attotime::from_hz(32768));
			break;
		case 3:
			m_timer_src_c->adjust(attotime::from_hz(8192), 0, attotime::from_hz(8192));
			break;
		case 4:
			m_timer_src_c->adjust(attotime::from_hz(4096), 0, attotime::from_hz(4096));
			break;
	}
}

TIMER_CALLBACK_MEMBER(spg2xx_io_device::timer_ab_tick)
{
	if (m_timer_b_tick_rate == 0)
		return;

	m_timer_b_divisor++;
	if (m_timer_b_divisor >= m_timer_b_tick_rate)
	{
		m_timer_b_divisor = 0;
		increment_timer_a();
	}
}

void spg2xx_io_device::increment_timer_a()
{
	m_io_regs[REG_TIMERA_DATA]++;
	if (m_io_regs[REG_TIMERA_DATA] == 0)
	{
		m_io_regs[REG_TIMERA_DATA] = m_timer_a_preload;
		const uint16_t old = IO_IRQ_STATUS;
		IO_IRQ_STATUS |= 0x0800;
		const uint16_t changed = (old & IO_IRQ_ENABLE) ^ (IO_IRQ_STATUS & IO_IRQ_ENABLE);
		if (changed)
			check_timers_irq();
	}
}

TIMER_CALLBACK_MEMBER(spg2xx_io_device::timer_c_tick)
{
	m_io_regs[REG_TIMERB_DATA]++;
	if (m_io_regs[REG_TIMERB_DATA] == 0)
	{
		m_io_regs[REG_TIMERB_DATA] = m_timer_b_preload;
		const uint16_t old = IO_IRQ_STATUS;
		IO_IRQ_STATUS |= 0x0400;
		const uint16_t changed = (old & IO_IRQ_ENABLE) ^ (IO_IRQ_STATUS & IO_IRQ_ENABLE);
		if (changed)
			check_timers_irq();
	}
}


void spg28x_io_device::io_extended_w(offs_t offset, uint16_t data)
{
	offset += REG_UART_CTRL;

	if (offset == REG_UART_BAUD1)
	{
		m_io_regs[offset] = data;
		m_uart_baud_rate = 27000000 / (0x10000 - m_io_regs[REG_UART_BAUD1]);
		LOGMASKED(LOG_UART, "%s: io_w: UART Baud Rate scaler = %04x (%d baud)\n", machine().describe_context(), data, m_uart_baud_rate);
	}
	else
	{
		spg2xx_io_device::io_extended_w(offset - REG_UART_CTRL, data);
	}
}

void spg2xx_io_device::io_w(offs_t offset, uint16_t data)
{
	static const char *const gpioregs[] = { "GPIO Data Port", "GPIO Buffer Port", "GPIO Direction Port", "GPIO Attribute Port", "GPIO IRQ/Latch Port" };
	static const char gpioports[3] = { 'A', 'B', 'C' };

	switch (offset)
	{
	case REG_IO_MODE:
	{
		LOGMASKED(LOG_GPIO, "%s: io_w: GPIO Configuration = %04x (IOBWake:%d, IOAWake:%d, IOBSpecSel:%d, IOASpecSel:%d)\n", machine().describe_context(), data
			, BIT(data, 4), BIT(data, 3), BIT(data, 1), BIT(data, 0));
		const uint16_t old = m_io_regs[REG_IO_MODE];
		m_io_regs[REG_IO_MODE] = data;
		const uint16_t changed = old ^ data;
		if (BIT(changed, 0))
			update_porta_special_modes();
		if (BIT(changed, 1))
			update_portb_special_modes();
		break;
	}

	case REG_IOA_DATA: case REG_IOB_DATA: case REG_IOC_DATA:
		offset++;
		[[fallthrough]]; // we redirect data register writes to the buffer register.

	case REG_IOA_BUFFER: case REG_IOA_ATTRIB:
	case REG_IOB_BUFFER: case REG_IOB_ATTRIB:
	case REG_IOC_BUFFER: case REG_IOC_DIR: case REG_IOC_ATTRIB: case REG_IOC_MASK:
		LOGMASKED(LOG_GPIO, "%s: io_w: %s %c = %04x\n", machine().describe_context(), gpioregs[(offset - REG_IOA_DATA) % 5], gpioports[(offset - REG_IOA_DATA) / 5], data);
		m_io_regs[offset] = data;
		do_gpio(offset, true);
		break;

	case REG_IOA_DIR:
		LOGMASKED(LOG_GPIO, "%s: io_w: GPIO Direction Port A = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_IOA_DIR] = data;
		update_porta_special_modes();
		do_gpio(offset, true);
		break;

	case REG_IOB_DIR:
		LOGMASKED(LOG_GPIO, "%s: io_w: GPIO Direction Port B = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_IOB_DIR] = data;
		update_portb_special_modes();
		do_gpio(offset, true);
		break;

	case REG_IOA_MASK:
		LOGMASKED(LOG_GPIO, "%s: io_w: Port A Special Function Select: %04x\n", machine().describe_context(), data);
		m_io_regs[REG_IOA_MASK] = data;
		update_porta_special_modes();
		break;

	case REG_IOB_MASK:
		LOGMASKED(LOG_GPIO, "%s: io_w: Port B Special Function Select: %04x\n", machine().describe_context(), data);
		m_io_regs[REG_IOB_MASK] = data;
		update_portb_special_modes();
		break;

	case REG_TIMEBASE_SETUP:
	{
		static const char* const s_tmb1_sel[2][4] =
		{
			{ "8Hz", "16Hz", "32Hz", "64Hz" },
			{ "12kHz", "24kHz", "40kHz", "40kHz" }
		};
		static const char* const s_tmb2_sel[2][4] =
		{
			{ "128Hz", "256Hz", "512Hz", "1024Hz" },
			{ "105kHz", "210kHz", "420kHz", "840kHz" }
		};
		static const uint32_t s_tmb1_freq[2][4] =
		{
			{ 8, 16, 32, 64 },
			{ 12000, 24000, 40000, 40000 }
		};
		static const uint32_t s_tmb2_freq[2][4] =
		{
			{ 128, 256, 512, 1024 },
			{ 105000, 210000, 420000, 840000 }
		};
		LOGMASKED(LOG_TIMERS, "%s: io_w: Timebase Control = %04x (Source:%s, TMB2:%s, TMB1:%s)\n", machine().describe_context(), data,
			BIT(data, 4) ? "27MHz" : "32768Hz", s_tmb2_sel[BIT(data, 4)][(data >> 2) & 3], s_tmb1_sel[BIT(data, 4)][data & 3]);
		m_io_regs[REG_TIMEBASE_SETUP] = data;
		const uint8_t hifreq = BIT(data, 4);
		const uint32_t tmb1freq = s_tmb1_freq[hifreq][data & 3];
		m_tmb1->adjust(attotime::from_hz(tmb1freq), 0, attotime::from_hz(tmb1freq));
		const uint32_t tmb2freq = s_tmb2_freq[hifreq][(data >> 2) & 3];
		m_tmb2->adjust(attotime::from_hz(tmb2freq), 0, attotime::from_hz(tmb2freq));
		break;
	}

	case REG_TIMEBASE_CLEAR:
		LOGMASKED(LOG_TIMERS, "%s: io_w: Timebase Clear = %04x\n", machine().describe_context(), data);
		m_2khz_divider = 0;
		m_1khz_divider = 0;
		m_4hz_divider = 0;
		break;

	case REG_TIMERA_DATA:
		LOGMASKED(LOG_TIMERS, "%s: io_w: Timer A Data = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_TIMERA_DATA] = data;
		m_timer_a_preload = data;
		break;

	case REG_TIMERA_CTRL:
	{
		static const char* const s_source_a[8] = { "0", "0", "32768Hz", "8192Hz", "4096Hz", "1", "0", "ExtClk1" };
		static const char* const s_source_b[8] = { "2048Hz", "1024Hz", "256Hz", "TMB1", "4Hz", "2Hz", "1", "ExtClk2" };
		LOGMASKED(LOG_TIMERS, "%s: io_w: Timer A Control = %04x (Source A:%s, Source B:%s)\n", machine().describe_context(), data,
			s_source_a[data & 7], s_source_b[(data >> 3) & 7]);
		m_io_regs[REG_TIMERA_CTRL] = data;
		int timer_a_rate = 0;
		switch (data & 7)
		{
			case 0:
			case 1:
			case 5:
			case 6:
			case 7:
				m_timer_src_ab->adjust(attotime::never);
				break;
			case 2:
				m_timer_src_ab->adjust(attotime::from_hz(32768), 0, attotime::from_hz(32768));
				timer_a_rate = 32768;
				break;
			case 3:
				m_timer_src_ab->adjust(attotime::from_hz(8192), 0, attotime::from_hz(8192));
				timer_a_rate = 8192;
				break;
			case 4:
				m_timer_src_ab->adjust(attotime::from_hz(4096), 0, attotime::from_hz(4096));
				timer_a_rate = 4096;
				break;
		}
		switch ((data >> 3) & 7)
		{
			case 0:
				m_timer_b_tick_rate = timer_a_rate / 2048;
				break;
			case 1:
				m_timer_b_tick_rate = timer_a_rate / 1024;
				break;
			case 2:
				m_timer_b_tick_rate = timer_a_rate / 256;
				break;
			case 3:
				m_timer_b_tick_rate = 0;
				break;
			case 4:
				m_timer_b_tick_rate = timer_a_rate / 4;
				break;
			case 5:
				m_timer_b_tick_rate = timer_a_rate / 2;
				break;
			case 6:
				m_timer_b_tick_rate = 1;
				break;
			case 7:
				m_timer_b_tick_rate = 0;
				break;
		}
		break;
	}

	case REG_TIMERA_IRQCLR:
	{
		LOGMASKED(LOG_TIMERS, "%s: io_w: Timer A IRQ Clear\n", machine().describe_context());
		const uint16_t old = IO_IRQ_STATUS;
		IO_IRQ_STATUS &= ~0x0800;
		const uint16_t changed = (old & IO_IRQ_ENABLE) ^ (IO_IRQ_STATUS & IO_IRQ_ENABLE);
		if (changed)
			check_timers_irq();
		break;
	}

	case REG_TIMERB_DATA:
		LOGMASKED(LOG_TIMERS, "%s: io_w: Timer B Data = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_TIMERB_DATA] = data;
		m_timer_b_preload = data;
		break;

	case REG_TIMERB_CTRL:
	{
		static const char* const s_source_c[8] = { "0", "0", "32768Hz", "8192Hz", "4096Hz", "1", "0", "ExtClk1" };
		LOGMASKED(LOG_TIMERS, "%s: io_w: Timer B Control = %04x (Source C:%s)\n", machine().describe_context(), data, s_source_c[data & 7]);
		m_io_regs[REG_TIMERB_CTRL] = data;
		if (m_io_regs[REG_TIMERB_ON] == 1)
		{
			update_timer_b_rate();
		}
		break;
	}

	case REG_TIMERB_ON:
	{
		LOGMASKED(LOG_TIMERS, "%s: io_w: Timer B Enable = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_TIMERB_ON] = data & 1;
		if (data & 1)
		{
			update_timer_b_rate();
		}
		else
		{
			m_timer_src_c->adjust(attotime::never);
		}
		break;
	}

	case REG_TIMERB_IRQCLR:
	{
		LOGMASKED(LOG_TIMERS, "%s: io_w: Timer B IRQ Clear\n", machine().describe_context());
		const uint16_t old = IO_IRQ_STATUS;
		IO_IRQ_STATUS &= ~0x0400;
		const uint16_t changed = (old & IO_IRQ_ENABLE) ^ (IO_IRQ_STATUS & IO_IRQ_ENABLE);
		if (changed)
			check_timers_irq();
		break;
	}

	case REG_SYSTEM_CTRL:
	{
		static const char* const s_sysclk[4] = { "13.5MHz", "27MHz", "27MHz NoICE", "54MHz" };
		static const char* const s_lvd_voltage[4] = { "2.7V", "2.9V", "3.1V", "3.3V" };
		static const char* const s_weak_strong[2] = { "Weak", "Strong" };
		LOGMASKED(LOG_IO_WRITES, "%s: io_w: System Control = %04x (Watchdog:%d, Sleep:%d, SysClk:%s, SysClkInv:%d, LVROutEn:%d, LVREn:%d\n", machine().describe_context()
			, data, BIT(data, 15), BIT(data, 14), s_sysclk[(data >> 12) & 3], BIT(data, 11), BIT(data, 9), BIT(data, 8));
		LOGMASKED(LOG_IO_WRITES, "      LVDEn:%d, LVDVoltSel:%s, 32kHzDisable:%d, StrWkMode:%s, VDACDisable:%d, ADACDisable:%d, ADACOutDisable:%d)\n"
			, BIT(data, 7), s_lvd_voltage[(data >> 5) & 3], BIT(data, 4), s_weak_strong[BIT(data, 3)], BIT(data, 2), BIT(data, 1), BIT(data, 0));
		const uint16_t old = m_io_regs[REG_SYSTEM_CTRL];
		m_io_regs[REG_SYSTEM_CTRL] = data;
		if (BIT(old, 15) != BIT(data, 15))
		{
			if (BIT(data, 15))
				m_watchdog_timer->adjust(attotime::from_msec(750));
			else
				m_watchdog_timer->adjust(attotime::never);
		}
		break;
	}

	case REG_INT_CTRL:
	{
		LOGMASKED(LOG_IRQS, "%s: io_w: IRQ Enable = %04x\n", machine().describe_context(), data);
		const uint16_t old = IO_IRQ_ENABLE;
		m_io_regs[REG_INT_CTRL] = data;
		const uint16_t changed = (IO_IRQ_STATUS & old) ^ (IO_IRQ_STATUS & IO_IRQ_ENABLE);
		if (changed)
			check_all_irqs(changed);
		break;
	}

	case REG_INT_CLEAR:
	{
		LOGMASKED(LOG_IRQS, "%s: io_w: IRQ Acknowledge = %04x\n", machine().describe_context(), data);
		const uint16_t old = IO_IRQ_STATUS;
		IO_IRQ_STATUS &= ~data;
		const uint16_t changed = (old & IO_IRQ_ENABLE) ^ (IO_IRQ_STATUS & IO_IRQ_ENABLE);
		if (m_uart_rx_irq || m_uart_tx_irq)
		{
			LOGMASKED(LOG_IRQS | LOG_UART, "Re-setting UART IRQ due to still-unacknowledged Rx or Tx.\n");
			IO_IRQ_STATUS |= 0x0100;
		}
		if (changed)
			check_all_irqs(changed);
		break;
	}

	case REG_EXT_MEMORY_CTRL:
	{
		static const char* const s_bus_arb[8] =
		{
			"Forbidden", "Forbidden", "Forbidden", "Forbidden", "Forbidden", "1:SPU/2:PPU/3:CPU", "Forbidden", "1:PPU/2:SPU/3:CPU"
		};
		static const char* const s_addr_decode[4] =
		{
			"ROMCSB: 4000-3fffff, CSB1: ---,           CSB2: ---,           CSB3: ---",
			"ROMCSB: 4000-1fffff, CSB1: 200000-3fffff, CSB2: ---,           CSB3: ---",
			"ROMCSB: 4000-0fffff, CSB1: 100000-1fffff, CSB2: 200000-2fffff, CSB3: 300000-3fffff",
			"ROMCSB: 4000-0fffff, CSB1: 100000-1fffff, CSB2: 200000-2fffff, CSB3: 300000-3fffff"
		};
		static const char* const s_ram_decode[16] =
		{
			"None", "None", "None", "None", "None", "None", "None", "None",
			"4KW,   3ff000-3fffff\n",
			"8KW,   3fe000-3fffff\n",
			"16KW,  3fc000-3fffff\n",
			"32KW,  3f8000-3fffff\n",
			"64KW,  3f0000-3fffff\n",
			"128KW, 3e0000-3fffff\n",
			"256KW, 3c0000-3fffff\n",
			"512KW, 380000-3fffff\n"
		};
		LOGMASKED(LOG_EXT_MEM, "%s: io_w: Ext. Memory Control (not yet implemented) = %04x:\n", machine().describe_context(), data);
		LOGMASKED(LOG_EXT_MEM, "      WaitStates:%d, BusArbPrio:%s\n", (data >> 1) & 3, s_bus_arb[(data >> 3) & 7]);
		LOGMASKED(LOG_EXT_MEM, "      ROMAddrDecode:%s\n", s_addr_decode[(data >> 6) & 3]);
		LOGMASKED(LOG_EXT_MEM, "      RAMAddrDecode:%s\n", s_ram_decode[(data >> 8) & 15]);
		m_chip_sel((data >> 6) & 3);
		m_io_regs[REG_EXT_MEMORY_CTRL] = data;
		break;
	}

	case REG_WATCHDOG_CLEAR:
		LOGMASKED(LOG_WATCHDOG, "%s: io_w: Watchdog Clear = %04x\n", machine().describe_context(), data);
		if (data == 0x55aa && BIT(m_io_regs[REG_SYSTEM_CTRL], 15))
		{
			m_watchdog_timer->adjust(attotime::from_msec(750));
		}
		break;

	case REG_ADC_CTRL:
	{
		LOGMASKED(LOG_IO_WRITES | LOG_ADC, "%s: io_w: ADC Control = %04x\n", machine().describe_context(), data);

		const uint16_t old_ctrl = m_io_regs[REG_ADC_CTRL];
		m_io_regs[REG_ADC_CTRL] = data & ~0x2000;

		if (BIT(old_ctrl, 13) && BIT(data, 13))
		{
			m_io_regs[REG_ADC_CTRL] &= ~0x2000;
			IO_IRQ_STATUS &= ~0x2000;
			check_data_irq();
		}

		if (BIT(m_io_regs[REG_ADC_CTRL], 0))
		{
			// Assume ready unless stated otherwise (i.e., conversion request is issued).
			m_io_regs[REG_ADC_CTRL] |= 0x2000;

			const uint16_t adc_channel = (m_io_regs[REG_ADC_CTRL] >> 4) & 3;
			if (!BIT(old_ctrl, 12) && BIT(m_io_regs[REG_ADC_CTRL], 12))
			{
				m_io_regs[REG_ADC_CTRL] &= ~0x3000;
				const uint32_t adc_clocks = 16 << ((m_io_regs[REG_ADC_CTRL] >> 2) & 3);
				m_adc_timer[adc_channel]->adjust(attotime::from_ticks(adc_clocks, 27000000));
				m_io_regs[REG_ADC_DATA] &= ~0x8000;
			}

			// Req_Auto_8K
			if (BIT(data, 10))
			{
				m_io_regs[REG_ADC_DATA] &= ~0x8000;
				m_adc_timer[adc_channel]->adjust(attotime::from_hz(8000), 0, attotime::from_hz(8000));
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				m_adc_timer[i]->adjust(attotime::never);
			}
		}
		break;
	}

	case REG_ADC_PAD:
		LOGMASKED(LOG_IO_WRITES | LOG_ADC, "%s: io_w: ADC Pad Control = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_ADC_PAD] = data;
		for (int i = 0; i < 4; i++)
		{
			if (!BIT(data, i))
			{
				m_adc_timer[i]->adjust(attotime::never);
			}
		}
		break;

	case REG_SLEEP_MODE:
		LOGMASKED(LOG_IO_WRITES, "%s: io_w: Sleep Mode (%s enter value) = %04x\n", machine().describe_context(), data == 0xaa55 ? "valid" : "invalid", data);
		m_io_regs[REG_SLEEP_MODE] = data;
		break;

	case 0x29: // Wakeup Source
	{
		m_io_regs[REG_WAKEUP_SOURCE] = data;
		static const char* const s_sources[8] =
		{
			"TMB1", "TMB2", "2Hz", "4Hz", "1024Hz", "2048Hz", "4096Hz", "Key"
		};

		LOGMASKED(LOG_IO_WRITES, "%s: io_w: Wakeup Source = %04x:\n", machine().describe_context(), data);
		bool comma = false;
		char buf[1024];
		int char_idx = 0;
		for (int i = 7; i >= 0; i--)
		{
			if (BIT(data, i))
			{
				char_idx += sprintf(&buf[char_idx], "%s%s", comma ? ", " : "", s_sources[i]);
				comma = true;
			}
		}
		buf[char_idx] = 0;
		LOGMASKED(LOG_IO_WRITES, "      %s\n", buf);
		break;
	}

	case REG_PRNG1:
		LOGMASKED(LOG_IO_WRITES, "%s: io_w: PRNG 0 seed = %04x\n", machine().describe_context(), data & 0x7fff);
		m_io_regs[REG_PRNG1] = data & 0x7fff;
		break;

	case REG_PRNG2:
		LOGMASKED(LOG_IO_WRITES, "%s: io_w: PRNG 1 seed = %04x\n", machine().describe_context(), data & 0x7fff);
		m_io_regs[REG_PRNG2] = data & 0x7fff;
		break;

	case REG_FIQ_SEL:
	{
		static const char* const s_fiq_select[8] =
		{
			"PPU", "SPU Channel", "Timer A", "Timer B", "UART/SPI", "External", "Reserved", "None"
		};
		LOGMASKED(LOG_FIQ, "%s: io_w: FIQ Source Select (not yet implemented) = %04x, %s\n", machine().describe_context(), data, s_fiq_select[data & 7]);
		m_io_regs[REG_FIQ_SEL] = data;
		m_fiq_vector_w(data & 7);
		break;
	}

	case REG_DATA_SEGMENT:
		m_cpu->set_ds(data & 0x3f);
		LOGMASKED(LOG_SEGMENT, "%s: io_w: Data Segment = %04x\n", machine().describe_context(), data);
		break;

	default:
		LOGMASKED(LOG_UNKNOWN_IO, "%s: io_w: Unknown register %04x = %04x\n", machine().describe_context(), 0x3d00 + offset, data);
		m_io_regs[offset] = data;
		break;
	}
}




void spg2xx_io_device::io_extended_w(offs_t offset, uint16_t data)
{
	// this set of registers might only be on the 24x not the 11x

	offset += REG_UART_CTRL;

	switch (offset)
	{

	case REG_UART_CTRL: // UART Control
	{
		static const char* const s_9th_bit[4] = { "0", "1", "Odd", "Even" };
		LOGMASKED(LOG_UART, "%s: io_w: UART Control = %04x (TxEn:%d, RxEn:%d, Bits:%d, MultiProc:%d, 9thBit:%s, TxIntEn:%d, RxIntEn:%d\n",
			machine().describe_context(), data, BIT(data, 7), BIT(data, 6), BIT(data, 5) ? 9 : 8, BIT(data, 4), s_9th_bit[(data >> 2) & 3],
			BIT(data, 1), BIT(data, 0));
		const uint16_t changed = m_io_regs[REG_UART_CTRL] ^ data;
		m_io_regs[REG_UART_CTRL] = data;
		if (!BIT(data, 6))
		{
			m_uart_rx_available = false;
			m_io_regs[REG_UART_RXBUF] = 0;
		}
		if (BIT(changed, 7))
		{
			if (BIT(data, 7))
			{
				m_io_regs[REG_UART_STATUS] |= 0x0002;
			}
			else
			{
				m_io_regs[REG_UART_STATUS] &= ~0x0042;
				m_uart_tx_timer->adjust(attotime::never);
			}
		}
		break;
	}

	case REG_UART_STATUS:
		LOGMASKED(LOG_UART, "%s: io_w: UART Status = %04x\n", machine().describe_context(), data);
		if (BIT(data, 0))
		{
			m_io_regs[REG_UART_STATUS] &= ~1;
			m_uart_rx_irq = false;
		}
		if (BIT(data, 1))
		{
			m_io_regs[REG_UART_STATUS] &= ~2;
			m_uart_tx_irq = false;
		}
		if (!m_uart_rx_irq && !m_uart_tx_irq)
		{
			const uint16_t old = IO_IRQ_STATUS;
			IO_IRQ_STATUS &= ~0x0100;
			const uint16_t changed = (old & IO_IRQ_ENABLE) ^ (IO_IRQ_STATUS & IO_IRQ_ENABLE);
			if (changed)
				check_data_irq();
		}
		break;

	case REG_UART_BAUD1: // (low byte)
	case REG_UART_BAUD2: // (high byte)
	{
		m_io_regs[offset] = data;
		const uint32_t divisor = 16 * (0x10000 - ((m_io_regs[REG_UART_BAUD2] << 8) | m_io_regs[REG_UART_BAUD1]));
		LOGMASKED(LOG_UART, "%s: io_w: UART Baud Rate (%s byte): Baud rate = %d\n", offset == 0x33 ? "low" : "high", machine().describe_context(), 27000000 / divisor);
		m_uart_baud_rate = 27000000 / divisor;
		break;
	}

	case REG_UART_TXBUF:
		LOGMASKED(LOG_UART, "%s: io_w: UART Tx Data = %02x\n", machine().describe_context(), data & 0x00ff);
		m_io_regs[REG_UART_TXBUF] = data;
		if (BIT(m_io_regs[REG_UART_CTRL], 7))
		{
			LOGMASKED(LOG_UART, "io_w: UART Tx: Clearing ready bit, setting busy bit, setting up timer\n");
			m_uart_tx_timer->adjust(attotime::from_ticks(BIT(m_io_regs[REG_UART_CTRL], 5) ? 11 : 10, m_uart_baud_rate));
			m_io_regs[REG_UART_STATUS] &= ~0x0002;
			m_io_regs[REG_UART_STATUS] |= 0x0040;
		}
		break;

	case REG_UART_RXBUF:
		LOGMASKED(LOG_UART, "%s: io_w: UART Rx Data (read-only) = %04x\n", machine().describe_context(), data);
		break;

	case REG_UART_RXFIFO:
		LOGMASKED(LOG_UART, "%s: io_w: UART Rx FIFO Control = %04x (Reset:%d, Overrun:%d, Underrun:%d, Count:%d, Threshold:%d)\n",
			machine().describe_context(), data, BIT(data, 15), BIT(data, 14), BIT(data, 13), (data >> 4) & 7, data & 7);
		if (data & 0x8000)
		{
			m_uart_rx_available = false;
			m_io_regs[REG_UART_RXBUF] = 0;
		}
		m_io_regs[REG_UART_RXFIFO] &= ~data & 0x6000;
		m_io_regs[REG_UART_RXFIFO] &= ~0x0007;
		m_io_regs[REG_UART_RXFIFO] |= data & 0x0007;
		break;

	case REG_SPI_CTRL:
	{
		static const char* const s_spi_clock[8] = { "SYSCLK/2" , "SYSCLK/4", "SYSCLK/8", "SYSCLK/16", "SYSCLK/32", "SYSCLK/64", "SYSCLK/128", "Reserved" };
		LOGMASKED(LOG_SPI, "%s: io_w: SPI Control = %04x (Enable:%d, Loopback:%d, Reset:%d, Mode:%s, Phase:%d, Polarity:%d, Clock:%s)\n", machine().describe_context(), data,
			BIT(data, 15), BIT(data, 13), BIT(data, 11), BIT(data, 8) ? "Slave" : "Master", BIT(data, 5), BIT(data, 4), s_spi_clock[data & 7]);
		m_io_regs[offset] = data;
		if ((data & 7) == 7 || !BIT(m_io_regs[offset], 15))
		{
			m_spi_rate = 0;
			m_spi_tx_timer->adjust(attotime::never);
		}
		else
		{
			m_spi_rate = 2 << (data & 7);
			attotime rate = attotime::from_ticks(m_spi_rate, clock());
			if (m_spi_tx_timer->remaining() != attotime::never)
				m_spi_tx_timer->adjust(rate);
		}
		break;
	}

	case REG_SPI_TXSTATUS:
		LOGMASKED(LOG_SPI, "%s: io_w: SPI Tx Status = %04x\n", machine().describe_context(), data);
		m_io_regs[offset] &= ~0x40f0;
		m_io_regs[offset] |= data & 0x40f0;
		if (BIT(data, 15))
		{
			m_io_regs[offset] &= ~0x8000;
			update_spi_irqs();
		}
		break;

	case REG_SPI_TXDATA:
		LOGMASKED(LOG_SPI, "%s: io_w: SPI Tx Data = %04x\n", machine().describe_context(), data);
		m_io_regs[offset] = data;
		if (BIT(m_io_regs[REG_SPI_CTRL], 15))
		{
			if (m_spi_tx_fifo_count < 16)
			{
				LOGMASKED(LOG_SPI, "%s: SPI Tx FIFO is full, pushing onto FIFO and enabling timer.\n", machine().describe_context());
				m_spi_tx_fifo[m_spi_tx_fifo_start] = (uint8_t)m_io_regs[REG_SPI_TXDATA];
				m_spi_tx_fifo_start = (m_spi_tx_fifo_start + 1) & 0x0f;
				m_spi_tx_fifo_count++;

				attotime rate = attotime::from_ticks(m_spi_rate, clock());
				m_spi_tx_timer->adjust(rate, 0, rate);
			}
			else
			{
				LOGMASKED(LOG_SPI, "%s: SPI Tx FIFO is full, not pushing.\n", machine().describe_context());
			}
		}
		break;

	case REG_SPI_RXSTATUS:
		LOGMASKED(LOG_SPI, "%s: io_w: SPI Rx Status = %04x\n", machine().describe_context(), data);
		m_io_regs[offset] &= ~0x40f0;
		m_io_regs[offset] |= data & 0x40f0;
		m_io_regs[offset] &= ~0x0100; // Clear RXFOV
		if (BIT(data, 15))
		{
			m_io_regs[offset] &= ~0x8000;
			update_spi_irqs();
		}
		break;

	case REG_SPI_RXDATA:
		LOGMASKED(LOG_SPI, "%s: io_w: SPI Rx Data = %04x\n", machine().describe_context(), data);
		break;

	case REG_SPI_MISC:
	{
		LOGMASKED(LOG_SPI, "%s: io_w: SPI Misc. = %04x (Over:%d, SmartInt:%d, Busy:%d, RxFull:%d, RxNotEmpty:%d, TxNotFull:%d, TxEmpty:%d)\n", machine().describe_context(), data,
			BIT(data, 9), BIT(data, 8), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
		m_io_regs[offset] &= ~0x0300;
		m_io_regs[offset] |= data & 0x0300;
		break;
	}

	case REG_SIO_SETUP:
	{
		static const char* const s_addr_mode[4] = { "16-bit", "None", "8-bit", "24-bit" };
		static const char* const s_baud_rate[4] = { "/16", "/4", "/8", "/32" };
		LOGMASKED(LOG_SIO, "%s: io_w: SIO Setup (not implemented) = %04x (DS301Ready:%d, Start:%d, Auto:%d, IRQEn:%d, Width:%d, Related:%d\n", machine().describe_context(), data
			, BIT(data, 11), BIT(data, 10), BIT(data, 9), BIT(data, 8), BIT(data, 7) ? 16 : 8, BIT(data, 6));
		LOGMASKED(LOG_SIO, "                                         (Mode:%s, RWProtocol:%d, Rate:sysclk%s, AddrMode:%s)\n"
			, BIT(data, 5), BIT(data, 4), s_baud_rate[(data >> 2) & 3], s_addr_mode[data & 3]);
		if (BIT(data, 10))
		{
			m_io_regs[REG_SIO_STATUS] |= 0x8000;
			m_sio_bits_remaining = BIT(data, 7) ? 16 : 8;
			m_sio_writing = BIT(data, 5);
		}
		else
		{
			m_io_regs[REG_SIO_STATUS] &= ~0x8000;
		}
		break;
	}

	case REG_SIO_ADDRL:
		LOGMASKED(LOG_SIO, "%s: io_w: SIO Start Address (low) (not implemented) = %04x\n", machine().describe_context(), data);
		break;

	case REG_SIO_ADDRH:
		LOGMASKED(LOG_SIO, "%s: io_w: SIO Start Address (hi) (not implemented) = %04x\n", machine().describe_context(), data);
		break;

	case REG_SIO_DATA:
		LOGMASKED(LOG_SIO, "%s: io_w: SIO Data (not implemented) = %04x\n", machine().describe_context(), data);
		if ((m_io_regs[REG_SIO_SETUP] & 0x8000) && m_sio_writing)
		{
			m_sio_bits_remaining--;
			if (m_sio_bits_remaining == 0)
			{
				m_io_regs[REG_SIO_STATUS] &= ~0x8000;
			}
		}
		break;

	case REG_SIO_AUTO_TX_NUM:
		LOGMASKED(LOG_SIO, "%s: io_w: SIO Auto Transmit Count (not implemented) = %04x\n", machine().describe_context(), data);
		break;

	case REG_I2C_CMD:
		LOGMASKED(LOG_I2C, "%s: io_w: I2C Command = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_I2C_CMD] = data;
		do_i2c();
		break;

	case REG_I2C_STATUS:
		LOGMASKED(LOG_I2C, "%s: io_w: I2C Acknowledge = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_I2C_STATUS] &= ~data;
		break;

	case REG_I2C_ACCESS:
		LOGMASKED(LOG_I2C, "%s: io_w: I2C Access Mode = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_I2C_ACCESS] = data;
		break;

	case REG_I2C_ADDR:
		LOGMASKED(LOG_I2C, "%s: io_w: I2C Device Address = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_I2C_ADDR] = data;
		break;

	case REG_I2C_SUBADDR:
		LOGMASKED(LOG_I2C, "%s: io_w: I2C Sub-Address = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_I2C_SUBADDR] = data;
		break;

	case REG_I2C_DATA_OUT:
		LOGMASKED(LOG_I2C, "%s: io_w: I2C Data Out = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_I2C_DATA_OUT] = data;
		break;

	case REG_I2C_DATA_IN:
		LOGMASKED(LOG_I2C, "%s: io_w: I2C Data In = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_I2C_DATA_IN] = data;
		break;

	case REG_I2C_MODE:
		LOGMASKED(LOG_I2C, "%s: io_w: I2C Controller Mode = %04x\n", machine().describe_context(), data);
		m_io_regs[REG_I2C_MODE] = data;
		break;

	default:
		LOGMASKED(LOG_UNKNOWN_IO, "%s: io_w: Unknown register %04x = %04x\n", machine().describe_context(), 0x3d00 + offset, data);
		m_io_regs[offset] = data;
		break;
	}
}

template <int Which>
TIMER_CALLBACK_MEMBER(spg2xx_io_device::tmb_timer_tick)
{
	LOGMASKED(LOG_TIMERS, "TMB%d elapsed, setting IRQ Status bit 0 (old:%04x, new:%04x, enable:%04x)\n", Which + 1, IO_IRQ_STATUS, IO_IRQ_STATUS | (1 << Which), IO_IRQ_ENABLE);
	const uint16_t old = IO_IRQ_STATUS;
	IO_IRQ_STATUS |= (1 << Which);
	const uint16_t changed = (old & IO_IRQ_ENABLE) ^ (IO_IRQ_STATUS & IO_IRQ_ENABLE);
	if (changed)
		check_tmb_lofreq_key_irq();
}

TIMER_CALLBACK_MEMBER(spg2xx_io_device::watchdog_tick)
{
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(spg2xx_io_device::system_timer_tick)
{
	IO_IRQ_STATUS |= 0x0040;

	bool check_lofreq = false;

	m_2khz_divider++;
	if (m_2khz_divider == 2)
	{
		m_2khz_divider = 0;
		IO_IRQ_STATUS |= 0x0020;

		m_1khz_divider++;
		if (m_1khz_divider == 2)
		{
			m_1khz_divider = 0;
			IO_IRQ_STATUS |= 0x0010;

			m_4hz_divider++;
			if (m_4hz_divider == 256)
			{
				m_4hz_divider = 0;
				IO_IRQ_STATUS |= 0x0008;
				check_lofreq = BIT(IO_IRQ_ENABLE, 3);
			}
		}
	}

	check_hifreq_periodic_irq();
	if (check_lofreq)
		check_tmb_lofreq_key_irq();
}

TIMER_CALLBACK_MEMBER(spg2xx_io_device::uart_transmit_tick)
{
	LOGMASKED(LOG_UART, "uart_transmit_tick: Transmitting %02x, setting TxReady, clearing TxBusy\n", (uint8_t)m_io_regs[REG_UART_TXBUF]);
	m_uart_tx((uint8_t)m_io_regs[REG_UART_TXBUF]);
	m_io_regs[REG_UART_STATUS] |= 0x0002;
	m_io_regs[REG_UART_STATUS] &= ~0x0040;
	if (BIT(m_io_regs[REG_UART_CTRL], 1))
	{
		const uint16_t old = IO_IRQ_STATUS;
		IO_IRQ_STATUS |= 0x0100;
		m_uart_tx_irq = true;
		LOGMASKED(LOG_UART, "uart_transmit_tick: Setting UART IRQ bit\n");
		if (IO_IRQ_STATUS != old && BIT(IO_IRQ_ENABLE, 8))
		{
			LOGMASKED(LOG_UART, "uart_transmit_tick: Bit newly set, checking IRQs\n");
			check_data_irq();
		}
	}
}

TIMER_CALLBACK_MEMBER(spg2xx_io_device::uart_receive_tick)
{
	LOGMASKED(LOG_UART, "uart_receive_tick: Setting RBF and RxRDY\n");
	m_io_regs[REG_UART_STATUS] |= 0x81;
	m_uart_rx_available = true;
	if (BIT(m_io_regs[REG_UART_CTRL], 0))
	{
		LOGMASKED(LOG_UART, "uart_receive_tick: RxIntEn is set, setting rx_irq to true and setting UART IRQ\n");
		m_uart_rx_irq = true;
		IO_IRQ_STATUS |= 0x0100;
		if (BIT(IO_IRQ_ENABLE, 8))
			check_data_irq();
	}
}

void spg2xx_io_device::extint_w(int channel, bool state)
{
	LOGMASKED(LOG_EXTINT, "Setting extint channel %d to %s\n", channel, state ? "true" : "false");
	m_extint[channel] = state;
	check_extint_irq(channel);
}

void spg2xx_io_device::check_extint_irq(int channel)
{
	LOGMASKED(LOG_EXTINT, "%sing extint %d interrupt\n", m_extint[channel] ? "rais" : "lower", channel + 1);
	const uint16_t mask = (channel == 0) ? 0x0200 : 0x1000;
	const uint16_t old_irq = IO_IRQ_STATUS;
	if (m_extint[channel])
		IO_IRQ_STATUS |= mask;
	else
		IO_IRQ_STATUS &= ~mask;

	if (old_irq != IO_IRQ_STATUS)
	{
		LOGMASKED(LOG_EXTINT, "extint IRQ changed, so checking interrupts\n");
		check_external_irq();
	}
}

void spg2xx_io_device::check_timers_irq()
{
	LOGMASKED(LOG_TIMERS, "Checking IRQ2 (%04x)\n", IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x0c00);
	m_timer_irq_cb((IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x0c00) ? ASSERT_LINE : CLEAR_LINE);
}

void spg2xx_io_device::check_data_irq()
{
	LOGMASKED(LOG_UART | LOG_SIO | LOG_SPI | LOG_I2C | LOG_ADC, "Checking IRQ3 (%04x)\n", IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x2100);
	m_uart_adc_irq_cb((IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x6100) ? ASSERT_LINE : CLEAR_LINE);
}

void spg2xx_io_device::check_external_irq()
{
	LOGMASKED(LOG_EXTINT, "Checking IRQ5 (%04x)\n", IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x1200);
	m_external_irq_cb((IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x1200) ? ASSERT_LINE : CLEAR_LINE);
}

void spg2xx_io_device::check_hifreq_periodic_irq()
{
	LOGMASKED(LOG_TIMERS, "Checking IRQ6 (%04x)\n", IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x0070);
	m_ffreq_tmr1_irq_cb((IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x0070) ? ASSERT_LINE : CLEAR_LINE);
}

void spg2xx_io_device::check_tmb_lofreq_key_irq()
{
	LOGMASKED(LOG_IRQS, "Checking IRQ7 (%04x)\n", IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x008b);
	m_ffreq_tmr2_irq_cb((IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x008b) ? ASSERT_LINE : CLEAR_LINE);
}

void spg2xx_io_device::check_all_irqs(const uint16_t changed)
{
	if (changed & 0x0c00) // Timer A, Timer B IRQ
		check_timers_irq();

	if (changed & 0x6100) // UART, SPI, SIO, I2C, ADC IRQ
		check_data_irq();

	if (changed & 0x1200) // External IRQ
		check_external_irq();

	if (changed & 0x0070) // 1024Hz, 2048Hz, 4096Hz IRQ
		check_hifreq_periodic_irq();

	if (changed & 0x008b) // TMB1, TMB2, 4Hz, key change IRQ
		check_tmb_lofreq_key_irq();
}

uint16_t spg2xx_io_device::do_special_gpio(uint32_t index, uint16_t mask)
{
	uint16_t data = 0;
	switch (index)
	{
		case 0: // Port A
			if (mask & 0xe000)
			{
				const uint8_t csel = m_cpu->get_csb() & 0x0e;
				data = (csel << 12) & mask;
			}
			break;
		case 1: // Port B
			// To do
			break;
		case 2: // Port C
			// To do
			break;
		default:
			// Can't happen
			break;
	}
	return data;
}

void spg2xx_io_device::do_gpio(uint32_t offset, bool write)
{
	uint32_t index = (offset - 1) / 5;
	uint16_t buffer = m_io_regs[5 * index + 2];
	uint16_t dir = m_io_regs[5 * index + 3];
	uint16_t attr = m_io_regs[5 * index + 4];
	uint16_t special = m_io_regs[5 * index + 5];

	uint16_t push = dir;
	uint16_t pull = ~dir;
	uint16_t what = (buffer & (push | pull));
	what ^= (dir & ~attr);
	what &= ~special;

	switch (index)
	{
		case 0:
			if (write)
				m_porta_out(0, what, push &~ special);
			what = (what & ~pull);
			if (!write)
				what |= m_porta_in(0, pull &~ special) & pull;
			break;
		case 1:
			if (write)
				m_portb_out(0, what, push &~ special);
			what = (what & ~pull);
			if (!write)
				what |= m_portb_in(0, pull &~ special) & pull;
			break;
		case 2:
			if (write)
				m_portc_out(0, what, push &~ special);
			what = (what & ~pull);
			if (!write)
				what |= m_portc_in(0, pull &~ special) & pull;
			break;
	}

	what |= do_special_gpio(index, special);
	m_io_regs[5 * index + 1] = what;
}

template <int Which>
TIMER_CALLBACK_MEMBER(spg2xx_io_device::adc_convert_tick)
{
	m_io_regs[REG_ADC_DATA] = (m_adc_in[Which]() & 0x0fff) | 0x8000;
	m_io_regs[REG_ADC_CTRL] |= 0x2000;
	if (BIT(m_io_regs[REG_ADC_CTRL], 9))
	{
		IO_IRQ_STATUS |= 0x2000;
		if (BIT(m_io_regs[REG_ADC_CTRL], 9) && BIT(IO_IRQ_ENABLE, 13))
		{
			check_data_irq();
		}
	}
}

void spg2xx_io_device::do_i2c()
{
	const uint16_t addr = ((m_io_regs[REG_I2C_ADDR] & 0x06) << 7) | (uint8_t)m_io_regs[REG_I2C_SUBADDR];

	if (m_io_regs[REG_I2C_CMD] & 0x40) // Serial EEPROM read
		m_io_regs[REG_I2C_DATA_IN] = m_i2c_r(addr);
	else
		m_i2c_w(addr, m_io_regs[REG_I2C_DATA_OUT]);

	m_io_regs[REG_I2C_STATUS] |= 1;
}
