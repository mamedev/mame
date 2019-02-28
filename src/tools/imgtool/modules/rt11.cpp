// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/****************************************************************************

    rt11.cpp

    DEC RT-11 disk images

    References:

    VaFFM -- bitsavers://pdf/dec/pdp11/rt11/v5.6_Aug91/AA-PD6PA-TC_RT-11_Volume_and_File_Formats_Manual_Aug91.pdf
    DHM -- bitsavers://pdf/dec/pdp11/rt11/v5.6_Aug91/AA-PE7VA-TC_RT-11_Device_Handlers_Manual_Aug91.pdf
    SSM -- bitsavers://pdf/dec/pdp11/rt11/v5.0_Mar83/AA-H379B-TC_5.0_SWsuppMar83.pdf
    TSX+ -- bitsavers://pdf/dec/pdp11/tsxPlus/manuals_6.31/TSX-Plus_UsersRef_Jan88.pdf
    PUTR -- http://www.dbit.com/pub/putr/putr.asm

    To do:
    - filter for text files
    - read-write support
    - report empty 'last modified' time if date field is all zeros
    - report free space
    - arbitrary sized images
    - don't crash when strings in home block have non-ascii chars (charconverter does not apply)
    - do something about bootblock bug in imgtool (commit aca90520)

    LBN Contents
    --- --------
    0   Reserved (primary bootstrap)
    1   Reserved (home block)
    2-5 Reserved (secondary bootstrap)
    6-7 Directory segment 1
    ... Directory segment 2-n
    ... Data

    Home block
    ----------
    000-201 Bad block replacement table
    202-203 ?
    204-251 INITIALIZE/RESTORE data area
    252-273 BUP information area
    274-677 ?
    700-701 (Reserved for Digital, must be zero)
    702-703 (Reserved for Digital, must be zero)
    704-721 ?
    722-723 Pack cluster size (= 1)
    724-725 Block number of first directory segment
    726-727 System version (RAD50)
    730-742 Volume Identification
    744-757 Owner name
    760-773 System Identification
    776-777 Checksum

    Directory segment header
    ------------------------
    0   The total number of segments in this directory.
    1   The segment number of the next logical directory segment. If this word is 0, there are no more segments in the list.
    2   The number of the highest segment currently in use.  Valid only in the first directory segment.
    3   The number of extra bytes per directory entry, always an unsigned, even octal number.
    4   The block number on the volume where the actual stored data identified by this segment begins.

    Directory entry
    ---------------
    0   Status word
    1   File name 1-3 (RAD50)
    2   File name 4-6 (RAD50)
    3   File type 1-3 (RAD50)
    4   Total file length (blocks)
    5   Job#, Channel# (RT-11 uses this information only for tentative files)
    6   Creation date
    7-  Optional extra words

****************************************************************************/

#include "imgtool.h"
#include "formats/imageutl.h"
#include "iflopimg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace
{
	struct rt11_diskinfo
	{
		uint16_t directory_start;
		uint16_t total_segments;
		uint16_t last_segment;
		uint16_t dirent_size;
		int dirents_per_block;
		// autodetected
		int tracks;
		int heads;
		int sectors;
		uint32_t sector_size;
		// cache
		int map[16];
		int cached_track;
		int cached_head;
	};

	enum misc_t
	{
		HOME_BLOCK = 1,
		BLOCK_SIZE = 512
	};

	struct rt11_direnum
	{
		uint8_t segment_data[2 * BLOCK_SIZE];
		uint16_t segment;
		uint16_t index;
		uint16_t data;
	};

	struct rt11_dirent
	{
		uint16_t status;
		uint16_t filename[3];
		uint16_t time;
		// synthetic
		uint64_t filesize;
		uint16_t data;
	};

	enum rt11_status
	{
		E_PRE  = 0000020,
		E_TENT = 0000400,
		E_MPTY = 0001000,
		E_PERM = 0002000,
		E_EOS  = 0004000,
		E_READ = 0040000,
		E_PROT = 0100000
	};

	enum creation_policy_t
	{
		CREATE_NONE,
		CREATE_FILE,
	};


	util::arbitrary_datetime _rt11_crack_time(uint16_t rt11_time)
	{
		util::arbitrary_datetime dt;

		dt.second       = 0;
		dt.minute       = 0;
		dt.hour         = 0;
		dt.day_of_month = (rt11_time >> 5) & 31;
		dt.month        = (rt11_time >> 10) & 15;
		dt.year         = 1972 + (rt11_time & 31) + 32 * ((rt11_time >> 14) & 3);

		return dt;
	}

	imgtool::datetime rt11_crack_time(uint16_t rt11_time)
	{
		util::arbitrary_datetime dt;
		imgtool::datetime it;

		if (rt11_time == 0)
		{
			return imgtool::datetime(imgtool::datetime::datetime_type::NONE, dt);
		}

		dt = _rt11_crack_time(rt11_time);

		it = imgtool::datetime(imgtool::datetime::datetime_type::LOCAL, dt);

		return it;
	}

	void rt11_from_rad50(char *ascii, uint16_t *rad50, int num)
	{
		const char rad[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ$.%0123456789:";

		for (int i = 0, j = 0; i < num; i++)
		{
			ascii[j++] = rad[ rad50[i] / (050 * 050)];
			ascii[j++] = rad[(rad50[i] / 050) % 050];
			ascii[j++] = rad[ rad50[i] % 050];
		}
	}

	void rt11_filename_from_rad50(char *ascii, uint16_t *rad50)
	{
		int i, j;

		rt11_from_rad50(&ascii[0], &rad50[0], 2);
		for (i = 0; i < 6; i++)
		{
			if (ascii[i] == ' ') break;
		}
		ascii[i++] = '.';
		rt11_from_rad50(&ascii[i], &rad50[2], 1);
		for (j = i; j < i + 3; j++)
		{
			if (ascii[j] == ' ') break;
		}
		ascii[j] = '\0';
	}

	int is_file_storagetype(uint16_t status)
	{
		return !(status & (E_MPTY | E_EOS | E_TENT));
	}

	rt11_diskinfo *get_rt11_info(imgtool::image &image)
	{
		return (rt11_diskinfo *)imgtool_floppy_extrabytes(image);
	}
}


static imgtoolerr_t rt11_image_get_geometry(imgtool::image &image, uint32_t *tracks, uint32_t *heads, uint32_t *sectors)
{
	const rt11_diskinfo *di = get_rt11_info(image);

	*sectors = di->sectors;
	*heads = di->heads;
	*tracks = di->tracks;
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t rt11_get_sector_position(imgtool::image &image, uint32_t sector_index,
	uint32_t &head, uint32_t &track, uint32_t &sector)
{
	imgtoolerr_t err;
	rt11_diskinfo *di = get_rt11_info(image);
	uint32_t tracks, heads, sectors;

	err = image.get_geometry(&tracks, &heads, &sectors);
	if (err)
		return err;

	track = sector_index / sectors / heads;
	head = (sector_index / sectors) % heads;

	// map 1-based sector numbers, possibly interleaved, to sector indexes
	if (track != di->cached_track || head != di->cached_head)
	{
		memset(di->map, -1, sizeof(di->map));
		for (int i = 0; i < 256; i++)
		{
			int sector_id;

			if (floppy_get_indexed_sector_info(imgtool_floppy(image), head, track, i, NULL, NULL, &sector_id, NULL, NULL))
				continue;

			if (sector_id > 0 && sector_id <= sectors)
				di->map[sector_id - 1] = i;
		}
		di->cached_track = track;
		di->cached_head = head;
	}

	if (di->map[(sector_index % sectors)] < 0)
	{
		return IMGTOOLERR_SEEKERROR;
	}
	else
	{
		sector = di->map[(sector_index % sectors)];
		return IMGTOOLERR_SUCCESS;
	}
}


static imgtoolerr_t rt11_image_readblock(imgtool::image &image, void *buffer, uint64_t block)
{
	imgtoolerr_t err;
	floperr_t ferr;
	const rt11_diskinfo *di = get_rt11_info(image);
	uint32_t track, head, sector;
	unsigned long flags;

	if (di->sector_size == 256)
	{
		err = rt11_get_sector_position(image, block * 2, head, track, sector);
		if (err)
			return err;

		ferr = floppy_read_indexed_sector(imgtool_floppy(image), head, track, sector, 0, buffer, 256);
		if (ferr)
			return imgtool_floppy_error(ferr);

		ferr = floppy_get_indexed_sector_info(imgtool_floppy(image), head, track, sector, NULL, NULL, NULL, NULL, &flags);
		if (ferr)
			return imgtool_floppy_error(ferr);

		if (flags)
			return IMGTOOLERR_READERROR;

		err = rt11_get_sector_position(image, (block * 2) + 1, head, track, sector);
		if (err)
			return err;

		ferr = floppy_read_indexed_sector(imgtool_floppy(image), head, track, sector, 0, ((uint8_t *) buffer) + 256,  256);
		if (ferr)
			return imgtool_floppy_error(ferr);

		ferr = floppy_get_indexed_sector_info(imgtool_floppy(image), head, track, sector, NULL, NULL, NULL, NULL, &flags);
		if (ferr)
			return imgtool_floppy_error(ferr);

		if (flags)
			return IMGTOOLERR_READERROR;
	}
	else
	{
		err = rt11_get_sector_position(image, block, head, track, sector);
		if (err)
			return err;

		ferr = floppy_read_indexed_sector(imgtool_floppy(image), head, track, sector, 0, buffer, BLOCK_SIZE);
		if (ferr)
			return imgtool_floppy_error(ferr);

		ferr = floppy_get_indexed_sector_info(imgtool_floppy(image), head, track, sector, NULL, NULL, NULL, NULL, &flags);
		if (ferr)
			return imgtool_floppy_error(ferr);

		if (flags)
			return IMGTOOLERR_READERROR;
	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t rt11_probe_geometry(imgtool::image &image)
{
	floperr_t ferr;
	rt11_diskinfo *di = get_rt11_info(image);

	// MX (11x256 byte sectors) or MY (10x512 byte sectors)?
	ferr = floppy_get_indexed_sector_info(imgtool_floppy(image), 0, 0, 0, NULL, NULL, NULL, &di->sector_size, NULL);
	if (ferr)
		return imgtool_floppy_error(ferr);

	if (di->sector_size == 256)
		di->sectors = 11;
	else
		di->sectors = 10;

	// double- or single-sided?
	ferr = floppy_get_indexed_sector_info(imgtool_floppy(image), 1, 0, 0, NULL, NULL, NULL, NULL, NULL);
	if (ferr)
		di->heads = 1;
	else
		di->heads = 2;

	// 80 or 40 tracks?
	ferr = floppy_get_indexed_sector_info(imgtool_floppy(image), 0, 50, 0, NULL, NULL, NULL, NULL, NULL);
	if (ferr)
		di->tracks = 40;
	else
		di->tracks = 80;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t rt11_image_open(imgtool::image &image, imgtool::stream::ptr &&dummy)
{
	imgtoolerr_t err;
	uint8_t buffer[BLOCK_SIZE];
	rt11_diskinfo *di = get_rt11_info(image);

	di->cached_head = -1;
	di->cached_track = -1;
	memset(di->map, -1, sizeof(di->map));
	// dummy values for rt11_image_readblock
	di->tracks = 40;
	di->heads = 1;

	err = rt11_probe_geometry(image);
	if (err)
		return err;

	/* load home block */
	err = rt11_image_readblock(image, buffer, HOME_BLOCK);
	if (err)
		return err;

	di->directory_start = pick_integer_le(buffer, 0724, 2);

	// real-world images seem to never have a valid checksum, but directory_start is always 6
#if 0
	uint16_t tmp, cksum;

	tmp = 0;
	cksum = pick_integer_le(buffer, 0776, 2);
	for (int i = 0; i < 510; i+=2)
	{
		tmp += pick_integer_le(buffer, i, 2);
	}

	/* sanity check these values */
	if (cksum != tmp)
	{
		fprintf(stderr, "cksum stored:computed %04x:%04x\n", cksum, tmp);
		return IMGTOOLERR_CORRUPTIMAGE;
	}
#endif
	if (di->directory_start != 6)
	{
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	/* load first directory segment */
	err = rt11_image_readblock(image, buffer, di->directory_start);
	if (err)
		return err;

	di->total_segments = pick_integer_le(buffer, 0, 2);
	di->last_segment = pick_integer_le(buffer, 4, 2);
	di->dirent_size = (pick_integer_le(buffer, 6, 2) + 7) * 2;
	di->dirents_per_block = (2 * BLOCK_SIZE - 10) / di->dirent_size;

	return IMGTOOLERR_SUCCESS;
}

static void rt11_image_info(imgtool::image &image, std::ostream &stream)
{
	uint8_t buffer[BLOCK_SIZE];
	char system[4];
	char vid[13], oid[13], sid[13];

	rt11_image_readblock(image, buffer, HOME_BLOCK);
	rt11_from_rad50(system, (uint16_t *)&buffer[0726], 1);
	system[3] = '\0';

	memcpy(vid, &buffer[0730], 12);
	memcpy(oid, &buffer[0744], 12);
	memcpy(sid, &buffer[0760], 12);
	vid[12] = '\0';
	oid[12] = '\0';
	sid[12] = '\0';

	stream << "System version: '" << system << "', System ID: '" << sid << "', Volume ID: '" << vid << "', Owner: '" << oid << "'";
}

// directory operations

static imgtoolerr_t rt11_enum_seek(imgtool::image &image,
	rt11_direnum *rt11enum, uint16_t segment, uint16_t index)
{
	const rt11_diskinfo *di = get_rt11_info(image);
	imgtoolerr_t err;
	uint8_t buffer[BLOCK_SIZE];

	if (rt11enum->segment != segment)
	{
		if (segment != 0)
		{
			err = rt11_image_readblock(image, buffer, di->directory_start + (segment - 1) * 2);
			if (err)
				return err;
			memcpy(rt11enum->segment_data, buffer, sizeof(buffer));

			err = rt11_image_readblock(image, buffer, di->directory_start + (segment - 1) * 2 + 1);
			if (err)
				return err;
			memcpy(&rt11enum->segment_data[BLOCK_SIZE], buffer, sizeof(buffer));

			rt11enum->data = pick_integer_le(rt11enum->segment_data, 8, 2);
		}
		rt11enum->segment = segment;
	}

	rt11enum->index = index;
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t rt11_get_next_dirent(imgtool::image &image,
	rt11_direnum *rt11enum, rt11_dirent &rt_ent)
{
	imgtoolerr_t err;
	const rt11_diskinfo *di = get_rt11_info(image);
	uint32_t next_segment, next_index;
	uint32_t offset;

	memset(&rt_ent, 0, sizeof(rt_ent));

	/* have we hit the end of the file? */
	if (rt11enum->segment == 0)
		return IMGTOOLERR_SUCCESS;

	/* populate the resulting dirent */
	offset = (rt11enum->index * di->dirent_size) + 10;
	rt_ent.status = pick_integer_le(rt11enum->segment_data, offset, 2);
	memcpy(rt_ent.filename, &rt11enum->segment_data[offset + 2], 6);
	rt_ent.filesize = pick_integer_le(rt11enum->segment_data, offset + 8, 2) * BLOCK_SIZE;
	rt_ent.data = rt11enum->data;
	rt_ent.time = pick_integer_le(rt11enum->segment_data, offset + 12, 2);
	rt11enum->data += pick_integer_le(rt11enum->segment_data, offset + 8, 2);

	/* identify next entry */
	next_segment = rt11enum->segment;
	next_index = rt11enum->index + 1;
	if (next_index >= di->dirents_per_block || (rt_ent.status & E_EOS))
	{
		next_segment = pick_integer_le(rt11enum->segment_data, 2, 2);
		next_index = 0;
	}

	if (next_segment > di->total_segments || next_segment > di->last_segment)
		return IMGTOOLERR_CORRUPTIMAGE;

	/* seek next segment */
	err = rt11_enum_seek(image, rt11enum, next_segment, next_index);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t rt11_image_beginenum(imgtool::directory &enumeration, const char *path)
{
	imgtoolerr_t err;
	imgtool::image &image(enumeration.image());

	/* seek initial block */
	err = rt11_enum_seek(image, (rt11_direnum *) enumeration.extra_bytes(), 1, 0);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t rt11_image_nextenum(imgtool::directory &enumeration, imgtool_dirent &ent)
{
	imgtoolerr_t err;
	imgtool::image &image(enumeration.image());
	rt11_direnum *rt11enum = (rt11_direnum *)enumeration.extra_bytes();
	rt11_dirent rt_ent;

	do
	{
		err = rt11_get_next_dirent(image, rt11enum, rt_ent);
		if (err)
			return err;
	}
	while (rt11enum->segment && !is_file_storagetype(rt_ent.status));

	/* end of file? */
	if (rt11enum->segment == 0)
	{
		ent.eof = 1;
		return IMGTOOLERR_SUCCESS;
	}

	ent.directory          = 0;
	ent.lastmodified_time  = rt11_crack_time(rt_ent.time);
	ent.filesize           = rt_ent.filesize;

	rt11_filename_from_rad50(ent.filename, rt_ent.filename);

	snprintf(ent.attr, sizeof(ent.attr), "%c%c%c %4d %06o",
		rt_ent.status & E_PROT ? 'P' : '.',
		rt_ent.time == 0 ? 'B' : '.',
		rt_ent.status & E_TENT ? 'T' : '.',
		rt_ent.data, rt_ent.status);

	return IMGTOOLERR_SUCCESS;
}

// file operations

static imgtoolerr_t rt11_lookup_path(imgtool::image &image, const char *path,
	creation_policy_t create, rt11_direnum *direnum, rt11_dirent *rt_ent)
{
	imgtoolerr_t err;
	rt11_direnum my_direnum;
	uint16_t this_segment;
	uint32_t this_index;
	char filename[16];

	if (!direnum)
		direnum = &my_direnum;

	memset(direnum, 0, sizeof(*direnum));
	err = rt11_enum_seek(image, direnum, 1, 0);
	if (err)
		goto done;

	do
	{
		this_segment = direnum->segment;
		this_index = direnum->index;

		err = rt11_get_next_dirent(image, direnum, *rt_ent);
		if (err)
			goto done;
		rt11_filename_from_rad50(filename, rt_ent->filename);
	}
	while(direnum->segment && (strcmp(path, filename) ||
		!is_file_storagetype(rt_ent->status)));

	if (!direnum->segment)
	{
		/* did not find file; maybe we need to create it */
		if (create == CREATE_NONE)
		{
			err = IMGTOOLERR_FILENOTFOUND;
			goto done;
		}
	}
	else
	{
		/* we've found the file; seek that dirent */
		err = rt11_enum_seek(image, direnum, this_segment, this_index);
		if (err)
			goto done;
	}

	err = IMGTOOLERR_SUCCESS;
done:
	return err;
}

static imgtoolerr_t rt11_read_bootblock(imgtool::partition &partition, imgtool::stream &stream)
{
	imgtoolerr_t err;
	uint8_t block[BLOCK_SIZE];

	err = rt11_image_readblock(partition.image(), block, 0);
	if (err)
		return err;

	stream.write(block, sizeof(block));
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t rt11_image_readfile(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &destf)
{
	imgtoolerr_t err;
	imgtool::image &image(partition.image());
	rt11_dirent rt_ent;
	uint8_t buffer[BLOCK_SIZE];

	if (filename == FILENAME_BOOTBLOCK)
		return rt11_read_bootblock(partition, destf);

	err = rt11_lookup_path(image, filename, CREATE_NONE, NULL, &rt_ent);
	if (err)
		return err;

	for (uint16_t i = rt_ent.data; i < rt_ent.data + (rt_ent.filesize / BLOCK_SIZE); i++)
	{
		err = rt11_image_readblock(image, buffer, i);
		if (err)
			return err;

		destf.write(buffer, BLOCK_SIZE);
	}

	return IMGTOOLERR_SUCCESS;
}


void rt11_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_PREFER_UCASE:                  info->i = 1; break;
		case IMGTOOLINFO_INT_OPEN_IS_STRICT:                info->i = 1; break;
		case IMGTOOLINFO_INT_SUPPORTS_LASTMODIFIED_TIME:    info->i = 1; break;
		case IMGTOOLINFO_INT_SUPPORTS_BOOTBLOCK:            info->i = 1; break;
		case IMGTOOLINFO_INT_BLOCK_SIZE:                    info->i = BLOCK_SIZE; break;
		case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES:             info->i = sizeof(rt11_diskinfo); break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:         info->i = sizeof(rt11_direnum); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "rt11"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "RT11 format"); break;
		case IMGTOOLINFO_STR_FILE:                          strcpy(info->s = imgtool_temp_str(), __FILE__); break;
		case IMGTOOLINFO_STR_EOLN:                          strcpy(info->s = imgtool_temp_str(), EOLN_CRLF); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_INFO:                          info->info = rt11_image_info; break;
		case IMGTOOLINFO_PTR_MAKE_CLASS:                    info->make_class = imgtool_floppy_make_class; break;
		case IMGTOOLINFO_PTR_GET_GEOMETRY:                  info->get_geometry = rt11_image_get_geometry; break;
		case IMGTOOLINFO_PTR_READ_BLOCK:                    info->read_block = rt11_image_readblock; break;
		case IMGTOOLINFO_PTR_BEGIN_ENUM:                    info->begin_enum = rt11_image_beginenum; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                     info->next_enum = rt11_image_nextenum; break;
		case IMGTOOLINFO_PTR_READ_FILE:                     info->read_file = rt11_image_readfile; break;

		case IMGTOOLINFO_PTR_FLOPPY_OPEN:                   info->open = rt11_image_open; break;
		case IMGTOOLINFO_PTR_FLOPPY_FORMAT:                 info->p = (void *) floppyoptions_default; break;
	}
}
