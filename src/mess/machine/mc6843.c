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
#include "imagedev/flopdrv.h"


/******************* parameters ******************/

#define VERBOSE 0


/******************* internal chip data structure ******************/

struct mc6843_t
{

	/* interface */
	const mc6843_interface* iface;

	/* registers */
	UINT8 CTAR;       /* current track */
	UINT8 CMR;        /* command */
	UINT8 ISR;        /* interrupt status */
	UINT8 SUR;        /* set-up */
	UINT8 STRA;       /* status */
	UINT8 STRB;       /* status */
	UINT8 SAR;        /* sector address */
	UINT8 GCR;        /* general count */
	UINT8 CCR;        /* CRC control */
	UINT8 LTAR;       /* logical address track (=track destination) */

	/* internal state */
	UINT8  drive;
	UINT8  side;
	UINT8  data[128];   /* sector buffer */
	UINT32 data_size;   /* size of data */
	UINT32 data_idx;    /* current read/write position in data */
	UINT32 data_id;     /* chrd_id for sector write */
	UINT8  index_pulse;

	/* trigger delayed actions (bottom halves) */
	emu_timer* timer_cont;

};



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



INLINE mc6843_t* get_safe_token( device_t *device )
{
	assert( device != NULL );
	assert( device->type() == MC6843 );
	return (mc6843_t*) downcast<mc6843_device *>(device)->token();
}


/************************** floppy interface ****************************/



static device_t* mc6843_floppy_image ( device_t *device )
{
	mc6843_t* mc6843 = get_safe_token( device );
	return floppy_get_device( device->machine(), mc6843->drive );
}



void mc6843_set_drive( device_t *device, int drive )
{
	mc6843_t* mc6843 = get_safe_token( device );
	mc6843->drive = drive;
}



void mc6843_set_side( device_t *device, int side )
{
	mc6843_t* mc6843 = get_safe_token( device );
	mc6843->side = side;
}



/* called after ISR or STRB has changed */
static void mc6843_status_update( device_t *device )
{
	mc6843_t* mc6843 = get_safe_token( device );
	int irq = 0;

	/* ISR3 */
	if ( (mc6843->CMR & 0x40) || ! mc6843->STRB )
		mc6843->ISR &= ~8;
	else
		mc6843->ISR |=  8;

	/* interrupts */
	if ( mc6843->ISR & 4 )
		irq = 1; /* unmaskable */
	if ( ! (mc6843->CMR & 0x80) )
	{
		/* maskable */
		if ( mc6843->ISR & ~4 )
			irq = 1;
	}

	if ( mc6843->iface->irq_func )
	{
	  mc6843->iface->irq_func( device, irq );
		LOG(( "mc6843_status_update: irq=%i (CMR=%02X, ISR=%02X)\n", irq, mc6843->CMR, mc6843->ISR ));
	}
}


void mc6843_set_index_pulse  ( device_t *device, int index_pulse )
{
	mc6843_t* mc6843 = get_safe_token( device );
	mc6843->index_pulse = index_pulse;
}


/* called at end of command */
static void mc6843_cmd_end( device_t *device )
{
	mc6843_t* mc6843 = get_safe_token( device );
	int cmd = mc6843->CMR & 0x0f;
	if ( ( cmd == CMD_STZ ) || ( cmd == CMD_SEK ) )
	{
		mc6843->ISR |= 0x02; /* set Settling Time Complete */
	}
	else
	{
		mc6843->ISR |= 0x01;  /* set Macro Command Complete */
	}
	mc6843->STRA &= ~0x80; /* clear Busy */
	mc6843->CMR  &=  0xf0; /* clear command */
	mc6843_status_update( device );
}



/* Seek Track Zero bottom half */
static void mc6843_finish_STZ( device_t *device )
{
	mc6843_t* mc6843 = get_safe_token( device );
	device_t* img = mc6843_floppy_image( device );
	int i;

	/* seek to track zero */
	for ( i=0; i<83; i++ )
	{
		if (floppy_tk00_r(img) == CLEAR_LINE)
			break;
		floppy_drive_seek( img, -1 );
	}

	LOG(( "%f mc6843_finish_STZ: actual=%i\n", device->machine().time().as_double(), floppy_drive_get_current_track( img ) ));

	/* update state */
	mc6843->CTAR = 0;
	mc6843->GCR = 0;
	mc6843->SAR = 0;
	mc6843->STRB |= floppy_tk00_r(img) << 4;

	mc6843_cmd_end( device );
}



/* Seek bottom half */
static void mc6843_finish_SEK( device_t *device )
{
	mc6843_t* mc6843 = get_safe_token( device );
	device_t* img = mc6843_floppy_image( device );

	/* seek to track */
	floppy_drive_seek( img, mc6843->GCR - mc6843->CTAR );

	LOG(( "%f mc6843_finish_SEK: from %i to %i (actual=%i)\n", device->machine().time().as_double(), mc6843->CTAR, mc6843->GCR, floppy_drive_get_current_track( img ) ));

	/* update state */
	mc6843->CTAR = mc6843->GCR;
	mc6843->SAR = 0;
	mc6843_cmd_end( device );
}



/* preamble to all sector read / write commands, returns 1 if found */
static int mc6843_address_search( device_t *device, chrn_id* id )
{
	mc6843_t* mc6843 = get_safe_token( device );
	device_t* img = mc6843_floppy_image( device );
	int r = 0;

	while ( 1 )
	{

		if ( ( ! floppy_drive_get_next_id( img, mc6843->side, id ) ) || ( id->flags & ID_FLAG_CRC_ERROR_IN_ID_FIELD ) || ( id->N != 0 ) )
		{
			/* read address error */
			LOG(( "%f mc6843_address_search: get_next_id failed\n", device->machine().time().as_double() ));
			mc6843->STRB |= 0x0a; /* set CRC error & Sector Address Undetected */
			mc6843_cmd_end( device );
			return 0;
		}

		if ( id->C != mc6843->LTAR )
		{
			/* track mismatch */
			LOG(( "%f mc6843_address_search: track mismatch: logical=%i real=%i\n", device->machine().time().as_double(), mc6843->LTAR, id->C ));
			mc6843->data[0] = id->C; /* make the track number available to the CPU */
			mc6843->STRA |= 0x20;    /* set Track Not Equal */
			mc6843_cmd_end( device );
			return 0;
		}

		if ( id->R == mc6843->SAR )
		{
			/* found! */
			LOG(( "%f mc6843_address_search: sector %i found on track %i\n", device->machine().time().as_double(), id->R, id->C ));
			if ( ! (mc6843->CMR & 0x20) )
			{
				mc6843->ISR |= 0x04; /* if no DMA, set Status Sense */
			}
			return 1;
		}

		if ( floppy_drive_get_flag_state( img, FLOPPY_DRIVE_INDEX ) )
		{
			r++;
			if ( r >= 4 )
			{
				/* time-out after 3 full revolutions */
				LOG(( "%f mc6843_address_search: no sector %i found after 3 revolutions\n", device->machine().time().as_double(), mc6843->SAR ));
				mc6843->STRB |= 0x08; /* set Sector Address Undetected */
				mc6843_cmd_end( device );
				return 0;
			}
		}
	}

	return 0; /* unreachable */
}



/* preamble specific to read commands (adds extra checks) */
static int mc6843_address_search_read( device_t *device, chrn_id* id )
{
	mc6843_t* mc6843 = get_safe_token( device );
	if ( ! mc6843_address_search( device, id ) )
		return 0;

	if ( id->flags & ID_FLAG_CRC_ERROR_IN_DATA_FIELD )
	{
		LOG(( "%f mc6843_address_search_read: data CRC error\n", device->machine().time().as_double() ));
		mc6843->STRB |= 0x06; /* set CRC error & Data Mark Undetected */
		mc6843_cmd_end( device );
		return 0;
	}

	if ( id->flags & ID_FLAG_DELETED_DATA )
	{
		LOG(( "%f mc6843_address_search_read: deleted data\n", device->machine().time().as_double() ));
		mc6843->STRA |= 0x02; /* set Delete Data Mark Detected */
	}

	return 1;
}




/* Read CRC bottom half */
static void mc6843_finish_RCR( device_t *device )
{
	chrn_id id;
	if ( ! mc6843_address_search_read( device, &id ) )
		return;
	mc6843_cmd_end( device );
}



/* Single / Multiple Sector Read bottom half */
static void mc6843_cont_SR( device_t *device )
{
	mc6843_t* mc6843 = get_safe_token( device );
	chrn_id id;
	device_t* img = mc6843_floppy_image( device );

	/* sector seek */
	if ( ! mc6843_address_search_read( device, &id ) )
		return;

	/* sector read */
	floppy_drive_read_sector_data( img, mc6843->side, id.data_id, mc6843->data, 128 );
	mc6843->data_idx = 0;
	mc6843->data_size = 128;
	mc6843->STRA |= 0x01;     /* set Data Transfer Request */
	mc6843_status_update( device );
}



/* Single / Multiple Sector Write bottom half */
static void mc6843_cont_SW( device_t *device )
{
	mc6843_t* mc6843 = get_safe_token( device );
	chrn_id id;

	/* sector seek */
	if ( ! mc6843_address_search( device, &id ) )
		return;

	/* setup sector write buffer */
	mc6843->data_idx = 0;
	mc6843->data_size = 128;
	mc6843->STRA |= 0x01;         /* set Data Transfer Request */
	mc6843->data_id = id.data_id; /* for subsequent write sector command */
	mc6843_status_update( device );
}



/* bottom halves, called to continue / finish a command after some delay */
static TIMER_CALLBACK( mc6843_cont )
{
	device_t* device = (device_t*) ptr;
	mc6843_t* mc6843 = get_safe_token( device );
	int cmd = mc6843->CMR & 0x0f;

	LOG(( "%f mc6843_cont: timer called for cmd=%s(%i)\n", device->machine().time().as_double(), mc6843_cmd[cmd], cmd ));

	mc6843->timer_cont->adjust( attotime::never );

	switch ( cmd )
	{
	case CMD_STZ: mc6843_finish_STZ( device ); break;
	case CMD_SEK: mc6843_finish_SEK( device ); break;
	case CMD_SSR: mc6843_cont_SR( device );    break;
	case CMD_SSW: mc6843_cont_SW( device );    break;
	case CMD_RCR: mc6843_finish_RCR( device ); break;
	case CMD_SWD: mc6843_cont_SW( device );    break;
	case CMD_MSW: mc6843_cont_SW( device );    break;
	case CMD_MSR: mc6843_cont_SR( device );    break;
	}
}



/************************** CPU interface ****************************/



READ8_DEVICE_HANDLER ( mc6843_r )
{
	mc6843_t* mc6843 = get_safe_token( device );
	UINT8 data = 0;

	switch ( offset ) {

	case 0: /* Data Input Register (DIR) */
	{
		int cmd = mc6843->CMR & 0x0f;

		LOG(( "%f $%04x mc6843_r: data input cmd=%s(%i), pos=%i/%i, GCR=%i, ",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ),
		      mc6843_cmd[cmd], cmd, mc6843->data_idx,
		      mc6843->data_size, mc6843->GCR ));

		if ( cmd == CMD_SSR || cmd == CMD_MSR )
		{
			/* sector read */
			assert( mc6843->data_size > 0 );
			assert( mc6843->data_idx < mc6843->data_size );
			assert( mc6843->data_idx < sizeof(mc6843->data) );
			data = mc6843->data[ mc6843->data_idx ];
			mc6843->data_idx++;

			if ( mc6843->data_idx >= mc6843->data_size )
			{
				/* end of sector read */

				mc6843->STRA &= ~0x01; /* clear Data Transfer Request */

				if ( cmd == CMD_MSR )
				{
					/* schedule next sector in multiple sector read */
					mc6843->GCR--;
					mc6843->SAR++;
					if ( mc6843->GCR == 0xff )
					{
						mc6843_cmd_end( device );
					}
					else if ( mc6843->SAR > 26 )

					{
						mc6843->STRB |= 0x08; /* set Sector Address Undetected */
						mc6843_cmd_end( device );
					}
					else
					{
						mc6843->timer_cont->adjust( DELAY_ADDR );
					}
				}
				else
				{
					mc6843_cmd_end( device );
				}
			}
		}
		else if ( cmd == 0 )
		{
			data = mc6843->data[0];
		}
		else
		{
			/* XXX TODO: other read modes */
			data = mc6843->data[0];
			logerror( "$%04x mc6843 read in unsupported command mode %i\n", device->machine().firstcpu->pcbase( ), cmd );
		}

		LOG(( "data=%02X\n", data ));

		break;
	}

	case 1: /* Current-Track Address Register (CTAR) */
		data = mc6843->CTAR;
		LOG(( "%f $%04x mc6843_r: read CTAR %i (actual=%i)\n",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), data,
		      floppy_drive_get_current_track( mc6843_floppy_image( device ) ) ));
		break;

	case 2: /* Interrupt Status Register (ISR) */
		data = mc6843->ISR;
		LOG(( "%f $%04x mc6843_r: read ISR %02X: cmd=%scomplete settle=%scomplete sense-rq=%i STRB=%i\n",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), data,
		      (data & 1) ? "" : "not-" , (data & 2) ? "" : "not-",
		      (data >> 2) & 1, (data >> 3) & 1 ));

		/* reset */
		mc6843->ISR &= 8; /* keep STRB */
		mc6843_status_update( device );
		break;

	case 3: /* Status Register A (STRA) */
	{
		/* update */
		device_t* img = mc6843_floppy_image( device );
		int flag = floppy_drive_get_flag_state( img, FLOPPY_DRIVE_READY);
		mc6843->STRA &= 0xa3;
		if ( flag & FLOPPY_DRIVE_READY )
			mc6843->STRA |= 0x04;

		mc6843->STRA |= !floppy_tk00_r(img) << 3;
		mc6843->STRA |= !floppy_wpt_r(img) << 4;

		if ( mc6843->index_pulse )
			mc6843->STRA |= 0x40;

		data = mc6843->STRA;
		LOG(( "%f $%04x mc6843_r: read STRA %02X: data-rq=%i del-dta=%i ready=%i t0=%i wp=%i trk-dif=%i idx=%i busy=%i\n",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), data,
		      data & 1, (data >> 1) & 1, (data >> 2) & 1, (data >> 3) & 1,
		      (data >> 4) & 1, (data >> 5) & 1, (data >> 6) & 1, (data >> 7) & 1 ));
		break;
	}

	case 4: /* Status Register B (STRB) */
		data = mc6843->STRB;
		LOG(( "%f $%04x mc6843_r: read STRB %02X: data-err=%i CRC-err=%i dta--mrk-err=%i sect-mrk-err=%i seek-err=%i fi=%i wr-err=%i hard-err=%i\n",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), data,
		      data & 1, (data >> 1) & 1, (data >> 2) & 1, (data >> 3) & 1,
		      (data >> 4) & 1, (data >> 5) & 1, (data >> 6) & 1, (data >> 7) & 1 ));

		/* (partial) reset */
		mc6843->STRB &= ~0xfb;
		mc6843_status_update( device );
		break;

	case 7: /* Logical-Track Address Register (LTAR) */
		data = mc6843->LTAR;
		LOG(( "%f $%04x mc6843_r: read LTAR %i (actual=%i)\n",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), data,
		      floppy_drive_get_current_track( mc6843_floppy_image( device ) ) ));
		break;

	default:
		logerror( "$%04x mc6843 invalid read offset %i\n", device->machine().firstcpu->pcbase( ), offset );
	}

	return data;
}

WRITE8_DEVICE_HANDLER ( mc6843_w )
{
	mc6843_t* mc6843 = get_safe_token( device );
	switch ( offset ) {

	case 0: /* Data Output Register (DOR) */
	{
		int cmd = mc6843->CMR & 0x0f;
		int FWF = (mc6843->CMR >> 4) & 1;

		LOG(( "%f $%04x mc6843_w: data output cmd=%s(%i), pos=%i/%i, GCR=%i, data=%02X\n",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ),
		      mc6843_cmd[cmd], cmd, mc6843->data_idx,
		      mc6843->data_size, mc6843->GCR, data ));

		if ( cmd == CMD_SSW || cmd == CMD_MSW || cmd == CMD_SWD )
		{
			/* sector write */
			assert( mc6843->data_size > 0 );
			assert( mc6843->data_idx < mc6843->data_size );
			assert( mc6843->data_idx < sizeof(mc6843->data) );
			mc6843->data[ mc6843->data_idx ] = data;
			mc6843->data_idx++;
			if ( mc6843->data_idx >= mc6843->data_size )
			{
				/* end of sector write */
				device_t* img = mc6843_floppy_image( device );

				LOG(( "%f $%04x mc6843_w: write sector %i\n", device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), mc6843->data_id ));

				floppy_drive_write_sector_data(
					img, mc6843->side, mc6843->data_id,
					mc6843->data, mc6843->data_size,
					(cmd == CMD_SWD) ? ID_FLAG_DELETED_DATA : 0 );

				mc6843->STRA &= ~0x01; /* clear Data Transfer Request */

				if ( cmd == CMD_MSW )
				{
					mc6843->GCR--;
					mc6843->SAR++;
					if ( mc6843->GCR == 0xff )
					{
						mc6843_cmd_end( device );
					}
					else if ( mc6843->SAR > 26 )

					{
						mc6843->STRB |= 0x08; /* set Sector Address Undetected */
						mc6843_cmd_end( device );
					}
					else
					{
						mc6843->timer_cont->adjust( DELAY_ADDR );
					}
				}
				else
				{
					mc6843_cmd_end( device );
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

			assert( mc6843->data_idx < sizeof(mc6843->data) );

			mc6843->data[mc6843->data_idx / 2] =
				(mc6843->data[mc6843->data_idx / 2] << 4) | nibble;

			if ( (mc6843->data_idx == 0) && (mc6843->data[0] == 0xfe ) )
			{
				/* address mark detected */
				mc6843->data_idx = 2;
			}
			else if ( mc6843->data_idx == 9 )
			{
				/* address id field complete */
				if ( (mc6843->data[2] == 0) && (mc6843->data[4] == 0) )
				{
					/* valid address id field */
					device_t* img = mc6843_floppy_image( device );
					UINT8 track  = mc6843->data[1];
					UINT8 sector = mc6843->data[3];
					UINT8 filler = 0xe5; /* standard Thomson filler */
					LOG(( "%f $%04x mc6843_w: address id detected track=%i sector=%i\n", device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), track, sector));
					floppy_drive_format_sector( img, mc6843->side, sector, track, 0, sector, 0, filler );
				}
				else
				{
					/* abort */
					mc6843->data_idx = 0;
				}
			}
			else if ( mc6843->data_idx > 0 )
			{
				/* accumulate address id field */
				mc6843->data_idx++;
			}
		}
		else if ( cmd == 0 )
		{
			/* nothing */
		}
		else
		{
			/* XXX TODO: other write modes */
			logerror( "$%04x mc6843 write %02X in unsupported command mode %i (FWF=%i)\n", device->machine().firstcpu->pcbase( ), data, cmd, FWF );
		}
		break;
	}

	case 1: /* Current-Track Address Register (CTAR) */
		mc6843->CTAR = data & 0x7f;
		LOG(( "%f $%04x mc6843_w: set CTAR to %i %02X (actual=%i) \n",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), mc6843->CTAR, data,
		      floppy_drive_get_current_track( mc6843_floppy_image( device ) ) ));
		break;

	case 2: /* Command Register (CMR) */
	{
		int cmd = data & 15;

		LOG(( "%f $%04x mc6843_w: set CMR to $%02X: cmd=%s(%i) FWF=%i DMA=%i ISR3-intr=%i fun-intr=%i\n",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ),
		      data, mc6843_cmd[cmd], cmd, (data >> 4) & 1, (data >> 5) & 1,
		      (data >> 6) & 1, (data >> 7) & 1 ));

		/* sanitize state */
		mc6843->STRA &= ~0x81; /* clear Busy & Data Transfer Request */
		mc6843->data_idx = 0;
		mc6843->data_size = 0;

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
			mc6843->STRA |=  0x80; /* set Busy */
			mc6843->STRA &= ~0x22; /* clear Track Not Equal & Delete Data Mark Detected */
			mc6843->STRB &= ~0x04; /* clear Data Mark Undetected */
			mc6843->timer_cont->adjust( DELAY_ADDR );
			break;
		case CMD_STZ:
		case CMD_SEK:
			mc6843->STRA |= 0x80; /* set Busy */
			mc6843->timer_cont->adjust( DELAY_SEEK );
			break;
		case CMD_FFW:
		case CMD_FFR:
			mc6843->data_idx = 0;
			mc6843->STRA |= 0x01; /* set Data Transfer Request */
			break;
		}

		mc6843->CMR = data;
		mc6843_status_update( device );
		break;
	}

	case 3: /* Set-Up Register (SUR) */
		mc6843->SUR = data;

		/* assume CLK freq = 1MHz (IBM 3740 compatibility) */
		LOG(( "%f $%04x mc6843_w: set SUR to $%02X: head settling time=%fms, track-to-track seek time=%f\n",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ),
		      data, 4.096 * (data & 15), 1.024 * ((data >> 4) & 15) ));
		break;

	case 4: /* Sector Address Register (SAR) */
		mc6843->SAR = data & 0x1f;
		LOG(( "%f $%04x mc6843_w: set SAR to %i (%02X)\n", device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), mc6843->SAR, data ));
		break;

	case 5: /* General Count Register (GCR) */
		mc6843->GCR = data & 0x7f;
		LOG(( "%f $%04x mc6843_w: set GCR to %i (%02X)\n", device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), mc6843->GCR, data ));
		break;

	case 6: /* CRC Control Register (CCR) */
		mc6843->CCR = data & 3;
		LOG(( "%f $%04x mc6843_w: set CCR to %02X: CRC=%s shift=%i\n",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), data,
		      (data & 1) ? "enabled" : "disabled", (data >> 1) & 1 ));
		break;

	case 7: /* Logical-Track Address Register (LTAR) */
		mc6843->LTAR = data & 0x7f;
		LOG(( "%f $%04x mc6843_w: set LTAR to %i %02X (actual=%i)\n",
		      device->machine().time().as_double(), device->machine().firstcpu->pcbase( ), mc6843->LTAR, data,
		      floppy_drive_get_current_track( mc6843_floppy_image( device ) ) ));
		break;

	default:
		logerror( "$%04x mc6843 invalid write offset %i (data=$%02X)\n", device->machine().firstcpu->pcbase( ), offset, data );
	}
}



/************************ reset *****************************/

static DEVICE_RESET( mc6843 )
{
	mc6843_t* mc6843 = get_safe_token( device );
	int i;
	LOG (( "mc6843 reset\n" ));

	/* setup/reset floppy drive */
	for ( i = 0; i < 4; i++ )
	{
		device_t * img = floppy_get_device( device->machine(), i );
		floppy_mon_w(img, CLEAR_LINE);
		floppy_drive_set_ready_state( img, FLOPPY_DRIVE_READY, 0 );
		floppy_drive_set_rpm( img, 300. );
	}

	/* reset registers */
	mc6843->CMR &= 0xf0; /* zero only command */
	mc6843->ISR = 0;
	mc6843->STRA &= 0x5c;
	mc6843->SAR = 0;
	mc6843->STRB &= 0x20;
	mc6843_status_update( device );

	mc6843->data_size = 0;
	mc6843->data_idx = 0;
	mc6843->timer_cont->adjust( attotime::never );
}



/************************ start *****************************/

static DEVICE_START( mc6843 )
{
	mc6843_t* mc6843 = get_safe_token( device );

	mc6843->iface = (const mc6843_interface*)device->static_config();

	mc6843->timer_cont = device->machine().scheduler().timer_alloc(FUNC(mc6843_cont), (void*) device) ;

	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->CTAR );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->CMR );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->ISR );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->SUR );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->STRA );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->STRB );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->SAR );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->GCR );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->CCR );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->LTAR );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->drive );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->side );
	state_save_register_item_array( device->machine(),"mc6843", device->tag(), 0, mc6843->data );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->data_size );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->data_idx );
	state_save_register_item( device->machine(),"mc6843", device->tag(), 0, mc6843->data_id );
}


const device_type MC6843 = &device_creator<mc6843_device>;

mc6843_device::mc6843_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MC6843, "Motorola MC6843 floppy controller", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(mc6843_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mc6843_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc6843_device::device_start()
{
	DEVICE_START_NAME( mc6843 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc6843_device::device_reset()
{
	DEVICE_RESET_NAME( mc6843 )(this);
}



