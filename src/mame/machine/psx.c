/***************************************************************************

  machine/psx.c

  Thanks to Olivier Galibert for IDCT information

***************************************************************************/

#include "emu.h"
#include "cpu/mips/psx.h"
#include "includes/psx.h"

#define VERBOSE_LEVEL ( 0 )

#define	DCTSIZE ( 8 )
#define	DCTSIZE2 ( DCTSIZE * DCTSIZE )

#define SIO_BUF_SIZE ( 8 )

#define SIO_STATUS_TX_RDY ( 1 << 0 )
#define SIO_STATUS_RX_RDY ( 1 << 1 )
#define SIO_STATUS_TX_EMPTY ( 1 << 2 )
#define SIO_STATUS_OVERRUN ( 1 << 4 )
#define SIO_STATUS_DSR ( 1 << 7 )
#define SIO_STATUS_IRQ ( 1 << 9 )

#define SIO_CONTROL_TX_ENA ( 1 << 0 )
#define SIO_CONTROL_IACK ( 1 << 4 )
#define SIO_CONTROL_RESET ( 1 << 6 )
#define SIO_CONTROL_TX_IENA ( 1 << 10 )
#define SIO_CONTROL_RX_IENA ( 1 << 11 )
#define SIO_CONTROL_DSR_IENA ( 1 << 12 )
#define SIO_CONTROL_DTR ( 1 << 13 )

#define MDEC_COS_PRECALC_BITS ( 21 )


typedef struct _psx_dma_channel psx_dma_channel;
struct _psx_dma_channel
{
	UINT32 n_base;
	UINT32 n_blockcontrol;
	UINT32 n_channelcontrol;
	emu_timer *timer;
	psx_dma_read_handler fn_read;
	psx_dma_write_handler fn_write;
	UINT32 n_ticks;
	UINT32 b_running;
};

typedef struct _psx_root psx_root;
struct _psx_root
{
	emu_timer *timer;
	UINT16 n_count;
	UINT16 n_mode;
	UINT16 n_target;
	UINT64 n_start;
};

typedef struct _psx_sio psx_sio;
struct _psx_sio
{
	UINT32 n_status;
	UINT32 n_mode;
	UINT32 n_control;
	UINT32 n_baud;
	UINT32 n_tx;
	UINT32 n_rx;
	UINT32 n_tx_prev;
	UINT32 n_rx_prev;
	UINT32 n_tx_data;
	UINT32 n_rx_data;
	UINT32 n_tx_shift;
	UINT32 n_rx_shift;
	UINT32 n_tx_bits;
	UINT32 n_rx_bits;

	emu_timer *timer;
	psx_sio_handler fn_handler;
};

typedef struct _psx_mdec psx_mdec;
struct _psx_mdec
{
	UINT32 n_decoded;
	UINT32 n_offset;
	UINT16 p_n_output[ 24 * 16 ];

	INT32 p_n_quantize_y[ DCTSIZE2 ];
	INT32 p_n_quantize_uv[ DCTSIZE2 ];
	INT32 p_n_cos[ DCTSIZE2 ];
	INT32 p_n_cos_precalc[ DCTSIZE2 * DCTSIZE2 ];

	UINT32 n_0_command;
	UINT32 n_0_address;
	UINT32 n_0_size;
	UINT32 n_1_command;
	UINT32 n_1_status;

	UINT16 p_n_clamp8[ 256 * 3 ];
	UINT16 p_n_r5[ 256 * 3 ];
	UINT16 p_n_g5[ 256 * 3 ];
	UINT16 p_n_b5[ 256 * 3 ];

	INT32 p_n_unpacked[ DCTSIZE2 * 6 * 2 ];
};

struct _psx_machine
{
	running_machine *machine;
	UINT32 *p_n_psxram;
	size_t n_psxramsize;

	UINT32 n_com_delay;
	UINT32 n_irqdata;
	UINT32 n_irqmask;

	UINT32 n_dpcp;
	UINT32 n_dicr;
	psx_dma_channel channel[7];

	psx_root root[3];

	psx_sio sio[2];

	psx_mdec mdec;
};


INLINE void ATTR_PRINTF(3,4) verboselog( psx_machine *p_psx, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", cpuexec_describe_context(p_psx->machine), buf );
	}
}

#ifdef UNUSED_FUNCTION
INLINE void psxwriteword( UINT32 *p_n_psxram, UINT32 n_address, UINT16 n_data )
{
	*( (UINT16 *)( (UINT8 *)p_n_psxram + WORD_XOR_LE( n_address ) ) ) = n_data;
}
#endif

INLINE UINT16 psxreadword( UINT32 *p_n_psxram, UINT32 n_address )
{
	return *( (UINT16 *)( (UINT8 *)p_n_psxram + WORD_XOR_LE( n_address ) ) );
}

WRITE32_HANDLER( psx_com_delay_w )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;

	COMBINE_DATA( &p_psx->n_com_delay );
	verboselog( p_psx, 1, "psx_com_delay_w( %08x %08x )\n", data, mem_mask );
}

READ32_HANDLER( psx_com_delay_r )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;

	verboselog( p_psx, 1, "psx_com_delay_r( %08x )\n", mem_mask );
	return p_psx->n_com_delay;
}

/* IRQ */

static void psx_irq_update( psx_machine *p_psx )
{
	if( ( p_psx->n_irqdata & p_psx->n_irqmask ) != 0 )
	{
		verboselog( p_psx, 2, "psx irq assert\n" );
		cputag_set_input_line( p_psx->machine, "maincpu", PSXCPU_IRQ0, ASSERT_LINE );
	}
	else
	{
		verboselog( p_psx, 2, "psx irq clear\n" );
		cputag_set_input_line( p_psx->machine, "maincpu", PSXCPU_IRQ0, CLEAR_LINE );
	}
}

WRITE32_HANDLER( psx_irq_w )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;

	switch( offset )
	{
	case 0x00:
		verboselog( p_psx, 2, "psx irq data ( %08x, %08x ) %08x -> %08x\n", data, mem_mask, p_psx->n_irqdata, ( p_psx->n_irqdata & ~mem_mask ) | ( p_psx->n_irqdata & p_psx->n_irqmask & data ) );
		p_psx->n_irqdata = ( p_psx->n_irqdata & ~mem_mask ) | ( p_psx->n_irqdata & p_psx->n_irqmask & data );
		psx_irq_update(p_psx);
		break;
	case 0x01:
		verboselog( p_psx, 2, "psx irq mask ( %08x, %08x ) %08x -> %08x\n", data, mem_mask, p_psx->n_irqmask, ( p_psx->n_irqmask & ~mem_mask ) | data );
		p_psx->n_irqmask = ( p_psx->n_irqmask & ~mem_mask ) | data;
		if( ( p_psx->n_irqmask &~ PSX_IRQ_MASK ) != 0 )
		{
			verboselog( p_psx, 0, "psx_irq_w( %08x, %08x, %08x ) unknown irq\n", offset, data, mem_mask );
		}
		psx_irq_update(p_psx);
		break;
	default:
		verboselog( p_psx, 0, "psx_irq_w( %08x, %08x, %08x ) unknown register\n", offset, data, mem_mask );
		break;
	}
}

READ32_HANDLER( psx_irq_r )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;

	switch( offset )
	{
	case 0x00:
		verboselog( p_psx, 1, "psx_irq_r irq data %08x\n", p_psx->n_irqdata );
		return p_psx->n_irqdata;
	case 0x01:
		verboselog( p_psx, 1, "psx_irq_r irq mask %08x\n", p_psx->n_irqmask );
		return p_psx->n_irqmask;
	default:
		verboselog( p_psx, 0, "psx_irq_r unknown register %d\n", offset );
		break;
	}
	return 0;
}

void psx_irq_set( running_machine *machine, UINT32 data )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;

	verboselog( p_psx, 2, "psx_irq_set %08x\n", data );
	p_psx->n_irqdata |= data;
	psx_irq_update(p_psx);
}

/* DMA */

static void dma_start_timer( psx_machine *p_psx, int n_channel, UINT32 n_ticks )
{
	psx_dma_channel *dma = &p_psx->channel[ n_channel ];

	timer_adjust_oneshot( dma->timer, attotime_mul(ATTOTIME_IN_HZ(33868800), n_ticks), n_channel);
	dma->n_ticks = n_ticks;
	dma->b_running = 1;
}

static void dma_stop_timer( psx_machine *p_psx, int n_channel )
{
	psx_dma_channel *dma = &p_psx->channel[ n_channel ];

	timer_adjust_oneshot( dma->timer, attotime_never, 0);
	dma->b_running = 0;
}

static void dma_timer_adjust( psx_machine *p_psx, int n_channel )
{
	psx_dma_channel *dma = &p_psx->channel[ n_channel ];

	if( dma->b_running )
	{
		dma_start_timer( p_psx, n_channel, dma->n_ticks );
	}
	else
	{
		dma_stop_timer( p_psx, n_channel );
	}
}

static void dma_interrupt_update( psx_machine *p_psx )
{
	int n_int;
	int n_mask;

	n_int = ( p_psx->n_dicr >> 24 ) & 0x7f;
	n_mask = ( p_psx->n_dicr >> 16 ) & 0xff;

	if( ( n_mask & 0x80 ) != 0 && ( n_int & n_mask ) != 0 )
	{
		verboselog( p_psx, 2, "dma_interrupt_update( %02x, %02x ) interrupt triggered\n", n_int, n_mask );
		p_psx->n_dicr |= 0x80000000;
		psx_irq_set( p_psx->machine, PSX_IRQ_DMA );
	}
	else if( ( p_psx->n_dicr & 0x80000000 ) != 0 )
	{
		verboselog( p_psx, 2, "dma_interrupt_update( %02x, %02x ) interrupt cleared\n", n_int, n_mask );
		p_psx->n_dicr &= ~0x80000000;
	}
	else if( n_int != 0 )
	{
		verboselog( p_psx, 2, "dma_interrupt_update( %02x, %02x ) interrupt not enabled\n", n_int, n_mask );
	}

	p_psx->n_dicr &= 0x00ffffff | ( p_psx->n_dicr << 8 );
}

static void dma_finished(psx_machine *p_psx, int n_channel)
{
	UINT32 *p_n_psxram = p_psx->p_n_psxram;
	psx_dma_channel *dma = &p_psx->channel[ n_channel ];

	if( dma->n_channelcontrol == 0x01000401 && n_channel == 2 )
	{
		UINT32 n_size;
		UINT32 n_total;
		UINT32 n_address = ( dma->n_base & 0xffffff );
		UINT32 n_adrmask = p_psx->n_psxramsize - 1;
		UINT32 n_nextaddress;

		if( n_address != 0xffffff )
		{
			n_total = 0;
			for( ;; )
			{
				if( n_address == 0xffffff )
				{
					dma->n_base = n_address;
					dma_start_timer( p_psx, n_channel, 19000 );
					return;
				}
				if( n_total > 65535 )
				{
					dma->n_base = n_address;
					//FIXME:
					// 16000 below is based on try and error.
					// Mametesters.org: sfex20103red
					//dma_start_timer( p_psx, n_channel, 16 );
					dma_start_timer( p_psx, n_channel, 16000 );
					return;
				}
				n_address &= n_adrmask;
				n_nextaddress = p_n_psxram[ n_address / 4 ];
				n_size = n_nextaddress >> 24;
				(*dma->fn_write)( p_psx->machine, n_address + 4, n_size );
				//FIXME:
				// The following conditions will cause an endless loop.
				// If stopping the transfer is correct I cannot judge
				// The patch is meant as a hint for somebody who knows
				// the hardware.
				// Mametesters.org: psyforce0105u5red, raystorm0111u1red
				if ((n_nextaddress & 0xffffff) != 0xffffff)
					if (n_address == p_n_psxram[ (n_nextaddress & 0xffffff) / 4])
						break;
				if (n_address == (n_nextaddress & 0xffffff) )
					break;
				n_address = ( n_nextaddress & 0xffffff );

				n_total += ( n_size + 1 );
			}
		}
	}

	dma->n_channelcontrol &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );

	p_psx->n_dicr |= 1 << ( 24 + n_channel );
	dma_interrupt_update(p_psx);
	dma_stop_timer( p_psx, n_channel );
}

static TIMER_CALLBACK( dma_finished_callback )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;

	dma_finished(p_psx, param);
}

void psx_dma_install_read_handler( running_machine *machine, int n_channel, psx_dma_read_handler p_fn_dma_read )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;

	p_psx->channel[ n_channel ].fn_read = p_fn_dma_read;
}

void psx_dma_install_write_handler( running_machine *machine, int n_channel, psx_dma_read_handler p_fn_dma_write )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;

	p_psx->channel[ n_channel ].fn_write = p_fn_dma_write;
}

WRITE32_HANDLER( psx_dma_w )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;
	UINT32 *p_n_psxram = p_psx->p_n_psxram;
	int n_channel = offset / 4;
	psx_dma_channel *dma = &p_psx->channel[ n_channel ];

	if( n_channel < 7 )
	{
		switch( offset % 4 )
		{
		case 0:
			verboselog( p_psx, 2, "dmabase( %d ) = %08x\n", n_channel, data );
			dma->n_base = data;
			break;
		case 1:
			verboselog( p_psx, 2, "dmablockcontrol( %d ) = %08x\n", n_channel, data );
			dma->n_blockcontrol = data;
			break;
		case 2:
			verboselog( p_psx, 2, "dmachannelcontrol( %d ) = %08x\n", n_channel, data );
			dma->n_channelcontrol = data;
			if( ( dma->n_channelcontrol & ( 1L << 0x18 ) ) != 0 && ( p_psx->n_dpcp & ( 1 << ( 3 + ( n_channel * 4 ) ) ) ) != 0 )
			{
				INT32 n_size;
				UINT32 n_address;
				UINT32 n_nextaddress;
				UINT32 n_adrmask;

				n_adrmask = p_psx->n_psxramsize - 1;

				n_address = ( dma->n_base & n_adrmask );
				n_size = dma->n_blockcontrol;
				if( ( dma->n_channelcontrol & 0x200 ) != 0 )
				{
					UINT32 n_ba;
					n_ba = dma->n_blockcontrol >> 16;
					if( n_ba == 0 )
					{
						n_ba = 0x10000;
					}
					n_size = ( n_size & 0xffff ) * n_ba;
				}

				if( dma->n_channelcontrol == 0x01000000 &&
					dma->fn_read != NULL )
				{
					verboselog( p_psx, 1, "dma %d read block %08x %08x\n", n_channel, n_address, n_size );
					(*dma->fn_read)( space->machine, n_address, n_size );
					dma_finished( p_psx, n_channel );
				}
				else if( dma->n_channelcontrol == 0x01000200 &&
					dma->fn_read != NULL )
				{
					verboselog( p_psx, 1, "dma %d read block %08x %08x\n", n_channel, n_address, n_size );
					(*dma->fn_read)( space->machine, n_address, n_size );
					if( n_channel == 1 )
					{
						dma_start_timer( p_psx, n_channel, 26000 );
					}
					else
					{
						dma_finished( p_psx, n_channel );
					}
				}
				else if( dma->n_channelcontrol == 0x01000201 &&
					dma->fn_write != NULL )
				{
					verboselog( p_psx, 1, "dma %d write block %08x %08x\n", n_channel, n_address, n_size );
					(*dma->fn_write)( space->machine, n_address, n_size );
					dma_finished( p_psx, n_channel );
				}
				else if( dma->n_channelcontrol == 0x11050100 &&
					dma->fn_write != NULL )
				{
					/* todo: check this is a write not a read... */
					verboselog( p_psx, 1, "dma %d write block %08x %08x\n", n_channel, n_address, n_size );
					(*dma->fn_write)( space->machine, n_address, n_size );
					dma_finished( p_psx, n_channel );
				}
				else if( dma->n_channelcontrol == 0x11150100 &&
					dma->fn_write != NULL )
				{
					/* todo: check this is a write not a read... */
					verboselog( p_psx, 1, "dma %d write block %08x %08x\n", n_channel, n_address, n_size );
					(*dma->fn_write)( space->machine, n_address, n_size );
					dma_finished( p_psx, n_channel );
				}
				else if( dma->n_channelcontrol == 0x01000401 &&
					n_channel == 2 &&
					dma->fn_write != NULL )
				{
					verboselog( p_psx, 1, "dma %d write linked list %08x\n",
						n_channel, dma->n_base );

					dma_finished( p_psx, n_channel );
				}
				else if( dma->n_channelcontrol == 0x11000002 &&
					n_channel == 6 )
				{
					verboselog( p_psx, 1, "dma 6 reverse clear %08x %08x\n",
						dma->n_base, dma->n_blockcontrol );
					if( n_size > 0 )
					{
						n_size--;
						while( n_size > 0 )
						{
							n_nextaddress = ( n_address - 4 ) & 0xffffff;
							p_n_psxram[ n_address / 4 ] = n_nextaddress;
							n_address = n_nextaddress;
							n_size--;
						}
						p_n_psxram[ n_address / 4 ] = 0xffffff;
					}
					dma_start_timer( p_psx, n_channel, 2150 );
				}
				else
				{
					verboselog( p_psx, 0, "dma %d unknown mode %08x\n", n_channel, dma->n_channelcontrol );
				}
			}
			else if( dma->n_channelcontrol != 0 )
			{
				verboselog( p_psx, 1, "psx_dma_w( %04x, %08x, %08x ) channel not enabled\n", offset, dma->n_channelcontrol, mem_mask );
			}
			break;
		default:
			verboselog( p_psx, 1, "psx_dma_w( %04x, %08x, %08x ) Unknown dma channel register\n", offset, data, mem_mask );
			break;
		}
	}
	else
	{
		switch( offset % 4 )
		{
		case 0x0:
			verboselog( p_psx, 1, "psx_dma_w( %04x, %08x, %08x ) dpcp\n", offset, data, mem_mask );
			p_psx->n_dpcp = ( p_psx->n_dpcp & ~mem_mask ) | data;
			break;
		case 0x1:
			p_psx->n_dicr = ( p_psx->n_dicr & ~mem_mask ) |
				( mem_mask & 0x80000000 & p_psx->n_dicr ) |
				( ~data & mem_mask & 0x7f000000 & p_psx->n_dicr ) |
				( data & mem_mask & 0x00ffffff );
#if 0
			/* todo: find out whether to do this instead of dma_interrupt_update() */
			if( ( p_psx->n_dicr & 0x7f000000 ) != 0 )
			{
				p_psx->n_dicr &= ~0x80000000;
			}
#endif
			verboselog( p_psx, 1, "psx_dma_w( %04x, %08x, %08x ) dicr -> %08x\n", offset, data, mem_mask, p_psx->n_dicr );
			dma_interrupt_update(p_psx);
			break;
		default:
			verboselog( p_psx, 0, "psx_dma_w( %04x, %08x, %08x ) Unknown dma control register\n", offset, data, mem_mask );
			break;
		}
	}
}

READ32_HANDLER( psx_dma_r )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;
	int n_channel = offset / 4;
	psx_dma_channel *dma = &p_psx->channel[ n_channel ];

	if( n_channel < 7 )
	{
		switch( offset % 4 )
		{
		case 0:
			verboselog( p_psx, 1, "psx_dma_r dmabase[ %d ] ( %08x )\n", n_channel, dma->n_base );
			return dma->n_base;
		case 1:
			verboselog( p_psx, 1, "psx_dma_r dmablockcontrol[ %d ] ( %08x )\n", n_channel, dma->n_blockcontrol );
			return dma->n_blockcontrol;
		case 2:
			verboselog( p_psx, 1, "psx_dma_r dmachannelcontrol[ %d ] ( %08x )\n", n_channel, dma->n_channelcontrol );
			return dma->n_channelcontrol;
		default:
			verboselog( p_psx, 0, "psx_dma_r( %08x, %08x ) Unknown dma channel register\n", offset, mem_mask );
			break;
		}
	}
	else
	{
		switch( offset % 4 )
		{
		case 0x0:
			verboselog( p_psx, 1, "psx_dma_r dpcp ( %08x )\n", p_psx->n_dpcp );
			return p_psx->n_dpcp;
		case 0x1:
			verboselog( p_psx, 1, "psx_dma_r dicr ( %08x )\n", p_psx->n_dicr );
			return p_psx->n_dicr;
		default:
			verboselog( p_psx, 0, "psx_dma_r( %08x, %08x ) Unknown dma control register\n", offset, mem_mask );
			break;
		}
	}
	return 0;
}

/* Root Counters */

static UINT64 psxcpu_gettotalcycles( psx_machine *p_psx )
{
	/* TODO: should return the start of the current tick. */
	return p_psx->machine->firstcpu->total_cycles() * 2;
}

static int root_divider( psx_machine *p_psx, int n_counter )
{
	psx_root *root = &p_psx->root[ n_counter ];

	if( n_counter == 0 && ( root->n_mode & PSX_RC_CLC ) != 0 )
	{
		/* TODO: pixel clock, probably based on resolution */
		return 5;
	}
	else if( n_counter == 1 && ( root->n_mode & PSX_RC_CLC ) != 0 )
	{
		return 2150;
	}
	else if( n_counter == 2 && ( root->n_mode & PSX_RC_DIV ) != 0 )
	{
		return 8;
	}
	return 1;
}

static UINT16 root_current( psx_machine *p_psx, int n_counter )
{
	psx_root *root = &p_psx->root[ n_counter ];

	if( ( root->n_mode & PSX_RC_STOP ) != 0 )
	{
		return root->n_count;
	}
	else
	{
		UINT64 n_current;
		n_current = psxcpu_gettotalcycles(p_psx) - root->n_start;
		n_current /= root_divider( p_psx, n_counter );
		n_current += root->n_count;
		if( n_current > 0xffff )
		{
			/* TODO: use timer for wrap on 0x10000. */
			root->n_count = n_current;
			root->n_start = psxcpu_gettotalcycles(p_psx);
		}
		return n_current;
	}
}

static int root_target( psx_machine *p_psx, int n_counter )
{
	psx_root *root = &p_psx->root[ n_counter ];

	if( ( root->n_mode & PSX_RC_COUNTTARGET ) != 0 ||
		( root->n_mode & PSX_RC_IRQTARGET ) != 0 )
	{
		return root->n_target;
	}
	return 0x10000;
}

static void root_timer_adjust( psx_machine *p_psx, int n_counter )
{
	psx_root *root = &p_psx->root[ n_counter ];

	if( ( root->n_mode & PSX_RC_STOP ) != 0 )
	{
		timer_adjust_oneshot( root->timer, attotime_never, n_counter);
	}
	else
	{
		int n_duration;

		n_duration = root_target( p_psx, n_counter ) - root_current( p_psx, n_counter );
		if( n_duration < 1 )
		{
			n_duration += 0x10000;
		}

		n_duration *= root_divider( p_psx, n_counter );

		timer_adjust_oneshot( root->timer, attotime_mul(ATTOTIME_IN_HZ(33868800), n_duration), n_counter);
	}
}

static TIMER_CALLBACK( root_finished )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;
	int n_counter = param;
	psx_root *root = &p_psx->root[ n_counter ];

	verboselog( p_psx, 2, "root_finished( %d ) %04x\n", n_counter, root_current( p_psx, n_counter ) );
	//if( ( root->n_mode & PSX_RC_COUNTTARGET ) != 0 )
	{
		/* TODO: wrap should be handled differently as PSX_RC_COUNTTARGET & PSX_RC_IRQTARGET don't have to be the same. */
		root->n_count = 0;
		root->n_start = psxcpu_gettotalcycles(p_psx);
	}
	if( ( root->n_mode & PSX_RC_REPEAT ) != 0 )
	{
		root_timer_adjust( p_psx, n_counter );
	}
	if( ( root->n_mode & PSX_RC_IRQOVERFLOW ) != 0 ||
		( root->n_mode & PSX_RC_IRQTARGET ) != 0 )
	{
		psx_irq_set( machine, (n_counter == 3) ? PSX_IRQ_ROOTCOUNTER3 : (PSX_IRQ_ROOTCOUNTER0 << n_counter) );
	}
}

WRITE32_HANDLER( psx_counter_w )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;
	int n_counter = offset / 4;
	psx_root *root = &p_psx->root[ n_counter ];

	verboselog( p_psx, 1, "psx_counter_w ( %08x, %08x, %08x )\n", offset, data, mem_mask );

	switch( offset % 4 )
	{
	case 0:
		root->n_count = data;
		root->n_start = psxcpu_gettotalcycles(p_psx);
		break;
	case 1:
		root->n_count = root_current( p_psx, n_counter );
		root->n_start = psxcpu_gettotalcycles(p_psx);
		root->n_mode = data;

		if( ( root->n_mode & PSX_RC_RESET ) != 0 )
		{
			root->n_count = 0;
		}

#if 0
		if( ( data & 0xfca6 ) != 0 ||
			( ( data & 0x0100 ) != 0 && n_counter != 0 && n_counter != 1 ) ||
			( ( data & 0x0200 ) != 0 && n_counter != 2 ) )
		{
			mame_printf_debug( "mode %d 0x%04x\n", n_counter, data & 0xfca6 );
		}
#endif
		break;
	case 2:
		root->n_target = data;
		break;
	default:
		verboselog( p_psx, 0, "psx_counter_w( %08x, %08x, %08x ) unknown register\n", offset, mem_mask, data );
		return;
	}

	root_timer_adjust( p_psx, n_counter );
}

READ32_HANDLER( psx_counter_r )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;
	int n_counter = offset / 4;
	psx_root *root = &p_psx->root[ n_counter ];
	UINT32 data;

	switch( offset % 4 )
	{
	case 0:
		data = root_current( p_psx, n_counter );
		break;
	case 1:
		data = root->n_mode;
		break;
	case 2:
		data = root->n_target;
		break;
	default:
		verboselog( p_psx, 0, "psx_counter_r( %08x, %08x ) unknown register\n", offset, mem_mask );
		return 0;
	}
	verboselog( p_psx, 1, "psx_counter_r ( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

/* SIO */

static void sio_interrupt( psx_machine *p_psx, int n_port )
{
	psx_sio *sio = &p_psx->sio[ n_port ];

	verboselog( p_psx, 1, "sio_interrupt( %d )\n", n_port );
	sio->n_status |= SIO_STATUS_IRQ;
	if( n_port == 0 )
	{
		psx_irq_set( p_psx->machine, PSX_IRQ_SIO0 );
	}
	else
	{
		psx_irq_set( p_psx->machine, PSX_IRQ_SIO1 );
	}
}

static void sio_timer_adjust( psx_machine *p_psx, int n_port )
{
	psx_sio *sio = &p_psx->sio[ n_port ];
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
			n_time = attotime_mul(ATTOTIME_IN_HZ(33868800), n_prescaler * sio->n_baud);
			verboselog( p_psx, 2, "sio_timer_adjust( %d ) = %s ( %d x %d )\n", n_port, attotime_string(n_time, 9), n_prescaler, sio->n_baud );
		}
		else
		{
			n_time = attotime_never;
			verboselog( p_psx, 0, "sio_timer_adjust( %d ) invalid baud rate ( %d x %d )\n", n_port, n_prescaler, sio->n_baud );
		}
	}
	else
	{
		n_time = attotime_never;
		verboselog( p_psx, 2, "sio_timer_adjust( %d ) finished\n", n_port );
	}
	timer_adjust_oneshot( sio->timer, n_time, n_port);
}

static TIMER_CALLBACK( sio_clock )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;
	int n_port = param;
	psx_sio *sio = &p_psx->sio[ n_port ];
	verboselog( p_psx, 2, "sio tick\n" );

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
				(*sio->fn_handler)( machine, sio->n_tx );
				sio->n_tx |= PSX_SIO_OUT_CLOCK;
			}
			(*sio->fn_handler)( machine, sio->n_tx );
		}

		if( sio->n_tx_bits == 0 &&
			( sio->n_control & SIO_CONTROL_TX_IENA ) != 0 )
		{
			sio_interrupt( p_psx, n_port );
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
				sio_interrupt( p_psx, n_port );
			}
		}
	}

	sio_timer_adjust( p_psx, n_port );
}

void psx_sio_input( running_machine *machine, int n_port, int n_mask, int n_data )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;
	psx_sio *sio = &p_psx->sio[ n_port ];
	verboselog( p_psx, 1, "psx_sio_input( %d, %02x, %02x )\n", n_port, n_mask, n_data );
	sio->n_rx = ( sio->n_rx & ~n_mask ) | ( n_data & n_mask );

	if( ( sio->n_rx & PSX_SIO_IN_DSR ) != 0 )
	{
		sio->n_status |= SIO_STATUS_DSR;
		if( ( sio->n_rx_prev & PSX_SIO_IN_DSR ) == 0 &&
			( sio->n_control & SIO_CONTROL_DSR_IENA ) != 0 )
		{
			sio_interrupt( p_psx, n_port );
		}
	}
	else
	{
		sio->n_status &= ~SIO_STATUS_DSR;
	}
	sio->n_rx_prev = sio->n_rx;
}

WRITE32_HANDLER( psx_sio_w )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;
	int n_port = offset / 4;
	psx_sio *sio = &p_psx->sio[ n_port ];

	switch( offset % 4 )
	{
	case 0:
		verboselog( p_psx, 1, "psx_sio_w %d data %02x (%08x)\n", n_port, data, mem_mask );
		sio->n_tx_data = data;
		sio->n_status &= ~( SIO_STATUS_TX_RDY );
		sio->n_status &= ~( SIO_STATUS_TX_EMPTY );
		sio_timer_adjust( p_psx, n_port );
		break;
	case 1:
		verboselog( p_psx, 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		break;
	case 2:
		if( ACCESSING_BITS_0_15 )
		{
			sio->n_mode = data & 0xffff;
			verboselog( p_psx, 1, "psx_sio_w %d mode %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( p_psx, 1, "psx_sio_w %d control %04x\n", n_port, data >> 16 );
			sio->n_control = data >> 16;

			if( ( sio->n_control & SIO_CONTROL_RESET ) != 0 )
			{
				verboselog( p_psx, 1, "psx_sio_w reset\n" );
				sio->n_status |= SIO_STATUS_TX_EMPTY | SIO_STATUS_TX_RDY;
				sio->n_status &= ~( SIO_STATUS_RX_RDY | SIO_STATUS_OVERRUN | SIO_STATUS_IRQ );
			}
			if( ( sio->n_control & SIO_CONTROL_IACK ) != 0 )
			{
				verboselog( p_psx, 1, "psx_sio_w iack\n" );
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
					(*sio->fn_handler)( space->machine, sio->n_tx );
				}
			}
			sio->n_tx_prev = sio->n_tx;

		}
		break;
	case 3:
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( p_psx, 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		}
		if( ACCESSING_BITS_16_31 )
		{
			sio->n_baud = data >> 16;
			verboselog( p_psx, 1, "psx_sio_w %d baud %04x\n", n_port, data >> 16 );
		}
		break;
	default:
		verboselog( p_psx, 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		break;
	}
}

READ32_HANDLER( psx_sio_r )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;
	int n_port = offset / 4;
	psx_sio *sio = &p_psx->sio[ n_port ];
	UINT32 data;

	switch( offset % 4 )
	{
	case 0:
		data = sio->n_rx_data;
		sio->n_status &= ~( SIO_STATUS_RX_RDY );
		sio->n_rx_data = 0xff;
		verboselog( p_psx, 1, "psx_sio_r %d data %02x (%08x)\n", n_port, data, mem_mask );
		break;
	case 1:
		data = sio->n_status;
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( p_psx, 1, "psx_sio_r %d status %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( p_psx, 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		}
		break;
	case 2:
		data = ( sio->n_control << 16 ) | sio->n_mode;
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( p_psx, 1, "psx_sio_r %d mode %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( p_psx, 1, "psx_sio_r %d control %04x\n", n_port, data >> 16 );
		}
		break;
	case 3:
		data = sio->n_baud << 16;
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( p_psx, 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( p_psx, 1, "psx_sio_r %d baud %04x\n", n_port, data >> 16 );
		}
		break;
	default:
		data = 0;
		verboselog( p_psx, 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		break;
	}
	return data;
}

void psx_sio_install_handler( running_machine *machine, int n_port, psx_sio_handler p_f_sio_handler )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;

	p_psx->sio[ n_port ].fn_handler = p_f_sio_handler;
}

/* MDEC */

static const UINT32 m_p_n_mdec_zigzag[ DCTSIZE2 ] =
{
	 0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

static void mdec_cos_precalc( psx_machine *p_psx )
{
	psx_mdec *p_mdec = &p_psx->mdec;
	UINT32 n_x;
	UINT32 n_y;
	UINT32 n_u;
	UINT32 n_v;
	INT32 *p_n_precalc;

	p_n_precalc = p_mdec->p_n_cos_precalc;

	for( n_y = 0; n_y < 8; n_y++ )
	{
		for( n_x = 0; n_x < 8; n_x++ )
		{
			for( n_v = 0; n_v < 8; n_v++ )
			{
				for( n_u = 0; n_u < 8; n_u++ )
				{
					*( p_n_precalc++ ) =
						( ( p_mdec->p_n_cos[ ( n_u * 8 ) + n_x ] *
						p_mdec->p_n_cos[ ( n_v * 8 ) + n_y ] ) >> ( 30 - MDEC_COS_PRECALC_BITS ) );
				}
			}
		}
	}
}

static void mdec_idct( psx_machine *p_psx, INT32 *p_n_src, INT32 *p_n_dst )
{
	psx_mdec *p_mdec = &p_psx->mdec;
	UINT32 n_yx;
	UINT32 n_vu;
	INT32 p_n_z[ 8 ];
	INT32 *p_n_data;
	INT32 *p_n_precalc;

	p_n_precalc = p_mdec->p_n_cos_precalc;

	for( n_yx = 0; n_yx < DCTSIZE2; n_yx++ )
	{
		p_n_data = p_n_src;

		memset( p_n_z, 0, sizeof( p_n_z ) );

		for( n_vu = 0; n_vu < DCTSIZE2 / 8; n_vu++ )
		{
			p_n_z[ 0 ] += p_n_data[ 0 ] * p_n_precalc[ 0 ];
			p_n_z[ 1 ] += p_n_data[ 1 ] * p_n_precalc[ 1 ];
			p_n_z[ 2 ] += p_n_data[ 2 ] * p_n_precalc[ 2 ];
			p_n_z[ 3 ] += p_n_data[ 3 ] * p_n_precalc[ 3 ];
			p_n_z[ 4 ] += p_n_data[ 4 ] * p_n_precalc[ 4 ];
			p_n_z[ 5 ] += p_n_data[ 5 ] * p_n_precalc[ 5 ];
			p_n_z[ 6 ] += p_n_data[ 6 ] * p_n_precalc[ 6 ];
			p_n_z[ 7 ] += p_n_data[ 7 ] * p_n_precalc[ 7 ];
			p_n_data += 8;
			p_n_precalc += 8;
		}

		*( p_n_dst++ ) = ( p_n_z[ 0 ] + p_n_z[ 1 ] + p_n_z[ 2 ] + p_n_z[ 3 ] +
			p_n_z[ 4 ] + p_n_z[ 5 ] + p_n_z[ 6 ] + p_n_z[ 7 ] ) >> ( MDEC_COS_PRECALC_BITS + 2 );
	}
}

INLINE UINT16 mdec_unpack_run( UINT16 n_packed )
{
	return n_packed >> 10;
}

INLINE INT32 mdec_unpack_val( UINT16 n_packed )
{
	return ( ( (INT32)n_packed ) << 22 ) >> 22;
}

static UINT32 mdec_unpack( psx_machine *p_psx, UINT32 n_address )
{
	UINT32 *p_n_psxram = p_psx->p_n_psxram;
	psx_mdec *p_mdec = &p_psx->mdec;
	UINT8 n_z;
	INT32 n_qscale;
	UINT16 n_packed;
	UINT32 n_block;
	INT32 *p_n_block;
	INT32 p_n_unpacked[ 64 ];
	INT32 *p_n_q;

	p_n_q = p_mdec->p_n_quantize_uv;
	p_n_block = p_mdec->p_n_unpacked;

	for( n_block = 0; n_block < 6; n_block++ )
	{
		memset( p_n_unpacked, 0, sizeof( p_n_unpacked ) );

		if( n_block == 2 )
		{
			p_n_q = p_mdec->p_n_quantize_y;
		}
		n_packed = psxreadword( p_n_psxram, n_address );
		n_address += 2;
		if( n_packed == 0xfe00 )
		{
			break;
		}

		n_qscale = mdec_unpack_run( n_packed );
		p_n_unpacked[ 0 ] = mdec_unpack_val( n_packed ) * p_n_q[ 0 ];

		n_z = 0;
		for( ;; )
		{
			n_packed = psxreadword( p_n_psxram, n_address );
			n_address += 2;

			if( n_packed == 0xfe00 )
			{
				break;
			}
			n_z += mdec_unpack_run( n_packed ) + 1;
			if( n_z > 63 )
			{
				break;
			}
			p_n_unpacked[ m_p_n_mdec_zigzag[ n_z ] ] = ( mdec_unpack_val( n_packed ) * p_n_q[ n_z ] * n_qscale ) / 8;
		}
		mdec_idct( p_psx, p_n_unpacked, p_n_block );
		p_n_block += DCTSIZE2;
	}
	return n_address;
}

INLINE INT32 mdec_cr_to_r( INT32 n_cr )
{
	return ( 1435 * n_cr ) >> 10;
}

INLINE INT32 mdec_cr_to_g( INT32 n_cr )
{
	return ( -731 * n_cr ) >> 10;
}

INLINE INT32 mdec_cb_to_g( INT32 n_cb )
{
	return ( -351 * n_cb ) >> 10;
}

INLINE INT32 mdec_cb_to_b( INT32 n_cb )
{
	return ( 1814 * n_cb ) >> 10;
}

INLINE UINT16 mdec_clamp_r5( psx_mdec *p_mdec, INT32 n_r )
{
	return p_mdec->p_n_r5[ n_r + 128 + 256 ];
}

INLINE UINT16 mdec_clamp_g5( psx_mdec *p_mdec, INT32 n_g )
{
	return p_mdec->p_n_g5[ n_g + 128 + 256 ];
}

INLINE UINT16 mdec_clamp_b5( psx_mdec *p_mdec, INT32 n_b )
{
	return p_mdec->p_n_b5[ n_b + 128 + 256 ];
}

INLINE void mdec_makergb15( psx_mdec *p_mdec, UINT32 n_address, INT32 n_r, INT32 n_g, INT32 n_b, INT32 *p_n_y, UINT16 n_stp )
{
	p_mdec->p_n_output[ WORD_XOR_LE( n_address + 0 ) / 2 ] = n_stp |
		mdec_clamp_r5( p_mdec, p_n_y[ 0 ] + n_r ) |
		mdec_clamp_g5( p_mdec, p_n_y[ 0 ] + n_g ) |
		mdec_clamp_b5( p_mdec, p_n_y[ 0 ] + n_b );

	p_mdec->p_n_output[ WORD_XOR_LE( n_address + 2 ) / 2 ] = n_stp |
		mdec_clamp_r5( p_mdec, p_n_y[ 1 ] + n_r ) |
		mdec_clamp_g5( p_mdec, p_n_y[ 1 ] + n_g ) |
		mdec_clamp_b5( p_mdec, p_n_y[ 1 ] + n_b );
}

static void mdec_yuv2_to_rgb15( psx_machine *p_psx )
{
	psx_mdec *p_mdec = &p_psx->mdec;
	INT32 n_r;
	INT32 n_g;
	INT32 n_b;
	INT32 n_cb;
	INT32 n_cr;
	INT32 *p_n_cb;
	INT32 *p_n_cr;
	INT32 *p_n_y;
	UINT32 n_x;
	UINT32 n_y;
	UINT32 n_z;
	UINT16 n_stp;
	int n_address = 0;

	if( ( p_mdec->n_0_command & ( 1L << 25 ) ) != 0 )
	{
		n_stp = 0x8000;
	}
	else
	{
		n_stp = 0x0000;
	}

	p_n_cr = &p_mdec->p_n_unpacked[ 0 ];
	p_n_cb = &p_mdec->p_n_unpacked[ DCTSIZE2 ];
	p_n_y = &p_mdec->p_n_unpacked[ DCTSIZE2 * 2 ];

	for( n_z = 0; n_z < 2; n_z++ )
	{
		for( n_y = 0; n_y < 4; n_y++ )
		{
			for( n_x = 0; n_x < 4; n_x++ )
			{
				n_cr = *( p_n_cr );
				n_cb = *( p_n_cb );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb15( p_mdec, ( n_address +  0 ), n_r, n_g, n_b, p_n_y, n_stp );
				mdec_makergb15( p_mdec, ( n_address + 32 ), n_r, n_g, n_b, p_n_y + 8, n_stp );

				n_cr = *( p_n_cr + 4 );
				n_cb = *( p_n_cb + 4 );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb15( p_mdec, ( n_address + 16 ), n_r, n_g, n_b, p_n_y + DCTSIZE2, n_stp );
				mdec_makergb15( p_mdec, ( n_address + 48 ), n_r, n_g, n_b, p_n_y + DCTSIZE2 + 8, n_stp );

				p_n_cr++;
				p_n_cb++;
				p_n_y += 2;
				n_address += 4;
			}
			p_n_cr += 4;
			p_n_cb += 4;
			p_n_y += 8;
			n_address += 48;
		}
		p_n_y += DCTSIZE2;
	}
	p_mdec->n_decoded = ( 16 * 16 ) / 2;
}

INLINE UINT16 mdec_clamp8( psx_mdec *p_mdec, INT32 n_r )
{
	return p_mdec->p_n_clamp8[ n_r + 128 + 256 ];
}

INLINE void mdec_makergb24( psx_mdec *p_mdec, UINT32 n_address, INT32 n_r, INT32 n_g, INT32 n_b, INT32 *p_n_y, UINT32 n_stp )
{
	p_mdec->p_n_output[ WORD_XOR_LE( n_address + 0 ) / 2 ] = ( mdec_clamp8( p_mdec, p_n_y[ 0 ] + n_g ) << 8 ) | mdec_clamp8( p_mdec, p_n_y[ 0 ] + n_r );
	p_mdec->p_n_output[ WORD_XOR_LE( n_address + 2 ) / 2 ] = ( mdec_clamp8( p_mdec, p_n_y[ 1 ] + n_r ) << 8 ) | mdec_clamp8( p_mdec, p_n_y[ 0 ] + n_b );
	p_mdec->p_n_output[ WORD_XOR_LE( n_address + 4 ) / 2 ] = ( mdec_clamp8( p_mdec, p_n_y[ 1 ] + n_b ) << 8 ) | mdec_clamp8( p_mdec, p_n_y[ 1 ] + n_g );
}

static void mdec_yuv2_to_rgb24( psx_machine *p_psx )
{
	psx_mdec *p_mdec = &p_psx->mdec;
	INT32 n_r;
	INT32 n_g;
	INT32 n_b;
	INT32 n_cb;
	INT32 n_cr;
	INT32 *p_n_cb;
	INT32 *p_n_cr;
	INT32 *p_n_y;
	UINT32 n_x;
	UINT32 n_y;
	UINT32 n_z;
	UINT32 n_stp;
	int n_address = 0;

	if( ( p_psx->mdec.n_0_command & ( 1L << 25 ) ) != 0 )
	{
		n_stp = 0x80008000;
	}
	else
	{
		n_stp = 0x00000000;
	}

	p_n_cr = &p_psx->mdec.p_n_unpacked[ 0 ];
	p_n_cb = &p_psx->mdec.p_n_unpacked[ DCTSIZE2 ];
	p_n_y = &p_psx->mdec.p_n_unpacked[ DCTSIZE2 * 2 ];

	for( n_z = 0; n_z < 2; n_z++ )
	{
		for( n_y = 0; n_y < 4; n_y++ )
		{
			for( n_x = 0; n_x < 4; n_x++ )
			{
				n_cr = *( p_n_cr );
				n_cb = *( p_n_cb );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb24( p_mdec, ( n_address +  0 ), n_r, n_g, n_b, p_n_y, n_stp );
				mdec_makergb24( p_mdec, ( n_address + 48 ), n_r, n_g, n_b, p_n_y + 8, n_stp );

				n_cr = *( p_n_cr + 4 );
				n_cb = *( p_n_cb + 4 );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb24( p_mdec, ( n_address + 24 ), n_r, n_g, n_b, p_n_y + DCTSIZE2, n_stp );
				mdec_makergb24( p_mdec, ( n_address + 72 ), n_r, n_g, n_b, p_n_y + DCTSIZE2 + 8, n_stp );

				p_n_cr++;
				p_n_cb++;
				p_n_y += 2;
				n_address += 6;
			}
			p_n_cr += 4;
			p_n_cb += 4;
			p_n_y += 8;
			n_address += 72;
		}
		p_n_y += DCTSIZE2;
	}
	p_psx->mdec.n_decoded = ( 24 * 16 ) / 2;
}

static void mdec0_write( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;
	UINT32 *p_n_psxram = p_psx->p_n_psxram;
	int n_index;

	verboselog( p_psx, 2, "mdec0_write( %08x, %08x )\n", n_address, n_size );
	switch( p_psx->mdec.n_0_command >> 28 )
	{
	case 0x3:
		verboselog( p_psx, 1, "mdec decode %08x %08x %08x\n", p_psx->mdec.n_0_command, n_address, n_size );
		p_psx->mdec.n_0_address = n_address;
		p_psx->mdec.n_0_size = n_size * 4;
		p_psx->mdec.n_1_status |= ( 1L << 29 );
		break;
	case 0x4:
		verboselog( p_psx, 1, "mdec quantize table %08x %08x %08x\n", p_psx->mdec.n_0_command, n_address, n_size );
		n_index = 0;
		while( n_size > 0 )
		{
			if( n_index < DCTSIZE2 )
			{
				p_psx->mdec.p_n_quantize_y[ n_index + 0 ] = ( p_n_psxram[ n_address / 4 ] >> 0 ) & 0xff;
				p_psx->mdec.p_n_quantize_y[ n_index + 1 ] = ( p_n_psxram[ n_address / 4 ] >> 8 ) & 0xff;
				p_psx->mdec.p_n_quantize_y[ n_index + 2 ] = ( p_n_psxram[ n_address / 4 ] >> 16 ) & 0xff;
				p_psx->mdec.p_n_quantize_y[ n_index + 3 ] = ( p_n_psxram[ n_address / 4 ] >> 24 ) & 0xff;
			}
			else if( n_index < DCTSIZE2 * 2 )
			{
				p_psx->mdec.p_n_quantize_uv[ n_index + 0 - DCTSIZE2 ] = ( p_n_psxram[ n_address / 4 ] >> 0 ) & 0xff;
				p_psx->mdec.p_n_quantize_uv[ n_index + 1 - DCTSIZE2 ] = ( p_n_psxram[ n_address / 4 ] >> 8 ) & 0xff;
				p_psx->mdec.p_n_quantize_uv[ n_index + 2 - DCTSIZE2 ] = ( p_n_psxram[ n_address / 4 ] >> 16 ) & 0xff;
				p_psx->mdec.p_n_quantize_uv[ n_index + 3 - DCTSIZE2 ] = ( p_n_psxram[ n_address / 4 ] >> 24 ) & 0xff;
			}
			n_index += 4;
			n_address += 4;
			n_size--;
		}
		break;
	case 0x6:
		verboselog( p_psx, 1, "mdec cosine table %08x %08x %08x\n", p_psx->mdec.n_0_command, n_address, n_size );
		n_index = 0;
		while( n_size > 0 )
		{
			p_psx->mdec.p_n_cos[ n_index + 0 ] = (INT16)( ( p_n_psxram[ n_address / 4 ] >> 0 ) & 0xffff );
			p_psx->mdec.p_n_cos[ n_index + 1 ] = (INT16)( ( p_n_psxram[ n_address / 4 ] >> 16 ) & 0xffff );
			n_index += 2;
			n_address += 4;
			n_size--;
		}
		mdec_cos_precalc(p_psx);
		break;
	default:
		verboselog( p_psx, 0, "mdec unknown command %08x %08x %08x\n", p_psx->mdec.n_0_command, n_address, n_size );
		break;
	}
}

static void mdec1_read( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;
	UINT32 *p_n_psxram = p_psx->p_n_psxram;
	UINT32 n_this;
	UINT32 n_nextaddress;

	verboselog( p_psx, 2, "mdec1_read( %08x, %08x )\n", n_address, n_size );
	if( ( p_psx->mdec.n_0_command & ( 1L << 29 ) ) != 0 && p_psx->mdec.n_0_size != 0 )
	{
		while( n_size > 0 )
		{
			if( p_psx->mdec.n_decoded == 0 )
			{
				if( (int)p_psx->mdec.n_0_size <= 0 )
				{
					mame_printf_debug( "ran out of data %08x\n", n_size );
					p_psx->mdec.n_0_size = 0;
					break;
				}

				n_nextaddress = mdec_unpack( p_psx, p_psx->mdec.n_0_address );
				p_psx->mdec.n_0_size -= n_nextaddress - p_psx->mdec.n_0_address;
				p_psx->mdec.n_0_address = n_nextaddress;

				if( ( p_psx->mdec.n_0_command & ( 1L << 27 ) ) != 0 )
				{
					mdec_yuv2_to_rgb15(p_psx);
				}
				else
				{
					mdec_yuv2_to_rgb24(p_psx);
				}
				p_psx->mdec.n_offset = 0;
			}

			n_this = p_psx->mdec.n_decoded;
			if( n_this > n_size )
			{
				n_this = n_size;
			}
			p_psx->mdec.n_decoded -= n_this;

			memcpy( (UINT8 *)p_n_psxram + n_address, (UINT8 *)p_psx->mdec.p_n_output + p_psx->mdec.n_offset, n_this * 4 );
			p_psx->mdec.n_offset += n_this * 4;
			n_address += n_this * 4;
			n_size -= n_this;
		}

		if( (int)p_psx->mdec.n_0_size < 0 )
		{
			mame_printf_debug( "ran out of data %d\n", p_psx->mdec.n_0_size );
		}
	}
	else
	{
		mame_printf_debug( "mdec1_read no conversion :%08x:%08x:\n", p_psx->mdec.n_0_command, p_psx->mdec.n_0_size );
	}
	p_psx->mdec.n_1_status &= ~( 1L << 29 );
}

WRITE32_HANDLER( psx_mdec_w )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;

	switch( offset )
	{
	case 0:
		verboselog( p_psx, 2, "mdec 0 command %08x\n", data );
		p_psx->mdec.n_0_command = data;
		break;
	case 1:
		verboselog( p_psx, 2, "mdec 1 command %08x\n", data );
		p_psx->mdec.n_1_command = data;
		break;
	}
}

READ32_HANDLER( psx_mdec_r )
{
	psx_machine *p_psx = space->machine->driver_data<psx_state>()->p_psx;

	switch( offset )
	{
	case 0:
		verboselog( p_psx, 2, "mdec 0 status %08x\n", 0 );
		return 0;
	case 1:
		verboselog( p_psx, 2, "mdec 1 status %08x\n", p_psx->mdec.n_1_status );
		return p_psx->mdec.n_1_status;
	}
	return 0;
}

static void gpu_read( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;
	UINT32 *p_n_psxram = p_psx->p_n_psxram;

	psx_gpu_read( machine, &p_n_psxram[ n_address / 4 ], n_size );
}

static void gpu_write( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;
	UINT32 *p_n_psxram = p_psx->p_n_psxram;

	psx_gpu_write( machine, &p_n_psxram[ n_address / 4 ], n_size );
}

void psx_machine_init( running_machine *machine )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;
	int n;

	/* irq */
	p_psx->n_irqdata = 0;
	p_psx->n_irqmask = 0;

	/* dma */
	p_psx->n_dpcp = 0;
	p_psx->n_dicr = 0;

	p_psx->mdec.n_0_command = 0;
	p_psx->mdec.n_0_address = 0;
	p_psx->mdec.n_0_size = 0;
	p_psx->mdec.n_1_command = 0;
	p_psx->mdec.n_1_status = 0;

	for( n = 0; n < 7; n++ )
	{
		dma_stop_timer( p_psx, n );
	}

	for( n = 0; n < 2; n++ )
	{
		p_psx->sio[ n ].n_status = SIO_STATUS_TX_EMPTY | SIO_STATUS_TX_RDY;
		p_psx->sio[ n ].n_mode = 0;
		p_psx->sio[ n ].n_control = 0;
		p_psx->sio[ n ].n_baud = 0;
		p_psx->sio[ n ].n_tx = 0;
		p_psx->sio[ n ].n_rx = 0;
		p_psx->sio[ n ].n_tx_prev = 0;
		p_psx->sio[ n ].n_rx_prev = 0;
		p_psx->sio[ n ].n_rx_data = 0;
		p_psx->sio[ n ].n_tx_data = 0;
		p_psx->sio[ n ].n_rx_shift = 0;
		p_psx->sio[ n ].n_tx_shift = 0;
		p_psx->sio[ n ].n_rx_bits = 0;
		p_psx->sio[ n ].n_tx_bits = 0;
	}

	psx_gpu_reset(machine);
}

static STATE_POSTLOAD( psx_postload )
{
	psx_machine *p_psx = machine->driver_data<psx_state>()->p_psx;
	int n;

	psx_irq_update(p_psx);

	for( n = 0; n < 7; n++ )
	{
		dma_timer_adjust( p_psx, n );
	}

	for( n = 0; n < 3; n++ )
	{
		root_timer_adjust( p_psx, n );
	}

	for( n = 0; n < 2; n++ )
	{
		sio_timer_adjust( p_psx, n );
	}

	mdec_cos_precalc(p_psx);
}

void psx_driver_init( running_machine *machine )
{
	psx_state *state = machine->driver_data<psx_state>();
	psx_machine *p_psx = auto_alloc_clear(machine, psx_machine);
	int n;

	state->p_psx = p_psx;
	state->p_n_psxram = (UINT32 *)memory_get_shared(*machine, "share1", state->n_psxramsize);

	p_psx->machine = machine;
	p_psx->p_n_psxram = state->p_n_psxram;
	p_psx->n_psxramsize = state->n_psxramsize;

	for( n = 0; n < 7; n++ )
	{
		p_psx->channel[ n ].timer = timer_alloc( machine, dma_finished_callback, machine );
		p_psx->channel[ n ].fn_read = NULL;
		p_psx->channel[ n ].fn_write = NULL;
	}

	for( n = 0; n < 3; n++ )
	{
		p_psx->root[ n ].timer = timer_alloc( machine, root_finished , NULL);
	}

	for( n = 0; n < 2; n++ )
	{
		p_psx->sio[ n ].timer = timer_alloc( machine, sio_clock , NULL);
		p_psx->sio[ n ].fn_handler = NULL;
	}

	for( n = 0; n < 256; n++ )
	{
		p_psx->mdec.p_n_clamp8[ n ] = 0;
		p_psx->mdec.p_n_clamp8[ n + 256 ] = n;
		p_psx->mdec.p_n_clamp8[ n + 512 ] = 255;

		p_psx->mdec.p_n_r5[ n ] = 0;
		p_psx->mdec.p_n_r5[ n + 256 ] = ( n >> 3 );
		p_psx->mdec.p_n_r5[ n + 512 ] = ( 255 >> 3 );

		p_psx->mdec.p_n_g5[ n ] = 0;
		p_psx->mdec.p_n_g5[ n + 256 ] = ( n >> 3 ) << 5;
		p_psx->mdec.p_n_g5[ n + 512 ] = ( 255 >> 3 ) << 5;

		p_psx->mdec.p_n_b5[ n ] = 0;
		p_psx->mdec.p_n_b5[ n + 256 ] = ( n >> 3 ) << 10;
		p_psx->mdec.p_n_b5[ n + 512 ] = ( 255 >> 3 ) << 10;
	}

	psx_dma_install_read_handler( machine, 1, mdec1_read );
	psx_dma_install_read_handler( machine, 2, gpu_read );

	psx_dma_install_write_handler( machine, 0, mdec0_write );
	psx_dma_install_write_handler( machine, 2, gpu_write );

	state_save_register_global( machine, p_psx->n_irqdata );
	state_save_register_global( machine, p_psx->n_irqmask );
	for (n = 0; n < 7; n++ )
	{
		state_save_register_item( machine, "psxdma", NULL, n, p_psx->channel[n].n_base );
		state_save_register_item( machine, "psxdma", NULL, n, p_psx->channel[n].n_blockcontrol );
		state_save_register_item( machine, "psxdma", NULL, n, p_psx->channel[n].n_channelcontrol );
		state_save_register_item( machine, "psxdma", NULL, n, p_psx->channel[n].n_ticks );
		state_save_register_item( machine, "psxdma", NULL, n, p_psx->channel[n].b_running );
	}

	state_save_register_global( machine, p_psx->n_dpcp );
	state_save_register_global( machine, p_psx->n_dicr );
	for ( n = 0; n < 3; n++ )
	{
		state_save_register_item( machine, "psxroot", NULL, n, p_psx->root[n].n_count );
		state_save_register_item( machine, "psxroot", NULL, n, p_psx->root[n].n_mode );
		state_save_register_item( machine, "psxroot", NULL, n, p_psx->root[n].n_target );
		state_save_register_item( machine, "psxroot", NULL, n, p_psx->root[n].n_start );
	}
	for( n = 0; n < 2; n++ )
	{
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_status );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_mode );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_control );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_baud );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_tx );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_rx );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_tx_prev );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_rx_prev );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_rx_data );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_tx_data );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_rx_shift );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_tx_shift );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_rx_bits );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_tx_bits );
	}

	state_save_register_global( machine, p_psx->mdec.n_0_command );
	state_save_register_global( machine, p_psx->mdec.n_0_address );
	state_save_register_global( machine, p_psx->mdec.n_0_size );
	state_save_register_global( machine, p_psx->mdec.n_1_command );
	state_save_register_global( machine, p_psx->mdec.n_1_status );
	state_save_register_global_array( machine, p_psx->mdec.p_n_quantize_y );
	state_save_register_global_array( machine, p_psx->mdec.p_n_quantize_uv );
	state_save_register_global_array( machine, p_psx->mdec.p_n_cos );

	state_save_register_postload( machine, psx_postload, NULL );
}
