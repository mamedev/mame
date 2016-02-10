// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont
/*********************************************************************

    ap_dsk35.c

    Apple 3.5" disk images

    This code supports 3.5" 400k/800k disks used in early Macintoshes
    and the Apple IIgs, and 3.5" 1440k MFM disks used on most Macintoshes.

    These disks have the following properties:

        400k:   80 tracks, 1 head
        800k:   80 tracks, 2 heads

        Tracks  0-15 have 12 sectors each
        Tracks 16-31 have 11 sectors each
        Tracks 32-47 have 10 sectors each
        Tracks 48-63 have  9 sectors each
        Tracks 64-79 have  8 sectors each

        1440k disks are simply 80 tracks, 2 heads and 18 sectors per track.

    Each sector on 400/800k disks has 524 bytes, 512 of which are really used by the Macintosh

    (80 tracks) * (avg of 10 sectors) * (512 bytes) * (2 sides) = 800 kB

    Data is nibblized : 3 data bytes -> 4 bytes on disk.

    In addition to 512 logical bytes, each sector contains 800 physical
    bytes.  Here is the layout of the physical sector:

        Pos
        0       0xFF (pad byte where head is turned on ???)
        1-35    self synch 0xFFs (7*5) (42 bytes actually written to media)
        36      0xD5
        37      0xAA
        38      0x96
        39      diskbytes[(track number) & 0x3F]
        40      diskbytes[(sector number)]
        41      diskbytes[("side")]
        42      diskbytes[("format byte")]
        43      diskbytes[("sum")]
        44      0xDE
        45      0xAA
        46      pad byte where head is turned off/on (0xFF here)
        47-51   self synch 0xFFs (6 bytes actually written to media)
        52      0xD5
        53      0xAA
        54      0xAD
        55      spare byte, generally diskbytes[(sector number)]
        56-754  "nibblized" sector data ...
        755-758 checksum
        759     0xDE
        760     0xAA
        761     pad byte where head is turned off (0xFF here)

    MFM Track layout for 1440K disks:
        Pos
    --------- track ID
    0   0x4E (x80)
    80  00 (x12)
    92  C2 (x3) Mark byte
    93  FC Index mark
    94  4E (x50)
    --------- sector ID
    144 00 (x12)
    156 A1 (x3) Mark byte
    159 FE Address mark
    160 xx Track number
    161 xx Side number
    162 xx Sector number
    163 xx Sector length
    164/165 xx CRC
    166 4E (x22)
    --------- sector data
    188 00 (x12)
    200 A1 (x3) Mark byte
    203 FB Data address mark
    204 xx (x256) data
    460 4E (x54)
    (repeat from sector ID to fill track; end of track is padded with 4E)

    Note : "Self synch refers to a technique whereby two zeroes are inserted
    between each synch byte written to the disk.", i.e. "0xFF, 0xFF, 0xFF,
    0xFF, 0xFF" is actually "0xFF, 0x3F, 0xCF, 0xF3, 0xFC, 0xFF" on disk.
    Since the IWM assumes the data transfer is complete when the MSBit of its
    shift register is 1, we do read 4 0xFF, even though they are not
    contiguous on the disk media.  Some reflexion shows that 4 synch bytes
    allow the IWM to synchronize with the trailing data.

    Format byte codes:
        0x00    Apple II
        0x01    Lisa
        0x02    Mac MFS (single sided)?
        0x22    Mac MFS (double sided)?

*********************************************************************/

#include <stdio.h>
#include <assert.h>

#include "emu.h" // logerror
#include "ap_dsk35.h"

struct apple35_tag
{
	UINT32 data_offset;
	UINT32 data_size;
	UINT8 format_byte;
	UINT8 sides;
	unsigned int is_1440k : 1;

	/* stuff used in DiskCopy images */
	UINT32 tag_offset;
	UINT32 tag_size;
};



/* normal number of sector for each track */
static const UINT8 apple35_tracklen_800kb[80] =
{
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
		9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
		8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8
};

/* blocks of data used to nibblize tracks */
static const UINT8 diskbytes[] =
{
	0x96, 0x97, 0x9A, 0x9B,  0x9D, 0x9E, 0x9F, 0xA6, /* 0x00 */
	0xA7, 0xAB, 0xAC, 0xAD,  0xAE, 0xAF, 0xB2, 0xB3,
	0xB4, 0xB5, 0xB6, 0xB7,  0xB9, 0xBA, 0xBB, 0xBC, /* 0x10 */
	0xBD, 0xBE, 0xBF, 0xCB,  0xCD, 0xCE, 0xCF, 0xD3,
	0xD6, 0xD7, 0xD9, 0xDA,  0xDB, 0xDC, 0xDD, 0xDE, /* 0x20 */
	0xDF, 0xE5, 0xE6, 0xE7,  0xE9, 0xEA, 0xEB, 0xEC,
	0xED, 0xEE, 0xEF, 0xF2,  0xF3, 0xF4, 0xF5, 0xF6, /* 0x30 */
	0xF7, 0xF9, 0xFA, 0xFB,  0xFC, 0xFD, 0xFE, 0xFF
};

/* reverse lookup of diskbytes */
static const INT16 rev_diskbytes[] =
{
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,   /* 0x00 */
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,   /* 0x10 */
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,   /* 0x20 */
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,   /* 0x30 */
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,   /* 0x40 */
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,   /* 0x50 */
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,   /* 0x60 */
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,   /* 0x70 */
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,   /* 0x80 */
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,    -1,   -1,   0x00, 0x01, /* 0x90 */
	-1,   -1,   0x02, 0x03,  -1,   0x04, 0x05, 0x06,
	-1,   -1,   -1,   -1,    -1,   -1,   0x07, 0x08, /* 0xA0 */
	-1,   -1,   -1,   0x09,  0x0A, 0x0B, 0x0C, 0x0D,
	-1,   -1,   0x0E, 0x0F,  0x10, 0x11, 0x12, 0x13, /* 0xB0 */
	-1,   0x14, 0x15, 0x16,  0x17, 0x18, 0x19, 0x1A,
	-1,   -1,   -1,   -1,    -1,   -1,   -1,   -1,   /* 0xC0 */
	-1,   -1,   -1,   0x1B,  -1,   0x1C, 0x1D, 0x1E,
	-1,   -1,   -1,   0x1F,  -1,   -1,   0x20, 0x21, /* 0xD0 */
	-1,   0x22, 0x23, 0x24,  0x25, 0x26, 0x27, 0x28,
	-1,   -1,   -1,   -1,    -1,   0x29, 0x2A, 0x2B, /* 0xE0 */
	-1,   0x2C, 0x2D, 0x2E,  0x2F, 0x30, 0x31, 0x32,
	-1,   -1,   0x33, 0x34,  0x35, 0x36, 0x37, 0x38, /* 0xF0 */
	-1,   0x39, 0x3A, 0x3B,  0x3C, 0x3D, 0x3E, 0x3F
};

static const UINT8 blk1[] =
{
	/*0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xD5, 0xAA, 0x96*/
	0xFF,
	0xFF, 0x3F, 0xCF, 0xF3, 0xFC, 0xFF,
	0xFF, 0x3F, 0xCF, 0xF3, 0xFC, 0xFF,
	0xFF, 0x3F, 0xCF, 0xF3, 0xFC, 0xFF,
	0xFF, 0x3F, 0xCF, 0xF3, 0xFC, 0xFF,
	0xFF, 0x3F, 0xCF, 0xF3, 0xFC, 0xFF,
	0xFF, 0x3F, 0xCF, 0xF3, 0xFC, 0xFF,
	0xFF, 0x3F, 0xCF, 0xF3, 0xFC, 0xFF,
	0xD5, 0xAA, 0x96
};
static const UINT8 blk2[] =
{
	0xDE, 0xAA, 0xFF, 0xFF, 0x3F, 0xCF, 0xF3, 0xFC, 0xFF, 0xD5, 0xAA, 0xAD
};
static const UINT8 blk3[] =
{
	0xDE, 0xAA, 0xFF
};



static struct apple35_tag *get_apple35_tag(floppy_image_legacy *floppy)
{
	struct apple35_tag *tag;
	tag = (struct apple35_tag *) floppy_tag(floppy);
	return tag;
}



int apple35_sectors_per_track(floppy_image_legacy *image, int track)
{
	int sectors;

	assert(track >= 0);
	assert(track < ARRAY_LENGTH(apple35_tracklen_800kb));

	if (get_apple35_tag(image)->is_1440k)
		sectors = 18;
	else
		sectors = apple35_tracklen_800kb[track];
	return sectors;
}



/*
    converts data to its nibblized representation, and generate checksum

    now handles 524-byte-long sectors

    tag data IS important, since it allows data recovery when the catalog is trashed
*/
static void sony_nibblize35(const UINT8 *in, UINT8 *nib_ptr, UINT8 *csum)
{
	int i, j;
	UINT32 c1, c2, c3, c4;
	UINT8 val;
	UINT8 w1, w2, w3, w4;
	UINT8 b1[175], b2[175], b3[175];

	/* Copy from the user's buffer to our buffer, while computing
	 * the three-byte data checksum
	 */

	i = 0;
	j = 0;
	c1 = 0;
	c2 = 0;
	c3 = 0;
	while (1)
	{
		c1 = (c1 & 0xFF) << 1;
		if (c1 & 0x0100)
			c1++;

		val = in[i++];
		c3 += val;
		if (c1 & 0x0100)
		{
			c3++;
			c1 &= 0xFF;
		}
		b1[j] = (val ^ c1) & 0xFF;

		val = in[i++];
		c2 += val;
		if (c3 > 0xFF)
		{
			c2++;
			c3 &= 0xFF;
		}
		b2[j] = (val ^ c3) & 0xFF;

		if (i == 524) break;

		val = in[i++];
		c1 += val;
		if (c2 > 0xFF)
		{
			c1++;
			c2 &= 0xFF;
		}
		b3[j] = (val ^ c2) & 0xFF;
		j++;
	}
	c4 =  ((c1 & 0xC0) >> 6) | ((c2 & 0xC0) >> 4) | ((c3 & 0xC0) >> 2);
	b3[174] = 0;

	j = 0;
	for (i = 0; i <= 174; i++)
	{
		w1 = b1[i] & 0x3F;
		w2 = b2[i] & 0x3F;
		w3 = b3[i] & 0x3F;
		w4 =  ((b1[i] & 0xC0) >> 2);
		w4 |= ((b2[i] & 0xC0) >> 4);
		w4 |= ((b3[i] & 0xC0) >> 6);

		nib_ptr[j++] = w4;
		nib_ptr[j++] = w1;
		nib_ptr[j++] = w2;

		if (i != 174) nib_ptr[j++] = w3;
	}

	csum[0] = c1 & 0x3F;
	csum[1] = c2 & 0x3F;
	csum[2] = c3 & 0x3F;
	csum[3] = c4 & 0x3F;
}



/*
    does the reverse process of sony_nibblize35
*/
static void sony_denibblize35(UINT8 *out, const UINT8 *nib_ptr, UINT8 *checksum)
{
	int i, j;
	UINT32 c1,c2,c3,c4;
	UINT8 val;
	UINT8 w1,w2,w3=0,w4;
	UINT8 b1[175],b2[175],b3[175];

	j = 0;
	for (i=0; i<=174; i++)
	{
		w4 = nib_ptr[j++];
		w1 = nib_ptr[j++];
		w2 = nib_ptr[j++];

		if (i != 174) w3 = nib_ptr[j++];

		b1[i] = (w1 & 0x3F) | ((w4 << 2) & 0xC0);
		b2[i] = (w2 & 0x3F) | ((w4 << 4) & 0xC0);
		b3[i] = (w3 & 0x3F) | ((w4 << 6) & 0xC0);
	}

	/* Copy from the user's buffer to our buffer, while computing
	 * the three-byte data checksum
	 */

	i = 0;
	j = 0;
	c1 = 0;
	c2 = 0;
	c3 = 0;
	while (1)
	{
		c1 = (c1 & 0xFF) << 1;
		if (c1 & 0x0100)
			c1++;

		val = (b1[j] ^ c1) & 0xFF;
		c3 += val;
		if (c1 & 0x0100)
		{
			c3++;
			c1 &= 0xFF;
		}
		out[i++] = val;

		val = (b2[j] ^ c3) & 0xFF;
		c2 += val;
		if (c3 > 0xFF)
		{
			c2++;
			c3 &= 0xFF;
		}
		out[i++] = val;

		if (i == 524) break;

		val = (b3[j] ^ c2) & 0xFF;
		c1 += val;
		if (c2 > 0xFF)
		{
			c1++;
			c2 &= 0xFF;
		}
		out[i++] = val;
		j++;
	}
	c4 =  ((c1 & 0xC0) >> 6) | ((c2 & 0xC0) >> 4) | ((c3 & 0xC0) >> 2);
	b3[174] = 0;

	checksum[0] = c1 & 0x3F;
	checksum[1] = c2 & 0x3F;
	checksum[2] = c3 & 0x3F;
	checksum[3] = c4 & 0x3F;
}



void sony_filltrack(UINT8 *buffer, size_t buffer_len, size_t *pos, UINT8 data)
{
	buffer[*pos / 8] &= 0xFF << (8 - (*pos % 8));
	buffer[*pos / 8] |= data >> (*pos % 8);

	*pos += 8;
	*pos %= buffer_len * 8;

	buffer[*pos / 8] &= 0xFF >> (*pos % 8);
	buffer[*pos / 8] |= data << (8 - (*pos % 8));
}



UINT8 sony_fetchtrack(const UINT8 *buffer, size_t buffer_len, size_t *pos)
{
	UINT8 data;

	data = buffer[*pos / 8] << (*pos % 8);
	*pos += 8;
	*pos %= (buffer_len * 8);
	data |= buffer[*pos / 8] >> (8 - (*pos % 8));

	while ((data & 0x80) == 0)
	{
		/* this code looks weird because it isn't simply rotating the new bit
		 * in, but for some reason it won't work if I rotate the bit in; I
		 * have to match the algorithm used by the old code */
		data <<= 1;
		data |= (buffer[*pos / 8] >> (8 - ((*pos % 8) + 1)));
		(*pos)++;
		*pos %= (buffer_len * 8);
	}

//    printf("sony_fetchtrack: pos %ld = %02x\n", *pos/8, data);

	return data;
}



static UINT32 apple35_get_offset(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *tag_offset)
{
	int i;
	UINT32 sector_index = 0;
	struct apple35_tag *tag;

	tag = get_apple35_tag(floppy);

	if (track >= ARRAY_LENGTH(apple35_tracklen_800kb))
		return ~0;
	if (head >= tag->sides)
		return ~0;
	if (sector >= apple35_sectors_per_track(floppy, track))
		return ~0;

	for (i = 0; i < track; i++)
		sector_index += apple35_sectors_per_track(floppy, i);
	sector_index *= tag->sides;
	if (head)
		sector_index += apple35_sectors_per_track(floppy, i);
	sector_index += sector;

	if (tag_offset)
	{
		*tag_offset = sector_index * 12;
		if (*tag_offset >= tag->tag_size)
		{
			*tag_offset = ~0;
		}
		else
		{
			*tag_offset += tag->tag_offset;
		}
	}
	return sector_index * 0x200 + tag->data_offset;
}



static floperr_t apple35_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	UINT32 data_offset;
	data_offset = apple35_get_offset(floppy, head, track, sector, nullptr);
	if (data_offset == ~0)
	{
		return FLOPPY_ERROR_SEEKERROR;
	}
	floppy_image_read(floppy, buffer, data_offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple35_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	UINT32 data_offset;

	data_offset = apple35_get_offset(floppy, head, track, sector, nullptr);
	if (data_offset == ~0)
		return FLOPPY_ERROR_SEEKERROR;
	floppy_image_write(floppy, buffer, data_offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple35_read_sector_td(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	floperr_t err;
	UINT32 tag_offset = 0;

	assert(buflen == 524);

	/* first read the sector */
	err = apple35_read_sector(floppy, head, track, sector, ((UINT8 *) buffer) + 12, 512);
	if (err)
	{
		return err;
	}

	/* read the tag data, if possible */
	memset(buffer, '\0', 12);
	apple35_get_offset(floppy, head, track, sector, &tag_offset);
	if (tag_offset != ~0)
	{
		floppy_image_read(floppy, buffer, tag_offset, 12);
	}

	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple35_write_sector_td(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	floperr_t err;
	UINT32 tag_offset = 0;

	assert(buflen == 524);

	/* first write the sector */
	err = apple35_write_sector(floppy, head, track, sector, ((const UINT8 *) buffer) + 12, 512, 0);
	if (err)
		return err;

	/* write the tag data, if possible */
	apple35_get_offset(floppy, head, track, sector, &tag_offset);
	if (tag_offset != ~0)
		floppy_image_write(floppy, buffer, tag_offset, 12);

	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple35_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	*sector_length = 512;
	return FLOPPY_ERROR_SUCCESS;
}



static int apple35_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_apple35_tag(floppy)->sides;
}



static int apple35_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return 80;
}



static UINT32 apple35_get_track_size(floppy_image_legacy *floppy, int head, int track)
{
	if ((track < 0) || (track >= 80))
		return 0;
	return apple35_sectors_per_track(floppy, track) * 800;
}



static UINT8 calculate_side(int head, int track)
{
	UINT8 side;
	side = head ? 0x20 : 0x00;
	if (track & 0x40)
		side |= 0x01;
	return side;
}



static floperr_t apple35_read_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buflen)
{
	floperr_t err;
	size_t pos = 0;
	int sector_count, sector, i;
	UINT8 sum, side;
	struct apple35_tag *tag;
	UINT8 sector_data[524];
	UINT8 nibble_data[699];
	UINT8 checksum[4];

	tag = get_apple35_tag(floppy);

	if (track >= ARRAY_LENGTH(apple35_tracklen_800kb))
		return FLOPPY_ERROR_SEEKERROR;
	if (offset != 0)
		return FLOPPY_ERROR_UNSUPPORTED;

	memset(buffer, 0xFF, buflen);
	sector_count = apple35_sectors_per_track(floppy, track);
	side = calculate_side(head, track);

	for (sector = 0; sector < sector_count; sector++)
	{
		/* read the sector */
		err = apple35_read_sector_td(floppy, head, track, sector, sector_data, ARRAY_LENGTH(sector_data));
		if (err)
		{
			return err;
		}

		sony_nibblize35(sector_data, nibble_data, checksum);

		for (i = 0; i < ARRAY_LENGTH(blk1); i++)
			sony_filltrack((UINT8*)buffer, buflen, &pos, blk1[i]);

		sum = (track ^ sector ^ side ^ tag->format_byte) & 0x3F;

		sony_filltrack((UINT8*)buffer, buflen, &pos, diskbytes[track & 0x3f]);
		sony_filltrack((UINT8*)buffer, buflen, &pos, diskbytes[sector]);
		sony_filltrack((UINT8*)buffer, buflen, &pos, diskbytes[side]);
		sony_filltrack((UINT8*)buffer, buflen, &pos, diskbytes[tag->format_byte]);
		sony_filltrack((UINT8*)buffer, buflen, &pos, diskbytes[sum]);

		for (i = 0; i < ARRAY_LENGTH(blk2); i++)
			sony_filltrack((UINT8*)buffer, buflen, &pos, blk2[i]);

		sony_filltrack((UINT8*)buffer, buflen, &pos, diskbytes[sector]);

		for (i = 0; i < ARRAY_LENGTH(nibble_data); i++)
			sony_filltrack((UINT8*)buffer, buflen, &pos, diskbytes[nibble_data[i]]);

		for (i = 3; i >= 0; i--)
			sony_filltrack((UINT8*)buffer, buflen, &pos, diskbytes[checksum[i]]);

		for (i = 0; i < ARRAY_LENGTH(blk3); i++)
			sony_filltrack((UINT8*)buffer, buflen, &pos, blk3[i]);
	}

	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple35_write_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, const void *buffer, size_t buflen)
{
	floperr_t err;
	size_t pos = 0;
	int sector_count, sector, i, j;
	struct apple35_tag *tag;
	UINT8 sum, format_byte, val, side;
	UINT32 found_sectors = 0;
	UINT8 sector_data[524];
	UINT8 nibble_data[699];
	UINT8 checksum[4];

	tag = get_apple35_tag(floppy);

	if (track >= ARRAY_LENGTH(apple35_tracklen_800kb))
		return FLOPPY_ERROR_SEEKERROR;
	if (offset != 0)
		return FLOPPY_ERROR_UNSUPPORTED;

	sector_count = apple35_sectors_per_track(floppy, track);
	side = calculate_side(head, track);

	/* do 2 rotations, in case the bit slip stuff prevent us to read the first sector */
	for (j = 0; j < (buflen * 2); j++)
	{
		if (sony_fetchtrack((UINT8*)buffer, buflen, &pos) != 0xD5)
			continue;
		j++;

		if (sony_fetchtrack((UINT8*)buffer, buflen, &pos) != 0xAA)
			continue;
		j++;

		if (sony_fetchtrack((UINT8*)buffer, buflen, &pos) != 0x96)
			continue;
		j++;

		if (rev_diskbytes[sony_fetchtrack((UINT8*)buffer, buflen, &pos)] != (track & 0x3F))
			continue;
		j++;

		sector = rev_diskbytes[sony_fetchtrack((UINT8*)buffer, buflen, &pos)];
		if ((sector < 0) || (sector >= sector_count))
			continue;
		j++;

		if (rev_diskbytes[sony_fetchtrack((UINT8*)buffer, buflen, &pos)] != side)
			continue;
		j++;

		format_byte = rev_diskbytes[sony_fetchtrack((UINT8*)buffer, buflen, &pos)];
		if (format_byte != tag->format_byte)
		{
			/* this is an error, but not THAT critical, I guess */
		}
		j++;

		sum = track ^ sector ^ side ^ format_byte;
		if (rev_diskbytes[sony_fetchtrack((UINT8*)buffer, buflen, &pos)] != sum)
			continue;
		j++;

		if (sony_fetchtrack((UINT8*)buffer, buflen, &pos) != 0xDE)
			continue;
		j++;

		if (sony_fetchtrack((UINT8*)buffer, buflen, &pos) != 0xAA)
			continue;
		j++;

		while((val = sony_fetchtrack((UINT8*)buffer, buflen, &pos)) == 0xFF)
			j++;
		if (val != 0xD5)
			continue;   /* lost bit slip mark! */
		j++;

		if (sony_fetchtrack((UINT8*)buffer, buflen, &pos) != 0xAA)
			continue;
		j++;

		if (sony_fetchtrack((UINT8*)buffer, buflen, &pos) != 0xAD)
			continue;
		j++;

		/* should this be regarded as a critical error ??? */
		if (rev_diskbytes[sony_fetchtrack((UINT8*)buffer, buflen, &pos)] != sector)
			continue;
		j++;

		for (i = 0; i < ARRAY_LENGTH(nibble_data); i++)
		{
			nibble_data[i] = rev_diskbytes[sony_fetchtrack((UINT8*)buffer, buflen, &pos)];
			j++;
		}

		for (i = 3; i >= 0; i--)
		{
			/* should be checking checksum */
			sony_fetchtrack((UINT8*)buffer, buflen, &pos);
		}

		sony_fetchtrack((UINT8*)buffer, buflen, &pos);  /* should get 0xDE */
		sony_fetchtrack((UINT8*)buffer, buflen, &pos);  /* should get 0xAA */
		sony_fetchtrack((UINT8*)buffer, buflen, &pos);  /* should get 0xFF */

		/* did we already write this sector? */
		if ((found_sectors & (1 << sector)) == 0)
		{
			sony_denibblize35(sector_data, nibble_data, checksum);

			/* write the sector */
			err = apple35_write_sector_td(floppy, head, track, sector, sector_data, ARRAY_LENGTH(sector_data), 0);
			if (err)
				return err;

			found_sectors |= 1 << sector;
		}
	}
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple35_construct(floppy_image_legacy *floppy, UINT32 data_offset, UINT32 data_size,
	UINT32 tag_offset, UINT32 tag_size, INT16 format_byte, UINT8 sides, int is_1440k)
{
	struct apple35_tag *tag;
	struct FloppyCallbacks *format;

	/* figure out format byte if not specified */
	if (format_byte < 0)
	{
		switch(sides)
		{
			case 1:
				format_byte = 0x02;
				break;
			case 2:
				format_byte = 0x22;
				break;
			default:
				return FLOPPY_ERROR_INVALIDIMAGE;
		}
	}

	/* create tag */
	tag = (struct apple35_tag *) floppy_create_tag(floppy, sizeof(struct apple35_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;
	tag->data_offset = data_offset;
	tag->data_size = data_size;
	tag->tag_offset = tag_offset;
	tag->tag_size = tag_size;
	tag->format_byte = (UINT8) format_byte;
	tag->sides = sides;
	tag->is_1440k = is_1440k ? 1 : 0;

	/* set up format callbacks */
	format = floppy_callbacks(floppy);
	format->read_sector = apple35_read_sector;
	format->write_sector = apple35_write_sector;
	format->read_track = apple35_read_track;
	format->write_track = apple35_write_track;
	format->get_sector_length = apple35_get_sector_length;
	format->get_heads_per_disk = apple35_get_heads_per_disk;
	format->get_tracks_per_disk = apple35_get_tracks_per_disk;
	format->get_track_size = apple35_get_track_size;
	return FLOPPY_ERROR_SUCCESS;
}



/* ----------------------------------------------------------------------- */

static FLOPPY_IDENTIFY(apple35_raw_identify)
{
	UINT64 size;
	size = floppy_image_size(floppy);
	*vote = ((size == 80*1*10*512) || (size == 80*2*10*512) || (size == (80*2*18*512)+84)
		|| (size == 80*2*18*512)) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}




static FLOPPY_CONSTRUCT(apple35_raw_construct)
{
	UINT64 size;
	UINT8 sides;
	int is_1440k;

	if (params)
	{
		/* create */
		sides = option_resolution_lookup_int(params, PARAM_HEADS);
		size = 80*sides*10*512;
		is_1440k = FALSE;
	}
	else
	{
		/* load */
		size = floppy_image_size(floppy);
		if (size == 80*1*10*512)
		{
			sides = 1;
			is_1440k = FALSE;
		}
		else if ((size == 80*2*10*512) || (size == 80*2*18*512) || (size == (80*2*18*512)+84))
		{
			sides = 2;
			is_1440k = (size == 80*2*18*512) || (size == (80*2*18*512)+84);
		}
		else
			return FLOPPY_ERROR_INVALIDIMAGE;
	}

	return apple35_construct(floppy, 0, (UINT32) size, 0, 0, -1, sides, is_1440k);
}



/* ----------------------------------------------------------------------- */

struct header_diskcopy
{
	UINT8 disk_name[64];    /* name of the disk */
	UINT32 data_size;       /* total size of data for all sectors (512*number_of_sectors) */
	UINT32 tag_size;        /* total size of tag data for all sectors (12*number_of_sectors for GCR 3.5" floppies, 20*number_of_sectors for HD20, 0 otherwise) */
	UINT32 data_checksum;   /* CRC32 checksum of all sector data */
	UINT32 tag_checksum;    /* CRC32 checksum of all tag data */
	UINT8 disk_format;      /* 0 = 400K, 1 = 800K, 2 = 720K, 3 = 1440K  (other values reserved) */
	UINT8 format_byte;      /* should be $00 Apple II, $01 Lisa, $02 Mac MFS ??? */
							/* $12 = 400K, $22 = >400K Macintosh (DiskCopy uses this value for
							   all Apple II disks not 800K in size, and even for some of those),
							   $24 = 800K Apple II disk */
	UINT16 magic;           /* always $0100 (otherwise, the file may be in a different format. */
};



static floperr_t apple35_diskcopy_headerdecode(floppy_image_legacy *floppy, UINT32 *data_offset,
	UINT32 *data_size, UINT32 *tag_offset, UINT32 *tag_size, UINT8 *format_byte, UINT8 *sides)
{
	UINT64 size;
	struct header_diskcopy header;

	if (data_offset)
		*data_offset = 0;
	if (data_size)
		*data_size = 0;
	if (tag_offset)
		*tag_offset = 0;
	if (tag_size)
		*tag_size = 0;
	if (format_byte)
		*format_byte = 0;
	if (sides)
		*sides = 0;

	size = floppy_image_size(floppy);
	if (size < sizeof(struct header_diskcopy))
		return FLOPPY_ERROR_INVALIDIMAGE;

	floppy_image_read(floppy, &header, 0, sizeof(header));

	header.data_size = BIG_ENDIANIZE_INT32(header.data_size);
	header.tag_size = BIG_ENDIANIZE_INT32(header.tag_size);
	header.data_checksum = BIG_ENDIANIZE_INT32(header.data_checksum);
	header.tag_checksum = BIG_ENDIANIZE_INT32(header.tag_checksum);
	header.magic = BIG_ENDIANIZE_INT16(header.magic);

	if (header.disk_name[0] >= sizeof(header.disk_name))
		return FLOPPY_ERROR_INVALIDIMAGE;
	if (header.magic != 0x0100)
		return FLOPPY_ERROR_INVALIDIMAGE;
	if (size != (header.data_size + header.tag_size + sizeof(header)))
		return FLOPPY_ERROR_INVALIDIMAGE;

	if (header.data_size == 80*1*10*512)
	{
		if (sides)
			*sides = 1;
	}
	else if (header.data_size == 80*2*10*512)
	{
		if (sides)
			*sides = 2;
	}
	else
		return FLOPPY_ERROR_INVALIDIMAGE;

	if (data_offset)
		*data_offset = sizeof(header);
	if (data_size)
		*data_size = header.data_size;
	if (tag_offset)
		*tag_offset = sizeof(header) + header.data_size;
	if (tag_size)
		*tag_size = header.tag_size;
	if (format_byte)
		*format_byte = header.format_byte;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_IDENTIFY(apple35_diskcopy_identify)
{
	*vote = apple35_diskcopy_headerdecode(floppy, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) ? 0 : 100;
	return FLOPPY_ERROR_SUCCESS;
}




static FLOPPY_CONSTRUCT(apple35_diskcopy_construct)
{
	floperr_t err;
	UINT8 format_byte, sides;
	UINT32 data_offset, data_size;
	UINT32 tag_offset, tag_size;
	INT16 format_byte_param = -1;
	struct header_diskcopy header;

	if (params)
	{
		/* create */
		sides = option_resolution_lookup_int(params, PARAM_HEADS);

		data_size = 80*sides*10*512;
		tag_size = 80*sides*10*12;

		memset(&header, 0, sizeof(header));
		header.data_size = BIG_ENDIANIZE_INT32(data_size);
		header.tag_size = BIG_ENDIANIZE_INT32(tag_size);
		header.disk_format = (sides > 1) ? 1 : 0;
		header.magic = BIG_ENDIANIZE_INT16(0x100);

		floppy_image_write(floppy, &header, 0, sizeof(header));
		floppy_image_write_filler(floppy, 0, sizeof(header), data_size + tag_size);
	}

	/* load */
	err = apple35_diskcopy_headerdecode(floppy, &data_offset, &data_size,
		&tag_offset, &tag_size, &format_byte, &sides);
	if (err)
		return err;

	format_byte_param = format_byte;

	return apple35_construct(floppy, data_offset, data_size,
		tag_offset, tag_size, format_byte_param, sides, FALSE);
}



/* ----------------------------------------------------------------------- */

struct header_2img
{
	char magic[4];          /* '2IMG' */
	char creator[4];        /* signature; 'MESS' for MESS */
	UINT16 header_length;
	UINT16 version;
	UINT32 image_format;
	UINT32 flags;
	UINT32 block_count;
	UINT32 data_offset;
	UINT32 data_length;
	UINT32 comment_offset;
	UINT32 comment_length;
	UINT32 creator_offset;
	UINT32 creator_length;
	UINT32 padding[4];
};

#define IMAGE_FORMAT_DO     0
#define IMAGE_FORMAT_PO     1
#define IMAGE_FORMAT_NIB    2
#define IMAGE_FLAGS_LOCKED  0x80000000



static floperr_t apple35_2img_decode(floppy_image_legacy *floppy, UINT32 *image_format,
	UINT32 *data_offset, UINT32 *data_length)
{
	struct header_2img header;
	UINT64 size;

	if (image_format)
		*image_format = 0;
	if (data_offset)
		*data_offset = 0;
	if (data_length)
		*data_length = 0;

	size = floppy_image_size(floppy);
	if (size < sizeof(header))
	{
		return FLOPPY_ERROR_INVALIDIMAGE;
	}

	floppy_image_read(floppy, &header, 0, sizeof(header));

	if (memcmp(header.magic, "2IMG", 4))
	{
		return FLOPPY_ERROR_INVALIDIMAGE;
	}

	header.header_length    = LITTLE_ENDIANIZE_INT16(header.header_length);
	header.version          = LITTLE_ENDIANIZE_INT16(header.version);
	header.image_format     = LITTLE_ENDIANIZE_INT32(header.image_format);
	header.flags            = LITTLE_ENDIANIZE_INT32(header.flags);
	header.block_count      = LITTLE_ENDIANIZE_INT32(header.block_count);
	header.data_offset      = LITTLE_ENDIANIZE_INT32(header.data_offset);
	header.data_length      = LITTLE_ENDIANIZE_INT32(header.data_length);
	header.comment_offset   = LITTLE_ENDIANIZE_INT32(header.comment_offset);
	header.comment_length   = LITTLE_ENDIANIZE_INT32(header.comment_length);
	header.creator_offset   = LITTLE_ENDIANIZE_INT32(header.creator_offset);
	header.creator_length   = LITTLE_ENDIANIZE_INT32(header.creator_length);

	// at least some images "in the wild" (e.g. TOSEC Minor Set 1) have big-endian data sizes
	// even though that's against the .2mg spec
	if (header.data_length == 0x800c00)
	{
		LOG_FORMATS("ap_dsk35: corrected bad-endian data length\n");
		header.data_length = 0x0c8000;
	}

	if ((((UINT64) header.data_offset) + header.data_length) > size)
		return FLOPPY_ERROR_INVALIDIMAGE;
	if ((((UINT64) header.comment_offset) + header.comment_length) > size)
		return FLOPPY_ERROR_INVALIDIMAGE;
	if ((((UINT64) header.creator_offset) + header.creator_length) > size)
		return FLOPPY_ERROR_INVALIDIMAGE;
	if ((header.image_format != IMAGE_FORMAT_DO) &&
			(header.image_format != IMAGE_FORMAT_PO) &&
			(header.image_format != IMAGE_FORMAT_NIB))
		return FLOPPY_ERROR_INVALIDIMAGE;

	if (image_format)
		*image_format = header.image_format;
	if (data_offset)
		*data_offset = header.data_offset;
	if (data_length)
		*data_length = header.data_length;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_IDENTIFY(apple35_2img_identify)
{
	*vote = apple35_2img_decode(floppy, nullptr, nullptr, nullptr) ? 0 : 100;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_CONSTRUCT(apple35_2img_construct)
{
	floperr_t err;
	UINT32 image_format;
	UINT32 data_offset;
	UINT32 data_size;
	UINT8 sides = 2;
	struct header_2img header;

	if (params)
	{
		/* create */
		sides = option_resolution_lookup_int(params, PARAM_HEADS);

		data_offset = sizeof(header);
		data_size = 80*sides*10*512;

		memset(&header, 0, sizeof(header));
		header.header_length    = LITTLE_ENDIANIZE_INT16(sizeof(header));
		header.block_count      = LITTLE_ENDIANIZE_INT32(80*sides*10);
		header.data_offset      = LITTLE_ENDIANIZE_INT32(data_offset);
		header.data_length      = LITTLE_ENDIANIZE_INT32(data_size);

		floppy_image_write(floppy, &header, 0, sizeof(header));
		floppy_image_write_filler(floppy, 0, sizeof(header), data_size);
	}
	else
	{
		/* load */
		err = apple35_2img_decode(floppy, &image_format, &data_offset, &data_size);
		if (err)
			return err;

		if (data_size == 80*1*10*512)
			sides = 1;  /* single sided */
		else if (data_size == 80*2*10*512)
			sides = 2;  /* double sided */
		else
			sides = 2;  /* unknown... what to do... */
	}

	return apple35_construct(floppy, data_offset, data_size,
		0, 0, -1, sides, FALSE);
}



LEGACY_FLOPPY_OPTIONS_START( apple35_mac )
	LEGACY_FLOPPY_OPTION( apple35_raw, "dsk,img,image", "Apple raw 3.5\" disk image",   apple35_raw_identify,       apple35_raw_construct, nullptr,
		HEADS([1]-2)
		TRACKS([80])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION( apple35_dc, "dc,dc42,dsk,img,image",  "Apple DiskCopy disk image",    apple35_diskcopy_identify,  apple35_diskcopy_construct, nullptr,
		HEADS([1]-2)
		TRACKS([80])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END

LEGACY_FLOPPY_OPTIONS_START( apple35_iigs )
	LEGACY_FLOPPY_OPTION( apple35_raw, "dsk,img,image,po", "Apple raw 3.5\" disk image",   apple35_raw_identify,       apple35_raw_construct, nullptr,
		HEADS([1]-2)
		TRACKS([80])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION( apple35_dc, "dc,dsk,img,image",   "Apple DiskCopy disk image",    apple35_diskcopy_identify,  apple35_diskcopy_construct, nullptr,
		HEADS([1]-2)
		TRACKS([80])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION( apple35_2img, "2img,2mg",     "Apple ][gs 2IMG disk image",   apple35_2img_identify,      apple35_2img_construct, nullptr,
		HEADS([1]-2)
		TRACKS([80])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END


// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
dc42_format::dc42_format() : floppy_image_format_t()
{
}

const char *dc42_format::name() const
{
	return "dc42";
}

const char *dc42_format::description() const
{
	return "DiskCopy 4.2 image";
}

const char *dc42_format::extensions() const
{
	return "dc42";
}

bool dc42_format::supports_save() const
{
	return true;
}

int dc42_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 h[0x54];
	UINT64 size = io_generic_size(io);
	if(size < 0x54)
		return 0;

	io_generic_read(io, h, 0, 0x54);
	UINT32 dsize = (h[0x40] << 24) | (h[0x41] << 16) | (h[0x42] << 8) | h[0x43];
	UINT32 tsize = (h[0x44] << 24) | (h[0x45] << 16) | (h[0x46] << 8) | h[0x47];

	return size == 0x54+tsize+dsize && h[0] < 64 && h[0x52] == 1 && h[0x53] == 0 ? 100 : 0;
}

const floppy_image_format_t::desc_e dc42_format::mac_gcr[] = {
	{ SECTOR_LOOP_START, 0, -1 },
	{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
	{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
	{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
	{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
	{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
	{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
	{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
	{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
	{   RAWBITS, 0xd5aa96, 24 },
	{   CRC_MACHEAD_START, 0 },
	{     TRACK_ID_GCR6 },
	{     SECTOR_ID_GCR6 },
	{     TRACK_HEAD_ID_GCR6 },
	{     SECTOR_INFO_GCR6 },
	{   CRC_END, 0 },
	{   CRC, 0 },
	{   RAWBITS, 0xdeaaff, 24 },
	{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
	{   RAWBITS, 0xd5aaad, 24 },
	{   SECTOR_ID_GCR6 },
	{   SECTOR_DATA_MAC, -1 },
	{   RAWBITS, 0xdeaaff, 24 },
	{   RAWBITS, 0xff, 8 },
	{ SECTOR_LOOP_END },
	{ END },
};


bool dc42_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT8 h[0x54];
	io_generic_read(io, h, 0, 0x54);
	int dsize = (h[0x40] << 24) | (h[0x41] << 16) | (h[0x42] << 8) | h[0x43];
	int tsize = (h[0x44] << 24) | (h[0x45] << 16) | (h[0x46] << 8) | h[0x47];

	UINT8 encoding = h[0x50];
	UINT8 format = h[0x51];

	if((encoding != 0x00 || format != 0x02) && (encoding != 0x01 || format != 0x22)) {
		osd_printf_error("dc42: Unsupported encoding/format combination %02x/%02x\n", encoding, format);
		return false;
	}

	UINT8 sector_data[(512+12)*12];
	memset(sector_data, 0, sizeof(sector_data));

	desc_s sectors[12];

	int pos_data = 0x54;
	int pos_tag = 0x54+dsize;

	int head_count = encoding == 1 ? 2 : 1;

	for(int track=0; track < 80; track++) {
		for(int head=0; head < head_count; head++) {
			int ns = 12 - (track/16);
			int si = 0;
			for(int i=0; i<ns; i++) {
				UINT8 *data = sector_data + (512+12)*i;
				sectors[si].data = data;
				sectors[si].size = 512+12;
				sectors[si].sector_id = i;
				sectors[si].sector_info = format;
				if(tsize) {
					io_generic_read(io, data, pos_tag, 12);
					pos_tag += 12;
				}
				io_generic_read(io, data+12, pos_data, 512);
				pos_data += 512;
				si = (si + 2) % ns;
				if(si == 0)
					si++;
			}
			generate_track(mac_gcr, track, head, sectors, ns, 6208*ns, image);
		}
	}
	return true;
}

UINT8 dc42_format::gb(const UINT8 *buf, int ts, int &pos, int &wrap)
{
	UINT8 v = 0;
	int w1 = wrap;
	while(wrap != w1+2 && !(v & 0x80)) {
		v = v << 1 | ((buf[pos >> 3] >> (7-(pos & 7))) & 1);
		pos++;
		if(pos == ts) {
			pos = 0;
			wrap++;
		}
	}
	return v;
}

void dc42_format::update_chk(const UINT8 *data, int size, UINT32 &chk)
{
	for(int i=0; i<size; i+=2) {
		chk += (data[i] << 8) | data[i+1];
		chk = (chk >> 1) | (chk << 31);
	}
}

bool dc42_format::save(io_generic *io, floppy_image *image)
{
	int g_tracks, g_heads;
	image->get_actual_geometry(g_tracks, g_heads);

	if(g_heads == 0)
		g_heads = 1;

	UINT8 h[0x54];
	memset(h, 0, 0x54);
	strcpy((char *)h+1, "Unnamed");
	h[0] = 7;
	int nsect = 16*(12+11+10+9+8)*g_heads;
	UINT32 dsize = nsect*512;
	UINT32 tsize = nsect*12;
	h[0x40] = dsize >> 24;
	h[0x41] = dsize >> 16;
	h[0x42] = dsize >> 8;
	h[0x43] = dsize;
	h[0x44] = tsize >> 24;
	h[0x45] = tsize >> 16;
	h[0x46] = tsize >> 8;
	h[0x47] = tsize;
	h[0x50] = g_heads == 2 ? 0x01 : 0x00;
	h[0x51] = g_heads == 2 ? 0x22 : 0x02;
	h[0x52] = 0x01;
	h[0x53] = 0x00;

	UINT32 dchk = 0;
	UINT32 tchk = 0;

	int pos_data = 0x54;
	int pos_tag = 0x54+dsize;

	for(int track=0; track < 80; track++) {
		for(int head=0; head < g_heads; head++) {
			UINT8 sectdata[(512+12)*12];
			memset(sectdata, 0, sizeof(sectdata));
			int nsect = 12-(track/16);
			UINT8 buf[13000];
			int ts;
			generate_bitstream_from_track(track, head, 200000000/(6208*nsect), buf, ts, image);
			int pos = 0;
			int wrap = 0;
			int hb = 0;
			for(;;) {
				UINT8 v = gb(buf, ts, pos, wrap);
				if(v == 0xff)
					hb = 1;
				else if(hb == 1 && v == 0xd5)
					hb = 2;
				else if(hb == 2 && v == 0xaa)
					hb = 3;
				else if(hb == 3 && v == 0x96)
					hb = 4;
				else
					hb = 0;

				if(hb == 4) {
					UINT8 h[7];
					for(auto & elem : h)
						elem = gb(buf, ts, pos, wrap);
					UINT8 v2 = gcr6bw_tb[h[2]];
					UINT8 v3 = gcr6bw_tb[h[3]];
					UINT8 tr = gcr6bw_tb[h[0]] | (v2 & 1 ? 0x40 : 0x00);
					UINT8 se = gcr6bw_tb[h[1]];
					UINT8 si = v2 & 0x20 ? 1 : 0;
					//                  UINT8 ds = v3 & 0x20 ? 1 : 0;
					//                  UINT8 fmt = v3 & 0x1f;
					UINT8 c1 = (tr^se^v2^v3) & 0x3f;
					UINT8 chk = gcr6bw_tb[h[4]];
					if(chk == c1 && tr == track && si == head && se < nsect) {
						int opos = pos;
						int owrap = wrap;
						hb = 0;
						for(int i=0; i<20 && hb != 4; i++) {
							v = gb(buf, ts, pos, wrap);
							if(v == 0xff)
								hb = 1;
							else if(hb == 1 && v == 0xd5)
								hb = 2;
							else if(hb == 2 && v == 0xaa)
								hb = 3;
							else if(hb == 3 && v == 0xad)
								hb = 4;
							else
								hb = 0;
						}
						if(hb == 4) {
							UINT8 *dest = sectdata+(512+12)*se;
							gb(buf, ts, pos, wrap); // Ignore the sector byte
							UINT8 ca = 0, cb = 0, cc = 0;
							for(int i=0; i<522/3+1; i++) {
								UINT8 e0 = gb(buf, ts, pos, wrap);
								UINT8 e1 = gb(buf, ts, pos, wrap);
								UINT8 e2 = gb(buf, ts, pos, wrap);
								UINT8 e3 = i == 522/3 ? 0x96 : gb(buf, ts, pos, wrap);
								UINT8 va, vb, vc;
								gcr6_decode(e0, e1, e2, e3, va, vb, vc);
								cc = (cc << 1) | (cc >> 7);
								va = va ^ cc;
								int suma = ca + va + (cc & 1);
								ca = suma;
								vb = vb ^ ca;
								int sumb = cb + vb + (suma >> 8);
								cb = sumb;
								vc = vc ^ cb;
								cc = cc + vc + (sumb >> 8);
								*dest++ = va;
								*dest++ = vb;
								if(i != 522/3)
									*dest++ = vc;
							}
						} else {
							pos = opos;
							wrap = owrap;
						}
					}
					hb = 0;
				}
				if(wrap)
					break;
			}
			for(int i=0; i<nsect; i++) {
				UINT8 *data = sectdata + (512+12)*i;
				io_generic_write(io, data, pos_tag, 12);
				io_generic_write(io, data+12, pos_data, 512);
				pos_tag += 12;
				pos_data += 512;
				if(track || head || i)
					update_chk(data, 12, tchk);
				update_chk(data+12, 512, dchk);
			}
		}
	}

	h[0x48] = dchk >> 24;
	h[0x49] = dchk >> 16;
	h[0x4a] = dchk >> 8;
	h[0x4b] = dchk;
	h[0x4c] = tchk >> 24;
	h[0x4d] = tchk >> 16;
	h[0x4e] = tchk >> 8;
	h[0x4f] = tchk;

	io_generic_write(io, h, 0, 0x54);
	return true;
}

const floppy_format_type FLOPPY_DC42_FORMAT = &floppy_image_format_creator<dc42_format>;
