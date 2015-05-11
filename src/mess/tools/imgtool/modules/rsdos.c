// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    rsdos.c

    CoCo RS-DOS disk images

****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "imgtool.h"
#include "formats/coco_dsk.h"
#include "iflopimg.h"

/* this structure mirrors the structure of an RS-DOS directory entry on disk */
struct rsdos_dirent
{
	char fname[8];
	char fext[3];
	char ftype;
	char asciiflag;
	unsigned char first_granule;
	unsigned char lastsectorbytes_msb;
	unsigned char lastsectorbytes_lsb;
	unsigned char unused[16];
};

struct rsdos_direnum
{
	int index;
	int eof;
};

#define RSDOS_OPTIONS_FTYPE     'T'
#define RSDOS_OPTIONS_ASCII     'M'



/*********************************************************************
    Imgtool module code
*********************************************************************/

#define MAX_DIRENTS     ((18-2)*(256/32))
static floperr_t get_rsdos_dirent(imgtool_image *f, int index_loc, struct rsdos_dirent *ent)
{
	return floppy_read_sector(imgtool_floppy(f), 0, 17, 3, index_loc * 32, (void *) ent, sizeof(*ent));
}



static floperr_t put_rsdos_dirent(imgtool_image *f, int index_loc, const struct rsdos_dirent *ent)
{
	if (index_loc >= MAX_DIRENTS)
		return (floperr_t)IMGTOOLERR_FILENOTFOUND;
	return floppy_write_sector(imgtool_floppy(f), 0, 17, 3, index_loc * 32, (void *) ent, sizeof(*ent), 0); /* TODO: pass ddam argument from imgtool */
}



/* fnamebuf must have at least 13 bytes */
static void get_dirent_fname(char *fnamebuf, const struct rsdos_dirent *ent)
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



static imgtoolerr_t lookup_rsdos_file(imgtool_image *f, const char *fname, struct rsdos_dirent *ent, int *position)
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
			ferr = get_rsdos_dirent(f, i++, ent);
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
	UINT16 tracks;
	UINT16 granules;

	tracks = floppy_get_tracks_per_disk(imgtool_floppy(img));
	granules = (tracks - 1) * 2;
	return (granules > 255) ? 255 : (UINT8) granules;
}

#define MAX_GRANULEMAP_SIZE 256

/* granule_map must be an array of MAX_GRANULEMAP_SIZE bytes */
static floperr_t get_granule_map(imgtool_image *img, UINT8 *granule_map, UINT8 *granule_count)
{
	UINT8 count;

	count = get_granule_count(img);
	if (granule_count)
		*granule_count = count;

	return floppy_read_sector(imgtool_floppy(img), 0, 17, 2, 0, granule_map, count);
}



static floperr_t put_granule_map(imgtool_image *img, const UINT8 *granule_map, UINT8 granule_count)
{
	return floppy_write_sector(imgtool_floppy(img), 0, 17, 2, 0, granule_map, granule_count, 0);    /* TODO: pass ddam argument from imgtool */
}




static imgtoolerr_t transfer_granule(imgtool_image *img, UINT8 granule, int length, imgtool_stream *f, imgtoolerr_t (*proc)(imgtool_image *, int, int, int, int, size_t, imgtool_stream *))
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	UINT8 track, sector;

	track = granule / 2;
	if (track >= 17)
		track++;

	sector = (granule % 2) ? 10 : 1;

	if (length > 0)
		err = proc(img, 0, track, sector, 0, length, f);
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



static imgtoolerr_t process_rsdos_file(struct rsdos_dirent *ent, imgtool_image *img, imgtool_stream *destf, size_t *size)
{
	floperr_t ferr;
	size_t s, lastgransize;
	UINT8 granule_count;
	unsigned char i = 0, granule;
	UINT8 usedmap[MAX_GRANULEMAP_SIZE]; /* Used to detect infinite loops */
	UINT8 granule_map[MAX_GRANULEMAP_SIZE];

	ferr = get_granule_map(img, granule_map, &granule_count);
	if (ferr)
		return imgtool_floppy_error(ferr);

	memset(usedmap, 0, granule_count);

	lastgransize = ent->lastsectorbytes_lsb + (((int) ent->lastsectorbytes_msb) << 8);
	s = 0;
	granule = ent->first_granule;

	while(!usedmap[granule] && ((i = granule_map[granule]) < granule_count))
	{
		usedmap[granule] = 1;
		if (destf)
			transfer_from_granule(img, granule, 9*256, destf);

		/* i is the next granule */
		s += (256 * 9);
		granule = i;
	}

	if ((i < 0xc0) || (i > 0xc9))
		return IMGTOOLERR_CORRUPTIMAGE;

	if (lastgransize)
		i--;
	lastgransize += (256 * (i - 0xc0));

	if (destf)
		transfer_from_granule(img, granule, lastgransize, destf);

	if (size)
		*size = s + lastgransize;
	return IMGTOOLERR_SUCCESS;
}



/* create a new directory entry with a specified name */
static imgtoolerr_t prepare_dirent(struct rsdos_dirent *ent, const char *fname)
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

	/* We had better be an 8.3 filename */
	if (((fname_end - fname) > 8) || (fname_ext_len > 3))
		return IMGTOOLERR_BADFILENAME;

	memcpy(ent->fname, fname, fname_end - fname);
	memcpy(ent->fext, fname_ext, fname_ext_len);

	/* For now, all files are type 2 binary files */
	ent->ftype = 2;
	ent->asciiflag = 0;
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t rsdos_diskimage_nextenum(imgtool_directory *enumeration, imgtool_dirent *ent)
{
	floperr_t ferr;
	imgtoolerr_t err;
	size_t filesize;
	struct rsdos_direnum *rsenum;
	struct rsdos_dirent rsent;
	char fname[13];
	imgtool_image *image;

	image = imgtool_directory_image(enumeration);
	rsenum = (struct rsdos_direnum *) imgtool_directory_extrabytes(enumeration);

	/* Did we hit the end of file before? */
	if (rsenum->eof)
		goto eof;

	do
	{
		if (rsenum->index >= MAX_DIRENTS)
			goto eof;

		ferr = get_rsdos_dirent(image, rsenum->index++, &rsent);
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
		err = process_rsdos_file(&rsent, image, NULL, &filesize);
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



static imgtoolerr_t rsdos_diskimage_freespace(imgtool_partition *partition, UINT64 *size)
{
	floperr_t ferr;
	UINT8 i;
	size_t s = 0;
	UINT8 granule_count;
	UINT8 granule_map[MAX_GRANULEMAP_SIZE];
	imgtool_image *image = imgtool_partition_image(partition);

	ferr = get_granule_map(image, granule_map, &granule_count);
	if (ferr)
		return imgtool_floppy_error(ferr);

	for (i = 0; i < granule_count; i++)
	{
		if (granule_map[i] == 0xff)
			s += (9 * 256);
	}
	*size = s;
	return (imgtoolerr_t)FLOPPY_ERROR_SUCCESS;
}



static imgtoolerr_t delete_entry(imgtool_image *img, struct rsdos_dirent *ent, int pos)
{
	floperr_t ferr;
	unsigned char g, i;
	UINT8 granule_count;
	UINT8 granule_map[MAX_GRANULEMAP_SIZE];

	/* Write a NUL in the filename, marking it deleted */
	ent->fname[0] = 0;
	ferr = put_rsdos_dirent(img, pos, ent);
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



static imgtoolerr_t rsdos_diskimage_readfile(imgtool_partition *partition, const char *fname, const char *fork, imgtool_stream *destf)
{
	imgtoolerr_t err;
	struct rsdos_dirent ent;
	size_t size;
	imgtool_image *img = imgtool_partition_image(partition);

	err = lookup_rsdos_file(img, fname, &ent, NULL);
	if (err)
		return err;

	err = process_rsdos_file(&ent, img, destf, &size);
	if (err)
		return err;

	if (size == (size_t) -1)
		return IMGTOOLERR_CORRUPTIMAGE;

	return (imgtoolerr_t)0;
}



static imgtoolerr_t rsdos_diskimage_writefile(imgtool_partition *partition, const char *fname, const char *fork, imgtool_stream *sourcef, option_resolution *writeoptions)
{
	floperr_t ferr;
	imgtoolerr_t err;
	imgtool_image *img = imgtool_partition_image(partition);
	struct rsdos_dirent ent, ent2;
	size_t i;
	UINT64 sz;
	UINT64 freespace = 0;
	unsigned char g;
	unsigned char *gptr;
	UINT8 granule_count;
	UINT8 granule_map[MAX_GRANULEMAP_SIZE];

	/* can we write to this image? */
	if (floppy_is_read_only(imgtool_floppy(img)))
		return IMGTOOLERR_READONLY;

	err = rsdos_diskimage_freespace(partition, &freespace);
	if (err)
		return err;

	/* is there enough space? */
	sz = stream_size(sourcef);
	if (sz > freespace)
		return IMGTOOLERR_NOSPACE;

	/* setup our directory entry */
	err = prepare_dirent(&ent, fname);
	if (err)
		return err;

	ent.ftype = option_resolution_lookup_int(writeoptions, RSDOS_OPTIONS_FTYPE);
	ent.asciiflag = ((UINT8) option_resolution_lookup_int(writeoptions, RSDOS_OPTIONS_ASCII)) - 1;
	ent.lastsectorbytes_lsb = sz % 256;
	ent.lastsectorbytes_msb = (((sz % 256) == 0) && (sz > 0)) ? 1 : 0;
	gptr = &ent.first_granule;

	ferr = get_granule_map(img, granule_map, &granule_count);
	if (ferr)
		return imgtool_floppy_error(ferr);

	g = 0x00;

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


		i = MIN(sz, (9*256));
		err = transfer_to_granule(img, g, i, sourcef);
		if (err)
			return err;

		sz -= i;

		/* Go to next granule */
		g++;
	}
	while(sz > 0);

	/* Now that we are done with the file, we need to specify the final entry
	 * in the file allocation table
	 */
	*gptr = 0xc0 + ((i + 255) / 256);

	/* Now we need to find an empty directory entry */
	i = -1;
	do
	{
		ferr = get_rsdos_dirent(img, ++i, &ent2);
		if (ferr)
			return imgtool_floppy_error(ferr);
	}
	while((ent2.fname[0] != '\0') && strcmp(ent.fname, ent2.fname) && (ent2.fname[0] != -1));

	/* delete file if it already exists */
	if (ent2.fname[0] && (ent2.fname[0] != -1))
	{
		err = delete_entry(img, &ent2, i);
		if (err)
			return err;
	}

	ferr = put_rsdos_dirent(img, i, &ent);
	if (ferr)
		return imgtool_floppy_error(ferr);

	/* write the granule map back out */
	ferr = put_granule_map(img, granule_map, granule_count);
	if (ferr)
		return imgtool_floppy_error(ferr);

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t rsdos_diskimage_deletefile(imgtool_partition *partition, const char *fname)
{
	imgtoolerr_t err;
	imgtool_image *image = imgtool_partition_image(partition);
	int pos;
	struct rsdos_dirent ent;

	err = lookup_rsdos_file(image, fname, &ent, &pos);
	if (err)
		return err;

	return delete_entry(image, &ent, pos);
}



static imgtoolerr_t rsdos_diskimage_suggesttransfer(imgtool_partition *partition, const char *fname, imgtool_transfer_suggestion *suggestions, size_t suggestions_length)
{
	imgtoolerr_t err;
	imgtool_image *image = imgtool_partition_image(partition);
	struct rsdos_dirent ent;
	int pos;

	if (fname)
	{
		err = lookup_rsdos_file(image, fname, &ent, &pos);
		if (err)
			return err;

		if (ent.asciiflag == (char) 0xFF)
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
			suggestions[1].filter = filter_cocobas_getinfo;
		}
	}
	else
	{
		suggestions[0].viability = SUGGESTION_RECOMMENDED;
		suggestions[0].filter = NULL;
		suggestions[1].viability = SUGGESTION_POSSIBLE;
		suggestions[1].filter = filter_eoln_getinfo;
		suggestions[2].viability = SUGGESTION_POSSIBLE;
		suggestions[2].filter = filter_cocobas_getinfo;
	}

	return IMGTOOLERR_SUCCESS;
}



/*********************************************************************
    Imgtool module declaration
*********************************************************************/

static OPTION_GUIDE_START( coco_rsdos_writefile_optionguide )
	OPTION_ENUM_START(  RSDOS_OPTIONS_FTYPE, "ftype", "File type" )
		OPTION_ENUM(    0,      "basic",        "Basic" )
		OPTION_ENUM(    1,      "data",         "Data" )
		OPTION_ENUM(    2,      "binary",       "Binary" )
		OPTION_ENUM(    3,      "assembler",    "Assembler Source" )
	OPTION_ENUM_END
	OPTION_ENUM_START(  RSDOS_OPTIONS_ASCII, "ascii", "Ascii flag" )
		OPTION_ENUM(    0,      "ascii",        "Ascii" )
		OPTION_ENUM(    1,      "binary",       "Binary" )
	OPTION_ENUM_END
OPTION_GUIDE_END



void rsdos_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_PREFER_UCASE:                  info->i = 1; break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:             info->i = sizeof(struct rsdos_direnum); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "rsdos"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "RS-DOS format"); break;
		case IMGTOOLINFO_STR_FILE:                          strcpy(info->s = imgtool_temp_str(), __FILE__); break;
		case IMGTOOLINFO_STR_EOLN:                          strcpy(info->s = imgtool_temp_str(), "\r"); break;
		case IMGTOOLINFO_STR_WRITEFILE_OPTSPEC:             strcpy(info->s = imgtool_temp_str(), "T0-[2]-3;M0-[1]"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_MAKE_CLASS:                    info->make_class = imgtool_floppy_make_class; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                     info->next_enum = rsdos_diskimage_nextenum; break;
		case IMGTOOLINFO_PTR_FREE_SPACE:                    info->free_space = rsdos_diskimage_freespace; break;
		case IMGTOOLINFO_PTR_READ_FILE:                     info->read_file = rsdos_diskimage_readfile; break;
		case IMGTOOLINFO_PTR_WRITE_FILE:                    info->write_file = rsdos_diskimage_writefile; break;
		case IMGTOOLINFO_PTR_DELETE_FILE:                   info->delete_file = rsdos_diskimage_deletefile; break;
		case IMGTOOLINFO_PTR_SUGGEST_TRANSFER:              info->suggest_transfer = rsdos_diskimage_suggesttransfer; break;
		case IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE:            info->writefile_optguide = coco_rsdos_writefile_optionguide; break;
		case IMGTOOLINFO_PTR_FLOPPY_FORMAT:                 info->p = (void *) floppyoptions_coco; break;
	}
}
