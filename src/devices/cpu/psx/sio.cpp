// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation Serial I/O emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#include "emu.h"
#include "sio.h"

#define LOG_TIMER (1U << 1)

#define VERBOSE ( 0 )
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PSX_SIO0, psxsio0_device, "psxsio0", "Sony PSX SIO-0")
DEFINE_DEVICE_TYPE(PSX_SIO1, psxsio1_device, "psxsio1", "Sony PSX SIO-1")

psxsio0_device::psxsio0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	psxsio_device(mconfig, PSX_SIO0, tag, owner, clock)
{
}

psxsio1_device::psxsio1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	psxsio_device(mconfig, PSX_SIO1, tag, owner, clock)
{
}

psxsio_device::psxsio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_status(SIO_STATUS_TX_EMPTY | SIO_STATUS_TX_RDY), m_mode(0), m_control(0), m_baud(0),
	m_rxd(1), m_tx_data(0), m_rx_data(0), m_tx_shift(0), m_rx_shift(0), m_tx_bits(0), m_rx_bits(0), m_timer(nullptr),
	m_irq_handler(*this),
	m_sck_handler(*this),
	m_txd_handler(*this),
	m_dtr_handler(*this),
	m_rts_handler(*this)
{
}

void psxsio_device::device_post_load()
{
	sio_timer_adjust();
}

void psxsio_device::device_start()
{
	m_timer = timer_alloc(FUNC( psxsio_device::sio_tick ), this);
	m_mode = 0;
	m_control = 0;
	m_baud = 0;
	m_rx_data = 0;
	m_tx_data = 0;
	m_rx_shift = 0;
	m_tx_shift = 0;
	m_rx_bits = 0;
	m_tx_bits = 0;

	save_item(NAME(m_status));
	save_item(NAME(m_mode));
	save_item(NAME(m_control));
	save_item(NAME(m_baud));
	save_item(NAME(m_rxd));
	save_item(NAME(m_rx_data));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_tx_bits));
}

void psxsio_device::sio_interrupt()
{
	LOG("sio_interrupt\n");
	m_status |= SIO_STATUS_IRQ;
	m_irq_handler(1);
}

void psxsio_device::sio_timer_adjust()
{
	attotime n_time;

	if( ( m_status & SIO_STATUS_TX_EMPTY ) == 0 || m_tx_bits != 0 )
	{
		int n_prescaler;

		switch( m_mode & 3 )
		{
		case 1:
			n_prescaler = 1;
			break;
		case 2:
			n_prescaler = 16;
			break;
		case 3:
			n_prescaler = 64;
			break;
		default:
			n_prescaler = 0;
			break;
		}

		if( m_baud != 0 && n_prescaler != 0 )
		{
			n_time = attotime::from_hz(33868800) * (n_prescaler * m_baud);
			LOGMASKED( LOG_TIMER, "sio_timer_adjust = %s ( %d x %d )\n", n_time.as_string(), n_prescaler, m_baud );
		}
		else
		{
			n_time = attotime::never;
			logerror( "sio_timer_adjust invalid baud rate ( %d x %d )\n", n_prescaler, m_baud );
		}
	}
	else
	{
		n_time = attotime::never;
		LOG( "sio_timer_adjust finished\n" );
	}

	m_timer->adjust( n_time );
}

TIMER_CALLBACK_MEMBER( psxsio_device::sio_tick )
{
	LOGMASKED( LOG_TIMER, "sio tick\n" );

	if( m_tx_bits == 0 &&
		( m_control & SIO_CONTROL_TX_ENA ) != 0 &&
		( m_status & SIO_STATUS_TX_EMPTY ) == 0 )
	{
		m_tx_bits = 8;
		m_tx_shift = m_tx_data;

		if( type() == PSX_SIO0 )
		{
			m_rx_bits = 8;
			m_rx_shift = 0;
		}

		m_status |= SIO_STATUS_TX_EMPTY;
		m_status |= SIO_STATUS_TX_RDY;
	}

	if( m_tx_bits != 0 )
	{
		if( type() == PSX_SIO0 )
		{
			m_sck_handler(0);
		}

		m_txd_handler( m_tx_shift & 1 );
		m_tx_shift >>= 1;
		m_tx_bits--;

		if( type() == PSX_SIO0 )
		{
			m_sck_handler(1);
		}

		if( m_tx_bits == 0 &&
			( m_control & SIO_CONTROL_TX_IENA ) != 0 )
		{
			sio_interrupt();
		}
	}

	if( m_rx_bits != 0 )
	{
		m_rx_shift = ( m_rx_shift >> 1 ) | ( m_rxd << 7 );
		m_rx_bits--;

		if( m_rx_bits == 0 )
		{
			if( ( m_status & SIO_STATUS_RX_RDY ) != 0 )
			{
				m_status |= SIO_STATUS_OVERRUN;
			}
			else
			{
				m_rx_data = m_rx_shift;
				m_status |= SIO_STATUS_RX_RDY;
			}

			if( ( m_control & SIO_CONTROL_RX_IENA ) != 0 )
			{
				sio_interrupt();
			}
		}
	}

	sio_timer_adjust();
}

void psxsio_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch( offset % 4 )
	{
	case 0:
		LOG( "%s: psx_sio_w data %02x (%08x)\n", machine().describe_context(), data, mem_mask );
		m_tx_data = data;
		m_status &= ~( SIO_STATUS_TX_RDY );
		m_status &= ~( SIO_STATUS_TX_EMPTY );
		sio_timer_adjust();
		break;
	case 1:
		logerror( "%s: psx_sio_w( %08x, %08x, %08x )\n", machine().describe_context(), offset, data, mem_mask );
		break;
	case 2:
		if( ACCESSING_BITS_0_15 )
		{
			m_mode = data & 0xffff;
			LOG( "%s: psx_sio_w mode %04x\n", machine().describe_context(), data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			LOG( "%s: psx_sio_w control %04x\n", machine().describe_context(), data >> 16 );
			m_control = data >> 16;

			if( ( m_control & SIO_CONTROL_RESET ) != 0 )
			{
				LOG( "%s: psx_sio_w reset\n", machine().describe_context() );
				m_status |= SIO_STATUS_TX_EMPTY | SIO_STATUS_TX_RDY;
				m_status &= ~( SIO_STATUS_RX_RDY | SIO_STATUS_OVERRUN | SIO_STATUS_IRQ );
				m_irq_handler(0);

				// toggle DTR to reset controllers, Star Ocean 2, at least, requires it
				// the precise mechanism of the reset is unknown
				// maybe it's related to the bottom 2 bits of control which are usually set
				m_dtr_handler(0);
				m_dtr_handler(1);

				m_tx_bits = 0;
				m_rx_bits = 0;
				m_txd_handler(1);
			}
			if( ( m_control & SIO_CONTROL_IACK ) != 0 )
			{
				LOG( "%s: psx_sio_w iack\n", machine().describe_context() );
				m_status &= ~( SIO_STATUS_IRQ );
				m_control &= ~( SIO_CONTROL_IACK );
				m_irq_handler(0);
			}
			if( ( m_control & SIO_CONTROL_DTR ) != 0 )
			{
				m_dtr_handler(0);
			}
			else
			{
				m_dtr_handler(1);
			}
		}
		break;
	case 3:
		if( ACCESSING_BITS_0_15 )
		{
			logerror( "%s: psx_sio_w( %08x, %08x, %08x )\n", machine().describe_context(), offset, data, mem_mask );
		}
		if( ACCESSING_BITS_16_31 )
		{
			m_baud = data >> 16;
			LOG( "%s: psx_sio_w baud %04x\n", machine().describe_context(), data >> 16 );
		}
		break;
	default:
		logerror( "%s: psx_sio_w( %08x, %08x, %08x )\n", machine().describe_context(), offset, data, mem_mask );
		break;
	}
}

uint32_t psxsio_device::read(offs_t offset, uint32_t mem_mask)
{
	uint32_t data;

	switch( offset % 4 )
	{
	case 0:
		data = m_rx_data;
		m_status &= ~( SIO_STATUS_RX_RDY );
		m_rx_data = 0xff;
		LOG( "%s: psx_sio_r data %02x (%08x)\n", machine().describe_context(), data, mem_mask );
		break;
	case 1:
		data = m_status;
		if( ACCESSING_BITS_0_15 )
		{
			LOG( "%s: psx_sio_r status %04x\n", machine().describe_context(), data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			logerror( "%s: psx_sio_r( %08x, %08x ) %08x\n", machine().describe_context(), offset, mem_mask, data );
		}
		break;
	case 2:
		data = ( m_control << 16 ) | m_mode;
		if( ACCESSING_BITS_0_15 )
		{
			LOG( "%s: psx_sio_r mode %04x\n", machine().describe_context(), data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			LOG( "%s: psx_sio_r control %04x\n", machine().describe_context(), data >> 16 );
		}
		break;
	case 3:
		data = m_baud << 16;
		if( ACCESSING_BITS_0_15 )
		{
			logerror( "%s: psx_sio_r( %08x, %08x ) %08x\n", machine().describe_context(), offset, mem_mask, data );
		}
		if( ACCESSING_BITS_16_31 )
		{
			LOG( "%s: psx_sio_r baud %04x\n", machine().describe_context(), data >> 16 );
		}
		break;
	default:
		data = 0;
		logerror( "%s: psx_sio_r( %08x, %08x ) %08x\n", machine().describe_context(), offset, mem_mask, data );
		break;
	}
	return data;
}

void psxsio_device::write_rxd(int state)
{
	m_rxd = state;
}

void psxsio_device::write_dsr(int state)
{
	if (state)
	{
		m_status &= ~SIO_STATUS_DSR;
	}
	else if ((m_status & SIO_STATUS_DSR) == 0)
	{
		m_status |= SIO_STATUS_DSR;

		if( ( m_control & SIO_CONTROL_DSR_IENA ) != 0 )
		{
			sio_interrupt();
		}
	}
}
