// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit computers

**********************************************************************/

#include "includes/thomson.h"
#include "machine/wd_fdc.h"


#define VERBOSE 0 /* 0, 1 or 2 */

#define PRINT(x) osd_printf_info x

#define LOG(x)  do { if (VERBOSE > 0) logerror x; } while (0)
#define VLOG(x) do { if (VERBOSE > 1) logerror x; } while (0)




/******************* 3''1/2 & 5''1/4 disk format ********************/

/*
  single density sector format:

  data   bytes

  id field     00      6      byte synchro
  FE      1      id field mark
  1      track
  1      side
  1      sector
  00      1      log sector size (0->128, 1->256, 2->512,...)
  2      CRC (unemulated)
  FF     12      spaces
  data field   00      6      bytes synchro
  FB      1      data field mark
  E5    128      actual data (set to E5 when formatting)
  2      CRC (unemulated)
  FF     22 ?    spaces



  double density sector format:

  data   bytes

  id field     00     12      bit synchro
  A1      3      byte synchro
  FE      1      id field mark
  1      track
  1      side
  1      sector
  01      1      log sector size (0->128, 1->256, 2->512,...)
  2      CRC (unemulated)
  4E/F7   22      spaces
  data field   00     12      bit synchro
  A1      3      bytes synchro
  FB      1      data field mark
  E5    256      actual data (set to E5 when formatting)
  2      CRC (unemulated)
  4E/F7   74 ?    spaces

  => at most  392 bytes / sector
  => at most 6272 bytes / track

  Notes:
  - the BIOS puts 4E bytes as spaces after id and data fields
  - some protected games expect F7 bytes (instead of 4E) after id fields
*/

#define THOM_SIZE_ID          32
#define THOM_SIZE_DATA_LO    (128+80)
#define THOM_SIZE_DATA_HI    (256+80)
#define THOM_SIZE_SYNCHRO     12

static int motor_on;


/* build an identifier, with header & space */
int thomson_state::thom_floppy_make_addr( chrn_id id, UINT8* dst, int sector_size )
{
	if ( sector_size == 128 )
	{
		/* single density */
		memset( dst, 0x00, 6 ); /* synchro bytes */
		dst[  7 ] = 0xfe; /* address field mark */
		dst[  8 ] = id.C;
		dst[  9 ] = id.H;
		dst[ 10 ] = id.N;
		dst[ 11 ] = id.R;
		dst[ 12 ] = 0; /* TODO: CRC */
		dst[ 13 ] = 0; /* TODO: CRC */
		memset( dst + 14, 0xff, 12 ); /* end mark */
		return 36;
	}
	else
	{
		/* double density */
		memset( dst, 0xa1, 3 ); /* synchro bytes */
		dst[ 3 ] = 0xfe; /* address field mark */
		dst[ 4 ] = id.C;
		dst[ 5 ] = id.H;
		dst[ 6 ] = id.N;
		dst[ 7 ] = id.R;
		dst[ 8 ] = 0; /* TODO: CRC */
		dst[ 9 ] = 0; /* TODO: CRC */
		memset( dst + 10, 0xf7, 22 ); /* end mark */
		return 32;
	}
}



/* build a sector, with header & space */
int thomson_state::thom_floppy_make_sector( legacy_floppy_image_device* img, chrn_id id, UINT8* dst, int sector_size )
{
	if ( sector_size == 128 )
	{
		/* single density */
		memset( dst, 0x00, 6 ); /* synchro bytes */
		dst[ 6 ] = 0xfb; /* data field mark */
		img->floppy_drive_read_sector_data
			(  id.H, id.data_id, dst + 7, sector_size );
		dst[ sector_size + 7 ] = 0; /* TODO: CRC */
		dst[ sector_size + 8 ] = 0; /* TODO: CRC */
		memset( dst + sector_size + 9, 0xff, 22 ); /* end mark */
		return sector_size + 31;
	}
	else
	{
		/* double density */
		memset( dst, 0xa1, 3 ); /* synchro bytes */
		dst[ 3 ] = 0xfb; /* data field mark */
		img->floppy_drive_read_sector_data
			(  id.H, id.data_id, dst + 4, sector_size );
		dst[ sector_size + 4 ] = 0; /* TODO: CRC */
		dst[ sector_size + 5 ] = 0; /* TODO: CRC */
		memset( dst + sector_size + 6, 0xF7, 74 ); /* end mark */
		return sector_size + 80;
	}
}



/* build a whole track */
int thomson_state::thom_floppy_make_track( legacy_floppy_image_device* img, UINT8* dst, int sector_size, int side )
{
	UINT8 space = ( sector_size == 128 ) ? 0xff : 0;
	UINT8* org = dst;
	chrn_id id;
	int nb;

	/* go to start of track */
	while ( ! img->floppy_drive_get_flag_state( FLOPPY_DRIVE_INDEX ) )
	{
		if ( ! img->floppy_drive_get_next_id( side, &id ) )
			return 0;
	}

	/* for each sector... */
	for ( nb = 0; nb < 16; nb++ )
	{
		if ( ! img->floppy_drive_get_next_id( side, &id ) )
			break;

		memset( dst, space, THOM_SIZE_SYNCHRO ); dst += THOM_SIZE_SYNCHRO;
		dst += thom_floppy_make_addr( id, dst, sector_size );
		memset( dst, space, THOM_SIZE_SYNCHRO ); dst += THOM_SIZE_SYNCHRO;
		dst += thom_floppy_make_sector( img, id, dst, sector_size );

		if ( img->floppy_drive_get_flag_state( FLOPPY_DRIVE_INDEX ) )
			break;
	}
	return dst - org;
}



/******************* QDD disk format ********************/

/*
  sector format:

  data   bytes

  id field     16     17      synchro
  A5      1      id field mark
  1      sector (1-400) hi byte
  1      sector (1-400) low byte
  1      check-sum (sum modulo 256 of 3 last bytes)

  data field   16     10      synchro
  5A      1      data field mark
  FF    128      actual data (set to FF when formatting)
  1      check-sum (sum modulo 256 of 129 last bytes)


  there are 400 sectors numbered from 1
*/

#define THOM_QDD_SYNCH_DISK 100
#define THOM_QDD_SYNCH_ADDR  17
#define THOM_QDD_SYNCH_DATA  10

#define THOM_QDD_SIZE_ID     (   4 + THOM_QDD_SYNCH_ADDR  )
#define THOM_QDD_SIZE_DATA   ( 130 + THOM_QDD_SYNCH_DATA )


/* build an identifier, with header */
int thomson_state::thom_qdd_make_addr( int sector, UINT8* dst )
{
	dst[  0 ] = 0xa5;
	dst[  1 ] = sector >> 8;
	dst[  2 ] = sector & 0xff;
	dst[  3 ] = dst[ 0 ] + dst[ 1 ] + dst[ 2 ];
	return 4;
}



/* build a sector, with header */
int thomson_state::thom_qdd_make_sector( legacy_floppy_image_device* img, int sector, UINT8* dst )
{
	int i;
	dst[ 0 ] = 0x5a;
	img->floppy_drive_read_sector_data (  0, sector, dst + 1, 128 );
	dst[ 129 ] = 0;
	for ( i = 0; i < 129; i++ )
		dst[ 129 ] += dst[ i ];
	return 130;
}



/* build a whole disk */
int thomson_state::thom_qdd_make_disk ( legacy_floppy_image_device* img, UINT8* dst )
{
	UINT8* org = dst;
	int i;

	memset( dst, 0x16, THOM_QDD_SYNCH_DISK ); dst += THOM_QDD_SYNCH_DISK;

	for ( i = 1; i <= 400; i++ )
	{
		memset( dst, 0x16, THOM_QDD_SYNCH_ADDR ); dst += THOM_QDD_SYNCH_ADDR;
		dst += thom_qdd_make_addr( i, dst );
		memset( dst, 0x16, THOM_QDD_SYNCH_DATA ); dst += THOM_QDD_SYNCH_DATA;
		dst += thom_qdd_make_sector( img, i, dst );
	}

	memset( dst, 0x16, THOM_QDD_SYNCH_DISK ); dst += THOM_QDD_SYNCH_DISK;

	return dst - org;
}



/*********************** CD 90-640 controller ************************/

/* 5''1/4 two-sided double-density
   used in TO7, TO7/70, MO5 computers
   based on a WD2793 (or lower ?) chip
*/



static UINT8 to7_5p14_select;



READ8_MEMBER( thomson_state::to7_5p14_r )
{
	wd2793_t *fdc = machine().device<wd2793_t>("wd2793");

	if ( offset < 4 )
		return fdc->read(space, offset );
	else if ( offset == 8 )
		return to7_5p14_select;
	else
		logerror ( "%f $%04x to7_5p14_r: invalid read offset %i\n", machine().time().as_double(), m_maincpu->pc(), offset );
	return 0;
}



WRITE8_MEMBER( thomson_state::to7_5p14_w )
{
	wd2793_t *fdc = machine().device<wd2793_t>("wd2793");
	if ( offset < 4 )
		fdc->write(space, offset, data );
	else if ( offset == 8 )
	{
		/* drive select */
		floppy_image_device *floppy = nullptr;

		if (BIT(data, 1)) floppy = fdc->subdevice<floppy_connector>("0")->get_device();
		if (BIT(data, 2)) floppy = fdc->subdevice<floppy_connector>("1")->get_device();

		fdc->set_floppy(floppy);

		if (floppy)
		{
			thom_floppy_active( 0 );
			floppy->mon_w(0);
			floppy->ss_w(BIT(data, 0));
		}
	}
	else
		logerror ( "%f $%04x to7_5p14_w: invalid write offset %i (data=$%02X)\n",
				machine().time().as_double(), m_maincpu->pc(), offset, data );
}



void thomson_state::to7_5p14_reset()
{
	wd2793_t *fdc = machine().device<wd2793_t>("wd2793");
	LOG(( "to7_5p14_reset: CD 90-640 controller\n" ));
	fdc->reset();
}



void thomson_state::to7_5p14_init()
{
	LOG(( "to7_5p14_init: CD 90-640 controller\n" ));
	save_item(NAME(to7_5p14_select));
}



/*********************** CD 90-015 controller ************************/

/* 5''1/4 one-sided single-density (up to 4 one-sided drives)
   used in TO7, TO7/70, MO5 computers
   based on HD 46503 S chip, but we actually use a MC 6843 instead
   (seems they are clone)
*/



static UINT8 to7_5p14sd_select;



READ8_MEMBER( thomson_state::to7_5p14sd_r )
{
	if ( offset < 8 )
		return m_mc6843->read( space, offset );
	else if ( offset >= 8 && offset <= 9 )
		return to7_5p14sd_select;
	else
		logerror ( "%f $%04x to7_5p14sd_r: invalid read offset %i\n",  machine().time().as_double(), m_maincpu->pc(), offset );
	return 0;
}



WRITE8_MEMBER( thomson_state::to7_5p14sd_w )
{
	if ( offset < 8 )
		m_mc6843->write( space, offset, data );
	else if ( offset >= 8 && offset <= 9 )
	{
		/* drive select */
		int drive = -1, side = 0;

		if ( data & 1 )
		{
			drive = 0;
			side = 0;
		}
		else if ( data & 2 )
		{
			drive = 1;
			side = 1;
		}
		else if ( data & 4 )
		{
			drive = 2;
			side = 0;
		}
		else if ( data & 8 )
		{
			drive = 3;
			side = 1;
		}

		to7_5p14sd_select = data;

		if ( drive != -1 )
		{
			thom_floppy_active( 0 );
			m_mc6843->set_drive( drive );
			m_mc6843->set_side( side );
			LOG(( "%f $%04x to7_5p14sd_w: $%02X set drive=%i side=%i\n",
					machine().time().as_double(), m_maincpu->pc(), data, drive, side ));
		}
	}
	else
		logerror ( "%f $%04x to7_5p14sd_w: invalid write offset %i (data=$%02X)\n",
				machine().time().as_double(), m_maincpu->pc(), offset, data );
}

void thomson_state::to7_5p14_index_pulse_callback( device_t *controller,legacy_floppy_image_device *image, int state )
{
	m_mc6843->set_index_pulse( state );
}

void thomson_state::to7_5p14sd_reset()
{
	int i;
	LOG(( "to7_5p14sd_reset: CD 90-015 controller\n" ));
	for ( i = 0; i < floppy_get_count( machine() ); i++ )
	{
		legacy_floppy_image_device * img = floppy_get_device( machine(), i );
		if (img) {
			img->floppy_drive_set_ready_state( FLOPPY_DRIVE_READY, 0 );
			img->floppy_drive_set_rpm( 300. );
			img->floppy_drive_seek( - img->floppy_drive_get_current_track() );
		}
	}
}


void thomson_state::to7_5p14sd_init()
{
	LOG(( "to7_5p14sd_init: CD 90-015 controller\n" ));
	save_item(NAME(to7_5p14sd_select));
}


/*********************** CQ 90-028 controller ************************/

/* QDD 2''8 controller
   used in TO7, TO7/70, MO5 computers
   it is based on a MC6852 SSDA

   Note: the MC6852 is only partially emulated, most features are not used in
   the controller and are ignored.
*/



/* MC6852 status */
#define QDD_S_RDA  0x01 /* receiver data available */
#define QDD_S_TDRA 0x02 /* transitter data register available */
#define QDD_S_NDCD 0x04 /* data carrier detect, negated (unused) */
#define QDD_S_NCTS 0x08 /* clear-to-send, negated write-protect */
#define QDD_S_TUF  0x10 /* transmitter underflow (unused) */
#define QDD_S_OVR  0x20 /* receiver overrun (unused) */
#define QDD_S_PE   0x40 /* receiver parity error (unused) */
#define QDD_S_IRQ  0x80 /* interrupt request */
#define QDD_S_ERR  (QDD_S_TUF | QDD_S_OVR | QDD_S_PE | QDD_S_NDCD | QDD_S_NCTS)


/* MC6852 control */
#define QDD_C1_RIE       0x20 /* interrupt on reveive */
#define QDD_C1_TIE       0x10 /* interrupt on transmit */
#define QDD_C1_CLRSYNC   0x08 /* clear receiver sync char (unused) */
#define QDD_C1_STRIPSYNC 0x04 /* strips sync from received (unused) */
#define QDD_C1_TRESET    0x02 /* transmitter reset */
#define QDD_C1_RRESET    0x01 /* receiver reset */
#define QDD_C2_EIE       0x80 /* interrupt on error */
#define QDD_C2_TSYNC     0x40 /* underflow = ff if 0 / sync if 1 (unused) */
#define QDD_C2_BLEN      0x04 /* transfer byte length (unused) */
#define QDD_C3_CLRTUF    0x08 /* clear underflow */
#define QDD_C3_CLRCTS    0x04 /* clear CTS  */
#define QDD_C3_SYNCLEN   0x02 /* sync byte length (unused) */
#define QDD_C3_SYNCMODE  0x01 /* external / internal sync mode (unused) */


/* a track is actually the whole disk = 400 128-byte sectors + headers */
#define QDD_MAXBUF ( THOM_QDD_SIZE_ID + THOM_QDD_SIZE_DATA ) * 512


struct to7qdd_t
{
	/* MC6852 registers */
	UINT8 status;
	UINT8 ctrl1;
	UINT8 ctrl2;
	UINT8 ctrl3;

	/* extra registers */
	UINT8 drive;

	/* internal state */
	UINT8  data[QDD_MAXBUF];  /* enough for a whole track */
	UINT32 data_idx;          /* byte position in track */
	UINT32 start_idx;         /* start of write position */
	UINT32 data_size;         /* track length */
	UINT8  data_crc;          /* checksum when writing */
	UINT8  index_pulse;       /* one pulse per track */

};

static to7qdd_t * to7qdd;



void thomson_state::to7_qdd_index_pulse_cb( device_t *controller,legacy_floppy_image_device *image, int state )
{
	to7qdd->index_pulse = state;

	if ( state )
	{
		/* rewind to disk start */
		to7qdd->data_idx = 0;
		to7qdd->start_idx = 0;
		to7qdd->data_size = 0;
	}

	VLOG(( "%f to7_qdd_pulse_cb: state=%i\n", machine().time().as_double(), state ));
}



legacy_floppy_image_device * thomson_state::to7_qdd_image()
{
	return floppy_get_device( machine(), 0 );
}



/* update MC6852 status register */
void thomson_state::to7_qdd_stat_update()
{
	/* byte-ready */
	to7qdd->status |= QDD_S_RDA | QDD_S_TDRA;
	if ( ! to7qdd->drive )
		to7qdd->status |= QDD_S_PE;

	/* write-protect */
	if (to7_qdd_image()->floppy_wpt_r() == CLEAR_LINE)
		to7qdd->status |= QDD_S_NCTS;

	/* sticky reset conditions */
	if ( to7qdd->ctrl1 & QDD_C1_RRESET )
		to7qdd->status &= ~(QDD_S_PE | QDD_S_RDA | QDD_S_OVR);
	if ( to7qdd->ctrl1 & QDD_C1_TRESET )
		to7qdd->status &= ~(QDD_S_TDRA | QDD_S_TUF);

	/* irq update */

	if ( ( (to7qdd->ctrl1 & QDD_C1_RIE) && !(to7qdd->status & QDD_S_RDA ) ) ||
			( (to7qdd->ctrl1 & QDD_C1_TIE) && !(to7qdd->status & QDD_S_TDRA) ) ||
			( (to7qdd->ctrl2 & QDD_C2_EIE) && !(to7qdd->status & QDD_S_ERR ) ) )
		to7qdd->status &= ~QDD_S_IRQ;

	if ( ( (to7qdd->ctrl1 & QDD_C1_RIE) && (to7qdd->status & QDD_S_RDA ) ) ||
			( (to7qdd->ctrl1 & QDD_C1_TIE) && (to7qdd->status & QDD_S_TDRA) ) ||
			( (to7qdd->ctrl2 & QDD_C2_EIE) && (to7qdd->status & QDD_S_ERR ) ) )
		to7qdd->status |= QDD_S_IRQ;
}



UINT8 thomson_state::to7_qdd_read_byte()
{
	UINT8 data;

	/* rebuild disk if needed */
	if ( !to7qdd->data_size )
	{
		to7qdd->data_size = thom_qdd_make_disk( to7_qdd_image(), to7qdd->data );
		assert( to7qdd->data_idx < sizeof( to7qdd->data ) );
	}

	if ( to7qdd->data_idx >= to7qdd->data_size )
		data = 0;
	else
		data = to7qdd->data[ to7qdd->data_idx ];

	VLOG(( "%f $%04x to7_qdd_read_byte: RDATA off=%i/%i data=$%02X\n",
			machine().time().as_double(), m_maincpu->pc(),
			to7qdd->data_idx, to7qdd->data_size, data ));

	to7qdd->data_idx++;
	to7qdd->start_idx = to7qdd->data_idx;

	return data;
}



/* This is quite complex: bytes are written one at a time by the CPU and we
   must detect the following patterns:
   * CPU write id field and data field => format
   * CPU write data field after it has read an id field => sector write
   */
void thomson_state::to7_qdd_write_byte( UINT8 data )
{
	int i;

	/* rebuild disk if needed */
	if ( !to7qdd->data_size )
	{
		to7qdd->data_size = thom_qdd_make_disk( to7_qdd_image(), to7qdd->data );
		assert( to7qdd->data_idx < sizeof( to7qdd->data ) );
	}

	if ( ( to7qdd->start_idx != to7qdd->data_idx || /* field in construction */
			data==0xA5 || data==0x5A ) &&    /* first byte of tentative field */
			to7qdd->data_idx <to7qdd->data_size )
	{
		/* this is the first byte of the field */
		if ( to7qdd->start_idx == to7qdd->data_idx )
			to7qdd->data_crc = 0;

		/* accumulate bytes */
		to7qdd->data[ to7qdd->data_idx ] = data;
		to7qdd->data_idx++;

		VLOG (( "%f $%04x to7_qdd_write_byte: got $%02X offs=%i-%i\n",
			machine().time().as_double(), m_maincpu->pc(), data,
			to7qdd->start_idx, to7qdd->data_idx ));

		/* end of tentative id field */
		if ( to7qdd->data_idx == to7qdd->start_idx + 4 &&
				to7qdd->data[ to7qdd->start_idx ] == 0xA5 &&
				to7qdd->data[ to7qdd->start_idx + 3 ] == to7qdd->data_crc )
		{
			/* got an id field => format */
			int sector = (int) to7qdd->data[ to7qdd->start_idx + 1 ] * 256 + (int) to7qdd->data[ to7qdd->start_idx + 2 ];
			UINT8 filler = 0xff;

			LOG(( "%f $%04x to7_qdd_write_byte: got id field for sector=%i\n",
					machine().time().as_double(), m_maincpu->pc(), sector ));

			to7_qdd_image()->floppy_drive_format_sector(
							0, sector, 0, 0, sector, 128, filler );
						thom_floppy_active( 1 );
			to7qdd->start_idx = to7qdd->data_idx;
		}

		/* end of tentative data field */
		else if ( to7qdd->data_idx == to7qdd->start_idx + 130 &&
				to7qdd->data[ to7qdd->start_idx  ] == 0x5A &&
				to7qdd->data[ to7qdd->start_idx + 129 ] == to7qdd->data_crc )
		{
			/* look backwards for previous id field */
			for ( i = to7qdd->start_idx - 3; i >= 0; i-- )
			{
				if ( to7qdd->data[ i ] == 0xA5 &&
						( ( to7qdd->data[ i ] + to7qdd->data[ i + 1 ] +
						to7qdd->data[ i + 2 ] ) & 0xff
							) == to7qdd->data[ i + 3 ] )
					break;
			}

			if ( i >= 0 )
			{
				/* got an id & a data field => write */
				int sector = (int) to7qdd->data[ i + 1 ] * 256 + (int) to7qdd->data[ i + 2 ];

				LOG(( "%f $%04x to7_qdd_write_byte: goto data field for sector=%i\n",
						machine().time().as_double(), m_maincpu->pc(), sector ));

				to7_qdd_image()->floppy_drive_write_sector_data( 0, sector, to7qdd->data + to7qdd->start_idx + 1, 128, 0 );
								thom_floppy_active( 1 );
			}

			to7qdd->start_idx = to7qdd->data_idx;
		}

		else to7qdd->data_crc += data;
	}
}



READ8_MEMBER( thomson_state::to7_qdd_r )
{
	switch ( offset )
	{
	case 0: /* MC6852 status */
		to7_qdd_stat_update();
		VLOG(( "%f $%04x to7_qdd_r: STAT=$%02X irq=%i pe=%i ovr=%i und=%i tr=%i rd=%i ncts=%i\n",
				machine().time().as_double(), m_maincpu->pc(), to7qdd->status,
				to7qdd->status & QDD_S_IRQ  ? 1 : 0,
				to7qdd->status & QDD_S_PE   ? 1 : 0,
				to7qdd->status & QDD_S_OVR  ? 1 : 0,
				to7qdd->status & QDD_S_TUF  ? 1 : 0,
				to7qdd->status & QDD_S_TDRA ? 1 : 0,
				to7qdd->status & QDD_S_RDA  ? 1 : 0,
				to7qdd->status & QDD_S_NCTS ? 1 : 0 ));
		return to7qdd->status;

	case 1: /* MC6852 data input => read byte from disk */
		to7qdd->status &= ~(QDD_S_RDA | QDD_S_PE | QDD_S_OVR);
		to7_qdd_stat_update();
		return to7_qdd_read_byte();

	case 8: /* floppy status */
	{
		UINT8 data = 0;
		device_image_interface* img = dynamic_cast<device_image_interface *>(to7_qdd_image());
		if ( ! img->exists() )
			data |= 0x40; /* disk present */
		if ( to7qdd->index_pulse )
			data |= 0x80; /* disk start */
		VLOG(( "%f $%04x to7_qdd_r: STATUS8 $%02X\n", machine().time().as_double(), m_maincpu->pc(), data ));
		return data;
	}

	default:
		logerror ( "%f $%04x to7_qdd_r: invalid read offset %i\n", machine().time().as_double(), m_maincpu->pc(), offset );
		return 0;
	}
}



WRITE8_MEMBER( thomson_state::to7_qdd_w )
{
	switch ( offset )
	{
	case 0: /* MC6852 control 1 */
		/* reset */
		if ( data & QDD_C1_RRESET )
			to7qdd->status &= ~(QDD_S_PE | QDD_S_RDA | QDD_S_OVR);
		if ( data & QDD_C1_TRESET )
			to7qdd->status &= ~(QDD_S_TDRA | QDD_S_TUF);

		to7qdd->ctrl1 = ( data & ~(QDD_C1_RRESET | QDD_C1_TRESET) ) |( data &  (QDD_C1_RRESET | QDD_C1_TRESET) & to7qdd->ctrl1 );
		to7_qdd_stat_update();
		VLOG(( "%f $%04x to7_qdd_w: CTRL1=$%02X reset=%c%c %s%sirq=%c%c\n",
				machine().time().as_double(), m_maincpu->pc(), data,
				data & QDD_C1_RRESET ? 'r' : '-', data & QDD_C1_TRESET ? 't' : '-',
				data & QDD_C1_STRIPSYNC ? "strip-sync " : "",
				data & QDD_C1_CLRSYNC ? "clear-sync " : "",
				data & QDD_C1_RIE ? 'r' : '-',
				data & QDD_C1_TIE ? 't' : '-' ));
		break;

	case 1:
		switch ( to7qdd->ctrl1 >> 6 )
		{
		case 0: /* MC6852 control 2 */
		{
#if 0
			/* most of these are unused now */
			static const int bit[8] = { 6, 6, 7, 8, 7, 7, 8, 8 };
			static const int par[8] = { 2, 1, 0, 0, 2, 1, 2, 1 };
			static const char *const parname[3] = { "none", "odd", "even" };
			int bits, parity;
			bits   = bit[ (data >> 3) & 7 ];
			parity = par[ (data >> 3) & 7 ];
			to7_qdd_stat_update(space.machine());
			VLOG(( "%f $%04x to7_qdd_w: CTRL2=$%02X bits=%i par=%s blen=%i under=%s%s\n",
					machine().time().as_double(), m_maincpu->pc(), data,
					bits, parname[ parity ], data & QDD_C2_BLEN ? 1 : 2,
					data & QDD_C2_TSYNC ? "sync" : "ff",
					data & QDD_C2_EIE ? "irq-err" : "" ));
#endif
			to7qdd->ctrl2 = data;
			break;
		}

		case 1: /* MC6852 control 3 */
			to7qdd->ctrl3 = data;
			/* reset just once each write, not sticky */
			if ( data & QDD_C3_CLRTUF )
				to7qdd->status &= ~QDD_S_TUF;
			if ( data & QDD_C3_CLRCTS )
				to7qdd->status &= ~QDD_S_NCTS;
			to7_qdd_stat_update();
			VLOG(( "%f $%04x to7_qdd_w: CTRL3=$%02X %s%ssync-len=%i sync-mode=%s\n",
					machine().time().as_double(), m_maincpu->pc(), data,
					data & QDD_C3_CLRTUF ? "clr-tuf " : "",
					data & QDD_C3_CLRCTS ? "clr-cts " : "",
					data & QDD_C3_SYNCLEN ? 1 : 2,
					data & QDD_C3_SYNCMODE ? "ext" : "int" ));
			break;

		case 2: /* MC6852 sync code => write byte to disk */
			to7_qdd_write_byte( data );
			break;

		case 3: /* MC6852 data out => does not seem to be used */
			VLOG(( "%f $%04x to7_qdd_w: ignored WDATA=$%02X\n", machine().time().as_double(), m_maincpu->pc(), data ));
			break;

		}
		break;

	case 8: /* set drive */
		to7qdd->drive = data;
		VLOG(( "%f $%04x to7_qdd_w: DRIVE=$%02X\n", machine().time().as_double(), m_maincpu->pc(), data ));
		break;

	case 12: /* motor pulse ? */
		thom_floppy_active( 0 );
		VLOG(( "%f $%04x to7_qdd_w: MOTOR=$%02X\n", machine().time().as_double(), m_maincpu->pc(), data ));
		break;

	default:
		logerror ( "%f $%04x to7_qdd_w: invalid write offset %i (data=$%02X)\n", machine().time().as_double(), m_maincpu->pc(), offset, data );
	}
}



void thomson_state::to7_qdd_reset()
{
	int i;
	LOG(( "to7_qdd_reset: CQ 90-028 controller\n" ));

	for ( i = 0; i < floppy_get_count( machine() ); i++ )
	{
		legacy_floppy_image_device * img = floppy_get_device( machine(), i );
		if (img) {
			img->floppy_drive_set_ready_state(  FLOPPY_DRIVE_READY, 0 );

			motor_on = CLEAR_LINE;
			img->floppy_mon_w( motor_on);

			/* pulse each time the whole-disk spiraling track ends */
			/* at 90us per byte read, the disk can be read in 6s */
			img->floppy_drive_set_rpm( 60. / 6. );
		}
	}

	to7qdd->ctrl1 |= QDD_C1_TRESET | QDD_C1_RRESET; /* reset */
	to7qdd->ctrl2 &= 0x7c; /* clear EIE, PC2-PC1 */
	to7qdd->ctrl3 &= 0xfe; /* internal sync */
	to7qdd->drive = 0;
	to7_qdd_stat_update();
}



void thomson_state::to7_qdd_init()
{
	LOG(( "to7_qdd_init: CQ 90-028 controller\n" ));

	to7qdd = auto_alloc(machine(), to7qdd_t);

	save_item(NAME(to7qdd->status));
	save_item(NAME(to7qdd->ctrl1));
	save_item(NAME(to7qdd->ctrl2));
	save_item(NAME(to7qdd->ctrl3));
	save_item(NAME(to7qdd->drive));
	save_item(NAME(to7qdd->data_idx));
	save_item(NAME(to7qdd->start_idx));
	save_item(NAME(to7qdd->data_size));
	save_item(NAME(to7qdd->data_crc));
	save_item(NAME(to7qdd->index_pulse));
	save_item(NAME(to7qdd->data));
}



/********************** THMFC1 controller *************************/

/* custom Thomson Gate-array for 3''1/2, 5''1/4 (double density only) & QDD
 */



/* types of high-level operations */
#define THMFC1_OP_RESET         0
#define THMFC1_OP_WRITE_SECT    1
#define THMFC1_OP_READ_ADDR     2
#define THMFC1_OP_READ_SECT     3


/* STAT0 flags */
#define THMFC1_STAT0_SYNCHRO        0x01 /* bit clock synchronized */
#define THMFC1_STAT0_BYTE_READY_OP  0x02 /* byte ready (high-level operation) */
#define THMFC1_STAT0_CRC_ERROR      0x04
#define THMFC1_STAT0_FINISHED       0x08
#define THMFC1_STAT0_FINISHING      0x10 /* (unemulated) */
#define THMFC1_STAT0_BYTE_READY_POL 0x80 /* polling mode */


/*#define THOM_MAXBUF (THOM_SIZE_ID+THOM_SIZE_DATA_HI+2*THOM_SIZE_SYNCHRO)*17*/
#define THOM_MAXBUF ( THOM_QDD_SIZE_ID + THOM_QDD_SIZE_DATA ) * 512


struct thmfc1_t
{
	UINT8   op;
	UINT8   sector;            /* target sector, in [1,16] */
		UINT32  sector_id;
	UINT8   track;             /* current track, in [0,79] */
	UINT8   side;              /* current side, 0 or 1 */
	UINT8   drive;             /* 0 to 3 */
	UINT16  sector_size;       /* 128 or 256 (512, 1024 not supported) */
	UINT8   formatting;
	UINT8   ipl;               /* index pulse / QDD start */
	UINT8   wsync;             /* synchronization word */

	UINT8   data[THOM_MAXBUF]; /* enough for a whole track */
	UINT32  data_idx;          /* reading / writing / formatting pos */
	UINT32  data_size;         /* bytes to read / write */
	UINT32  data_finish;       /* when to raise the finished flag */
	UINT32  data_raw_idx;      /* byte index for raw track reading */
	UINT32  data_raw_size;     /* size of track already cached in data */
	UINT8   data_crc;          /* check-sum of written data */

	UINT8   stat0;             /* status register */

};

static thmfc1_t * thmfc1;


static emu_timer* thmfc_floppy_cmd;



legacy_floppy_image_device * thomson_state::thmfc_floppy_image()
{
	return floppy_get_device( machine(), thmfc1->drive );
}



int thomson_state::thmfc_floppy_is_qdd ( legacy_floppy_image_device *image )
{
	if (image==nullptr) return 0;
	if (!image->exists()) return 0;
	return image->length()==51200; // idf QDD
}



void thomson_state::thmfc_floppy_index_pulse_cb ( device_t *controller,legacy_floppy_image_device *image, int state )
{
	if ( image != thmfc_floppy_image())
		return;

	if ( thmfc_floppy_is_qdd(image))
	{
		/* pulse each time the whole-disk spiraling track ends */
		image->floppy_drive_set_rpm( 16.92f /* 423/25 */ );
		thmfc1->ipl = state;
		if ( state )
		{
			thmfc1->data_raw_size = 0;
			thmfc1->data_raw_idx = 0;
			thmfc1->data_idx = 0;
		}
	}
	else
	{
		image->floppy_drive_set_rpm( 300. );
		thmfc1->ipl = state;
		if ( state  )
			thmfc1->data_raw_idx = 0;
	}

	VLOG(( "%f thmfc_floppy_index_pulse_cb: state=%i\n", machine().time().as_double(), state ));
}



int thomson_state::thmfc_floppy_find_sector( chrn_id* dst )
{
	legacy_floppy_image_device* img = thmfc_floppy_image();
	chrn_id id;
	int r = 0;

	/* scan track, try 4 revolutions */
	while ( r < 4 )
	{
		if ( img->floppy_drive_get_next_id( thmfc1->side, &id ) )
		{
			if ( id.C == thmfc1->track &&
					id.R == thmfc1->sector &&
					(128 << id.N) == thmfc1->sector_size
					/* check side ?  id.H == thmfc1->side */ )
			{
				if ( dst )
					memcpy( dst, &id, sizeof( chrn_id ) );
				thmfc1->stat0 = THMFC1_STAT0_BYTE_READY_POL;
				LOG (( "thmfc_floppy_find_sector: sector found C=%i H=%i R=%i N=%i\n", id.C, id.H, id.R, id.N ));
				return 1;
			}
		}

		if ( img->floppy_drive_get_flag_state( FLOPPY_DRIVE_INDEX ) )
			r++;
	}

	thmfc1->stat0 = THMFC1_STAT0_CRC_ERROR | THMFC1_STAT0_FINISHED;
	LOG (( "thmfc_floppy_find_sector: sector not found drive=%i track=%i sector=%i\n", floppy_get_drive(img), thmfc1->track, thmfc1->sector ));
	return 0;
}



/* complete command (by read, write, or timeout) */
void thomson_state::thmfc_floppy_cmd_complete()
{
	LOG (( "%f thmfc_floppy_cmd_complete_cb: cmd=%i off=%i/%i/%i\n",
			machine().time().as_double(), thmfc1->op, thmfc1->data_idx,
			thmfc1->data_finish - 1, thmfc1->data_size - 1 ));

	if ( thmfc1->op == THMFC1_OP_WRITE_SECT )
	{
		legacy_floppy_image_device * img = thmfc_floppy_image();
		img->floppy_drive_write_sector_data( thmfc1->side, thmfc1->sector_id, thmfc1->data + 3, thmfc1->data_size - 3, 0 );
				thom_floppy_active( 1 );
	}
	thmfc1->op = THMFC1_OP_RESET;
	thmfc1->stat0 |= THMFC1_STAT0_FINISHED;
	thmfc1->data_idx = 0;
	thmfc1->data_size = 0;
	thmfc_floppy_cmd->adjust(attotime::never);
}



TIMER_CALLBACK_MEMBER( thomson_state::thmfc_floppy_cmd_complete_cb )
{
	thmfc_floppy_cmd_complete();
}



/* intelligent read: show just one field, skip header */
UINT8 thomson_state::thmfc_floppy_read_byte()
{
	UINT8 data = thmfc1->data[ thmfc1->data_idx ];

	VLOG(( "%f $%04x thmfc_floppy_read_byte: off=%i/%i/%i data=$%02X\n",
			machine().time().as_double(), m_maincpu->pc(),
			thmfc1->data_idx, thmfc1->data_finish - 1, thmfc1->data_size - 1,
			data ));

	if ( thmfc1->data_idx >= thmfc1->data_size - 1 )
		thmfc_floppy_cmd_complete();
	else
		thmfc1->data_idx++;

	if ( thmfc1->data_idx >= thmfc1->data_finish )
		thmfc1->stat0 |= THMFC1_STAT0_FINISHED;

	return data;
}



/* dumb read: show whole track with field headers and gaps  */
UINT8 thomson_state::thmfc_floppy_raw_read_byte()
{
	UINT8 data;

	/* rebuild track if needed */
	if ( ! thmfc1->data_raw_size )
	{
		if ( thmfc_floppy_is_qdd(thmfc_floppy_image()))
			/* QDD: track = whole disk */
			thmfc1->data_raw_size = thom_qdd_make_disk ( thmfc_floppy_image(), thmfc1->data );
		else
		{
			thmfc1->data_raw_idx = 0;
			thmfc1->data_raw_size = thom_floppy_make_track( thmfc_floppy_image(), thmfc1->data,
									thmfc1->sector_size, thmfc1->side );
		}
		assert( thmfc1->data_raw_size < sizeof( thmfc1->data ) );
	}

	if ( thmfc1->data_raw_idx >= thmfc1->data_raw_size )
		data = 0;
	else
		data = thmfc1->data[ thmfc1->data_raw_idx ];

	VLOG(( "%f $%04x thmfc_floppy_raw_read_byte: off=%i/%i data=$%02X\n",
			machine().time().as_double(), m_maincpu->pc(),
			thmfc1->data_raw_idx, thmfc1->data_raw_size, data ));

	thmfc1->data_raw_idx++;

	return data;
}



/* QDD writing / formating */
void thomson_state::thmfc_floppy_qdd_write_byte( UINT8 data )
{
	int i;

	if ( thmfc1->formatting &&
			( thmfc1->data_idx || data==0xA5 || data==0x5A ) &&
			thmfc1->data_raw_idx < THOM_MAXBUF )
	{
		if ( ! thmfc1->data_raw_size )
		{
			thmfc1->data_raw_size = thom_qdd_make_disk ( thmfc_floppy_image(), thmfc1->data );
			assert( thmfc1->data_raw_size < sizeof( thmfc1->data ) );
		}

		/* accumulate bytes to form a field */
		thmfc1->data[ thmfc1->data_raw_idx ] = data;
		thmfc1->data_raw_idx++;

		if ( ! thmfc1->data_idx  )
		{
			/* start */
			thmfc1->data_crc = 0;
			thmfc1->data_idx = thmfc1->data_raw_idx;
		}

		VLOG (( "%f $%04x thmfc_floppy_qdd_write_byte: $%02X offs=%i-%i\n",
			machine().time().as_double(), m_maincpu->pc(), data,
			thmfc1->data_idx,thmfc1->data_raw_idx ));

		if ( thmfc1->data_raw_idx == thmfc1->data_idx + 3 &&
				thmfc1->data[ thmfc1->data_idx - 1 ] == 0xA5 &&
				thmfc1->data[ thmfc1->data_idx + 2 ] == thmfc1->data_crc )
		{
			/* got an id field => format */
			int sector = (int) thmfc1->data[ thmfc1->data_idx ] * 256 + (int) thmfc1->data[ thmfc1->data_idx + 1 ];
			UINT8 filler = 0xff;

			LOG(( "%f $%04x thmfc_floppy_qdd_write_byte: id field, sector=%i\n", machine().time().as_double(), m_maincpu->pc(), sector ));

			thmfc_floppy_image()->floppy_drive_format_sector( 0, sector, 0, 0, sector, 128, filler );
			thom_floppy_active( 1 );
			thmfc1->data_idx = 0;
		}

		else if ( thmfc1->data_raw_idx == thmfc1->data_idx + 129 &&
				thmfc1->data[ thmfc1->data_idx -   1 ] == 0x5A &&
				thmfc1->data[ thmfc1->data_idx + 128 ] == thmfc1->data_crc )
		{
			/* look backwards for previous id field */
			for ( i = thmfc1->data_idx - 4; i >= 0; i-- )
			{
				if ( thmfc1->data[ i ] == 0xA5 &&
						( ( thmfc1->data[ i ] + thmfc1->data[ i + 1 ] +
						thmfc1->data[ i + 2 ] ) & 0xff
							) == thmfc1->data[ i + 3 ] )
					break;
			}

			if ( i >= 0 )
			{
				/* got an id & a data field => write */
				legacy_floppy_image_device * img = thmfc_floppy_image();
				int sector = (int) thmfc1->data[ i + 1 ] * 256 +
					(int) thmfc1->data[ i + 2 ];

				LOG(( "%f $%04x thmfc_floppy_qdd_write_byte: data field, sector=%i\n",
						machine().time().as_double(), m_maincpu->pc(), sector ));

				img->floppy_drive_write_sector_data( 0, sector, thmfc1->data + thmfc1->data_idx, 128, 0 );
				thom_floppy_active( 1 );
			}

			thmfc1->data_idx = 0;

		}
		else
			thmfc1->data_crc += data;
	}
	else
	{
		thmfc1->data_raw_idx++;
		VLOG (( "%f $%04x thmfc_floppy_qdd_write_byte: ignored $%02X\n", machine().time().as_double(), m_maincpu->pc(), data ));
	}

}



/* intelligent writing */
void thomson_state::thmfc_floppy_write_byte( UINT8 data )
{
	VLOG (( "%f $%04x thmfc_floppy_write_byte: off=%i/%i data=$%02X\n",
		machine().time().as_double(), m_maincpu->pc(),
		thmfc1->data_idx, thmfc1->data_size - 1, data ));

	thmfc1->data_raw_size = 0;
	thmfc1->data[ thmfc1->data_idx ] = data;
	if ( thmfc1->data_idx >= thmfc1->data_size - 1 )
		thmfc_floppy_cmd_complete();
	else
		thmfc1->data_idx++;
}

/* intelligent formatting */
void thomson_state::thmfc_floppy_format_byte( UINT8 data )
{
	VLOG (( "%f $%04x thmfc_floppy_format_byte: $%02X\n", machine().time().as_double(), m_maincpu->pc(), data ));

	thmfc1->data_raw_size = 0;

	/* accumulate bytes to form an id field */
	if ( thmfc1->data_idx || data==0xA1 )
	{
		static const UINT8 header[] = { 0xa1, 0xa1, 0xa1, 0xfe };
		thmfc1->data[ thmfc1->data_idx ] = data;
		thmfc1->data_idx++;
		if ( thmfc1->data_idx > 11 )
		{
			if ( !memcmp ( thmfc1->data, header, sizeof( header ) ) )
			{
				/* got id field => format */
				legacy_floppy_image_device * img = thmfc_floppy_image();
				UINT8 track  = thmfc1->data[4];
				UINT8 side   = thmfc1->data[5];
				UINT8 sector = thmfc1->data[6];
				UINT8 length = thmfc1->data[7]; /* actually, log length */
				UINT8 filler = 0xe5;            /* standard Thomson filler */
								chrn_id id;
								if ( thmfc_floppy_find_sector( &id ) )
								{
										img->floppy_drive_format_sector( side, thmfc1->sector_id, track, thmfc1->side, sector, length, filler );
										thom_floppy_active( 1 );
								}
			}

			thmfc1->data_idx = 0;
		}

	}
}



READ8_MEMBER( thomson_state::thmfc_floppy_r )
{
	switch ( offset )
	{
	case 0: /* STAT0 */
		thmfc1->stat0 ^= THMFC1_STAT0_SYNCHRO | THMFC1_STAT0_BYTE_READY_POL;
		VLOG(( "%f $%04x thmfc_floppy_r: STAT0=$%02X\n", machine().time().as_double(), m_maincpu->pc(), thmfc1->stat0 ));
		return thmfc1->stat0;

	case 1: /* STAT1 */
	{
		UINT8 data = 0;
		legacy_floppy_image_device * img = thmfc_floppy_image();
		int flags = img->floppy_drive_get_flag_state(-1 );
		if ( thmfc_floppy_is_qdd(img) )
		{
			if ( ! img->exists() )
				data |= 0x40; /* disk present */
			if ( ! thmfc1->ipl )
				data |= 0x02;       /* disk start */
			data |= 0x08; /* connected */
		}
		else
		{
			if ( thmfc1->ipl )
				data |= 0x40;
			if ( img->exists() )
				data |= 0x20; /* disk change (?) */

			data |= !img->floppy_tk00_r() << 3;

			if ( flags & FLOPPY_DRIVE_READY )
				data |= 0x02;
		}
		if (!motor_on)
			data |= 0x10;
		if (!img->floppy_wpt_r())
			data |= 0x04;
		VLOG(( "%f $%04x thmfc_floppy_r: STAT1=$%02X\n", machine().time().as_double(), m_maincpu->pc(), data ));
		return data;
	}

	case 3: /* RDATA */

		if ( thmfc1->op == THMFC1_OP_READ_SECT || thmfc1->op == THMFC1_OP_READ_ADDR )
			return thmfc_floppy_read_byte();
		else
			return thmfc_floppy_raw_read_byte();

	case 6:
		return 0;

	case 8:
	{
		/* undocumented => emulate TO7 QDD controller ? */
		UINT8 data = thmfc1->ipl << 7;
		VLOG(( "%f $%04x thmfc_floppy_r: STAT8=$%02X\n", machine().time().as_double(), m_maincpu->pc(), data ));
		return data;
	}

	default:
		logerror ( "%f $%04x thmfc_floppy_r: invalid read offset %i\n", machine().time().as_double(), m_maincpu->pc(), offset );
		return 0;
	}
}



WRITE8_MEMBER( thomson_state::thmfc_floppy_w )
{
	switch ( offset ) {
	case 0: /* CMD0 */
	{
		int wsync = (data >> 4) & 1;
		int qdd = thmfc_floppy_is_qdd(thmfc_floppy_image());
		chrn_id id;
		thmfc1->formatting = (data >> 2) & 1;
		LOG (( "%f $%04x thmfc_floppy_w: CMD0=$%02X dens=%s wsync=%i dsync=%i fmt=%i op=%i\n",
				machine().time().as_double(), m_maincpu->pc(), data,
				(BIT(data, 5) ? "FM" : "MFM"),
				wsync, (data >> 3) & 1,
				thmfc1->formatting, data & 3 ));

		/* abort previous command, if any */
		thmfc1->op = THMFC1_OP_RESET;
		thmfc_floppy_cmd->adjust(attotime::never);

		switch ( data & 3 )
		{
		case THMFC1_OP_RESET:
			thmfc1->stat0 = THMFC1_STAT0_FINISHED;
			break;

		case THMFC1_OP_WRITE_SECT:
			if ( qdd )
				logerror( "thmfc_floppy_w: smart operation 1 not supported for QDD\n" );
			else if ( thmfc_floppy_find_sector( &id ) )
			{
								thmfc1->sector_id = id.data_id;
				thmfc1->data_idx = 0;
				thmfc1->data_size = thmfc1->sector_size + 3; /* A1 A1 FB <data> */
				thmfc1->data_finish = thmfc1->sector_size + 3;
				thmfc1->stat0 |= THMFC1_STAT0_BYTE_READY_OP;
				thmfc1->op = THMFC1_OP_WRITE_SECT;
				thmfc_floppy_cmd->adjust(attotime::from_msec( 10 ));
			}
			break;

		case THMFC1_OP_READ_ADDR:
			if ( qdd )
				logerror( "thmfc_floppy_w: smart operation 2 not supported for QDD\n" );
			else if ( thmfc_floppy_find_sector( &id ) )
			{
				thmfc1->data_size =
					thom_floppy_make_addr( id, thmfc1->data, thmfc1->sector_size );
				assert( thmfc1->data_size < sizeof( thmfc1->data ) );
				thmfc1->data_finish = 10;
				thmfc1->data_idx = 1;
				thmfc1->stat0 |= THMFC1_STAT0_BYTE_READY_OP;
				thmfc1->op = THMFC1_OP_READ_ADDR;
				thmfc_floppy_cmd->adjust(attotime::from_msec( 1 ));
			}
			break;

		case THMFC1_OP_READ_SECT:
			if ( qdd )
				logerror( "thmfc_floppy_w: smart operation 3 not supported for QDD\n" );
			else if ( thmfc_floppy_find_sector( &id ) )
			{
				thmfc1->data_size = thom_floppy_make_sector
					( thmfc_floppy_image(), id, thmfc1->data, thmfc1->sector_size );
				assert( thmfc1->data_size < sizeof( thmfc1->data ) );
				thmfc1->data_finish = thmfc1->sector_size + 4;
				thmfc1->data_idx = 1;
				thmfc1->stat0 |= THMFC1_STAT0_BYTE_READY_OP;
				thmfc1->op = THMFC1_OP_READ_SECT;
				thmfc_floppy_cmd->adjust(attotime::from_msec( 10 ));
			}
			break;
		}

		/* synchronize to word, if needed (QDD only) */
		if ( wsync && qdd ) {
			if ( ! thmfc1->data_raw_size )
				thmfc1->data_raw_size = thom_qdd_make_disk ( thmfc_floppy_image(), thmfc1->data );
			while ( thmfc1->data_raw_idx < thmfc1->data_raw_size &&
				thmfc1->data[ thmfc1->data_raw_idx ] != thmfc1->wsync )
			{
				thmfc1->data_raw_idx++;
			}
		}
	}

	break;


	case 1: /* CMD1 */
		thmfc1->data_raw_size = 0;
		thmfc1->sector_size = 128 << ( (data >> 5) & 3);
		thmfc1->side = (data >> 4) & 1;
		if ( thmfc1->sector_size > 256 )
		{
			logerror( "$%04x thmfc_floppy_w: sector size %i > 256 not handled\n",
					m_maincpu->pc(), thmfc1->sector_size );
			thmfc1->sector_size = 256;
		}

		LOG (( "%f $%04x thmfc_floppy_w: CMD1=$%02X sect-size=%i comp=%i head=%i\n",
				machine().time().as_double(), m_maincpu->pc(), data,
				thmfc1->sector_size, (data >> 1) & 7, thmfc1->side ));
		break;


	case 2: /* CMD2 */
	{
		legacy_floppy_image_device * img;
		int seek = 0, motor;
		thmfc1->drive = data & 2;

		img  = thmfc_floppy_image();
		if ( thmfc_floppy_is_qdd(img))
		{
			motor = !(data & 0x40);
			/* no side select & no seek for QDD */
		}
		else
		{
			if ( data & 0x10 )
				seek = (data & 0x20) ? 1 : -1;
			motor =  (data >> 2) & 1;
			thmfc1->drive |= 1 ^ ((data >> 6) & 1);
						img = thmfc_floppy_image();
		}

		thom_floppy_active( 0 );

		LOG (( "%f $%04x thmfc_floppy_w: CMD2=$%02X drv=%i step=%i motor=%i\n",
				machine().time().as_double(), m_maincpu->pc(), data,
				thmfc1->drive, seek, motor ));

		if ( seek )
		{
			thmfc1->data_raw_size = 0;
			img->floppy_drive_seek( seek );
		}

		/* in real life, to keep the motor running, it is sufficient to
		   set motor to 1 every few seconds.
		   instead of counting, we assume the motor is always running...
		*/
		motor_on = CLEAR_LINE /* motor */;
		img->floppy_mon_w(motor_on);
	}
	break;


	case 3: /* WDATA */
		thmfc1->wsync = data;
		if ( thmfc_floppy_is_qdd(thmfc_floppy_image()))
			thmfc_floppy_qdd_write_byte( data );
		else if ( thmfc1->op==THMFC1_OP_WRITE_SECT )
			thmfc_floppy_write_byte( data );
		else if ( thmfc1->formatting )
			thmfc_floppy_format_byte( data );
		else
		{
			/* TODO: implement other forms of raw track writing */
			LOG (( "%f $%04x thmfc_floppy_w: ignored raw WDATA $%02X\n",
					machine().time().as_double(), m_maincpu->pc(), data ));
		}
		break;


	case 4: /* WCLK (unemulated) */
		/* clock configuration: FF for data, 0A for synchro */
		LOG (( "%f $%04x thmfc_floppy_w: WCLK=$%02X (%s)\n",
				machine().time().as_double(), m_maincpu->pc(), data,
				(data == 0xff) ? "data" : (data == 0x0A) ? "synchro" : "?" ));
		break;

	case 5: /* WSECT */
		thmfc1->sector = data;
		LOG (( "%f $%04x thmfc_floppy_w: WSECT=%i\n",
				machine().time().as_double(), m_maincpu->pc(), data ));
		break;

	case 6: /* WTRCK */
		thmfc1->track = data;
		LOG (( "%f $%04x thmfc_floppy_w: WTRCK=%i (real=%i)\n",
				machine().time().as_double(), m_maincpu->pc(), data,
				thmfc_floppy_image()->floppy_drive_get_current_track()));
		break;

	case 7: /* WCELL */
		/* precompensation (unemulated) */
		LOG (( "%f $%04x thmfc_floppy_w: WCELL=$%02X\n",
				machine().time().as_double(), m_maincpu->pc(), data ));
		break;

	default:
		logerror ( "%f $%04x thmfc_floppy_w: invalid write offset %i (data=$%02X)\n",
				machine().time().as_double(), m_maincpu->pc(), offset, data );
	}
}



void thomson_state::thmfc_floppy_reset()
{
	int i;
	LOG(( "thmfc_floppy_reset: THMFC1 controller\n" ));

	for ( i = 0; i < floppy_get_count( machine() ); i++ )
	{
		legacy_floppy_image_device * img = floppy_get_device( machine(), i );
		if (img) {
			img->floppy_drive_set_ready_state( FLOPPY_DRIVE_READY, 0 );
			img->floppy_drive_seek(  - img->floppy_drive_get_current_track() );
		}
	}

	thmfc1->op = THMFC1_OP_RESET;
	thmfc1->track = 0;
	thmfc1->sector = 0;
	thmfc1->side = 0;
	thmfc1->drive = 0;
	thmfc1->sector_size = 256;
	thmfc1->formatting = 0;
	thmfc1->stat0 = 0;
	thmfc1->data_idx = 0;
	thmfc1->data_size = 0;
	thmfc1->data_raw_idx = 0;
	thmfc1->data_raw_size = 0;
	thmfc1->data_crc = 0;
	thmfc1->wsync = 0;
	thmfc_floppy_cmd->adjust(attotime::never);
}



void thomson_state::thmfc_floppy_init()
{
	LOG(( "thmfc_floppy_init: THMFC1 controller\n" ));

	thmfc1 = auto_alloc(machine(), thmfc1_t);
	memset(thmfc1, 0, sizeof(thmfc1_t));

	thmfc_floppy_cmd = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(thomson_state::thmfc_floppy_cmd_complete_cb),this));

	save_item(NAME(thmfc1->op));
	save_item(NAME(thmfc1->sector));
	save_item(NAME(thmfc1->sector_id));
	save_item(NAME(thmfc1->track));
	save_item(NAME(thmfc1->side));
	save_item(NAME(thmfc1->drive));
	save_item(NAME(thmfc1->sector_size));
	save_item(NAME(thmfc1->formatting));
	save_item(NAME(thmfc1->ipl));
	save_item(NAME(thmfc1->data_idx));
	save_item(NAME(thmfc1->data_size));
	save_item(NAME(thmfc1->data_finish));
	save_item(NAME(thmfc1->stat0));
	save_item(NAME(thmfc1->data_raw_idx));
	save_item(NAME(thmfc1->data_raw_size));
	save_item(NAME(thmfc1->data_crc));
	save_item(NAME(thmfc1->wsync));
	save_item(NAME(thmfc1->data));
}



/*********************** Network ************************/

/* The network extension is built as an external floppy controller.
   It uses the same ROM and I/O space, and so, it is natural to have the
   toplevel network emulation here!
*/

/* NOTE: This is work in progress!
   For the moment, only hand-checks works: the TO7 can take the line, then
   perform a DKBOOT request. We do not have the server emulated yet, so,
   no way to answer the request.
*/

TIMER_CALLBACK_MEMBER( thomson_state::ans4 )
{
	LOG(( "%f ans4\n", machine().time().as_double() ));
	m_mc6854->set_cts( 0 );
}

TIMER_CALLBACK_MEMBER( thomson_state::ans3 )
{
	LOG(( "%f ans3\n", machine().time().as_double() ));
	m_mc6854->set_cts( 1 );
	machine().scheduler().timer_set( attotime::from_usec( 100 ), timer_expired_delegate(FUNC(thomson_state::ans4),this));
}

TIMER_CALLBACK_MEMBER( thomson_state::ans2 )
{
	LOG(( "%f ans2\n", machine().time().as_double() ));
	m_mc6854->set_cts( 0 );
	machine().scheduler().timer_set( attotime::from_usec( 100 ), timer_expired_delegate(FUNC(thomson_state::ans3),this));
}

TIMER_CALLBACK_MEMBER( thomson_state::ans )
{
	LOG(( "%f ans\n", machine().time().as_double() ));
	m_mc6854->set_cts( 1 );
	machine().scheduler().timer_set( attotime::from_usec( 100 ), timer_expired_delegate(FUNC(thomson_state::ans2),this));
}
/* consigne DKBOOT

   MO5 BASIC
   $00 $00 $01 $00 $00 $00 $00 $00 $00 $00 $01 $00<$41 $00 $FF $20
   $3D $4C $01 $60 $20 $3C $4F $01 $05 $20 $3F $9C $19 $25 $03 $11
   $93 $15 $10 $25 $32 $8A $7E $FF $E1 $FD $E9 $41>$00 $00 $00 $00
   $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00

   TO7/70 BASIC
   $00 $00 $01 $00 $00 $00 $00 $00 $00 $00 $02 $00<$20 $42 $41 $53
   $49 $43 $20 $4D $49 $43 $52 $4F $53 $4F $46 $54 $20 $31 $2E $30
   $04 $00 $00 $00 $00 $00 $60 $FF $37 $9B $37 $9C>$00 $00 $00 $00
   $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00

   TO7 BASIC
   $00 $00 $01 $00 $00 $00 $00 $00 $00 $00 $00 $00<$20 $42 $41 $53
   $49 $43 $20 $4D $49 $43 $52 $4F $53 $4F $46 $54 $20 $31 $2E $30
   $04 $00 $00 $00 $00 $00 $60 $FF $37 $9B $37 $9C>$00 $00 $00 $00
   $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00

   TO7 LOGO
   $00 $00 $01 $00 $00 $00 $00 $00 $00 $00 $00 $00<$00 $00 $00 $00
   $00 $20 $4C $4F $47 $4F $04 $00 $00 $00 $00 $00 $00 $00 $00 $00
   $00 $00 $00 $00 $00 $00 $AA $FF $01 $16 $00 $C8>$00 $00 $00 $00
   $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00


*/

MC6854_OUT_FRAME_CB(thomson_state::to7_network_got_frame)
{
	LOG(( "%f to7_network_got_frame:", machine().time().as_double() ));
	for ( int i = 0; i < length; i++ )
		LOG(( " $%02X", data[i] ));
	LOG(( "\n" ));

	if ( data[1] == 0xff )
	{
		LOG(( "to7_network_got_frame: %i phones %i\n", data[2], data[0] ));
		machine().scheduler().timer_set( attotime::from_usec( 100 ), timer_expired_delegate(FUNC(thomson_state::ans), this));
		m_mc6854->set_cts( 0 );
	}
	else if ( ! data[1] )
	{
		char name[33];
		memcpy( name, data + 12, 32 );
		name[32] = 0;
		for (int i=0;i<32;i++)
		{
			if ( name[i]<32 || name[i]>=127 )
				name[i]=' ';
		}
		LOG(( "to7_network_got_frame: DKBOOT system=%s appli=\"%s\"\n",
				(data[10] == 0) ? "TO7" : (data[10] == 1) ? "MO5" :
				(data[10] == 2) ? "TO7/70" : "?", name ));
	}
}


void thomson_state::to7_network_init()
{
	LOG(( "to7_network_init: NR 07-005 network extension\n" ));
	logerror( "to7_network_init: network not handled!\n" );
}



void thomson_state::to7_network_reset()
{
	LOG(( "to7_network_reset: NR 07-005 network extension\n" ));
	m_mc6854->set_cts( 0 );
	m_mc6854->set_cts( 1 );
}



READ8_MEMBER( thomson_state::to7_network_r )
{
	if ( offset < 4 )
		return m_mc6854->read( space, offset );

	if ( offset == 8 )
	{
		/* network ID of the computer */
		UINT8 id = m_io_fconfig->read() >> 3;
		VLOG(( "%f $%04x to7_network_r: read id $%02X\n", machine().time().as_double(), m_maincpu->pc(), id ));
		return id;
	}

	logerror( "%f $%04x to7_network_r: invalid read offset %i\n", machine().time().as_double(), m_maincpu->pc(), offset );
	return 0;
}



WRITE8_MEMBER( thomson_state::to7_network_w )
{
	if ( offset < 4 )
		m_mc6854->write( space, offset, data );
	else
	{
		logerror( "%f $%04x to7_network_w: invalid write offset %i (data=$%02X)\n",
				machine().time().as_double(), m_maincpu->pc(), offset, data );
	}
}



/*********************** TO7 dispatch ************************/

/* The TO7, TO7/70, MO5 and MO6 can use a variety of floppy controllers.
   We use a PORT_CONFSETTING to chose the current controller, and dispatch
   here.

   NOTE: you need to reset the computer after changing the floppy controller!

   The CD 90-351 controller seems similar to the THMFC1 gate-array
   => we reuse the THMFC code!
*/



UINT8 to7_controller_type;
UINT8 to7_floppy_bank;


void thomson_state::to7_floppy_init( void* base )
{
	m_flopbank->configure_entries( 0, TO7_NB_FLOP_BANK, base, 0x800 );
	save_item(NAME(to7_controller_type));
	save_item(NAME(to7_floppy_bank));
	to7_5p14sd_init();
	to7_5p14_init();
	to7_qdd_init();
	thmfc_floppy_init();
	to7_network_init();
}



void thomson_state::to7_floppy_reset()
{
	to7_controller_type = (m_io_fconfig->read() ) & 7;

	switch ( to7_controller_type )
	{
	case 1:
		to7_floppy_bank = 1;
		to7_5p14sd_reset();
		break;

	case 2:
		to7_floppy_bank = 2;
		to7_5p14_reset();
		break;

	case 3:
		to7_floppy_bank = 3;
		thmfc_floppy_reset();
		break;

	case 4:
		to7_floppy_bank = 7;
		to7_qdd_reset();
		break;

	case 5:
		to7_floppy_bank = 8;
		to7_network_reset();
		break;

	default:
		to7_floppy_bank = 0;
		break;
	}

	m_flopbank->set_entry( to7_floppy_bank );
}



READ8_MEMBER( thomson_state::to7_floppy_r )
{
	switch ( to7_controller_type )
	{
	case 1:
		return to7_5p14sd_r( space, offset, mem_mask );

	case 2:
		return to7_5p14_r( space, offset, mem_mask );

	case 3:
		return thmfc_floppy_r( space, offset );

	case 4:
		return to7_qdd_r( space, offset, mem_mask );

	case 5:
		return to7_network_r( space, offset, mem_mask );
	}

	return 0;
}



WRITE8_MEMBER( thomson_state::to7_floppy_w )
{
	switch ( to7_controller_type )
	{
	case 1:
		to7_5p14sd_w( space, offset, data, mem_mask );
		return;

	case 2:
		to7_5p14_w( space, offset, data, mem_mask );
		break;

	case 3:
		if ( offset == 8 )
		{
			to7_floppy_bank = 3 + (data & 3);
			m_flopbank->set_entry( to7_floppy_bank );
			VLOG (( "to7_floppy_w: set CD 90-351 ROM bank to %i\n", data & 3 ));
		}
		else
			thmfc_floppy_w( space, offset, data );
		break;

	case 4:
		to7_qdd_w( space, offset, data, mem_mask );
		break;

	case 5:
		to7_network_w( space, offset, data, mem_mask );
		break;
	}
}



/*********************** TO9 ************************/

/* the internal controller is WD2793-based (so, similar to to7_5p14)
   we also emulate external controllers
*/



void thomson_state::to9_floppy_init( void* int_base, void* ext_base )
{
	to7_floppy_init( ext_base );
	m_flopbank->configure_entry( TO7_NB_FLOP_BANK, int_base);
}



void thomson_state::to9_floppy_reset()
{
	to7_floppy_reset();
	if ( THOM_FLOPPY_EXT )
	{
		LOG(( "to9_floppy_reset: external controller\n" ));
	}
	else
	{
		LOG(( "to9_floppy_reset: internal controller\n" ));
		to7_5p14_reset();
		m_flopbank->set_entry( TO7_NB_FLOP_BANK );
	}
}



READ8_MEMBER( thomson_state::to9_floppy_r )
{
	if ( THOM_FLOPPY_EXT )
		return to7_floppy_r( space, offset );
	else
		return  to7_5p14_r( space, offset, mem_mask );
}

WRITE8_MEMBER( thomson_state::to9_floppy_w )
{
	if ( THOM_FLOPPY_EXT )
		to7_floppy_w( space, offset, data );
	else
		to7_5p14_w( space, offset, data, mem_mask );
}

void thomson_state::thomson_index_callback(legacy_floppy_image_device *device, int state)
{
	switch ( to7_controller_type )
	{
	case 1:
		to7_5p14_index_pulse_callback(nullptr, device, state);
		break;

	case 2:
		break;

	case 3:
		thmfc_floppy_index_pulse_cb(nullptr, device, state);
		break;

	case 4:
		to7_qdd_index_pulse_cb(nullptr, device, state);
		break;

	default:
		break;
	}
}
