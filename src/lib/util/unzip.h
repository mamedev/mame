/***************************************************************************

    unzip.h

    ZIP file management.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UNZIP_H__
#define __UNZIP_H__

#include "osdcore.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ZIP_DECOMPRESS_BUFSIZE	16384

/* Error types */
enum _zip_error
{
	ZIPERR_NONE = 0,
	ZIPERR_OUT_OF_MEMORY,
	ZIPERR_FILE_ERROR,
	ZIPERR_BAD_SIGNATURE,
	ZIPERR_DECOMPRESS_ERROR,
	ZIPERR_FILE_TRUNCATED,
	ZIPERR_FILE_CORRUPT,
	ZIPERR_UNSUPPORTED,
	ZIPERR_BUFFER_TOO_SMALL
};
typedef enum _zip_error zip_error;



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* contains extracted file header information */
typedef struct _zip_file_header zip_file_header;
struct _zip_file_header
{
	UINT32			signature;				/* central file header signature */
	UINT16			version_created;		/* version made by */
	UINT16			version_needed;			/* version needed to extract */
	UINT16			bit_flag;				/* general purpose bit flag */
	UINT16			compression;			/* compression method */
	UINT16			file_time;				/* last mod file time */
	UINT16			file_date;				/* last mod file date */
	UINT32			crc;					/* crc-32 */
	UINT32			compressed_length;		/* compressed size */
	UINT32			uncompressed_length;	/* uncompressed size */
	UINT16			filename_length;		/* filename length */
	UINT16			extra_field_length;		/* extra field length */
	UINT16			file_comment_length;	/* file comment length */
	UINT16			start_disk_number;		/* disk number start */
	UINT16			internal_attributes;	/* internal file attributes */
	UINT32			external_attributes;	/* external file attributes */
	UINT32			local_header_offset;	/* relative offset of local header */
	const char *	filename;				/* filename */

	UINT8 *			raw;					/* pointer to the raw data */
	UINT32			rawlength;				/* length of the raw data */
	UINT8			saved;					/* saved byte from after filename */
};


/* contains extracted end of central directory information */
typedef struct _zip_ecd zip_ecd;
struct _zip_ecd
{
	UINT32			signature;				/* end of central dir signature */
	UINT16			disk_number;			/* number of this disk */
	UINT16			cd_start_disk_number;	/* number of the disk with the start of the central directory */
	UINT16			cd_disk_entries;		/* total number of entries in the central directory on this disk */
	UINT16			cd_total_entries;		/* total number of entries in the central directory */
	UINT32			cd_size;				/* size of the central directory */
	UINT32			cd_start_disk_offset;	/* offset of start of central directory with respect to the starting disk number */
	UINT16			comment_length;			/* .ZIP file comment length */
	const char *	comment;				/* .ZIP file comment */

	UINT8 *			raw;					/* pointer to the raw data */
	UINT32			rawlength;				/* length of the raw data */
};


/* describes an open ZIP file */
typedef struct _zip_file zip_file;
struct _zip_file
{
	const char *	filename;				/* copy of ZIP filename (for caching) */
	osd_file *		file;					/* OSD file handle */
	UINT64			length;					/* length of zip file */

	zip_ecd			ecd;					/* end of central directory */

	UINT8 *			cd;						/* central directory raw data */
	UINT32			cd_pos;					/* position in central directory */
	zip_file_header	header;					/* current file header */

	UINT8			buffer[ZIP_DECOMPRESS_BUFSIZE];	/* buffer for decompression */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- ZIP file access ----- */

/* open a ZIP file and parse its central directory */
zip_error zip_file_open(const char *filename, zip_file **zip);

/* close a ZIP file (may actually be left open due to caching) */
void zip_file_close(zip_file *zip);

/* clear out all open ZIP files from the cache */
void zip_file_cache_clear(void);


/* ----- contained file access ----- */

/* find the first file in the ZIP */
const zip_file_header *zip_file_first_file(zip_file *zip);

/* find the next file in the ZIP */
const zip_file_header *zip_file_next_file(zip_file *zip);

/* decompress the most recently found file in the ZIP */
zip_error zip_file_decompress(zip_file *zip, void *buffer, UINT32 length);


#endif	/* __UNZIP_H__ */
