// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    formats/coco_dsk.c

    Tandy Color Computer / Dragon disk images

*********************************************************************/

#include <string.h>
#include <time.h>
#include <assert.h>

#include "formats/coco_dsk.h"
#include "formats/basicdsk.h"
#include "formats/imageutl.h"
#include "coretmpl.h"

/* -----------------------------------------------------------------------
 * JVC (Jeff Vavasour CoCo) format
 *
 * Used by Jeff Vavasour's CoCo Emulators
 *
 *  Documentation taken from Tim Linder's web site:
 *      http://home.netcom.com/~tlindner/JVC.html
 *
 *  A. Header length
 *      The header length is determined by the file length modulo 256:
 *          headerSize = fileLength % 256;
 *      This means that the header is variable length and the minimum size
 *      is zero bytes, and the maximum size of 255 bytes.
 *
 * B. Header
 *      Here is a description of the header bytes:
 *          Byte Offset     Description            Default
 *          -----------     -----------------      -------
 *                    0     Sectors per track      18
 *                    1     Side count              1
 *                    2     Sector size code        1
 *                    3     First sector ID         1
 *                    4     Sector attribute flag   0
 *
 *  If the sector attribute flag is zero then the track count is determined
 *  by the formula:
 *
 *      (fileLength - headerSize) / (sectorsPerTrack * (128 <<
 *          sectorSizeCode)) / sideCount
 *
 *  If the sector attribute flag is non zero then the track count is
 *  determined by the more complex formula:
 *
 *      (fileLength - headerSize) / (sectorsPerTrack * ((128 <<
 *          sectorSizeCode) + 1) ) / sideCount
 *
 *  If the length of the header is to short to contain the geometry desired,
 *  then the default values are assumed. If the header length is zero the all
 *  of the geometry is assumed. When creating disk images it is desirable to
 *  make the header length as short as possible. The header should only be
 *  used to deviate from the default values.
 *
 *  The sector data begins immediately after the header. If the header length
 *  is zero then the sector data is at the beginning file.
 *
 *  C. Sectors per track
 *      This is the number of sectors per track (ones based). A value of 18
 *      means there are 18 sectors per track
 *
 *  D. Side Count
 *      This is the number of sides in the disk image. Values of 1 or 2 are
 *      acceptable. If there are two sides then the tracks are interleaved.
 *      The first track in the image file is track zero side 1, the second
 *      track in the image file is track zero side 2.
 *
 *  E. Sector size
 *      The is the same value that is stored in the wd179x ID field to
 *      determine sector size:
 *
 *          0x00         128 bytes
 *          0x01         256 bytes
 *          0x02         512 bytes
 *          0x03        1024 bytes
 *
 *  Other values are undefined. Every sector in the disk image must be the
 *  same size.
 *
 *  F. First sector ID
 *      This determines the first sector ID for each track. Each successive
 *      sector adds one to the previous ID. If the first sector ID is 1, then
 *      the second sector has an ID of 2, and the third has an ID of 3.
 *
 *  G. Sector Attribute Flag
 *      If this byte is non zero, then each sector contains an additional
 *      byte prepended to the sector data. If the attribute flag is zero then
 *      there are no extra bytes in front of the sector data.
 *
 *  H. Sector attribute byte
 *      This byte is put at the beginning of every sector if the header flag
 *      is turned on. The information this byte contains is the same as the
 *      status register (of the wd179x) would contain when a 'Read Sector'
 *      command was issued. The bit fields are defined as:
 *
 *      Bit position:
 *      ---------------
 *      7 6 5 4 3 2 1 0
 *      | | | | | | | |
 *      | | | | | | | +--- Not used. Set to zero.
 *      | | | | | | +----- Not used. Set to zero.
 *      | | | | | +------- Not used. Set to zero.
 *      | | | | +--------- Set on CRC error.
 *      | | | +----------- Set if sector not found.
 *      | | +------------- Record type: 1 - Deleted Data Mark, 0 - Data Mark.
 *      | +--------------- Not Used. Set to zero.
 *      +----------------- Not Used. Set to zero.
 *
 * ----------------------------------------------------------------------- */

static int coco_jvc_decode_header(floppy_image_legacy *floppy, UINT64 size,
	struct basicdsk_geometry *geometry)
{
	UINT8 header[256];
	UINT8 sector_attribute_flag;
	UINT16 physical_bytes_per_sector;
	UINT32 header_size, file_size;
	struct basicdsk_geometry dummy_geometry;

	if (geometry)
		memset(geometry, 0, sizeof(*geometry));
	else
		geometry = &dummy_geometry;

	if (size > 0xFFFFFFFF)
		return -1;
	file_size = (UINT32) size;

	/* read the header */
	header_size = (UINT32) file_size % 0x100;
	floppy_image_read(floppy, header, 0, header_size);
	geometry->offset = header_size;

	/* byte offset 0 - sectors per track */
	geometry->sectors = (header_size > 0) ? header[0] : 18;
	if (geometry->sectors <= 0)
		return -1;

	/* byte offset 1 - side count */
	geometry->heads = (header_size > 1) ? header[1] : 1;
	if (geometry->heads <= 0)
		return -1;

	/* byte offset 2 - sector size code */
	geometry->sector_length = 128 << ((header_size > 2) ? header[2] : 1);
	if (geometry->sector_length <= 0)
		return -1;

	/* byte offset 3 - first sector ID */
	geometry->first_sector_id = (header_size > 3) ? header[3] : 1;

	/* byte offset 4 - sector attribute flag */
	sector_attribute_flag = (header_size > 4) ? header[4] : 0;
	if (sector_attribute_flag != 0)
		return -1;  /* we do not support sector attribute flags */

	physical_bytes_per_sector = geometry->sector_length;
	if (sector_attribute_flag)
		physical_bytes_per_sector++;

	geometry->tracks = (file_size - header_size) / geometry->sectors / geometry->heads / physical_bytes_per_sector;

	/* do we have an oddball size?  reject this file if not */
	if ((file_size - header_size) % physical_bytes_per_sector)
		return -1;

	/* minimum of 35 tracks; support degenerate JVC files */
	if (geometry->tracks < 35)
		geometry->tracks = 35;

	return 0;
}



static FLOPPY_IDENTIFY(coco_jvc_identify)
{
	UINT64 size;
	size = floppy_image_size(floppy);
	*vote = coco_jvc_decode_header(floppy, size, NULL) ? 0 : 100;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_CONSTRUCT(coco_jvc_construct)
{
	struct basicdsk_geometry geometry;
	UINT8 header[5];
	size_t header_size;

	if (params)
	{
		/* create */
		memset(&geometry, 0, sizeof(geometry));
		geometry.heads              = option_resolution_lookup_int(params, PARAM_HEADS);
		geometry.tracks             = option_resolution_lookup_int(params, PARAM_TRACKS);
		geometry.sectors            = option_resolution_lookup_int(params, PARAM_SECTORS);
		geometry.first_sector_id    = option_resolution_lookup_int(params, PARAM_FIRST_SECTOR_ID);
		geometry.sector_length      = option_resolution_lookup_int(params, PARAM_SECTOR_LENGTH);

		header[0] = (UINT8) geometry.sectors;
		header[1] = (UINT8) geometry.heads;
		header[2] = (UINT8) compute_log2(geometry.sector_length) - 7;
		header[3] = (UINT8) geometry.first_sector_id;
		header[4] = 0;

		/* now that we have the header computed, figure out the header size */
		header_size = 0;
		if (header[0] != 18)
			header_size = 1;
		if (header[1] != 1)
			header_size = 2;
		if (header[2] != 1)
			header_size = 3;
		if (header[3] != 1)
			header_size = 4;
		if (header[4] != 0)
			header_size = 5;

		geometry.offset = header_size;

		floppy_image_write(floppy, header, 0, header_size);
	}
	else
	{
		/* load */
		if (coco_jvc_decode_header(floppy, floppy_image_size(floppy), &geometry))
			return FLOPPY_ERROR_INVALIDIMAGE;
	}
	return basicdsk_construct(floppy, &geometry);
}


/* -----------------------------------------------------------------------
 * OS-9 file format
 *
 * This file format is largely a hack because there are a large amount of
 * disk images that do not have geometry image separate from the disk image
 * itself.  So we support OS-9 images with are simply basic disks whose
 * geometry is determined by the disk image.
 *
 * OS-9 images identified by an LSN; which are simply blocks of 256 bytes
 *
 * LSN0
 * Byte    size    use
 * $00     3       sectors on disk
 * $03     1       track size in sectors
 * $04     2       bytes in allocation bit map; typically 1bit/sector so for
 *                 35 tracks of 18 sectors each that's $4E (single sided disk)
 *                 40 tracks per side, 18 sectors each = $B4
 * $06     2       sectors per bit in allocation map; normally 1
 * $08     3       LSN of root directory; normally 2 but depends on size of
 *                 $04 value
 * $0B     2       owner's user number; normally 0
 * $0D     1       disk attributes; normally $FF
 * $0E     2       psuedo random number for identification
 * $10     1       disk format; typical is 3
 *                 %00000001 0=single side 1=double side
 *                 %00000010 0=single density (non Coco) 1=double density
 *                 %00000100 0=48tracks/inch 1=96tracks/inch
 * $11     2       sectors per track; normal is $12 skip several not needed
 *                 for Format
 * $1A     5       date of creation Y:M:D:H:M
 * $1F     32      ASCII name of disk, last letter has $80 added to it,
 *                 the full 32 bytes do not need to be used.
 *
 * Allocation bit map, fill with zeros and set bits from low to high as
 * sectors are used. So, for a fresh disk sectors LSN0,LSN1,LSN2, and LSN3
 * will be in use so the first byte will be $FF $C0 and all others
 * in the map are $00
 *
 * Root directory LSN2
 * Byte    size     use
 * $00      1       attributes will be $BF
 * $01      2       owners ID will be $0000
 * $03      5       date last modified will be creation date  Y:M:D:H:M
 * $08      1       link count; set to $02
 * $09      4       file size in bytes, set to $40
 * $0D      3       date created Y:M:D
 * $10      3       block LSN set to current sector number+1 ie $03 in this case
 * $13      2       size in sectors of directory block, set to $07
 * All other bytes in sector set to $00
 * LSN3 first sector of directory with names
 * Fill sector with all $00 and then set listed bytes
 * $00      2       value $2EAE   which is .. with last byte+$80
 * $1F      1       value $02     LSN for start of this directory as there is
 *                  none higher in tree
 * $20      1       value $AE     which is . with $80 added
 * $3F      1       value $02     LSN for start of this directory
 * ----------------------------------------------------------------------- */

static floperr_t coco_os9_readheader(floppy_image_legacy *floppy, struct basicdsk_geometry *geometry)
{
	UINT8 header[0x20];
	int total_sectors;

	floppy_image_read(floppy, header, 0, sizeof(header));

	total_sectors = (header[0x00] << 16) | (header[0x01] << 8) | header[0x02];

	memset(geometry, 0, sizeof(*geometry));
	geometry->first_sector_id = 1;
	geometry->sector_length = 256;
	geometry->sectors = (header[0x11] << 8) + header[0x12];
	geometry->heads = (header[0x10] & 0x01) ? 2 : 1;

	if (!geometry->sectors)
		return FLOPPY_ERROR_INVALIDIMAGE;

	geometry->tracks = total_sectors / geometry->sectors / geometry->heads;

	if (total_sectors != geometry->tracks * geometry->sectors * geometry->heads)
		return FLOPPY_ERROR_INVALIDIMAGE;

	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t coco_os9_post_format(floppy_image_legacy *floppy, option_resolution *params)
{
	UINT8 header[0x0400];
	floperr_t err;
	time_t t;
	struct tm *ltime;
	int heads, tracks, sectors, total_sectors;

	heads   = option_resolution_lookup_int(params, PARAM_HEADS);
	tracks  = option_resolution_lookup_int(params, PARAM_TRACKS);
	sectors = option_resolution_lookup_int(params, PARAM_SECTORS);
	total_sectors = heads * tracks * sectors;

	/* write the initial header */
	time(&t);
	ltime = localtime(&t);

	memset(&header, 0, sizeof(header));
	header[0x0000] = (UINT8) (total_sectors >> 16);
	header[0x0001] = (UINT8) (total_sectors >>  8);
	header[0x0002] = (UINT8) (total_sectors >>  0);
	header[0x0003] = (UINT8) sectors;
	header[0x0004] = (UINT8) (((total_sectors + 7) / 8) >> 8);
	header[0x0005] = (UINT8) (((total_sectors + 7) / 8) >> 0);
	header[0x0006] = 0x00;
	header[0x0007] = 0x01;
	header[0x0008] = 0x00;
	header[0x0009] = 0x00;
	header[0x000a] = 0x02;
	header[0x000b] = 0x00;
	header[0x000c] = 0x00;
	header[0x000d] = 0xff;
	header[0x000e] = floppy_random_byte(floppy);
	header[0x000f] = floppy_random_byte(floppy);
	header[0x0010] = (heads == 2) ? 3 : 2;
	header[0x0011] = (UINT8) (sectors >> 8);
	header[0x0012] = (UINT8) (sectors >> 0);
	header[0x001A] = (UINT8) ltime->tm_year;
	header[0x001B] = (UINT8) ltime->tm_mon + 1;
	header[0x001C] = (UINT8) ltime->tm_mday;
	header[0x001D] = (UINT8) ltime->tm_hour;
	header[0x001E] = (UINT8) ltime->tm_min;
	header[0x001F] = 0xA0;
	header[0x0100] = 0xFF;
	header[0x0101] = 0xC0;
	header[0x0200] = 0xBF;
	header[0x0201] = 0x00;
	header[0x0202] = 0x00;
	header[0x0203] = (UINT8) ltime->tm_year;
	header[0x0204] = (UINT8) ltime->tm_mon + 1;
	header[0x0205] = (UINT8) ltime->tm_mday;
	header[0x0206] = (UINT8) ltime->tm_hour;
	header[0x0207] = (UINT8) ltime->tm_min;
	header[0x0208] = 0x02;
	header[0x0209] = 0x00;
	header[0x020A] = 0x00;
	header[0x020B] = 0x00;
	header[0x020C] = 0x40;
	header[0x020D] = (UINT8) (ltime->tm_year % 100);
	header[0x020E] = (UINT8) ltime->tm_mon;
	header[0x020F] = (UINT8) ltime->tm_mday;
	header[0x0210] = 0x00;
	header[0x0211] = 0x00;
	header[0x0212] = 0x03;
	header[0x0213] = 0x00;
	header[0x0214] = 0x07;
	header[0x0300] = 0x2E;
	header[0x0301] = 0xAE;
	header[0x031F] = 0x02;
	header[0x0320] = 0xAE;
	header[0x033F] = 0x02;

	if (total_sectors % 8)
		header[0x0100 + (total_sectors / 8)] = 0xFF >> (total_sectors % 8);

	err = floppy_write_sector(floppy, 0, 0, 1, 0, &header[0x0000], 256, 0);
	if (err)
		return err;

	err = floppy_write_sector(floppy, 0, 0, 2, 0, &header[0x0100], 256, 0);
	if (err)
		return err;

	err = floppy_write_sector(floppy, 0, 0, 3, 0, &header[0x0200], 256, 0);
	if (err)
		return err;

	err = floppy_write_sector(floppy, 0, 0, 4, 0, &header[0x0300], 256, 0);
	if (err)
		return err;

	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_IDENTIFY(coco_os9_identify)
{
	struct basicdsk_geometry geometry;
	*vote = coco_os9_readheader(floppy, &geometry) ? 0 : 100;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_CONSTRUCT(coco_os9_construct)
{
	floperr_t err;
	struct basicdsk_geometry geometry;

	if (params)
	{
		/* create */
		memset(&geometry, 0, sizeof(geometry));
		geometry.heads              = option_resolution_lookup_int(params, PARAM_HEADS);
		geometry.tracks             = option_resolution_lookup_int(params, PARAM_TRACKS);
		geometry.sectors            = option_resolution_lookup_int(params, PARAM_SECTORS);
		geometry.first_sector_id    = option_resolution_lookup_int(params, PARAM_FIRST_SECTOR_ID);
		geometry.sector_length      = option_resolution_lookup_int(params, PARAM_SECTOR_LENGTH);
	}
	else
	{
		/* open */
		err = coco_os9_readheader(floppy, &geometry);
		if (err)
			return err;
	}

	/* actually construct the image */
	err = basicdsk_construct(floppy, &geometry);
	floppy_callbacks(floppy)->post_format = coco_os9_post_format;
	return err;
}



/* -----------------------------------------------------------------------
 * VDK file format
 *
 * Used by Paul Burgin's PC-Dragon emulator
 *
 *  Offset  Bytes   Field           Description
 *  ------  -----   ------------    -----------
 *       0      1   magic1          Signature byte 1 ('d')
 *       1      1   magic2          Signature byte 2 ('k')
 *       2      2   header_len      Total header length (little endian)
 *       4      1   ver_actual      Version of the VDK format (0x10)
 *       5      1   ver_compat      Backwards compatibility version (0x10)
 *       6      1   source_id       Identify of the file source
 *       7      1   source_ver      Version of the file source
 *       8      1   tracks          Number of tracks
 *       9      1   sides           Number of sides (1-2)
 *      10      1   flags           Various flags
 *                                      bit 0: Write protect
 *                                      bit 1: A Lock
 *                                      bit 2: F Lock
 *                                      bit 3: Disk set
 *      11      1   compression     Compression flags (bits 0-2) and name length
 * ----------------------------------------------------------------------- */



static int coco_vdk_decode_header(floppy_image_legacy *floppy, struct basicdsk_geometry *geometry)
{
	UINT8 header[12];
	UINT8 heads, tracks, sectors;
	UINT16 sector_length, offset;
	UINT64 size;

	size = floppy_image_size(floppy);

	floppy_image_read(floppy, header, 0, sizeof(header));

	if (header[0] != 'd')
		return -1;
	if (header[1] != 'k')
		return -1;
	if (header[5] != 0x10)
		return -1;
	if (header[11] & 0x07)
		return -1;

	heads = header[9];
	tracks = header[8];
	sectors = 18;
	sector_length = 0x100;

	offset = header[3] * 0x100 + header[2];

	if (size != ((UINT32) heads * tracks * sectors * sector_length + offset))
		return -1;

	if (geometry)
	{
		memset(geometry, 0, sizeof(*geometry));
		geometry->heads = heads;
		geometry->tracks = tracks;
		geometry->sectors = sectors;
		geometry->first_sector_id = 1;
		geometry->sector_length = sector_length;
		geometry->offset = offset;
	}
	return 0;
}



static FLOPPY_IDENTIFY(coco_vdk_identify)
{
	*vote = coco_vdk_decode_header(floppy, NULL) ? 0 : 100;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_CONSTRUCT(coco_vdk_construct)
{
	struct basicdsk_geometry geometry;
	UINT8 header[12];

	if (params)
	{
		/* create */
		memset(&geometry, 0, sizeof(geometry));
		geometry.heads = option_resolution_lookup_int(params, PARAM_HEADS);
		geometry.tracks = option_resolution_lookup_int(params, PARAM_TRACKS);
		geometry.sectors = 18;
		geometry.first_sector_id = 1;
		geometry.sector_length = 256;
		geometry.offset = sizeof(header);

		memset(&header, 0, sizeof(header));
		header[0] = 'd';
		header[1] = 'k';
		header[2] = (UINT8) (sizeof(header) >> 0);
		header[3] = (UINT8) (sizeof(header) >> 8);
		header[4] = 0x10;
		header[5] = 0x10;
		header[8] = (UINT8) geometry.tracks;
		header[9] = (UINT8) geometry.heads;

		floppy_image_write(floppy, header, 0, sizeof(header));
	}
	else
	{
		/* load */
		if (coco_vdk_decode_header(floppy, &geometry))
			return FLOPPY_ERROR_INVALIDIMAGE;
	}
	return basicdsk_construct(floppy, &geometry);
}



/* -----------------------------------------------------------------------
 * DMK file format
 *
 * David M. Keil's disk image format is aptly called an 'on disk' image
 * format. This means that whatever written to the disk is enocded into
 * the image file. IDAMS, sector headers, traling CRCs, and intra sector
 * spacing.                                                         *
 *
 *  HEADER DESCRIPTION:
 *
 *  Offset  Bytes   Field           Description
 *  ------  -----   ------------    -----------
 *       0      1   write_prot      0xff = Writed Protected, 0x00 = R/W
 *       1      1   tracks          Number of tracks
 *       2      2   track_length    Bytes per track (little endian)
 *       4      1   disk_options    Miscellaneous flags
 *                                      bit 0-3:  Unused
 *                                      bit   4:  1=single sided 0=dbl
 *                                      bit   5:  Unused
 *                                      bit   6:  Single density?
 *                                      bit   6:  Ignore density flags?
 *       5      7   reserved        Reserved for future use
 *      12      4   real_disk_code  If this is 0x12345678 (little endian)
 *                                  then access a real disk drive
 *                                  (unsupported)
 *
 * Each track begins with a track TOC, consisting of 64 little endian 16-bit
 * integers.  Each integer has the following format:
 *      bit 0-13:   Offset from beginning of track to 'FE' byte of IDAM
 *                  Note these are always sorted from first to last. All empty
 *                  entries are 0x00
 *      bit 14:     Undefined (reserved)
 *      bit 15:     Sector double density (0=SD 1=DD)
 * ----------------------------------------------------------------------- */

struct dmk_tag
{
	int heads;
	int tracks;
	UINT32 track_size;
};

#define DMK_HEADER_LEN          16
#define DMK_TOC_LEN             64
#define DMK_IDAM_LENGTH         7
#define DMK_DATA_GAP            80
#define DMK_LEAD_IN             32
#define DMK_EXTRA_TRACK_LENGTH  156

#define dmk_idam_type(x)            (x)[0]
#define dmk_idam_track(x)           (x)[1]
#define dmk_idam_side(x)            (x)[2]
#define dmk_idam_sector(x)          (x)[3]
#define dmk_idam_sectorlength(x)    (x)[4]
#define dmk_idam_crc(x)             (((x)[5] << 8) + (x)[6])
#define dmk_idam_set_crc(x, crc)    (x)[5] = ((crc) >> 8); (x)[6] = ((crc) >> 0);


static struct dmk_tag *get_dmk_tag(floppy_image_legacy *floppy)
{
	return (dmk_tag *)floppy_tag(floppy);
}


static floperr_t coco_dmk_get_offset(floppy_image_legacy *floppy, int head, int track, UINT64 *offset)
{
	struct dmk_tag *tag = get_dmk_tag(floppy);

	if ((head < 0) || (head >= tag->heads) || (track < 0) || (track >= tag->tracks))
		return FLOPPY_ERROR_SEEKERROR;

	*offset = track;
	*offset *= tag->heads;
	*offset += head;
	*offset *= tag->track_size;
	*offset += DMK_HEADER_LEN;
	return FLOPPY_ERROR_SUCCESS;
}



static UINT32 coco_dmk_min_track_size(int sectors, int sector_length)
{
	int sector_physical_length;
	sector_physical_length = 8 + 3 + DMK_IDAM_LENGTH + 22 + 12 + 3 + 1 + sector_length + 2 + 24;
	return DMK_TOC_LEN * 2 + DMK_LEAD_IN + (sectors * sector_physical_length);
}



static floperr_t coco_dmk_read_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buflen)
{
	floperr_t err;
	UINT64 track_offset;

	err = coco_dmk_get_offset(floppy, head, track, &track_offset);
	if (err)
		return err;

	floppy_image_read(floppy, buffer, offset + track_offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t coco_dmk_write_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, const void *buffer, size_t buflen)
{
	floperr_t err;
	UINT64 track_offset;

	err = coco_dmk_get_offset(floppy, head, track, &track_offset);
	if (err)
		return err;

	floppy_image_write(floppy, buffer, offset + track_offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t coco_dmk_get_track_data_offset(floppy_image_legacy *floppy, int head, int track, UINT64 *offset)
{
	*offset = DMK_TOC_LEN + 1;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t coco_dmk_format_track(floppy_image_legacy *floppy, int head, int track, option_resolution *params)
{
	int sectors;
	int sector_length;
	int interleave;
	int first_sector_id;
	floperr_t err;
	int physical_sector;
	int logical_sector;
	int track_position;
	UINT16 idam_offset;
	UINT16 crc;
	UINT8 *track_data;
	void *track_data_v;
	UINT32 max_track_size;
	std::vector<int> sector_map;

	sectors         = option_resolution_lookup_int(params, PARAM_SECTORS);
	sector_length   = option_resolution_lookup_int(params, PARAM_SECTOR_LENGTH);
	interleave      = option_resolution_lookup_int(params, PARAM_INTERLEAVE);
	first_sector_id = option_resolution_lookup_int(params, PARAM_FIRST_SECTOR_ID);

	max_track_size = get_dmk_tag(floppy)->track_size;

	if (sectors > DMK_TOC_LEN)
	{
		err = FLOPPY_ERROR_INTERNAL;
		goto done;
	}

	if (max_track_size < coco_dmk_min_track_size(sectors, sector_length))
	{
		err = FLOPPY_ERROR_NOSPACE;
		goto done;
	}

	err = floppy_load_track(floppy, head, track, TRUE, &track_data_v, NULL);
	if (err)
		goto done;
	track_data = (UINT8 *) track_data_v;

	/* set up sector map */
	memset(&sector_map[0], 0xff, sectors*sizeof(int));

	physical_sector = 0;
	for (logical_sector = 0; logical_sector < sectors; logical_sector++)
	{
		while(sector_map[physical_sector] >= 0)
		{
			physical_sector++;
			physical_sector %= sectors;
		}

		sector_map[physical_sector] = logical_sector + first_sector_id;
		physical_sector += interleave + 1;
		physical_sector %= sectors;
	}

	/* set up track table of contents */
	physical_sector = 0;
	track_position = DMK_TOC_LEN * 2 + DMK_LEAD_IN;
	while(physical_sector < DMK_TOC_LEN)
	{
		if (physical_sector >= sectors)
		{
			/* no more sectors */
			idam_offset = 0;
		}
		else
		{
			/* this is a sector */
			logical_sector = sector_map[physical_sector];

			/* write the sector */
			memset(&track_data[track_position], 0x00, 8);
			track_position += 8;

			memset(&track_data[track_position], 0xA1, 3);
			track_position += 3;

			idam_offset = track_position | 0x8000;
			dmk_idam_type(              &track_data[track_position]) = 0xFE;
			dmk_idam_track(             &track_data[track_position]) = track;
			dmk_idam_side(              &track_data[track_position]) = head;
			dmk_idam_sector(            &track_data[track_position]) = logical_sector;
			dmk_idam_sectorlength(      &track_data[track_position]) = compute_log2(sector_length / 128);
			crc = ccitt_crc16(0xcdb4,   &track_data[track_position], DMK_IDAM_LENGTH - 2);
			dmk_idam_set_crc(           &track_data[track_position], crc);
			track_position += DMK_IDAM_LENGTH;

			memset(&track_data[track_position], 0x4E, 22);
			track_position += 22;

			memset(&track_data[track_position], 0x00, 12);
			track_position += 12;

			memset(&track_data[track_position], 0xA1, 3);
			track_position += 3;

			/* write sector body */
			track_data[track_position] = 0xFB;
			memset(&track_data[track_position + 1], floppy_get_filler(floppy), sector_length);
			crc = ccitt_crc16(0xcdb4, &track_data[track_position], sector_length + 1);
			track_data[track_position + sector_length + 1] = (UINT8) (crc >> 8);
			track_data[track_position + sector_length + 2] = (UINT8) (crc >> 0);
			track_position += sector_length + 3;

			/* write sector footer */
			memset(&track_data[track_position], 0x4E, 24);
			track_position += 24;
		}

		/* write the TOC entry */
		track_data[physical_sector * 2 + 0] = (UINT8) (idam_offset >> 0);
		track_data[physical_sector * 2 + 1] = (UINT8) (idam_offset >> 8);

		physical_sector++;
	}

	/* write track lead in */
	memset(&track_data[physical_sector * 2], 0x4e, DMK_LEAD_IN);

	/* write track footer */
	assert(max_track_size >= (UINT32)track_position);
	memset(&track_data[track_position], 0x4e, max_track_size - track_position);

done:
	return err;
}



static int coco_dmk_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_dmk_tag(floppy)->heads;
}



static int coco_dmk_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return get_dmk_tag(floppy)->tracks;
}



static UINT32 coco_dmk_get_track_size(floppy_image_legacy *floppy, int head, int track)
{
	return get_dmk_tag(floppy)->track_size;
}



static floperr_t coco_dmk_seek_sector_in_track(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, int dirtify, UINT8 **sector_data, UINT32 *sector_length)
{
	struct dmk_tag *tag = get_dmk_tag(floppy);
	floperr_t err;
	UINT32 idam_offset = 0;
	UINT16 calculated_crc;
	size_t i;
	size_t offs;
	int state;
	UINT8 *track_data;
	void *track_data_v;
	size_t track_length;
	size_t sec_len;

	err = floppy_load_track(floppy, head, track, dirtify, &track_data_v, &track_length);
	if (err)
		return err;
	track_data = (UINT8 *) track_data_v;

	/* search for matching IDAM */
	for (i = 0; i < DMK_TOC_LEN; i++)
	{
		idam_offset = track_data[i * 2 + 1];
		idam_offset <<= 8;
		idam_offset |= track_data[i * 2 + 0];
		idam_offset &= 0x3FFF;

		if (idam_offset == 0)
		{
			/* we've reached the end of the road */
			i = DMK_TOC_LEN;
			break;
		}

		if ((idam_offset + DMK_IDAM_LENGTH) >= tag->track_size)
			continue;

		calculated_crc = ccitt_crc16(0xCDB4, &track_data[idam_offset], DMK_IDAM_LENGTH - 2);

		if (calculated_crc == dmk_idam_crc(&track_data[idam_offset]))
		{
			if (sector_is_index)
			{
				/* the sector is indexed; decrement the index and go */
				if (sector-- == 0)
					break;
			}
			else
			{
				/* check IDAM integrity and check for matching sector */
				if (sector == dmk_idam_sector(&track_data[idam_offset])
/*                  && (track == dmk_idam_track(&track_data[idam_offset]))  */
/*                  && (head == dmk_idam_side(&track_data[idam_offset]))    */
					)
					break;
			}
		}
	}

	if (i >= DMK_TOC_LEN)
		return FLOPPY_ERROR_SEEKERROR;

	/* we found a matching sector ID */
	state = 0;
	offs = idam_offset + DMK_IDAM_LENGTH;

	/* find pattern 0xA1A1FB; this represents the start of a data sector */
	for (i = 0; i < DMK_DATA_GAP; i++)
	{
		/* overflowing the track? */
		if ((i + offs) >= tag->track_size)
			return FLOPPY_ERROR_SEEKERROR;

		if (track_data[offs + i] == 0xA1)
			state++;
		else if ((track_data[offs + i] == 0xFB) && state)
			break;
		else
			state = 0;
	}
	if (i >= DMK_DATA_GAP)
		return FLOPPY_ERROR_SEEKERROR;

	offs += i + 1;
	sec_len = 128 << dmk_idam_sectorlength(&track_data[idam_offset]);

	if ((offs + sec_len) > track_length)
		return FLOPPY_ERROR_INVALIDIMAGE;

	if (sector_data)
		*sector_data = track_data + offs;
	if (sector_length)
		*sector_length = sec_len;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t coco_dmk_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	return coco_dmk_seek_sector_in_track(floppy, head, track, sector, FALSE, FALSE, NULL, sector_length);
}



static floperr_t coco_dmk_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
{
	floperr_t err;
	UINT32 idam_offset;
	const UINT8 *track_data;
	void *track_data_v;

	if (sector_index*2 >= DMK_TOC_LEN)
		return FLOPPY_ERROR_SEEKERROR;

	err = floppy_load_track(floppy, head, track, FALSE, &track_data_v, NULL);
	if (err)
		return err;
	track_data = (UINT8 *) track_data_v;

	idam_offset = track_data[sector_index * 2 + 1];
	idam_offset <<= 8;
	idam_offset |= track_data[sector_index * 2 + 0];
	idam_offset &= 0x3FFF;

	if (idam_offset == 0)
		return FLOPPY_ERROR_SEEKERROR;

	if (cylinder)
		*cylinder = dmk_idam_track(&track_data[idam_offset]);
	if (side)
		*side = dmk_idam_side(&track_data[idam_offset]);
	if (sector)
		*sector = dmk_idam_sector(&track_data[idam_offset]);
	if (sector_length)
		*sector_length = 128 << dmk_idam_sectorlength(&track_data[idam_offset]);
	if (flags)
		/* TODO: read DAM or DDAM and determine flags */
		*flags = 0;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_coco_dmk_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, void *buffer, size_t buflen)
{
	floperr_t err;
	UINT32 sector_length;
	UINT16 crc_on_disk;
	UINT16 calculated_crc;
	UINT8 *sector_data;

	err = coco_dmk_seek_sector_in_track(floppy, head, track, sector, sector_is_index, FALSE, &sector_data, &sector_length);
	if (err)
		return err;

	crc_on_disk = sector_data[sector_length + 0];
	crc_on_disk <<= 8;
	crc_on_disk += sector_data[sector_length + 1];

	calculated_crc = ccitt_crc16(0xE295, sector_data, sector_length);
	if (calculated_crc != crc_on_disk)
		return FLOPPY_ERROR_INVALIDIMAGE;

	memcpy(buffer, sector_data, MIN(sector_length, buflen));

	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_coco_dmk_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, const void *buffer, size_t buflen, int ddam)
{
	floperr_t err;
	UINT32 sector_length;
	UINT8 *sector_data;
	UINT16 crc;

	err = coco_dmk_seek_sector_in_track(floppy, head, track, sector, sector_is_index, TRUE, &sector_data, &sector_length);
	if (err)
		return err;

	if (buflen > sector_length)
		return FLOPPY_ERROR_INTERNAL;

	memcpy(sector_data, buffer, buflen);

	crc = ccitt_crc16(0xE295, sector_data, sector_length);
	sector_data[sector_length + 0] = crc >> 8;
	sector_data[sector_length + 1] = crc >> 0;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t coco_dmk_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_coco_dmk_read_sector(floppy, head, track, sector, FALSE, buffer, buflen);
}

static floperr_t coco_dmk_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_coco_dmk_write_sector(floppy, head, track, sector, FALSE, buffer, buflen, ddam);
}

static floperr_t coco_dmk_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_coco_dmk_read_sector(floppy, head, track, sector, TRUE, buffer, buflen);
}

static floperr_t coco_dmk_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_coco_dmk_write_sector(floppy, head, track, sector, TRUE, buffer, buflen, ddam);
}



static void coco_dmk_interpret_header(floppy_image_legacy *floppy, int *heads, int *tracks, int *track_size)
{
	UINT8 header[DMK_HEADER_LEN];

	floppy_image_read(floppy, header, 0, DMK_HEADER_LEN);

	if (tracks)
		*tracks = header[1];
	if (heads)
		*heads = (header[4] & 0x10) ? 1 : 2;
	if (track_size)
		*track_size = ((int) header[3]) * 0x100 + header[2];
}



FLOPPY_CONSTRUCT(coco_dmk_construct)
{
	struct FloppyCallbacks *callbacks;
	struct dmk_tag *tag;
	UINT8 header[DMK_HEADER_LEN];
	int heads, tracks, track_size, sectors, sector_length;

	if (params)
	{
		heads           = option_resolution_lookup_int(params, PARAM_HEADS);
		tracks          = option_resolution_lookup_int(params, PARAM_TRACKS);
		sectors         = option_resolution_lookup_int(params, PARAM_SECTORS);
		sector_length   = option_resolution_lookup_int(params, PARAM_SECTOR_LENGTH);

		track_size = coco_dmk_min_track_size(sectors, sector_length) + DMK_EXTRA_TRACK_LENGTH;

		memset(header, 0, sizeof(header));
		header[1] = tracks;
		header[2] = track_size >> 0;
		header[3] = track_size >> 8;
		header[4] = (heads == 2) ? 0x00 : 0x10;

		floppy_image_write(floppy, header, 0, sizeof(header));
	}
	else
	{
		coco_dmk_interpret_header(floppy, &heads, &tracks, &track_size);
	}

	tag = (dmk_tag *)floppy_create_tag(floppy, sizeof(struct dmk_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;
	tag->heads = heads;
	tag->track_size = track_size;
	tag->tracks = tracks;

	callbacks = floppy_callbacks(floppy);
	callbacks->read_track = coco_dmk_read_track;
	callbacks->write_track = coco_dmk_write_track;
	callbacks->get_track_data_offset = coco_dmk_get_track_data_offset;
	callbacks->format_track = coco_dmk_format_track;
	callbacks->get_heads_per_disk = coco_dmk_get_heads_per_disk;
	callbacks->get_tracks_per_disk = coco_dmk_get_tracks_per_disk;
	callbacks->get_track_size = coco_dmk_get_track_size;
	callbacks->get_sector_length = coco_dmk_get_sector_length;
	callbacks->get_indexed_sector_info = coco_dmk_get_indexed_sector_info;
	callbacks->read_sector = coco_dmk_read_sector;
	callbacks->write_sector = coco_dmk_write_sector;
	callbacks->read_indexed_sector = coco_dmk_read_indexed_sector;
	callbacks->write_indexed_sector = coco_dmk_write_indexed_sector;

	return FLOPPY_ERROR_SUCCESS;
}



FLOPPY_IDENTIFY(coco_dmk_identify)
{
	int heads, tracks, track_size;
	UINT64 size, expected_size;

	size = floppy_image_size(floppy);
	coco_dmk_interpret_header(floppy, &heads, &tracks, &track_size);
	expected_size = DMK_HEADER_LEN + (heads * tracks * track_size);
	*vote = (size == expected_size) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}



/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( coco )
	LEGACY_FLOPPY_OPTION( coco_jvc, "dsk",          "CoCo JVC disk image",  coco_jvc_identify,  coco_jvc_construct, NULL,
		HEADS([1]-2)
		TRACKS([35]-255)
		SECTORS(1-[18]-255)
		SECTOR_LENGTH(128/[256]/512/1024)
		FIRST_SECTOR_ID(0-[1]))
	LEGACY_FLOPPY_OPTION( coco_os9, "os9",          "CoCo OS-9 disk image", coco_os9_identify,  coco_os9_construct, NULL,
		HEADS([1]-2)
		TRACKS([35]-255)
		SECTORS(1-[18]-255)
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION( coco_vdk, "vdk",          "CoCo VDK disk image",  coco_vdk_identify,  coco_vdk_construct, NULL,
		HEADS([1]-2)
		TRACKS([35]-255)
		SECTORS([18])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION( coco_dmk, "dsk,dmk",  "CoCo DMK disk image",  coco_dmk_identify,  coco_dmk_construct, NULL,
		HEADS([1]-2)
		TRACKS([35]-255)
		SECTORS(1-[18])
		SECTOR_LENGTH(128/[256]/512/1024/2048/4096/8192)
		INTERLEAVE(0-[6]-17)
		FIRST_SECTOR_ID(0-[1]))
LEGACY_FLOPPY_OPTIONS_END
