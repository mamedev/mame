// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*
    Handlers for concept floppy images

    Disk images are in MESS format.

    Raphael Nabet, 2003
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "imgtool.h"

struct UINT16xE
{
	UINT8 bytes[2];
};

/*
    get_UINT16xE

    Read a 16-bit word, whether it is little-endian or big-endian

    little_endian (I): non-zero if word is little-endian, zero if word is
        big-endian
    word (I): pointer to word to read

    Returns value of word in native format
*/
static inline UINT16 get_UINT16xE(int little_endian, UINT16xE word)
{
	return little_endian ? (word.bytes[0] | (word.bytes[1] << 8)) : ((word.bytes[0] << 8) | word.bytes[1]);
}

#if 0
/*
    set_UINT16xE

    Write a 16-bit word, whether it is little-endian or big-endian

    little_endian (I): non-zero if word is little-endian, zero if word is
        big-endian
    word (O): pointer to word to write
    data (I): value to write in word, in native format
*/
static inline void set_UINT16xE(int little_endian, UINT16xE *word, UINT16 data)
{
	if (little_endian)
	{
		word->bytes[0] = data & 0xff;
		word->bytes[1] = (data >> 8) & 0xff;
	}
	else
	{
		word->bytes[0] = (data >> 8) & 0xff;
		word->bytes[1] = data & 0xff;
	}
}
#endif

/*
    Disk structure:

    Track 0 Sector 0 & 1: bootstrap loader
    Track 0 Sector 2 through 5: disk directory
    Remaining sectors are used for data.
*/

/*
    device directory record (Disk sector 2-5)
*/

struct concept_vol_hdr_entry
{
	UINT16xE    first_block;
	UINT16xE    next_block;
	UINT16xE    ftype;

	unsigned char   volname[8];
	UINT16xE    last_block;
	UINT16xE    num_files;
	UINT16xE    last_boot;
	UINT16xE    last_access;
	char        mem_flipped;
	char        disk_flipped;
	UINT16xE    unused;
};

struct concept_file_dir_entry
{
	UINT16xE    first_block;
	UINT16xE    next_block;
	UINT16xE    ftype;

	unsigned char   filename[16];
	UINT16xE    last_byte;
	UINT16xE    last_access;
};

struct concept_dev_dir
{
	concept_vol_hdr_entry vol_hdr;
	concept_file_dir_entry file_dir[77];
	char unused[20];
};

/*
    concept disk image descriptor
*/
struct concept_image
{
	imgtool_stream *file_handle;        /* imgtool file handle */
	concept_dev_dir dev_dir;    /* cached copy of device directory */
};

/*
    concept catalog iterator, used when imgtool reads the catalog
*/
struct concept_iterator
{
	concept_image *image;
	int index;                          /* current index */
};


static imgtoolerr_t concept_image_init(imgtool_image *img, imgtool_stream *f);
static void concept_image_exit(imgtool_image *img);
static void concept_image_info(imgtool_image *img, char *string, size_t len);
static imgtoolerr_t concept_image_beginenum(imgtool_directory *enumeration, const char *path);
static imgtoolerr_t concept_image_nextenum(imgtool_directory *enumeration, imgtool_dirent *ent);
static void concept_image_closeenum(imgtool_directory *enumeration);
static imgtoolerr_t concept_image_freespace(imgtool_partition *partition, UINT64 *size);
static imgtoolerr_t concept_image_readfile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *destf);
#if 0
static imgtoolerr_t concept_image_writefile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *sourcef, option_resolution *writeoptions);
static imgtoolerr_t concept_image_deletefile(imgtool_partition *partition, const char *filename);
static imgtoolerr_t concept_image_create(const imgtool_module *mod, imgtool_stream *f, option_resolution *createoptions);
#endif

void concept_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES:             info->i = sizeof(concept_image); break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:             info->i = sizeof(concept_iterator); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "concept"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "Concept floppy disk image"); break;
		case IMGTOOLINFO_STR_FILE_EXTENSIONS:               strcpy(info->s = imgtool_temp_str(), "img"); break;
		case IMGTOOLINFO_STR_EOLN:                          strcpy(info->s = imgtool_temp_str(), "\r"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_OPEN:                          info->open = concept_image_init; break;
		case IMGTOOLINFO_PTR_CLOSE:                         info->close = concept_image_exit; break;
		case IMGTOOLINFO_PTR_INFO:                          info->info = concept_image_info; break;
		case IMGTOOLINFO_PTR_BEGIN_ENUM:                    info->begin_enum = concept_image_beginenum; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                     info->next_enum = concept_image_nextenum; break;
		case IMGTOOLINFO_PTR_CLOSE_ENUM:                    info->close_enum = concept_image_closeenum; break;
		case IMGTOOLINFO_PTR_FREE_SPACE:                    info->free_space = concept_image_freespace; break;
		case IMGTOOLINFO_PTR_READ_FILE:                     info->read_file = concept_image_readfile; break;
		case IMGTOOLINFO_PTR_WRITE_FILE:                    /* info->write_file = concept_image_writefile */; break;
		case IMGTOOLINFO_PTR_DELETE_FILE:                   /* info->delete_file = concept_image_deletefile */; break;
		case IMGTOOLINFO_PTR_CREATE:                        /* info->create = concept_image_create */; break;
	}
}

/*
    read_physical_record

    Read one 512-byte physical record from a disk image

    file_handle: imgtool file handle
    secnum: physical record address
    dest: pointer to destination buffer

    Return non-zero on error
*/
static int read_physical_record(imgtool_stream *file_handle, int secnum, void *dest)
{
	int reply;

	/* seek to sector */
	reply = stream_seek(file_handle, secnum*512, SEEK_SET);
	if (reply)
		return 1;
	/* read it */
	reply = stream_read(file_handle, dest, 512);
	if (reply != 512)
		return 1;

	return 0;
}

#ifdef UNUSED_FUNCTION
/*
    write_physical_record

    Write one 512-byte physical record to a disk image

    file_handle: imgtool file handle
    secnum: logical sector address
    src: pointer to source buffer

    Return non-zero on error
*/
static int write_physical_record(imgtool_stream *file_handle, int secnum, const void *src)
{
	int reply;

	/* seek to sector */
	reply = stream_seek(file_handle, secnum*512, SEEK_SET);
	if (reply)
		return 1;
	/* read it */
	reply = stream_write(file_handle, src, 512);
	if (reply != 512)
		return 1;

	return 0;
}
#endif

/*
    Search for a file name on a concept_image

    image (I): image reference
    filename (I): name of the file to search
    entry_index (O): index of file in disk catalog

    Return non-zero on error
*/
static int get_catalog_entry(concept_image *image, const unsigned char *filename, int *entry_index)
{
	int filename_len = filename[0];
	int i;

	if (filename_len > 15)
		/* file name is bad */
		return 1;

	for (i = 0; i < 77; i++)
	{
		if (!memcmp(filename, image->dev_dir.file_dir[i].filename, filename_len+1))
		{
			/* file found */
			*entry_index = i;
			return 0;
		}
	}

	/* file not found */
	return 1;
}

/*
    Open a file as a concept_image.
*/
static imgtoolerr_t concept_image_init(imgtool_image *img, imgtool_stream *f)
{
	concept_image *image = (concept_image *) imgtool_image_extra_bytes(img);
	int reply;
	int i;
	unsigned totphysrecs;

	image->file_handle = f;

	/* read device directory */
	for (i=0; i<4; i++)
	{
		reply = read_physical_record(f, i+2, ((char *) & image->dev_dir)+i*512);
		if (reply)
			return IMGTOOLERR_READERROR;
	}

	/* do primitive checks */
	totphysrecs = get_UINT16xE(image->dev_dir.vol_hdr.disk_flipped, image->dev_dir.vol_hdr.last_block)
					- get_UINT16xE(image->dev_dir.vol_hdr.disk_flipped, image->dev_dir.vol_hdr.first_block);

	if ((get_UINT16xE(image->dev_dir.vol_hdr.disk_flipped, image->dev_dir.vol_hdr.first_block) != 0)
		|| (get_UINT16xE(image->dev_dir.vol_hdr.disk_flipped, image->dev_dir.vol_hdr.next_block) != 6)
		|| (totphysrecs < 6) /*|| (stream_size(f) != totphysrecs*512)*/
		|| (image->dev_dir.vol_hdr.volname[0] > 7))
	{
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	return IMGTOOLERR_SUCCESS;
}

/*
    close a concept_image
*/
static void concept_image_exit(imgtool_image *img)
{
	/*concept_image *image = (concept_image *) imgtool_image_extra_bytes(img);*/
}

/*
    get basic information on a concept_image

    Currently returns the volume name
*/
static void concept_image_info(imgtool_image *img, char *string, size_t len)
{
	concept_image *image = (concept_image *) imgtool_image_extra_bytes(img);
	char vol_name[8];

	memcpy(vol_name, image->dev_dir.vol_hdr.volname + 1, image->dev_dir.vol_hdr.volname[0]);
	vol_name[image->dev_dir.vol_hdr.volname[0]] = 0;

	snprintf(string, len, "%s", vol_name);
}

/*
    Open the disk catalog for enumeration
*/
static imgtoolerr_t concept_image_beginenum(imgtool_directory *enumeration, const char *path)
{
	concept_iterator *iter;

	iter = (concept_iterator *) imgtool_directory_extrabytes(enumeration);
	iter->image = (concept_image *) imgtool_image_extra_bytes(imgtool_directory_image(enumeration));
	iter->index = 0;
	return IMGTOOLERR_SUCCESS;
}

/*
    Enumerate disk catalog next entry
*/
static imgtoolerr_t concept_image_nextenum(imgtool_directory *enumeration, imgtool_dirent *ent)
{
	concept_iterator *iter = (concept_iterator *) imgtool_directory_extrabytes(enumeration);


	ent->corrupt = 0;
	ent->eof = 0;

	if ((iter->image->dev_dir.file_dir[iter->index].filename[0] == 0) || (iter->index > 77))
	{
		ent->eof = 1;
	}
	else if (iter->image->dev_dir.file_dir[iter->index].filename[0] > 15)
	{
		ent->corrupt = 1;
	}
	else
	{
		int len = iter->image->dev_dir.file_dir[iter->index].filename[0];
		const char *type;

		if (len > ARRAY_LENGTH(ent->filename))
			len = ARRAY_LENGTH(ent->filename);
		memcpy(ent->filename, iter->image->dev_dir.file_dir[iter->index].filename + 1, len);
		ent->filename[len] = 0;

		/* parse flags */
		switch (get_UINT16xE(iter->image->dev_dir.vol_hdr.disk_flipped, iter->image->dev_dir.file_dir[iter->index].ftype) & 0xf)
		{
		case 0:
		case 8:
			type = "DIRHDR";
			break;
		case 2:
			type = "CODE";
			break;
		case 3:
			type = "TEXT";
			break;
		case 5:
			type = "DATA";
			break;
		default:
			type = "???";
			break;
		}
		snprintf(ent->attr, ARRAY_LENGTH(ent->attr), "%s", type);

		/* len in physrecs */
		ent->filesize = get_UINT16xE(iter->image->dev_dir.vol_hdr.disk_flipped, iter->image->dev_dir.file_dir[iter->index].next_block)
							- get_UINT16xE(iter->image->dev_dir.vol_hdr.disk_flipped, iter->image->dev_dir.file_dir[iter->index].first_block);

		iter->index++;
	}

	return (imgtoolerr_t)0;
}

/*
    Free enumerator
*/
static void concept_image_closeenum(imgtool_directory *enumeration)
{
}

/*
    Compute free space on disk image
*/
static imgtoolerr_t concept_image_freespace(imgtool_partition *partition, UINT64 *size)
{
	imgtool_image *img = imgtool_partition_image(partition);
	concept_image *image = (concept_image*) img;
	int free_blocks;
	int i;

	/* first get number of data blocks */
	free_blocks = get_UINT16xE(image->dev_dir.vol_hdr.disk_flipped, image->dev_dir.vol_hdr.last_block)
					- get_UINT16xE(image->dev_dir.vol_hdr.disk_flipped, image->dev_dir.vol_hdr.next_block);

	/* next substract length of each file */
	for (i=0; (image->dev_dir.file_dir[i].filename[0] != 0) && (i <= 77); i++)
	{
		free_blocks -= get_UINT16xE(image->dev_dir.vol_hdr.disk_flipped, image->dev_dir.file_dir[i].next_block)
						- get_UINT16xE(image->dev_dir.vol_hdr.disk_flipped, image->dev_dir.file_dir[i].first_block);
	}

	*size = free_blocks;

	return IMGTOOLERR_SUCCESS;
}

/*
    Extract a file from a concept_image.
*/
static imgtoolerr_t concept_image_readfile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *destf)
{
	imgtool_image *img = imgtool_partition_image(partition);
	concept_image *image = (concept_image *) imgtool_image_extra_bytes(img);
	size_t filename_len = strlen(filename);
	unsigned char concept_fname[16];
	int catalog_index;
	int i;
	UINT8 buf[512];

	if (filename_len > 15)
		return IMGTOOLERR_BADFILENAME;

	concept_fname[0] = filename_len;
	memcpy(concept_fname+1, filename, filename_len);

	if (get_catalog_entry(image, concept_fname, &catalog_index))
		return IMGTOOLERR_FILENOTFOUND;

	for (i = get_UINT16xE(image->dev_dir.vol_hdr.disk_flipped, image->dev_dir.file_dir[catalog_index].first_block);
			i < get_UINT16xE(image->dev_dir.vol_hdr.disk_flipped, image->dev_dir.file_dir[catalog_index].next_block);
			i++)
	{
		if (read_physical_record(image->file_handle, i, buf))
			return IMGTOOLERR_READERROR;

		if (stream_write(destf, buf, 512) != 512)
			return IMGTOOLERR_WRITEERROR;
	}

	return (imgtoolerr_t)0;
}

#if 0
/*
    Add a file to a concept_image.
*/
static imgtoolerr_t concept_image_writefile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *sourcef, option_resolution *writeoptions)
{
	/* ... */

	return 0;
}

/*
    Delete a file from a concept_image.
*/
static imgtoolerr_t concept_image_deletefile(imgtool_partition *partition, const char *filename)
{
	/* ... */

	return 0;
}

/*
    Create a blank concept_image.
*/
static imgtoolerr_t concept_image_create(const imgtool_module *mod, imgtool_stream *f, option_resolution *createoptions)
{
	/* ... */

	return 0;
}
#endif
