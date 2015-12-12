// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    CD-i-specific cdi68070 SoC peripheral emulation
    -------------------

    written by Ryan Holtz


*******************************************************************************

STATUS:

- Skeleton.  Just enough for the CD-i to run.

TODO:

- Proper handling of the 68070's internal devices (UART, DMA, Timers, etc.)

*******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/cdi070.h"
#include "includes/cdi.h"

// device type definition
const device_type MACHINE_CDI68070 = &device_creator<cdi68070_device>;

#if ENABLE_VERBOSE_LOG
static inline void ATTR_PRINTF(1,2) verboselog(running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%08x: %s", machine.device("maincpu")->safe_pc(), buf );
	}
}
#else
#define verboselog(x,y,z, ...)
#endif

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdi68070_device - constructor
//-------------------------------------------------

cdi68070_device::cdi68070_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MACHINE_CDI68070, "CDI68070", tag, owner, clock, "cdi68070", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdi68070_device::device_start()
{
	save_item(NAME(m_lir));

	save_item(NAME(m_picr1));
	save_item(NAME(m_picr2));

	save_item(NAME(m_i2c.data_register));
	save_item(NAME(m_i2c.address_register));
	save_item(NAME(m_i2c.status_register));
	save_item(NAME(m_i2c.control_register));
	save_item(NAME(m_i2c.clock_control_register));

	save_item(NAME(m_uart.mode_register));
	save_item(NAME(m_uart.status_register));
	save_item(NAME(m_uart.clock_select));
	save_item(NAME(m_uart.command_register));
	save_item(NAME(m_uart.transmit_holding_register));
	save_item(NAME(m_uart.receive_holding_register));

	save_item(NAME(m_timers.timer_status_register));
	save_item(NAME(m_timers.timer_control_register));
	save_item(NAME(m_timers.reload_register));
	save_item(NAME(m_timers.timer0));
	save_item(NAME(m_timers.timer1));
	save_item(NAME(m_timers.timer2));

	save_item(NAME(m_dma.channel[0].channel_status));
	save_item(NAME(m_dma.channel[0].channel_error));
	save_item(NAME(m_dma.channel[0].device_control));
	save_item(NAME(m_dma.channel[0].operation_control));
	save_item(NAME(m_dma.channel[0].sequence_control));
	save_item(NAME(m_dma.channel[0].channel_control));
	save_item(NAME(m_dma.channel[0].transfer_counter));
	save_item(NAME(m_dma.channel[0].memory_address_counter));
	save_item(NAME(m_dma.channel[0].device_address_counter));
	save_item(NAME(m_dma.channel[1].channel_status));
	save_item(NAME(m_dma.channel[1].channel_error));
	save_item(NAME(m_dma.channel[1].device_control));
	save_item(NAME(m_dma.channel[1].operation_control));
	save_item(NAME(m_dma.channel[1].sequence_control));
	save_item(NAME(m_dma.channel[1].channel_control));
	save_item(NAME(m_dma.channel[1].transfer_counter));
	save_item(NAME(m_dma.channel[1].memory_address_counter));
	save_item(NAME(m_dma.channel[1].device_address_counter));

	save_item(NAME(m_mmu.status));
	save_item(NAME(m_mmu.control));
	save_item(NAME(m_mmu.desc[0].attr));
	save_item(NAME(m_mmu.desc[0].length));
	save_item(NAME(m_mmu.desc[0].segment));
	save_item(NAME(m_mmu.desc[0].base));
	save_item(NAME(m_mmu.desc[1].attr));
	save_item(NAME(m_mmu.desc[1].length));
	save_item(NAME(m_mmu.desc[1].segment));
	save_item(NAME(m_mmu.desc[1].base));
	save_item(NAME(m_mmu.desc[2].attr));
	save_item(NAME(m_mmu.desc[2].length));
	save_item(NAME(m_mmu.desc[2].segment));
	save_item(NAME(m_mmu.desc[2].base));
	save_item(NAME(m_mmu.desc[3].attr));
	save_item(NAME(m_mmu.desc[3].length));
	save_item(NAME(m_mmu.desc[3].segment));
	save_item(NAME(m_mmu.desc[3].base));
	save_item(NAME(m_mmu.desc[4].attr));
	save_item(NAME(m_mmu.desc[4].length));
	save_item(NAME(m_mmu.desc[4].segment));
	save_item(NAME(m_mmu.desc[4].base));
	save_item(NAME(m_mmu.desc[5].attr));
	save_item(NAME(m_mmu.desc[5].length));
	save_item(NAME(m_mmu.desc[5].segment));
	save_item(NAME(m_mmu.desc[5].base));
	save_item(NAME(m_mmu.desc[6].attr));
	save_item(NAME(m_mmu.desc[6].length));
	save_item(NAME(m_mmu.desc[6].segment));
	save_item(NAME(m_mmu.desc[6].base));
	save_item(NAME(m_mmu.desc[7].attr));
	save_item(NAME(m_mmu.desc[7].length));
	save_item(NAME(m_mmu.desc[7].segment));
	save_item(NAME(m_mmu.desc[7].base));

	m_timers.timer0_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cdi68070_device::timer0_callback), this));
	m_timers.timer0_timer->adjust(attotime::never);

	m_uart.rx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cdi68070_device::rx_callback), this));
	m_uart.rx_timer->adjust(attotime::never);

	m_uart.tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cdi68070_device::tx_callback), this));
	m_uart.tx_timer->adjust(attotime::never);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdi68070_device::device_reset()
{
	m_lir = 0;

	m_picr1 = 0;
	m_picr2 = 0;

	m_i2c.data_register = 0;
	m_i2c.address_register = 0;
	m_i2c.status_register = 0;
	m_i2c.control_register = 0;
	m_i2c.clock_control_register = 0;

	m_uart.mode_register = 0;
	m_uart.status_register = USR_TXRDY;
	m_uart.clock_select = 0;
	m_uart.command_register = 0;
	m_uart.transmit_holding_register = 0;
	m_uart.receive_holding_register = 0;
	m_uart.receive_pointer = -1;
	m_uart.transmit_pointer = -1;

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

	memset(m_seeds, 0, 10 * sizeof(UINT16));
	memset(m_state, 0, 8 * sizeof(UINT8));
	m_mcu_value = 0;
	m_mcu_ack = 0;
}

void cdi68070_device::set_timer_callback(int channel)
{
	switch(channel)
	{
		case 0:
		{
			UINT32 compare = 0x10000 - m_timers.timer0;
			attotime period = attotime::from_hz(CLOCK_A/192) * compare;
			m_timers.timer0_timer->adjust(period);
			break;
		}
		default:
		{
			fatalerror( "Unsupported timer channel to set_timer_callback!\n" );
		}
	}
}

void cdi68070_device::set_quizard_mcu_ack(UINT8 ack)
{
	m_mcu_ack = ack;
}

void cdi68070_device::set_quizard_mcu_value(UINT16 value)
{
	m_mcu_value = value;
}

TIMER_CALLBACK_MEMBER( cdi68070_device::timer0_callback )
{
	cdi_state *state = machine().driver_data<cdi_state>();

	m_timers.timer0 = m_timers.reload_register;
	m_timers.timer_status_register |= TSR_OV0;
	if(m_picr1 & 7)
	{
		UINT8 interrupt = m_picr1 & 7;
		m_timers.timer_status_register |= TSR_OV0;
		if(interrupt)
		{
			state->m_maincpu->set_input_line_vector(M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
			state->m_maincpu->set_input_line(M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
		}
	}

	set_timer_callback(0);
}

void cdi68070_device::uart_rx_check()
{
	if((m_uart.command_register & 3) == 1)
	{
		UINT32 div = 0x10000 >> ((m_uart.clock_select >> 4) & 7);
		m_uart.rx_timer->adjust(attotime::from_hz((49152000 / div) / 8));
	}
	else
	{
		m_uart.status_register &= ~USR_RXRDY;
		m_uart.rx_timer->adjust(attotime::never);
	}
}

void cdi68070_device::uart_tx_check()
{
	if(((m_uart.command_register >> 2) & 3) == 1)
	{
		if(m_uart.transmit_pointer >= 0)
		{
			m_uart.status_register &= ~USR_TXRDY;
		}
		else
		{
			m_uart.status_register |= USR_TXRDY;
		}

		if(m_uart.tx_timer->remaining() == attotime::never)
		{
			UINT32 div = 0x10000 >> (m_uart.clock_select & 7);
			m_uart.tx_timer->adjust(attotime::from_hz((49152000 / div) / 8));
		}
	}
	else
	{
		m_uart.tx_timer->adjust(attotime::never);
	}
}

void cdi68070_device::uart_rx(UINT8 data)
{
	m_uart.receive_pointer++;
	m_uart.receive_buffer[m_uart.receive_pointer] = data;
	uart_rx_check();
}

void cdi68070_device::uart_tx(UINT8 data)
{
	m_uart.transmit_pointer++;
	m_uart.transmit_buffer[m_uart.transmit_pointer] = data;
	uart_tx_check();
}

TIMER_CALLBACK_MEMBER( cdi68070_device::rx_callback )
{
	cdi_state *state = machine().driver_data<cdi_state>();

	if((m_uart.command_register & 3) == 1)
	{
		if(m_uart.receive_pointer >= 0)
		{
			m_uart.status_register |= USR_RXRDY;
		}
		else
		{
			m_uart.status_register &= ~USR_RXRDY;
		}

		m_uart.receive_holding_register = m_uart.receive_buffer[0];

		if(m_uart.receive_pointer > -1)
		{
			verboselog(machine(), 2, "scc68070_rx_callback: Receiving %02x\n", m_uart.receive_holding_register);

			UINT8 interrupt = (m_picr2 >> 4) & 7;
			if(interrupt)
			{
				state->m_maincpu->set_input_line_vector(M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
				state->m_maincpu->set_input_line(M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
			}

			m_uart.status_register |= USR_RXRDY;
			UINT32 div = 0x10000 >> ((m_uart.clock_select >> 4) & 7);
			m_uart.rx_timer->adjust(attotime::from_hz((49152000 / div) / 8));
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

	uart_rx_check();
}

void cdi68070_device::quizard_rx(UINT8 data)
{
	uart_rx(0x5a);
	uart_rx(data);
}

void cdi68070_device::quizard_set_seeds(UINT8 *rx)
{
	m_seeds[0] = (rx[1] << 8) | rx[0];
	m_seeds[1] = (rx[3] << 8) | rx[2];
	m_seeds[2] = (rx[5] << 8) | rx[4];
	m_seeds[3] = (rx[7] << 8) | rx[6];
	m_seeds[4] = (rx[9] << 8) | rx[8];
	m_seeds[5] = (rx[11] << 8) | rx[10];
	m_seeds[6] = (rx[13] << 8) | rx[12];
	m_seeds[7] = (rx[15] << 8) | rx[14];
	m_seeds[8] = (rx[17] << 8) | rx[16];
	m_seeds[9] = (rx[19] << 8) | rx[18];
}

void cdi68070_device::quizard_calculate_state()
{
	//const UINT16 desired_bitfield = mcu_value;
	const UINT16 field0 = 0x00ff;
	const UINT16 field1 = m_mcu_value ^ 0x00ff;

	UINT16 total0 = 0;
	UINT16 total1 = 0;

	for(int index = 0; index < 10; index++)
	{
		if(field0 & (1 << index))
		{
			total0 += m_seeds[index];
		}
		if(field1 & (1 << index))
		{
			total1 += m_seeds[index];
		}
	}

	UINT16 hi0 = (total0 >> 8) + 0x40;
	m_state[2] = hi0 / 2;
	m_state[3] = hi0 - m_state[2];

	UINT16 lo0 = (total0 & 0x00ff) + 0x40;
	m_state[0] = lo0 / 2;
	m_state[1] = lo0 - m_state[0];

	UINT16 hi1 = (total1 >> 8) + 0x40;
	m_state[6] = hi1 / 2;
	m_state[7] = hi1 - m_state[6];

	UINT16 lo1 = (total1 & 0x00ff) + 0x40;
	m_state[4] = lo1 / 2;
	m_state[5] = lo1 - m_state[4];
}

void cdi68070_device::mcu_frame()
{
	if(0)//mcu_active)
	{
		quizard_calculate_state();
		uart_rx(0x5a);
		for(auto & elem : m_state)
		{
			uart_rx(elem);
		}
	}
}

void cdi68070_device::quizard_handle_byte_tx()
{
	static int state = 0;
	static UINT8 rx[0x100];
	static UINT8 rx_ptr = 0xff;
	UINT8 tx = m_uart.transmit_holding_register;

	switch(state)
	{
		case 0: // Waiting for a leadoff byte
			if(tx == m_mcu_ack) // Sequence end
			{
				//scc68070_uart_rx(machine, scc68070, 0x5a);
				//scc68070_uart_rx(machine, scc68070, 0x42);
			}
			else
			{
				switch(tx)
				{
					case 0x44: // DATABASEPATH = **_DATABASE/
						rx[0] = 0x44;
						rx_ptr = 1;
						state = 3;
						break;
					case 0x2e: // Unknown; ignored
						break;
					case 0x56: // Seed start
						rx_ptr = 0;
						state = 1;
						break;
					default:
						//printf("Unknown leadoff byte: %02x\n", tx);
						break;
				}
			}
			break;

		case 1: // Receiving the seed
			rx[rx_ptr] = tx;
			rx_ptr++;
			if(rx_ptr == 20)
			{
				//printf("Calculating seeds\n");
				quizard_set_seeds(rx);
				quizard_calculate_state();
				state = 2;
			}
			break;

		case 2: // Receiving the seed acknowledge
		case 4:
			if(tx == m_mcu_ack)
			{
				if(state == 2)
				{
					state = 4;
				}
				else
				{
					state = 0;
				}
				//printf("Sending seed ack\n");
				uart_rx(0x5a);
				uart_rx(m_state[0]);
				uart_rx(m_state[1]);
				uart_rx(m_state[2]);
				uart_rx(m_state[3]);
				uart_rx(m_state[4]);
				uart_rx(m_state[5]);
				uart_rx(m_state[6]);
				uart_rx(m_state[7]);
			}
			break;

		case 3: // Receiving the database path
			rx[rx_ptr] = tx;
			rx_ptr++;
			if(tx == 0x0a)
			{
				/*rx[rx_ptr] = 0;
				//printf("Database path: %s\n", rx);
				scc68070_uart_rx(machine, scc68070, 0x5a);
				scc68070_uart_rx(machine, scc68070, g_state[0]);
				scc68070_uart_rx(machine, scc68070, g_state[1]);
				scc68070_uart_rx(machine, scc68070, g_state[2]);
				scc68070_uart_rx(machine, scc68070, g_state[3]);
				scc68070_uart_rx(machine, scc68070, g_state[4]);
				scc68070_uart_rx(machine, scc68070, g_state[5]);
				scc68070_uart_rx(machine, scc68070, g_state[6]);
				scc68070_uart_rx(machine, scc68070, g_state[7]);*/
				state = 0;
			}
			break;
	}
}

TIMER_CALLBACK_MEMBER( cdi68070_device::tx_callback )
{
	cdi_state *state = machine().driver_data<cdi_state>();

	if(((m_uart.command_register >> 2) & 3) == 1)
	{
		UINT8 interrupt = m_picr2 & 7;
		if(interrupt)
		{
			state->m_maincpu->set_input_line_vector(M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
			state->m_maincpu->set_input_line(M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
		}

		if(m_uart.transmit_pointer > -1)
		{
			m_uart.transmit_holding_register = m_uart.transmit_buffer[0];
			quizard_handle_byte_tx();

			verboselog(machine(), 2, "cdi68070_tx_callback: Transmitting %02x\n", m_uart.transmit_holding_register);
			for(int index = 0; index < m_uart.transmit_pointer; index++)
			{
				m_uart.transmit_buffer[index] = m_uart.transmit_buffer[index+1];
			}
			m_uart.transmit_pointer--;

			UINT32 div = 0x10000 >> (m_uart.clock_select & 7);
			m_uart.tx_timer->adjust(attotime::from_hz((49152000 / div) / 8));
		}
		else
		{
			m_uart.tx_timer->adjust(attotime::never);
		}
	}
	else
	{
		m_uart.tx_timer->adjust(attotime::never);
	}

	uart_tx_check();
}

READ16_MEMBER( cdi68070_device::periphs_r )
{
	cdi_state *state = machine().driver_data<cdi_state>();

	switch(offset)
	{
		// Interrupts: 80001001
		case 0x1000/2: // LIR priority level
			return m_lir;

		// I2C interface: 80002001 to 80002009
		case 0x2000/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: I2C Data Register: %04x & %04x\n", m_i2c.data_register, mem_mask);
			}
			return m_i2c.data_register;
		case 0x2002/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: I2C Address Register: %04x & %04x\n", m_i2c.address_register, mem_mask);
			}
			return m_i2c.address_register;
		case 0x2004/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: I2C Status Register: %04x & %04x\n", m_i2c.status_register, mem_mask);
			}
			return m_i2c.status_register;
		case 0x2006/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: I2C Control Register: %04x & %04x\n", m_i2c.control_register, mem_mask);
			}
			return m_i2c.control_register;
		case 0x2008/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: I2C Clock Control Register: %04x & %04x\n", m_i2c.clock_control_register, mem_mask);
			}
			return m_i2c.clock_control_register;

		// UART interface: 80002011 to 8000201b
		case 0x2010/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: UART Mode Register: %04x & %04x\n", m_uart.mode_register, mem_mask);
			}
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}
			return m_uart.mode_register | 0x20;
		case 0x2012/2:
			m_uart.status_register |= (1 << 1);
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: UART Status Register: %04x & %04x\n", m_uart.status_register, mem_mask);
			}
			else
			{
				verboselog(space.machine(), 0, "cdi68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}

			if((m_picr2 >> 4) & 7)
			{
				state->m_maincpu->set_input_line(M68K_IRQ_1 + (((m_picr2 >> 4) & 7) - 1), ASSERT_LINE);
			}

			if(m_picr2 & 7)
			{
				state->m_maincpu->set_input_line(M68K_IRQ_1 + ((m_picr2 & 7) - 1), ASSERT_LINE);
			}

			return m_uart.status_register;

		case 0x2014/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: UART Clock Select: %04x & %04x\n", m_uart.clock_select, mem_mask);
			}
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}
			return m_uart.clock_select | 0x08;
		case 0x2016/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: UART Command Register: %02x & %04x\n", m_uart.command_register, mem_mask);
			}
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}
			return m_uart.command_register | 0x80;
		case 0x2018/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: UART Transmit Holding Register: %02x & %04x\n", m_uart.transmit_holding_register, mem_mask);
			}
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}
			return m_uart.transmit_holding_register;
		case 0x201a/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: UART Receive Holding Register: %02x & %04x\n", m_uart.receive_holding_register, mem_mask);
			}
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}
			if((m_picr2 >> 4) & 7)
			{
				state->m_maincpu->set_input_line(M68K_IRQ_1 + (((m_picr2 >> 4) & 7) - 1), CLEAR_LINE);
			}

			m_uart.receive_holding_register = m_uart.receive_buffer[0];
			if(m_uart.receive_pointer >= 0)
			{
				for(int index = 0; index < m_uart.receive_pointer; index++)
				{
					m_uart.receive_buffer[index] = m_uart.receive_buffer[index + 1];
				}
				m_uart.receive_pointer--;
			}
			//printf("R: %02x\n", m_uart.receive_holding_register);
			return m_uart.receive_holding_register;

		// Timers: 80002020 to 80002029
		case 0x2020/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: Timer Control Register: %02x & %04x\n", m_timers.timer_control_register, mem_mask);
			}
			if(ACCESSING_BITS_8_15)
			{
				verboselog(machine(), 12, "cdi68070_periphs_r: Timer Status Register: %02x & %04x\n", m_timers.timer_status_register, mem_mask);
			}
			return (m_timers.timer_status_register << 8) | m_timers.timer_control_register;
		case 0x2022/2:
			verboselog(machine(), 2, "cdi68070_periphs_r: Timer Reload Register: %04x & %04x\n", m_timers.reload_register, mem_mask);
			return m_timers.reload_register;
		case 0x2024/2:
			verboselog(machine(), 2, "cdi68070_periphs_r: Timer 0: %04x & %04x\n", m_timers.timer0, mem_mask);
			return m_timers.timer0;
		case 0x2026/2:
			verboselog(machine(), 2, "cdi68070_periphs_r: Timer 1: %04x & %04x\n", m_timers.timer1, mem_mask);
			return m_timers.timer1;
		case 0x2028/2:
			verboselog(machine(), 2, "cdi68070_periphs_r: Timer 2: %04x & %04x\n", m_timers.timer2, mem_mask);
			return m_timers.timer2;

		// PICR1: 80002045
		case 0x2044/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: Peripheral Interrupt Control Register 1: %02x & %04x\n", m_picr1, mem_mask);
			}
			return m_picr1;

		// PICR2: 80002047
		case 0x2046/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: Peripheral Interrupt Control Register 2: %02x & %04x\n", m_picr2, mem_mask);
			}
			return m_picr2 & 0x77;

		// DMA controller: 80004000 to 8000406d
		case 0x4000/2:
		case 0x4040/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: DMA(%d) Error Register: %04x & %04x\n", (offset - 0x2000) / 32, m_dma.channel[(offset - 0x2000) / 32].channel_error, mem_mask);
			}
			if(ACCESSING_BITS_8_15)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: DMA(%d) Status Register: %04x & %04x\n", (offset - 0x2000) / 32, m_dma.channel[(offset - 0x2000) / 32].channel_status, mem_mask);
			}
			return (m_dma.channel[(offset - 0x2000) / 32].channel_status << 8) | m_dma.channel[(offset - 0x2000) / 32].channel_error;
		case 0x4004/2:
		case 0x4044/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: DMA(%d) Operation Control Register: %02x & %04x\n", (offset - 0x2000) / 32, m_dma.channel[(offset - 0x2000) / 32].operation_control, mem_mask);
			}
			if(ACCESSING_BITS_8_15)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: DMA(%d) Device Control Register: %02x & %04x\n", (offset - 0x2000) / 32, m_dma.channel[(offset - 0x2000) / 32].device_control, mem_mask);
			}
			return (m_dma.channel[(offset - 0x2000) / 32].device_control << 8) | m_dma.channel[(offset - 0x2000) / 32].operation_control;
		case 0x4006/2:
		case 0x4046/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: DMA(%d) Channel Control Register: %02x & %04x\n", (offset - 0x2000) / 32, m_dma.channel[(offset - 0x2000) / 32].channel_control, mem_mask);
			}
			if(ACCESSING_BITS_8_15)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: DMA(%d) Sequence Control Register: %02x & %04x\n", (offset - 0x2000) / 32, m_dma.channel[(offset - 0x2000) / 32].sequence_control, mem_mask);
			}
			return (m_dma.channel[(offset - 0x2000) / 32].sequence_control << 8) | m_dma.channel[(offset - 0x2000) / 32].channel_control;
		case 0x400a/2:
			verboselog(machine(), 2, "cdi68070_periphs_r: DMA(%d) Memory Transfer Counter: %04x & %04x\n", (offset - 0x2000) / 32, m_dma.channel[(offset - 0x2000) / 32].transfer_counter, mem_mask);
			return m_dma.channel[(offset - 0x2000) / 32].transfer_counter;
		case 0x400c/2:
		case 0x404c/2:
			verboselog(machine(), 2, "cdi68070_periphs_r: DMA(%d) Memory Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, (m_dma.channel[(offset - 0x2000) / 32].memory_address_counter >> 16), mem_mask);
			return (m_dma.channel[(offset - 0x2000) / 32].memory_address_counter >> 16);
		case 0x400e/2:
		case 0x404e/2:
			verboselog(machine(), 2, "cdi68070_periphs_r: DMA(%d) Memory Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, m_dma.channel[(offset - 0x2000) / 32].memory_address_counter, mem_mask);
			return m_dma.channel[(offset - 0x2000) / 32].memory_address_counter;
		case 0x4014/2:
		case 0x4054/2:
			verboselog(machine(), 2, "cdi68070_periphs_r: DMA(%d) Device Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, (m_dma.channel[(offset - 0x2000) / 32].device_address_counter >> 16), mem_mask);
			return (m_dma.channel[(offset - 0x2000) / 32].device_address_counter >> 16);
		case 0x4016/2:
		case 0x4056/2:
			verboselog(machine(), 2, "cdi68070_periphs_r: DMA(%d) Device Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, m_dma.channel[(offset - 0x2000) / 32].device_address_counter, mem_mask);
			return m_dma.channel[(offset - 0x2000) / 32].device_address_counter;

		// MMU: 80008000 to 8000807f
		case 0x8000/2:  // Status / Control register
			if(ACCESSING_BITS_0_7)
			{   // Control
				verboselog(machine(), 2, "cdi68070_periphs_r: MMU Control: %02x & %04x\n", m_mmu.control, mem_mask);
				return m_mmu.control;
			}   // Status
			else
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: MMU Status: %02x & %04x\n", m_mmu.status, mem_mask);
				return m_mmu.status;
			}
		case 0x8040/2:
		case 0x8048/2:
		case 0x8050/2:
		case 0x8058/2:
		case 0x8060/2:
		case 0x8068/2:
		case 0x8070/2:
		case 0x8078/2:  // Attributes (SD0-7)
			verboselog(machine(), 2, "cdi68070_periphs_r: MMU descriptor %d attributes: %04x & %04x\n", (offset - 0x4020) / 4, m_mmu.desc[(offset - 0x4020) / 4].attr, mem_mask);
			return m_mmu.desc[(offset - 0x4020) / 4].attr;
		case 0x8042/2:
		case 0x804a/2:
		case 0x8052/2:
		case 0x805a/2:
		case 0x8062/2:
		case 0x806a/2:
		case 0x8072/2:
		case 0x807a/2:  // Segment Length (SD0-7)
			verboselog(machine(), 2, "cdi68070_periphs_r: MMU descriptor %d length: %04x & %04x\n", (offset - 0x4020) / 4, m_mmu.desc[(offset - 0x4020) / 4].length, mem_mask);
			return m_mmu.desc[(offset - 0x4020) / 4].length;
		case 0x8044/2:
		case 0x804c/2:
		case 0x8054/2:
		case 0x805c/2:
		case 0x8064/2:
		case 0x806c/2:
		case 0x8074/2:
		case 0x807c/2:  // Segment Number (SD0-7, A0=1 only)
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_r: MMU descriptor %d segment: %02x & %04x\n", (offset - 0x4020) / 4, m_mmu.desc[(offset - 0x4020) / 4].segment, mem_mask);
				return m_mmu.desc[(offset - 0x4020) / 4].segment;
			}
			break;
		case 0x8046/2:
		case 0x804e/2:
		case 0x8056/2:
		case 0x805e/2:
		case 0x8066/2:
		case 0x806e/2:
		case 0x8076/2:
		case 0x807e/2:  // Base Address (SD0-7)
			verboselog(machine(), 2, "cdi68070_periphs_r: MMU descriptor %d base: %04x & %04x\n", (offset - 0x4020) / 4, m_mmu.desc[(offset - 0x4020) / 4].base, mem_mask);
			return m_mmu.desc[(offset - 0x4020) / 4].base;
		default:
			verboselog(space.machine(), 0, "cdi68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			break;
	}

	return 0;
}

WRITE16_MEMBER( cdi68070_device::periphs_w )
{
	cdi_state *state = machine().driver_data<cdi_state>();

	switch(offset)
	{
		// Interrupts: 80001001
		case 0x1000/2: // LIR priority level
			verboselog(machine(), 2, "cdi68070_periphs_w: LIR: %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_lir);
			break;

		// I2C interface: 80002001 to 80002009
		case 0x2000/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: I2C Data Register: %04x & %04x\n", data, mem_mask);
				m_i2c.data_register = data & 0x00ff;
			}
			break;
		case 0x2002/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: I2C Address Register: %04x & %04x\n", data, mem_mask);
				m_i2c.address_register = data & 0x00ff;
			}
			break;
		case 0x2004/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: I2C Status Register: %04x & %04x\n", data, mem_mask);
				m_i2c.status_register = data & 0x00ff;
			}
			break;
		case 0x2006/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: I2C Control Register: %04x & %04x\n", data, mem_mask);
				m_i2c.control_register = data & 0x00ff;
			}
			break;
		case 0x2008/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: I2C Clock Control Register: %04x & %04x\n", data, mem_mask);
				m_i2c.clock_control_register = data & 0x00ff;
			}
			break;

		// UART interface: 80002011 to 8000201b
		case 0x2010/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: UART Mode Register: %04x & %04x\n", data, mem_mask);
				m_uart.mode_register = data & 0x00ff;
			}
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
			break;
		case 0x2012/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: UART Status Register: %04x & %04x\n", data, mem_mask);
				m_uart.status_register = data & 0x00ff;
			}
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
			break;
		case 0x2014/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: UART Clock Select: %04x & %04x\n", data, mem_mask);
				m_uart.clock_select = data & 0x00ff;
			}
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
			break;
		case 0x2016/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: UART Command Register: %04x & %04x\n", data, mem_mask);
				m_uart.command_register = data & 0x00ff;
				uart_rx_check();
				uart_tx_check();
			}
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
			break;
		case 0x2018/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: UART Transmit Holding Register: %04x & %04x: %c\n", data, mem_mask, (data >= 0x20 && data < 0x7f) ? (data & 0x00ff) : ' ');
				uart_tx(data & 0x00ff);
				m_uart.transmit_holding_register = data & 0x00ff;
			}
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
			break;
		case 0x201a/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: UART Receive Holding Register: %04x & %04x\n", data, mem_mask);
				m_uart.receive_holding_register = data & 0x00ff;
			}
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
			break;

		// Timers: 80002020 to 80002029
		case 0x2020/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: Timer Control Register: %04x & %04x\n", data, mem_mask);
				m_timers.timer_control_register = data & 0x00ff;
			}
			if(ACCESSING_BITS_8_15)
			{
				verboselog(machine(), 12, "cdi68070_periphs_w: Timer Status Register: %04x & %04x\n", data, mem_mask);
				m_timers.timer_status_register &= ~(data >> 8);
				if(!m_timers.timer_status_register)
				{
					UINT8 interrupt = m_picr1 & 7;
					state->m_maincpu->set_input_line(M68K_IRQ_1 + (interrupt - 1), CLEAR_LINE);
				}
			}
			break;
		case 0x2022/2:
			verboselog(machine(), 2, "cdi68070_periphs_w: Timer Reload Register: %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_timers.reload_register);
			break;
		case 0x2024/2:
			verboselog(machine(), 2, "cdi68070_periphs_w: Timer 0: %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_timers.timer0);
			set_timer_callback(0);
			break;
		case 0x2026/2:
			verboselog(machine(), 2, "cdi68070_periphs_w: Timer 1: %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_timers.timer1);
			break;
		case 0x2028/2:
			verboselog(machine(), 2, "cdi68070_periphs_w: Timer 2: %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_timers.timer2);
			break;

		// PICR1: 80002045
		case 0x2044/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: Peripheral Interrupt Control Register 1: %04x & %04x\n", data, mem_mask);
				m_picr1 = data & 0x00ff;
			}
			break;

		// PICR2: 80002047
		case 0x2046/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: Peripheral Interrupt Control Register 2: %04x & %04x\n", data, mem_mask);
				m_picr2 = data & 0x00ff;
			}
			break;

		// DMA controller: 80004000 to 8000406d
		case 0x4000/2:
		case 0x4040/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: DMA(%d) Error (invalid): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
			}
			if(ACCESSING_BITS_8_15)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: DMA(%d) Status: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
				m_dma.channel[(offset - 0x2000) / 32].channel_status &= ~(data & 0xb0);
			}
			break;
		case 0x4004/2:
		case 0x4044/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: DMA(%d) Operation Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
				m_dma.channel[(offset - 0x2000) / 32].operation_control = data & 0x00ff;
			}
			if(ACCESSING_BITS_8_15)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: DMA(%d) Device Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
				m_dma.channel[(offset - 0x2000) / 32].device_control = data >> 8;
			}
			break;
		case 0x4006/2:
		case 0x4046/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: DMA(%d) Channel Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
				m_dma.channel[(offset - 0x2000) / 32].channel_control = data & 0x007f;
				if(data & CCR_SO)
				{
					m_dma.channel[(offset - 0x2000) / 32].channel_status |= CSR_COC;
				}
			}
			if(ACCESSING_BITS_8_15)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: DMA(%d) Sequence Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
				m_dma.channel[(offset - 0x2000) / 32].sequence_control = data >> 8;
			}
			break;
		case 0x400a/2:
			verboselog(machine(), 2, "cdi68070_periphs_w: DMA(%d) Memory Transfer Counter: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
			COMBINE_DATA(&m_dma.channel[(offset - 0x2000) / 32].transfer_counter);
			break;
		case 0x400c/2:
		case 0x404c/2:
			verboselog(machine(), 2, "cdi68070_periphs_w: DMA(%d) Memory Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
			m_dma.channel[(offset - 0x2000) / 32].memory_address_counter &= ~(mem_mask << 16);
			m_dma.channel[(offset - 0x2000) / 32].memory_address_counter |= data << 16;
			break;
		case 0x400e/2:
		case 0x404e/2:
			verboselog(machine(), 2, "cdi68070_periphs_w: DMA(%d) Memory Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
			m_dma.channel[(offset - 0x2000) / 32].memory_address_counter &= ~mem_mask;
			m_dma.channel[(offset - 0x2000) / 32].memory_address_counter |= data;
			break;
		case 0x4014/2:
		case 0x4054/2:
			verboselog(machine(), 2, "cdi68070_periphs_w: DMA(%d) Device Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
			m_dma.channel[(offset - 0x2000) / 32].device_address_counter &= ~(mem_mask << 16);
			m_dma.channel[(offset - 0x2000) / 32].device_address_counter |= data << 16;
			break;
		case 0x4016/2:
		case 0x4056/2:
			verboselog(machine(), 2, "cdi68070_periphs_w: DMA(%d) Device Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
			m_dma.channel[(offset - 0x2000) / 32].device_address_counter &= ~mem_mask;
			m_dma.channel[(offset - 0x2000) / 32].device_address_counter |= data;
			break;

		// MMU: 80008000 to 8000807f
		case 0x8000/2:  // Status / Control register
			if(ACCESSING_BITS_0_7)
			{   // Control
				verboselog(machine(), 2, "cdi68070_periphs_w: MMU Control: %04x & %04x\n", data, mem_mask);
				m_mmu.control = data & 0x00ff;
			}   // Status
			else
			{
				verboselog(machine(), 0, "cdi68070_periphs_w: MMU Status (invalid): %04x & %04x\n", data, mem_mask);
			}
			break;
		case 0x8040/2:
		case 0x8048/2:
		case 0x8050/2:
		case 0x8058/2:
		case 0x8060/2:
		case 0x8068/2:
		case 0x8070/2:
		case 0x8078/2:  // Attributes (SD0-7)
			verboselog(machine(), 2, "cdi68070_periphs_w: MMU descriptor %d attributes: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
			COMBINE_DATA(&m_mmu.desc[(offset - 0x4020) / 4].attr);
			break;
		case 0x8042/2:
		case 0x804a/2:
		case 0x8052/2:
		case 0x805a/2:
		case 0x8062/2:
		case 0x806a/2:
		case 0x8072/2:
		case 0x807a/2:  // Segment Length (SD0-7)
			verboselog(machine(), 2, "cdi68070_periphs_w: MMU descriptor %d length: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
			COMBINE_DATA(&m_mmu.desc[(offset - 0x4020) / 4].length);
			break;
		case 0x8044/2:
		case 0x804c/2:
		case 0x8054/2:
		case 0x805c/2:
		case 0x8064/2:
		case 0x806c/2:
		case 0x8074/2:
		case 0x807c/2:  // Segment Number (SD0-7, A0=1 only)
			if(ACCESSING_BITS_0_7)
			{
				verboselog(machine(), 2, "cdi68070_periphs_w: MMU descriptor %d segment: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
				m_mmu.desc[(offset - 0x4020) / 4].segment = data & 0x00ff;
			}
			break;
		case 0x8046/2:
		case 0x804e/2:
		case 0x8056/2:
		case 0x805e/2:
		case 0x8066/2:
		case 0x806e/2:
		case 0x8076/2:
		case 0x807e/2:  // Base Address (SD0-7)
			verboselog(machine(), 2, "cdi68070_periphs_w: MMU descriptor %d base: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
			COMBINE_DATA(&m_mmu.desc[(offset - 0x4020) / 4].base);
			break;
		default:
			verboselog(machine(), 0, "cdi68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			break;
	}
}

#if ENABLE_UART_PRINTING
READ16_MEMBER( cdi68070_device::uart_loopback_enable )
{
	return 0x1234;
}
#endif
