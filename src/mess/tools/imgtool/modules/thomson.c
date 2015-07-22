// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/****************************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit micro-computers.

  Handles SAP, QD and FD floppy formats with a BASIC-DOS filesystem
  (most floppies except some games and demo).

*****************************************************************************/

/* TODO:
   - improve two-sided floppy support
   - check & correct BASIC read filter
   - implement BASIC write filter
   - handle accented letters
 */


/* Thomson floppy geometry are:
   - 5"1/4 single density: 40 tracks, 16 sectors/track, 128 bytes/sector
   - 5"1/4 double density: 40 tracks, 16 sectors/track, 256 bytes/sector
   - 3"1/2 double density: 80 tracks, 16 sectors/track, 256 bytes/sector
   - 2"8: 25 logical tracks, 16 logical sectors/track, 128 bytes/sector

   Notes:

   - 2"8 QDD actually contain a single spiraling track, with 400 128-byte
   sectors; however, the filesystem considers it has 25 virtual tracks of
   16 sectors => we do the same in imgtool (but thomflop.c uses a 400-sector
   addressing to emulate the device hardware)

   - tracks always have 16 sectors, this is requred by the filesystem

   - two-sided floppies are handled as two one-sided floppies in two
   logical floppy drives on the original Thomsons; each has its own
   independent FAT and directory.
   In imgtool, we handle two-sided images as two partitions.
   For now, only 3"1/2 .fd and QDD .qd files can be two-sided.

   - it seems possible to create 3"1/2 single density floppies on the TO9,
   I am not sure this is standard, and we do not support it...

*/


/* Microsoft BASIC Filesystem.

   The floppy is divided into blocks, with always two blocks / track.
   Also, in double density, only the first 255 bytes of each 256-byte
   sector is used. Thus, blocks are 1024 bytes in single density, and
   2040 bytes in double density.
   Depending on the number of tracks, there can be 50, 80 or 160 blocks.

   Filesystem data always occupy track 20, whatever the floppy geometry.

   * sector 1:
     - bytes 0-7: floppy name
     - remaining bytes: undocumented, seem unused

   * sector 2: FAT, 1 byte info per block
     - byte 0: always 0
     - byte 1: block 0
        ...
     - byte i: block i-1
        ...
     - byte 160: block 159
     - remaining bytes: undocumented, seem unused

     The byte has the following meaning:
     - ff: block is free
     - fb: block is reserved
     - 00-bf: block allocated, points to then next file block
     - c1-c8: last block in file, (value & 15) is the number of sectors
              actually allocated to the file

     Note: in single density, we can reference only 127 blocks. This is not
     a problem because single density floppies can only have 50 or 80 blocks,
     never 160.

   * sector 3-16: directory

     no subdirectories

     each file entry occupies 32 bytes:
     - bytes 00-07: filename, padded with whitespaces
     - bytes 08-0A: file extension, padedd with whitespaces
     - byte  0B:    file type
                    . 0 = BASIC, ASCII or binary program (B)
            . 1 = BASIC or ASCII data (D)
            . 2 = machine code (M)
            . 3 = ASCII assembler file (A)
     - byte  0C:    format flag
            . 00 = binary (tokenized)
                    . ff = ASCII
     - byte  0D:    first block
     - bytes 0E-0F: bytes used in the last sector
     - bytes 10-17: comment
     - byte 18:     day (of month)
     - byte 19:     month
     - byte 1A:     year (two last digits)
     - bytes 1B-1f: unknown

     Note that byte 0 has a special significance:
     - 00:    free directory entry
     - 20-7F: actual directory entry, value is the first character ini filename
     - ff:    end of directory

     There can be 56 files in single density, 112 in double density.
 */

#include "imgtool.h"
#include "iflopimg.h"

#include <time.h>


#define MAXSIZE 80*16*256*2 /* room for two faces, double-density, 80 tracks */

struct thom_floppy {
	imgtool_stream *stream;

	UINT16 sector_size;   /* 128 or 256 */
	UINT16 sectuse_size;  /* bytes used in sector: 128 or 255 */
	UINT8  tracks;        /* 25, 40, or 80 */
	UINT8  heads;         /* 1 or 2 */
	UINT8  data[MAXSIZE]; /* image data */

	int    modified;      /* data need to be copied back to image file */

};


enum thom_dirent_type {
	THOM_DIRENT_END,
	THOM_DIRENT_FREE,
	THOM_DIRENT_FILE,
	THOM_DIRENT_INVALID,

};


struct thom_dirent {
	thom_dirent_type type;
	int    index;

	char   name[9];
	char   ext[4];
	char   comment[9];
	UINT8  ftype;
	UINT8  format;
	UINT8  firstblock;
	UINT16 lastsectsize;
	UINT8  day;
	UINT8  month;
	UINT8  year;

};


static void thom_basic_get_info(const imgtool_class *clas, UINT32 param,
				union imgtoolinfo *info);

static UINT8* thom_get_sector(thom_floppy* f, unsigned head,
					unsigned track, unsigned sector);


/*********************** .fd / .qd format ************************/

/* .fd and .qd formats are very simple: the sectors are simply put one after
   another, starting at sector 1 of track 0 to sector 16 track 0, then
   starting again at sector 1 of track 1, and so on.
   There is no image or sector header, and no gap between sectors.
   The number of tracks and sector size are determined by the extension
   (.fd have 40 or 80 tracks, .qd have 25 tracks) and the file size.
*/

static imgtoolerr_t thom_open_fd_qd(imgtool_image *img, imgtool_stream *stream)
{
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	int size = stream_size( stream );

	f->stream = stream;
	f->modified = 0;

	/* guess format */
	switch ( size ) {
	case 81920:
	f->tracks = 40;
	f->sector_size = 128;
	f->sectuse_size = 128;
	f->heads = 1;
	break;

	case 163840:
	f->tracks = 40;
	f->sector_size = 256;
	f->sectuse_size = 255;
	f->heads = 1;
	/* could also be: sector_size=128, heads=2 */
	/* maight even be: tracks=80, sector_size=128 */
	break;

	case 327680:
	f->tracks = 80;
	f->sector_size = 256;
	f->sectuse_size = 255;
	f->heads = 1;
	/* could also be: tracks=40, heads=2 */
	break;

	case 655360:
	f->tracks = 80;
	f->sector_size = 256;
	f->sectuse_size = 255;
	f->heads = 2;
	break;

	case 51200:
	f->tracks = 25;
	f->sector_size = 128;
	f->sectuse_size = 128;
	f->heads = 1;
	break;

	case 62400:
	f->tracks = 25;
	f->sector_size = 128;
	f->sectuse_size = 128;
	f->heads = 2;
	break;

	default:
	return IMGTOOLERR_CORRUPTIMAGE;
	}

	assert( size == f->heads * f->tracks * 16 * f->sector_size );

	stream_seek( stream, 0, SEEK_SET );
	if ( stream_read( stream, f->data, size ) < size )
	return IMGTOOLERR_READERROR;

	return IMGTOOLERR_SUCCESS;
}

static void thom_close_fd_qd(imgtool_image *img)
{
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );

	/* save image */
	if ( f->modified ) {
	int size = f->heads * f->tracks * 16 * f->sector_size;
	stream_seek( f->stream, 0, SEEK_SET );
	if ( stream_write( f->stream, f->data, size ) < size )
	{
		/* logerror( "thom_diskimage_close_fd_qd: write error\n" ); */
	}
	}

	stream_close( f->stream );
}


/*********************** .sap format ************************/

/* SAP format, Copyright 1998 by Alexandre Pukall

   image is composed of:
   - byte 0:        version (0 or 1)
   - bytes 1..65:   signature
   - bytes 66..end: sectors

   each sector is composed of
   - a 4-byte header:
     . byte 0:     format
                      0 3"1/2 double density
                      1 5"1/4 low density
     . byte 1:     protection (?)
     . byte 2:     track index, in 0..79
     . byte 3:     sector index, in 1..16
   - sector data, XOR 0xB3
   - 2-byte CRC of header + data, high byte first

   there is no provision for two-sided images in sap
 */

static const int sap_magic_num = 0xB3; /* simple XOR crypt */

static const char sap_header[]=
	"\001SYSTEME D'ARCHIVAGE PUKALL S.A.P. "
	"(c) Alexandre PUKALL Avril 1998";

static const UINT16 sap_crc[] = {
	0x0000, 0x1081, 0x2102, 0x3183,   0x4204, 0x5285, 0x6306, 0x7387,
	0x8408, 0x9489, 0xa50a, 0xb58b,   0xc60c, 0xd68d, 0xe70e, 0xf78f,
};

static UINT16 thom_sap_crc( UINT8* data, int size )
{
	int i;
	UINT16 crc = 0xffff, crc2;
	for ( i = 0; i < size; i++ ) {
	crc2 = ( crc >> 4 ) ^ sap_crc[ ( crc ^ data[i] ) & 15 ];
	crc = ( crc2 >> 4 ) ^ sap_crc[ ( crc2 ^ (data[i] >> 4) ) & 15 ];
	}
	return crc;
}

static imgtoolerr_t thom_open_sap(imgtool_image *img, imgtool_stream *stream)
{
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	UINT8 buf[262];

	f->stream = stream;
	f->modified = 0;

	/* check image header */
	stream_seek( stream, 0, SEEK_SET );
	stream_read( stream, buf, 66 );
	if ( memcmp( buf+1, sap_header+1, 65 ) ) return IMGTOOLERR_CORRUPTIMAGE;

	/* guess format */
	stream_read( stream, buf, 1 );
	switch ( buf[0] ) {
	case 1:
	case 3:
	f->heads = 1;
	f->tracks = 40;
	f->sector_size = 128;
	f->sectuse_size = 128;
	break;
	case 0:
	case 2:
	case 4:
	f->heads = 1;
	f->tracks = 80;
	f->sector_size = 256;
	f->sectuse_size = 255;
	break;
	default: return IMGTOOLERR_CORRUPTIMAGE;
	}

	stream_seek( stream, 66, SEEK_SET );
	while ( 1) {
	int i, sector, track;
	UINT16 crc;

	/* load sector */
	if ( stream_read( stream, buf, 6 + f->sector_size ) < 6 + f->sector_size )
		break;

	/* parse sector header */
	track = buf[2];
	sector = buf[3];
	if ( sector < 1 || sector > 16 || track >= f->tracks )
		return IMGTOOLERR_CORRUPTIMAGE;

	/* decrypt */
	for ( i = 0; i < f->sector_size; i++ ) buf[ i + 4 ] ^= sap_magic_num;
	memcpy( thom_get_sector( f, 0, track, sector ), buf + 4, f->sector_size );

	/* check CRC */
	crc = thom_sap_crc( buf, f->sector_size + 4 );
	if ( ( (crc >> 8)   != buf[ f->sector_size + 4 ] ) ||
			( (crc & 0xff) != buf[ f->sector_size + 5 ] ) )
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	return IMGTOOLERR_SUCCESS;
}

static void thom_close_sap(imgtool_image *img)
{
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );

	if ( f->modified ) {
	int i, sector, track;
	UINT8 buf[262];
	UINT16 crc;

	/* rewind */
	stream_seek( f->stream, 0, SEEK_SET );

	/* image header */
	if ( stream_write( f->stream, sap_header, 66 ) < 66 ) {
		/* logerror( "thom_diskimage_close_sap: write error\n" ); */
		return;
	}

	for ( track = 0; track < f->tracks; track++ )
		for ( sector = 1; sector <= 16; sector++ ) {
	/* sector header & data */
	buf[0] = ( f->tracks == 80 ) ? 2 : 1;
	buf[1] = 0;
	buf[2] = track;
	buf[3] = sector;
	memcpy( buf + 4,
		thom_get_sector( f, 0, track, sector), f->sector_size );

	/* compute crc */
	crc = thom_sap_crc( buf, f->sector_size + 4 );
	buf[ f->sector_size + 4 ] = crc >> 8;
	buf[ f->sector_size + 5 ] = crc & 0xff;

	/* crypt */
	for ( i = 0; i < f->sector_size; i++ ) buf[ i + 4 ] ^= sap_magic_num;

	/* save */
	if ( stream_write( f->stream, buf, f->sector_size + 6 ) <
			f->sector_size + 6 ) {
		/* logerror( "thom_diskimage_close_sap: write error\n" ); */
		return;
	}
		}
	}

	stream_close( f->stream );
}


/*********************** low-level functions *********************/

static UINT8* thom_get_sector(thom_floppy* f, unsigned head,
					unsigned track, unsigned sector)
{
	assert( head < f->heads);
	assert( track < f->tracks );
	assert( sector > 0 && sector <= 16 );
	return & f->data[ ( (head * f->tracks + track) * 16 + (sector-1) ) *
			f->sector_size ];
}

static int thom_nb_blocks(thom_floppy* f)
{
	int n1 = f->tracks * 2;      /* 2 blocks per track */
	int n2 = f->sector_size - 1; /* FAT entries */
	return (n1 < n2) ? n1 : n2;
}

static int thom_max_dirent(thom_floppy* f)
{
	return 14 * f->sector_size / 32;
}

/* remove trailing spaces */
static void thom_stringify(char* str)
{
	char* s = str + strlen(str) - 1;
	while ( s >= str && *s == ' ' ) { *s = 0; s--; }
	for ( s = str; *s; s++ )
	if ( (UINT8)*s == 0xff ) *s = 0;
	else if ( *s < ' ' || *s >= 127 ) *s = '?';
}

/* pad with spaces until the length is nb */
static void thom_unstringity(char* str, int nb)
{
	char* s = str;
	while ( s < str + nb && *s ) s++;
	while ( s < str + nb ) { *s = ' '; s++; }
}

/* get filename and extension & truncate them to 8 + 3 */
static void thom_conv_filename(const char* s, char name[9], char ext[4])
{
	int i;
	memset( name, 0, 9 );
	memset( ext, 0, 4 );
	for ( i = 0; i < 8 && *s && *s != '.'; i++, s++ ) name[i] = *s;
	while ( *s && *s != '.' ) s++;
	if ( *s == '.' ) {
	s++;
	for ( i = 0; i < 3 && *s; i++, s++ ) ext[i] = *s;
	}
}

static time_t thom_crack_time(thom_dirent* d)
{
	struct tm t;
	time_t now;

	/* check */
	if ( d->day < 1 || d->day > 31 || d->month < 1 || d->month > 12 ) return 0;

	/* converts */
	time( &now );
	t = *localtime( &now );
	t.tm_sec = 0;
	t.tm_min = 0;
	t.tm_hour = 0;
	t.tm_mday = d->day;
	t.tm_mon = d->month - 1;
	t.tm_year = (d->year < 65 ) ? d->year + 100 : d->year;
	return mktime(&t);
}

static void thom_make_time(thom_dirent* d, time_t time)
{
	if ( ! time ) {
	d->day = d->month = d->year = 0;
	}
	else {
	struct tm t = *localtime( &time );
	d->day = t.tm_mday;
	d->month = t.tm_mon + 1;
	d->year = t.tm_year % 100;
	}
}

static void thom_make_time_now(thom_dirent* d)
{
	time_t now;
	time( &now );
	thom_make_time( d, now );
}

static void thom_get_dirent(thom_floppy* f, unsigned head,
				unsigned n, thom_dirent* d)
{
	UINT8* base = thom_get_sector( f, head, 20, 3 ) + n * 32;
	memset( d, 0, sizeof(*d) );
	d->index = n;
	if ( n >= thom_max_dirent( f ) )
	d->type = THOM_DIRENT_END;
	else if ( *base == 0xff )
	d->type = THOM_DIRENT_END;
	else if ( *base == 0 )
	d->type = THOM_DIRENT_FREE;
	else {
	int i;
	/* fill regular entry */
	d->type = THOM_DIRENT_FILE;
	memcpy( d->name, base, 8 );
	memcpy( d->ext, base + 8, 3 );
	memcpy( d->comment, base + 16, 8 );
	thom_stringify( d->name );
	thom_stringify( d->ext );
	thom_stringify( d->comment );
	d->ftype = base[11];
	d->format = base[12];
	d->firstblock = base[13];
	d->lastsectsize = ((int)base[14] << 8) + base[15];
	d->day = base[24];
	d->month = base[25];
	d->year = base[26];
	/* sanity check */
	for ( i = 0; i < 11; i++ )
		if ( base[i] < ' ' || base[i] > 127 ) d->type = THOM_DIRENT_INVALID;
	}
}

static void thom_set_dirent(thom_floppy* f, unsigned head,
				unsigned n, thom_dirent* d)
{
	UINT8* base = thom_get_sector( f, head, 20, 3 ) + n * 32;
	if ( n >= thom_max_dirent( f ) ) return;
	memset( base, 0, 32 );
	if ( d->type == THOM_DIRENT_END ) base[ 0 ] = 0xff;
	else if ( d->type == THOM_DIRENT_FREE ) base[ 0 ] = 0;
	else {
	memcpy( base, d->name, 8 );
	memcpy( base + 8, d->ext, 3 );
	memcpy( base + 16, d->comment, 8 );
	thom_unstringity( (char*)base, 8 );
	thom_unstringity( (char*)base + 8, 3 );
	if ( base[16] ) thom_unstringity( (char*)base + 16, 8 );
	base[11] = d->ftype;
	base[12] = d->format;
	base[13] = d->firstblock;
	base[14] = d->lastsectsize >> 8;
	base[15] = d->lastsectsize & 0xff;
	base[24] = d->day;
	base[25] = d->month;
	base[26] = d->year;
	}
	f->modified = 1;
}

/* returns 1 if file found, 0 if not */
static int thom_find_dirent(thom_floppy* f, unsigned head,
				const char* name, thom_dirent* d)
{
	int n = 0;
	while ( 1 ) {
	thom_get_dirent( f, head, n, d );
	if ( d->type == THOM_DIRENT_END ) return 0;
	if ( d->type == THOM_DIRENT_FILE ) {
		char buf[13];
		sprintf( buf, "%s.%s", d->name, d->ext );
		if ( ! strcmp( buf, name ) ) return 1;
	}
	n++;
	}
}

/* returns 1 if free entry found, 0 if none available */
static int thom_find_free_dirent(thom_floppy* f, unsigned head, thom_dirent* d)
{
	int n = 0;
	while ( 1 ) {
	thom_get_dirent( f, head, n, d );
	if ( d->type == THOM_DIRENT_FREE ) return 1;
	if ( d->type == THOM_DIRENT_END ) break;
	n++;
	}
	if ( n + 1 >= thom_max_dirent( f ) ) return 0;
	thom_set_dirent( f, head, n+1, d );
	d->type = THOM_DIRENT_FREE;
	return 1;
}

/* returns the file size in bytes, or -1 if error */
static int thom_get_file_size(thom_floppy* f, unsigned head, thom_dirent* d)
{
	UINT8* fat = thom_get_sector( f, head, 20, 2 );
	int nbblocks = thom_nb_blocks(f);
	int block = d->firstblock;
	int size = 0;
	int timeout = nbblocks;
	if ( d->type != THOM_DIRENT_FILE ) return -1;
	if ( block >= nbblocks ) return -1;
	block = fat[ block + 1 ];
	while ( 1 ) {
	if ( block < nbblocks ) {
		/* full block */
		size += 8 * f->sectuse_size;
		block = fat[ block + 1 ];
	}
	else if ( block >= 0xc1 && block <= 0xc8 ) {
		/* last block in file */
		size += (block-0xc1) * f->sectuse_size;
		size += d->lastsectsize;
		return size;
	}
	else return -1;
	timeout--;
	if ( timeout < 0 ) return -1;
	}
}

/* number of blocks used by file */
static int thom_get_file_blocks(thom_floppy* f, unsigned head, thom_dirent* d)
{
	UINT8* fat = thom_get_sector( f, head, 20, 2 );
	int nbblocks = thom_nb_blocks(f);
	int block = d->firstblock;
	int nb = 0;
	if ( d->type != THOM_DIRENT_FILE ) return 0;
	if ( block >= nbblocks ) return 0;
	block = fat[ block + 1 ];
	while ( 1 ) {
	if ( block < nbblocks ) {
		/* full block */
		nb++;
		block = fat[ block + 1 ];
	}
	else if ( block >= 0xc1 && block <= 0xc8 ) {
		/* last block in file */
		nb++;
		return nb;
	}
	else return nb;
	}
}

/* number of free blocks */
static int thom_get_free_blocks(thom_floppy* f, unsigned head)
{
	UINT8* fat = thom_get_sector( f, head, 20, 2 );
	int nbblocks = thom_nb_blocks(f);
	int i, nb = 0;
	for ( i = 1; i <= nbblocks; i++ )
	if ( fat[i] == 0xff ) nb++;
	return nb;
}

/* dump file contents into dst */
static void thom_get_file(thom_floppy* f, unsigned head,
				thom_dirent* d, imgtool_stream *dst)
{
	UINT8* fat = thom_get_sector( f, head, 20, 2 );
	int nbblocks = thom_nb_blocks(f);
	int block = d->firstblock;
	if ( block >= nbblocks ) return;
	while ( 1 ) {
	int nextblock = fat[ block + 1 ];
	int track = block / 2;
	int firstsect = (block % 2) ? 9 : 1;
	if ( nextblock < nbblocks ) {
		/* full block */
		int i;
		for ( i = 0; i < 8; i++ ) {
	UINT8* data = thom_get_sector( f, head, track, firstsect + i );
	stream_write( dst, data, f->sectuse_size );
		}
		block = fat[ block + 1 ];
	}
	else if ( nextblock >= 0xc1 && nextblock <= 0xc8 ) {
		/* last block in file */
		int i;
		UINT8* data;
		for ( i = 0; i < nextblock - 0xc1; i++ ) {
	data = thom_get_sector( f, head, track, firstsect + i );
	stream_write( dst, data, f->sectuse_size );
		}
		data = thom_get_sector( f, head, track, firstsect + i );
		stream_write( dst, data, d->lastsectsize );
		return;
	}
	else {
		/* invalid, assume last block */
		UINT8* data = thom_get_sector( f, head, track, firstsect );
		stream_write( dst, data, d->lastsectsize );
		return;
	}
	block = nextblock;
	}
}

static void thom_del_file(thom_floppy* f, unsigned head, thom_dirent* d)
{
	UINT8* fat = thom_get_sector( f, head, 20, 2 );
	int nbblocks = thom_nb_blocks(f);
	int block = d->firstblock;
	if ( d->type != THOM_DIRENT_FILE ) return;
	if ( block >= nbblocks ) return;
	while ( 1 ) {
	int nextblock = fat[ block + 1 ];
	fat[ block ] = 0xff;
	if ( nextblock < nbblocks ) block = fat[ block + 1 ];
	else break;
	}
	d->type = THOM_DIRENT_FREE;
	thom_set_dirent( f, head, d->index, d );
	f->modified = 1;
}

/* create a new file or overwrite an old one, with the contents of src */
static void thom_put_file(thom_floppy* f, unsigned head,
				thom_dirent* d, imgtool_stream *src)
{
	int size = stream_size( src );
	UINT8* fat = thom_get_sector( f, head, 20, 2 );
	int nbblocks = thom_nb_blocks(f);
	int block;

	/* find first free block */
	for ( block = 0; block < nbblocks && fat[ block + 1 ] != 0xff; block++ );
	if ( block >= nbblocks ) return;
	d->firstblock = block;

	/* store file */
	while (1) {
	int track = block / 2;
	int firstsect = (block % 2) ? 9 : 1;
	int i;

	/* store data, full sectors */
	for ( i = 0; i < 8 && size > f->sectuse_size; i++ ) {
		UINT8* dst = thom_get_sector( f, head, track, firstsect + i );
		stream_read( src, dst, f->sectuse_size );
		size -= f->sectuse_size;
	}

	/* store data, last sector */
	if ( i < 8 ) {
		UINT8* dst = thom_get_sector( f, head, track, firstsect + i );
		stream_read( src, dst, size );
		fat[ block + 1 ] = 0xc1 + i;
		d->lastsectsize = size;
		break;
	}

	/* find next free block & update fat */
	i = block;
	for ( block++; block < nbblocks && fat[ block + 1 ] != 0xff; block++ );
	if ( block >= nbblocks ) {
		/* out of memory! */
		fat[ i + 1 ] = 0;
		d->lastsectsize = 0;
		break;
	}
	fat[ i + 1 ] = block;
	}

	d->type = THOM_DIRENT_FILE;
	thom_set_dirent( f, head, d->index, d );
	f->modified = 1;
}


/********************** module functions ***********************/

static imgtoolerr_t thom_get_sector_size(imgtool_image* img, UINT32 track,
						UINT32 head, UINT32 sector,
						UINT32 *sector_size)
{
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );

	if ( head >= f->heads || sector < 1 || sector > 16 || track >= f->tracks ) {
	if ( sector_size ) *sector_size = 0;
	return IMGTOOLERR_SEEKERROR;
	}
	if ( sector_size ) *sector_size = f->sector_size;
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_get_geometry(imgtool_image* img, UINT32* tracks,
						UINT32* heads, UINT32* sectors)
{
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	if ( tracks ) *tracks = f->tracks;
	if ( heads ) *heads = f->heads;
	if ( sectors ) *sectors = 16;
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_read_sector(imgtool_image* img, UINT32 track,
						UINT32 head, UINT32 sector, void *buf,
						size_t len)
{
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	if ( head >= f->heads || sector < 1 || sector > 16 || track >= f->tracks )
	return IMGTOOLERR_SEEKERROR;
	if ( len > f->sector_size) return IMGTOOLERR_READERROR;
	memcpy( buf, thom_get_sector( f, head, track, sector ), len );
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_write_sector(imgtool_image *img, UINT32 track,
						UINT32 head, UINT32 sector,
						const void *buf, size_t len, int ddam)
{
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	if ( stream_isreadonly( f->stream ) ) return IMGTOOLERR_WRITEERROR;
	if ( head >= f->heads || sector < 1 || sector > 16 || track >= f->tracks )
	return IMGTOOLERR_SEEKERROR;
	if ( len > f->sector_size) return IMGTOOLERR_WRITEERROR;
	f->modified = 1;
	memcpy( thom_get_sector( f, head, track, sector ), buf, len );
	return IMGTOOLERR_SUCCESS;
}

/* returns floopy name */
/* actually, each side has its own name, but we only return the one on side 0.
 */
static void thom_info(imgtool_image *img, char *string, size_t len)
{
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	UINT8* base = thom_get_sector( f, 0, 20, 1 );
	char buf[9];
	memcpy( buf, base, 8 );
	buf[8] = 0;
	thom_stringify( buf );
	strncpy( string, buf, len );
}

/* each side of a floppy has its own filesystem, we treat them as'partitions'
 */
static imgtoolerr_t thom_list_partitions(imgtool_image *img,
						imgtool_partition_info *partitions,
						size_t len)
{
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	if ( len >= 1 ) {
	partitions[0].get_info = thom_basic_get_info;
	partitions[0].base_block = 0;
	partitions[0].block_count = 1;
	}
	if ( len >= 2 && f->heads >= 2 ) {
	partitions[1].get_info = thom_basic_get_info;
	partitions[1].base_block = 1;
	partitions[1].block_count = 1;
	}
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_open_partition(imgtool_partition *part,
					UINT64 first_block, UINT64 block_count)
{
	imgtool_image* img = imgtool_partition_image( part );
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	if ( first_block >= f->heads )
	return IMGTOOLERR_INVALIDPARTITION;
	* ( (int*) imgtool_partition_extra_bytes( part ) ) = first_block;
	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t thom_begin_enum(imgtool_directory *enumeration,
					const char *path)
{
	int* n = (int*) imgtool_directory_extrabytes( enumeration );
	*n = 0;
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_next_enum(imgtool_directory *enumeration,
					imgtool_dirent *ent)
{
	imgtool_partition *part = imgtool_directory_partition( enumeration);
	int head = *( (int*) imgtool_partition_extra_bytes( part ) );
	imgtool_image* img = imgtool_partition_image( part );
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	int* n = (int*) imgtool_directory_extrabytes( enumeration );
	thom_dirent d;

	do {
	thom_get_dirent( f, head, *n, &d );
	(*n) ++;
	}
	while ( d.type == THOM_DIRENT_FREE );
	if ( d.type == THOM_DIRENT_END ) ent->eof = 1;
	else if ( d.type == THOM_DIRENT_INVALID ) {
	ent->corrupt = 1;
	}
	else {
	int size;
	snprintf( ent->filename, sizeof(ent->filename), "%s.%s", d.name, d.ext );
	snprintf( ent->attr, sizeof(ent->attr), "%c %c %s",
			(d.ftype == 0) ? 'B' :  (d.ftype == 1) ? 'D' :
			(d.ftype == 2) ? 'M' :  (d.ftype == 3) ? 'A' : '?',
			(d.format == 0) ? 'B' : (d.format == 0xff) ? 'A' : '?',
			d.comment );
	ent->creation_time = thom_crack_time( &d );
	size  = thom_get_file_size( f, head, &d );
	if ( size >= 0 ) ent->filesize = size;
	else {
		ent->filesize = 0;
		ent->corrupt = 1;
	}
	}
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_free_space(imgtool_partition *part, UINT64 *size)
{
	int head = *( (int*) imgtool_partition_extra_bytes( part ) );
	imgtool_image* img = imgtool_partition_image( part );
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	int nb = thom_get_free_blocks( f, head );
	(*size) = nb * f->sectuse_size * 8;
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_read_file(imgtool_partition *part,
					const char *filename,
					const char *fork,
					imgtool_stream *destf)
{
	int head = *( (int*) imgtool_partition_extra_bytes( part ) );
	imgtool_image* img = imgtool_partition_image( part );
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	thom_dirent d;
	char name[9], ext[4], fname[14];
	int size;

	/* convert filename */
	thom_conv_filename( filename, name, ext );
	sprintf( fname, "%s.%s", name, ext );

	if ( ! thom_find_dirent( f, head, fname, &d ) )
	return IMGTOOLERR_FILENOTFOUND;
	size = thom_get_file_size( f, head, &d );
	if ( size < 0 ) return IMGTOOLERR_CORRUPTFILE;
	thom_get_file( f, head, &d, destf );
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_delete_file(imgtool_partition *part,
						const char *filename)
{
	int head = *( (int*) imgtool_partition_extra_bytes( part ) );
	imgtool_image* img = imgtool_partition_image( part );
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	thom_dirent d;
	char name[9], ext[4], fname[14];

	/* convert filename */
	thom_conv_filename( filename, name, ext );
	sprintf( fname, "%s.%s", name, ext );

	if ( ! thom_find_dirent( f, head, fname, &d ) )
	return IMGTOOLERR_FILENOTFOUND;
	/*if ( thom_get_file_size( f, head, &d ) < 0 ) return IMGTOOLERR_CORRUPTFILE;*/
	if ( stream_isreadonly( f->stream ) ) return IMGTOOLERR_WRITEERROR;
	thom_del_file( f, head, &d );
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_write_file(imgtool_partition *part,
					const char *filename,
					const char *fork,
					imgtool_stream *sourcef,
					option_resolution *opts)
{
	int head = *( (int*) imgtool_partition_extra_bytes( part ) );
	imgtool_image* img = imgtool_partition_image( part );
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	thom_dirent d;
	int size = stream_size( sourcef );
	int blocks = thom_get_free_blocks( f, head );
	char name[9], ext[4], fname[14];
	const char* comment;
	int is_new = 1;

	if ( stream_isreadonly( f->stream ) ) return IMGTOOLERR_WRITEERROR;

	/* convert filename */
	thom_conv_filename( filename, name, ext );
	sprintf( fname, "%s.%s", name, ext );

	/* check available space & find dir entry */
	if ( thom_find_dirent( f, head, fname, &d ) ) {
	/* file already exists: delete it */
	if ( thom_get_file_size( f, head, &d ) < 0 ) return IMGTOOLERR_CORRUPTFILE;
	blocks += thom_get_file_blocks( f, head, &d );
	if ( blocks * 8 * f->sectuse_size < size ) return IMGTOOLERR_NOSPACE;
	thom_del_file( f, head, &d );
	is_new = 0;
	}
	else {
	/* new file, need new dir entry */
	if ( blocks * 8 * f->sectuse_size < size ) return IMGTOOLERR_NOSPACE;
	if ( ! thom_find_free_dirent( f, head, &d ) ) return IMGTOOLERR_NOSPACE;
	thom_make_time_now( &d );
	}

	/* fill-in dir entry */
	memcpy( d.name, name, 9 );
	memcpy( d.ext, ext, 4 );

	/* file type */
	switch ( option_resolution_lookup_int( opts, 'T' ) ) {
	case 0:
	if ( ! is_new ) break;
	if ( ! core_stricmp( ext, "BAS" ) ) d.ftype = 0;
	else if ( ! core_stricmp( ext, "BAT" ) ) d.ftype = 0;
	else if ( ! core_stricmp( ext, "DAT" ) ) d.ftype = 1;
	else if ( ! core_stricmp( ext, "ASC" ) ) d.ftype = 1;
	else if ( ! core_stricmp( ext, "BIN" ) ) d.ftype = 2;
	else if ( ! core_stricmp( ext, "MAP" ) ) d.ftype = 2;
	else if ( ! core_stricmp( ext, "CFG" ) ) d.ftype = 2;
	else if ( ! core_stricmp( ext, "ASM" ) ) d.ftype = 3;
	else d.ftype = 2;
	break;
	case 1: d.ftype = 0; break;
	case 2: d.ftype = 1; break;
	case 3: d.ftype = 2; break;
	case 4: d.ftype = 3; break;
	}

	/* format flag */
	switch ( option_resolution_lookup_int( opts, 'F' ) ) {
	case 0:
	if ( ! is_new ) break;
	if ( ! core_stricmp( ext, "DAT" ) ) d.format = 0xff;
	else if ( ! core_stricmp( ext, "ASC" ) ) d.format = 0xff;
	else if ( ! core_stricmp( ext, "ASM" ) ) d.format = 0xff;
	else d.format = 0;
	break;
	case 1: d.format = 0; break;
	case 2: d.format = 0xff; break;
	}

	/* comment */
	comment = option_resolution_lookup_string( opts, 'C' );
	if ( comment && *comment )
	strncpy( d.comment, comment, 8 );

	/* write file */
	thom_put_file( f, head, &d, sourcef );
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_suggest_transfer(imgtool_partition *part,
						const char *fname,
						imgtool_transfer_suggestion *suggestions,
						size_t suggestions_length)
{
	int head = *( (int*) imgtool_partition_extra_bytes( part ) );
	imgtool_image* img = imgtool_partition_image( part );
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	thom_dirent d;
	int is_basic = 0;

	if ( suggestions_length < 1 ) return IMGTOOLERR_SUCCESS;

	if ( fname ) {
	if ( ! thom_find_dirent( f, head, fname, &d ) )
		return IMGTOOLERR_FILENOTFOUND;
	if ( d.ftype == 0 && d.format == 0 ) is_basic = 1;
	}

	if ( is_basic ) {
	suggestions[0].viability = SUGGESTION_RECOMMENDED;
	suggestions[0].filter = filter_thombas128_getinfo;
	if ( suggestions_length >= 2 ) {
		suggestions[1].viability = SUGGESTION_POSSIBLE;
		suggestions[1].filter = filter_thombas7_getinfo;
	}
	if ( suggestions_length >= 3 ) {
		suggestions[2].viability = SUGGESTION_POSSIBLE;
		suggestions[2].filter = filter_thombas5_getinfo;
	}
	if ( suggestions_length >= 4 ) {
		suggestions[3].viability = SUGGESTION_POSSIBLE;
		suggestions[3].filter = NULL;
	}
	}
	else {
	suggestions[0].viability = SUGGESTION_RECOMMENDED;
	suggestions[0].filter = NULL;
	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_create(imgtool_image* img,
				imgtool_stream *stream,
				option_resolution *opts)
{
	thom_floppy* f = (thom_floppy*) imgtool_image_extra_bytes( img );
	int i;
	UINT8* buf;
	const char* name;

	f->stream = stream;
	f->modified = 0;

	/* get parameters */
	f->heads = option_resolution_lookup_int( opts, 'H' );
	f->tracks = option_resolution_lookup_int( opts, 'T' );
	name = option_resolution_lookup_string( opts, 'N' );
	switch ( option_resolution_lookup_int( opts, 'D' ) ) {
	case 0: f->sector_size = 128; f->sectuse_size = 128; break;
	case 1: f->sector_size = 256; f->sectuse_size = 255; break;
	default: return IMGTOOLERR_PARAMCORRUPT;
	}

	/* sanity checks */
	switch ( f->tracks ) {
	case 25: if ( f->sector_size != 128 ) return IMGTOOLERR_PARAMCORRUPT; break;
	case 40: break;
	case 80: if ( f->sector_size != 256 ) return IMGTOOLERR_PARAMCORRUPT; break;
	default: return IMGTOOLERR_PARAMCORRUPT;
	}

	memset( f->data, 0xe5, sizeof( f->data ) );

	for ( i = 0; i < f->heads; i++ ) {
	/* disk info */
	buf = thom_get_sector( f, i, 20, 1 );
	memset( buf, 0xff, f->sector_size );
	if ( name ) {
		strncpy( (char*)buf, name, 8 );
		thom_unstringity( (char*)buf, 8 );
	}
	else memset( buf, ' ', 8 );

	/* FAT */
	buf = thom_get_sector( f, i, 20, 2 );
	memset( buf, 0, f->sector_size );
	memset( buf + 1, 0xff, thom_nb_blocks( f ) );
	buf[ 41 ] = buf[ 42 ] = 0xfe; /* reserved for FAT */

	/* (empty) directory */
	buf = thom_get_sector( f, i, 20, 3 );
	memset( buf, 0xff, 14 * f->sector_size );
	}

	f->modified = 1;

	return IMGTOOLERR_SUCCESS;
}


/****************** BASIC tokenization *********************/

/* character codes >= 128 are reserved for BASIC keywords */

static const char *const thombas7[2][128] = {
	{ /* statements */
	"END", "FOR", "NEXT", "DATA", "DIM", "READ", "LET", "GO", "RUN", "IF",
	"RESTORE", "RETURN", "REM", "'", "STOP", "ELSE", "TRON", "TROFF", "DEFSTR",
	"DEFINT", "DEFSNG", "DEFDBL", "ON", "WAIT", "ERROR", "RESUME", "AUTO",
	"DELETE", "LOCATE", "CLS", "CONSOLE", "PSET", "MOTOR", "SKIP", "EXEC",
	"BEEP", "COLOR", "LINE", "BOX", "UNMASK", "ATTRB", "DEF", "POKE", "PRINT",
	"CONT", "LIST", "CLEAR", "WHILE", "WHEN", "NEW", "SAVE", "LOAD", "MERGE",
	"OPEN", "CLOSE", "INPEN", "PEN", "PLAY", "TAB(", "TO", "SUB", "FN",
	"SPC(", "USING", "USR", "ERL", "ERR", "OFF", "THEN", "NOT", "STEP",
	"+", "-", "*", "/", "^", "AND", "OR", "XOR", "EQV", "IMP", "MOD", "@",
	">", "=", "<",
	/* DOS specific */
	"DSKINI", "DSKO$", "KILL", "NAME", "FIELD", "LSET", "RSET",
	"PUT", "GET", "VERIFY", "DEVICE", "DIR", "FILES", "WRITE", "UNLOAD",
	"BACKUP", "COPY", "CIRCLE", "PAINT", "DRAW", "RENUM", "SWAP", "DENSITY",
	},
	{ /* functions: 0xff prefix */
	"SGN", "INT", "ABS", "FRE", "SQR", "LOG", "EXP", "COS", "SIN",
	"TAN", "PEEK", "LEN", "STR$", "VAL", "ASC", "CHR$", "EOF", "CINT", "CSNG",
	"CDBL", "FIX", "HEX$", "OCT$", "STICK", "STRIG", "GR$", "LEFT$", "RIGHT$",
	"MID$", "INSTR", "VARPTR", "RND", "INKEY$", "INPUT", "CSRLIN", "POINT",
	"SCREEN", "POS", "PTRIG",
	/* DOS specific */
	"DSKL", "CVI", "CVS", "CVD", "MKI$", "MKS$", "MKD$", "LOC", "LOF",
	"SPACE$", "STRING$", "DSKI$",
	}
};

/* MO5: some keywords ar missing; DOS and TUNE are added */
static const char *const thombas5[2][128] = {
	{ /* statements */
	"END", "FOR", "NEXT", "DATA", "DIM", "READ", NULL, "GO", "RUN", "IF",
	"RESTORE", "RETURN", "REM", "'", "STOP", "ELSE", "TRON", "TROFF", "DEFSTR",
	"DEFINT", "DEFSNG", NULL, "ON", "TUNE", "ERROR", "RESUME", "AUTO",
	"DELETE", "LOCATE", "CLS", "CONSOLE", "PSET", "MOTOR", "SKIP", "EXEC",
	"BEEP", "COLOR", "LINE", "BOX", NULL, "ATTRB", "DEF", "POKE", "PRINT",
	"CONT", "LIST", "CLEAR", "DOS", NULL, "NEW", "SAVE", "LOAD", "MERGE",
	"OPEN", "CLOSE", "INPEN", "PEN", "PLAY", "TAB(", "TO", "SUB", "FN",
	"SPC(", "USING", "USR", "ERL", "ERR", "OFF", "THEN", "NOT", "STEP",
	"+", "-", "*", "/", "^", "AND", "OR", "XOR", "EQV", "IMP", "MOD", "@",
	">", "=", "<",
	/* DOS specific */
	"DSKINI", "DSKO$", "KILL", "NAME", "FIELD", "LSET", "RSET",
	"PUT", "GET", "VERIFY", "DEVICE", "DIR", "FILES", "WRITE", "UNLOAD",
	"BACKUP","COPY", "CIRCLE", "PAINT", "DRAW", "RENUM", "SWAP", "SEARCH",
	"DENSITY",
	},
	{ /* functions: 0xff prefix */
	"SGN", "INT", "ABS", "FRE", "SQR", "LOG", "EXP", "COS", "SIN",
	"TAN", "PEEK", "LEN", "STR$", "VAL", "ASC", "CHR$", "EOF", "CINT", NULL,
	NULL, "FIX", "HEX$", NULL, "STICK", "STRIG", "GR$", "LEFT$", "RIGHT$",
	"MID$", "INSTR", "VARPTR", "RND", "INKEY$", "INPUT", "CSRLIN", "POINT",
	"SCREEN", "POS", "PTRIG",
	/* DOS specific */
	"DSKL", "CVI", "CVS", "CVD", "MKI$", "MKS$", "MKD$", "LOC", "LOF",
	"SPACE$", "STRING$", "DSKI$",
	}
};

/* BASIC 128 has many new keywords */
/* DENSITY is missing on BASIC 512 & BASIC 128 for MO6 but, otherwise, they
   have the same tokens
*/
static const char *const thombas128[2][128] = {
	{ /* statements */
	"END", "FOR", "NEXT", "DATA", "DIM", "READ", "LET", "GO", "RUN", "IF",
	"RESTORE", "RETURN", "REM", "'", "STOP", "ELSE", "TRON", "TROFF", "DEFSTR",
	"DEFINT", "DEFSNG", "DEFDBL", "ON", "WAIT", "ERROR", "RESUME", "AUTO",
	"DELETE", "LOCATE", "CLS", "CONSOLE", "PSET", "MOTOR", "SKIP", "EXEC",
	"BEEP", "COLOR", "LINE", "BOX", "UNMASK", "ATTRB", "DEF", "POKE", "PRINT",
	"CONT", "LIST", "CLEAR", "INTERVAL", "KEY", "NEW", "SAVE", "LOAD", "MERGE",
	"OPEN", "CLOSE", "INPEN", "PEN", "PLAY", "TAB(", "TO", "SUB", "FN",
	"SPC(", "USING", "USR", "ERL", "ERR", "OFF", "THEN", "NOT", "STEP",
	"+", "-", "*", "/", "^", "AND", "OR", "XOR", "EQV", "IMP", "MOD", "@",
	">", "=", "<",
	"DSKINI", "DSKO$", "KILL", "NAME", "FIELD", "LSET", "RSET",
	"PUT", "GET", "VERIFY", "DEVICE", "DIR", "FILES", "WRITE", "UNLOAD",
	"BACKUP", "COPY", "CIRCLE", "PAINT", "RESET", "RENUM", "SWAP", "DENSITY",
	"WINDOW", "PATTERN", "DO", "LOOP", "EXIT", "INMOUSE", "MOUSE", "CHAINE",
	"COMMON", "SEARCH", "FWD", "TURTLE",
	},
	{ /* functions: 0xff prefix */
	"SGN", "INT", "ABS", "FRE", "SQR", "LOG", "EXP", "COS", "SIN",
	"TAN", "PEEK", "LEN", "STR$", "VAL", "ASC", "CHR$", "EOF", "CINT", "CSNG",
	"CDBL", "FIX", "HEX$", "OCT$", "STICK", "STRIG", "GR$", "LEFT$", "RIGHT$",
	"MID$", "INSTR", "VARPTR", "RND", "INKEY$", "INPUT", "CSRLIN", "POINT",
	"SCREEN", "POS", "PTRIG",
	"DSKL", "CVI", "CVS", "CVD", "MKI$", "MKS$", "MKD$", "LOC", "LOF",
	"SPACE$", "STRING$", "DSKI$",
	"FKEY$", "MIN(", "MAX(", "ATN", "CRUNCH$", "MTRIG", "EVAL", "PALETTE",
	"BANK", "HEAD", "ROT", "SHOW", "ZOOM", "TRACE",
	}
};

/* tables for simple XOR encryption sieve */
static const UINT8 crypt1[11] = {
	0x80, 0x19, 0x56, 0xAA, 0x80, 0x76, 0x22, 0xF1,  0x82, 0x38, 0xAA
};

static const UINT8 crypt2[13] = {
	0x86, 0x1E, 0xD7, 0xBA, 0x87, 0x99, 0x26, 0x64, 0x87, 0x23, 0x34, 0x58, 0x86
};

/* decrypt BASIC protected files */
static void thom_decrypt(imgtool_stream* out, imgtool_stream* in)
{
	int i1 = 11, i2 = 13;
	while ( 1 ) {
	UINT8 b;
	if ( stream_read( in, &b, 1) < 1 ) break;
	b = ( (UINT8)(b - i2) ^ crypt2[i2-1] ^ crypt1[i1-1] ) + i1;
	stream_putc( out, b );
	i1--; i2--;
	if ( !i1 ) i1 = 11;
	if ( !i2 ) i2 = 13;
	}
}

/* encrypt BASIC protected files */
static void thom_encrypt(imgtool_stream* out, imgtool_stream* in)
{
	int i1 = 11, i2 = 13;
	while ( 1 ) {
	UINT8 b;
	if ( stream_read( in, &b, 1) < 1 ) break;
	b = ( (UINT8)(b - i1) ^ crypt2[i2-1] ^ crypt1[i1-1] ) + i2;
	stream_putc( out, b );
	i1--; i2--;
	if ( !i1 ) i1 = 11;
	if ( !i2 ) i2 = 13;
	}
}

static imgtoolerr_t thomcrypt_read_file(imgtool_partition *part,
					const char *name,
					const char *fork, imgtool_stream *dst)
{
	UINT8 buf[3];
	imgtool_stream *org = stream_open_mem( NULL, 0 );
	imgtoolerr_t err;
	if ( !org ) return IMGTOOLERR_OUTOFMEMORY;

	/* read file */
	err = thom_read_file( part, name, fork, org );
	if ( err ) {
	stream_close( org );
	return err;
	}

	stream_seek( org, 0, SEEK_SET );
	if ( stream_read( org, buf, 3 ) < 3 || buf[0] != 0xfe ) {
	/* regular file */
	stream_seek( org, 0, SEEK_SET );
	stream_transfer_all( dst, org );
	}
	else {
	/* encrypted file */
	stream_putc( dst, '\xff' );
	stream_write( dst, buf+1, 2 );
	thom_decrypt( dst, org );
	}

	stream_close( org );
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thomcrypt_write_file(imgtool_partition *part,
						const char *name,
						const char *fork, imgtool_stream *src,
						option_resolution *opts)
{
	UINT8 buf[3];

	if ( stream_read( src, buf, 3 ) < 3 || buf[0] == 0xfe ) {
	/* too short or already encrypted file */
	stream_seek( src, 0, SEEK_SET );
	return thom_write_file( part, name, fork, src, opts );
	}
	else {
	/* regular file */
	imgtool_stream *dst = stream_open_mem( NULL, 0 );
	imgtoolerr_t err;
	if ( !dst ) return IMGTOOLERR_OUTOFMEMORY;
	stream_putc( dst, '\xfe' );
	stream_write( dst, buf+1, 2 );
	thom_encrypt( dst, src );
	stream_seek( dst, 0, SEEK_SET );
	err = thom_write_file( part, name, fork, dst, opts );
	stream_close( dst);
	return err;
	}
}

void filter_thomcrypt_getinfo(UINT32 state, union filterinfo *info)
{
	switch(state) {
	case FILTINFO_STR_NAME:
	info->s = "thomcrypt";
	break;
	case FILTINFO_STR_HUMANNAME:
	info->s = "Thomson BASIC, Protected file encryption (no tokenization)";
	break;
	case FILTINFO_PTR_READFILE:
	info->read_file = thomcrypt_read_file;
	break;
	case FILTINFO_PTR_WRITEFILE:
	info->write_file = thomcrypt_write_file;
	break;
	}
}

/* untokenization automatically decrypt protected files */
static imgtoolerr_t thom_basic_read_file(imgtool_partition *part,
						const char *name,
						const char *fork,
						imgtool_stream *dst,
						const char *const table[2][128])
{
	imgtool_stream *org = stream_open_mem( NULL, 0 );
	imgtoolerr_t err;
	UINT8 buf[4];
	int i;

	if ( !org ) return IMGTOOLERR_OUTOFMEMORY;

	err = thomcrypt_read_file( part, name, fork, org );
	if ( err ) {
	stream_close( org );
	return err;
	}
	stream_seek( org, 3, SEEK_SET ); /* skip header */

	while ( 1 ) {
	int in_str = 0, in_fun = 0;
	int linelength, linenum;

	/* line header: line length and line number */
	/* I am not sure this is 100% correct but it works in many cases */
	if ( stream_read( org, buf, 2 ) < 2 ) goto end;
	linelength = ((int)buf[0] << 8) + (int)buf[1] - 4;
	if ( linelength <= 0 ) goto end;
	if ( stream_read( org, buf, 2 ) < 2 ) goto end;
	linenum = ((int)buf[0] << 8) + buf[1];
	stream_printf( dst, "%u ", linenum );

	/* process line */
	for ( i = 0; i < linelength; i++ ) {
		UINT8 c;
		if ( stream_read( org, &c, 1 ) < 1 ) break;
		if ( c == 0 ) {
	/* Sometimes, linelength seems wrong and we must rely on the fact that
	   BASIC lines are 0-terminated.
	   At other times, there are 0 embedded within lines or extra stuff
	   between the 0 and the following line, and so, we must rely
	   on linelength to cut the line (!)
	*/
	if ( linelength > 256 ) break;
		}
		else if ( c == 0xff && ! in_str ) in_fun = 1; /* function prefix */
		else {
	if ( c >= 0x80 && ! in_str ) {
		/* token */
		const char* token = table[ in_fun ][ c - 0x80 ];
		if ( token ) stream_puts( dst, token );
		else stream_puts( dst, "???" );
	}
	else {
		/* regular character */
		if ( c == '"' ) in_str = 1 - in_str;
		stream_putc( dst, c ); /* normal letter */
	}
	in_fun = 0;
		}
	}
	stream_putc( dst, '\n' );
	}
	end:

	stream_close( org );
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t thom_basic_write_file(imgtool_partition *part,
						const char *name,
						const char *fork,
						imgtool_stream *src,
						option_resolution *opts,
						const char *const table[2][128])
{
	return IMGTOOLERR_UNIMPLEMENTED;
}


#define FILTER(short,long)                      \
	static imgtoolerr_t short##_read_file(imgtool_partition *part,  \
					const char *name,       \
					const char *fork,       \
					imgtool_stream *dst)        \
	{                                   \
	return thom_basic_read_file( part, name, fork, dst, short );    \
	}                                   \
	static imgtoolerr_t short##_write_file(imgtool_partition *part, \
						const char *name,       \
						const char *fork,       \
						imgtool_stream *src,        \
						option_resolution *opts)    \
	{                                   \
	return thom_basic_write_file( part, name, fork, src, opts, short ); \
	}                                   \
	void filter_##short##_getinfo(UINT32 state, union filterinfo *info) \
	{                                   \
	switch(state)                           \
		{                                   \
		case FILTINFO_STR_NAME:                     \
	info->s = #short;                       \
	break;                              \
		case FILTINFO_STR_HUMANNAME:                    \
	info->s = long;                         \
	break;                              \
		case FILTINFO_PTR_READFILE:                 \
	info->read_file = short##_read_file;                \
	break;                              \
		case FILTINFO_PTR_WRITEFILE:                    \
	info->write_file = short##_write_file;              \
	break;                              \
		}                                   \
	}

FILTER( thombas5,
	"Thomson MO5 w/ BASIC 1.0, Tokenized Files (read-only, auto-decrypt)" )
FILTER( thombas7,
	"Thomson TO7 w/ BASIC 1.0, Tokenized Files (read-only, auto-decrypt)" )
FILTER( thombas128,
	"Thomson w/ BASIC 128/512, Tokenized Files (read-only, auto-decrypt)" )


/************************* driver ***************************/

static OPTION_GUIDE_START( thom_createimage_optguide )
	OPTION_INT( 'H', "heads", "Heads" )
	OPTION_INT( 'T', "tracks", "Tracks" )
	OPTION_ENUM_START( 'D', "density", "Density" )
	OPTION_ENUM( 0, "SD", "Single density (128 bytes)" )
	OPTION_ENUM( 1, "DD", "Double density (256 bytes)" )
	OPTION_ENUM_END
	OPTION_STRING( 'N', "name", "Floppy name" )
OPTION_GUIDE_END

static OPTION_GUIDE_START( thom_writefile_optguide )
	OPTION_ENUM_START( 'T', "ftype", "File type" )
	OPTION_ENUM( 0, "auto", "Automatic (by extension)" )
	OPTION_ENUM( 1, "B", "Program" )
	OPTION_ENUM( 2, "D", "Data" )
	OPTION_ENUM( 3, "M", "Machine Code" )
	OPTION_ENUM( 4, "A",  "Assembler Source" )
	OPTION_ENUM_END
	OPTION_ENUM_START( 'F', "format", "Format flag" )
	OPTION_ENUM( 0, "auto",   "Automatic (by extension)" )
	OPTION_ENUM( 1, "B", "Binary, Tokenized BASIC" )
	OPTION_ENUM( 2, "A", "ASCII" )
	OPTION_ENUM_END
	OPTION_STRING( 'C', "comment", "Comment" )
OPTION_GUIDE_END

static void thom_basic_get_info(const imgtool_class *clas,
				UINT32 param,
				union imgtoolinfo *info)
{
	switch ( param ) {
	case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES:
	info->i = sizeof(thom_floppy); break;
	case IMGTOOLINFO_INT_PARTITION_EXTRA_BYTES:
	info->i = sizeof(int); break;
	case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:
	info->i = sizeof(int); break;
	case IMGTOOLINFO_INT_SUPPORTS_CREATION_TIME:
	case IMGTOOLINFO_INT_PREFER_UCASE:
	info->i = 1; break;
	case IMGTOOLINFO_INT_PATH_SEPARATOR:
	info->i = 0; break;
	case IMGTOOLINFO_STR_FILE:
	strcpy( info->s = imgtool_temp_str(), __FILE__ ); break;
	case IMGTOOLINFO_STR_NAME:
	strcpy( info->s = imgtool_temp_str(), "thom" ); break;
	case IMGTOOLINFO_STR_DESCRIPTION:
	strcpy( info->s = imgtool_temp_str(), "Thomson BASIC filesystem" );
	break;
	case IMGTOOLINFO_PTR_CREATE:
	info->create = thom_create; break;
	case IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE:
	info->createimage_optguide = thom_createimage_optguide; break;
	case IMGTOOLINFO_PTR_BEGIN_ENUM:
	info->begin_enum = thom_begin_enum; break;
	case IMGTOOLINFO_PTR_NEXT_ENUM:
	info->next_enum = thom_next_enum; break;
	case IMGTOOLINFO_PTR_READ_FILE:
	info->read_file = thom_read_file; break;
	case IMGTOOLINFO_PTR_WRITE_FILE:
	info->write_file = thom_write_file; break;
	case IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE:
	info->writefile_optguide = thom_writefile_optguide; break;
	case IMGTOOLINFO_STR_WRITEFILE_OPTSPEC:
	strcpy( info->s = imgtool_temp_str(), "T[0]-4;F[0]-2;C" ); break;
	case IMGTOOLINFO_PTR_SUGGEST_TRANSFER:
	info->suggest_transfer = thom_suggest_transfer; break;
	case IMGTOOLINFO_PTR_DELETE_FILE:
	info->delete_file = thom_delete_file; break;
	case IMGTOOLINFO_PTR_FREE_SPACE:
	info->free_space = thom_free_space; break;
	case IMGTOOLINFO_PTR_GET_SECTOR_SIZE:
	info->get_sector_size = thom_get_sector_size; break;
	case IMGTOOLINFO_PTR_GET_GEOMETRY:
	info->get_geometry = thom_get_geometry; break;
	case IMGTOOLINFO_PTR_INFO:
	info->info = thom_info; break;
	case IMGTOOLINFO_PTR_READ_SECTOR:
	info->read_sector = thom_read_sector; break;
	case IMGTOOLINFO_PTR_WRITE_SECTOR:
	info->write_sector = thom_write_sector; break;
	case IMGTOOLINFO_PTR_LIST_PARTITIONS:
	info->list_partitions = thom_list_partitions; break;
	case IMGTOOLINFO_PTR_OPEN_PARTITION:
	info->open_partition = thom_open_partition; break;
	}
}

void thom_fd_basic_get_info(const imgtool_class *clas,
				UINT32 param,
				union imgtoolinfo *info)
{
	switch ( param ) {
	case IMGTOOLINFO_STR_NAME:
	strcpy( info->s = imgtool_temp_str(), "thom_fd" ); break;
	case IMGTOOLINFO_STR_DESCRIPTION:
	strcpy( info->s = imgtool_temp_str(),
		"Thomson .fd disk image, BASIC format" );
	break;
	case IMGTOOLINFO_STR_FILE_EXTENSIONS:
	strcpy( info->s = imgtool_temp_str(), "fd" ); break;
	case IMGTOOLINFO_PTR_OPEN:
	info->open = thom_open_fd_qd; break;
	case IMGTOOLINFO_PTR_CLOSE:
	info->close = thom_close_fd_qd; break;
	case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC:
	strcpy( info->s = imgtool_temp_str(), "H[1]-2;T40/[80];D0-[1];N" ); break;
	default:
	thom_basic_get_info( clas, param, info );
	}
}

void thom_qd_basic_get_info(const imgtool_class *clas,
				UINT32 param,
				union imgtoolinfo *info)
{
	switch ( param ) {
	case IMGTOOLINFO_STR_NAME:
	strcpy( info->s = imgtool_temp_str(), "thom_qd" ); break;
	case IMGTOOLINFO_STR_DESCRIPTION:
	strcpy( info->s = imgtool_temp_str(),
		"Thomson .qd disk image, BASIC format" );
	break;
	case IMGTOOLINFO_STR_FILE_EXTENSIONS:
	strcpy( info->s = imgtool_temp_str(), "qd" ); break;
	case IMGTOOLINFO_PTR_OPEN:
	info->open = thom_open_fd_qd; break;
	case IMGTOOLINFO_PTR_CLOSE:
	info->close = thom_close_fd_qd; break;
	case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC:
	strcpy( info->s = imgtool_temp_str(), "H[1]-2;T[25];D[0];N" ); break;
	default:
	thom_basic_get_info( clas, param, info );
	}
}

void thom_sap_basic_get_info(const imgtool_class *clas,
					UINT32 param,
					union imgtoolinfo *info)
{
	switch ( param ) {
	case IMGTOOLINFO_STR_NAME:
	strcpy( info->s = imgtool_temp_str(), "thom_sap" ); break;
	case IMGTOOLINFO_STR_DESCRIPTION:
	strcpy( info->s = imgtool_temp_str(),
		"Thomson .sap disk image, BASIC format" );
	break;
	case IMGTOOLINFO_STR_FILE_EXTENSIONS:
	strcpy( info->s = imgtool_temp_str(), "sap" ); break;
	case IMGTOOLINFO_PTR_OPEN:
	info->open = thom_open_sap; break;
	case IMGTOOLINFO_PTR_CLOSE:
	info->close = thom_close_sap; break;
	case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC:
	strcpy( info->s = imgtool_temp_str(), "H[1];T40/[80];D0-[1];N" ); break;
	default:
	thom_basic_get_info( clas, param, info );
	}
}
