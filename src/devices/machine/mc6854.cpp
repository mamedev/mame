// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Motorola 6854 emulation.

  The MC6854 chip is an Advanced Data-Link Controller (ADLC).
  It provides a high-level network interface that can transimit frames with
  arbitrary data and address length, and is compatible with the following
  standards:
  - ADCCP (Advanced Data Communication Control Procedure)
  - HDLC  (High-Level Data-Link Control)
  - SDLC  (Synchronous Data-Link Control)
  It is designed to be interfaced with a M6800-family CPU.

  It is used in the "Nano-network" extension of the Thomson computers to
  link up to 32 computers at 500 Kbps.
  Many networks involving one PC server and several MO5 or TO7/70 computers
  were build in French schools in the 1980's to teach computer science.

  TODO:
  - CRC
  - DMA mode
  - loop mode
  - status prioritization
  - NRZI vs. NRZ coding
  - FD output

**********************************************************************/


#include "emu.h"
#include "mc6854.h"


/******************* parameters ******************/



#define VERBOSE 0


#define FLAG 0x7e
/* flag value, as defined by HDLC protocol: 01111110 */

#define BIT_LENGTH attotime::from_hz( 500000 )


/******************* utility function and macros ********************/



#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)



/* control register 1 */

#define AC ( m_cr1 & 1 )
#define FCTDRA ( m_cr2 & 8 )
/* extra register select bits */

#define RRESET ( m_cr1 & 0x40 )
#define TRESET ( m_cr1 & 0x80 )
/* transmit / reset condition */

#define RIE ( m_cr1 & 2 )
#define TIE ( m_cr1 & 4 )
/* interrupt enable */

#define DISCONTINUE ( m_cr1 & 0x20 )
/* discontinue received frame */



/* control register 2 */

#define PSE ( m_cr2 & 1 )
/* prioritize status bits (TODO) */

#define TWOBYTES ( m_cr2 & 2 )
/* two-bytes mode */

#define FMIDLE ( m_cr2 & 4 )
/* flag time fill (vs. mark idle) */

#define TLAST ( m_cr2 & 0x10 )
/* transmit last byte of frame */

#define RTS ( m_cr2 & 0x80 )
/* request-to-send */



/* control register 3 */

#define LCF ( m_cr3 & 1 )
/* logical control field select */

#define CEX ( m_cr3 & 2 )
/* control field is 16 bits instead of 8 */

#define AEX ( m_cr3 & 4 )
/* extended address mode (vs normal 8-bit address mode) */

#define IDL0 ( m_cr3 & 8 )
/* idle condition begins with a '0' instead of a '1" */

#define FDSE ( m_cr3 & 0x10 )
/* enable the flag detect status in SR1 */

#define LOOP ( m_cr3 & 0x20 )
/* loop mode */

#define TST ( m_cr3 & 0x40 )
/* test mode (or go active on poll) */

#define DTR ( m_cr3 & 0x80 )
/* data-transmit-ready (or loop on-line control) */



/* control register 4 */

#define TWOINTER ( m_cr4 & 1 )
/* both an openning and a closing inter-frame are sent */

static const int word_length[4] = { 5, 6, 7, 8 };
#define TWL word_length[ ( m_cr4 >> 1 ) & 3 ]
#define RWL word_length[ ( m_cr4 >> 3 ) & 3 ]
/* transmit / receive word length */

#define ABT ( m_cr4 & 0x20 )
/* aborts */

#define ABTEX ( m_cr4 & 0x40 )
/* abort generates 16 '1' bits instead of 8 */

#define NRZ ( m_cr4 & 0x80 )
/* zero complement / non-zero complement data format */



/* status register 1 */
#define RDA  0x01  /* receiver data available */
#define S2RQ 0x02  /* status register #2 read request */
#define FD   0x04  /* flag detect */
#define CTS  0x10  /* clear-to-send */
#define TU   0x20  /* transmitter underrun */
#define TDRA 0x40  /* transmitter data register available */
#define IRQ  0x80  /* interrupt request */


/* status register 2 */
#define AP    0x01  /* address present */
#define FV    0x02  /* frame valid */
#define RIDLE 0x04  /* receiver idle */
#define RABT  0x08  /* receiver abort */
#define ERR   0x10  /* invalid frame error */
#define DCD   0x20  /* data carrier detect (ignored) */
#define OVRN  0x40  /* receiver overrun */
#define RDA2  0x80  /* copy of RDA */



const device_type MC6854 = &device_creator<mc6854_device>;

mc6854_device::mc6854_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MC6854, "MC6854 ADLC", tag, owner, clock, "mc6854", __FILE__),
	m_out_irq_cb(*this),
	m_out_txd_cb(*this),
	m_out_rts_cb(*this),
	m_out_dtr_cb(*this),
	m_cr1(0),
	m_cr2(0),
	m_cr3(0),
	m_cr4(0),
	m_sr1(0),
	m_sr2(0),
	m_cts(0),
	m_dcd(0),
	m_tstate(0),
	m_tones(0),
	m_ttimer(nullptr),
	m_rstate(0),
	m_rreg(0),
	m_rones(0),
	m_rsize(0),
	m_flen(0),
	m_fpos(0)
{
	for (int i = 0; i < MC6854_FIFO_SIZE; i++)
	{
		m_tfifo[i] = 0;
		m_rfifo[i] = 0;
	}

	for (auto & elem : m_frame)
	{
		elem = 0;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc6854_device::device_start()
{
	m_out_irq_cb.resolve_safe();
	m_out_txd_cb.resolve();
	m_out_frame_cb.bind_relative_to(*owner());
	m_out_rts_cb.resolve_safe();
	m_out_dtr_cb.resolve_safe();

	m_ttimer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc6854_device::tfifo_cb), this));

	save_item(NAME(m_cr1));
	save_item(NAME(m_cr2));
	save_item(NAME(m_cr3));
	save_item(NAME(m_cr4));
	save_item(NAME(m_sr1));
	save_item(NAME(m_sr2));
	save_item(NAME(m_cts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_tstate));
	save_item(NAME(m_tfifo));
	save_item(NAME(m_tones));
	save_item(NAME(m_rstate));
	save_item(NAME(m_rreg));
	save_item(NAME(m_rones));
	save_item(NAME(m_rsize));
	save_item(NAME(m_rfifo));
	save_item(NAME(m_frame));
	save_item(NAME(m_flen));
	save_item(NAME(m_fpos));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc6854_device::device_reset()
{
	LOG (( "mc6854 reset\n" ));
	m_cr1 = 0xc0; /* reset condition */
	m_cr2 = 0;
	m_cr3 = 0;
	m_cr4 = 0;
	m_sr1 = 0;
	m_sr2 = 0;
	m_cts = 0;
	m_dcd = 0;
	tfifo_clear( );
	rfifo_clear( );
}

/*********************** transmit ***********************/



/* MC6854 fills bit queue */
void mc6854_device::send_bits( UINT32 data, int len, int zi )
{
	attotime expire;
	int i;
	if ( zi )
	{
		/* zero-insertion mode */
		UINT32 d = 0;
		int l = 0;
		for ( i = 0; i < len; i++, data >>= 1, l++ )
		{
			if ( data & 1 )
			{
				d |= 1 << l;
				m_tones++;
				if ( m_tones == 5 )
				{
					/* insert a '0' after 5 consecutive '1" */
					m_tones = 0;
					l++;
				}
			}
			else
				m_tones = 0;
		}
		data = d;
		len = l;
	}
	else
		m_tones = 0;

	/* send bits */
	if ( !m_out_txd_cb.isnull() )
	{
		for ( i = 0; i < len; i++, data >>= 1 )
			m_out_txd_cb( data & 1 );
	}

	/* schedule when to ask the MC6854 for more bits */
	expire = m_ttimer ->remaining( );
	if ( expire== attotime::never )
		expire = attotime::zero;
	m_ttimer->reset( expire + (BIT_LENGTH * len));
}



/* CPU push -> tfifo[0] -> ... -> tfifo[MC6854_FIFO_SIZE-1] -> pop */
void mc6854_device::tfifo_push( UINT8 data )
{
	int i;

	if ( TRESET )
		return;

	/* push towards the rightmost free entry */
	for ( i = MC6854_FIFO_SIZE - 1; i >= 0; i-- )
	{
		if ( ! ( m_tfifo[ i ] & 0x100 ) )
			break;
	}

	if ( i >= 0 )
		m_tfifo[ i ] = data | 0x100;
	else
		logerror( "%f mc6854_tfifo_push: FIFO overrun\n", machine().time().as_double() );

	/* start frame, if needed */
	if ( ! m_tstate )
	{
		LOG(( "%f mc6854_tfifo_push: start frame\n", machine().time().as_double() ));
		m_tstate = 2;
		send_bits( FLAG, 8, 0 );
	}
}



/* CPU asks for normal frame termination */
void mc6854_device::tfifo_terminate( )
{
	/* mark most recently pushed byte as the last one of the frame */
	int i;
	for ( i = 0; i < MC6854_FIFO_SIZE; i++ )
	{
		if ( m_tfifo[ i ] & 0x100 )
		{
			m_tfifo[ i ] |= 0x200;
			break;
		}
	}
}



/* call-back to refill the bit-stream from the FIFO */
TIMER_CALLBACK_MEMBER(mc6854_device::tfifo_cb)
{
	int i, data = m_tfifo[ MC6854_FIFO_SIZE - 1 ];

	if ( ! m_tstate )
		return;

	/* shift FIFO to the right */
	for ( i = MC6854_FIFO_SIZE - 1; i > 0; i-- )
		m_tfifo[ i ] = m_tfifo[ i - 1 ];
	m_tfifo[ 0 ] = 0;

	if ( data & 0x100 )
	{
		/* got data */

		int blen = 8;

		switch ( m_tstate )
		{
		case 2: /* 8-bit address field */
			if ( ( data & 1 ) || ( ! AEX ) )
				m_tstate = 3;
			LOG(( "%f mc6854_tfifo_cb: address field $%02X\n", machine().time().as_double(), data & 0xff ));
			break;

		case 3: /* 8-bit control field */
			if ( CEX )
				m_tstate = 4;
			else if ( LCF )
				m_tstate = 5;
			else
				m_tstate = 6;
			LOG(( "%f mc6854_tfifo_cb: control field $%02X\n", machine().time().as_double(), data & 0xff ));
			break;

		case 4: /* 8-bit extended control field (optional) */
			if ( LCF )
				m_tstate = 5;
			else
				m_tstate = 6;
			LOG(( "%f mc6854_tfifo_cb: control field $%02X\n", machine().time().as_double(), data & 0xff ));
			break;

		case 5: /* 8-bit logical control (optional) */
			if ( ! ( data & 0x80 ) )
				m_tstate = 6;
			LOG(( "%f mc6854_tfifo_cb: logical control field $%02X\n", machine().time().as_double(), data & 0xff ));
			break;

		case 6: /* variable-length data */
			blen = TWL;
			LOG(( "%f mc6854_tfifo_cb: data field $%02X, %i bits\n", machine().time().as_double(), data & 0xff, blen ));
			break;

		default:
			LOG(( "%f mc6854_tfifo_cb: state=%i\n", machine().time().as_double(), m_tstate));
		}

		if ( m_flen < MAX_FRAME_LENGTH )
			m_frame[ m_flen++ ] = data;
		else
			logerror( "mc6854_tfifo_cb: truncated frame, max=%i\n", MAX_FRAME_LENGTH );

		send_bits( data, blen, 1 );
	}
	else
	{
		/* data underrun => abort */
		logerror( "%f mc6854_tfifo_cb: FIFO underrun\n", machine().time().as_double() );
		m_sr1 |= TU;
		m_tstate = 0;
		send_bits( 0xffff, ABTEX ? 16 : 8, 0 );
		m_flen = 0;
	}

	/* close frame, if needed */
	if ( data & 0x200 )
	{
		int len = m_flen;

		LOG(( "%f mc6854_tfifo_cb: end frame\n", machine().time().as_double() ));
		send_bits( 0xdeadbeef, 16, 1 );  /* send check-sum: TODO */
		send_bits( FLAG, 8, 0 );         /* send closing flag */

		if ( m_tfifo[ MC6854_FIFO_SIZE - 1 ] & 0x100 )
		{
			/* re-open frame asap */
			LOG(( "%f mc6854_tfifo_cb: start frame\n", machine().time().as_double() ));
			if ( TWOINTER )
				send_bits( FLAG, 8, 0 );
		}
		else
			m_tstate = 0;

		m_flen = 0;
		if ( !m_out_frame_cb.isnull() )
			m_out_frame_cb( m_frame, len );
	}
}



void mc6854_device::tfifo_clear( )
{
	memset( m_tfifo, 0, sizeof( m_tfifo ) );
	m_tstate = 0;
	m_flen = 0;
	m_ttimer->reset(  );
}



/*********************** receive ***********************/



/* MC6854 pushes a field in the FIFO */
void mc6854_device::rfifo_push( UINT8 d )
{
	int i, blen = 8;
	unsigned data = d;

	switch ( m_rstate )
	{
	case 0:
	case 1:
	case 2: /* 8-bit address field */
		if ( ( data & 1 ) || ( ! AEX ) )
			m_rstate = 3;
		else
			m_rstate = 2;
		LOG(( "%f mc6854_rfifo_push: address field $%02X\n", machine().time().as_double(), data ));
		data |= 0x400; /* address marker */
		break;

	case 3: /* 8-bit control field */
		if ( CEX )
			m_rstate = 4;
		else if ( LCF )
			m_rstate = 5;
		else
			m_rstate = 6;
		LOG(( "%f mc6854_rfifo_push: control field $%02X\n", machine().time().as_double(), data ));
		break;

	case 4: /* 8-bit extended control field (optional) */
		if ( LCF )
			m_rstate = 5;
		else
			m_rstate = 6;
		LOG(( "%f mc6854_rfifo_push: control field $%02X\n", machine().time().as_double(), data ));
		break;

	case 5: /* 8-bit logical control (optional) */
		if ( ! ( data & 0x80 ) )
			m_rstate = 6;
		LOG(( "%f mc6854_rfifo_push: logical control field $%02X\n", machine().time().as_double(), data ));
		break;

	case 6: /* variable-length data */
		blen = RWL;
		data >>= 8 - blen;
		LOG(( "%f mc6854_rfifo_push: data field $%02X, %i bits\n", machine().time().as_double(), data, blen ));
		break;
	}

	/* no further FIFO fill until FV is cleared! */
	if ( m_sr2 & FV )
	{
		LOG(( "%f mc6854_rfifo_push: field not pushed\n", machine().time().as_double() ));
		return;
	}

	data |= 0x100; /* entry full marker */

	/* push towards the rightmost free entry */
	for ( i = MC6854_FIFO_SIZE - 1; i >= 0; i-- )
	{
		if ( ! ( m_rfifo[ i ] & 0x100 ) )
			break;
	}

	if ( i >= 0 )
		m_rfifo[ i ] = data | 0x100;
	else
	{
		/* FIFO full */
		m_sr2 |= OVRN;
		m_rfifo[ 0 ] = data;
		logerror( "%f mc6854_rfifo_push: FIFO overrun\n", machine().time().as_double() );
	}

	m_rsize -= blen;
}



void mc6854_device::rfifo_terminate( )
{
	/* mark most recently pushed byte as the last one of the frame */
	int i;
	for ( i = 0; i < MC6854_FIFO_SIZE; i++ )
	{
		if ( m_rfifo[ i ] & 0x100 )
		{
			m_tfifo[ i ] |= 0x200;
			break;
		}

	}

	m_flen = 0;
	m_rstate = 1;
}



/* CPU pops the FIFO */
UINT8 mc6854_device::rfifo_pop( )
{
	int i, data = m_rfifo[ MC6854_FIFO_SIZE - 1 ];

	/* shift FIFO to the right */
	for ( i = MC6854_FIFO_SIZE - 1; i > 0; i -- )
		m_rfifo[ i ] = m_rfifo[ i - 1 ];
	m_rfifo[ 0 ] = 0;

	if ( m_rfifo[ MC6854_FIFO_SIZE - 1 ] & 0x200 )
	{
		/* last byte in frame */
		m_sr2 |= FV; /* TODO: check CRC & set ERR instead of FV if error*/
	}

	/* auto-refill in frame mode */
	if ( m_flen > 0 )
	{
		rfifo_push( m_frame[ m_fpos++ ] );
		if ( m_fpos == m_flen )
			rfifo_terminate( );
	}

	return data;
}


/* MC6854 makes fields from bits */
WRITE_LINE_MEMBER( mc6854_device::set_rx )
{
	int fieldlen = ( m_rstate < 6 ) ? 8 : RWL;

	if ( RRESET || (m_sr2 & DCD) )
		return;

	if ( state )
	{
		m_rones++;
		m_rreg = (m_rreg >> 1) | 0x80000000;
		if ( m_rones >= 8 )
		{
			/* abort */
			m_rstate = 0;
			m_rsize = 0;
			if ( m_rstate > 1 )
			{
				/* only in-frame abort */
				m_sr2 |= RABT;
				LOG(( "%f mc6854_receive_bit: abort\n", machine().time().as_double() ));
			}
		}
		else
		{
			m_rsize++;
			if ( m_rstate && m_rsize >= fieldlen + 24 )
				rfifo_push( m_rreg );
		}
	}
	else if ( m_rones == 5 )
	{
		/* discards '0' inserted after 5 '1' */
		m_rones = 0;
		return;
	}
	else if ( m_rones == 6 )
	{
		/* flag */
		if ( FDSE )
			m_sr1 |= FD;

		if ( m_rstate > 1 )
		{
			/* end of frame */
			m_rreg >>= 1;
			m_rsize++;
			if ( m_rsize >= fieldlen + 24 ) /* last field */
				rfifo_push( m_rreg );
			rfifo_terminate( );
			LOG(( "%f mc6854_receive_bit: end of frame\n", machine().time().as_double() ));
		}
		m_rones = 0;
		m_rstate = 1;
		m_rsize = 0;
	} else
	{
		m_rones = 0;
		m_rreg >>= 1;
		m_rsize++;
		if ( m_rstate && m_rsize >= fieldlen + 24 )
			rfifo_push( m_rreg );
	}
}



void mc6854_device::rfifo_clear( )
{
	memset( m_rfifo, 0, sizeof( m_rfifo ) );
	m_rstate = 0;
	m_rreg = 0;
	m_rsize = 0;
	m_rones = 0;
	m_flen = 0;
}



int mc6854_device::send_frame( UINT8* data, int len )
{
	if ( m_rstate > 1 || m_tstate > 1 || RTS )
		return -1; /* busy */

	if ( len > MAX_FRAME_LENGTH )
	{
		logerror( "mc6854_send_frame: truncated frame, size=%i, max=%i\n", len, MAX_FRAME_LENGTH );
		len = MAX_FRAME_LENGTH;
	}
	else if ( len < 2 )
	{
		logerror( "mc6854_send_frame: frame too short, size=%i, min=2\n", len );
		len = 2;
	}
	memcpy( m_frame, data, len );
	if ( FDSE )
		m_sr1 |= FD;
	m_flen = len;
	m_fpos = 0;
	rfifo_push( m_frame[ m_fpos++ ] );
	rfifo_push( m_frame[ m_fpos++ ] );
	if ( m_fpos == m_flen )
		rfifo_terminate( );
	return 0;
}



/************************** CPU interface ****************************/



WRITE_LINE_MEMBER( mc6854_device::set_cts )
{
	if ( ! m_cts && state )
		m_sr1 |= CTS;
	m_cts = state;

	if ( m_cts )
		m_sr1 |= CTS;
	else
		m_sr1 &= ~CTS;
}



WRITE_LINE_MEMBER( mc6854_device::set_dcd )
{
	if ( ! m_dcd && state )
	{
		m_sr2 |= DCD;
		/* partial reset */
		m_rstate = 0;
		m_rreg = 0;
		m_rsize = 0;
		m_rones = 0;
	}
	m_dcd = state;
}



void mc6854_device::update_sr2( )
{
	/* update RDA */
	m_sr2 |= RDA2;
	if ( ! (m_rfifo[ MC6854_FIFO_SIZE - 1 ] & 0x100) )
		m_sr2 &= ~RDA2;
	else if ( TWOBYTES && ! (m_tfifo[ MC6854_FIFO_SIZE - 2 ] & 0x100) )
		m_sr2 &= ~RDA2;

	/* update AP */
	if ( m_rfifo[ MC6854_FIFO_SIZE - 1 ] & 0x400 )
		m_sr2 |= AP;
	else
		m_sr2 &= ~AP;
}



void mc6854_device::update_sr1( )
{
	update_sr2( );

	/* update S2RQ */
	if ( m_sr2 & 0x7f )
		m_sr1 |= S2RQ;
	else
		m_sr1 &= ~S2RQ;

	/* update TRDA (always prioritized by CTS) */
	if ( TRESET || ( m_sr1 & CTS ) )
		m_sr1 &= ~TDRA;
	else
	{
		m_sr1 |= TDRA;
		if ( m_tfifo[ 0 ] & 0x100 )
			m_sr1 &= ~TDRA;
		else if ( TWOBYTES && (m_tfifo[ 1 ] & 0x100) )
			m_sr1 &= ~TDRA;
	}

	/* update RDA */
	if ( m_sr2 & RDA2 )
		m_sr1 |= RDA;
	else
		m_sr1 &= ~RDA;

	/* update IRQ */
	m_sr1 &= ~IRQ;
	if ( RIE && (m_sr1 & (TU | TDRA) ) )
		m_sr1 |= IRQ;
	if ( TIE )
	{
		if ( m_sr1 & (S2RQ | RDA | CTS) )
			m_sr1 |= IRQ;
		if ( m_sr2 & (ERR | FV | DCD | OVRN | RABT | RIDLE | AP) )
			m_sr1 |= IRQ;
	}

	m_out_irq_cb((m_sr1 & IRQ) ? ASSERT_LINE : CLEAR_LINE);
}



READ8_MEMBER( mc6854_device::read )
{
	switch ( offset )
	{
	case 0: /* status register 1 */
		update_sr1( );
		LOG(( "%f %s mc6854_r: get SR1=$%02X (rda=%i,s2rq=%i,fd=%i,cts=%i,tu=%i,tdra=%i,irq=%i)\n",
				space.machine().time().as_double(), machine().describe_context(), m_sr1,
				( m_sr1 & RDA) ? 1 : 0, ( m_sr1 & S2RQ) ? 1 : 0,
				( m_sr1 & FD ) ? 1 : 0, ( m_sr1 & CTS ) ? 1 : 0,
				( m_sr1 & TU ) ? 1 : 0, ( m_sr1 & TDRA) ? 1 : 0,
				( m_sr1 & IRQ) ? 1 : 0 ));
		return m_sr1;

	case 1: /* status register 2 */
		update_sr2( );
		LOG(( "%f %s mc6854_r: get SR2=$%02X (ap=%i,fv=%i,ridle=%i,rabt=%i,err=%i,dcd=%i,ovrn=%i,rda2=%i)\n",
				space.machine().time().as_double(), machine().describe_context(), m_sr2,
				( m_sr2 & AP   ) ? 1 : 0, ( m_sr2 & FV  ) ? 1 : 0,
				( m_sr2 & RIDLE) ? 1 : 0, ( m_sr2 & RABT) ? 1 : 0,
				( m_sr2 & ERR  ) ? 1 : 0, ( m_sr2 & DCD ) ? 1 : 0,
				( m_sr2 & OVRN ) ? 1 : 0, ( m_sr2 & RDA2) ? 1 : 0 ));
		return m_sr2;

	case 2: /* receiver data register */
	case 3:
	{
		UINT8 data = rfifo_pop( );
		LOG(( "%f %s mc6854_r: get data $%02X\n",
				space.machine().time().as_double(), machine().describe_context(), data ));
		return data;
	}

	default:
		logerror( "%s mc6854 invalid read offset %i\n", machine().describe_context(), offset );
	}
	return 0;
}



WRITE8_MEMBER( mc6854_device::write )
{
	switch ( offset )
	{
	case 0: /* control register 1 */
		m_cr1 = data;
		LOG(( "%f %s mc6854_w: set CR1=$%02X (ac=%i,irq=%c%c,%sreset=%c%c)\n",
				space.machine().time().as_double(), machine().describe_context(), m_cr1,
				AC ? 1 : 0,
				RIE ? 'r' : '-', TIE ? 't' : '-',
				DISCONTINUE ? "discontinue," : "",
				RRESET ? 'r' : '-', TRESET ? 't' : '-'
				));
		if ( m_cr1 & 0xc )
			logerror( "%s mc6854 DMA not handled (CR1=$%02X)\n",
					machine().describe_context(), m_cr1 );
		if ( DISCONTINUE )
		{
			/* abort receive FIFO but keeps shift register & synchro */
			m_rstate = 0;
			memset( m_rfifo, 0, sizeof( m_rfifo ) );
		}
		if ( RRESET )
		{
			/* abort FIFO & synchro */
			rfifo_clear( );
			m_sr1 &= ~FD;
			m_sr2 &= ~(AP | FV | RIDLE | RABT | ERR | OVRN | DCD);
			if ( m_dcd ) m_sr2 |= DCD;
		}
		if ( TRESET )
		{
			tfifo_clear( );
			m_sr1 &= ~(TU | TDRA | CTS);
			if ( m_cts ) m_sr1 |= CTS;
		}
		break;

	case 1:
		if ( AC )
		{
			/* control register 3 */
			m_cr3 = data;
			LOG(( "%f %s mc6854_w: set CR3=$%02X (lcf=%i,aex=%i,idl=%i,fdse=%i,loop=%i,tst=%i,dtr=%i)\n",
					space.machine().time().as_double(), machine().describe_context(), m_cr3,
					LCF ? (CEX ? 16 : 8) : 0,  AEX ? 1 : 0,
					IDL0 ? 0 : 1, FDSE ? 1 : 0, LOOP ? 1 : 0,
					TST ? 1 : 0, DTR ? 1 : 0
					));
			if ( LOOP )
				logerror( "%s mc6854 loop mode not handled (CR3=$%02X)\n", machine().describe_context(), m_cr3 );
			if ( TST )
				logerror( "%s mc6854 test mode not handled (CR3=$%02X)\n", machine().describe_context(), m_cr3 );

			m_out_dtr_cb( DTR ? 1 : 0 );

		}
		else
		{
			/* control register 2 */
			m_cr2 = data;
			LOG(( "%f %s mc6854_w: set CR2=$%02X (pse=%i,bytes=%i,fmidle=%i,%s,tlast=%i,clr=%c%c,rts=%i)\n",
					space.machine().time().as_double(), machine().describe_context(), m_cr2,
					PSE ? 1 : 0,  TWOBYTES ? 2 : 1,  FMIDLE ? 1 : 0,
					FCTDRA ? "fc" : "tdra", TLAST ? 1 : 0,
					data & 0x20 ? 'r' : '-',  data & 0x40 ? 't' : '-',
					RTS ? 1 : 0 ));
			if ( PSE )
				logerror( "%s mc6854 status prioritization not handled (CR2=$%02X)\n", machine().describe_context(), m_cr2 );
			if ( TLAST )
				tfifo_terminate( );
			if ( data & 0x20 )
			{
				/* clear receiver status */
				m_sr1 &= ~FD;
				m_sr2 &= ~(AP | FV | RIDLE | RABT | ERR | OVRN | DCD);
				if ( m_dcd )
					m_sr2 |= DCD;
			}
			if ( data & 0x40 )
			{
				/* clear transmitter status */
				m_sr1 &= ~(TU | TDRA | CTS);
				if ( m_cts )
					m_sr1 |= CTS;
			}

			m_out_rts_cb( RTS ? 1 : 0 );
		}
		break;

	case 2: /* transmitter data: continue data */
		LOG(( "%f %smc6854_w: push data=$%02X\n", space.machine().time().as_double(), machine().describe_context(), data ));
		tfifo_push( data );
		break;

	case 3:
		if ( AC )
		{
			/* control register 4 */
			m_cr4 = data;
			LOG(( "%f %s mc6854_w: set CR4=$%02X (interframe=%i,tlen=%i,rlen=%i,%s%s)\n", space.machine().time().as_double(), machine().describe_context(), m_cr4,
					TWOINTER ? 2 : 1,
					TWL, RWL,
					ABT ? ( ABTEX ? "abort-ext," : "abort,") : "",
					NRZ ? "nrz" : "nrzi" ));
			if ( ABT )
			{
				m_tstate = 0;
				send_bits( 0xffff, ABTEX ? 16 : 8, 0 );
				m_flen = 0;
			}
		}
		else
		{
			/* transmitter data: last data */
			LOG(( "%f %s mc6854_w: push last-data=$%02X\n", space.machine().time().as_double(), machine().describe_context(), data ));
			tfifo_push( data );
			tfifo_terminate( );
		}
		break;

	default:
		logerror( "%s mc6854 invalid write offset %i (data=$%02X)\n", machine().describe_context(), offset, data );
	}
}

WRITE_LINE_MEMBER( mc6854_device::rxc_w )
{
	// TODO
}

WRITE_LINE_MEMBER( mc6854_device::txc_w )
{
	// TODO
}
