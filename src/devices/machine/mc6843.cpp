// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2007

  Motorola 6843 Floppy Disk Controller emulation.

**********************************************************************/

/*
  Main MC 6843 features are:
   - single density floppies
   - IBM 3740 compatible
   - DMA-able
   - high-level commands (including multi-sector read/write)

   CLONES: HD 46503S seems to be a clone of MC 6843

   BUGS
   The driver was designed with Thomson computer emulation in mind
   (CD 90-015 5"1/4 floppy controller) and works in this context.
   It might work in other contexts but has currently shortcomings:
   - DMA is not emulated
   - Free-Format Read is not emulated
   - Free-Format Write only supports track formatting, in a specific
   format (FWF=1, Thomson-like sector formats)
   - very rough timing: basically, there is a fixed delay between
   a command request (CMR write) and its response (first byte
   available, seek complete, etc.); there is no delay between
   read / write
 */


#include "emu.h"
#include "mc6843.h"


/******************* parameters ******************/

#define VERBOSE 0

/* macro-command numbers */
#define CMD_STZ 0x2 /* seek track zero */
#define CMD_SEK 0x3 /* seek */
#define CMD_SSR 0x4 /* single sector read */
#define CMD_SSW 0x5 /* single sector write */
#define CMD_RCR 0x6 /* read CRC */
#define CMD_SWD 0x7 /* single sector write with delete data mark */
#define CMD_MSW 0xd /* multiple sector write */
#define CMD_MSR 0xc /* multiple sector read */
#define CMD_FFW 0xb /* free format write */
#define CMD_FFR 0xa /* free format read */

/* coarse delays */
#define DELAY_SEEK   attotime::from_usec( 100 )  /* track seek time */
#define DELAY_ADDR   attotime::from_usec( 100 )  /* search-address time */



static const char *const mc6843_cmd[16] =
{
	"---", "---", "STZ", "SEK", "SSR", "SSW", "RCR", "SWD",
	"---", "---", "FFR", "FFW", "MSR", "MSW", "---", "---",
};


/******************* utility function and macros ********************/

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)




const device_type MC6843 = &device_creator<mc6843_device>;

mc6843_device::mc6843_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MC6843, "MC6843 floppy controller", tag, owner, clock, "mc6843", __FILE__),
	m_write_irq(*this),
	m_CTAR(0),
	m_CMR(0),
	m_ISR(0),
	m_SUR(0),
	m_STRA(0),
	m_STRB(0),
	m_SAR(0),
	m_GCR(0),
	m_CCR(0),
	m_LTAR(0),
	m_drive(0),
	m_side(0),
	m_data_size(0),
	m_data_idx(0),
	m_data_id(0),
	m_index_pulse(0),
	m_timer_cont(nullptr)
{
	for (int i = 0; i < 128; i++)
	{
		m_data[i] = 0;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc6843_device::device_start()
{
	m_write_irq.resolve_safe();

	m_timer_cont = timer_alloc(TIMER_CONT);

	save_item(NAME(m_CTAR));
	save_item(NAME(m_CMR));
	save_item(NAME(m_ISR));
	save_item(NAME(m_SUR));
	save_item(NAME(m_STRA));
	save_item(NAME(m_STRB));
	save_item(NAME(m_SAR));
	save_item(NAME(m_GCR));
	save_item(NAME(m_CCR));
	save_item(NAME(m_LTAR));
	save_item(NAME(m_drive));
	save_item(NAME(m_side));
	save_item(NAME(m_data));
	save_item(NAME(m_data_size));
	save_item(NAME(m_data_idx));
	save_item(NAME(m_data_id));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc6843_device::device_reset()
{
	int i;
	LOG (( "mc6843 reset\n" ));

	/* setup/reset floppy drive */
	for ( i = 0; i < 4; i++ )
	{
		legacy_floppy_image_device * img = floppy_image( i );
		img->floppy_mon_w(CLEAR_LINE);
		img->floppy_drive_set_ready_state(FLOPPY_DRIVE_READY, 0 );
		img->floppy_drive_set_rpm( 300. );
	}

	/* reset registers */
	m_CMR &= 0xf0; /* zero only command */
	m_ISR = 0;
	m_STRA &= 0x5c;
	m_SAR = 0;
	m_STRB &= 0x20;
	status_update( );

	m_data_size = 0;
	m_data_idx = 0;
	m_timer_cont->adjust( attotime::never );
}

/************************** floppy interface ****************************/



legacy_floppy_image_device* mc6843_device::floppy_image( UINT8 drive )
{
	legacy_floppy_image_device *img = floppy_get_device( machine(), drive );
	if (!img && owner()) {
		// For slot devices, drives are typically attached to the slot rather than the machine
		const char *floppy_name = nullptr;
		switch (drive) {
		case 0:
			floppy_name = FLOPPY_0;
			break;
		case 1:
			floppy_name = FLOPPY_1;
			break;
		case 2:
			floppy_name = FLOPPY_2;
			break;
		case 3:
			floppy_name = FLOPPY_3;
			break;
		}
		img = owner()->subdevice<legacy_floppy_image_device>(floppy_name);
	}
	return img;
}


legacy_floppy_image_device* mc6843_device::floppy_image( )
{
	return floppy_image( m_drive );
}


void mc6843_device::set_drive( int drive )
{
	m_drive = drive;
}



void mc6843_device::set_side( int side )
{
	m_side = side;
}



/* called after ISR or STRB has changed */
void mc6843_device::status_update( )
{
	int irq = 0;

	/* ISR3 */
	if ( (m_CMR & 0x40) || ! m_STRB )
		m_ISR &= ~8;
	else
		m_ISR |=  8;

	/* interrupts */
	if ( m_ISR & 4 )
		irq = 1; /* unmaskable */
	if ( ! (m_CMR & 0x80) )
	{
		/* maskable */
		if ( m_ISR & ~4 )
			irq = 1;
	}

	m_write_irq( irq );
	LOG(( "status_update: irq=%i (CMR=%02X, ISR=%02X)\n", irq, m_CMR, m_ISR ));
}


void mc6843_device::set_index_pulse( int index_pulse )
{
	m_index_pulse = index_pulse;
}


/* called at end of command */
void mc6843_device::cmd_end( )
{
	int cmd = m_CMR & 0x0f;
	if ( ( cmd == CMD_STZ ) || ( cmd == CMD_SEK ) )
	{
		m_ISR |= 0x02; /* set Settling Time Complete */
	}
	else
	{
		m_ISR |= 0x01;  /* set Macro Command Complete */
	}
	m_STRA &= ~0x80; /* clear Busy */
	m_CMR  &=  0xf0; /* clear command */
	status_update( );
}



/* Seek Track Zero bottom half */
void mc6843_device::finish_STZ( )
{
	legacy_floppy_image_device* img = floppy_image( );
	int i;

	/* seek to track zero */
	for ( i=0; i<83; i++ )
	{
		if (img->floppy_tk00_r() == CLEAR_LINE)
			break;
		img->floppy_drive_seek( -1 );
	}

	LOG(( "%f mc6843_finish_STZ: actual=%i\n", machine().time().as_double(), img->floppy_drive_get_current_track() ));

	/* update state */
	m_CTAR = 0;
	m_GCR = 0;
	m_SAR = 0;
	m_STRB |= img->floppy_tk00_r() << 4;

	cmd_end( );
}



/* Seek bottom half */
void mc6843_device::finish_SEK( )
{
	legacy_floppy_image_device* img = floppy_image( );

	/* seek to track */
	// TODO: not sure how CTAR bit 7 is handled here, but this is the safest approach for now
	img->floppy_drive_seek( m_GCR - (m_CTAR & 0x7F) );

	LOG(( "%f mc6843_finish_SEK: from %i to %i (actual=%i)\n", machine().time().as_double(), (m_CTAR & 0x7F), m_GCR, img->floppy_drive_get_current_track() ));

	/* update state */
	m_CTAR = m_GCR;
	m_SAR = 0;
	cmd_end( );
}



/* preamble to all sector read / write commands, returns 1 if found */
int mc6843_device::address_search( chrn_id* id )
{
	legacy_floppy_image_device* img = floppy_image( );
	int r = 0;

	while ( 1 )
	{
		if ( ( ! img->floppy_drive_get_next_id( m_side, id ) ) || ( id->flags & ID_FLAG_CRC_ERROR_IN_ID_FIELD ) || ( id->N != 0 ) )
		{
			/* read address error */
			LOG(( "%f mc6843_address_search: get_next_id failed\n", machine().time().as_double() ));
			m_STRB |= 0x0a; /* set CRC error & Sector Address Undetected */
			cmd_end( );
			return 0;
		}

		if ( id->C != m_LTAR )
		{
			/* track mismatch */
			LOG(( "%f mc6843_address_search: track mismatch: logical=%i real=%i\n", machine().time().as_double(), m_LTAR, id->C ));
			m_data[0] = id->C; /* make the track number available to the CPU */
			m_STRA |= 0x20;    /* set Track Not Equal */
			cmd_end( );
			return 0;
		}

		if ( id->R == m_SAR )
		{
			/* found! */
			LOG(( "%f mc6843_address_search: sector %i found on track %i\n", machine().time().as_double(), id->R, id->C ));
			if ( ! (m_CMR & 0x20) )
			{
				m_ISR |= 0x04; /* if no DMA, set Status Sense */
			}
			return 1;
		}

		if ( img->floppy_drive_get_flag_state( FLOPPY_DRIVE_INDEX ) )
		{
			r++;
			if ( r >= 4 )
			{
				/* time-out after 3 full revolutions */
				LOG(( "%f mc6843_address_search: no sector %i found after 3 revolutions\n", machine().time().as_double(), m_SAR ));
				m_STRB |= 0x08; /* set Sector Address Undetected */
				cmd_end( );
				return 0;
			}
		}
	}

	//return 0; /* unreachable */
}



/* preamble specific to read commands (adds extra checks) */
int mc6843_device::address_search_read( chrn_id* id )
{
	if ( ! address_search( id ) )
		return 0;

	if ( id->flags & ID_FLAG_CRC_ERROR_IN_DATA_FIELD )
	{
		LOG(( "%f mc6843_address_search_read: data CRC error\n", machine().time().as_double() ));
		m_STRB |= 0x06; /* set CRC error & Data Mark Undetected */
		cmd_end( );
		return 0;
	}

	if ( id->flags & ID_FLAG_DELETED_DATA )
	{
		LOG(( "%f mc6843_address_search_read: deleted data\n", machine().time().as_double() ));
		m_STRA |= 0x02; /* set Delete Data Mark Detected */
	}

	return 1;
}




/* Read CRC bottom half */
void mc6843_device::finish_RCR( )
{
	chrn_id id;
	if ( ! address_search_read( &id ) )
		return;
	cmd_end( );
}



/* Single / Multiple Sector Read bottom half */
void mc6843_device::cont_SR( )
{
	chrn_id id;
	legacy_floppy_image_device* img = floppy_image( );

	/* sector seek */
	if ( ! address_search_read( &id ) )
		return;

	/* sector read */
	img->floppy_drive_read_sector_data( m_side, id.data_id, m_data, 128 );
	m_data_idx = 0;
	m_data_size = 128;
	m_STRA |= 0x01;     /* set Data Transfer Request */
	status_update( );
}



/* Single / Multiple Sector Write bottom half */
void mc6843_device::cont_SW( )
{
	chrn_id id;

	/* sector seek */
	if ( ! address_search( &id ) )
		return;

	/* setup sector write buffer */
	m_data_idx = 0;
	m_data_size = 128;
	m_STRA |= 0x01;         /* set Data Transfer Request */
	m_data_id = id.data_id; /* for subsequent write sector command */
	status_update( );
}



/* bottom halves, called to continue / finish a command after some delay */
void mc6843_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_CONT:
			{
				int cmd = m_CMR & 0x0f;

				LOG(( "%f mc6843_cont: timer called for cmd=%s(%i)\n", machine().time().as_double(), mc6843_cmd[cmd], cmd ));

				m_timer_cont->adjust( attotime::never );

				switch ( cmd )
				{
					case CMD_STZ: finish_STZ( ); break;
					case CMD_SEK: finish_SEK( ); break;
					case CMD_SSR: cont_SR( );    break;
					case CMD_SSW: cont_SW( );    break;
					case CMD_RCR: finish_RCR( ); break;
					case CMD_SWD: cont_SW( );    break;
					case CMD_MSW: cont_SW( );    break;
					case CMD_MSR: cont_SR( );    break;
				}
			}
			break;

		default:
			break;
	}
}



/************************** CPU interface ****************************/



READ8_MEMBER( mc6843_device::read )
{
	UINT8 data = 0;

	switch ( offset ) {
	case 0: /* Data Input Register (DIR) */
	{
		int cmd = m_CMR & 0x0f;

		LOG(( "%f %s mc6843_r: data input cmd=%s(%i), pos=%i/%i, GCR=%i, ",
				machine().time().as_double(), machine().describe_context(),
				mc6843_cmd[cmd], cmd, m_data_idx,
				m_data_size, m_GCR ));

		if ( cmd == CMD_SSR || cmd == CMD_MSR )
		{
			/* sector read */
			assert( m_data_size > 0 );
			assert( m_data_idx < m_data_size );
			assert( m_data_idx < sizeof(m_data) );
			data = m_data[ m_data_idx ];
			m_data_idx++;

			if ( m_data_idx >= m_data_size )
			{
				/* end of sector read */

				m_STRA &= ~0x01; /* clear Data Transfer Request */

				if ( cmd == CMD_MSR )
				{
					/* schedule next sector in multiple sector read */
					m_GCR--;
					m_SAR++;
					if ( m_GCR == 0xff )
					{
						cmd_end( );
					}
					else if ( m_SAR > 26 )

					{
						m_STRB |= 0x08; /* set Sector Address Undetected */
						cmd_end( );
					}
					else
					{
						m_timer_cont->adjust( DELAY_ADDR );
					}
				}
				else
				{
					cmd_end( );
				}
			}
		}
		else if ( cmd == 0 )
		{
			data = m_data[0];
		}
		else
		{
			/* XXX TODO: other read modes */
			data = m_data[0];
			logerror( "%s mc6843 read in unsupported command mode %i\n", machine().describe_context(), cmd );
		}

		LOG(( "data=%02X\n", data ));

		break;
	}

	case 1: /* Current-Track Address Register (CTAR) */
		data = m_CTAR;
		LOG(( "%f %s mc6843_r: read CTAR %i (actual=%i)\n",
				machine().time().as_double(), machine().describe_context(), data,
				floppy_image()->floppy_drive_get_current_track()));
		break;

	case 2: /* Interrupt Status Register (ISR) */
		data = m_ISR;
		LOG(( "%f %s mc6843_r: read ISR %02X: cmd=%scomplete settle=%scomplete sense-rq=%i STRB=%i\n",
				machine().time().as_double(), machine().describe_context(), data,
				(data & 1) ? "" : "not-" , (data & 2) ? "" : "not-",
				(data >> 2) & 1, (data >> 3) & 1 ));

		/* reset */
		m_ISR &= 8; /* keep STRB */
		status_update( );
		break;

	case 3: /* Status Register A (STRA) */
	{
		/* update */
		legacy_floppy_image_device* img = floppy_image( );
		int flag = img->floppy_drive_get_flag_state( FLOPPY_DRIVE_READY);
		m_STRA &= 0xa3;
		if ( flag & FLOPPY_DRIVE_READY )
			m_STRA |= 0x04;

		m_STRA |= !img->floppy_tk00_r() << 3;
		m_STRA |= !img->floppy_wpt_r() << 4;

		if ( m_index_pulse )
			m_STRA |= 0x40;

		data = m_STRA;
		LOG(( "%f %s mc6843_r: read STRA %02X: data-rq=%i del-dta=%i ready=%i t0=%i wp=%i trk-dif=%i idx=%i busy=%i\n",
				machine().time().as_double(), machine().describe_context(), data,
				data & 1, (data >> 1) & 1, (data >> 2) & 1, (data >> 3) & 1,
				(data >> 4) & 1, (data >> 5) & 1, (data >> 6) & 1, (data >> 7) & 1 ));
		break;
	}

	case 4: /* Status Register B (STRB) */
		data = m_STRB;
		LOG(( "%f %s mc6843_r: read STRB %02X: data-err=%i CRC-err=%i dta--mrk-err=%i sect-mrk-err=%i seek-err=%i fi=%i wr-err=%i hard-err=%i\n",
				machine().time().as_double(), machine().describe_context(), data,
				data & 1, (data >> 1) & 1, (data >> 2) & 1, (data >> 3) & 1,
				(data >> 4) & 1, (data >> 5) & 1, (data >> 6) & 1, (data >> 7) & 1 ));

		/* (partial) reset */
		m_STRB &= ~0xfb;
		status_update( );
		break;

	case 7: /* Logical-Track Address Register (LTAR) */
		data = m_LTAR;
		LOG(( "%f %s mc6843_r: read LTAR %i (actual=%i)\n",
				machine().time().as_double(), machine().describe_context(), data,
				floppy_image()->floppy_drive_get_current_track()));
		break;

	default:
		logerror( "%s mc6843 invalid read offset %i\n", machine().describe_context(), offset );
	}

	return data;
}

WRITE8_MEMBER( mc6843_device::write )
{
	switch ( offset ) {
	case 0: /* Data Output Register (DOR) */
	{
		int cmd = m_CMR & 0x0f;
		int FWF = (m_CMR >> 4) & 1;

		LOG(( "%f %s mc6843_w: data output cmd=%s(%i), pos=%i/%i, GCR=%i, data=%02X\n",
				machine().time().as_double(), machine().describe_context(),
				mc6843_cmd[cmd], cmd, m_data_idx,
				m_data_size, m_GCR, data ));

		if ( cmd == CMD_SSW || cmd == CMD_MSW || cmd == CMD_SWD )
		{
			/* sector write */
			assert( m_data_size > 0 );
			assert( m_data_idx < m_data_size );
			assert( m_data_idx < sizeof(m_data) );
			m_data[ m_data_idx ] = data;
			m_data_idx++;
			if ( m_data_idx >= m_data_size )
			{
				/* end of sector write */
				legacy_floppy_image_device* img = floppy_image( );

				LOG(( "%f %s mc6843_w: write sector %i\n", machine().time().as_double(), machine().describe_context(), m_data_id ));

				img->floppy_drive_write_sector_data(
					m_side, m_data_id,
					m_data, m_data_size,
					(cmd == CMD_SWD) ? ID_FLAG_DELETED_DATA : 0 );

				m_STRA &= ~0x01; /* clear Data Transfer Request */

				if ( cmd == CMD_MSW )
				{
					m_GCR--;
					m_SAR++;
					if ( m_GCR == 0xff )
					{
						cmd_end( );
					}
					else if ( m_SAR > 26 )

					{
						m_STRB |= 0x08; /* set Sector Address Undetected */
						cmd_end( );
					}
					else
					{
						m_timer_cont->adjust( DELAY_ADDR );
					}
				}
				else
				{
					cmd_end( );
				}
			}
		}
		else if ( (cmd == CMD_FFW) && FWF )
		{
			/* assume we are formatting */
			UINT8 nibble;
			nibble =
				(data & 0x01) |
				((data & 0x04) >> 1 )|
				((data & 0x10) >> 2 )|
				((data & 0x40) >> 3 );

			assert( m_data_idx < sizeof(m_data) );

			m_data[m_data_idx / 2] =
				(m_data[m_data_idx / 2] << 4) | nibble;

			if ( (m_data_idx == 0) && (m_data[0] == 0xfe ) )
			{
				/* address mark detected */
				m_data_idx = 2;
			}
			else if ( m_data_idx == 9 )
			{
				/* address id field complete */
				if ( (m_data[2] == 0) && (m_data[4] == 0) )
				{
					/* valid address id field */
					legacy_floppy_image_device* img = floppy_image( );
					UINT8 track  = m_data[1];
					UINT8 sector = m_data[3];
					UINT8 filler = 0xe5; /* standard Thomson filler */
					LOG(( "%f %s mc6843_w: address id detected track=%i sector=%i\n", machine().time().as_double(), machine().describe_context(), track, sector));
					img->floppy_drive_format_sector( m_side, sector, track, 0, sector, 0, filler );
				}
				else
				{
					/* abort */
					m_data_idx = 0;
				}
			}
			else if ( m_data_idx > 0 )
			{
				/* accumulate address id field */
				m_data_idx++;
			}
		}
		else if ( cmd == 0 )
		{
			/* nothing */
		}
		else
		{
			/* XXX TODO: other write modes */
			logerror( "%s mc6843 write %02X in unsupported command mode %i (FWF=%i)\n", machine().describe_context(), data, cmd, FWF );
		}
		break;
	}

	case 1: /* Current-Track Address Register (CTAR) */
		m_CTAR = data;
		LOG(( "%f %s mc6843_w: set CTAR to %i %02X (actual=%i) \n",
				machine().time().as_double(), machine().describe_context(), m_CTAR, data,
				floppy_image()->floppy_drive_get_current_track()));
		break;

	case 2: /* Command Register (CMR) */
	{
		int cmd = data & 15;

		LOG(( "%f %s mc6843_w: set CMR to $%02X: cmd=%s(%i) FWF=%i DMA=%i ISR3-intr=%i fun-intr=%i\n",
				machine().time().as_double(), machine().describe_context(),
				data, mc6843_cmd[cmd], cmd, (data >> 4) & 1, (data >> 5) & 1,
				(data >> 6) & 1, (data >> 7) & 1 ));

		/* sanitize state */
		m_STRA &= ~0x81; /* clear Busy & Data Transfer Request */
		m_data_idx = 0;
		m_data_size = 0;

		/* commands are initiated by updating some flags and scheduling
		   a bottom-half (mc6843_cont) after some delay */

		switch (cmd)
		{
		case CMD_SSW:
		case CMD_SSR:
		case CMD_SWD:
		case CMD_RCR:
		case CMD_MSR:
		case CMD_MSW:
			m_STRA |=  0x80; /* set Busy */
			m_STRA &= ~0x22; /* clear Track Not Equal & Delete Data Mark Detected */
			m_STRB &= ~0x04; /* clear Data Mark Undetected */
			m_timer_cont->adjust( DELAY_ADDR );
			break;
		case CMD_STZ:
		case CMD_SEK:
			m_STRA |= 0x80; /* set Busy */
			m_timer_cont->adjust( DELAY_SEEK );
			break;
		case CMD_FFW:
		case CMD_FFR:
			m_data_idx = 0;
			m_STRA |= 0x01; /* set Data Transfer Request */
			break;
		}

		m_CMR = data;
		status_update( );
		break;
	}

	case 3: /* Set-Up Register (SUR) */
		m_SUR = data;

		/* assume CLK freq = 1MHz (IBM 3740 compatibility) */
		LOG(( "%f %s mc6843_w: set SUR to $%02X: head settling time=%fms, track-to-track seek time=%f\n",
				machine().time().as_double(), machine().describe_context(),
				data, 4.096 * (data & 15), 1.024 * ((data >> 4) & 15) ));
		break;

	case 4: /* Sector Address Register (SAR) */
		m_SAR = data & 0x1f;
		LOG(( "%f %s mc6843_w: set SAR to %i (%02X)\n", machine().time().as_double(), machine().describe_context(), m_SAR, data ));
		break;

	case 5: /* General Count Register (GCR) */
		m_GCR = data & 0x7f;
		LOG(( "%f %s mc6843_w: set GCR to %i (%02X)\n", machine().time().as_double(), machine().describe_context(), m_GCR, data ));
		break;

	case 6: /* CRC Control Register (CCR) */
		m_CCR = data & 3;
		LOG(( "%f %s mc6843_w: set CCR to %02X: CRC=%s shift=%i\n",
				machine().time().as_double(), machine().describe_context(), data,
				(data & 1) ? "enabled" : "disabled", (data >> 1) & 1 ));
		break;

	case 7: /* Logical-Track Address Register (LTAR) */
		m_LTAR = data & 0x7f;
		LOG(( "%f %s mc6843_w: set LTAR to %i %02X (actual=%i)\n",
				machine().time().as_double(), machine().describe_context(), m_LTAR, data,
				floppy_image()->floppy_drive_get_current_track()));
		break;

	default:
		logerror( "%s mc6843 invalid write offset %i (data=$%02X)\n", machine().describe_context(), offset, data );
	}
}
