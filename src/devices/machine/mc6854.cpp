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
  - loop mode
  - status prioritization
  - 01/11 idle
  - partial last byte of the information field
  - FD output

**********************************************************************/


#include "emu.h"
#include "mc6854.h"

#define LOG_SETUP    (1U << 1)
#define LOG_BITS     (1U << 2)
#define LOG_IRQ      (1U << 3)

//#define VERBOSE (LOG_IRQ | LOG_BITS | LOG_GENERAL | LOG_SETUP)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSETUP(...)   LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGBITS(...)    LOGMASKED(LOG_BITS,    __VA_ARGS__)
#define LOGIRQ(...)     LOGMASKED(LOG_IRQ,     __VA_ARGS__)


/******************* parameters ******************/



#define FLAG 0x7e
/* flag value, as defined by HDLC protocol: 01111110 */

constexpr unsigned mc6854_device::MAX_FRAME_LENGTH;


/******************* utility function and macros ********************/



/* control register 1 */

#define AC ( m_cr1 & 1 )        /* extra register select bits */
#define FCTDRA ( m_cr2 & 8 )

#define RRESET ( m_cr1 & 0x40 ) /* transmit / reset condition */
#define TRESET ( m_cr1 & 0x80 )

#define RIE ( m_cr1 & 2 )       /* interrupt enable */
#define TIE ( m_cr1 & 4 )

#define RDSR ( m_cr1 & 8 )       /* DMA mode */
#define TDSR ( m_cr1 & 0x10 )

#define DISCONTINUE ( m_cr1 & 0x20 ) /* discontinue received frame */


/* control register 2 */

#define PSE ( m_cr2 & 1 ) /* prioritize status bits (TODO) */
#define TWOBYTES ( m_cr2 & 2 ) /* two-bytes mode */
#define FMIDLE ( m_cr2 & 4 )   /* flag time fill (vs. mark idle) */
#define TLAST ( m_cr2 & 0x10 ) /* transmit last byte of frame */
#define RTS ( m_cr2 & 0x80 )   /* request-to-send */


/* control register 3 */

#define LCF ( m_cr3 & 1 ) /* logical control field select */
#define CEX ( m_cr3 & 2 ) /* control field is 16 bits instead of 8 */
#define AEX ( m_cr3 & 4 ) /* extended address mode (vs normal 8-bit address mode) */
#define IDL0 ( m_cr3 & 8 ) /* idle condition begins with a '0' instead of a '1" */
#define FDSE ( m_cr3 & 0x10 ) /* enable the flag detect status in SR1 */
#define LOOP ( m_cr3 & 0x20 ) /* loop mode */
#define TST ( m_cr3 & 0x40 ) /* test mode (or go active on poll) */
#define DTR ( m_cr3 & 0x80 ) /* data-transmit-ready (or loop on-line control) */


/* control register 4 */

#define TWOINTER ( m_cr4 & 1 ) /* both an openning and a closing inter-frame are sent */

static const int word_length[4] = { 5, 6, 7, 8 }; /* transmit / receive word length */
#define TWL word_length[ ( m_cr4 >> 1 ) & 3 ]
#define RWL word_length[ ( m_cr4 >> 3 ) & 3 ]

#define ABT ( m_cr4 & 0x20 ) /* aborts */
#define ABTEX ( m_cr4 & 0x40 ) /* abort generates 16 '1' bits instead of 8 */
#define NRZI ( m_cr4 & 0x80 ) /* NRZI (zero complement) vs. NRZ data format */


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
#define DCD   0x20  /* data carrier detect */
#define OVRN  0x40  /* receiver overrun */
#define RDA2  0x80  /* copy of RDA */


/* FIFO entry markers */

#define FIFO_FULL  0x100
#define FIFO_LAST  0x200
#define FIFO_ADDR  0x400
#define FIFO_ERROR 0x800


static inline uint16_t crc_bit( uint16_t crc, int bit )
{
	/* CRC-CCITT x^16 + x^12 + x^5 + 1, least significant bit first */
	return ( ( crc ^ bit ) & 1 ) ? ( ( crc >> 1 ) ^ 0x8408 ) : ( crc >> 1 );
}


DEFINE_DEVICE_TYPE(MC6854, mc6854_device, "mc6854", "Motorola MC6854 ADLC")

mc6854_device::mc6854_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MC6854, tag, owner, clock),
	m_out_irq_cb(*this),
	m_out_rdsr_cb(*this),
	m_out_tdsr_cb(*this),
	m_out_txd_cb(*this),
	m_out_frame_cb(*this),
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
	m_tqueue(0),
	m_tqlen(0),
	m_tcrc(0),
	m_tabort(false),
	m_tclose(0),
	m_tflag(0),
	m_tfc(false),
	m_cts_stored(false),
	m_tflen(0),
	m_txc(false),
	m_txd(true),
	m_rts(false),
	m_rstate(0),
	m_rreg(0),
	m_rones(0),
	m_rsize(0),
	m_rcrc(0),
	m_rdelay(0),
	m_rdcount(0),
	m_rbits(0),
	m_rxd(true),
	m_rxd_last(true),
	m_rxc(false),
	m_flen(0),
	m_fpos(0)
{
	for (int i = 0; i < FIFO_SIZE; i++)
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
	m_out_frame_cb.resolve_safe();

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
	save_item(NAME(m_tqueue));
	save_item(NAME(m_tqlen));
	save_item(NAME(m_tcrc));
	save_item(NAME(m_tabort));
	save_item(NAME(m_tclose));
	save_item(NAME(m_tflag));
	save_item(NAME(m_tfc));
	save_item(NAME(m_cts_stored));
	save_item(NAME(m_tflen));
	save_item(NAME(m_txc));
	save_item(NAME(m_txd));
	save_item(NAME(m_rts));
	save_item(NAME(m_rstate));
	save_item(NAME(m_rreg));
	save_item(NAME(m_rones));
	save_item(NAME(m_rsize));
	save_item(NAME(m_rfifo));
	save_item(NAME(m_rcrc));
	save_item(NAME(m_rdelay));
	save_item(NAME(m_rdcount));
	save_item(NAME(m_rbits));
	save_item(NAME(m_rxd));
	save_item(NAME(m_rxd_last));
	save_item(NAME(m_rxc));
	save_item(NAME(m_frame));
	save_item(NAME(m_flen));
	save_item(NAME(m_fpos));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc6854_device::device_reset()
{
	LOG( "mc6854 reset\n" );
	m_cr1 = 0xc0; /* reset condition */
	m_cr2 = 0;
	m_cr3 = 0;
	m_cr4 = 0;
	m_sr1 = 0;
	m_sr2 = 0;
	m_cts_stored = false;
	tfifo_clear( );
	rfifo_clear( );
	update_sr1( );

	m_txd = true;
	m_out_txd_cb( 1 );
	m_rts = false;
	m_out_rts_cb( 0 );
	m_out_dtr_cb( 0 );
}

/*********************** transmit ***********************/



/* queue serial bits, least significant bit first */
void mc6854_device::queue_bits( uint32_t data, int len, bool zi )
{
	for ( int i = 0; i < len; i++, data >>= 1 )
	{
		m_tqueue |= uint64_t( data & 1 ) << m_tqlen++;

		if ( zi )
		{
			if ( data & 1 )
			{
				if ( ++m_tones == 5 )
				{
					/* insert a '0' after 5 consecutive '1' */
					m_tones = 0;
					m_tqlen++;
				}
			}
			else
				m_tones = 0;
		}
	}

	if ( ! zi )
		m_tones = 0;
}



void mc6854_device::queue_flag( )
{
	queue_bits( FLAG, 8, false );
}



void mc6854_device::queue_abort( )
{
	LOG( "%f mc6854 abort\n", machine().time().as_double() );
	queue_bits( 0xffff, ABTEX ? 16 : 8, false );
	m_tabort = false;
	m_tstate = 0;
	m_tflag = 0;
	m_tflen = 0;
	m_tclose = m_tqlen + 1;
}



/* MC6854 fills the transmitter shift register */
void mc6854_device::tx_refill( )
{
	if ( TRESET )
	{
		/* transmitter in reset: marks */
		queue_bits( 1, 1, false );
		m_tflag = 0;
		return;
	}

	if ( m_tabort )
	{
		queue_abort( );
		return;
	}

	if ( ! m_tstate )
	{
		if ( ! ( m_tfifo[ FIFO_SIZE - 1 ] & FIFO_FULL ) )
		{
			/* idle */
			if ( FMIDLE )
			{
				queue_flag( );
				m_tflag = 1;
			}
			else
			{
				queue_bits( 1, 1, false );
				m_tflag = 0;
			}
			return;
		}

		/* start frame */
		LOG( "%f mc6854 start frame\n", machine().time().as_double() );
		m_tstate = 2;
		m_tcrc = 0xffff;
		m_tflen = 0;

		if ( ! m_tflag || ( m_tflag == 2 && TWOINTER ) )
		{
			/* opening flag */
			queue_flag( );
			m_tflag = 1;
			return;
		}
	}

	/* in frame: take a field from the FIFO */
	int const data = m_tfifo[ FIFO_SIZE - 1 ];

	for ( int i = FIFO_SIZE - 1; i > 0; i-- )
		m_tfifo[ i ] = m_tfifo[ i - 1 ];
	m_tfifo[ 0 ] = 0;

	m_tflag = 0;

	if ( ! ( data & FIFO_FULL ) )
	{
		/* data underrun => abort */
		logerror( "%f mc6854 FIFO underrun\n", machine().time().as_double() );
		m_sr1 |= TU;
		queue_abort( );
		update_sr1( );
		return;
	}

	int blen = 8;

	switch ( m_tstate )
	{
	case 2: /* 8-bit address field */
		if ( ( data & 1 ) || ( ! AEX ) )
			m_tstate = 3;
		LOG( "%f mc6854 tx address field $%02X\n", machine().time().as_double(), data & 0xff );
		break;

	case 3: /* 8-bit control field */
		if ( CEX )
			m_tstate = 4;
		else if ( LCF )
			m_tstate = 5;
		else
			m_tstate = 6;
		LOG( "%f mc6854 tx control field $%02X\n", machine().time().as_double(), data & 0xff );
		break;

	case 4: /* 8-bit extended control field (optional) */
		if ( LCF )
			m_tstate = 5;
		else
			m_tstate = 6;
		LOG( "%f mc6854 tx control field $%02X\n", machine().time().as_double(), data & 0xff );
		break;

	case 5: /* 8-bit logical control (optional) */
		if ( ! ( data & 0x80 ) )
			m_tstate = 6;
		LOG( "%f mc6854 tx logical control field $%02X\n", machine().time().as_double(), data & 0xff );
		break;

	case 6: /* variable-length data */
		blen = TWL;
		LOG( "%f mc6854 tx data field $%02X, %i bits\n", machine().time().as_double(), data & 0xff, blen );
		break;
	}

	for ( int i = 0; i < blen; i++ )
		m_tcrc = crc_bit( m_tcrc, BIT( data, i ) );
	queue_bits( data, blen, true );

	if ( !m_out_frame_cb.isnull() )
	{
		if ( m_tflen < MAX_FRAME_LENGTH )
			m_frame[ m_tflen++ ] = data;
		else
			logerror( "mc6854 truncated frame, max=%i\n", MAX_FRAME_LENGTH );
	}

	if ( data & FIFO_LAST )
	{
		/* close frame */
		LOG( "%f mc6854 end frame\n", machine().time().as_double() );
		queue_bits( ~m_tcrc & 0xffff, 16, true );
		queue_flag( );
		m_tflag = 2;
		m_tstate = 0;
		m_tclose = m_tqlen + 1;

		if ( m_tflen > 0 )
		{
			int const len = m_tflen;
			m_tflen = 0;
			m_out_frame_cb( m_frame, len );
		}
	}

	update_sr1( );
}



void mc6854_device::update_rts( )
{
	bool const active = RTS || m_tstate || m_tclose;

	if ( active != m_rts )
	{
		m_rts = active;
		m_out_rts_cb( m_rts ? 1 : 0 );
	}
}



/* shift one bit out on TxD */
void mc6854_device::tx_bit( )
{
	if ( m_tclose && ! --m_tclose )
	{
		/* last bit cell of the closing flag or abort has ended */
		m_tfc = true;
		update_sr1( );
	}

	update_rts( );

	if ( ! m_tqlen )
		tx_refill( );

	int const bit = m_tqueue & 1;
	m_tqueue >>= 1;
	m_tqlen--;

	if ( ! NRZI )
		m_txd = bit;
	else if ( ! bit )
		m_txd = ! m_txd;

	m_out_txd_cb( m_txd ? 1 : 0 );

	if ( m_tqlen <= 1 )
		tx_refill( );
}



/* CPU pushes a field */
void mc6854_device::tfifo_push( uint8_t data )
{
	int i;

	if ( TRESET )
		return;

	/* push towards the rightmost free entry */
	for ( i = FIFO_SIZE - 1; i >= 0; i-- )
	{
		if ( ! ( m_tfifo[ i ] & FIFO_FULL ) )
			break;
	}

	if ( i >= 0 )
		m_tfifo[ i ] = data | FIFO_FULL;
	else
		logerror( "%f mc6854_tfifo_push: FIFO overrun\n", machine().time().as_double() );

	update_sr1( );
}



/* CPU asks for normal frame termination */
void mc6854_device::tfifo_terminate( )
{
	/* mark most recently pushed byte as the last one of the frame */
	for ( int i = 0; i < FIFO_SIZE; i++ )
	{
		if ( m_tfifo[ i ] & FIFO_FULL )
		{
			m_tfifo[ i ] |= FIFO_LAST;
			break;
		}
	}
}



void mc6854_device::tfifo_clear( )
{
	std::fill( std::begin( m_tfifo ), std::end( m_tfifo ), 0 );
	m_tstate = 0;
	m_tones = 0;
	m_tqueue = 0;
	m_tqlen = 0;
	m_tabort = false;
	m_tclose = 0;
	m_tflag = 0;
	m_tflen = 0;
}



void mc6854_device::txc_w(int state)
{
	if ( m_txc && ! state )
		tx_bit( );

	m_txc = state;
}



/*********************** receive ***********************/



/* MC6854 pushes a field in the FIFO */
void mc6854_device::rfifo_push( uint8_t d )
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
		LOG( "%f mc6854_rfifo_push: address field $%02X\n", machine().time().as_double(), data );
		data |= FIFO_ADDR;
		break;

	case 3: /* 8-bit control field */
		if ( CEX )
			m_rstate = 4;
		else if ( LCF )
			m_rstate = 5;
		else
			m_rstate = 6;
		LOG( "%f mc6854_rfifo_push: control field $%02X\n", machine().time().as_double(), data );
		break;

	case 4: /* 8-bit extended control field (optional) */
		if ( LCF )
			m_rstate = 5;
		else
			m_rstate = 6;
		LOG( "%f mc6854_rfifo_push: control field $%02X\n", machine().time().as_double(), data );
		break;

	case 5: /* 8-bit logical control (optional) */
		if ( ! ( data & 0x80 ) )
			m_rstate = 6;
		LOG( "%f mc6854_rfifo_push: logical control field $%02X\n", machine().time().as_double(), data );
		break;

	case 6: /* variable-length data */
		blen = RWL;
		data >>= 8 - blen;
		LOG( "%f mc6854_rfifo_push: data field $%02X, %i bits\n", machine().time().as_double(), data, blen );
		break;
	}

	m_rsize -= blen;

	/* no further FIFO fill until FV is cleared! */
	if ( m_sr2 & ( FV | ERR ) )
	{
		LOG( "%f mc6854_rfifo_push: field not pushed\n", machine().time().as_double() );
		return;
	}

	data |= FIFO_FULL;

	/* push towards the rightmost free entry */
	for ( i = FIFO_SIZE - 1; i >= 0; i-- )
	{
		if ( ! ( m_rfifo[ i ] & FIFO_FULL ) )
			break;
	}

	if ( i >= 0 )
		m_rfifo[ i ] = data;
	else
	{
		/* FIFO full */
		m_sr2 |= OVRN;
		m_rfifo[ 0 ] = data;
		logerror( "%f mc6854_rfifo_push: FIFO overrun\n", machine().time().as_double() );
	}

	update_sr1( );
}


void mc6854_device::rfifo_terminate( bool error )
{
	/* mark most recently pushed byte as the last one of the frame */
	for ( int i = 0; i < FIFO_SIZE; i++ )
	{
		if ( m_rfifo[ i ] & FIFO_FULL )
		{
			m_rfifo[ i ] |= FIFO_LAST | ( error ? FIFO_ERROR : 0 );
			break;
		}
	}

	m_rstate = 1;
	update_sr1( );
}


/* CPU pops the FIFO */
uint8_t mc6854_device::rfifo_pop( )
{
	int i, data = m_rfifo[ FIFO_SIZE - 1 ];

	/* shift FIFO to the right */
	for ( i = FIFO_SIZE - 1; i > 0; i -- )
		m_rfifo[ i ] = m_rfifo[ i - 1 ];
	m_rfifo[ 0 ] = 0;

	/* auto-refill in frame mode */
	if ( m_flen > 0 )
	{
		rfifo_push( m_frame[ m_fpos++ ] );
		if ( m_fpos == m_flen )
		{
			m_flen = 0;
			rfifo_terminate( false );
		}
	}

	update_sr1( );

	return data;
}


void mc6854_device::set_rx(int state)
{
	m_rxd = state;
}



void mc6854_device::rfifo_clear( )
{
	std::fill( std::begin( m_rfifo ), std::end( m_rfifo ), 0 );
	m_rstate = 0;
	m_rreg = 0;
	m_rsize = 0;
	m_rones = 0;
	m_rdcount = 0;
	m_rbits = 0;
	m_flen = 0;
}



int mc6854_device::send_frame( uint8_t* data, int len )
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
	{
		m_sr1 |= FD;
		update_sr1();
	}
	m_flen = len;
	m_fpos = 0;
	rfifo_push( m_frame[ m_fpos++ ] );
	rfifo_push( m_frame[ m_fpos++ ] );
	if ( m_fpos == m_flen )
	{
		m_flen = 0;
		rfifo_terminate( false );
	}
	return 0;
}



/* a bit enters the receiver shift register */
void mc6854_device::rx_shift( int bit )
{
	if ( ! m_rstate )
		return;

	m_rreg = ( m_rreg >> 1 ) | ( bit ? 0x80000000 : 0 );
	m_rsize++;

	/* the last 8 bits may be the closing flag, keep them out of the FCS */
	if ( m_rdcount == 8 )
	{
		m_rcrc = crc_bit( m_rcrc, m_rdelay & 1 );
		m_rbits++;
	}
	else
		m_rdcount++;
	m_rdelay = ( m_rdelay >> 1 ) | ( bit ? 0x80 : 0 );

	int const fieldlen = ( m_rstate < 6 ) ? 8 : RWL;
	if ( m_rsize >= fieldlen + 24 )
		rfifo_push( m_rreg );
}



void mc6854_device::rx_end_frame( )
{
	if ( m_rbits < 25 )
	{
		/* short frame, ignored */
		LOG( "%f mc6854 short frame ignored (%u bits)\n", machine().time().as_double(), m_rbits );
		return;
	}

	bool const error = ( m_rbits < 32 ) || ( m_rcrc != 0xf0b8 );
	LOG( "%f mc6854 end of frame (%u bits, FCS %s)\n", machine().time().as_double(), m_rbits, error ? "bad" : "good" );
	rfifo_terminate( error );
}



/* MC6854 makes fields from bits */
void mc6854_device::rx_bit( int bit )
{
	if ( bit )
	{
		LOGBITS("I ");
		if ( m_rones < 0xff )
			m_rones++;

		if ( m_rones == 7 )
		{
			/* abort */
			if ( m_rstate && ( m_rbits + m_rdcount >= 26 ) )
			{
				m_sr2 |= RABT;
				LOG( "%f mc6854 receive abort\n", machine().time().as_double() );
			}
			if ( m_rstate > 1 )
			{
				/* discard the aborted frame, keeping the end of the previous one */
				for ( int i = 0; ( i < FIFO_SIZE ) && ! ( m_rfifo[ i ] & FIFO_LAST ); i++ )
					m_rfifo[ i ] = 0;
			}
			m_rstate = 0;
			m_rsize = 0;
			update_sr1( );
		}
		else if ( m_rones == 15 )
		{
			/* idle */
			m_sr2 |= RIDLE;
			update_sr1( );
		}

		if ( m_rones < 7 )
			rx_shift( 1 );
	}
	else if ( m_rones == 5 )
	{
		/* discards '0' inserted after 5 '1' */
		LOGBITS("A zero is discarded\n");
		m_rones = 0;
	}
	else if ( m_rones == 6 )
	{
		/* flag */
		m_rones = 0;

		if ( FDSE )
			m_sr1 |= FD;

		if ( m_rstate )
		{
			rx_shift( 0 );
			rx_end_frame( );
		}

		m_rstate = 1;
		m_rreg = 0;
		m_rsize = 0;
		m_rcrc = 0xffff;
		m_rdelay = 0;
		m_rdcount = 0;
		m_rbits = 0;
		update_sr1( );
	}
	else
	{
		LOGBITS("O ");
		bool const idle = m_rones >= 15;
		m_rones = 0;
		rx_shift( 0 );
		if ( idle )
			update_sr1( );
	}
}



void mc6854_device::rxc_w(int state)
{
	if ( state && ! m_rxc )
	{
		int const level = m_rxd ? 1 : 0;
		int const bit = NRZI ? ( level == ( m_rxd_last ? 1 : 0 ) ) : level;
		m_rxd_last = m_rxd;

		/* the receiver is held reset while the DCD input or stored DCD is high */
		if ( ! RRESET && ! ( m_sr2 & DCD ) && ! m_dcd )
			rx_bit( bit );
	}

	m_rxc = state;
}



/************************** CPU interface ****************************/



void mc6854_device::set_cts(int state)
{
	/* the stored CTS condition is held reset by Tx reset */
	if ( ! m_cts && state && ! TRESET )
		m_cts_stored = true;
	m_cts = state;

	update_sr1();
}



void mc6854_device::set_dcd(int state)
{
	if ( ! m_dcd && state )
	{
		/* the stored DCD condition is held reset by Rx reset */
		if ( ! RRESET )
			m_sr2 |= DCD;

		/* partial reset */
		m_rstate = 0;
		m_rreg = 0;
		m_rsize = 0;
		m_rones = 0;
	}
	m_dcd = state;

	update_sr1();
}



void mc6854_device::update_sr2( )
{
	/* update RDA */
	m_sr2 |= RDA2;
	if ( ! ( m_rfifo[ FIFO_SIZE - 1 ] & FIFO_FULL ) )
		m_sr2 &= ~RDA2;
	else if ( TWOBYTES && ! ( m_rfifo[ FIFO_SIZE - 2 ] & FIFO_FULL ) )
		m_sr2 &= ~RDA2;

	/* update AP */
	if ( m_rfifo[ FIFO_SIZE - 1 ] & FIFO_ADDR )
		m_sr2 |= AP;
	else
		m_sr2 &= ~AP;

	/* frame boundary reached the last FIFO location */
	if ( m_rfifo[ FIFO_SIZE - 1 ] & FIFO_LAST )
		m_sr2 |= ( m_rfifo[ FIFO_SIZE - 1 ] & FIFO_ERROR ) ? ERR : FV;

}



void mc6854_device::update_sr1( )
{
	update_sr2( );

	/* update S2RQ */
	if ( m_sr2 & 0x7f )
		m_sr1 |= S2RQ;
	else
		m_sr1 &= ~S2RQ;

	/* update CTS */
	if ( m_cts_stored || m_cts )
		m_sr1 |= CTS;
	else
		m_sr1 &= ~CTS;

	/* update TDRA/FC (always inhibited by CTS) */
	m_sr1 &= ~TDRA;
	if ( FCTDRA )
	{
		if ( m_tfc )
			m_sr1 |= TDRA;
	}
	else if ( ! TRESET && ! m_cts )
	{
		if ( ! ( m_tfifo[ 0 ] & FIFO_FULL ) && ! ( TWOBYTES && ( m_tfifo[ 1 ] & FIFO_FULL ) ) )
			m_sr1 |= TDRA;
	}

	/* update RDA */
	if ( m_sr2 & RDA2 )
		m_sr1 |= RDA;
	else
		m_sr1 &= ~RDA;

	/* update IRQ */
	LOGIRQ("\nUpdate IRQ in: %d\n", (m_sr1 & IRQ) ? 1 : 0);
	m_sr1 &= ~IRQ;
	if ( TIE )
	{
		if ( m_sr1 & TU ) m_sr1 |= IRQ;
		if ( m_cts_stored ) m_sr1 |= IRQ;
		LOGIRQ(" - Update IRQ TU/CTS: %d\n", (m_sr1 & IRQ) ? 1 : 0);
		if ( ( m_sr1 & TDRA ) && !TDSR ) m_sr1 |= IRQ; // TDRA will not cause interrupt if in DMA mode
		LOGIRQ(" - Update IRQ TDRA: %d\n", (m_sr1 & IRQ) ? 1 : 0);
	}
	if ( RIE )
	{
		if ( m_sr1 & FD ) m_sr1 |= IRQ;
		if ( ( m_sr1 & RDA ) && !RDSR ) m_sr1 |= IRQ; // RDA will not cause interrupt if in DMA mode
		LOGIRQ(" - Update IRQ RDA(%d) && !RDSR(%d): %d\n", (m_sr1 & RDA) ? 1 : 0, RDSR ? 1 : 0, (m_sr1 & IRQ) ? 1 : 0);
		if ( m_sr2 & (ERR | FV | DCD | OVRN | RABT | RIDLE | AP) ) m_sr1 |= IRQ;
		LOGIRQ(" - Update IRQ SR2: %d\n", (m_sr1 & IRQ) ? 1 : 0);
	}

	m_out_irq_cb((m_sr1 & IRQ) ? ASSERT_LINE : CLEAR_LINE);
	m_out_rdsr_cb((m_sr1 & RDA) ? ASSERT_LINE : CLEAR_LINE);
	m_out_tdsr_cb((m_sr1 & TDRA) ? ASSERT_LINE : CLEAR_LINE);
}



uint8_t mc6854_device::read(offs_t offset)
{
	switch ( offset )
	{
	case 0: /* status register 1 */
		LOG( "%f %s mc6854_r: get SR1=$%02X (rda=%i,s2rq=%i,fd=%i,cts=%i,tu=%i,tdra=%i,irq=%i)\n",
				machine().time().as_double(), machine().describe_context(), m_sr1,
				( m_sr1 & RDA) ? 1 : 0, ( m_sr1 & S2RQ) ? 1 : 0,
				( m_sr1 & FD ) ? 1 : 0, ( m_sr1 & CTS ) ? 1 : 0,
				( m_sr1 & TU ) ? 1 : 0, ( m_sr1 & TDRA) ? 1 : 0,
				( m_sr1 & IRQ) ? 1 : 0 );
		return m_sr1;

	case 1: /* status register 2 */
	{
		/* DCD and Rx idle are the logical OR of the stored and present conditions */
		uint8_t sr2 = m_sr2;
		if ( m_dcd )
			sr2 |= DCD;
		if ( m_rones >= 15 )
			sr2 |= RIDLE;

		LOG( "%f %s mc6854_r: get SR2=$%02X (ap=%i,fv=%i,ridle=%i,rabt=%i,err=%i,dcd=%i,ovrn=%i,rda2=%i)\n",
				machine().time().as_double(), machine().describe_context(), sr2,
				( sr2 & AP   ) ? 1 : 0, ( sr2 & FV  ) ? 1 : 0,
				( sr2 & RIDLE) ? 1 : 0, ( sr2 & RABT) ? 1 : 0,
				( sr2 & ERR  ) ? 1 : 0, ( sr2 & DCD ) ? 1 : 0,
				( sr2 & OVRN ) ? 1 : 0, ( sr2 & RDA2) ? 1 : 0 );
		return sr2;
	}

	case 2: /* receiver data register */
	case 3:
	{
		if ( machine().side_effects_disabled() )
			return m_rfifo[ FIFO_SIZE - 1 ] & 0xff;

		uint8_t data = rfifo_pop( );
		LOG( "%f %s mc6854_r: get data $%02X\n",
				machine().time().as_double(), machine().describe_context(), data );
		return data;
	}

	default:
		logerror( "%s mc6854 invalid read offset %i\n", machine().describe_context(), offset );
	}
	return 0;
}



void mc6854_device::write(offs_t offset, uint8_t data)
{
	switch ( offset )
	{
	case 0: /* control register 1 */
		m_cr1 = data;
		LOGSETUP( "%f %s mc6854_w: set CR1=$%02X (ac=%i,irq=%c%c,%sreset=%c%c)\n",
				machine().time().as_double(), machine().describe_context(), m_cr1,
				AC ? 1 : 0,
				RIE ? 'r' : '-', TIE ? 't' : '-',
				DISCONTINUE ? "discontinue," : "",
				RRESET ? 'r' : '-', TRESET ? 't' : '-' );

		if ( DISCONTINUE )
		{
			/* abort receive FIFO but keeps shift register & synchro */
			m_rstate = 0;
			std::fill( std::begin( m_rfifo ), std::end( m_rfifo ), 0 );
		}
		if ( RRESET )
		{
			/* abort FIFO & synchro */
			rfifo_clear( );
			m_sr1 &= ~FD;
			m_sr2 &= ~(AP | FV | RIDLE | RABT | ERR | OVRN | DCD);
		}
		if ( TRESET )
		{
			tfifo_clear( );
			m_sr1 &= ~TU;
			m_tfc = false;
			m_cts_stored = false;
		}
		update_sr1( );
		break;

	case 1:
		if ( AC )
		{
			/* control register 3 */
			m_cr3 = data;
			LOGSETUP( "%f %s mc6854_w: set CR3=$%02X (lcf=%i,aex=%i,idl=%i,fdse=%i,loop=%i,tst=%i,dtr=%i)\n",
					machine().time().as_double(), machine().describe_context(), m_cr3,
					LCF ? (CEX ? 16 : 8) : 0,  AEX ? 1 : 0,
					IDL0 ? 0 : 1, FDSE ? 1 : 0, LOOP ? 1 : 0,
					TST ? 1 : 0, DTR ? 1 : 0 );
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
			LOGSETUP( "%f %s mc6854_w: set CR2=$%02X (pse=%i,bytes=%i,fmidle=%i,%s,tlast=%i,clr=%c%c,rts=%i)\n",
					machine().time().as_double(), machine().describe_context(), m_cr2,
					PSE ? 1 : 0,  TWOBYTES ? 2 : 1,  FMIDLE ? 1 : 0,
					FCTDRA ? "fc" : "tdra", TLAST ? 1 : 0,
					data & 0x20 ? 'r' : '-',  data & 0x40 ? 't' : '-',
					RTS ? 1 : 0 );
			if ( PSE )
				logerror( "%s mc6854 status prioritization not handled (CR2=$%02X)\n", machine().describe_context(), m_cr2 );
			if ( TLAST )
				tfifo_terminate( );
			if ( data & 0x20 )
			{
				/* clear receiver status */
				m_sr1 &= ~FD;
				m_sr2 &= ~(FV | RIDLE | RABT | ERR | OVRN | DCD);
			}
			if ( data & 0x40 )
			{
				/* clear transmitter status */
				m_sr1 &= ~TU;
				m_tfc = false;
				m_cts_stored = false;
			}
			update_sr1( );
			update_rts( );
		}
		break;

	case 2: /* transmitter data: continue data */
		LOGSETUP( "%f %smc6854_w: push data=$%02X\n", machine().time().as_double(), machine().describe_context(), data );
		tfifo_push( data );
		break;

	case 3:
		if ( AC )
		{
			/* control register 4 */
			m_cr4 = data;
			LOGSETUP( "%f %s mc6854_w: set CR4=$%02X (interframe=%i,tlen=%i,rlen=%i,%s%s)\n", machine().time().as_double(), machine().describe_context(), m_cr4,
					TWOINTER ? 2 : 1,
					TWL, RWL,
					ABT ? ( ABTEX ? "abort-ext," : "abort,") : "",
					NRZI ? "nrzi" : "nrz" );
			if ( ABT )
			{
				/* the Tx FIFO is cleared and an abort is sent */
				std::fill( std::begin( m_tfifo ), std::end( m_tfifo ), 0 );
				m_tabort = true;
				m_cr4 &= ~0x20;
				update_sr1( );
			}
		}
		else
		{
			/* transmitter data: last data */
			LOGSETUP( "%f %s mc6854_w: push last-data=$%02X\n", machine().time().as_double(), machine().describe_context(), data );
			tfifo_push( data );
			tfifo_terminate( );
		}
		break;

	default:
		logerror( "%s mc6854 invalid write offset %i (data=$%02X)\n", machine().describe_context(), offset, data );
	}
}
