// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/****************************************************************************

    bml3.c

    Hitachi bml3 disk images

    By Jonathan Edwards, based on rsdos.c (both use Microsoft BASIC)

****************************************************************************/

/* Supported Hitachi floppy formats are:
   - 3" or 5"1/4 single density, single-sided: 40 tracks, 16 sectors/track, 128 bytes/sector
   -   (used with MP-1805 floppy disk controller card)
   - 5"1/4 double density, double-sided: 40 tracks, 16 sectors/track, 256 bytes/sector
   -   (first track on first head may be single density)
   -   (used with MP-1802 floppy disk controller card)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "imgtool.h"
#include "iflopimg.h"

#define MAX_SECTOR_SIZE 256

struct bml3_diskinfo
{
	UINT16 sector_size;   /* 128 or 256 */
	UINT8  heads;         /* 1 or 2 */
	UINT8  fat_start_sector; /* in cylinder 20, start sector of FAT */
	UINT8  fat_start_offset; /* start byte of FAT in sector */
	UINT8  fat_sectors;   /* the number of sectors in the FAT */
	UINT8  dirent_start_sector;  /* in cylinder 20, start sector of directory entries */
	UINT8  granule_sectors;  /* how many sectors per granule */
	UINT8  first_granule_cylinder; /* the number of the first cylinder with granule numbers assigned */
	UINT8  variant;       /* 0 - older version, uses EOF to terminate files, 1 - newer version, stores file length */
};

/* this structure mirrors the structure of a directory entry on disk */
struct bml3_dirent
{
	char fname[8];
	char fext[3];
	UINT8 ftype;
	UINT8 asciiflag;
	UINT8 first_granule;
	UINT16 lastsectorbytes;
	// TODO there are some 'unused' bytes here that are sometimes used to store a timestamp, maybe support this?
};

struct bml3_direnum
{
	int index;
	int eof;
};

#define MAX_GRANULEMAP_SIZE 256

struct granule_list_t {
	UINT8 granules[MAX_GRANULEMAP_SIZE];
	UINT8 granule_count;
	UINT8 last_granule_sectors;
};

#define BML3_OPTIONS_FTYPE     'T'
#define BML3_OPTIONS_ASCII     'M'

static imgtoolerr_t bml3_diskimage_deletefile(imgtool_partition *partition, const char *fname);



/*********************************************************************
    Imgtool module code
*********************************************************************/

static bml3_diskinfo *bml3_get_diskinfo(imgtool_image *image)
{
	return (bml3_diskinfo *) imgtool_floppy_extrabytes(image);
}



static int max_dirents(imgtool_image *image)
{
	bml3_diskinfo *info = bml3_get_diskinfo(image);
	return (16 * info->heads + 1 - info->dirent_start_sector)*(info->sector_size/32);
}



static void dirent_location(imgtool_image *image, int index_loc, UINT8 *head, UINT8 *track, UINT8 *sector, UINT8 *offset)
{
	bml3_diskinfo *info = bml3_get_diskinfo(image);
	*track = 20;
	*sector = info->dirent_start_sector + index_loc / (info->sector_size / 32);
	*head = 0;
	if (*sector > 16) {
		// wrap to second head
		*sector -= 16;
		(*head)++;
	}
	*offset = index_loc % (info->sector_size/32) * 32;
}



static floperr_t get_bml3_dirent(imgtool_image *f, int index_loc, struct bml3_dirent *ent)
{
	floperr_t err;
	UINT8 head, track, sector, offset;
	UINT8 buf[32];
	bml3_diskinfo *info = bml3_get_diskinfo(f);
	dirent_location(f, index_loc, &head, &track, &sector, &offset);
	err = floppy_read_sector(imgtool_floppy(f), head, track, sector, offset, (void *) buf, sizeof(buf));
	memset(ent, 0, sizeof(*ent));
	switch (info->variant) {
	case 0:
		memcpy(&ent->fname, &buf[0], 8);
		ent->ftype = buf[11];
		ent->asciiflag = buf[12];
		ent->first_granule = buf[14];
		break;
	case 1:
		memcpy(&ent->fname, &buf[0], 8);
		memcpy(&ent->fext, &buf[8], 3);
		ent->ftype = buf[11];
		ent->asciiflag = buf[12];
		ent->first_granule = buf[13];
		ent->lastsectorbytes = (buf[14] << 8) | buf[15];
		break;
	default:
		return FLOPPY_ERROR_INVALIDIMAGE;
	}
	return err;
}



static floperr_t put_bml3_dirent(imgtool_image *f, int index_loc, const struct bml3_dirent *ent)
{
	floperr_t err;
	UINT8 head, track, sector, offset;
	UINT8 buf[32];
	bml3_diskinfo *info = bml3_get_diskinfo(f);
	if (index_loc >= max_dirents(f))
		return (floperr_t)IMGTOOLERR_FILENOTFOUND;
	dirent_location(f, index_loc, &head, &track, &sector, &offset);
	memset(buf, 0, sizeof(buf));
	switch (info->variant) {
	case 0:
		memcpy(&buf[0], &ent->fname, 8);
		buf[11] = ent->ftype;
		buf[12] = ent->asciiflag;
		buf[14] = ent->first_granule;
		break;
	case 1:
		memcpy(&buf[0], &ent->fname, 8);
		memcpy(&buf[8], &ent->fext, 3);
		buf[11] = ent->ftype;
		buf[12] = ent->asciiflag;
		buf[13] = ent->first_granule;
		buf[14] = ent->lastsectorbytes >> 8;
		buf[15] = ent->lastsectorbytes & 0xff;
		break;
	default:
		return FLOPPY_ERROR_INVALIDIMAGE;
	}
	err = floppy_write_sector(imgtool_floppy(f), head, track, sector, offset, (void *) buf, sizeof(buf), 0); /* TODO: pass ddam argument from imgtool */
	return err;
}



/* fnamebuf must have at least 13 bytes */
static void get_dirent_fname(char *fnamebuf, const struct bml3_dirent *ent)
{
	char *s;

	memset(fnamebuf, 0, 13);
	memcpy(fnamebuf, ent->fname, sizeof(ent->fname));
	rtrim(fnamebuf);
	s = fnamebuf + strlen(fnamebuf);
	*(s++) = '.';
	memcpy(s, ent->fext, sizeof(ent->fext));
	rtrim(s);

	/* If no extension, remove period */
	if (*s == '\0')
		s[-1] = '\0';
}



static imgtoolerr_t lookup_bml3_file(imgtool_image *f, const char *fname, struct bml3_dirent *ent, int *position)
{
	int i;
	floperr_t ferr;
	char fnamebuf[13];

	i = 0;
	fnamebuf[0] = '\0';

	do
	{
		do
		{
			ferr = get_bml3_dirent(f, i++, ent);
			if (ferr)
				return imgtool_floppy_error(ferr);
		}
		while(ent->fname[0] == '\0');


		if (ent->fname[0] != -1)
			get_dirent_fname(fnamebuf, ent);
	}
	while((ent->fname[0] != -1) && core_stricmp(fnamebuf, fname));

	if (ent->fname[0] == -1)
		return IMGTOOLERR_FILENOTFOUND;

	if (position)
		*position = i - 1;
	return (imgtoolerr_t)0;
}



static UINT8 get_granule_count(imgtool_image *img)
{
	// UINT16 tracks;
	UINT16 disk_granules;
	bml3_diskinfo *info = bml3_get_diskinfo(img);

	// This always returns 82 for D88, so not quite right
	// tracks = floppy_get_tracks_per_disk(imgtool_floppy(img));

	// The number of granules is primarily constrained by the disk capacity.
	disk_granules = (40 - 1 - info->first_granule_cylinder) * info->heads * (16 / info->granule_sectors);
	// Also, granule numbers from 0xC0 upwards are reserved for terminating a granule chain
	return (UINT8)((disk_granules < 0xC0) ? disk_granules : 0xC0);
}

/* granule_map must be an array of MAX_GRANULEMAP_SIZE bytes */
static floperr_t get_granule_map(imgtool_image *img, UINT8 *granule_map, UINT8 *granule_count)
{
	bml3_diskinfo *info = bml3_get_diskinfo(img);
	UINT8 count;

	count = get_granule_count(img);
	if (granule_count)
		*granule_count = count;

	// The first byte of the granule map sector is ignored (and expected to be 0)
	return floppy_read_sector(imgtool_floppy(img), 0, 20, info->fat_start_sector, info->fat_start_offset, granule_map, count);
}



static floperr_t put_granule_map(imgtool_image *img, const UINT8 *granule_map, UINT8 granule_count)
{
	bml3_diskinfo *info = bml3_get_diskinfo(img);
	return floppy_write_sector(imgtool_floppy(img), 0, 20, info->fat_start_sector, info->fat_start_offset, granule_map, granule_count, 0);    /* TODO: pass ddam argument from imgtool */
}




static void granule_location(imgtool_image *image, UINT8 granule, UINT8 *head, UINT8 *track, UINT8 *sector)
{
	bml3_diskinfo *info = bml3_get_diskinfo(image);
	UINT16 abs_track = granule * info->granule_sectors / 16;
	*head = abs_track % info->heads;
	*track = abs_track / info->heads + info->first_granule_cylinder;
	// skip filesystem cylinder
	if (*track >= 20)
		(*track)++;
	*sector = granule * info->granule_sectors % 16 + 1;
}



static imgtoolerr_t transfer_granule(imgtool_image *img, UINT8 granule, int length, imgtool_stream *f, imgtoolerr_t (*proc)(imgtool_image *, int, int, int, int, size_t, imgtool_stream *))
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	UINT8 head, track, sector;
	granule_location(img, granule, &head, &track, &sector);
	if (length > 0)
		err = proc(img, head, track, sector, 0, length, f);
	return err;
}


static imgtoolerr_t transfer_from_granule(imgtool_image *img, UINT8 granule, int length, imgtool_stream *destf)
{
	return transfer_granule(img, granule, length, destf, imgtool_floppy_read_sector_to_stream);
}



static imgtoolerr_t transfer_to_granule(imgtool_image *img, UINT8 granule, int length, imgtool_stream *sourcef)
{
	return transfer_granule(img, granule, length, sourcef, imgtool_floppy_write_sector_from_stream);
}



static floperr_t read_granule(imgtool_image *img, UINT8 granule, int offset, int length, UINT8 *buf)
{
	UINT8 head, track, sector;
	granule_location(img, granule, &head, &track, &sector);
	return floppy_read_sector(imgtool_floppy(img), head, track, sector, offset, buf, length);
}



static floperr_t write_granule(imgtool_image *img, UINT8 granule, int offset, int length, const UINT8 *buf)
{
	UINT8 head, track, sector;
	granule_location(img, granule, &head, &track, &sector);
	return floppy_write_sector(imgtool_floppy(img), head, track, sector, offset, buf, length, 0); /* TODO: pass ddam argument from imgtool */
}



static imgtoolerr_t list_granules(struct bml3_dirent *ent, imgtool_image *img, struct granule_list_t *granule_list)
{
	floperr_t ferr;
	UINT8 max_granules;
	UINT8 granule;
	UINT8 usedmap[MAX_GRANULEMAP_SIZE]; /* Used to detect infinite loops */
	UINT8 granule_map[MAX_GRANULEMAP_SIZE];
	bml3_diskinfo *info = bml3_get_diskinfo(img);

	ferr = get_granule_map(img, granule_map, &max_granules);
	if (ferr)
		return imgtool_floppy_error(ferr);

	memset(usedmap, 0, max_granules);

	granule = ent->first_granule;
	granule_list->granule_count = 0;

	while(!usedmap[granule] && granule < max_granules)
	{
		usedmap[granule] = 1;
		granule_list->granules[granule_list->granule_count++] = granule;
		granule = granule_map[granule];
	}

	granule_list->last_granule_sectors = granule - 0xc0;
	if (info->variant == 0) {
		// add final incomplete sector
		granule_list->last_granule_sectors++;
	}

	// A value of zero (variant 1) and max (variant 0) seem to indicate a file open for writing.
	// Strictly speaking this means the image is corrupt, although a real system will happily read
	//   garbage from the file.
	if (granule_list->last_granule_sectors > info->granule_sectors)
		return IMGTOOLERR_CORRUPTIMAGE;

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t get_file_size(struct bml3_dirent *ent, imgtool_image *img, const struct granule_list_t *granule_list, size_t *size)
{
	floperr_t ferr;
	size_t last_sector_bytes = 0;
	bml3_diskinfo *info = bml3_get_diskinfo(img);

	// TODO are these special cases valid, or maybe indicate a corrupt image?
	if (granule_list->granule_count == 0) {
		*size = 0;
		return IMGTOOLERR_SUCCESS;
	}
	else if (granule_list->last_granule_sectors == 0) {
		*size = info->sector_size * ((granule_list->granule_count - 1) * info->granule_sectors);
		return IMGTOOLERR_SUCCESS;
	}

	// determine size excluding final sector
	*size = info->sector_size * ((granule_list->granule_count - 1) * info->granule_sectors + granule_list->last_granule_sectors - 1);

	// determine bytes used in final sector
	switch (info->variant) {
	case 0:
		// look for EOF (ASCII SUB) and trailing NULs in final sector
		{
			UINT8 buf[MAX_SECTOR_SIZE];
			ferr = read_granule(img, granule_list->granules[granule_list->granule_count-1], info->sector_size * (granule_list->last_granule_sectors - 1), info->sector_size, buf);
			if (ferr)
				return imgtool_floppy_error(ferr);
			for (last_sector_bytes = info->sector_size - 1; ; last_sector_bytes--) {
				if (buf[last_sector_bytes] != 0)
					break;
				if (last_sector_bytes == 0)
					break;
			}
			if (buf[last_sector_bytes] != 0x1a) {
				last_sector_bytes++;
			}
		}
		break;
	case 1:
		last_sector_bytes = ent->lastsectorbytes;
		break;
	}

	// TODO is it valid for last_sector_bytes == 0?
	if (last_sector_bytes > info->sector_size) {
		return IMGTOOLERR_CORRUPTIMAGE;
	}
	*size += last_sector_bytes;
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t process_bml3_file(struct bml3_dirent *ent, imgtool_image *img, imgtool_stream *destf, size_t *size)
{
	imgtoolerr_t err;
	size_t remaining_size, granule_size;
	bml3_diskinfo *info = bml3_get_diskinfo(img);
	struct granule_list_t granule_list;
	granule_list.granule_count = 0;

	err = list_granules(ent, img, &granule_list);
	if (err)
		return err;
	err = get_file_size(ent, img, &granule_list, size);
	if (err)
		return err;

	if (destf) {
		remaining_size = *size;
		granule_size = info->granule_sectors * info->sector_size;

		for (int c = 0; c < granule_list.granule_count; c++) {
			if (granule_size >= remaining_size)
				granule_size = remaining_size;
			transfer_from_granule(img, granule_list.granules[c], granule_size, destf);
			remaining_size -= granule_size;
		}
	}
	return IMGTOOLERR_SUCCESS;
}



/* create a new directory entry with a specified name */
static imgtoolerr_t prepare_dirent(UINT8 variant, struct bml3_dirent *ent, const char *fname)
{
	const char *fname_end;
	const char *fname_ext;
	int fname_ext_len;

	memset(ent, '\0', sizeof(*ent));
	memset(ent->fname, ' ', sizeof(ent->fname));
	memset(ent->fext, ' ', sizeof(ent->fext));

	fname_end = strchr(fname, '.');
	if (fname_end)
		fname_ext = fname_end + 1;
	else
		fname_end = fname_ext = fname + strlen(fname);

	fname_ext_len = strlen(fname_ext);

	switch (variant) {
	case 0:
		/* 8-character max filename */
		if (((fname_end - fname) > 8) || (fname_ext_len > 0))
			return IMGTOOLERR_BADFILENAME;
		break;
	case 1:
		/*8.3 filename */
		if (((fname_end - fname) > 8) || (fname_ext_len > 3))
			return IMGTOOLERR_BADFILENAME;
		break;
	default:
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	memcpy(ent->fname, fname, fname_end - fname);
	memcpy(ent->fext, fname_ext, fname_ext_len);

	/* By default, set as a type 2 binary file */
	ent->ftype = 2;
	ent->asciiflag = 0;
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t bml3_diskimage_open(imgtool_image *image, imgtool_stream *stream)
{
	// imgtoolerr_t err;
	floperr_t ferr;
	bml3_diskinfo *info = bml3_get_diskinfo(image);
	floppy_image_legacy *floppy = imgtool_floppy(image);
	const struct FloppyCallbacks *callbacks = floppy_callbacks(floppy);

	// probe disk geometry to guess format
	int heads_per_disk = callbacks->get_heads_per_disk(floppy);
	UINT32 sector_length;
	ferr = callbacks->get_sector_length(floppy, 0, 20, 1, &sector_length);
	if (ferr)
		return imgtool_floppy_error(ferr);
	int sectors_per_track = callbacks->get_sectors_per_track(floppy, 0, 20);

	if (heads_per_disk == 2 && sector_length == 128 && sectors_per_track == 16) {
		// single-sided, single-density
		info->sector_size = 128;
		info->heads = 1;
		info->fat_start_sector = 1;
		info->fat_start_offset = 5;
		info->fat_sectors = 2;
		info->dirent_start_sector = 7;
		info->granule_sectors = 4;
		info->first_granule_cylinder = 0;
		info->variant = 0;
	}
	else if (heads_per_disk == 2 && sector_length == 256 && sectors_per_track == 16) {
		// double-sided, double-density
		info->sector_size = 256;
		info->heads = 2;
		info->fat_start_sector = 2;
		info->fat_start_offset = 1;
		info->fat_sectors = 1;
		info->dirent_start_sector = 5;
		info->granule_sectors = 8;
		info->first_granule_cylinder = 1;
		info->variant = 1;
	}
	else {
		// invalid or unsupported format
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t bml3_diskimage_nextenum(imgtool_directory *enumeration, imgtool_dirent *ent)
{
	floperr_t ferr;
	imgtoolerr_t err;
	size_t filesize;
	struct bml3_direnum *rsenum;
	struct bml3_dirent rsent;
	char fname[13];
	imgtool_image *image;

	image = imgtool_directory_image(enumeration);
	rsenum = (struct bml3_direnum *) imgtool_directory_extrabytes(enumeration);

	/* Did we hit the end of file before? */
	if (rsenum->eof)
		goto eof;

	do
	{
		if (rsenum->index >= max_dirents(image))
			goto eof;

		ferr = get_bml3_dirent(image, rsenum->index++, &rsent);
		if (ferr)
			return imgtool_floppy_error(ferr);
	}
	while(rsent.fname[0] == '\0');

	/* Now are we at the eof point? */
	if (rsent.fname[0] == -1)
	{
		rsenum->eof = 1;
eof:
		ent->eof = 1;
	}
	else
	{
		/* Not the end of file */
		err = process_bml3_file(&rsent, image, NULL, &filesize);
		if (err)
			return err;

		if (filesize == ((size_t) -1))
		{
			/* corrupt! */
			ent->filesize = 0;
			ent->corrupt = 1;
		}
		else
		{
			ent->filesize = filesize;
			ent->corrupt = 0;
		}
		ent->eof = 0;

		get_dirent_fname(fname, &rsent);

		snprintf(ent->filename, ARRAY_LENGTH(ent->filename), "%s", fname);
		snprintf(ent->attr, ARRAY_LENGTH(ent->attr), "%d %c", (int) rsent.ftype, (char) (rsent.asciiflag + 'B'));
	}
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t bml3_diskimage_freespace(imgtool_partition *partition, UINT64 *size)
{
	floperr_t ferr;
	UINT8 i;
	size_t s = 0;
	UINT8 granule_count;
	UINT8 granule_map[MAX_GRANULEMAP_SIZE];
	imgtool_image *image = imgtool_partition_image(partition);
	bml3_diskinfo *info = bml3_get_diskinfo(image);

	ferr = get_granule_map(image, granule_map, &granule_count);
	if (ferr)
		return imgtool_floppy_error(ferr);

	for (i = 0; i < granule_count; i++)
	{
		if (granule_map[i] == 0xff)
			s += (info->granule_sectors * info->sector_size);
	}
	*size = s;
	return (imgtoolerr_t)FLOPPY_ERROR_SUCCESS;
}



static imgtoolerr_t delete_entry(imgtool_image *img, struct bml3_dirent *ent, int pos)
{
	floperr_t ferr;
	unsigned char g, i;
	UINT8 granule_count;
	UINT8 granule_map[MAX_GRANULEMAP_SIZE];

	/* Write a NUL in the filename, marking it deleted */
	ent->fname[0] = 0;
	ferr = put_bml3_dirent(img, pos, ent);
	if (ferr)
		return imgtool_floppy_error(ferr);

	ferr = get_granule_map(img, granule_map, &granule_count);
	if (ferr)
		return imgtool_floppy_error(ferr);

	/* Now free up the granules */
	g = ent->first_granule;
	while (g < granule_count)
	{
		i = granule_map[g];
		granule_map[g] = 0xff;
		g = i;
	}

	ferr = put_granule_map(img, granule_map, granule_count);
	if (ferr)
		return imgtool_floppy_error(ferr);

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t bml3_diskimage_readfile(imgtool_partition *partition, const char *fname, const char *fork, imgtool_stream *destf)
{
	imgtoolerr_t err;
	struct bml3_dirent ent;
	size_t size;
	imgtool_image *img = imgtool_partition_image(partition);

	err = lookup_bml3_file(img, fname, &ent, NULL);
	if (err)
		return err;

	err = process_bml3_file(&ent, img, destf, &size);
	if (err)
		return err;

	if (size == (size_t) -1)
		return IMGTOOLERR_CORRUPTIMAGE;

	return (imgtoolerr_t)0;
}



static imgtoolerr_t bml3_diskimage_writefile(imgtool_partition *partition, const char *fname, const char *fork, imgtool_stream *sourcef, option_resolution *writeoptions)
{
	floperr_t ferr;
	imgtoolerr_t err;
	imgtool_image *img = imgtool_partition_image(partition);
	bml3_diskinfo *info = bml3_get_diskinfo(img);
	struct bml3_dirent ent, ent2;
	size_t i;
	UINT64 sz, read_sz;
	UINT64 freespace = 0;
	unsigned char *gptr;
	UINT8 granule_count;
	UINT8 granule_map[MAX_GRANULEMAP_SIZE];
	UINT8 eof_buf[MAX_SECTOR_SIZE];

	// one-time setup of eof_buf
	memset(eof_buf, 0, sizeof(eof_buf));
	eof_buf[0] = 0x1A;

	/* can we write to this image? */
	if (floppy_is_read_only(imgtool_floppy(img)))
		return IMGTOOLERR_READONLY;

	err = bml3_diskimage_freespace(partition, &freespace);
	if (err)
		return err;

	/* is there enough space? */
	sz = read_sz = stream_size(sourcef);
	if (info->variant == 0) {
		// also need to write EOF
		sz++;
	}
	if (sz > freespace)
		return IMGTOOLERR_NOSPACE;

	/* setup our directory entry */
	err = prepare_dirent(info->variant, &ent, fname);
	if (err)
		return err;

	ent.ftype = option_resolution_lookup_int(writeoptions, BML3_OPTIONS_FTYPE);
	ent.asciiflag = ((UINT8) option_resolution_lookup_int(writeoptions, BML3_OPTIONS_ASCII)) - 1;
	gptr = &ent.first_granule;

	ferr = get_granule_map(img, granule_map, &granule_count);
	if (ferr)
		return imgtool_floppy_error(ferr);

	unsigned char g = 0x00;
	UINT32 granule_bytes = info->granule_sectors * info->sector_size;

	do
	{
		while (granule_map[g] != 0xff)
		{
			g++;
			if ((g >= granule_count) || (g == 0))
				return IMGTOOLERR_UNEXPECTED;   /* We should have already verified that there is enough space */
		}
		*gptr = g;
		gptr = &granule_map[g];


		i = MIN(read_sz, granule_bytes);
		if (i > 0) {
			err = transfer_to_granule(img, g, i, sourcef);
			if (err)
				return err;
			read_sz -= i;
			sz -= i;
		}
		if (i < info->granule_sectors * info->sector_size && sz > 0) {
			// write EOF and trailing NULs in the final sector
			ferr = write_granule(img, g, i, (info->granule_sectors * info->sector_size - i - 1) % info->sector_size + 1, eof_buf);
			if (ferr)
				return imgtool_floppy_error(ferr);
			sz--;
			i++;
		}

		/* Go to next granule */
		g++;
	}
	while(sz > 0);

	/* Now that we are done with the file, we need to specify the final entry
	 * in the file allocation table
	 */
	*gptr = 0xc0 + ((i + info->sector_size-1) / info->sector_size) - (info->variant == 0 ? 1 : 0);
	ent.lastsectorbytes = (i - 1) % info->sector_size + 1;

	/* delete file if it already exists */
	err = bml3_diskimage_deletefile(partition, fname);
	if (err && err != IMGTOOLERR_FILENOTFOUND)
		return err;

	/* Now we need to find an empty directory entry */
	i = -1;
	do
	{
		ferr = get_bml3_dirent(img, ++i, &ent2);
		if (ferr)
			return imgtool_floppy_error(ferr);
	}
	while(ent2.fname[0] != '\0' && ent2.fname[0] != -1);

	ferr = put_bml3_dirent(img, i, &ent);
	if (ferr)
		return imgtool_floppy_error(ferr);

	/* write the granule map back out */
	ferr = put_granule_map(img, granule_map, granule_count);
	if (ferr)
		return imgtool_floppy_error(ferr);

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t bml3_diskimage_deletefile(imgtool_partition *partition, const char *fname)
{
	imgtoolerr_t err;
	imgtool_image *image = imgtool_partition_image(partition);
	int pos = 0;
	struct bml3_dirent ent;

	err = lookup_bml3_file(image, fname, &ent, &pos);
	if (err)
		return err;

	return delete_entry(image, &ent, pos);
}



static imgtoolerr_t bml3_diskimage_suggesttransfer(imgtool_partition *partition, const char *fname, imgtool_transfer_suggestion *suggestions, size_t suggestions_length)
{
	imgtoolerr_t err;
	imgtool_image *image = imgtool_partition_image(partition);
	struct bml3_dirent ent;
	int pos;

	if (fname)
	{
		err = lookup_bml3_file(image, fname, &ent, &pos);
		if (err)
			return err;

		if (ent.asciiflag == 0xFF)
		{
			/* ASCII file */
			suggestions[0].viability = SUGGESTION_RECOMMENDED;
			suggestions[0].filter = filter_eoln_getinfo;
			suggestions[1].viability = SUGGESTION_POSSIBLE;
			suggestions[1].filter = NULL;
		}
		else if (ent.ftype == 0)
		{
			/* tokenized BASIC file */
			suggestions[0].viability = SUGGESTION_RECOMMENDED;
			suggestions[0].filter = NULL;
			suggestions[1].viability = SUGGESTION_POSSIBLE;
			suggestions[1].filter = filter_bml3bas_getinfo;
		}
	}
	else
	{
		suggestions[0].viability = SUGGESTION_RECOMMENDED;
		suggestions[0].filter = NULL;
		suggestions[1].viability = SUGGESTION_POSSIBLE;
		suggestions[1].filter = filter_eoln_getinfo;
		suggestions[2].viability = SUGGESTION_POSSIBLE;
		suggestions[2].filter = filter_bml3bas_getinfo;
	}

	return IMGTOOLERR_SUCCESS;
}



/*********************************************************************
    Imgtool module declaration
*********************************************************************/

static OPTION_GUIDE_START( bml3_writefile_optionguide )
	OPTION_ENUM_START(  BML3_OPTIONS_FTYPE, "ftype", "File type" )
		OPTION_ENUM(    0,      "basic",        "Basic" )
		OPTION_ENUM(    1,      "data",         "Data" )
		OPTION_ENUM(    2,      "binary",       "Binary" )
		OPTION_ENUM(    3,      "assembler",    "Assembler Source" )
	OPTION_ENUM_END
	OPTION_ENUM_START(  BML3_OPTIONS_ASCII, "ascii", "Ascii flag" )
		OPTION_ENUM(    0,      "ascii",        "Ascii" )
		OPTION_ENUM(    1,      "binary",       "Binary" )
	OPTION_ENUM_END
OPTION_GUIDE_END



void bml3_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_PREFER_UCASE:                  info->i = 1; break;
		case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES:             info->i = sizeof(bml3_diskinfo); break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:             info->i = sizeof(struct bml3_direnum); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "bml3"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "Basic Master Level 3 format"); break;
		case IMGTOOLINFO_STR_FILE:                          strcpy(info->s = imgtool_temp_str(), __FILE__); break;
		case IMGTOOLINFO_STR_EOLN:                          strcpy(info->s = imgtool_temp_str(), "\r"); break;
		case IMGTOOLINFO_STR_WRITEFILE_OPTSPEC:             strcpy(info->s = imgtool_temp_str(), "T0-[2]-3;M0-[1]"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_MAKE_CLASS:                    info->make_class = imgtool_floppy_make_class; break;
		case IMGTOOLINFO_PTR_FLOPPY_OPEN:                   info->open = bml3_diskimage_open; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                     info->next_enum = bml3_diskimage_nextenum; break;
		case IMGTOOLINFO_PTR_FREE_SPACE:                    info->free_space = bml3_diskimage_freespace; break;
		case IMGTOOLINFO_PTR_READ_FILE:                     info->read_file = bml3_diskimage_readfile; break;
		case IMGTOOLINFO_PTR_WRITE_FILE:                    info->write_file = bml3_diskimage_writefile; break;
		case IMGTOOLINFO_PTR_DELETE_FILE:                   info->delete_file = bml3_diskimage_deletefile; break;
		case IMGTOOLINFO_PTR_SUGGEST_TRANSFER:              info->suggest_transfer = bml3_diskimage_suggesttransfer; break;
		case IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE:            info->writefile_optguide = bml3_writefile_optionguide; break;
		case IMGTOOLINFO_PTR_FLOPPY_FORMAT:                 info->p = (void *) floppyoptions_default; break;
	}
}
