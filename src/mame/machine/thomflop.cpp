// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit computers

**********************************************************************/

#include "emu.h"
#include "includes/thomson.h"


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


// build an identifier, with header & space
int thomson_legacy_floppy_interface::floppy_make_addr(chrn_id id, uint8_t *dst, int sector_size)
{
	if ( sector_size == 128 )
	{
		// single density
		memset( dst, 0x00, 6 ); // synchro bytes
		dst[  7 ] = 0xfe; // address field mark
		dst[  8 ] = id.C;
		dst[  9 ] = id.H;
		dst[ 10 ] = id.N;
		dst[ 11 ] = id.R;
		dst[ 12 ] = 0; // TODO: CRC
		dst[ 13 ] = 0; // TODO: CRC
		memset( dst + 14, 0xff, 12 ); // end mark
		return 36;
	}
	else
	{
		// double density
		memset( dst, 0xa1, 3 ); // synchro bytes
		dst[ 3 ] = 0xfe; // address field mark
		dst[ 4 ] = id.C;
		dst[ 5 ] = id.H;
		dst[ 6 ] = id.N;
		dst[ 7 ] = id.R;
		dst[ 8 ] = 0; // TODO: CRC
		dst[ 9 ] = 0; // TODO: CRC
		memset( dst + 10, 0xf7, 22 ); // end mark
		return 32;
	}
}



// build a sector, with header & space
int thomson_legacy_floppy_interface::floppy_make_sector(legacy_floppy_image_device *img, chrn_id id, uint8_t *dst, int sector_size)
{
	if ( sector_size == 128 )
	{
		// single density
		memset( dst, 0x00, 6 ); // synchro bytes
		dst[ 6 ] = 0xfb; // data field mark
		img->floppy_drive_read_sector_data
			(  id.H, id.data_id, dst + 7, sector_size );
		dst[ sector_size + 7 ] = 0; // TODO: CRC
		dst[ sector_size + 8 ] = 0; // TODO: CRC
		memset( dst + sector_size + 9, 0xff, 22 ); // end mark
		return sector_size + 31;
	}
	else
	{
		// double density
		memset( dst, 0xa1, 3 ); // synchro bytes
		dst[ 3 ] = 0xfb; // data field mark
		img->floppy_drive_read_sector_data
			(  id.H, id.data_id, dst + 4, sector_size );
		dst[ sector_size + 4 ] = 0; // TODO: CRC
		dst[ sector_size + 5 ] = 0; // TODO: CRC
		memset( dst + sector_size + 6, 0xF7, 74 ); // end mark
		return sector_size + 80;
	}
}



// build a whole track
int thomson_legacy_floppy_interface::floppy_make_track(legacy_floppy_image_device *img, uint8_t *dst, int sector_size, int side)
{
	uint8_t space = ( sector_size == 128 ) ? 0xff : 0;
	uint8_t *org = dst;
	chrn_id id;
	int nb;

	// go to start of track
	while ( ! img->floppy_drive_get_flag_state( FLOPPY_DRIVE_INDEX ) )
	{
		if ( ! img->floppy_drive_get_next_id( side, &id ) )
			return 0;
	}

	// for each sector...
	for ( nb = 0; nb < 16; nb++ )
	{
		if ( ! img->floppy_drive_get_next_id( side, &id ) )
			break;

		memset( dst, space, THOM_SIZE_SYNCHRO ); dst += THOM_SIZE_SYNCHRO;
		dst += floppy_make_addr( id, dst, sector_size );
		memset( dst, space, THOM_SIZE_SYNCHRO ); dst += THOM_SIZE_SYNCHRO;
		dst += floppy_make_sector( img, id, dst, sector_size );

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


// build an identifier, with header
int thomson_legacy_floppy_interface::qdd_make_addr(int sector, uint8_t *dst)
{
	dst[  0 ] = 0xa5;
	dst[  1 ] = sector >> 8;
	dst[  2 ] = sector & 0xff;
	dst[  3 ] = dst[ 0 ] + dst[ 1 ] + dst[ 2 ];
	return 4;
}



// build a sector, with header
int thomson_legacy_floppy_interface::qdd_make_sector(legacy_floppy_image_device *img, int sector, uint8_t *dst)
{
	int i;
	dst[ 0 ] = 0x5a;
	img->floppy_drive_read_sector_data (  0, sector, dst + 1, 128 );
	dst[ 129 ] = 0;
	for ( i = 0; i < 129; i++ )
		dst[ 129 ] += dst[ i ];
	return 130;
}



// build a whole disk
int thomson_legacy_floppy_interface::qdd_make_disk(legacy_floppy_image_device *img, uint8_t *dst)
{
	uint8_t* org = dst;
	int i;

	memset( dst, 0x16, THOM_QDD_SYNCH_DISK ); dst += THOM_QDD_SYNCH_DISK;

	for ( i = 1; i <= 400; i++ )
	{
		memset( dst, 0x16, THOM_QDD_SYNCH_ADDR ); dst += THOM_QDD_SYNCH_ADDR;
		dst += qdd_make_addr( i, dst );
		memset( dst, 0x16, THOM_QDD_SYNCH_DATA ); dst += THOM_QDD_SYNCH_DATA;
		dst += qdd_make_sector( img, i, dst );
	}

	memset( dst, 0x16, THOM_QDD_SYNCH_DISK ); dst += THOM_QDD_SYNCH_DISK;

	return dst - org;
}



/*********************** CD 90-640 controller ************************/

/* 5''1/4 two-sided double-density
   used in TO7, TO7/70, MO5 computers
   based on a WD2793 (or lower ?) chip
*/



uint8_t thomson_state::to7_5p14_r(offs_t offset)
{
	if ( offset < 4 )
		return m_wd2793_fdc->read(offset);
	else if ( offset == 8 )
		return m_to7_5p14_select;
	else
		logerror ( "%s: to7_5p14_r: invalid read offset %i\n", machine().describe_context(), offset );
	return 0;
}



void thomson_state::to7_5p14_w(offs_t offset, uint8_t data)
{
	if ( offset < 4 )
		m_wd2793_fdc->write(offset, data);
	else if ( offset == 8 )
	{
		// drive select
		floppy_image_device *floppy = nullptr;

		if (BIT(data, 1)) floppy = m_wd2793_fdc->subdevice<floppy_connector>("0")->get_device();
		if (BIT(data, 2)) floppy = m_wd2793_fdc->subdevice<floppy_connector>("1")->get_device();

		m_wd2793_fdc->set_floppy(floppy);

		if (floppy)
		{
			thom_floppy_active( 0 );
			floppy->mon_w(0);
			floppy->ss_w(BIT(data, 0));
		}
	}
	else
	{
		logerror("%s: to7_5p14_w: invalid write offset %i (data=$%02X)\n",
				machine().describe_context(), offset, data );
	}
}



void thomson_state::to7_5p14_reset()
{
	logerror("%s: to7_5p14_reset: CD 90-640 controller\n", machine().describe_context());
	m_wd2793_fdc->reset();
}



void thomson_state::to7_5p14_init()
{
	logerror("%s: to7_5p14_init: CD 90-640 controller\n", machine().describe_context());
	save_item(NAME(m_to7_5p14_select));
}



/*********************** CD 90-015 controller ************************/

/* 5''1/4 one-sided single-density (up to 4 one-sided drives)
   used in TO7, TO7/70, MO5 computers
   based on HD 46503 S chip, but we actually use a MC 6843 instead
   (seems they are clone)
*/



uint8_t thomson_state::to7_5p14sd_r(offs_t offset)
{
	if ( offset < 8 )
		return m_mc6843->read(offset);
	else if ( offset >= 8 && offset <= 9 )
		return m_to7_5p14sd_select;
	else
		logerror ( "%f $%04x to7_5p14sd_r: invalid read offset %i\n",  machine().time().as_double(), m_maincpu->pc(), offset );
	return 0;
}



void thomson_state::to7_5p14sd_w(offs_t offset, uint8_t data)
{
	if ( offset < 8 )
		m_mc6843->write(offset, data);
	else if ( offset >= 8 && offset <= 9 )
	{
		// drive select
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

		m_to7_5p14sd_select = data;

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

void thomson_state::to7_5p14_index_pulse_callback( int state )
{
	m_mc6843->set_index_pulse( state );
}

void thomson_state::to7_5p14sd_reset()
{
	LOG(( "to7_5p14sd_reset: CD 90-015 controller\n" ));
	for (auto &img : m_floppy_image)
	{
		if (img.found())
		{
			img->floppy_drive_set_ready_state( FLOPPY_DRIVE_READY, 0 );
			img->floppy_drive_set_rpm( 300. );
			img->floppy_drive_seek( - img->floppy_drive_get_current_track() );
		}
	}
}


void thomson_state::to7_5p14sd_init()
{
	LOG(( "to7_5p14sd_init: CD 90-015 controller\n" ));
	save_item(NAME(m_to7_5p14sd_select));
}


/*********************** CQ 90-028 controller ************************/

/* QDD 2''8 controller
   used in TO7, TO7/70, MO5 computers
   it is based on a MC6852 SSDA

   Note: the MC6852 is only partially emulated, most features are not used in
   the controller and are ignored.
*/



// MC6852 status
enum : uint8_t
{
	QDD_S_RDA  = 0x01, // receiver data available
	QDD_S_TDRA = 0x02, // transitter data register available
	QDD_S_NDCD = 0x04, // data carrier detect, negated (unused)
	QDD_S_NCTS = 0x08, // clear-to-send, negated write-protect
	QDD_S_TUF  = 0x10, // transmitter underflow (unused)
	QDD_S_OVR  = 0x20, // receiver overrun (unused)
	QDD_S_PE   = 0x40, // receiver parity error (unused)
	QDD_S_IRQ  = 0x80, // interrupt request
	QDD_S_ERR  = (QDD_S_TUF | QDD_S_OVR | QDD_S_PE | QDD_S_NDCD | QDD_S_NCTS)
};


// MC6852 control
enum : uint8_t
{
	QDD_C1_RIE       = 0x20, // interrupt on reveive
	QDD_C1_TIE       = 0x10, // interrupt on transmit
	QDD_C1_CLRSYNC   = 0x08, // clear receiver sync char (unused)
	QDD_C1_STRIPSYNC = 0x04, // strips sync from received (unused)
	QDD_C1_TRESET    = 0x02, // transmitter reset
	QDD_C1_RRESET    = 0x01, // receiver reset
	QDD_C2_EIE       = 0x80, // interrupt on error
	QDD_C2_TSYNC     = 0x40, // underflow = ff if 0 / sync if 1 (unused)
	QDD_C2_BLEN      = 0x04, // transfer byte length (unused)
	QDD_C3_CLRTUF    = 0x08, // clear underflow
	QDD_C3_CLRCTS    = 0x04, // clear CTS
	QDD_C3_SYNCLEN   = 0x02, // sync byte length (unused)
	QDD_C3_SYNCMODE  = 0x01  // external / internal sync mode (unused)
};


// a track is actually the whole disk = 400 128-byte sectors + headers
#define QDD_MAXBUF ( THOM_QDD_SIZE_ID + THOM_QDD_SIZE_DATA ) * 512


DEFINE_DEVICE_TYPE(CQ90_028, cq90_028_device, "cq90_028", "Thomson CQ 90-028 Quick Disk Controller")

cq90_028_device::cq90_028_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CQ90_028, tag, owner, clock)
	, thomson_legacy_floppy_interface(mconfig, *this)
	, m_qdd_image(*this, "^floppy0")
{
}


void cq90_028_device::index_pulse_cb(int state)
{
	m_index_pulse = state;

	if ( state )
	{
		// rewind to disk start
		m_data_idx = 0;
		m_start_idx = 0;
		m_data_size = 0;
	}

	VLOG(( "%f to7_qdd_pulse_cb: state=%i\n", machine().time().as_double(), state ));
}


// update MC6852 status register
void cq90_028_device::stat_update()
{
	// byte-ready
	m_status |= QDD_S_RDA | QDD_S_TDRA;
	if ( !m_drive )
		m_status |= QDD_S_PE;

	// write-protect
	if (m_qdd_image->floppy_wpt_r() == CLEAR_LINE)
		m_status |= QDD_S_NCTS;

	// sticky reset conditions
	if ( m_ctrl1 & QDD_C1_RRESET )
		m_status &= ~(QDD_S_PE | QDD_S_RDA | QDD_S_OVR);
	if ( m_ctrl1 & QDD_C1_TRESET )
		m_status &= ~(QDD_S_TDRA | QDD_S_TUF);

	// irq update
	if ( ( (m_ctrl1 & QDD_C1_RIE) && !(m_status & QDD_S_RDA ) ) ||
			( (m_ctrl1 & QDD_C1_TIE) && !(m_status & QDD_S_TDRA) ) ||
			( (m_ctrl2 & QDD_C2_EIE) && !(m_status & QDD_S_ERR ) ) )
		m_status &= ~QDD_S_IRQ;

	if ( ( (m_ctrl1 & QDD_C1_RIE) && (m_status & QDD_S_RDA ) ) ||
			( (m_ctrl1 & QDD_C1_TIE) && (m_status & QDD_S_TDRA) ) ||
			( (m_ctrl2 & QDD_C2_EIE) && (m_status & QDD_S_ERR ) ) )
		m_status |= QDD_S_IRQ;
}



uint8_t cq90_028_device::qdd_read_byte()
{
	uint8_t data;

	// rebuild disk if needed
	if ( !m_data_size )
	{
		m_data_size = qdd_make_disk( m_qdd_image.target(), &m_data[0] );
		assert( m_data_idx < QDD_MAXBUF );
	}

	if ( m_data_idx >= m_data_size )
		data = 0;
	else
		data = m_data[ m_data_idx ];

	VLOG(( "%f %s to7_qdd_read_byte: RDATA off=%i/%i data=$%02X\n",
			machine().time().as_double(), machine().describe_context(),
			m_data_idx, m_data_size, data ));

	m_data_idx++;
	m_start_idx = m_data_idx;

	return data;
}



/* This is quite complex: bytes are written one at a time by the CPU and we
   must detect the following patterns:
   * CPU write id field and data field => format
   * CPU write data field after it has read an id field => sector write
   */
void cq90_028_device::qdd_write_byte(uint8_t data)
{
	// rebuild disk if needed
	if ( !m_data_size )
	{
		m_data_size = qdd_make_disk( m_qdd_image.target(), &m_data[0] );
		assert( m_data_idx < QDD_MAXBUF );
	}

	if ( ( m_start_idx != m_data_idx || // field in construction
			data==0xA5 || data==0x5A ) &&    // first byte of tentative field
			m_data_idx < m_data_size )
	{
		// this is the first byte of the field
		if ( m_start_idx == m_data_idx )
			m_data_crc = 0;

		// accumulate bytes
		m_data[ m_data_idx ] = data;
		m_data_idx++;

		VLOG (( "%f %s to7_qdd_write_byte: got $%02X offs=%i-%i\n",
			machine().time().as_double(), machine().describe_context(), data,
			m_start_idx, m_data_idx ));

		// end of tentative id field
		if ( m_data_idx == m_start_idx + 4 &&
				m_data[ m_start_idx ] == 0xA5 &&
				m_data[ m_start_idx + 3 ] == m_data_crc )
		{
			// got an id field => format
			int sector = (int) m_data[ m_start_idx + 1 ] * 256 + (int) m_data[ m_start_idx + 2 ];
			uint8_t filler = 0xff;

			LOG(( "%f %s to7_qdd_write_byte: got id field for sector=%i\n",
					machine().time().as_double(), machine().describe_context(), sector ));

			m_qdd_image->floppy_drive_format_sector(
							0, sector, 0, 0, sector, 128, filler );
			//thom_floppy_active( 1 );
			m_start_idx = m_data_idx;
		}

		// end of tentative data field
		else if ( m_data_idx == m_start_idx + 130 &&
				m_data[ m_start_idx  ] == 0x5A &&
				m_data[ m_start_idx + 129 ] == m_data_crc )
		{
			int i;

			// look backwards for previous id field
			for ( i = m_start_idx - 3; i >= 0; i-- )
			{
				if ( m_data[ i ] == 0xA5 &&
						( ( m_data[ i ] + m_data[ i + 1 ] +
						m_data[ i + 2 ] ) & 0xff
							) == m_data[ i + 3 ] )
					break;
			}

			if ( i >= 0 )
			{
				// got an id & a data field => write
				int sector = (int) m_data[ i + 1 ] * 256 + (int) m_data[ i + 2 ];

				LOG(( "%f %s to7_qdd_write_byte: goto data field for sector=%i\n",
						machine().time().as_double(), machine().describe_context(), sector ));

				m_qdd_image->floppy_drive_write_sector_data( 0, sector, &m_data[ m_start_idx + 1 ], 128, 0 );
				//thom_floppy_active( 1 );
			}

			m_start_idx = m_data_idx;
		}

		else m_data_crc += data;
	}
}



uint8_t cq90_028_device::qdd_r(offs_t offset)
{
	switch ( offset )
	{
	case 0: // MC6852 status
		stat_update();
		VLOG(( "%f %s to7_qdd_r: STAT=$%02X irq=%i pe=%i ovr=%i und=%i tr=%i rd=%i ncts=%i\n",
				machine().time().as_double(), machine().describe_context(), m_status,
				m_status & QDD_S_IRQ  ? 1 : 0,
				m_status & QDD_S_PE   ? 1 : 0,
				m_status & QDD_S_OVR  ? 1 : 0,
				m_status & QDD_S_TUF  ? 1 : 0,
				m_status & QDD_S_TDRA ? 1 : 0,
				m_status & QDD_S_RDA  ? 1 : 0,
				m_status & QDD_S_NCTS ? 1 : 0 ));
		return m_status;

	case 1: // MC6852 data input => read byte from disk
		m_status &= ~(QDD_S_RDA | QDD_S_PE | QDD_S_OVR);
		stat_update();
		return qdd_read_byte();

	case 8: // floppy status
	{
		uint8_t data = 0;
		device_image_interface* img = m_qdd_image.target();
		if ( ! img->exists() )
			data |= 0x40; // disk present
		if ( m_index_pulse )
			data |= 0x80; // disk start
		VLOG(( "%f %s to7_qdd_r: STATUS8 $%02X\n", machine().time().as_double(), machine().describe_context(), data ));
		return data;
	}

	default:
		logerror ( "%f %s to7_qdd_r: invalid read offset %i\n", machine().time().as_double(), machine().describe_context(), offset );
		return 0;
	}
}



void cq90_028_device::qdd_w(offs_t offset, uint8_t data)
{
	switch ( offset )
	{
	case 0: // MC6852 control 1
		// reset
		if ( data & QDD_C1_RRESET )
			m_status &= ~(QDD_S_PE | QDD_S_RDA | QDD_S_OVR);
		if ( data & QDD_C1_TRESET )
			m_status &= ~(QDD_S_TDRA | QDD_S_TUF);

		m_ctrl1 = ( data & ~(QDD_C1_RRESET | QDD_C1_TRESET) ) |( data &  (QDD_C1_RRESET | QDD_C1_TRESET) & m_ctrl1 );
		stat_update();
		VLOG(( "%f %s to7_qdd_w: CTRL1=$%02X reset=%c%c %s%sirq=%c%c\n",
				machine().time().as_double(), machine().describe_context(), data,
				data & QDD_C1_RRESET ? 'r' : '-', data & QDD_C1_TRESET ? 't' : '-',
				data & QDD_C1_STRIPSYNC ? "strip-sync " : "",
				data & QDD_C1_CLRSYNC ? "clear-sync " : "",
				data & QDD_C1_RIE ? 'r' : '-',
				data & QDD_C1_TIE ? 't' : '-' ));
		break;

	case 1:
		switch ( m_ctrl1 >> 6 )
		{
		case 0: // MC6852 control 2
		{
#if 0
			// most of these are unused now
			static const int bit[8] = { 6, 6, 7, 8, 7, 7, 8, 8 };
			static const int par[8] = { 2, 1, 0, 0, 2, 1, 2, 1 };
			static const char *const parname[3] = { "none", "odd", "even" };
			int bits, parity;
			bits   = bit[ (data >> 3) & 7 ];
			parity = par[ (data >> 3) & 7 ];
			stat_update();
			VLOG(( "%f %s to7_qdd_w: CTRL2=$%02X bits=%i par=%s blen=%i under=%s%s\n",
					machine().time().as_double(), machine().describe_context(), data,
					bits, parname[ parity ], data & QDD_C2_BLEN ? 1 : 2,
					data & QDD_C2_TSYNC ? "sync" : "ff",
					data & QDD_C2_EIE ? "irq-err" : "" ));
#endif
			m_ctrl2 = data;
			break;
		}

		case 1: // MC6852 control 3
			m_ctrl3 = data;
			// reset just once each write, not sticky
			if ( data & QDD_C3_CLRTUF )
				m_status &= ~QDD_S_TUF;
			if ( data & QDD_C3_CLRCTS )
				m_status &= ~QDD_S_NCTS;
			stat_update();
			VLOG(( "%f %s to7_qdd_w: CTRL3=$%02X %s%ssync-len=%i sync-mode=%s\n",
					machine().time().as_double(), machine().describe_context(), data,
					data & QDD_C3_CLRTUF ? "clr-tuf " : "",
					data & QDD_C3_CLRCTS ? "clr-cts " : "",
					data & QDD_C3_SYNCLEN ? 1 : 2,
					data & QDD_C3_SYNCMODE ? "ext" : "int" ));
			break;

		case 2: // MC6852 sync code => write byte to disk
			qdd_write_byte( data );
			break;

		case 3: // MC6852 data out => does not seem to be used
			VLOG(( "%f %s to7_qdd_w: ignored WDATA=$%02X\n", machine().time().as_double(), machine().describe_context(), data ));
			break;

		}
		break;

	case 8: // set drive
		m_drive = data;
		VLOG(( "%f %s to7_qdd_w: DRIVE=$%02X\n", machine().time().as_double(), machine().describe_context(), data ));
		break;

	case 12: // motor pulse ?
		//thom_floppy_active( 0 );
		VLOG(( "%f %s to7_qdd_w: MOTOR=$%02X\n", machine().time().as_double(), machine().describe_context(), data ));
		break;

	default:
		logerror ( "%f %s to7_qdd_w: invalid write offset %i (data=$%02X)\n", machine().time().as_double(), machine().describe_context(), offset, data );
	}
}



void cq90_028_device::qdd_reset()
{
	LOG(( "to7_qdd_reset: CQ 90-028 controller\n" ));

	m_qdd_image->floppy_drive_set_ready_state(  FLOPPY_DRIVE_READY, 0 );

	m_qdd_image->floppy_mon_w(CLEAR_LINE);

	// pulse each time the whole-disk spiraling track ends
	// at 90us per byte read, the disk can be read in 6s
	m_qdd_image->floppy_drive_set_rpm( 60. / 6. );

	m_ctrl1 |= QDD_C1_TRESET | QDD_C1_RRESET; // reset
	m_ctrl2 &= 0x7c; // clear EIE, PC2-PC1
	m_ctrl3 &= 0xfe; // internal sync
	m_drive = 0;
	stat_update();
}



void cq90_028_device::device_start()
{
	m_data = make_unique_clear<uint8_t[]>(QDD_MAXBUF);

	save_item(NAME(m_status));
	save_item(NAME(m_ctrl1));
	save_item(NAME(m_ctrl2));
	save_item(NAME(m_ctrl3));
	save_item(NAME(m_drive));
	save_item(NAME(m_data_idx));
	save_item(NAME(m_start_idx));
	save_item(NAME(m_data_size));
	save_item(NAME(m_data_crc));
	save_item(NAME(m_index_pulse));
	save_pointer(NAME(m_data), QDD_MAXBUF);
}



/********************** THMFC1 controller *************************/

/* custom Thomson Gate-array for 3''1/2, 5''1/4 (double density only) & QDD
 */


// STAT0 flags
enum : uint8_t
{
	STAT0_SYNCHRO        = 0x01, // bit clock synchronized
	STAT0_BYTE_READY_OP  = 0x02, // byte ready (high-level operation)
	STAT0_CRC_ERROR      = 0x04,
	STAT0_FINISHED       = 0x08,
	STAT0_FINISHING      = 0x10, // (unemulated)
	STAT0_BYTE_READY_POL = 0x80  // polling mode
};

/*#define THOM_MAXBUF (THOM_SIZE_ID+THOM_SIZE_DATA_HI+2*THOM_SIZE_SYNCHRO)*17*/
#define THOM_MAXBUF ( THOM_QDD_SIZE_ID + THOM_QDD_SIZE_DATA ) * 512


DEFINE_DEVICE_TYPE(THMFC1, thmfc1_device, "thmfc1", "Thomson THMFC1 floppy controller")

ALLOW_SAVE_TYPE(thmfc1_device::thmfc1_op);

thmfc1_device::thmfc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, THMFC1, tag, owner, clock)
	, thomson_legacy_floppy_interface(mconfig, *this)
	, m_floppy_image(*this, "^floppy%u", 0U)
	, m_floppy_active_cb(*this)
	, m_op(OP_RESET)
	, m_sector(0)
	, m_sector_id(0)
	, m_track(0)
	, m_side(0)
	, m_drive(0)
	, m_sector_size(0)
	, m_formatting(0)
	, m_ipl(0)
	, m_wsync(0)
	, m_motor_on(0)
	, m_data_idx(0)
	, m_data_size(0)
	, m_data_finish(0)
	, m_data_raw_idx(0)
	, m_data_raw_size(0)
	, m_data_crc(0)
	, m_stat0(0)
	, m_floppy_cmd(nullptr)
{
}


void thmfc1_device::device_resolve_objects()
{
	m_floppy_active_cb.resolve_safe();
}


legacy_floppy_image_device *thmfc1_device::get_floppy_image()
{
	return m_floppy_image[m_drive].target();
}



bool thmfc1_device::floppy_is_qdd( legacy_floppy_image_device *image ) const
{
	if (!image) return false;
	if (!image->exists()) return false;
	return image->length() == 51200; // idf QDD
}



void thmfc1_device::index_pulse_cb( int index, int state )
{
	legacy_floppy_image_device *const image = m_floppy_image[index];

	if ( image != get_floppy_image())
		return;

	if ( floppy_is_qdd(image) )
	{
		// pulse each time the whole-disk spiraling track ends
		image->floppy_drive_set_rpm( 16.92f /* 423/25 */ );
		m_ipl = state;
		if ( state )
		{
			m_data_raw_size = 0;
			m_data_raw_idx = 0;
			m_data_idx = 0;
		}
	}
	else
	{
		image->floppy_drive_set_rpm( 300. );
		m_ipl = state;
		if ( state  )
			m_data_raw_idx = 0;
	}

	VLOG(( "%f thmfc_floppy_index_pulse_cb: state=%i\n", machine().time().as_double(), state ));
}



int thmfc1_device::floppy_find_sector( chrn_id* dst )
{
	legacy_floppy_image_device *const img = get_floppy_image();
	chrn_id id;
	int r = 0;

	// scan track, try 4 revolutions
	while ( r < 4 )
	{
		if ( img->floppy_drive_get_next_id( m_side, &id ) )
		{
			if ( id.C == m_track &&
					id.R == m_sector &&
					(128 << id.N) == m_sector_size
					/* check side ?  id.H == m_side */ )
			{
				if ( dst )
					memcpy( dst, &id, sizeof( chrn_id ) );
				m_stat0 = STAT0_BYTE_READY_POL;
				LOG (( "thmfc_floppy_find_sector: sector found C=%i H=%i R=%i N=%i\n", id.C, id.H, id.R, id.N ));
				return 1;
			}
		}

		if ( img->floppy_drive_get_flag_state( FLOPPY_DRIVE_INDEX ) )
			r++;
	}

	m_stat0 = STAT0_CRC_ERROR | STAT0_FINISHED;
	LOG (( "thmfc_floppy_find_sector: sector not found drive=%s track=%i sector=%i\n", img->tag(), m_track, m_sector ));
	return 0;
}



// complete command (by read, write, or timeout)
void thmfc1_device::floppy_cmd_complete()
{
	LOG (( "%f thmfc_floppy_cmd_complete_cb: cmd=%i off=%i/%i/%i\n",
			machine().time().as_double(), m_op, m_data_idx,
			m_data_finish - 1, m_data_size - 1 ));

	if ( m_op == OP_WRITE_SECT )
	{
		legacy_floppy_image_device *const img = get_floppy_image();
		img->floppy_drive_write_sector_data( m_side, m_sector_id, &m_data[3], m_data_size - 3, 0 );
		m_floppy_active_cb( 1 );
	}
	m_op = OP_RESET;
	m_stat0 |= STAT0_FINISHED;
	m_data_idx = 0;
	m_data_size = 0;
	m_floppy_cmd->adjust(attotime::never);
}



TIMER_CALLBACK_MEMBER( thmfc1_device::floppy_cmd_complete_cb )
{
	floppy_cmd_complete();
}



// intelligent read: show just one field, skip header
uint8_t thmfc1_device::floppy_read_byte()
{
	uint8_t data = m_data[ m_data_idx ];

	VLOG(( "%f %s thmfc_floppy_read_byte: off=%i/%i/%i data=$%02X\n",
			machine().time().as_double(), machine().describe_context(),
			m_data_idx, m_data_finish - 1, m_data_size - 1,
			data ));

	if ( m_data_idx >= m_data_size - 1 )
		floppy_cmd_complete();
	else
		m_data_idx++;

	if ( m_data_idx >= m_data_finish )
		m_stat0 |= STAT0_FINISHED;

	return data;
}



// dumb read: show whole track with field headers and gaps
uint8_t thmfc1_device::floppy_raw_read_byte()
{
	uint8_t data;

	// rebuild track if needed
	if ( ! m_data_raw_size )
	{
		if ( floppy_is_qdd(get_floppy_image()))
			// QDD: track = whole disk
			m_data_raw_size = qdd_make_disk ( get_floppy_image(), &m_data[0] );
		else
		{
			m_data_raw_idx = 0;
			m_data_raw_size = floppy_make_track( get_floppy_image(), &m_data[0],
									m_sector_size, m_side );
		}
		assert( m_data_raw_size < THOM_MAXBUF );
	}

	if ( m_data_raw_idx >= m_data_raw_size )
		data = 0;
	else
		data = m_data[ m_data_raw_idx ];

	VLOG(( "%f %s thmfc_floppy_raw_read_byte: off=%i/%i data=$%02X\n",
			machine().time().as_double(), machine().describe_context(),
			m_data_raw_idx, m_data_raw_size, data ));

	m_data_raw_idx++;

	return data;
}



// QDD writing / formating
void thmfc1_device::floppy_qdd_write_byte( uint8_t data )
{
	int i;

	if ( m_formatting &&
			( m_data_idx || data==0xA5 || data==0x5A ) &&
			m_data_raw_idx < THOM_MAXBUF )
	{
		if ( ! m_data_raw_size )
		{
			m_data_raw_size = qdd_make_disk ( get_floppy_image(), &m_data[0] );
			assert( m_data_raw_size < THOM_MAXBUF );
		}

		// accumulate bytes to form a field
		m_data[ m_data_raw_idx ] = data;
		m_data_raw_idx++;

		if ( ! m_data_idx  )
		{
			// start
			m_data_crc = 0;
			m_data_idx = m_data_raw_idx;
		}

		VLOG (( "%f %s thmfc_floppy_qdd_write_byte: $%02X offs=%i-%i\n",
			machine().time().as_double(), machine().describe_context(), data,
			m_data_idx, m_data_raw_idx ));

		if ( m_data_raw_idx == m_data_idx + 3 &&
				m_data[ m_data_idx - 1 ] == 0xA5 &&
				m_data[ m_data_idx + 2 ] == m_data_crc )
		{
			// got an id field => format
			int sector = (int) m_data[ m_data_idx ] * 256 + (int) m_data[ m_data_idx + 1 ];
			uint8_t filler = 0xff;

			LOG(( "%f %s thmfc_floppy_qdd_write_byte: id field, sector=%i\n", machine().time().as_double(), machine().describe_context(), sector ));

			get_floppy_image()->floppy_drive_format_sector( 0, sector, 0, 0, sector, 128, filler );
			m_floppy_active_cb( 1 );
			m_data_idx = 0;
		}

		else if ( m_data_raw_idx == m_data_idx + 129 &&
				m_data[ m_data_idx -   1 ] == 0x5A &&
				m_data[ m_data_idx + 128 ] == m_data_crc )
		{
			// look backwards for previous id field
			for ( i = m_data_idx - 4; i >= 0; i-- )
			{
				if ( m_data[ i ] == 0xA5 &&
						( ( m_data[ i ] + m_data[ i + 1 ] +
						m_data[ i + 2 ] ) & 0xff
							) == m_data[ i + 3 ] )
					break;
			}

			if ( i >= 0 )
			{
				// got an id & a data field => write
				legacy_floppy_image_device * img = get_floppy_image();
				int sector = (int) m_data[ i + 1 ] * 256 +
					(int) m_data[ i + 2 ];

				LOG(( "%f %s thmfc_floppy_qdd_write_byte: data field, sector=%i\n",
						machine().time().as_double(), machine().describe_context(), sector ));

				img->floppy_drive_write_sector_data( 0, sector, &m_data[m_data_idx], 128, 0 );
				m_floppy_active_cb( 1 );
			}

			m_data_idx = 0;

		}
		else
			m_data_crc += data;
	}
	else
	{
		m_data_raw_idx++;
		VLOG (( "%f %s thmfc_floppy_qdd_write_byte: ignored $%02X\n", machine().time().as_double(), machine().describe_context(), data ));
	}

}



// intelligent writing
void thmfc1_device::floppy_write_byte( uint8_t data )
{
	VLOG (( "%f %s thmfc_floppy_write_byte: off=%i/%i data=$%02X\n",
		machine().time().as_double(), machine().describe_context(),
		m_data_idx, m_data_size - 1, data ));

	m_data_raw_size = 0;
	m_data[ m_data_idx ] = data;
	if ( m_data_idx >= m_data_size - 1 )
		floppy_cmd_complete();
	else
		m_data_idx++;
}

// intelligent formatting
void thmfc1_device::floppy_format_byte( uint8_t data )
{
	VLOG (( "%f %s thmfc_floppy_format_byte: $%02X\n", machine().time().as_double(), machine().describe_context(), data ));

	m_data_raw_size = 0;

	// accumulate bytes to form an id field
	if ( m_data_idx || data==0xA1 )
	{
		static const uint8_t header[] = { 0xa1, 0xa1, 0xa1, 0xfe };
		m_data[ m_data_idx ] = data;
		m_data_idx++;
		if ( m_data_idx > 11 )
		{
			if ( !memcmp ( &m_data[0], header, sizeof( header ) ) )
			{
				// got id field => format
				legacy_floppy_image_device * img = get_floppy_image();
				uint8_t track  = m_data[4];
				uint8_t side   = m_data[5];
				uint8_t sector = m_data[6];
				uint8_t length = m_data[7]; // actually, log length
				uint8_t filler = 0xe5;            // standard Thomson filler

				chrn_id id;
				if ( floppy_find_sector( &id ) )
				{
					img->floppy_drive_format_sector( side, m_sector_id, track, m_side, sector, length, filler );
					m_floppy_active_cb( 1 );
				}
			}

			m_data_idx = 0;
		}

	}
}



uint8_t thmfc1_device::floppy_r(offs_t offset)
{
	switch ( offset )
	{
	case 0: // STAT0
		m_stat0 ^= STAT0_SYNCHRO | STAT0_BYTE_READY_POL;
		VLOG(( "%f %s thmfc_floppy_r: STAT0=$%02X\n", machine().time().as_double(), machine().describe_context(), m_stat0 ));
		return m_stat0;

	case 1: // STAT1
	{
		uint8_t data = 0;
		legacy_floppy_image_device * img = get_floppy_image();
		int flags = img->floppy_drive_get_flag_state(-1 );
		if ( floppy_is_qdd(img) )
		{
			if ( ! img->exists() )
				data |= 0x40; // disk present
			if ( ! m_ipl )
				data |= 0x02;       // disk start
			data |= 0x08; // connected
		}
		else
		{
			if ( m_ipl )
				data |= 0x40;
			if ( img->exists() )
				data |= 0x20; // disk change (?)

			data |= !img->floppy_tk00_r() << 3;

			if ( flags & FLOPPY_DRIVE_READY )
				data |= 0x02;
		}
		if (!m_motor_on)
			data |= 0x10;
		if (!img->floppy_wpt_r())
			data |= 0x04;
		VLOG(( "%f %s thmfc_floppy_r: STAT1=$%02X\n", machine().time().as_double(), machine().describe_context(), data ));
		return data;
	}

	case 3: // RDATA

		if ( m_op == OP_READ_SECT || m_op == OP_READ_ADDR )
			return floppy_read_byte();
		else
			return floppy_raw_read_byte();

	case 6:
		return 0;

	case 8:
	{
		// undocumented => emulate TO7 QDD controller ?
		uint8_t data = m_ipl << 7;
		VLOG(( "%f %s thmfc_floppy_r: STAT8=$%02X\n", machine().time().as_double(), machine().describe_context(), data ));
		return data;
	}

	default:
		logerror ( "%f %s thmfc_floppy_r: invalid read offset %i\n", machine().time().as_double(), machine().describe_context(), offset );
		return 0;
	}
}



void thmfc1_device::floppy_w(offs_t offset, uint8_t data)
{
	switch ( offset ) {
	case 0: // CMD0
	{
		int wsync = (data >> 4) & 1;
		int qdd = floppy_is_qdd(get_floppy_image());
		chrn_id id;
		m_formatting = (data >> 2) & 1;
		LOG (( "%f %s thmfc_floppy_w: CMD0=$%02X dens=%s wsync=%i dsync=%i fmt=%i op=%i\n",
				machine().time().as_double(), machine().describe_context(), data,
				(BIT(data, 5) ? "FM" : "MFM"),
				wsync, (data >> 3) & 1,
				m_formatting, data & 3 ));

		// abort previous command, if any
		m_op = OP_RESET;
		m_floppy_cmd->adjust(attotime::never);

		switch ( data & 3 )
		{
		case OP_RESET:
			m_stat0 = STAT0_FINISHED;
			break;

		case OP_WRITE_SECT:
			if ( qdd )
				logerror( "thmfc_floppy_w: smart operation 1 not supported for QDD\n" );
			else if ( floppy_find_sector( &id ) )
			{
				m_sector_id = id.data_id;
				m_data_idx = 0;
				m_data_size = m_sector_size + 3; // A1 A1 FB <data>
				m_data_finish = m_sector_size + 3;
				m_stat0 |= STAT0_BYTE_READY_OP;
				m_op = OP_WRITE_SECT;
				m_floppy_cmd->adjust(attotime::from_msec( 10 ));
			}
			break;

		case OP_READ_ADDR:
			if ( qdd )
				logerror( "thmfc_floppy_w: smart operation 2 not supported for QDD\n" );
			else if ( floppy_find_sector( &id ) )
			{
				m_data_size =
					floppy_make_addr( id, &m_data[0], m_sector_size );
				assert( m_data_size < THOM_MAXBUF );
				m_data_finish = 10;
				m_data_idx = 1;
				m_stat0 |= STAT0_BYTE_READY_OP;
				m_op = OP_READ_ADDR;
				m_floppy_cmd->adjust(attotime::from_msec( 1 ));
			}
			break;

		case OP_READ_SECT:
			if ( qdd )
				logerror( "thmfc_floppy_w: smart operation 3 not supported for QDD\n" );
			else if ( floppy_find_sector( &id ) )
			{
				m_data_size = floppy_make_sector
					( get_floppy_image(), id, &m_data[0], m_sector_size );
				assert( m_data_size < THOM_MAXBUF );
				m_data_finish = m_sector_size + 4;
				m_data_idx = 1;
				m_stat0 |= STAT0_BYTE_READY_OP;
				m_op = OP_READ_SECT;
				m_floppy_cmd->adjust(attotime::from_msec( 10 ));
			}
			break;
		}

		// synchronize to word, if needed (QDD only)
		if ( wsync && qdd ) {
			if ( ! m_data_raw_size )
				m_data_raw_size = qdd_make_disk ( get_floppy_image(), &m_data[0] );
			while ( m_data_raw_idx < m_data_raw_size &&
				m_data[ m_data_raw_idx ] != m_wsync )
			{
				m_data_raw_idx++;
			}
		}
	}

	break;


	case 1: // CMD1
		m_data_raw_size = 0;
		m_sector_size = 128 << ( (data >> 5) & 3);
		m_side = (data >> 4) & 1;
		if ( m_sector_size > 256 )
		{
			logerror( "%s thmfc_floppy_w: sector size %i > 256 not handled\n",
					machine().describe_context(), m_sector_size );
			m_sector_size = 256;
		}

		LOG (( "%f %s thmfc_floppy_w: CMD1=$%02X sect-size=%i comp=%i head=%i\n",
				machine().time().as_double(), machine().describe_context(), data,
				m_sector_size, (data >> 1) & 7, m_side ));
		break;


	case 2: // CMD2
	{
		legacy_floppy_image_device * img;
		int seek = 0, motor;
		m_drive = data & 2;

		img  = get_floppy_image();
		if ( floppy_is_qdd(img))
		{
			motor = !(data & 0x40);
			// no side select & no seek for QDD
		}
		else
		{
			if ( data & 0x10 )
				seek = (data & 0x20) ? 1 : -1;
			motor =  (data >> 2) & 1;
			m_drive |= 1 ^ ((data >> 6) & 1);
			img = get_floppy_image();
		}

		m_floppy_active_cb( 0 );

		LOG (( "%f %s thmfc_floppy_w: CMD2=$%02X drv=%i step=%i motor=%i\n",
				machine().time().as_double(), machine().describe_context(), data,
				m_drive, seek, motor ));

		if ( seek )
		{
			m_data_raw_size = 0;
			img->floppy_drive_seek( seek );
		}

		/* in real life, to keep the motor running, it is sufficient to
		   set motor to 1 every few seconds.
		   instead of counting, we assume the motor is always running...
		*/
		m_motor_on = CLEAR_LINE /* motor */;
		img->floppy_mon_w(m_motor_on);
	}
	break;


	case 3: // WDATA
		m_wsync = data;
		if ( floppy_is_qdd(get_floppy_image()))
			floppy_qdd_write_byte( data );
		else if ( m_op == OP_WRITE_SECT )
			floppy_write_byte( data );
		else if ( m_formatting )
			floppy_format_byte( data );
		else
		{
			// TODO: implement other forms of raw track writing
			LOG (( "%f %s thmfc_floppy_w: ignored raw WDATA $%02X\n",
					machine().time().as_double(), machine().describe_context(), data ));
		}
		break;


	case 4: // WCLK (unemulated)
		// clock configuration: FF for data, 0A for synchro
		LOG (( "%f %s thmfc_floppy_w: WCLK=$%02X (%s)\n",
				machine().time().as_double(), machine().describe_context(), data,
				(data == 0xff) ? "data" : (data == 0x0A) ? "synchro" : "?" ));
		break;

	case 5: // WSECT
		m_sector = data;
		LOG (( "%f %s thmfc_floppy_w: WSECT=%i\n",
				machine().time().as_double(), machine().describe_context(), data ));
		break;

	case 6: // WTRCK
		m_track = data;
		LOG (( "%f %s thmfc_floppy_w: WTRCK=%i (real=%i)\n",
				machine().time().as_double(), machine().describe_context(), data,
				get_floppy_image()->floppy_drive_get_current_track()));
		break;

	case 7: // WCELL
		// precompensation (unemulated)
		LOG (( "%f %s thmfc_floppy_w: WCELL=$%02X\n",
				machine().time().as_double(), machine().describe_context(), data ));
		break;

	default:
		logerror ( "%f %s thmfc_floppy_w: invalid write offset %i (data=$%02X)\n",
				machine().time().as_double(), machine().describe_context(), offset, data );
	}
}



void thmfc1_device::floppy_reset()
{
	LOG(( "thmfc_floppy_reset: THMFC1 controller\n" ));

	for (auto &img : m_floppy_image)
	{
		if (img.found())
		{
			img->floppy_drive_set_ready_state( FLOPPY_DRIVE_READY, 0 );
			img->floppy_drive_seek(  - img->floppy_drive_get_current_track() );
		}
	}

	m_op = OP_RESET;
	m_track = 0;
	m_sector = 0;
	m_side = 0;
	m_drive = 0;
	m_sector_size = 256;
	m_formatting = 0;
	m_stat0 = 0;
	m_data_idx = 0;
	m_data_size = 0;
	m_data_raw_idx = 0;
	m_data_raw_size = 0;
	m_data_crc = 0;
	m_wsync = 0;
	m_motor_on = 0;
	m_floppy_cmd->adjust(attotime::never);
}



void thmfc1_device::device_start()
{
	m_data = make_unique_clear<uint8_t[]>(THOM_MAXBUF);

	m_floppy_cmd = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(thmfc1_device::floppy_cmd_complete_cb), this));

	save_item(NAME(m_op));
	save_item(NAME(m_sector));
	save_item(NAME(m_sector_id));
	save_item(NAME(m_track));
	save_item(NAME(m_side));
	save_item(NAME(m_drive));
	save_item(NAME(m_sector_size));
	save_item(NAME(m_formatting));
	save_item(NAME(m_ipl));
	save_item(NAME(m_data_idx));
	save_item(NAME(m_data_size));
	save_item(NAME(m_data_finish));
	save_item(NAME(m_stat0));
	save_item(NAME(m_data_raw_idx));
	save_item(NAME(m_data_raw_size));
	save_item(NAME(m_data_crc));
	save_item(NAME(m_wsync));
	save_item(NAME(m_motor_on));
	save_pointer(NAME(m_data), THOM_MAXBUF);
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

void thomson_state::to7_network_got_frame(uint8_t *data, int length)
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



uint8_t thomson_state::to7_network_r(offs_t offset)
{
	if ( offset < 4 )
		return m_mc6854->read(offset);

	if ( offset == 8 )
	{
		// network ID of the computer
		uint8_t id = m_io_fconfig->read() >> 3;
		VLOG(( "%f $%04x to7_network_r: read id $%02X\n", machine().time().as_double(), m_maincpu->pc(), id ));
		return id;
	}

	logerror( "%f $%04x to7_network_r: invalid read offset %i\n", machine().time().as_double(), m_maincpu->pc(), offset );
	return 0;
}



void thomson_state::to7_network_w(offs_t offset, uint8_t data)
{
	if ( offset < 4 )
		m_mc6854->write(offset, data);
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



void thomson_state::to7_floppy_init()
{
	m_flopbank->configure_entry( 0, memregion("floppy_none")->base() );
	m_flopbank->configure_entry( 1, memregion("floppy_cd90_015")->base() );
	m_flopbank->configure_entry( 2, memregion("floppy_cd90_640")->base() );
	m_flopbank->configure_entries( 3, 4, memregion("floppy_cd90_351")->base(), 0x800 );
	m_flopbank->configure_entry( 7, memregion("floppy_cq90_028")->base() );
	m_flopbank->configure_entry( 8, memregion("floppy_nano")->base() );
	save_item(NAME(m_to7_controller_type));
	save_item(NAME(m_to7_floppy_bank));
	to7_5p14sd_init();
	to7_5p14_init();
	to7_network_init();
}



void thomson_state::to7_floppy_reset()
{
	m_to7_controller_type = (m_io_fconfig->read() ) & 7;

	switch ( m_to7_controller_type )
	{
	case 1:
		m_to7_floppy_bank = 1;
		to7_5p14sd_reset();
		break;

	case 2:
		m_to7_floppy_bank = 2;
		to7_5p14_reset();
		break;

	case 3:
		m_to7_floppy_bank = 3;
		m_thmfc->floppy_reset();
		break;

	case 4:
		m_to7_floppy_bank = 7;
		m_to7qdd->qdd_reset();
		break;

	case 5:
		m_to7_floppy_bank = 8;
		to7_network_reset();
		break;

	default:
		m_to7_floppy_bank = 0;
		break;
	}

	m_flopbank->set_entry( m_to7_floppy_bank );
}



uint8_t thomson_state::to7_floppy_r(offs_t offset)
{
	switch (m_to7_controller_type)
	{
	case 1:
		return to7_5p14sd_r(offset);

	case 2:
		return to7_5p14_r(offset);

	case 3:
		return m_thmfc->floppy_r(offset);

	case 4:
		return m_to7qdd->qdd_r(offset);

	case 5:
		return to7_network_r(offset);
	}

	return 0;
}



void thomson_state::to7_floppy_w(offs_t offset, uint8_t data)
{
	switch (m_to7_controller_type)
	{
	case 1:
		to7_5p14sd_w(offset, data);
		return;

	case 2:
		to7_5p14_w(offset, data);
		break;

	case 3:
		if (offset == 8)
		{
			m_to7_floppy_bank = 3 + (data & 3);
			m_flopbank->set_entry(m_to7_floppy_bank);
			VLOG (( "to7_floppy_w: set CD 90-351 ROM bank to %i\n", data & 3 ));
		}
		else
			m_thmfc->floppy_w(offset, data);
		break;

	case 4:
		m_to7qdd->qdd_w(offset, data);
		break;

	case 5:
		to7_network_w(offset, data);
		break;
	}
}



/*********************** TO9 ************************/

/* the internal controller is WD2793-based (so, similar to to7_5p14)
   we also emulate external controllers
*/



void thomson_state::to9_floppy_init( void* int_base )
{
	to7_floppy_init();
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



uint8_t thomson_state::to9_floppy_r(offs_t offset)
{
	if ( THOM_FLOPPY_EXT )
		return to7_floppy_r( offset );
	else
		return to7_5p14_r( offset );
}

void thomson_state::to9_floppy_w(offs_t offset, uint8_t data)
{
	if ( THOM_FLOPPY_EXT )
		to7_floppy_w( offset, data );
	else
		to7_5p14_w( offset, data );
}

void thomson_state::thomson_index_callback(int index, int state)
{
	switch ( m_to7_controller_type )
	{
	case 1:
		to7_5p14_index_pulse_callback(state);
		break;

	case 2:
		break;

	case 3:
		m_thmfc->index_pulse_cb(index, state);
		break;

	case 4:
		m_to7qdd->index_pulse_cb(state);
		break;

	default:
		break;
	}
}
