// license:BSD-3-Clause
// copyright-holders:Dirk Best
/****************************************************************************

    vzdos.cpp

    Laser/VZ disk images

****************************************************************************/

#include "imgtool.h"
#include "filter.h"
#include "iflopimg.h"

#include "formats/vt_dsk_legacy.h"

#include "corestr.h"
#include "hashing.h"
#include "multibyte.h"
#include "opresolv.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/*

sector format

| GAP1 | IDAM | GAP2 | DATA | CHECKSUM | ...

GAP1     = 80 80 80 80 80 00                                         6
IDAM     = FE E7 18 C3 Track# Sector# Checksum-8 of prev. 2 bytes    7
GAP2     = 80 80 80 80 80 80 00 C3 18 E7 FE                         11
DATA     = 126 bytes actual data                                   126
         = nextTrack# nextSector#                                    2
CHECKSUM = Checksum-16 of the previous 128 bytes                     2
                                                                   ---
                                                                   154 bytes

sectors 0 to 14 are used for the disk directory. sector 15 is used
as track map, with one bit for each sector used.

*/

#define DATA_SIZE   (126)
#define SECTOR_SIZE (0x9b)
#define MAX_DIRENTS (15*8)

/* vzdos directry entry */
struct vzdos_dirent
{
	struct {
		char ftype;
		char delimitor;
		char fname[8];
	} node;
	uint8_t start_track;
	uint8_t start_sector;
	uint16_t start_address;
	uint16_t end_address;

	vzdos_dirent() :
		start_track(0), start_sector(0),
		start_address(0), end_address(0)
	{
		memset(&node, 0, sizeof(node));
	}
};

struct vz_iterator
{
	int index;
	int eof;
};

static const uint8_t sector_order[] =
{
	0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13
};

/*********************************************************************
    Internal functions
*********************************************************************/

/* get length of filename without trailing spaces */
static int vzdos_get_fname_len(const char *fname)
{
	int len;

	for (len = 7; len > 0; len--)
		if (fname[len] != 0x20)
			break;

	return len;
}

/* returns the offset where the actual sector data starts */
static imgtoolerr_t vzdos_get_data_start(imgtool::image &img, int track, int sector, int *start)
{
	imgtoolerr_t ret;
	uint8_t buffer[25]; /* enough to read the sector header */

	ret = (imgtoolerr_t)floppy_read_sector(imgtool_floppy(img), 0, track, sector_order[sector], 0, &buffer, sizeof(buffer));
	if (ret) return ret;

	/* search for start of data */
	if (memcmp(buffer + 19, "\xC3\x18\xE7\xFE", 4) == 0)
		*start = 23;
	else if (memcmp(buffer + 20, "\xC3\x18\xE7\xFE", 4) == 0)
		*start = 24;
	else if (memcmp(buffer + 21, "\xC3\x18\xE7\xFE", 4) == 0)
		*start = 25;
	else
		return IMGTOOLERR_CORRUPTIMAGE;

	return IMGTOOLERR_SUCCESS;
}

/* return the actual data of a sector */
static imgtoolerr_t vzdos_read_sector_data(imgtool::image &img, int track, int sector, uint8_t *data)
{
	int ret, data_start;
	uint8_t buffer[DATA_SIZE + 4]; /* data + checksum */

	ret = vzdos_get_data_start(img, track, sector, &data_start);
	if (ret) return (imgtoolerr_t)ret;

	ret = floppy_read_sector(imgtool_floppy(img), 0, track, sector_order[sector], data_start, &buffer, sizeof(buffer));
	if (ret) return (imgtoolerr_t)ret;

	/* verify sector checksums */
	if (get_u16le(&buffer[DATA_SIZE + 2]) != util::sum16_creator::simple(buffer, DATA_SIZE + 2))
		return IMGTOOLERR_CORRUPTFILE;

	memcpy(data, &buffer, DATA_SIZE + 2);

	return IMGTOOLERR_SUCCESS;
}

/* write data to sector */
static imgtoolerr_t vzdos_write_sector_data(imgtool::image &img, int track, int sector, uint8_t *data)
{
	int ret, data_start;
	uint8_t buffer[DATA_SIZE + 4]; /* data + checksum */

	ret = vzdos_get_data_start(img, track, sector, &data_start);
	if (ret) return (imgtoolerr_t)ret;

	memcpy(buffer, data, DATA_SIZE + 2);
	put_u16le(&buffer[DATA_SIZE + 2], util::sum16_creator::simple(data, DATA_SIZE + 2));

	ret = floppy_write_sector(imgtool_floppy(img), 0, track, sector_order[sector], data_start, buffer, sizeof(buffer), 0);  /* TODO: pass ddam argument from imgtool */
	if (ret) return (imgtoolerr_t)ret;

	return IMGTOOLERR_SUCCESS;
}

/* write formatted empty sector */
static imgtoolerr_t vzdos_clear_sector(imgtool::image &img, int track, int sector)
{
	uint8_t data[DATA_SIZE + 2];
	std::fill(std::begin(data), std::end(data), 0x00);

	return vzdos_write_sector_data(img, track, sector, data);
}

/* return a directory entry for an index */
static imgtoolerr_t vzdos_get_dirent(imgtool::image &img, int index, vzdos_dirent *ent)
{
	uint8_t buffer[DATA_SIZE + 2];

	imgtoolerr_t const ret = vzdos_read_sector_data(img, 0, int(index) / 8, buffer);
	if (IMGTOOLERR_SUCCESS != ret)
		return ret;

	int const entry = ((index % 8) * sizeof(vzdos_dirent));

	memcpy(&ent->node, &buffer[entry], 10);
	ent->start_track   = buffer[entry + 10];
	ent->start_sector  = buffer[entry + 11];
	ent->start_address = get_u16le(&buffer[entry + 12]);
	ent->end_address   = get_u16le(&buffer[entry + 14]);

	if (ent->node.ftype == 0x00)
		return IMGTOOLERR_FILENOTFOUND;

	/* check values */
	if (ent->start_track > 39)
		return IMGTOOLERR_CORRUPTFILE;

	if (ent->start_sector > 15)
		return IMGTOOLERR_CORRUPTFILE;

	return IMGTOOLERR_SUCCESS;
}

/* save a directory entry to disk */
static imgtoolerr_t vzdos_set_dirent(imgtool::image &img, int index, vzdos_dirent ent)
{
	int ret, entry;
	uint8_t buffer[DATA_SIZE + 2];

	/* read current sector with entries */
	ret = vzdos_read_sector_data(img, 0, (int) index / 8, buffer);
	if (ret) return (imgtoolerr_t)ret;

	entry = ((index % 8) * sizeof(vzdos_dirent));

	memcpy(&buffer[entry], &ent, 10);
	buffer[entry + 10] = ent.start_track;
	buffer[entry + 11] = ent.start_sector;
	put_u16le(&buffer[entry + 12], ent.start_address);
	put_u16le(&buffer[entry + 14], ent.end_address);

	/* save new sector */
	ret = vzdos_write_sector_data(img, 0, (int) index / 8, buffer);
	if (ret) return (imgtoolerr_t)ret;

	return IMGTOOLERR_SUCCESS;
}

/* clear a directory entry */
static imgtoolerr_t vzdos_clear_dirent(imgtool::image &img, int index)
{
	vzdos_dirent entry;
	return vzdos_set_dirent(img, index, entry);
}

/* search the index for a directory entry */
static imgtoolerr_t vzdos_searchentry(imgtool::image &image, const char *fname, int *entry) {
	vzdos_dirent ent;
	char filename[9];

	/* check for invalid filenames */
	if (strlen(fname) > 8)
		return IMGTOOLERR_BADFILENAME;

	/* TODO: check for invalid characters */

	*entry = -1;

	for (int i = 0; i < MAX_DIRENTS; i++) {
		imgtoolerr_t const ret = vzdos_get_dirent(image, i, &ent);
		if (IMGTOOLERR_SUCCESS != ret)
			return ret;

		int const len = vzdos_get_fname_len(ent.node.fname) + 1;

		if (strlen(fname) != len)
			continue;

		std::fill(std::begin(filename), std::end(filename), 0x00);
		memcpy(filename, ent.node.fname, len);

		if (!core_stricmp(fname, filename)) {
			*entry = i;
			break;
		}
	}

	if (*entry == -1)
		return IMGTOOLERR_FILENOTFOUND;

	return IMGTOOLERR_SUCCESS;
}

/* return a directory entry for a filename */
static imgtoolerr_t vzdos_get_dirent_fname(imgtool::image &img, const char *fname, vzdos_dirent *ent)
{
	int ret, index;

	ret = vzdos_searchentry(img, fname, &index);
	if (ret) return (imgtoolerr_t)ret;

	ret = vzdos_get_dirent(img, index, ent);
	if (ret) return (imgtoolerr_t)ret;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t vzdos_toggle_trackmap(imgtool::image &img, int track, int sector, int clear)
{
	int ret, value, bit;
	uint8_t buffer[DATA_SIZE + 2];

	/* get trackmap from image */
	ret = vzdos_read_sector_data(img, 0, 15, buffer);
	if (ret) return (imgtoolerr_t)ret;

	value = (int) (floor(((double)track * 16 + sector) / 8) - 1);
	bit   = 0x01 << ((track * 16 + sector) % 8);

	if (clear)
		buffer[value-1] &= ~bit;
	else
		buffer[value-1] |= bit;

	ret = vzdos_write_sector_data(img, 0, 15, buffer);
	if (ret) return (imgtoolerr_t)ret;

	return IMGTOOLERR_SUCCESS;
}

/* clear a trackmap entry */
static imgtoolerr_t vzdos_clear_trackmap(imgtool::image &img, int track, int sector)
{
	return vzdos_toggle_trackmap(img, track, sector, 1);
}

/* enable a trackmap entry */
static imgtoolerr_t vzdos_set_trackmap(imgtool::image &img, int track, int sector)
{
	return vzdos_toggle_trackmap(img, track, sector, 0);
}

/* return the status of a trackmap entry */
static imgtoolerr_t vzdos_get_trackmap(imgtool::image &img, int track, int sector, int *used)
{
	int ret, value, bit;
	uint8_t buffer[DATA_SIZE + 2];

	/* get trackmap from image */
	ret = vzdos_read_sector_data(img, 0, 15, buffer);
	if (ret) return (imgtoolerr_t)ret;

	value = (int) (floor(((double)track * 16 + sector) / 8) - 1);
	bit   = 0x01 << ((track * 16 + sector) % 8);

	if (buffer[value-1] & bit)
		*used = 1;
	else
		*used = 0;

	return IMGTOOLERR_SUCCESS;
}

/* return the next free sector */
static imgtoolerr_t vzdos_free_trackmap(imgtool::image &img, int *track, int *sector)
{
	int ret, used = 0;

	for (*track = 1; *track < 40; (*track)++) {
		for (*sector = 0; *sector < 16; (*sector)++) {
			ret = vzdos_get_trackmap(img, *track, *sector, &used);
			if (ret) return (imgtoolerr_t)ret;
			if (!used) return IMGTOOLERR_SUCCESS;
		}
	}

	return IMGTOOLERR_NOSPACE;
}

static imgtoolerr_t vzdos_write_formatted_sector(imgtool::image &img, int track, int sector)
{
	uint8_t sector_data[DATA_SIZE + 4 + 24];

	static const uint8_t sector_header[24] = {
		0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0xFE, 0xE7,
		0x18, 0xC3, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80,
		0x80, 0x80, 0x80, 0x00, 0xC3, 0x18, 0xE7, 0xFE
	};

	std::fill(std::begin(sector_data), std::end(sector_data), 0x00);
	memcpy(sector_data, sector_header, sizeof(sector_header));

	sector_data[10] = (uint8_t) track;            /* current track */
	sector_data[11] = (uint8_t) sector;           /* current sector */
	sector_data[12] = (uint8_t) track + sector;   /* checksum-8 */

	floperr_t const ret = floppy_write_sector(imgtool_floppy(img), 0, track, sector_order[sector], 0, sector_data, sizeof(sector_data), 0); /* TODO: pass ddam argument from imgtool */
	if (FLOPPY_ERROR_SUCCESS != ret)
		return IMGTOOLERR_WRITEERROR;

	return IMGTOOLERR_SUCCESS;
}

/*********************************************************************
    Imgtool module code
*********************************************************************/

static imgtoolerr_t vzdos_diskimage_beginenum(imgtool::directory &enumeration, const char *path)
{
	vz_iterator *iter;

	iter = (vz_iterator *) enumeration.extra_bytes();
	if (!iter) return IMGTOOLERR_OUTOFMEMORY;

	iter->index = 1;
	iter->eof = 0;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t vzdos_diskimage_nextenum(imgtool::directory &enumeration, imgtool_dirent &ent)
{
	vz_iterator *iter = (vz_iterator *) enumeration.extra_bytes();

	if (iter->eof == 1 || iter->index > MAX_DIRENTS) {
		ent.eof = 1;

	} else {
		const char *type;
		int ret, len;
		vzdos_dirent dirent;

		ret = vzdos_get_dirent(enumeration.image(), iter->index - 1, &dirent);

		if (ret == IMGTOOLERR_FILENOTFOUND)
		{
			iter->eof = 1;
			ent.eof = 1;
			return IMGTOOLERR_SUCCESS;
		}

		if (ret == IMGTOOLERR_CORRUPTFILE)
			ent.corrupt = 1;

		/* kill trailing spaces */
		for (len = 7; len > 0; len--) {
			if (dirent.node.fname[len] != 0x20) {
				break;
			}
		}

		memcpy(ent.filename, &dirent.node.fname, len + 1);
		ent.filesize = dirent.end_address - dirent.start_address;

		switch (dirent.node.ftype)
		{
		case 0x01: type = "Deleted";    break;
		case 'T':  type = "Basic";      break;
		case 'B':  type = "Binary";     break;
		case 'D':  type = "Data";       break;
		case 'F':  type = "Quickwrite"; break;
		case 'A':  type = "Assembler";  break;
		case 'S':  type = "Diskops";    break;
		case 'W':  type = "Wordpro";    break;
		default:   type = "Unknown";
		}

		snprintf(ent.attr, std::size(ent.attr), "%s", type);

		iter->index++;
	}

	return IMGTOOLERR_SUCCESS;
}

/* TRK 0 sector 15 is used to hold the track map of the disk with one bit
   corresponding to a sector used. */
static imgtoolerr_t vzdos_diskimage_freespace(imgtool::partition &partition, uint64_t *size)
{
	imgtoolerr_t ret;
	int i;
	imgtool::image &image(partition.image());
	uint8_t c, v, buffer[DATA_SIZE + 2];
	*size = 0;

	ret = vzdos_read_sector_data(image, 0, 15, buffer);
	if (ret) return ret;

	for (i = 0; i < DATA_SIZE; i++) {
		v = buffer[i];
		for (c = 0; v; c++) {
			v &= v - 1;
		}
		*size += c * DATA_SIZE;
	}
	*size = (DATA_SIZE * 16 * 39) - *size;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t vzdos_diskimage_readfile(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &destf)
{
	imgtoolerr_t ret;
	imgtool::image &image(partition.image());
	int filesize, track, sector;
	vzdos_dirent ent;
	uint8_t buffer[DATA_SIZE + 2];

	ret = vzdos_get_dirent_fname(image, filename, &ent);
	if (ret) return ret;

	filesize = ent.end_address - ent.start_address;
	track = ent.start_track;
	sector = ent.start_sector;

	while (filesize > 0) {
		int towrite;

		ret = vzdos_read_sector_data(image, track, sector, buffer);
		if (ret) return ret;

		/* detect sectors pointing to themselfs */
		if ((track == (int)buffer[DATA_SIZE]) && (sector == (int)buffer[DATA_SIZE + 1]))
			return IMGTOOLERR_CORRUPTIMAGE;

		/* load next track and sector values */
		track  = buffer[DATA_SIZE];
		sector = buffer[DATA_SIZE + 1];

		/* track 0 is invalid */
		if ((track == 0) && (filesize > DATA_SIZE))
			return IMGTOOLERR_CORRUPTIMAGE;

		/* write either DATA_SIZE or the remaining bytes */
		towrite = filesize > DATA_SIZE ? DATA_SIZE : filesize;

		if (destf.write(buffer, towrite) != towrite)
			return IMGTOOLERR_WRITEERROR;

		filesize -= DATA_SIZE;
	}

	return IMGTOOLERR_SUCCESS;
}

/* deletes directory entry, clears trackmap entries and sectors */
static imgtoolerr_t vzdos_diskimage_deletefile(imgtool::partition &partition, const char *fname)
{
	imgtoolerr_t ret;
	imgtool::image &img(partition.image());
	int index, filesize, track, sector, next_track, next_sector;
	vzdos_dirent entry, next_entry;
	uint8_t buffer[DATA_SIZE + 2];

	ret = vzdos_get_dirent_fname(img, fname, &entry);
	if (ret) return ret;

	ret = (imgtoolerr_t)floppy_read_sector(imgtool_floppy(img), 0, entry.start_track, sector_order[entry.start_sector], 24, &buffer, DATA_SIZE + 2);
	if (ret) return ret;

	filesize = entry.end_address - entry.start_address;
	track    = entry.start_track;
	sector   = entry.start_sector;

	/* delete directory entry */
	ret = vzdos_searchentry(img, fname, &index);
	if (ret) return ret;

	ret = vzdos_get_dirent(img, index + 1, &next_entry);

	if (ret == IMGTOOLERR_FILENOTFOUND) {
		/* we are the last directory entry, just delete it */
		ret = vzdos_clear_dirent(img, index);
		if (ret) return ret;

	} else if (ret) {
		/* an error occurred */
		return ret;

	} else {
		ret = vzdos_set_dirent(img, index++, next_entry);
		if (ret) return ret;

		while ((ret = vzdos_get_dirent(img, index + 1, &next_entry)) != IMGTOOLERR_FILENOTFOUND) {
			if (ret) return ret;
			ret = vzdos_set_dirent(img, index++, next_entry);
			if (ret) return ret;
		}

		ret = vzdos_clear_dirent(img, index);
		if (ret) return ret;

	}

	/* clear sectors and trackmap entries */
	while (filesize > 0) {
		filesize -= DATA_SIZE;

		/* clear trackmap entry */
		ret = vzdos_clear_trackmap(img, track, sector);
		if (ret) return ret;

		/* load next track and sector values */
		next_track  = buffer[DATA_SIZE];
		next_sector = buffer[DATA_SIZE + 1];

		/* overwrite sector with default values */
		ret = vzdos_clear_sector(img, track, sector);
		if (ret) return ret;

		/* read next sector */
		track  = next_track;
		sector = next_sector;

		if (filesize > 0) {
			ret = (imgtoolerr_t)floppy_read_sector(imgtool_floppy(img), 0, track, sector_order[sector], 24, &buffer, DATA_SIZE + 2);
			if (ret) return ret;
		}

	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t vzdos_writefile(imgtool::partition &partition, int offset, imgtool::stream &sourcef, vzdos_dirent *entry)
{
	imgtoolerr_t ret;
	imgtool::image &img(partition.image());
	int index, track, sector, toread, next_track, next_sector;
	vzdos_dirent temp_entry;
	uint64_t filesize = 0, freespace = 0;
	uint8_t buffer[DATA_SIZE + 2];
	char filename[9];

	/* is the image writeable? */
	if (floppy_is_read_only(imgtool_floppy(img)))
		return IMGTOOLERR_READONLY;

	/* check for already existing filename -> overwrite */
	strcpy(filename, entry->node.fname);
	filename[vzdos_get_fname_len(entry->node.fname) + 1] = 0x00;
	ret = vzdos_get_dirent_fname(img, filename, &temp_entry);
	if (!ret) {
		/* file already exists, delete it */
		ret = vzdos_diskimage_deletefile(partition, filename);
		if (ret) return ret;
	} else if (ret != IMGTOOLERR_FILENOTFOUND) {
		/* another error occurred, return it */
		return ret;
	}

	ret = (imgtoolerr_t) sourcef.seek(offset, SEEK_SET);
	if (ret) return ret;

	/* check if there is enough space */
	filesize = sourcef.size() - offset;

	ret = vzdos_diskimage_freespace(partition, &freespace);
	if (ret) return ret;

	if (filesize > freespace)
		return IMGTOOLERR_NOSPACE;

	/* get next free track and sector */
	ret = vzdos_free_trackmap(img, &track, &sector);
	if (ret) return ret;

	entry->end_address = entry->start_address + (unsigned int) filesize;
	entry->start_track   = track;
	entry->start_sector  = sector;

	/* search for next free directory entry */
	for (index = 0; index < MAX_DIRENTS; index++) {
		ret = vzdos_get_dirent(img, index, &temp_entry);
		if (ret == IMGTOOLERR_FILENOTFOUND)
			break;
		else if (ret)
			return (ret);
	}

	/* write directory entry to disk */
	ret = vzdos_set_dirent(img, index, *entry);
	if (ret) return ret;

	next_track  = 0;
	next_sector = 0;

	/* write data to disk */
	while (filesize > 0) {
		toread = filesize > DATA_SIZE ? DATA_SIZE : filesize;
		sourcef.read(buffer, toread);

		filesize -= toread;

		/* mark sector as used */
		ret = vzdos_set_trackmap(img, track, sector);
		if (ret) return ret;

		/* get track and sector for next sector */
		if (filesize > 0) {
			ret = vzdos_free_trackmap(img, &next_track, &next_sector);
			if (ret) return ret;
		} else {
			next_track = 0;
			next_sector = 0;
		}
		buffer[DATA_SIZE]     = next_track;
		buffer[DATA_SIZE + 1] = next_sector;

		/* write sector to disk */
		ret = vzdos_write_sector_data(img, track, sector, buffer);
		if (ret) return ret;

		track  = next_track;
		sector = next_sector;
	}

	return IMGTOOLERR_SUCCESS;

}

/* create a new file or overwrite a file */
static imgtoolerr_t vzdos_diskimage_writefile(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
{
	vzdos_dirent entry;

	/* TODO: check for leading spaces and strip */
	if (strlen(filename) > 8 || strlen(filename) < 1)
		return IMGTOOLERR_BADFILENAME;

	/* prepare directory entry */
	int const ftype = opts->lookup_int('T');

	switch (ftype) {
		case 0:
			entry.node.ftype    = 'T';
			entry.start_address = 31465;
			break;
		case 1:
			entry.node.ftype    = 'B';
			entry.start_address = 31465; /* ??? */
			break;
		case 2:
			entry.node.ftype    = 'D';
			entry.start_address = 31465; /* ??? */
			break;
		default:
			break;
	}

	entry.node.delimitor = ':';
	memset(&entry.node.fname, 0x20, 8); /* pad with spaces */
	memcpy(&entry.node.fname, filename, strlen(filename));

	/* write file to disk */
	return vzdos_writefile(partition, 0, sourcef, &entry);
}

static imgtoolerr_t vzdos_diskimage_suggesttransfer(imgtool::partition &partition, const char *fname, imgtool::transfer_suggestion *suggestions, size_t suggestions_length)
{
	imgtoolerr_t ret;
	imgtool::image &image(partition.image());
	vzdos_dirent entry;

	if (fname) {
		ret = vzdos_get_dirent_fname(image, fname, &entry);
		if (ret) return ret;

		switch (entry.node.ftype) {
			case 'B':
				suggestions[0].viability = imgtool::SUGGESTION_RECOMMENDED;
				suggestions[0].filter = filter_vzsnapshot_getinfo;
				suggestions[0].description = "VZ Snapshot";
				suggestions[1].viability = imgtool::SUGGESTION_POSSIBLE;
				suggestions[1].filter = nullptr;
				suggestions[1].description = "Raw";
				break;
			case 'T':
				suggestions[0].viability = imgtool::SUGGESTION_RECOMMENDED;
				suggestions[0].filter = filter_vzsnapshot_getinfo;
				suggestions[0].description = "VZ Snapshot";
				suggestions[1].viability = imgtool::SUGGESTION_POSSIBLE;
				suggestions[1].filter = nullptr;
				suggestions[1].description = "Raw";
				suggestions[2].viability = imgtool::SUGGESTION_POSSIBLE;
				suggestions[2].filter = filter_vzbas_getinfo;
				suggestions[2].description = "Tokenized Basic";
				break;
			default:
				suggestions[0].viability = imgtool::SUGGESTION_RECOMMENDED;
				suggestions[0].filter = nullptr;
				suggestions[0].description = "Raw";
		}

	} else {
		suggestions[0].viability = imgtool::SUGGESTION_RECOMMENDED;
		suggestions[0].filter = nullptr;
		suggestions[0].description = "Raw";
		suggestions[1].viability = imgtool::SUGGESTION_POSSIBLE;
		suggestions[1].filter = filter_vzsnapshot_getinfo;
		suggestions[1].description = "VZ Snapshot";
	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t vzdos_diskimage_create(imgtool::image &img, imgtool::stream::ptr &&dummy, util::option_resolution *opts)
{
	imgtoolerr_t ret;
	int track, sector;

	for (track = 0; track < 40; track++) {
		for (sector = 0; sector < 16; sector++) {
			ret = vzdos_write_formatted_sector(img, track, sector);
			if (ret) return ret;
		}
	}

	return IMGTOOLERR_SUCCESS;
}

/*********************************************************************
    Imgtool vz filter code
*********************************************************************/

static imgtoolerr_t vzsnapshot_readfile(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &destf)
{
	imgtoolerr_t ret;
	imgtool::image &image(partition.image());
	vzdos_dirent entry;
	uint8_t header[24];

	/* get directory entry from disk */
	ret = vzdos_get_dirent_fname(image, filename, &entry);
	if (ret) return ret;

	/* prepare header */
	header[0] = 'V';
	header[1] = 'Z';
	header[2] = 'F';

	switch (entry.node.ftype) {
		case 'B':
			header[3] = '1';
			header[21] = 0xF1;
			break;
		case 'T':
			header[3] = '0';
			header[21] = 0xF0;
			break;
		default:
			memset(header, 0x00, 4);
			header[21] = 0x00;
	}

	memset(header + 4, 0x00, 17);
	memcpy(header + 4, entry.node.fname, vzdos_get_fname_len(entry.node.fname) + 1);
	put_u16le(&header[22], entry.start_address);

	/* write header to file */
	destf.write(header, sizeof(header));

	/* write data to file */
	ret = vzdos_diskimage_readfile(partition, filename, "", destf);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t vzsnapshot_writefile(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
{
	imgtoolerr_t ret;
	int fnameopt;
	vzdos_dirent entry;
	uint8_t header[24];

	/* get header infos from file */
	sourcef.read(header, sizeof(header));

	/* prepare directory entry */
	entry.node.ftype     = (header[21] == 0xF1) ? 'B' : 'T';
	entry.node.delimitor = ':';
	entry.start_address  = get_u16le(&header[22]);

	/* filename from header or directly? */
	fnameopt = opts->lookup_int('F');

	if (fnameopt == 0) {
		memcpy(&entry.node.fname, &header[4], 8);
	} else {
		/* TODO: check for leading spaces and strip */
		if (strlen(filename) > 8 || strlen(filename) < 1)
		return IMGTOOLERR_BADFILENAME;

		memcpy(&entry.node.fname, filename, strlen(filename) - 3);
	}

	/* write file to disk */
	ret = vzdos_writefile(partition, 24, sourcef, &entry);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}

void filter_vzsnapshot_getinfo(uint32_t state, imgtool::filterinfo *info)
{
	switch(state)
	{
		case FILTINFO_STR_NAME:         info->s = "vzsnapshot"; break;
		case FILTINFO_STR_HUMANNAME:    info->s = "VZ Snapshot"; break;
		case FILTINFO_STR_EXTENSION:    info->s = "vz"; break;
		case FILTINFO_PTR_READFILE:     info->read_file = vzsnapshot_readfile; break;
		case FILTINFO_PTR_WRITEFILE:    info->write_file = vzsnapshot_writefile; break;
	}
}

/*********************************************************************
    Imgtool module declaration
*********************************************************************/

OPTION_GUIDE_START(vzdos_writefile_optionguide)
	OPTION_ENUM_START( 'T', "ftype",  "File type" )
		OPTION_ENUM(     0, "basic",  "Basic"  )
		OPTION_ENUM(     1, "binary", "Binary" )
		OPTION_ENUM(     2, "data",   "Data"   )
	OPTION_ENUM_END
	OPTION_ENUM_START( 'F', "fname", "Filename" )
		OPTION_ENUM(     0, "intern", "Filename from VZ-Header" )
		OPTION_ENUM(     1, "extern", "Actual filename"         )
	OPTION_ENUM_END
OPTION_GUIDE_END

/*
T   Basic Editor File
B   Binary File
D   Sequential Access Program Data File
F   Quickwrite Document
A   Russell Harrison's Edit Ass. File
S   Dave Mitchell/Mark Hardwood Edit Ass. File  Start Addr A280H
S   Quickwrite/Diskops System File/Label        Except Above
W   Edit Ass. with Diskops File Start           Addr A813H
W   E & F Word Pro with Patch 3.3 File          End Addr D000H
W   Russell Harrison Word Pro File              Except above two
*/

void vzdos_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_PREFER_UCASE:                  info->i = 1; break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:             info->i = sizeof(vz_iterator); break;

		/* --- the following bits of info are returned as NUL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "vzdos"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "VZ-DOS format"); break;
		case IMGTOOLINFO_STR_FILE:                          strcpy(info->s = imgtool_temp_str(), __FILE__); break;
		case IMGTOOLINFO_STR_WRITEFILE_OPTSPEC:             strcpy(info->s = imgtool_temp_str(), "T[0]-2;F[0]-1"); break;
		case IMGTOOLINFO_STR_EOLN:                          info->s = nullptr; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_MAKE_CLASS:                    info->make_class = imgtool_floppy_make_class; break;
		case IMGTOOLINFO_PTR_OPEN:                          info->open = nullptr; break;
		case IMGTOOLINFO_PTR_BEGIN_ENUM:                    info->begin_enum = vzdos_diskimage_beginenum; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                     info->next_enum = vzdos_diskimage_nextenum; break;
		case IMGTOOLINFO_PTR_FREE_SPACE:                    info->free_space = vzdos_diskimage_freespace; break;
		case IMGTOOLINFO_PTR_READ_FILE:                     info->read_file = vzdos_diskimage_readfile; break;
		case IMGTOOLINFO_PTR_WRITE_FILE:                    info->write_file = vzdos_diskimage_writefile; break;
		case IMGTOOLINFO_PTR_DELETE_FILE:                   info->delete_file = vzdos_diskimage_deletefile; break;
		case IMGTOOLINFO_PTR_FLOPPY_CREATE:                 info->create = vzdos_diskimage_create; break;
		case IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE:            info->writefile_optguide = &vzdos_writefile_optionguide; break;
		case IMGTOOLINFO_PTR_SUGGEST_TRANSFER:              info->suggest_transfer = vzdos_diskimage_suggesttransfer; break;
		case IMGTOOLINFO_PTR_FLOPPY_FORMAT:                 info->p = (void *) floppyoptions_vz; break;
	}
}
