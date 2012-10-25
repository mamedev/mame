/*
 * PlayStation Serial I/O emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#include "sio.h"
#include "includes/psx.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine& machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine.describe_context(), buf );
	}
}

const device_type PSX_SIO = &device_creator<psxsio_device>;

psxsio_device::psxsio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PSX_SIO, "PSX SIO", tag, owner, clock),
	m_irq0_handler(*this),
	m_irq1_handler(*this)
{
	int n;

	for( n = 0; n < 2; n++ )
	{
		port[ n ].fn_handler = NULL;
	}
}

void psxsio_device::device_reset()
{
}

void psxsio_device::device_post_load()
{
	int n;

	for( n = 0; n < 2; n++ )
	{
		sio_timer_adjust( n );
	}
}

void psxsio_device::device_start()
{
	int n;

	m_irq0_handler.resolve_safe();
	m_irq1_handler.resolve_safe();

	for( n = 0; n < 2; n++ )
	{
		port[ n ].timer = machine().scheduler().timer_alloc( timer_expired_delegate( FUNC( psxsio_device::sio_clock ), this ) );
		port[ n ].n_status = SIO_STATUS_TX_EMPTY | SIO_STATUS_TX_RDY;
		port[ n ].n_mode = 0;
		port[ n ].n_control = 0;
		port[ n ].n_baud = 0;
		port[ n ].n_tx = 0;
		port[ n ].n_rx = 0;
		port[ n ].n_tx_prev = 0;
		port[ n ].n_rx_prev = 0;
		port[ n ].n_rx_data = 0;
		port[ n ].n_tx_data = 0;
		port[ n ].n_rx_shift = 0;
		port[ n ].n_tx_shift = 0;
		port[ n ].n_rx_bits = 0;
		port[ n ].n_tx_bits = 0;
	}

	for( n = 0; n < 2; n++ )
	{
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_status );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_mode );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_control );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_baud );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_tx );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_rx );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_tx_prev );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_rx_prev );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_rx_data );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_tx_data );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_rx_shift );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_tx_shift );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_rx_bits );
		state_save_register_item( machine(), "psxsio", NULL, n, port[n].n_tx_bits );
	}
}

void psxsio_device::install_handler( int n_port, psx_sio_handler p_f_sio_handler )
{
	port[ n_port ].fn_handler = p_f_sio_handler;
}

void psxsio_device::sio_interrupt( int n_port )
{
	psx_sio *sio = &port[ n_port ];

	verboselog( machine(), 1, "sio_interrupt( %d )\n", n_port );
	sio->n_status |= SIO_STATUS_IRQ;
	if( n_port == 0 )
	{
		m_irq0_handler(1);
	}
	else
	{
		m_irq1_handler(1);
	}
}

void psxsio_device::sio_timer_adjust( int n_port )
{
	psx_sio *sio = &port[ n_port ];
	attotime n_time;

	if( ( sio->n_status & SIO_STATUS_TX_EMPTY ) == 0 || sio->n_tx_bits != 0 )
	{
		int n_prescaler;

		switch( sio->n_mode & 3 )
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

		if( sio->n_baud != 0 && n_prescaler != 0 )
		{
			n_time = attotime::from_hz(33868800) * (n_prescaler * sio->n_baud);
			verboselog( machine(), 2, "sio_timer_adjust( %d ) = %s ( %d x %d )\n", n_port, n_time.as_string(), n_prescaler, sio->n_baud );
		}
		else
		{
			n_time = attotime::never;
			verboselog( machine(), 0, "sio_timer_adjust( %d ) invalid baud rate ( %d x %d )\n", n_port, n_prescaler, sio->n_baud );
		}
	}
	else
	{
		n_time = attotime::never;
		verboselog( machine(), 2, "sio_timer_adjust( %d ) finished\n", n_port );
	}
	sio->timer->adjust( n_time, n_port);
}

TIMER_CALLBACK_MEMBER(psxsio_device::sio_clock)
{
	int n_port = param;
	psx_sio *sio = &port[ n_port ];
	verboselog( machine(), 2, "sio tick\n" );

	if( sio->n_tx_bits == 0 &&
		( sio->n_control & SIO_CONTROL_TX_ENA ) != 0 &&
		( sio->n_status & SIO_STATUS_TX_EMPTY ) == 0 )
	{
		sio->n_tx_bits = 8;
		sio->n_tx_shift = sio->n_tx_data;
		if( n_port == 0 )
		{
			sio->n_rx_bits = 8;
			sio->n_rx_shift = 0;
		}
		sio->n_status |= SIO_STATUS_TX_EMPTY;
		sio->n_status |= SIO_STATUS_TX_RDY;
	}

	if( sio->n_tx_bits != 0 )
	{
		sio->n_tx = ( sio->n_tx & ~PSX_SIO_OUT_DATA ) | ( ( sio->n_tx_shift & 1 ) * PSX_SIO_OUT_DATA );
		sio->n_tx_shift >>= 1;
		sio->n_tx_bits--;

		if( sio->fn_handler != NULL )
		{
			if( n_port == 0 )
			{
				sio->n_tx &= ~PSX_SIO_OUT_CLOCK;
				(*sio->fn_handler)( machine(), sio->n_tx );
				sio->n_tx |= PSX_SIO_OUT_CLOCK;
			}
			(*sio->fn_handler)( machine(), sio->n_tx );
		}

		if( sio->n_tx_bits == 0 &&
			( sio->n_control & SIO_CONTROL_TX_IENA ) != 0 )
		{
			sio_interrupt( n_port );
		}
	}

	if( sio->n_rx_bits != 0 )
	{
		sio->n_rx_shift = ( sio->n_rx_shift >> 1 ) | ( ( ( sio->n_rx & PSX_SIO_IN_DATA ) / PSX_SIO_IN_DATA ) << 7 );
		sio->n_rx_bits--;

		if( sio->n_rx_bits == 0 )
		{
			if( ( sio->n_status & SIO_STATUS_RX_RDY ) != 0 )
			{
				sio->n_status |= SIO_STATUS_OVERRUN;
			}
			else
			{
				sio->n_rx_data = sio->n_rx_shift;
				sio->n_status |= SIO_STATUS_RX_RDY;
			}
			if( ( sio->n_control & SIO_CONTROL_RX_IENA ) != 0 )
			{
				sio_interrupt( n_port );
			}
		}
	}

	sio_timer_adjust( n_port );
}

WRITE32_MEMBER( psxsio_device::write )
{
	int n_port = offset / 4;
	psx_sio *sio = &port[ n_port ];

	switch( offset % 4 )
	{
	case 0:
		verboselog( machine(), 1, "psx_sio_w %d data %02x (%08x)\n", n_port, data, mem_mask );
		sio->n_tx_data = data;
		sio->n_status &= ~( SIO_STATUS_TX_RDY );
		sio->n_status &= ~( SIO_STATUS_TX_EMPTY );
		sio_timer_adjust( n_port );
		break;
	case 1:
		verboselog( machine(), 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		break;
	case 2:
		if( ACCESSING_BITS_0_15 )
		{
			sio->n_mode = data & 0xffff;
			verboselog( machine(), 1, "psx_sio_w %d mode %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( machine(), 1, "psx_sio_w %d control %04x\n", n_port, data >> 16 );
			sio->n_control = data >> 16;

			if( ( sio->n_control & SIO_CONTROL_RESET ) != 0 )
			{
				verboselog( machine(), 1, "psx_sio_w reset\n" );
				sio->n_status |= SIO_STATUS_TX_EMPTY | SIO_STATUS_TX_RDY;
				sio->n_status &= ~( SIO_STATUS_RX_RDY | SIO_STATUS_OVERRUN | SIO_STATUS_IRQ );
			}
			if( ( sio->n_control & SIO_CONTROL_IACK ) != 0 )
			{
				verboselog( machine(), 1, "psx_sio_w iack\n" );
				sio->n_status &= ~( SIO_STATUS_IRQ );
				sio->n_control &= ~( SIO_CONTROL_IACK );
			}
			if( ( sio->n_control & SIO_CONTROL_DTR ) != 0 )
			{
				sio->n_tx |= PSX_SIO_OUT_DTR;
			}
			else
			{
				sio->n_tx &= ~PSX_SIO_OUT_DTR;
			}

			if( ( ( sio->n_tx ^ sio->n_tx_prev ) & PSX_SIO_OUT_DTR ) != 0 )
			{
				if( sio->fn_handler != NULL )
				{
					(*sio->fn_handler)( machine(), sio->n_tx );
				}
			}
			sio->n_tx_prev = sio->n_tx;

		}
		break;
	case 3:
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( machine(), 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		}
		if( ACCESSING_BITS_16_31 )
		{
			sio->n_baud = data >> 16;
			verboselog( machine(), 1, "psx_sio_w %d baud %04x\n", n_port, data >> 16 );
		}
		break;
	default:
		verboselog( machine(), 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		break;
	}
}

READ32_MEMBER( psxsio_device::read )
{
	int n_port = offset / 4;
	psx_sio *sio = &port[ n_port ];
	UINT32 data;

	switch( offset % 4 )
	{
	case 0:
		data = sio->n_rx_data;
		sio->n_status &= ~( SIO_STATUS_RX_RDY );
		sio->n_rx_data = 0xff;
		verboselog( machine(), 1, "psx_sio_r %d data %02x (%08x)\n", n_port, data, mem_mask );
		break;
	case 1:
		data = sio->n_status;
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( machine(), 1, "psx_sio_r %d status %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( machine(), 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		}
		break;
	case 2:
		data = ( sio->n_control << 16 ) | sio->n_mode;
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( machine(), 1, "psx_sio_r %d mode %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( machine(), 1, "psx_sio_r %d control %04x\n", n_port, data >> 16 );
		}
		break;
	case 3:
		data = sio->n_baud << 16;
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( machine(), 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( machine(), 1, "psx_sio_r %d baud %04x\n", n_port, data >> 16 );
		}
		break;
	default:
		data = 0;
		verboselog( machine(), 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		break;
	}
	return data;
}

void psxsio_device::input( int n_port, int n_mask, int n_data )
{
	psx_sio *sio = &port[ n_port ];
	verboselog( machine(), 1, "psx_sio_input( %d, %02x, %02x )\n", n_port, n_mask, n_data );
	sio->n_rx = ( sio->n_rx & ~n_mask ) | ( n_data & n_mask );

	if( ( sio->n_rx & PSX_SIO_IN_DSR ) != 0 )
	{
		sio->n_status |= SIO_STATUS_DSR;
		if( ( sio->n_rx_prev & PSX_SIO_IN_DSR ) == 0 &&
			( sio->n_control & SIO_CONTROL_DSR_IENA ) != 0 )
		{
			sio_interrupt( n_port );
		}
	}
	else
	{
		sio->n_status &= ~SIO_STATUS_DSR;
	}
	sio->n_rx_prev = sio->n_rx;
}
