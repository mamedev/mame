// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SCC68070 SoC peripheral emulation
    -------------------

    written by Ryan Holtz


*******************************************************************************

STATUS:

- Skeleton.  Just enough for the CD-i and Magicard to run.

TODO:

- Proper handling of the 68070's internal devices (UART, DMA, Timers, etc.)
- I2C could do with re-visiting.

*******************************************************************************/

#include "emu.h"
#include "scc68070.h"

#define LOG_I2C         (1U << 1)
#define LOG_UART        (1U << 2)
#define LOG_TIMERS      (1U << 3)
#define LOG_TIMERS_HF   (1U << 4)
#define LOG_DMA         (1U << 5)
#define LOG_MMU         (1U << 6)
#define LOG_IRQS        (1U << 7)
#define LOG_UNKNOWN     (1U << 8)
#define LOG_MORE_UART   (1U << 9)
#define LOG_ALL         (LOG_I2C | LOG_UART | LOG_TIMERS | LOG_DMA | LOG_MMU | LOG_IRQS | LOG_UNKNOWN)

#define VERBOSE         (0)

#include "logmacro.h"

#define ENABLE_UART_PRINTING (0)

//**************************************************************************
// Register defines
//**************************************************************************
enum isr_bits
{
	ISR_MST      =  0x80,                        // Master
	ISR_TRX      =  0x40,                        // Transmitter
	ISR_BB       =  0x20,                        // Busy
	ISR_PIN      =  0x10,                        // No Pending Interrupt
	ISR_AL       =  0x08,                        // Arbitration Lost
	ISR_AAS      =  0x04,                        // Addressed As Slave
	ISR_AD0      =  0x02,                        // Address Zero
	ISR_LRB      =  0x01,                        // Last Received Bit
	ISR_SSR_MASK =  (ISR_MST | ISR_TRX | ISR_BB),// Mask for detecting start/stop/restart
	ISR_START    =  (ISR_MST | ISR_TRX | ISR_BB),// Start bit request
	ISR_STOP     =  (ISR_MST | ISR_TRX)         // Stop bit request
};

enum umr_bits
{
	UMR_OM          = 0xc0,
	UMR_OM_NORMAL   = 0x00,
	UMR_OM_ECHO     = 0x40,
	UMR_OM_LOOPBACK = 0x80,
	UMR_OM_RLOOP    = 0xc0,
	UMR_TXC         = 0x10,
	UMR_PC          = 0x08,
	UMR_P           = 0x04,
	UMR_SB          = 0x02,
	UMR_CL          = 0x01
};

enum usr_bits
{
	USR_RB          = 0x80,
	USR_FE          = 0x40,
	USR_PE          = 0x20,
	USR_OE          = 0x10,
	USR_TXEMT       = 0x08,
	USR_TXRDY       = 0x04,
	USR_RXRDY       = 0x01
};

enum tsr_bits
{
	TSR_OV0         = 0x80,
	TSR_MA1         = 0x40,
	TSR_CAP1        = 0x20,
	TSR_OV1         = 0x10,
	TSR_MA2         = 0x08,
	TSR_CAP2        = 0x04,
	TSR_OV2         = 0x02
};

enum tcr_bits
{
	TCR_E1          = 0xc0,
	TCR_E1_NONE     = 0x00,
	TCR_E1_RISING   = 0x40,
	TCR_E1_FALLING  = 0x80,
	TCR_E1_BOTH     = 0xc0,
	TCR_M1          = 0x30,
	TCR_M1_NONE     = 0x00,
	TCR_M1_MATCH    = 0x10,
	TCR_M1_CAPTURE  = 0x20,
	TCR_M1_COUNT    = 0x30,
	TCR_E2          = 0x0c,
	TCR_E2_NONE     = 0x00,
	TCR_E2_RISING   = 0x04,
	TCR_E2_FALLING  = 0x08,
	TCR_E2_BOTH     = 0x0c,
	TCR_M2          = 0x03,
	TCR_M2_NONE     = 0x00,
	TCR_M2_MATCH    = 0x01,
	TCR_M2_CAPTURE  = 0x02,
	TCR_M2_COUNT    = 0x03
};

enum csr_bits
{
	CSR_COC         = 0x80,
	CSR_NDT         = 0x20,
	CSR_ERR         = 0x10,
	CSR_CA          = 0x08
};

enum cer_bits
{
	CER_EC          = 0x1f,
	CER_NONE        = 0x00,
	CER_TIMING      = 0x02,
	CER_BUSERR_MEM  = 0x09,
	CER_BUSERR_DEV  = 0x0a,
	CER_SOFT_ABORT  = 0x11
};

enum dcr1_bits
{
	DCR1_ERM        = 0x80,
	DCR1_DT         = 0x30
};

enum dcr2_bits
{
	DCR2_ERM        = 0x80,
	DCR2_DT         = 0x30,
	DCR2_DS         = 0x08
};


enum scr2_bits
{
	SCR2_MAC        = 0x0c,
	SCR2_MAC_NONE   = 0x00,
	SCR2_MAC_INC    = 0x04,
	SCR2_DAC        = 0x03,
	SCR2_DAC_NONE   = 0x00,
	SCR2_DAC_INC    = 0x01
};

enum ccr_bits
{
	CCR_SO          = 0x80,
	CCR_SA          = 0x10,
	CCR_INE         = 0x08,
	CCR_IPL         = 0x07
};

enum icr_bits
{
	ICR_SEL = 0x40,
	ICR_ESO = 0x08,
	ICR_ACK = 0x04
};

enum i2c_states
{
	I2C_IDLE = 0,
	I2C_TX_IN_PROGRESS,
	I2C_RX_IN_PROGRESS,
	I2C_RX_COMPLETE,
	I2C_GET_ACK,
	I2C_SEND_ACK,
	I2C_SEND_ACK_AND_RX,
	I2C_SEND_ACK_AND_STOP,
	I2C_SEND_STOP,
	I2C_CHANGED_TO_RX,
	I2C_SEND_RESTART
};

enum i2c_clock_states
{
	I2C_SCL_IDLE = 0,
	I2C_SCL_SET_0,
	I2C_SCL_SET_1,
	I2C_SCL_WAIT_1,
};


// device type definition
DEFINE_DEVICE_TYPE(SCC68070, scc68070_device, "scc68070", "Philips SCC68070")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

void scc68070_device::internal_map(address_map &map)
{
	map(0x80001001, 0x80001001).rw(FUNC(scc68070_device::lir_r), FUNC(scc68070_device::lir_w));
	map(0x80002001, 0x80002001).rw(FUNC(scc68070_device::idr_r), FUNC(scc68070_device::idr_w));
	map(0x80002003, 0x80002003).rw(FUNC(scc68070_device::iar_r), FUNC(scc68070_device::iar_w));
	map(0x80002005, 0x80002005).rw(FUNC(scc68070_device::isr_r), FUNC(scc68070_device::isr_w));
	map(0x80002007, 0x80002007).rw(FUNC(scc68070_device::icr_r), FUNC(scc68070_device::icr_w));
	map(0x80002009, 0x80002009).rw(FUNC(scc68070_device::iccr_r), FUNC(scc68070_device::iccr_w));
	map(0x80002011, 0x80002011).rw(FUNC(scc68070_device::umr_r), FUNC(scc68070_device::umr_w));
	map(0x80002013, 0x80002013).r(FUNC(scc68070_device::usr_r));
	map(0x80002015, 0x80002015).rw(FUNC(scc68070_device::ucsr_r), FUNC(scc68070_device::ucsr_w));
	map(0x80002017, 0x80002017).rw(FUNC(scc68070_device::ucr_r), FUNC(scc68070_device::ucr_w));
	map(0x80002019, 0x80002019).rw(FUNC(scc68070_device::uth_r), FUNC(scc68070_device::uth_w));
	map(0x8000201b, 0x8000201b).r(FUNC(scc68070_device::urh_r));
	map(0x80002020, 0x80002029).rw(FUNC(scc68070_device::timer_r), FUNC(scc68070_device::timer_w));
	map(0x80002045, 0x80002045).rw(FUNC(scc68070_device::picr1_r), FUNC(scc68070_device::picr1_w));
	map(0x80002047, 0x80002047).rw(FUNC(scc68070_device::picr2_r), FUNC(scc68070_device::picr2_w));
	map(0x80004000, 0x8000406d).rw(FUNC(scc68070_device::dma_r), FUNC(scc68070_device::dma_w));
	map(0x80008000, 0x8000807f).rw(FUNC(scc68070_device::mmu_r), FUNC(scc68070_device::mmu_w));
}

void scc68070_device::cpu_space_map(address_map &map)
{
	map(0xfffffff0, 0xffffffff).r(FUNC(scc68070_device::iack_r)).umask16(0x00ff);
}

//-------------------------------------------------
//  scc68070_device - constructor
//-------------------------------------------------

scc68070_device::scc68070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: scc68070_base_device(mconfig, tag, owner, clock, SCC68070, address_map_constructor(FUNC(scc68070_device::internal_map), this))
	, m_iack2_callback(*this, autovector(2))
	, m_iack4_callback(*this, autovector(4))
	, m_iack5_callback(*this, autovector(5))
	, m_iack7_callback(*this, autovector(7))
	, m_uart_tx_callback(*this)
	, m_uart_rtsn_callback(*this)
	, m_i2c_scl_callback(*this)
	, m_i2c_sdaw_callback(*this)
	, m_i2c_sdar_callback(*this, 0)
	, m_ipl(0)
	, m_in2_line(CLEAR_LINE)
	, m_in4_line(CLEAR_LINE)
	, m_in5_line(CLEAR_LINE)
	, m_nmi_line(CLEAR_LINE)
	, m_int1_line(CLEAR_LINE)
	, m_int2_line(CLEAR_LINE)
{
	m_cpu_space_config.m_internal_map = address_map_constructor(FUNC(scc68070_device::cpu_space_map), this);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void scc68070_device::device_start()
{
	scc68070_base_device::device_start();

	save_item(NAME(m_ipl));

	save_item(NAME(m_in2_line));
	save_item(NAME(m_in4_line));
	save_item(NAME(m_in5_line));
	save_item(NAME(m_nmi_line));

	save_item(NAME(m_int1_line));
	save_item(NAME(m_int2_line));
	save_item(NAME(m_lir));

	save_item(NAME(m_picr1));
	save_item(NAME(m_picr2));
	save_item(NAME(m_timer_int));
	save_item(NAME(m_i2c_int));
	save_item(NAME(m_uart_rx_int));
	save_item(NAME(m_uart_tx_int));

	save_item(NAME(m_i2c.data_register));
	save_item(NAME(m_i2c.address_register));
	save_item(NAME(m_i2c.status_register));
	save_item(NAME(m_i2c.control_register));
	save_item(NAME(m_i2c.clock_control_register));
	save_item(NAME(m_i2c.scl_out_state));
	save_item(NAME(m_i2c.scl_in_state));
	save_item(NAME(m_i2c.sda_out_state));
	save_item(NAME(m_i2c.sda_in_state));
	save_item(NAME(m_i2c.state));
	save_item(NAME(m_i2c.counter));
	save_item(NAME(m_i2c.clock_change_state));
	save_item(NAME(m_i2c.clocks));
	save_item(NAME(m_i2c.first_byte));
	save_item(NAME(m_i2c.ack_or_nak_sent));

	save_item(NAME(m_uart.mode_register));
	save_item(NAME(m_uart.status_register));
	save_item(NAME(m_uart.clock_select));
	save_item(NAME(m_uart.command_register));
	save_item(NAME(m_uart.receive_holding_register));
	save_item(NAME(m_uart.receive_pointer));
	save_item(NAME(m_uart.receive_buffer));
	save_item(NAME(m_uart.transmit_holding_register));
	save_item(NAME(m_uart.transmit_pointer));
	save_item(NAME(m_uart.transmit_buffer));
	save_item(NAME(m_uart.transmit_ctsn));

	save_item(NAME(m_timers.timer_status_register));
	save_item(NAME(m_timers.timer_control_register));
	save_item(NAME(m_timers.reload_register));
	save_item(NAME(m_timers.timer0));
	save_item(NAME(m_timers.timer1));
	save_item(NAME(m_timers.timer2));

	save_item(STRUCT_MEMBER(m_dma.channel, channel_status));
	save_item(STRUCT_MEMBER(m_dma.channel, channel_error));
	save_item(STRUCT_MEMBER(m_dma.channel, device_control));
	save_item(STRUCT_MEMBER(m_dma.channel, operation_control));
	save_item(STRUCT_MEMBER(m_dma.channel, sequence_control));
	save_item(STRUCT_MEMBER(m_dma.channel, channel_control));
	save_item(STRUCT_MEMBER(m_dma.channel, transfer_counter));
	save_item(STRUCT_MEMBER(m_dma.channel, memory_address_counter));
	save_item(STRUCT_MEMBER(m_dma.channel, device_address_counter));

	save_item(NAME(m_mmu.status));
	save_item(NAME(m_mmu.control));
	save_item(STRUCT_MEMBER(m_mmu.desc, attr));
	save_item(STRUCT_MEMBER(m_mmu.desc, length));
	save_item(STRUCT_MEMBER(m_mmu.desc, segment));
	save_item(STRUCT_MEMBER(m_mmu.desc, base));

	m_timers.timer0_timer = timer_alloc(FUNC(scc68070_device::timer0_callback), this);
	m_timers.timer0_timer->adjust(attotime::never);

	m_uart.rx_timer = timer_alloc(FUNC(scc68070_device::rx_callback), this);
	m_uart.rx_timer->adjust(attotime::never);

	m_uart.tx_timer = timer_alloc(FUNC(scc68070_device::tx_callback), this);
	m_uart.tx_timer->adjust(attotime::never);

	m_i2c.timer = timer_alloc(FUNC(scc68070_device::i2c_callback), this);
	m_i2c.timer->adjust(attotime::never);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void scc68070_device::device_reset()
{
	scc68070_base_device::device_reset();

	m_lir = 0;

	m_picr1 = 0;
	m_picr2 = 0;
	m_timer_int = false;
	m_i2c_int = false;
	m_uart_rx_int = false;
	m_uart_tx_int = false;

	m_i2c.data_register = 0;
	m_i2c.address_register = 0;
	m_i2c.status_register = ISR_PIN;
	m_i2c.control_register = 0;
	m_i2c.clock_control_register = 0;
	m_i2c.scl_out_state = true;
	m_i2c.scl_in_state = true;
	m_i2c.sda_out_state = true;
	m_i2c.state = I2C_IDLE;
	m_i2c.clock_change_state = I2C_SCL_IDLE;
	m_i2c.clocks = 0;

	m_uart.mode_register = 0;
	m_uart.status_register = USR_TXRDY;
	m_uart.clock_select = 0;
	m_uart.command_register = 0;
	m_uart.transmit_holding_register = 0;
	m_uart.receive_holding_register = 0;
	m_uart.receive_pointer = -1;
	m_uart.transmit_pointer = -1;
	m_uart.transmit_ctsn = true;

	m_timers.timer_status_register = 0;
	m_timers.timer_control_register = 0;
	m_timers.reload_register = 0;
	m_timers.timer0 = 0;
	m_timers.timer1 = 0;
	m_timers.timer2 = 0;

	for(int index = 0; index < 2; index++)
	{
		m_dma.channel[index].channel_status = 0;
		m_dma.channel[index].channel_error = 0;
		m_dma.channel[index].device_control = 0;
		m_dma.channel[index].operation_control = 0;
		m_dma.channel[index].sequence_control = 0;
		m_dma.channel[index].channel_control = 0;
		m_dma.channel[index].transfer_counter = 0;
		m_dma.channel[index].memory_address_counter = 0;
		m_dma.channel[index].device_address_counter = 0;
	}

	m_mmu.status = 0;
	m_mmu.control = 0;
	for(int index = 0; index < 8; index++)
	{
		m_mmu.desc[index].attr = 0;
		m_mmu.desc[index].length = 0;
		m_mmu.desc[index].segment = 0;
		m_mmu.desc[index].base = 0;
	}

	update_ipl();

	m_uart.rx_timer->adjust(attotime::never);
	m_uart.tx_timer->adjust(attotime::never);
	set_timer_callback(0);
}


void scc68070_device::device_config_complete()
{
	scc68070_base_device::device_config_complete();

	reset_cb().append(*this, FUNC(scc68070_device::reset_peripherals));
}


void scc68070_device::reset_peripherals(int state)
{
	if (state)
	{
		m_lir = 0;

		m_picr1 = 0;
		m_picr2 = 0;
		m_timer_int = false;
		m_i2c_int = false;
		m_uart_rx_int = false;
		m_uart_tx_int = false;

		m_i2c.status_register = ISR_PIN;
		m_i2c.control_register = 0;
		m_i2c.clock_control_register = 0;
		m_i2c.scl_out_state = true;
		m_i2c.scl_in_state = true;
		m_i2c.sda_out_state = true;
		m_i2c.state = I2C_IDLE;
		m_i2c.clock_change_state = I2C_SCL_IDLE;
		m_i2c.clocks = 0;
		m_uart.command_register = 0;
		m_uart.receive_pointer = -1;
		m_uart.transmit_pointer = -1;

		m_uart.mode_register = 0;
		m_uart.status_register = USR_TXRDY;
		m_uart.clock_select = 0;

		m_timers.timer_status_register = 0;
		m_timers.timer_control_register = 0;

		m_uart.rx_timer->adjust(attotime::never);
		m_uart.tx_timer->adjust(attotime::never);
		m_timers.timer0_timer->adjust(attotime::never);
		m_i2c.timer->adjust(attotime::never);

		update_ipl();
	}
}

void scc68070_device::update_ipl()
{
	const uint8_t external_level = (m_nmi_line == ASSERT_LINE) ? 7
		: (m_in5_line == ASSERT_LINE) ? 5
		: (m_in4_line == ASSERT_LINE) ? 4
		: (m_in2_line == ASSERT_LINE) ? 2 : 0;
	const uint8_t int1_level = BIT(m_lir, 7) ? (m_lir >> 4) & 7 : 0;
	const uint8_t int2_level = BIT(m_lir, 3) ? m_lir & 7 : 0;
	const uint8_t timer_level = m_timer_int ? m_picr1 & 7 : 0;
	const uint8_t uart_rx_level = m_uart_rx_int ? (m_picr2 >> 4) & 7 : 0;
	const uint8_t uart_tx_level = m_uart_tx_int ? m_picr2 & 7 : 0;
	const uint8_t i2c_level = m_i2c_int ? (m_picr1 >> 4) & 7 : 0;
	const uint8_t dma_ch1_level = (m_dma.channel[0].channel_status & CSR_COC) && (m_dma.channel[0].channel_control & CCR_INE) ? m_dma.channel[0].channel_control & CCR_IPL : 0;
	const uint8_t dma_ch2_level = (m_dma.channel[1].channel_status & CSR_COC) && (m_dma.channel[1].channel_control & CCR_INE) ? m_dma.channel[1].channel_control & CCR_IPL : 0;

	const uint8_t new_ipl = std::max({external_level, int1_level, int2_level, timer_level, uart_rx_level, uart_tx_level, i2c_level, dma_ch1_level, dma_ch2_level});

	if (m_ipl != new_ipl)
	{
		if (m_ipl != 0)
			set_input_line(m_ipl, CLEAR_LINE);
		if (new_ipl != 0)
			set_input_line(new_ipl, ASSERT_LINE);
		m_ipl = new_ipl;
	}
}

void scc68070_device::in2_w(int state)
{
	m_in2_line = state;
	update_ipl();
}

void scc68070_device::in4_w(int state)
{
	m_in4_line = state;
	update_ipl();
}

void scc68070_device::in5_w(int state)
{
	m_in5_line = state;
	update_ipl();
}

void scc68070_device::nmi_w(int state)
{
	m_nmi_line = state;
	update_ipl();
}

void scc68070_device::int1_w(int state)
{
	if (m_int1_line != state)
	{
		if (state == ASSERT_LINE && !BIT(m_lir, 7))
		{
			m_lir |= 0x80;
			update_ipl();
		}

		m_int1_line = state;
	}
}

void scc68070_device::int2_w(int state)
{
	if (m_int2_line != state)
	{
		if (state == ASSERT_LINE && !BIT(m_lir, 3))
		{
			m_lir |= 0x08;
			update_ipl();
		}

		m_int2_line = state;
	}
}

uint8_t scc68070_device::iack_r(offs_t offset)
{
	switch (offset)
	{
	case 2:
		if (m_in2_line == ASSERT_LINE)
			return m_iack2_callback();
		break;

	case 4:
		if (m_in4_line == ASSERT_LINE)
			return m_iack4_callback();
		break;

	case 5:
		if (m_in5_line == ASSERT_LINE)
			return m_iack5_callback();
		break;

	case 7:
		if (m_nmi_line == ASSERT_LINE)
			return m_iack7_callback();
		break;
	}

	if (!machine().side_effects_disabled())
	{
		if (BIT(m_lir, 7) && offset == ((m_lir >> 4) & 7))
		{
			m_lir &= 0x7f;
			update_ipl();
		}
		else if (BIT(m_lir, 3) && offset == (m_lir & 7))
		{
			m_lir &= 0xf7;
			update_ipl();
		}
		else if (m_timer_int && offset == (m_picr1 & 7))
		{
			m_timer_int = false;
			update_ipl();
		}
		else if (m_uart_rx_int && offset == ((m_picr2 >> 4) & 7))
		{
			m_uart_rx_int = false;
			update_ipl();
		}
		else if (m_uart_tx_int && offset == (m_picr2 & 7))
		{
			m_uart_tx_int = false;
			update_ipl();
		}
		else if (m_i2c_int && offset == ((m_picr2 >> 4) & 7))
		{
			m_i2c_int = false;
			update_ipl();
		}
	}

	return 0x38 + offset;
}

void scc68070_device::set_timer_callback(int channel)
{
	switch (channel)
	{
		case 0:
		{
			// Timer clock period is 96/CLKOUT
			uint32_t compare = 0x10000 - m_timers.timer0;
			attotime period = cycles_to_attotime(96 * compare);
			m_timers.timer0_timer->adjust(period);
			break;
		}
		default:
		{
			fatalerror( "Unsupported timer channel to set_timer_callback!\n" );
		}
	}
}

TIMER_CALLBACK_MEMBER(scc68070_device::timer0_callback)
{
	m_timers.timer0 = m_timers.reload_register;
	m_timers.timer_status_register |= TSR_OV0;
	if (!m_timer_int)
	{
		m_timer_int = true;
		update_ipl();
	}

	set_timer_callback(0);
}

void scc68070_device::uart_ctsn(int state)
{
	m_uart.transmit_ctsn = state ? true : false;
}

void scc68070_device::uart_rx(uint8_t data)
{
	m_uart.receive_pointer++;
	m_uart.receive_buffer[m_uart.receive_pointer] = data;
}

void scc68070_device::uart_tx(uint8_t data)
{
	m_uart.transmit_pointer++;
	m_uart.transmit_buffer[m_uart.transmit_pointer] = data;
	m_uart.status_register &= ~USR_TXEMT;
}

TIMER_CALLBACK_MEMBER(scc68070_device::rx_callback)
{
	if ((m_uart.command_register & 3) == 1)
	{
		if (m_uart.receive_pointer >= 0)
		{
			m_uart.status_register |= USR_RXRDY;
		}
		else
		{
			m_uart.status_register &= ~USR_RXRDY;
		}

		m_uart.receive_holding_register = m_uart.receive_buffer[0];

		if (m_uart.receive_pointer > -1)
		{
			LOGMASKED(LOG_UART, "scc68070_rx_callback: Receiving %02x\n", m_uart.receive_holding_register);

			m_uart_rx_int = true;
			update_ipl();

			m_uart.status_register |= USR_RXRDY;
		}
		else
		{
			m_uart.status_register &= ~USR_RXRDY;
		}
	}
	else
	{
		m_uart.status_register &= ~USR_RXRDY;
	}
}

TIMER_CALLBACK_MEMBER(scc68070_device::tx_callback)
{
	if (((m_uart.command_register >> 2) & 3) == 1)
	{
		m_uart.status_register |= USR_TXRDY;

		m_uart_tx_int = true;
		update_ipl();

		if (m_uart.transmit_pointer > -1)
		{
			if (m_uart.transmit_ctsn && BIT(m_uart.mode_register, 4))
			{
				return;
			}

			m_uart.transmit_holding_register = m_uart.transmit_buffer[0];
			m_uart_tx_callback(m_uart.transmit_holding_register);

			LOGMASKED(LOG_MORE_UART, "tx_callback: Transmitting %02x\n", m_uart.transmit_holding_register);
			for(int index = 0; index < m_uart.transmit_pointer; index++)
			{
				m_uart.transmit_buffer[index] = m_uart.transmit_buffer[index+1];
			}
			m_uart.transmit_pointer--;
		}

		if (m_uart.transmit_pointer < 0)
		{
			m_uart.status_register |= USR_TXEMT;
		}
	}
}

uint8_t scc68070_device::lir_r()
{
	// LIR priority level: 80001001
	return m_lir & 0x77;
}

void scc68070_device::lir_w(uint8_t data)
{
	LOGMASKED(LOG_IRQS, "%s: LIR Write: %02x\n", machine().describe_context(), data);

	switch (data & 0x88)
	{
	case 0x08:
		if (m_lir & 0x08)
		{
			m_lir &= 0xf7;
			update_ipl();
		}
		break;

	case 0x80:
		if (data & 0x80)
		{
			m_lir &= 0x7f;
			update_ipl();
		}
		break;

	case 0x88:
		if (data & 0x88)
		{
			m_lir &= 0x77;
			update_ipl();
		}
		break;
	}

	m_lir = (m_lir & 0x88) | (data & 0x77);
}

uint8_t scc68070_device::picr1_r()
{
	// PICR1: 80002045
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_IRQS, "%s: Peripheral Interrupt Control Register 1 Read: %02x\n", machine().describe_context(), m_picr1);
	return m_picr1 & 0x77;
}

void scc68070_device::picr1_w(uint8_t data)
{
	LOGMASKED(LOG_IRQS, "%s: Peripheral Interrupt Control Register 1 Write: %02x\n", machine().describe_context(), data);
	m_picr1 = data & 0x77;
	switch (data & 0x88)
	{
	case 0x08:
		if (m_timer_int)
		{
			m_timer_int = false;
			update_ipl();
		}
		break;

	case 0x80:
		if (m_i2c_int)
		{
			m_i2c_int = false;
			update_ipl();
		}
		break;

	case 0x88:
		if (m_timer_int || m_i2c_int)
		{
			m_timer_int = false;
			m_i2c_int = false;
			update_ipl();
		}
		break;
	}
}

uint8_t scc68070_device::picr2_r()
{
	// PICR2: 80002047
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_IRQS, "%s: Peripheral Interrupt Control Register 2 Read: %02x\n", machine().describe_context(), m_picr2);
	return m_picr2 & 0x77;
}

void scc68070_device::picr2_w(uint8_t data)
{
	LOGMASKED(LOG_IRQS, "%s: Peripheral Interrupt Control Register 2 Write: %02x\n", machine().describe_context(), data);
	m_picr2 = data & 0x77;
	switch (data & 0x88)
	{
	case 0x08:
		if (m_uart_tx_int)
		{
			m_uart_tx_int = false;
			update_ipl();
		}
		break;

	case 0x80:
		if (m_uart_rx_int)
		{
			m_uart_rx_int = false;
			update_ipl();
		}
		break;

	case 0x88:
		if (m_uart_tx_int || m_uart_rx_int)
		{
			m_uart_tx_int = false;
			m_uart_rx_int = false;
			update_ipl();
		}
		break;
	}
}

uint8_t scc68070_device::idr_r()
{
	// I2C data register: 80002001
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_I2C, "%s: I2C Data Register Read: %02x\n", machine().describe_context(), m_i2c.data_register);

	m_i2c.counter = 0;
	m_i2c.status_register |= ISR_PIN;
	m_i2c_int = false;
	update_ipl();

	if (m_i2c.state != I2C_RX_COMPLETE)
	{
	}
	else
	{
		m_i2c.sda_out_state = (m_i2c.control_register & ICR_ACK) ? false : true;
		m_i2c_sdaw_callback(m_i2c.sda_out_state);

		if (m_i2c.control_register & ICR_ACK)
		{
			m_i2c.state = I2C_SEND_ACK_AND_RX;
			m_i2c.clocks = 9;
		}
		else
		{
			m_i2c.state = I2C_SEND_ACK;
			m_i2c.clocks = 1;
		}
		m_i2c.ack_or_nak_sent = true;
		m_i2c.clock_change_state = I2C_SCL_SET_1;
		set_i2c_timer();

	}
	return m_i2c.data_register;
}

void scc68070_device::idr_w(uint8_t data)
{
	LOGMASKED(LOG_I2C, "%s: I2C Data Register Write: %02x\n", machine().describe_context(), data);
	m_i2c.data_register = data;
	if (m_i2c.status_register & ISR_MST && m_i2c.status_register & ISR_TRX && m_i2c.status_register & ISR_BB)
	{
		m_i2c.status_register |= ISR_PIN;
		m_i2c_int = false;
		update_ipl();
		m_i2c.counter = 0;
		m_i2c.state = I2C_TX_IN_PROGRESS;
		m_i2c.clocks = 9;
		i2c_process_falling_scl();
		m_i2c.clock_change_state = I2C_SCL_SET_1;
		set_i2c_timer();
	}
}

uint8_t scc68070_device::iar_r()
{
	// I2C address register: 80002003
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_I2C, "%s: I2C Address Register Read: %02x\n", machine().describe_context(), m_i2c.address_register);
	return m_i2c.address_register;
}

void scc68070_device::iar_w(uint8_t data)
{
	LOGMASKED(LOG_I2C, "%s: I2C Address Register Write: %02x\n", machine().describe_context(), data);
	m_i2c.address_register = data;
}

uint8_t scc68070_device::isr_r()
{
	// I2C status register: 80002005
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_I2C, "%s: I2C Status Register Read: %02x\n", machine().describe_context(), m_i2c.status_register);
	return m_i2c.status_register;
}

void scc68070_device::isr_w(uint8_t data)
{
	LOGMASKED(LOG_I2C, "%s: I2C Status Register Write: %02x\n", machine().describe_context(), data);
	if (data & ISR_MST)
	{
		if ((data & ISR_SSR_MASK) == ISR_START)
		{
			if ((m_i2c.status_register & ISR_SSR_MASK) == ISR_STOP || (m_i2c.status_register & ISR_SSR_MASK) == 0)
			{
				if (m_i2c_sdar_callback() && m_i2c.state == I2C_IDLE)
				{
					m_i2c.status_register = data;
					if (data & ISR_PIN)
					{
						m_i2c_int = false;
						update_ipl();
					}
					m_i2c.sda_out_state = false;
					m_i2c_sdaw_callback(false);
					m_i2c.clock_change_state = I2C_SCL_SET_0;
					m_i2c.clocks = 10;
					m_i2c.state = I2C_TX_IN_PROGRESS;
					m_i2c.first_byte = true;
					m_i2c.ack_or_nak_sent = false;
					set_i2c_timer();
					m_i2c.counter = 0;
				}
				else
				{
					m_i2c.status_register |= ISR_AL;
					m_i2c.status_register &= ~ISR_PIN;
					m_i2c_int = true;
					update_ipl();
				}
			}
			else if ((m_i2c.status_register & ISR_SSR_MASK) == ISR_MST)
			{
				m_i2c.status_register = data;
				if (data & ISR_PIN)
				{
					m_i2c_int = false;
					update_ipl();
				}
				m_i2c.sda_out_state = true;
				m_i2c_sdaw_callback(true);
				m_i2c.clock_change_state = I2C_SCL_SET_1;
				m_i2c.clocks = 10;
				m_i2c.state = I2C_SEND_RESTART;
				m_i2c.first_byte = true;
				m_i2c.ack_or_nak_sent = false;
				set_i2c_timer();
				m_i2c.counter = 0;
			}
		}
		else if ((data & ISR_SSR_MASK) == ISR_STOP && m_i2c.status_register & ISR_BB)
		{
			// we should send STOP here, however, unkte06 in magicard appears to expect
			// NAK followed by STOP when in read mode.

			if (data & ISR_PIN)
			{
				m_i2c_int = false;
				update_ipl();
			}

			if (m_i2c.ack_or_nak_sent || (m_i2c.status_register & ISR_TRX))
			{
				m_i2c.state = I2C_SEND_STOP;
				m_i2c.sda_out_state = false;
				m_i2c_sdaw_callback(false);
			}
			else
			{
				m_i2c.ack_or_nak_sent = true;
				m_i2c.sda_out_state = (m_i2c.control_register&ICR_ACK) ? false : true;
				m_i2c_sdaw_callback(m_i2c.sda_out_state);
				m_i2c.state = I2C_SEND_ACK_AND_STOP;
				m_i2c.clocks = 2;
			}
			m_i2c.status_register = data | ISR_BB;
			m_i2c.clock_change_state = I2C_SCL_SET_1;
			set_i2c_timer();
		}
		else if ((data & ISR_SSR_MASK) == ISR_MST)
		{
			m_i2c.status_register = data;
			if (data & ISR_PIN)
			{
				m_i2c_int = false;
				update_ipl();
			}
		}
		else
		{
			if (data & ISR_PIN && !(m_i2c.status_register & ISR_PIN))
			{
				if (m_i2c.state == I2C_CHANGED_TO_RX)
				{
					m_i2c.state = I2C_RX_IN_PROGRESS;
					m_i2c.clock_change_state = I2C_SCL_SET_1;
					m_i2c.status_register = data;
					m_i2c_int = false;
					update_ipl();
					m_i2c.counter = 0;
					m_i2c.clocks = 8;
					set_i2c_timer();
				}
				else
				{
					m_i2c.ack_or_nak_sent = true;
					m_i2c.sda_out_state = (m_i2c.control_register&ICR_ACK) ? false : true;
					m_i2c_sdaw_callback(m_i2c.sda_out_state);
					m_i2c.status_register = data;
					m_i2c_int = false;
					update_ipl();
					m_i2c.state = I2C_SEND_ACK;
					m_i2c.clock_change_state = I2C_SCL_SET_1;
					m_i2c.clocks = 1;
					set_i2c_timer();
				}
			}
			else
			{
				m_i2c.status_register = data;
				if (data & ISR_PIN)
				{
					m_i2c_int = false;
					update_ipl();
				}
			}
		}
	}
	else
	{
		m_i2c.status_register = data;
		m_i2c_int = false;
		update_ipl();
		m_i2c.timer->adjust(attotime::never);
		m_i2c_scl_callback(1);
		m_i2c_sdaw_callback(1);
		m_i2c.scl_out_state = true;
		m_i2c.scl_in_state = true;
		m_i2c.sda_out_state = true;
		m_i2c.state = I2C_IDLE;
	}
}

uint8_t scc68070_device::icr_r()
{
	// I2C control register: 80002007
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_I2C, "%s: I2C Control Register Read: %02x\n", machine().describe_context(), m_i2c.control_register);
	return m_i2c.control_register;
}

void scc68070_device::icr_w(uint8_t data)
{
	LOGMASKED(LOG_I2C, "%s: I2C Control Register Write: %02x\n", machine().describe_context(), data);
	m_i2c.control_register = data;

	if (!(data & ICR_ESO))
	{
		m_i2c.timer->adjust(attotime::never);
		m_i2c_scl_callback(1);
		m_i2c_sdaw_callback(1);
		m_i2c.scl_out_state = true;
		m_i2c.scl_in_state = true;
		m_i2c.sda_out_state = true;
		m_i2c.state = I2C_IDLE;
	}
}

uint8_t scc68070_device::iccr_r()
{
	// I2C clock control register: 80002009
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_I2C, "%s: I2C Clock Control Register Read: %02x\n", machine().describe_context(), m_i2c.clock_control_register);
	return m_i2c.clock_control_register | 0xe0;
}

void scc68070_device::iccr_w(uint8_t data)
{
	LOGMASKED(LOG_I2C, "%s: I2C Clock Control Register Write: %02x\n", machine().describe_context(), data);
	m_i2c.clock_control_register = data & 0x1f;
}


void scc68070_device::i2c_process_falling_scl()
{
	switch (m_i2c.state)
	{
		case I2C_TX_IN_PROGRESS:
			if (m_i2c.counter<8)
			{
				m_i2c.sda_out_state = BIT(m_i2c.data_register, 7 - m_i2c.counter);
				m_i2c_sdaw_callback(m_i2c.sda_out_state);
				m_i2c.counter++;
			}
			else
			{
				m_i2c.sda_out_state = true;
				m_i2c_sdaw_callback(true);
				m_i2c.state = I2C_GET_ACK;
			}
			break;

		case I2C_GET_ACK:
			m_i2c.status_register &= ~ISR_PIN;
			m_i2c_int = true;
			update_ipl();
			m_i2c.state = I2C_IDLE;
			if (m_i2c.first_byte)
			{
				m_i2c.first_byte = false;
				if (BIT(m_i2c.data_register, 0))
				{
					m_i2c.status_register &= ~ISR_TRX;
					if (!(m_i2c.status_register & ISR_LRB))
					{
						m_i2c.state = I2C_CHANGED_TO_RX;
					}
				}
			}
			break;

		case I2C_RX_IN_PROGRESS:
			if (m_i2c.counter >= 8)
			{
				m_i2c.status_register &= ~ISR_PIN;
				m_i2c_int = true;
				update_ipl();
				m_i2c.state = I2C_RX_COMPLETE;
			}
			break;

		case I2C_SEND_ACK_AND_RX:
			m_i2c.sda_out_state = true;
			m_i2c_sdaw_callback(true);
			m_i2c.state = I2C_RX_IN_PROGRESS;
			m_i2c.counter = 0;
			break;

		case I2C_SEND_ACK_AND_STOP:
			m_i2c.sda_out_state = false;
			m_i2c_sdaw_callback(false);
			m_i2c.state = I2C_SEND_STOP;
			break;

		case I2C_SEND_ACK:
			m_i2c.state = I2C_IDLE;
			m_i2c.status_register &= ~ISR_PIN;
			m_i2c_int = true;
			update_ipl();
			break;
	}
}

void scc68070_device::i2c_process_rising_scl()
{
	switch (m_i2c.state)
	{
		case I2C_GET_ACK:
			if (m_i2c_sdar_callback())
			{
				m_i2c.status_register |= ISR_LRB;
			}
			else
			{
				m_i2c.status_register &= ~ISR_LRB;
			}
			break;

		case I2C_SEND_STOP:
		case I2C_SEND_RESTART:
			m_i2c.timer->adjust(attotime::from_nsec(5000));
			break;

		case I2C_RX_IN_PROGRESS:
			if (m_i2c.counter < 8)
			{
				m_i2c.data_register <<= 1;
				m_i2c.data_register |= m_i2c_sdar_callback();
				m_i2c.counter++;
			}
			break;
	}
}

void scc68070_device::write_scl(int state)
{
	if (m_i2c.status_register & ISR_MST)
	{
		if (m_i2c.scl_in_state != state && state)
		{
			i2c_process_rising_scl();
			i2c_next_state();
		}
	}
	m_i2c.scl_in_state = state;
}

TIMER_CALLBACK_MEMBER(scc68070_device::i2c_callback)
{
	i2c_next_state();
}

void scc68070_device::i2c_next_state()
{
	switch (m_i2c.clock_change_state)
	{
		case I2C_SCL_SET_0:
			if (m_i2c.state == I2C_SEND_STOP)
			{
				if (!m_i2c.sda_out_state)
				{
					m_i2c.sda_out_state = true;
					m_i2c_sdaw_callback(true);
					set_i2c_timer();
				}
				else
				{
					m_i2c.state = I2C_IDLE;
					m_i2c.status_register &= ~(ISR_PIN | ISR_BB);
					m_i2c_int = true;
					update_ipl();
					m_i2c.clock_change_state = I2C_SCL_IDLE;
				}
			}
			else if (m_i2c.state == I2C_SEND_RESTART)
			{
				m_i2c.sda_out_state = false;
				m_i2c_sdaw_callback(false);
				set_i2c_timer();
				m_i2c.clock_change_state = I2C_SCL_SET_0;
				m_i2c.state = I2C_TX_IN_PROGRESS;
			}
			else
			{
				m_i2c.scl_out_state = false;
				m_i2c_scl_callback(false);
				if (m_i2c.clocks)
				{
					m_i2c.clocks--;
				}
				if (m_i2c.clocks == 0)
				{
					m_i2c.clock_change_state = I2C_SCL_IDLE;
				}
				else
				{
					set_i2c_timer();
					m_i2c.clock_change_state = I2C_SCL_SET_1;
				}
				i2c_process_falling_scl();
			}
			break;

		case I2C_SCL_SET_1:
			m_i2c.clock_change_state = I2C_SCL_WAIT_1;
			m_i2c.scl_out_state = true;
			m_i2c_scl_callback(true);
			break;

		case I2C_SCL_WAIT_1:
			set_i2c_timer();
			m_i2c.clock_change_state = I2C_SCL_SET_0;
			break;
	}
}

void scc68070_device::set_i2c_timer()
{
	// divider offset 0 entry is illegal
	static constexpr int divider[]={    1,   78,   90,  102,  126,  150,  174,    198,
									  246,  294,  342,  390,  486,  582,  678,    774,
									  996, 1158, 1350, 1542, 1926, 2310, 2694,   3078,
									 3846, 4614, 5382, 6150, 7686, 9222, 10758, 12294 };
	m_i2c.timer->adjust(cycles_to_attotime(divider[m_i2c.clock_control_register]));
}

uint8_t scc68070_device::umr_r()
{
	// UART mode register: 80002011
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_MORE_UART, "%s: UART Mode Register Read: %02x\n", machine().describe_context(), m_uart.mode_register);
	return m_uart.mode_register | 0x20;
}

void scc68070_device::umr_w(uint8_t data)
{
	LOGMASKED(LOG_MORE_UART, "%s: UART Mode Register Write: %02x\n", machine().describe_context(), data);
	m_uart.mode_register = data;
}

uint8_t scc68070_device::usr_r()
{
	// UART status register: 80002013
	if (!machine().side_effects_disabled())
	{
		m_uart.status_register |= (1 << 1);
		LOGMASKED(LOG_MORE_UART, "%s: UART Status Register Read: %02x\n", machine().describe_context(), m_uart.status_register);
	}
	return m_uart.status_register | 0x08; // hack for magicard
}

uint8_t scc68070_device::ucsr_r()
{
	// UART clock select register: 80002015
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_UART, "%s: UART Clock Select Read: %02x\n", machine().describe_context(), m_uart.clock_select);
	return m_uart.clock_select | 0x08;
}

void scc68070_device::ucsr_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "%s: UART Clock Select Write: %02x\n", machine().describe_context(), data);
	m_uart.clock_select = data;

	static const uint32_t s_baud_divisors[8] = { 65536, 32768, 16384, 4096, 2048, 1024, 512, 256 };

	attotime rx_rate = attotime::from_ticks(s_baud_divisors[(data >> 4) & 7] * 10, 49152000);
	attotime tx_rate = attotime::from_ticks(s_baud_divisors[data & 7] * 10, 49152000);
	m_uart.rx_timer->adjust(rx_rate, 0, rx_rate);
	m_uart.tx_timer->adjust(tx_rate, 0, tx_rate);
}

uint8_t scc68070_device::ucr_r()
{
	// UART command register: 80002017
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_UART, "%s: UART Command Register Read: %02x\n", machine().describe_context(), m_uart.command_register);
	return m_uart.command_register | 0x80;
}

void scc68070_device::ucr_w(uint8_t data)
{
	LOGMASKED(LOG_MORE_UART, "%s: UART Command Register Write: %02x\n", machine().describe_context(), data);
	m_uart.command_register = data;
	const uint8_t misc_command = (data & 0x70) >> 4;
	switch (misc_command)
	{
	case 0x2: // Reset receiver
		LOGMASKED(LOG_MORE_UART, "%s: Reset receiver\n", machine().describe_context());
		m_uart.receive_pointer = -1;
		m_uart.command_register &= 0xf0;
		m_uart.receive_holding_register = 0x00;
		break;
	case 0x3: // Reset transmitter
		LOGMASKED(LOG_MORE_UART, "%s: Reset transmitter\n", machine().describe_context());
		m_uart.transmit_pointer = -1;
		m_uart.status_register |= USR_TXEMT;
		m_uart.command_register &= 0xf0;
		m_uart.transmit_holding_register = 0x00;
		break;
	case 0x4: // Reset error status
		LOGMASKED(LOG_MORE_UART, "%s: Reset error status\n", machine().describe_context());
		m_uart.status_register &= 0x87; // Clear error bits in USR
		m_uart.command_register &= 0xf0;
		break;
	case 0x6: // Start break
		LOGMASKED(LOG_MORE_UART, "%s: Start break (not yet implemented)\n", machine().describe_context());
		break;
	case 0x7: // Stop break
		LOGMASKED(LOG_MORE_UART, "%s: Stop break (not yet implemented)\n", machine().describe_context());
		break;
	}
}

uint8_t scc68070_device::uth_r()
{
	// UART transmit holding register: 80002019
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_UART, "%s: UART Transmit Holding Register Read: %02x\n", machine().describe_context(), m_uart.transmit_holding_register);
	return m_uart.transmit_holding_register;
}

void scc68070_device::uth_w(uint8_t data)
{
	LOGMASKED(LOG_MORE_UART, "%s: UART Transmit Holding Register Write: %02x ('%c')\n", machine().describe_context(), data, (data >= 0x20 && data < 0x7f) ? data : ' ');
	uart_tx(data);
	m_uart.transmit_holding_register = data;
}

uint8_t scc68070_device::urh_r()
{
	// UART receive holding register: 8000201b
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_UART, "%s: UART Receive Holding Register Read: %02x\n", machine().describe_context(), m_uart.receive_holding_register);

		if (m_uart_rx_int)
		{
			m_uart_rx_int = false;
			update_ipl();
		}

		m_uart.receive_holding_register = m_uart.receive_buffer[0];
		if (m_uart.receive_pointer >= 0)
		{
			for(int index = 0; index < m_uart.receive_pointer; index++)
			{
				m_uart.receive_buffer[index] = m_uart.receive_buffer[index + 1];
			}
			m_uart.receive_pointer--;
		}
	}
	return m_uart.receive_holding_register;
}

uint16_t scc68070_device::timer_r(offs_t offset, uint16_t mem_mask)
{
	switch (offset)
	{
	// Timers: 80002020 to 80002029
	case 0x0/2:
		if (ACCESSING_BITS_0_7 && !machine().side_effects_disabled())
		{
			LOGMASKED(LOG_TIMERS, "%s: Timer Control Register Read: %02x & %04x\n", machine().describe_context(), m_timers.timer_control_register, mem_mask);
		}
		if (ACCESSING_BITS_8_15 && !machine().side_effects_disabled())
		{
			LOGMASKED(LOG_TIMERS_HF, "%s: Timer Status Register Read: %02x & %04x\n", machine().describe_context(), m_timers.timer_status_register, mem_mask);
		}
		return (m_timers.timer_status_register << 8) | m_timers.timer_control_register;
	case 0x2/2:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_TIMERS, "%s: Timer Reload Register Read: %04x & %04x\n", machine().describe_context(), m_timers.reload_register, mem_mask);
		return m_timers.reload_register;
	case 0x4/2:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_TIMERS, "%s: Timer 0 Read: %04x & %04x\n", machine().describe_context(), m_timers.timer0, mem_mask);
		return 0x10000 - (attotime_to_cycles(m_timers.timer0_timer->remaining()) / 96);

	case 0x6/2:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_TIMERS, "%s: Timer 1 Read: %04x & %04x\n", machine().describe_context(), m_timers.timer1, mem_mask);
		return m_timers.timer1;
	case 0x8/2:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_TIMERS, "%s: Timer 2 Read: %04x & %04x\n", machine().describe_context(), m_timers.timer2, mem_mask);
		return m_timers.timer2;
	default:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_TIMERS | LOG_UNKNOWN, "%s: Timer Unknown Register Read: %04x & %04x\n", machine().describe_context(), offset * 2, mem_mask);
		break;
	}

	return 0;
}

void scc68070_device::timer_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
	// Timers: 80002020 to 80002029
	case 0x0/2:
		if (ACCESSING_BITS_0_7)
		{
			LOGMASKED(LOG_TIMERS, "%s: Timer Control Register Write: %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_timers.timer_control_register = data & 0x00ff;
		}
		if (ACCESSING_BITS_8_15)
		{
			LOGMASKED(LOG_TIMERS_HF, "%s: Timer Status Register Write: %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_timers.timer_status_register &= ~(data >> 8);
		}
		break;
	case 0x2/2:
		LOGMASKED(LOG_TIMERS, "%s: Timer Reload Register Write: %04x & %04x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_timers.reload_register);
		set_timer_callback(0);
		break;
	case 0x4/2:
		LOGMASKED(LOG_TIMERS, "%s: Timer 0 Write: %04x & %04x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_timers.timer0);
		set_timer_callback(0);
		break;
	case 0x6/2:
		LOGMASKED(LOG_TIMERS, "%s: Timer 1 Write: %04x & %04x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_timers.timer1);
		break;
	case 0x8/2:
		LOGMASKED(LOG_TIMERS, "%s: Timer 2 Write: %04x & %04x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_timers.timer2);
		break;
	default:
		LOGMASKED(LOG_TIMERS | LOG_UNKNOWN, "%s: Timer Unknown Register Write: %04x = %04x & %04x\n", machine().describe_context(), offset * 2, data, mem_mask);
		break;
	}
}

uint16_t scc68070_device::dma_r(offs_t offset, uint16_t mem_mask)
{
	switch (offset)
	{
	// DMA controller: 80004000 to 8000406d
	case 0x00/2:
	case 0x40/2:
		if (ACCESSING_BITS_0_7 && !machine().side_effects_disabled())
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Error Register Read: %04x & %04x\n", machine().describe_context(), offset / 32, m_dma.channel[offset / 32].channel_error, mem_mask);
		}
		if (ACCESSING_BITS_8_15 && !machine().side_effects_disabled())
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Status Register Read: %04x & %04x\n", machine().describe_context(), offset / 32, m_dma.channel[offset / 32].channel_status, mem_mask);
		}
		return (m_dma.channel[offset / 32].channel_status << 8) | m_dma.channel[offset / 32].channel_error;
	case 0x04/2:
	case 0x44/2:
		if (ACCESSING_BITS_0_7 && !machine().side_effects_disabled())
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Operation Control Register Read: %02x & %04x\n", machine().describe_context(), offset / 32, m_dma.channel[offset / 32].operation_control, mem_mask);
		}
		if (ACCESSING_BITS_8_15 && !machine().side_effects_disabled())
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Device Control Register Read: %02x & %04x\n", machine().describe_context(), offset / 32, m_dma.channel[offset / 32].device_control, mem_mask);
		}
		return (m_dma.channel[offset / 32].device_control << 8) | m_dma.channel[offset / 32].operation_control;
	case 0x06/2:
	case 0x46/2:
		if (ACCESSING_BITS_0_7 && !machine().side_effects_disabled())
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Channel Control Register Read: %02x & %04x\n", machine().describe_context(), offset / 32, m_dma.channel[offset / 32].channel_control, mem_mask);
		}
		if (ACCESSING_BITS_8_15 && !machine().side_effects_disabled())
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Sequence Control Register Read: %02x & %04x\n", machine().describe_context(), offset / 32, m_dma.channel[offset / 32].sequence_control, mem_mask);
		}
		return (m_dma.channel[offset / 32].sequence_control << 8) | m_dma.channel[offset / 32].channel_control;
	case 0x0a/2:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Memory Transfer Counter Read: %04x & %04x\n", machine().describe_context(), offset / 32, m_dma.channel[offset / 32].transfer_counter, mem_mask);
		return m_dma.channel[offset / 32].transfer_counter;
	case 0x0c/2:
	case 0x4c/2:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Memory Address Counter (High Word) Read: %04x & %04x\n", machine().describe_context(), offset / 32, (m_dma.channel[offset / 32].memory_address_counter >> 16), mem_mask);
		return (m_dma.channel[offset / 32].memory_address_counter >> 16);
	case 0x0e/2:
	case 0x4e/2:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Memory Address Counter (Low Word) Read: %04x & %04x\n", machine().describe_context(), offset / 32, m_dma.channel[offset / 32].memory_address_counter, mem_mask);
		return m_dma.channel[offset / 32].memory_address_counter;
	case 0x14/2:
	case 0x54/2:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Device Address Counter (High Word) Read: %04x & %04x\n", machine().describe_context(), offset / 32, (m_dma.channel[offset / 32].device_address_counter >> 16), mem_mask);
		return (m_dma.channel[offset / 32].device_address_counter >> 16);
	case 0x16/2:
	case 0x56/2:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Device Address Counter (Low Word) Read: %04x & %04x\n", machine().describe_context(), offset / 32, m_dma.channel[offset / 32].device_address_counter, mem_mask);
		return m_dma.channel[offset / 32].device_address_counter;

	default:
		LOGMASKED(LOG_DMA | LOG_UNKNOWN, "%s: DMA Unknown Register Read: %04x & %04x\n", machine().describe_context(), offset * 2, mem_mask);
		break;
	}

	return 0;
}

void scc68070_device::dma_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
	// DMA controller: 80004000 to 8000406d
	case 0x00/2:
	case 0x40/2:
		if (ACCESSING_BITS_0_7)
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Error (invalid) Write: %04x & %04x\n", machine().describe_context(), offset / 32, data, mem_mask);
		}
		if (ACCESSING_BITS_8_15)
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Status Write: %04x & %04x\n", machine().describe_context(), offset / 32, data, mem_mask);
			m_dma.channel[offset / 32].channel_status &= ~((data >> 8) & 0xb0);
			update_ipl();
		}
		break;
	case 0x04/2:
	case 0x44/2:
		if (ACCESSING_BITS_0_7)
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Operation Control Register Write: %04x & %04x\n", machine().describe_context(), offset / 32, data, mem_mask);
			m_dma.channel[offset / 32].operation_control = data & 0x00ff;
		}
		if (ACCESSING_BITS_8_15)
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Device Control Register Write: %04x & %04x\n", machine().describe_context(), offset / 32, data, mem_mask);
			m_dma.channel[offset / 32].device_control = data >> 8;
		}
		break;
	case 0x06/2:
	case 0x46/2:
		if (ACCESSING_BITS_0_7)
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Channel Control Register Write: %04x & %04x\n", machine().describe_context(), offset / 32, data, mem_mask);
			m_dma.channel[offset / 32].channel_control = data & 0x007f;
			if (data & CCR_SO)
			{
				m_dma.channel[offset / 32].channel_status |= CSR_COC;
			}
			update_ipl();
		}
		if (ACCESSING_BITS_8_15)
		{
			LOGMASKED(LOG_DMA, "%s: DMA(%d) Sequence Control Register Write: %04x & %04x\n", machine().describe_context(), offset / 32, data, mem_mask);
			m_dma.channel[offset / 32].sequence_control = data >> 8;
		}
		break;
	case 0x0a/2:
		LOGMASKED(LOG_DMA, "%s: DMA(%d) Memory Transfer Counter Write: %04x & %04x\n", machine().describe_context(), offset / 32, data, mem_mask);
		COMBINE_DATA(&m_dma.channel[offset / 32].transfer_counter);
		break;
	case 0x0c/2:
	case 0x4c/2:
		LOGMASKED(LOG_DMA, "%s: DMA(%d) Memory Address Counter (High Word) Write: %04x & %04x\n", machine().describe_context(), offset / 32, data, mem_mask);
		m_dma.channel[offset / 32].memory_address_counter &= ~(mem_mask << 16);
		m_dma.channel[offset / 32].memory_address_counter |= data << 16;
		break;
	case 0x0e/2:
	case 0x4e/2:
		LOGMASKED(LOG_DMA, "%s: DMA(%d) Memory Address Counter (Low Word) Write: %04x & %04x\n", machine().describe_context(), offset / 32, data, mem_mask);
		m_dma.channel[offset / 32].memory_address_counter &= ~mem_mask;
		m_dma.channel[offset / 32].memory_address_counter |= data;
		break;
	case 0x14/2:
	case 0x54/2:
		LOGMASKED(LOG_DMA, "%s: DMA(%d) Device Address Counter (High Word) Write: %04x & %04x\n", machine().describe_context(), offset / 32, data, mem_mask);
		m_dma.channel[offset / 32].device_address_counter &= ~(mem_mask << 16);
		m_dma.channel[offset / 32].device_address_counter |= data << 16;
		break;
	case 0x16/2:
	case 0x56/2:
		LOGMASKED(LOG_DMA, "%s: DMA(%d) Device Address Counter (Low Word) Write: %04x & %04x\n", machine().describe_context(), offset / 32, data, mem_mask);
		m_dma.channel[offset / 32].device_address_counter &= ~mem_mask;
		m_dma.channel[offset / 32].device_address_counter |= data;
		break;
	default:
		LOGMASKED(LOG_DMA | LOG_UNKNOWN, "%s: DMA Unknown Register Write: %04x = %04x & %04x\n", machine().describe_context(), offset * 2, data, mem_mask);
		break;
	}
}

uint16_t scc68070_device::mmu_r(offs_t offset, uint16_t mem_mask)
{
	switch (offset)
	{
	// MMU: 80008000 to 8000807f
	case 0x00/2:  // Status / Control register
		if (ACCESSING_BITS_0_7)
		{   // Control
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_MMU, "%s: MMU Control Read: %02x & %04x\n", machine().describe_context(), m_mmu.control, mem_mask);
			return m_mmu.control;
		}   // Status
		else
		{
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_MMU, "%s: MMU Status Read: %02x & %04x\n", machine().describe_context(), m_mmu.status, mem_mask);
			return m_mmu.status;
		}
	case 0x40/2:
	case 0x48/2:
	case 0x50/2:
	case 0x58/2:
	case 0x60/2:
	case 0x68/2:
	case 0x70/2:
	case 0x78/2:  // Attributes (SD0-7)
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_MMU, "%s: MMU descriptor %d attributes Read: %04x & %04x\n", machine().describe_context(), (offset - 0x20) / 4, m_mmu.desc[(offset - 0x20) / 4].attr, mem_mask);
		return m_mmu.desc[(offset - 0x20) / 4].attr;
	case 0x42/2:
	case 0x4a/2:
	case 0x52/2:
	case 0x5a/2:
	case 0x62/2:
	case 0x6a/2:
	case 0x72/2:
	case 0x7a/2:  // Segment Length (SD0-7)
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_MMU, "%s: MMU descriptor %d length Read: %04x & %04x\n", machine().describe_context(), (offset - 0x20) / 4, m_mmu.desc[(offset - 0x20) / 4].length, mem_mask);
		return m_mmu.desc[(offset - 0x20) / 4].length;
	case 0x44/2:
	case 0x4c/2:
	case 0x54/2:
	case 0x5c/2:
	case 0x64/2:
	case 0x6c/2:
	case 0x74/2:
	case 0x7c/2:  // Segment Number (SD0-7, A0=1 only)
		if (ACCESSING_BITS_0_7)
		{
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_MMU, "%s: MMU descriptor %d segment Read: %02x & %04x\n", machine().describe_context(), (offset - 0x20) / 4, m_mmu.desc[(offset - 0x20) / 4].segment, mem_mask);
			return m_mmu.desc[(offset - 0x20) / 4].segment;
		}
		break;
	case 0x46/2:
	case 0x4e/2:
	case 0x56/2:
	case 0x5e/2:
	case 0x66/2:
	case 0x6e/2:
	case 0x76/2:
	case 0x7e/2:  // Base Address (SD0-7)
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_MMU, "%s: MMU descriptor %d base Read: %04x & %04x\n", machine().describe_context(), (offset - 0x20) / 4, m_mmu.desc[(offset - 0x20) / 4].base, mem_mask);
		return m_mmu.desc[(offset - 0x20) / 4].base;
	default:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_MMU | LOG_UNKNOWN, "%s: MMU Unknown Register Read: %04x & %04x\n", machine().describe_context(), offset * 2, mem_mask);
		break;
	}

	return 0;
}

void scc68070_device::mmu_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
	// MMU: 80008000 to 8000807f
	case 0x00/2:  // Status / Control register
		if (ACCESSING_BITS_0_7)
		{   // Control
			LOGMASKED(LOG_MMU, "%s: MMU Control Write: %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_mmu.control = data & 0x00ff;
		}   // Status
		else
		{
			LOGMASKED(LOG_MMU, "%s: MMU Status (invalid) Write: %04x & %04x\n", machine().describe_context(), data, mem_mask);
		}
		break;
	case 0x40/2:
	case 0x48/2:
	case 0x50/2:
	case 0x58/2:
	case 0x60/2:
	case 0x68/2:
	case 0x70/2:
	case 0x78/2:  // Attributes (SD0-7)
		LOGMASKED(LOG_MMU, "%s: MMU descriptor %d attributes Write: %04x & %04x\n", machine().describe_context(), (offset - 0x20) / 4, data, mem_mask);
		COMBINE_DATA(&m_mmu.desc[(offset - 0x20) / 4].attr);
		break;
	case 0x42/2:
	case 0x4a/2:
	case 0x52/2:
	case 0x5a/2:
	case 0x62/2:
	case 0x6a/2:
	case 0x72/2:
	case 0x7a/2:  // Segment Length (SD0-7)
		LOGMASKED(LOG_MMU, "%s: MMU descriptor %d length Write: %04x & %04x\n", machine().describe_context(), (offset - 0x20) / 4, data, mem_mask);
		COMBINE_DATA(&m_mmu.desc[(offset - 0x20) / 4].length);
		break;
	case 0x44/2:
	case 0x4c/2:
	case 0x54/2:
	case 0x5c/2:
	case 0x64/2:
	case 0x6c/2:
	case 0x74/2:
	case 0x7c/2:  // Segment Number (SD0-7, A0=1 only)
		if (ACCESSING_BITS_0_7)
		{
			LOGMASKED(LOG_MMU, "%s: MMU descriptor %d segment Write: %04x & %04x\n", machine().describe_context(), (offset - 0x20) / 4, data, mem_mask);
			m_mmu.desc[(offset - 0x20) / 4].segment = data & 0x00ff;
		}
		break;
	case 0x46/2:
	case 0x4e/2:
	case 0x56/2:
	case 0x5e/2:
	case 0x66/2:
	case 0x6e/2:
	case 0x76/2:
	case 0x7e/2:  // Base Address (SD0-7)
		LOGMASKED(LOG_MMU, "%s: MMU descriptor %d base Write: %04x & %04x\n", machine().describe_context(), (offset - 0x20) / 4, data, mem_mask);
		COMBINE_DATA(&m_mmu.desc[(offset - 0x20) / 4].base);
		break;
	default:
		LOGMASKED(LOG_MMU | LOG_UNKNOWN, "%s: Unknown Register Write: %04x = %04x & %04x\n", machine().describe_context(), offset * 2, data, mem_mask);
		break;
	}
}

#if ENABLE_UART_PRINTING
uint16_t scc68070_device::uart_loopback_enable()
{
	return 0x1234;
}
#endif
