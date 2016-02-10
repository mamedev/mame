// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*
    Handlers for ti99 disk images

    We support the following types of image:
    * floppy disks ("DSK format") in v9t9, pc99 and old mess image format
    * hard disks ("WIN format") in MAME harddisk format (256-byte sectors only,
      i.e. no SCSI)
    Files are extracted in TIFILES format.  There is a compile-time option to
    extract text files in flat format instead, but I need to re-implement it
    properly (using filters comes to mind).

    Raphael Nabet, 2002-2004

    TODO:
    - finish and test hd support ***test sibling FDR support***
    - check allocation bitmap against corruption when an image is opened
*/

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "imgtool.h"
#include "harddisk.h"
#include "imghd.h"

/*
    Concepts shared by both disk structures:
    ----------------------------------------

    In both formats, the entire disk surface is split into physical records,
    each 256-byte-long.  For best efficiency, the sector size is usually 256
    bytes so that one sector is equivalent to a physical record (this is true
    with floppies and HFDC hard disks).  If the sector size is not 256 bytes
    (e.g. 512 bytes with SCSI hard disks, possibly 128 bytes on a TI-99
    simulator running on TI-990), sectors are grouped or split to form 256-byte
    physical records.  Physical records are numbered from 0, with the first
    starting on sector 0 track 0.  To avoid confusion with physical records in
    files, such physical records will be called "absolute physical records",
    abbreviated as "absolute physrecs" or "aphysrecs".

    Disk space is allocated in units referred to as AUs.  Each AU represents an
    integral number of 256-byte physical records.  The number of physrecs per
    AU varies depending on the disk format and size.  AUs are numbered from 0,
    with the first starting with absolute physrec 0.

    Absolute physrec 0 contains a 256-byte record with volume information,
    which is called "Volume Information Block", abbreviated as "vib".

    To keep track of which areas on the disk are allocated and which are free,
    disk managers maintain a bit map of allocated AUs (called "allocation
    bitmap", abbreviated as "abm").  Each bit in the allocation bitmap
    represents an AU: if a bit is 0, the AU is free; if a bit is 1, the AU is
    allocated (or possibly bad if the disk manager can track bad sectors).
    Where on the disk the abm is located depends on the disk structure (see
    structure-specific description for details).

    Files are implemented as a succession of 256-byte physical records.  To
    avoid confusion with absolute physical records, these physical records will
    be called "file physical records", abbreviated as "file physrecs" or
    "fphysrecs".

    Programs do normally not access file physical records directly.  They may
    call high-level file routines that enable to create either fixed-length
    logical records of any size from 1 through 255, or variable-length records
    of any size from 0 through 254: logical records are grouped by file
    managers into 256-byte physical records.  Some disk managers (HFDC and
    SCSI) allow programs to create fixed-length records larger than 255 bytes,
    too, but few programs use this possibility.  Additionally, program files
    can be created: program files do not implement any logical record, and can
    be seen as a flat byte stream, not unlike files under MSDOS, UNIX and the C
    standard library.  (Unfortunately, the API for program files lacks
    flexibility, and most programs that need to process a flat byte stream
    will use fixed-size records of 128 bytes.)

        Fixed-length records (reclen < 256) are grouped like this:
              fphysrec 0        fphysrec 1
            _______________   _______________
            |R0 |R1 |R2 |X|   |R3 |R4 |R5 |X|
            ---------------   ---------------
                        \_/               \_/
                       unused            unused
                  (size < reclen)   (size < reclen)

        Fixed-length records (reclen > 256) are grouped like this:
              fphysrec 0         fphysrec 1         fphysrec 2          fphysrec 3      ...
            _________________  _________________  _________________   _________________
            |  R0 (#0-255)  |  | R0 (#256-511) |  |R0 (#512-end)|X|   |  R1 (#0-255)  |   ...
            -----------------  -----------------  -----------------   -----------------
                                                                \_/
                                                               unused

        Variable length records are grouped like this:
                             fphysrec 0:
            byte:
                    ------------------------------
                  0 |l0 = length of record 0 data|
                    |----------------------------|
                  1 |                            |
                    |                            |
                  : |       record 0 data        |
                    |                            |
                 l0 |                            |
                    |----------------------------|
               l0+1 |l1 = length of record 1 data|
                    |----------------------------|
               l0+2 |                            |
                    |                            |
                  : |       record 1 data        |
                    |                            |
            l0+l1+1 |                            |
                    |----------------------------|
                  : :                            :
                  : :                            :
                    |----------------------------|
                  n |lm = length of record m data|
                    |----------------------------|
                n+1 |                            |
                    |                            |
                  : |       record m data        |
                    |                            |
               n+lm |                            |
                    |----------------------------|
             n+lm+1 |>ff: EOR mark               |
                    |----------------------------|
             n+lm+2 |                            |
                    |                            |
                  : |           unused           |
                  : |(we should have size < lm+1)|
                    |                            |
                255 |                            |
                    ------------------------------

                             fphysrec 1:
            byte:
                    ------------------------------
                  0 |lm+1=length of record 0 data|
                    |----------------------------|
                  1 |                            |
                    |                            |
                  : |      record m+1 data       |
                    |                            |
               lm+1 |                            |
                    |----------------------------|
                  : :                            :
                  : :                            :

            I think the EOR mark is not required if all 256 bytes of a physical
            record are used by logical records.


    All files are associated with a "file descriptor record" ("fdr") that holds
    file information (name, format, length) and points to the data AUs. The WIN
    disk structure also supports sibling FDRs, in case a file is so fragmented
    that all the data pointers cannot fit in one FDR; the DSK disk structure
    does not implement any such feature, and you may be unable to append data
    to an existing file if it is too fragmented, even though the disk is not
    full.


    DSK disk structure:
    -------------------

    The DSK disk structure is used for floppy disks, and a few RAMDISKs as
    well.  It supports 1600 AUs and 16 MBytes at most (the 16 MByte value is a
    theorical maximum, as I doubt anyone has ever created so big a DSK volume).

    The disk sector size is always 256 bytes (the only potential exceptions are
    the SCSI floppy disk units and the TI-99 simulator on TI-990, but I don't
    know anything about either).

    For some reason, the number of physrecs per AU is always a power of 2.
    Originally, AUs always were one physical record each.  However, since the
    allocation bitmap cannot support more than 1600 AUs, disks larger than
    400kbytes need to have several physical records per AU.  Note that
    relatively few disk controllers implement AUs that span multiple physical
    records (IIRC, these are Myarc's HFDC, SCSI DSR with floppy disk interface,
    and some upgraded Myarc and Corcomp DD floppy controllers).

    The allocation bitmap is located in the VIB.  It is 200 bytes long, and
    supports 1600 AUs at most.

    Directories are implemented with a "File Descriptor Index Record" (FDIR)
    for each directory.  The FDIR is array of 0 through 128 words, containing
    the aphysrec address of each fdr in the directory, sorted by ascending file
    name, terminated with a 0.  Note that, while we should be prepared to read
    images images with 128 entries, I think (not 100% sure) that we should
    write no more than 127 for compatibility with some existing disk managers.

    Originally, the DSK structure only supported one directory level (i.e. the
    root directory level).  The FDIR record for the root directory is always
    located in aphysrec 1.  Moreover, Myarc extended the DSK structure to
    support up to 3 subdirectories in the root directory (note that there is no
    support for more subdirs, or for subdirs located in subdirs).  To do so,
    they used an unused field of the VIB to hold the 10-char name of each
    directory and the aphysrec address of the associated FDIR record.

    aphysrec 0: Volume Information Block (VIB): see below
    aphysrec 1: FDIR for root directory
    Remaining AUs are used for fdr and data (and subdirectory FDIR if
    applicable).  There is one FDIR record per directory; the FDIR points to
    the FDR for each file in the directory.  The FDR (File Descriptor Record)
    contains the file information (name, format, length, pointers to data
    physrecs/AUs, etc).


    WIN disk structure:
    -------------------

    The WIN disk structure is used for hard disks.  It supports 65535 AUs and
    256 MBytes at most.

    The disk sector size is 256 bytes on HFDC, 512 bytes on SCSI.  512-byte
    sectors are split into 256-byte physical records.

    We may have from 1 through 16 physrecs per AUs.  Since we cannot have more
    than 65535 AUs, we need to have several physical records per AU in disks
    larger than 16 Mbytes.

    Contrary to the DSK disk structure, the WIN disk structure supports
    hierarchic subdirectories, with a limit of 114 subdirectories and 127 files
    per directory.  Each directory is associated with both a "Directory
    Descriptor Record" (DDR) and a "File Descriptor Index Record" (FDIR).  The
    only exception is the root directory, which uses the VIB instead of a DDR
    (the VIB includes all the important fields of the DDR).  The DDR contains
    some directory info (name, number of files and subdirectories directly
    enclosed in the directory, AU address of the FDIR of the parent directory,
    etc), the AU address of the associated FDIR, and the AU address of the DDR
    of up to 114 subdirectories, sorted by ascending directory name.  The WIN
    FDIR is similar to, yet different from, the DSK FDIR: it contains up to 127
    (vs. 128) AU address (vs. aphysrec address) of each fdr in the directory,
    sorted by ascending file name.  Additionally, it includes the AU address of
    the associated DDR.

    When a file has more than 54 data fragments, the disk manager creates
    sibling FDRs that hold additional fragments.  (Sibling FDRs are called
    offspring FDRs in most existing documentation, but I prefer "sibling FDRs"
    as "offspring FDRs" wrongly suggests a directory-like hierarchy.
    Incidentally, I cannot really figure out why they chose to duplicate the
    FDR rather than create a file extension record, but there must be a
    reason.)  Note that sibling FDRs usually fill unused physrecs in the AU
    allocated for the eldest FDR, and a new AU is allocated for new sibling
    FDRs only when the first AU is full.

    aphysrec 0: Volume Information Block (VIB): see below
    aphysrec 1-n (where n = 1+SUP(number_of_AUs/2048)): Volume bitmap
    Remaining AUs are used for ddr, fdir, fdr and data.
*/

/* Since string length is encoded with a byte, the maximum length of a string
is 255.  Since we need to add a device name (1 char minimum, typically 4 chars)
and a '.' separator, we chose a practical value of 250 (though few applications
will accept paths of such length). */
#define MAX_PATH_LEN 253
#define MAX_SAFE_PATH_LEN 250

#define MAX_DIR_LEVEL 125   /* We need to put a recursion limit to avoid endless recursion hazard */


#if 0
#pragma mark MISCELLANEOUS UTILITIES
#endif

/*
    Miscellaneous utilities that are used to handle TI data types
*/

struct UINT16BE
{
	UINT8 bytes[2];
};

struct UINT16LE
{
	UINT8 bytes[2];
};

static inline UINT16 get_UINT16BE(UINT16BE word)
{
	return (word.bytes[0] << 8) | word.bytes[1];
}

static inline void set_UINT16BE(UINT16BE *word, UINT16 data)
{
	word->bytes[0] = (data >> 8) & 0xff;
	word->bytes[1] = data & 0xff;
}

static inline UINT16 get_UINT16LE(UINT16LE word)
{
	return word.bytes[0] | (word.bytes[1] << 8);
}

static inline void set_UINT16LE(UINT16LE *word, UINT16 data)
{
	word->bytes[0] = data & 0xff;
	word->bytes[1] = (data >> 8) & 0xff;
}

/*
    Convert a C string to a 10-character file name (padded with spaces if necessary)

    dst (O): destination 10-character array
    src (I): source C string
*/
static void str_to_fname(char dst[10], const char *src)
{
	int i;


	i = 0;

	/* copy 10 characters at most */
	if (src)
		while ((i<10) && (src[i]!='\0'))
		{
			dst[i] = src[i];
			i++;
		}

	/* pad with spaces */
	while (i<10)
	{
		dst[i] = ' ';
		i++;
	}
}

/*
    Convert a 10-character file name to a C string (removing trailing spaces if necessary)

    dst (O): destination C string
    src (I): source 10-character array
    n (I): length of dst buffer (string may be truncated if less than 11)
*/
static void fname_to_str(char *dst, const char src[10], int n)
{
	int i;
	int last_nonspace;


	/* copy 10 characters at most */
	if (--n > 10)
		n = 10;

	/* copy filename */
	i = 0;
	last_nonspace = -1;

	while (i<n)
	{
		dst[i] = src[i];
		if (src[i] != ' ')
			last_nonspace = i;
		i++;
	}

	/* terminate with '\0' */
	dst[last_nonspace+1] = '\0';
}

/*
    Check a 10-character file name.

    filename (I): 10-character array with file name

    Return non-zero if the file name is invalid.
*/
static int check_fname(const char filename[10])
{
	int i;
	int space_found_flag;


	/* check and copy file name */
	space_found_flag = FALSE;
	for (i=0; i<10; i++)
	{
		switch (filename[i])
		{
		case '.':
		case '\0':
			/* illegal characters */
			return 1;
		case ' ':
			/* illegal in a file name, but file names shorter than 10
			characters are padded with spaces */
			space_found_flag = TRUE;
			break;
		default:
			/* all other characters are legal (though non-ASCII characters,
			control characters and even lower-case characters are not
			recommended) */
			if (space_found_flag)
				return 1;
			break;
		}
	}

	return 0;
}

/*
    Check a file path.

    fpath (I): C string with file path

    Return non-zero if the file path is invalid.
*/
static int check_fpath(const char *fpath)
{
	const char *element_start, *element_end;
	int element_len;


	/* first check that the path is not too long and that there is no space
	character */
	if ((strlen(fpath) > MAX_PATH_LEN) || strchr(fpath, ' '))
		return 1;

	/* check that each path element has an acceptable length */
	element_start = fpath;
	do
	{
		/* find next path element */
		element_end = strchr(element_start, '.');
		element_len = element_end ? (element_end - element_start) : strlen(element_start);
		if ((element_len > 10) || (element_len == 0))
			return 1;

		/* iterate */
		if (element_end)
			element_start = element_end+1;
		else
			element_start = NULL;
	}
	while (element_start);

	return 0;
}

#if 0
#pragma mark -
#pragma mark LEVEL 1 DISK ROUTINES
#endif

/*
    Level 1 deals with accessing the disk hardware (in our case, the raw disk
    image).

    Higher level routines will only know the disk as a succession of
    256-byte-long physical records addressed by a linear address.
*/

/*
    Disk geometry
*/
struct ti99_geometry
{
	int secspertrack;
	int cylinders;
	int heads;
};

/*
    Physical sector address
*/
struct ti99_sector_address
{
	int sector;
	int cylinder;
	int side;
};

/*
    Time stamp (used in fdr, and WIN VIB/DDR)
*/
struct ti99_date_time
{
	UINT8 time_MSB, time_LSB;/* 0-4: hour, 5-10: minutes, 11-15: seconds/2 */
	UINT8 date_MSB, date_LSB;/* 0-6: year, 7-10: month, 11-15: day */
};

/*
    Subdirectory descriptor (HFDC only)

    The HFDC supports up to 3 subdirectories.
*/
struct dsk_subdir
{
	char name[10];          /* subdirectory name (10 characters, pad with spaces) */
	UINT16BE fdir_aphysrec; /* aphysrec address of fdir record for this subdirectory */
};

/*
    DSK VIB record

    Most fields in this record are only relevant to level 2 routines, but level
    1 routines need the disk geometry information extracted from the VIB.
*/
struct dsk_vib
{
	char name[10];          /* disk volume name (10 characters, pad with spaces) */
	UINT16BE totphysrecs;   /* total number of physrecs on disk (usually 360, */
								/* 720, 640, 1280 or 1440).  (TI originally */
								/* said it should be the total number of AUs, */
								/* but, in practice, DSRs that supports disks */
								/* over 400kbytes (e.g. HFDC) write the total */
								/* number of physrecs.) */
	UINT8 secspertrack;     /* sectors per track (usually 9 (FM SD), */
								/* 16 or 18 (MFM DD), or 36 (MFM HD) */
	UINT8 id[3];            /* 'DSK' */
	UINT8 protection;       /* 'P' if disk is protected, ' ' otherwise. */
	UINT8 cylinders;        /* tracks per side (usually 40) */
	UINT8 heads;            /* sides (1 or 2) */
	UINT8 density;          /* density: 1 (FM SD), 2 (MFM DD), or 3 (MFM HD) */
	dsk_subdir subdir[3];   /* descriptor for up to 3 subdirectories (HFDC only) */
								/* reserved by TI */
	UINT8 abm[200];         /* allocation bitmap: there is one bit per AU. */
								/* A binary 1 in a bit position indicates that */
								/* the allocation unit associated with that */
								/* bit has been allocated. */
								/* Bits corresponding to system reserved AUs, */
								/* non-existant AUs, and bad AUs, are set to one, too. */
								/* (AU 0 is associated to LSBit of byte 0, */
								/* AU 7 to MSBit of byte 0, AU 8 to LSBit */
								/* of byte 1, etc.) */
};

enum ti99_img_format
{
	if_mess,
	if_v9t9,
	if_pc99_fm,
	if_pc99_mfm,
	if_harddisk
};

/*
    level-1 disk image descriptor
*/
struct ti99_lvl1_imgref
{
	ti99_img_format img_format; /* tells the image format */
	imgtool_stream *file_handle;        /* imgtool file handle */
	struct mess_hard_disk_file harddisk_handle;     /* MAME harddisk handle (harddisk format) */
	ti99_geometry geometry;     /* geometry */
	unsigned pc99_track_len;        /* unformatted track length (pc99 format) */
	UINT32 *pc99_data_offset_array; /* offset for each sector (pc99 format) */
};

/*
    calculate CRC for data address marks or sector data

    crc (I/O): current CRC value to be updated (initialize to 0xffff before
        calling this function for the first time)
    value: new byte of data to update the CRC with
*/
static void calc_crc(UINT16 *crc, UINT8 value)
{
	UINT8 l, h;

	l = value ^ (*crc >> 8);
	*crc = (*crc & 0xff) | (l << 8);
	l >>= 4;
	l ^= (*crc >> 8);
	*crc <<= 8;
	*crc = (*crc & 0xff00) | l;
	l = (l << 4) | (l >> 4);
	h = l;
	l = (l << 2) | (l >> 6);
	l &= 0x1f;
	*crc = *crc ^ (l << 8);
	l = h & 0xf0;
	*crc = *crc ^ (l << 8);
	l = (h << 1) | (h >> 7);
	l &= 0xe0;
	*crc = *crc ^ l;
}

/*
    Parse a PC99 disk image

    file_handle (I/O): imgtool file handle
    fm_format (I): if true, the image is in FM format, otherwise it is in MFM
        format
    pass (I): 0 for first pass, 1 for second pass
    vib (O): buffer where the vib should be stored (first pass)
    geometry (O): disk image geometry (second pass)
    data_offset_array (O): array of data offset to generate (second pass)
    out_track_len (O): track length is returned there

    Return imgtool error code
*/
#define MAX_TRACK_LEN 6872
#define DATA_OFFSET_NONE 0xffffffff
static int parse_pc99_image(imgtool_stream *file_handle, int fm_format, int pass, dsk_vib *vib, const ti99_geometry *geometry, UINT32 *data_offset_array, unsigned *out_track_len)
{
	int track_len, num_tracks;  /* length of a track in bytes, and number of tracks */
	int phys_track;
	int expected_cylinder, expected_head;
	int track_start_pos, track_pos;
	UINT8 c;
	UINT8 cylinder, head, sector;
	int seclen;
	UINT8 crc1, crc2;
	UINT16 crc;
	long data_offset;
	UINT8 track_buf[MAX_TRACK_LEN];
	int i;

	if (fm_format)
		track_len = 3253;
	else
		track_len = 6872;

	if (out_track_len)
		*out_track_len = track_len;

	if (stream_size(file_handle) % track_len)
		return IMGTOOLERR_CORRUPTIMAGE;

	num_tracks = stream_size(file_handle) / track_len;
	if (num_tracks <= 0)
		return IMGTOOLERR_CORRUPTIMAGE;

	/* we only support 40-track-per-side images */
	if ((num_tracks != 40) && (num_tracks != 80))
		return IMGTOOLERR_UNIMPLEMENTED;

	if (pass == 1)
	{
		/* initialize offset map */
		for (head = 0; head < geometry->heads; head++)
			for (cylinder = 0; cylinder < geometry->cylinders; cylinder++)
				for (sector = 0; sector < geometry->secspertrack; sector++)
					data_offset_array[(head*geometry->cylinders + cylinder)*geometry->secspertrack + sector] = DATA_OFFSET_NONE;
	}
	/* rewind to start of file */
	stream_seek(file_handle, 0, SEEK_SET);

	/* pass 0 only looks for vib in track 0; pass 1 scans every track */
	for (phys_track=0; phys_track < ((pass == 1) ? num_tracks : 1); phys_track++)
	{
		if (stream_read(file_handle, track_buf, track_len) != track_len)
			return IMGTOOLERR_READERROR;

		/* we only support 40-track-per-side images */
		expected_cylinder = phys_track % 40;
		expected_head = phys_track / 40;

		track_start_pos = 0;

		while (track_start_pos < track_len)
		{
			if (fm_format)
			{
				do
				{
					c = track_buf[track_start_pos];
					track_start_pos++;
				} while ((c != 0xfe) && (track_start_pos < track_len));

				if (c != 0xfe)
					break;

				track_pos = track_start_pos;

				crc = 0xffff;
				calc_crc(& crc, c);
			}
			else
			{
				do
				{
					c = track_buf[track_start_pos];
					track_start_pos++;
				} while ((c != 0xa1) && (track_start_pos < track_len));

				if (c != 0xa1)
					break;

				track_pos = track_start_pos;

				c = track_buf[track_pos];
				track_pos++;
				if (track_pos == track_len)
					track_pos = 0;

				if (c != 0xa1)
					continue;

				c = track_buf[track_pos];
				track_pos++;
				if (track_pos == track_len)
					track_pos = 0;

				if (c != 0xa1)
					continue;

				crc = 0xffff;
				calc_crc(& crc, c);

				c = track_buf[track_pos];
				track_pos++;
				if (track_pos == track_len)
					track_pos = 0;

				if (c != 0xfe)
					continue;
			}

			c = track_buf[track_pos];
			track_pos++;
			if (track_pos == track_len)
				track_pos = 0;

			cylinder = c;
			calc_crc(& crc, c);

			c = track_buf[track_pos];
			track_pos++;
			if (track_pos == track_len)
				track_pos = 0;

			head = c;
			calc_crc(& crc, c);

			c = track_buf[track_pos];
			track_pos++;
			if (track_pos == track_len)
				track_pos = 0;

			sector = c;
			calc_crc(& crc, c);

			c = track_buf[track_pos];
			track_pos++;
			if (track_pos == track_len)
				track_pos = 0;

			seclen = 128 << c;
			calc_crc(& crc, c);

			c = track_buf[track_pos];
			track_pos++;
			if (track_pos == track_len)
				track_pos = 0;

			crc1 = c;
			calc_crc(& crc, c);

			c = track_buf[track_pos];
			track_pos++;
			if (track_pos == track_len)
				track_pos = 0;

			crc2 = c;
			calc_crc(& crc, c);

#if 0
			/* CRC seems to be completely hosed */
			if (crc)
				printf("aargh!");
#endif
			if ((seclen != 256) || (crc1 != 0xf7) || (crc2 != 0xf7)
					|| (cylinder != expected_cylinder) || (head != expected_head)
					|| ((pass == 1) && ((cylinder >= geometry->cylinders)
										|| (head >= geometry->heads)
										|| (sector >= geometry->secspertrack))))
				continue;

			c = track_buf[track_pos];
			track_pos++;
			if (track_pos == track_len)
				track_pos = 0;

			while (c == (fm_format ? 0xff : 0x4e))
			{
				c = track_buf[track_pos];
				track_pos++;
				if (track_pos == track_len)
					track_pos = 0;
			}

			while (c == 0x00)
			{
				c = track_buf[track_pos];
				track_pos++;
				if (track_pos == track_len)
					track_pos = 0;
			}

			if (fm_format)
			{
				if (c != 0xfb)
					continue;

				crc = 0xffff;
				calc_crc(& crc, c);
			}
			else
			{
				if (c != 0xa1)
					continue;

				c = track_buf[track_pos];
				track_pos++;
				if (track_pos == track_len)
					track_pos = 0;

				if (c != 0xa1)
					continue;

				c = track_buf[track_pos];
				track_pos++;
				if (track_pos == track_len)
					track_pos = 0;

				if (c != 0xa1)
					continue;

				crc = 0xffff;
				calc_crc(& crc, c);

				c = track_buf[track_pos];
				track_pos++;
				if (track_pos == track_len)
					track_pos = 0;

				if (c != 0xfb)
					continue;
			}
			data_offset = track_pos;
			for (i=0; i<seclen; i++)
			{
				c = track_buf[track_pos];
				track_pos++;
				if (track_pos == track_len)
					track_pos = 0;

				calc_crc(& crc, c);
			}

			c = track_buf[track_pos];
			track_pos++;
			if (track_pos == track_len)
				track_pos = 0;

			crc1 = c;
			calc_crc(& crc, c);

			c = track_buf[track_pos];
			track_pos++;
			if (track_pos == track_len)
				track_pos = 0;

			crc2 = c;
			calc_crc(& crc, c);

#if 0
			/* CRC seems to be completely hosed */
			if (crc)
				printf("aargh!");
#endif
			if ((crc1 != 0xf7) || (crc2 != 0xf7))
				continue;

			switch (pass)
			{
			case 0:
				if ((cylinder == 0) && (head == 0) && (sector == 0))
				{
					/* return vib */
					if ((data_offset + 256) <= track_len)
						memcpy(vib, track_buf + data_offset, 256);
					else
					{
						memcpy((UINT8 *)vib, track_buf + data_offset, track_len-data_offset);
						memcpy((UINT8 *)vib + (track_len-data_offset), track_buf, 256-(track_len-data_offset));
					}
					return 0;
				}
				break;

			case 1:
				/* set up offset map */
				if (data_offset_array[(head*geometry->cylinders + cylinder)*geometry->secspertrack + sector] != DATA_OFFSET_NONE)
					/* error: duplicate sector */
					return IMGTOOLERR_CORRUPTIMAGE;
				data_offset_array[(head*geometry->cylinders + cylinder)*geometry->secspertrack + sector] = data_offset;
				break;
			}
		}
	}

	if (pass == 0)
		return IMGTOOLERR_CORRUPTIMAGE;

	if (pass == 1)
	{
		/* check offset map */
		for (head = 0; head < geometry->heads; head++)
			for (cylinder = 0; cylinder < geometry->cylinders; cylinder++)
				for (sector = 0; sector < geometry->secspertrack; sector++)
					if (data_offset_array[(head*geometry->cylinders + cylinder)*geometry->secspertrack + sector] == DATA_OFFSET_NONE)
						/* error: missing sector */
						return IMGTOOLERR_CORRUPTIMAGE;
	}

	return 0;
}


/*
    Read the volume information block (aphysrec 0) assuming no geometry
    information.  (Called when an image is opened to figure out the
    geometry information.)

    file_handle (I/O): imgtool file handle
    img_format (I): image format (MESS, V9T9 or PC99)
    dest (O): pointer to 256-byte destination buffer

    Return imgtool error code
*/
static int read_image_vib_no_geometry(imgtool_stream *file_handle, ti99_img_format img_format, dsk_vib *dest)
{
	int reply;

	switch (img_format)
	{
	case if_mess:
	case if_v9t9:
		/* seek to sector */
		reply = stream_seek(file_handle, 0, SEEK_SET);
		if (reply)
			return IMGTOOLERR_READERROR;
		/* read it */
		reply = stream_read(file_handle, dest, 256);
		if (reply != 256)
			return IMGTOOLERR_READERROR;
		return 0;

	case if_pc99_fm:
	case if_pc99_mfm:
		return parse_pc99_image(file_handle, img_format == if_pc99_fm, 0, dest, NULL, NULL, NULL);

	case if_harddisk:
		/* not implemented, because we don't need it */
		break;
	}

	return IMGTOOLERR_UNIMPLEMENTED;
}

/*
    Open a disk image at level 1

    file_handle (I/O): imgtool file handle
    img_format (I): image format (MESS, V9T9, PC99, or MAME harddisk)
    l1_img (O): level-1 image handle
    vib (O): buffer where the vib should be stored (floppy images only)

    Return imgtool error code
*/
static imgtoolerr_t open_image_lvl1(imgtool_stream *file_handle, ti99_img_format img_format, ti99_lvl1_imgref *l1_img, dsk_vib *vib)
{
	imgtoolerr_t err;
	int reply;
	UINT16 totphysrecs;


	l1_img->img_format = img_format;
	l1_img->file_handle = file_handle;

	if (img_format == if_harddisk)
	{
		const hard_disk_info *info;

		err = imghd_open(file_handle, &l1_img->harddisk_handle);
		if (err)
			return err;

		info = imghd_get_header(&l1_img->harddisk_handle);
		l1_img->geometry.cylinders = info->cylinders;
		l1_img->geometry.heads = info->heads;
		l1_img->geometry.secspertrack = info->sectors;
		if (info->sectorbytes != 256)
		{
			imghd_close(&l1_img->harddisk_handle);
			return IMGTOOLERR_CORRUPTIMAGE; /* TODO: support 512-byte sectors */
		}

#if 0
		/* read vib */
		reply = read_absolute_physrec(l1_img, 0, vib);
		if (reply)
			return reply;
#endif
	}
	else
	{
		/* read vib */
		reply = read_image_vib_no_geometry(file_handle, img_format, vib);
		if (reply)
			return (imgtoolerr_t)reply;

		/* extract geometry information */
		totphysrecs = get_UINT16BE(vib->totphysrecs);

		l1_img->geometry.secspertrack = vib->secspertrack;
		if (l1_img->geometry.secspertrack == 0)
			/* Some images might be like this, because the original SSSD TI
			controller always assumes 9. */
			l1_img->geometry.secspertrack = 9;
		l1_img->geometry.cylinders = vib->cylinders;
		if (l1_img->geometry.cylinders == 0)
			/* Some images are like this, because the original SSSD TI
			controller always assumes 40. */
			l1_img->geometry.cylinders = 40;
		l1_img->geometry.heads = vib->heads;
		if (l1_img->geometry.heads == 0)
			/* Some images are like this, because the original SSSD TI
			controller always assumes that tracks beyond 40 are on side 2. */
			l1_img->geometry.heads = totphysrecs / (l1_img->geometry.secspertrack * l1_img->geometry.cylinders);

		/* check information */
		if ((totphysrecs != (l1_img->geometry.secspertrack * l1_img->geometry.cylinders * l1_img->geometry.heads))
				|| (totphysrecs < 2)
				|| memcmp(vib->id, "DSK", 3) || (! strchr(" P", vib->protection))
				|| (((img_format == if_mess) || (img_format == if_v9t9))
					&& (stream_size(file_handle) != totphysrecs*256U)))
			return (imgtoolerr_t)IMGTOOLERR_CORRUPTIMAGE;

		if ((img_format == if_pc99_fm) || (img_format == if_pc99_mfm))
		{
			l1_img->pc99_data_offset_array = (UINT32*)malloc(sizeof(*l1_img->pc99_data_offset_array)*totphysrecs);
			if (! l1_img->pc99_data_offset_array)
				return IMGTOOLERR_OUTOFMEMORY;
			reply = parse_pc99_image(file_handle, img_format == if_pc99_fm, 1, NULL, & l1_img->geometry, l1_img->pc99_data_offset_array, &l1_img->pc99_track_len);
			if (reply)
			{
				free(l1_img->pc99_data_offset_array);
				return (imgtoolerr_t)reply;
			}
		}
	}

	return (imgtoolerr_t)0;
}

/*
    Close a disk image at level 1

    l1_img (I/O): level-1 image handle

    Return imgtool error code
*/
static void close_image_lvl1(ti99_lvl1_imgref *l1_img)
{
	if (l1_img->img_format == if_harddisk)
	{
		imghd_close(&l1_img->harddisk_handle);
	}

	stream_close(l1_img->file_handle);

	if ((l1_img->img_format == if_pc99_fm) || (l1_img->img_format == if_pc99_mfm))
		free(l1_img->pc99_data_offset_array);
}

/*
    Convert physical sector address to offset in disk image (old MESS and V9T9
    formats only)

    l1_img (I/O): level-1 image handle
    address (I): physical sector address

    Return offset in image
*/
static inline int sector_address_to_image_offset(const ti99_lvl1_imgref *l1_img, const ti99_sector_address *address)
{
	int offset = 0;

	switch (l1_img->img_format)
	{
	case if_mess:
		/* old MESS format */
		offset = (((address->cylinder*l1_img->geometry.heads) + address->side)*l1_img->geometry.secspertrack + address->sector)*256;
		break;

	case if_v9t9:
		/* V9T9 format */
		if (address->side & 1)
			/* on side 1, tracks are stored in the reverse order */
			offset = (((address->side*l1_img->geometry.cylinders) + (l1_img->geometry.cylinders-1 - address->cylinder))*l1_img->geometry.secspertrack + address->sector)*256;
		else
			offset = (((address->side*l1_img->geometry.cylinders) + address->cylinder)*l1_img->geometry.secspertrack + address->sector)*256;
		break;

	case if_pc99_fm:
	case if_pc99_mfm:
		/* pc99 format */
	case if_harddisk:
		/* harddisk format */
		assert(1);      /* not implemented */
		break;
	}

	return offset;
}

/*
    Read one 256-byte sector from a disk image

    l1_img (I/O): level-1 image handle
    address (I): physical sector address
    dest (O): pointer to 256-byte destination buffer

    Return non-zero on error
*/
static int read_sector(ti99_lvl1_imgref *l1_img, const ti99_sector_address *address, void *dest)
{
	int reply;
	UINT32 track_len, track_offset, sector_offset;

	switch (l1_img->img_format)
	{
	case if_mess:
		/* old MESS format */
	case if_v9t9:
		/* V9T9 format */
		/* seek to sector */
		reply = stream_seek(l1_img->file_handle, sector_address_to_image_offset(l1_img, address), SEEK_SET);
		if (reply)
			return 1;
		/* read it */
		reply = stream_read(l1_img->file_handle, dest, 256);
		if (reply != 256)
			return 1;
		break;

	case if_pc99_fm:
	case if_pc99_mfm:
		/* pc99 format */
		track_len = l1_img->pc99_track_len;
		track_offset = (address->side*l1_img->geometry.cylinders + address->cylinder)*track_len;
		sector_offset = l1_img->pc99_data_offset_array[(address->side*l1_img->geometry.cylinders + address->cylinder)*l1_img->geometry.secspertrack + address->sector];

		if ((sector_offset + 256) <= track_len)
		{
			/* seek to sector */
			reply = stream_seek(l1_img->file_handle, track_offset+sector_offset, SEEK_SET);
			if (reply)
				return 1;
			/* read it */
			reply = stream_read(l1_img->file_handle, dest, 256);
			if (reply != 256)
				return 1;
		}
		else
		{
			/* seek to sector */
			reply = stream_seek(l1_img->file_handle, track_offset+sector_offset, SEEK_SET);
			if (reply)
				return 1;
			/* read first chunk (until end of track) */
			reply = stream_read(l1_img->file_handle, (UINT8 *)dest, track_len-sector_offset);
			if (reply != track_len-sector_offset)
				return 1;

			/* wrap to start of track */
			reply = stream_seek(l1_img->file_handle, track_offset, SEEK_SET);
			if (reply)
				return 1;
			/* read remnant of sector */
			reply = stream_read(l1_img->file_handle, (UINT8 *)dest + (track_len-sector_offset), 256-(track_len-sector_offset));
			if (reply != 256-(track_len-sector_offset))
				return 1;
		}
		break;

	case if_harddisk:
		/* not implemented */
		assert(1);
		/*return imghd_read(l1_img->harddisk_handle, ((address->cylinder*l1_img->geometry.heads) + address->side)*l1_img->geometry.secspertrack + address->sector, 1, dest) != 1;*/
		break;
	}

	return 0;
}

/*
    Write one 256-byte sector to a disk image

    l1_img (I/O): level-1 image handle
    address (I): physical sector address
    src (I): pointer to 256-byte source buffer

    Return non-zero on error
*/
static int write_sector(ti99_lvl1_imgref *l1_img, const ti99_sector_address *address, const void *src)
{
	int reply;
	UINT32 track_len, track_offset, sector_offset;

	switch (l1_img->img_format)
	{
	case if_mess:
		/* old MESS format */
	case if_v9t9:
		/* V9T9 format */
		/* seek to sector */
		reply = stream_seek(l1_img->file_handle, sector_address_to_image_offset(l1_img, address), SEEK_SET);
		if (reply)
			return 1;
		/* write it */
		reply = stream_write(l1_img->file_handle, src, 256);
		if (reply != 256)
			return 1;
		break;

	case if_pc99_fm:
	case if_pc99_mfm:
		/* pc99 format */
		track_len = l1_img->pc99_track_len;
		track_offset = (address->side*l1_img->geometry.cylinders + address->cylinder)*track_len;
		sector_offset = l1_img->pc99_data_offset_array[(address->side*l1_img->geometry.cylinders + address->cylinder)*l1_img->geometry.secspertrack + address->sector];

		if ((sector_offset + 256) <= track_len)
		{
			/* seek to sector */
			reply = stream_seek(l1_img->file_handle, track_offset+sector_offset, SEEK_SET);
			if (reply)
				return 1;
			/* write it */
			reply = stream_write(l1_img->file_handle, src, 256);
			if (reply != 256)
				return 1;
		}
		else
		{
			/* seek to sector */
			reply = stream_seek(l1_img->file_handle, track_offset+sector_offset, SEEK_SET);
			if (reply)
				return 1;
			/* write first chunk (until end of track) */
			reply = stream_write(l1_img->file_handle, (UINT8 *)src, track_len-sector_offset);
			if (reply != track_len-sector_offset)
				return 1;

			/* wrap to start of track */
			reply = stream_seek(l1_img->file_handle, track_offset, SEEK_SET);
			if (reply)
				return 1;
			/* write remnant of sector */
			reply = stream_write(l1_img->file_handle, (UINT8 *)src + (track_len-sector_offset), 256-(track_len-sector_offset));
			if (reply != 256-(track_len-sector_offset))
				return 1;
		}
		break;

	case if_harddisk:
		/* not implemented */
		assert(1);
		/*return imghd_write(l1_img->harddisk_handle, ((address->cylinder*l1_img->geometry.heads) + address->side)*l1_img->geometry.secspertrack + address->sector, 1, src) != 1;*/
		break;
	}

	return 0;
}

/*
    Convert physical record address to sector address (DSK format)

    aphysrec (I): absolute physrec address
    geometry (I): disk image geometry
    address (O): physical sector address
*/
static void dsk_aphysrec_to_sector_address(int aphysrec, const ti99_geometry *geometry, ti99_sector_address *address)
{
	address->sector = aphysrec % geometry->secspertrack;
	aphysrec /= geometry->secspertrack;
	address->cylinder = aphysrec % geometry->cylinders;
	address->side = aphysrec / geometry->cylinders;
	if (address->side & 1)
		/* on side 1, tracks are stored in the reverse order */
		address->cylinder = geometry->cylinders-1 - address->cylinder;
}

/*
    Convert physical record address to sector address (WIN format for HFDC)

    Note that physical address translation makes sense for HFDC, but not SCSI.

    aphysrec (I): absolute physrec address
    geometry (I): disk image geometry
    address (O): physical sector address
*/
#ifdef UNUSED_FUNCTION
static void win_aphysrec_to_sector_address(int aphysrec, const ti99_geometry *geometry, ti99_sector_address *address)
{
	address.sector = aphysrec % l1_img->geometry.secspertrack;
	aphysrec /= l1_img->geometry.secspertrack;
	address.side = aphysrec % l1_img->geometry.heads;
	address.cylinder = aphysrec / l1_img->geometry.heads;
}
#endif

/*
    Read one 256-byte physical record from a disk image

    l1_img (I/O): level-1 image handle
    aphysrec (I): absolute physrec address
    dest (O): pointer to 256-byte destination buffer

    Return non-zero on error
*/
static int read_absolute_physrec(ti99_lvl1_imgref *l1_img, unsigned aphysrec, void *dest)
{
	ti99_sector_address address;


	if (l1_img->img_format == if_harddisk)
	{
#if 0
		win_aphysrec_to_sector_address(aphysrec, & l1_img->geometry, & address);
		return read_sector(l1_img, & address, dest);
#endif

		return imghd_read(&l1_img->harddisk_handle, aphysrec, dest) != IMGTOOLERR_SUCCESS;
	}
	else
	{
		dsk_aphysrec_to_sector_address(aphysrec, & l1_img->geometry, & address);

		return read_sector(l1_img, & address, dest);
	}
}

/*
    Write one 256-byte physical record to a disk image

    l1_img (I/O): level-1 image handle
    aphysrec (I): absolute physrec address
    src (I): pointer to 256-byte source buffer

    Return non-zero on error
*/
static int write_absolute_physrec(ti99_lvl1_imgref *l1_img, unsigned aphysrec, const void *src)
{
	ti99_sector_address address;


	if (l1_img->img_format == if_harddisk)
	{
#if 0
		win_aphysrec_to_sector_address(aphysrec, & l1_img->geometry, & address);
		return write_sector(l1_img, & address, dest);
#endif

		return imghd_write(&l1_img->harddisk_handle, aphysrec, src) != IMGTOOLERR_SUCCESS;
	}
	else
	{
		dsk_aphysrec_to_sector_address(aphysrec, & l1_img->geometry, & address);

		return write_sector(l1_img, & address, src);
	}
}

#if 0
#pragma mark -
#pragma mark LEVEL 2 DISK ROUTINES
#endif

/*
    Level 2 implements files as a succession of 256-byte-long physical records.

    Level 2 implements allocation bitmap (and AU), disk catalog, etc.
*/

/*
    WIN VIB/DDR record
*/
struct win_vib_ddr
{
	char name[10];          /* disk volume name (10 characters, pad with spaces) */
	UINT16BE totAUs;        /* total number of AUs */
	UINT8 secspertrack;     /* HFDC: sectors per track (typically 32) */
							/* SCSI: reserved */
	union
	{
		struct
		{
			UINT8 id[3];    /* V1 VIB: 'WIN' */
		} vib_v1;

		struct              /* V2 VIB: extra params */
		{
			UINT8 res_AUs;  /* # AUs reserved for vib, bitmap, ddr, fdir and fdr, divided by 64 */
			UINT8 step_spd; /* HFDC: step speed (0-7) */
							/* SCSI: reserved */
			UINT8 red_w_cur;/* HFDC: reduced write current cylinder, divided by 8 */
							/* SCSI: reserved */
		} vib_v2;

		struct
		{
			UINT8 id[3];    /* DDR: 'DIR' */
		} ddr;
	} u;
	UINT16BE params;        /* bits 0-3: sectors/AU - 1 */
							/* HFDC: */
								/* bits 4-7: # heads - 1 */
								/* bit 8: V1: buffered head stepping flag */
								/*        V2: reserved */
								/* bit 9-15: write precompensation track, divided by 16 */
							/* SCSI: */
								/* bits 4-15: reserved */
	ti99_date_time creation;/* date and time of creation */
	UINT8 num_files;        /* number of files in directory */
	UINT8 num_subdirs;      /* number of subdirectories in directory */
	UINT16BE fdir_AU;       /* points to root directory fdir */
	union
	{
		struct
		{
			UINT16BE dsk1_AU;   /* HFDC: points to current dsk1 emulation image (0 if none) */
								/* SCSI: reserved */
		} vib;

		struct
		{
			UINT16BE parent_ddr_AU; /* points to parent directory DDR */
		} ddr;
	} u2;
	UINT16BE subdir_AU[114];/* points to all subdirectory DDRs */
};

/*
    AU format
*/
struct ti99_AUformat
{
	int totAUs;             /* total number of AUs */
	int physrecsperAU;      /* number of 256-byte physical records per AU */
};

/*
    DSK directory reference: 0 for root, 1 for 1st subdir, 2 for 2nd subdir, 3
    for 3rd subdir
*/
/*typedef int dir_ref;*/

/*
    catalog entry (used for in-memory catalog)
*/
struct dir_entry
{
	UINT16 dir_ptr;         /* DSK: unused */
							/* WIN: AU address of the DDR for this directory */
	char name[10];          /* name of this directory (copied from the VIB for DSK, DDR for WIN) */
};

struct file_entry
{
	UINT16 fdr_ptr;         /* DSK: aphysrec address of the FDR for this file */
							/* WIN: AU address of the FDR for this file */
	char name[10];          /* name of this file (copied from FDR) */
};

struct ti99_catalog
{
	int num_subdirs;        /* number of subdirectories */
	int num_files;          /* number of files */
	dir_entry subdirs[114]; /* description of each subdir */
	file_entry files[128];  /* description of each file */
};

/*
    level-2 disk image descriptor
*/
struct ti99_lvl2_imgref_dsk
{
	UINT16 totphysrecs;             /* total number of aphysrecs (extracted from vib record in aphysrec 0) */
	ti99_catalog catalogs[4];       /* catalog of root directory and up to 3 subdirectories */
	UINT16 fdir_aphysrec[4];        /* fdir aphysrec address for root directory
                                        and up to 3 subdirectories */
};

enum win_vib_t
{
	win_vib_v1,
	win_vib_v2
};
struct ti99_lvl2_imgref_win
{
	win_vib_t vib_version;          /* version of the vib record in aphysrec 0 (see win_vib_ddr) */
};

enum l2i_t
{
	L2I_DSK,
	L2I_WIN
};

struct ti99_lvl2_imgref
{
	ti99_lvl1_imgref l1_img;/* image format, imgtool image handle, image geometry */
	ti99_AUformat AUformat; /* AU format */
	int data_offset;        /* In order to reduce seek times when searching the
                                disk for a given file name, fdr (and ddr, and
                                fdir) records are preferentially allocated in
                                AUs n to data_offset, whereas data records are
                                preferentially allocated in AUs starting at
                                data_offset. */
							/* With the DSK disk structure, n is always 2 (if 1
							    physrec per AU) or 1 (if 2 physrecs per AU or
							    more), and data_offset is arbitrarily chosen as
							    34. */
							/* With the WIN disk structure, n depends on the
							    size of the volume bitmap, which itself depends
							    on the number of AUs on disk (we always have n
							    <= 33), and data_offset is read from the vib
							    record (except with the obsolete v1 VIB, where
							    we use a default value of 64). */
	char vol_name[10];      /* cached volume name (extracted from vib record in aphysrec 0) */

	UINT8 abm[8192];        /* allocation bitmap */

	l2i_t type;                 /* structure format */

	union
	{
		ti99_lvl2_imgref_dsk dsk;
		ti99_lvl2_imgref_win win;
	};                  /* structure-specific info */
};

/*
    file flags found in fdr (and tifiles)
*/
enum
{
	fdr99_f_program = 0x01, /* set for program files */
	fdr99_f_int     = 0x02, /* set for binary files */
	fdr99_f_wp      = 0x08, /* set if file is write-protected */
	/*fdr99_f_backup    = 0x10,*/   /* set if file has been modified since last backup */
	/*fdr99_f_dskimg    = 0x20,*/   /* set if file is a DSK image (HFDC HD only) */
	fdr99_f_var     = 0x80  /* set if file uses variable-length records*/
};

/*
    DSK FDR record
*/
struct dsk_fdr
{
	char name[10];          /* file name (10 characters, pad with spaces) */
	UINT16BE xreclen;       /* extended record len: if record len is >= 256, */
								/* reclen is set to 0 and the actual reclen is */
								/* stored here (Myarc HFDC only).  TI reserved */
								/* this  field for data chain pointer extension, */
								/* but this was never implemented. */
	UINT8 flags;            /* file status flags (see enum above) */
	UINT8 recsperphysrec;   /* logical records per physrec */
								/* ignored for variable length record files and */
								/* program files */
	UINT16BE fphysrecs;     /* file length in physrecs */
								/* Note that the HFDC defines this field as the */
								/* number of allocated physrecs in the cluster */
								/* chain (i.e. rounded on the next AU */
								/* boundary), so level-3 routines should use */
								/* the fixrecs field instead to determine the */
								/* logical length of field.  IIRC, the HFDC */
								/* implementation is regarded as a bug because */
								/* program files do not define the fixrecs field */
								/* field, so program field saved by the HFDC */
								/* DSR may be larger whan they should. */
	UINT8 eof;              /* EOF offset in last physrec for variable length */
								/* record files and program files (0->256)*/
	UINT8 reclen;           /* logical record size in bytes ([1,255] 0->256) */
								/* Maximum allowable record size for variable */
								/* length record files.  Reserved for program */
								/* files (set to 0).  Set to 0 if reclen >=256 */
								/* (HFDC only). */
	UINT16LE fixrecs;       /* file length in logical records */
								/* For variable length record files, number of */
								/* 256-byte records actually used. */
	ti99_date_time creation;/* date and time of creation (HFDC and BwG only; */
								/* reserved in TI) */
	ti99_date_time update;  /* date and time of last write to file (HFDC and */
								/* BwG only; reserved in TI) */
	UINT8 clusters[76][3];  /* data cluster table: 0 through 76 entries (3 */
								/* bytes each), one entry for each file cluster. */
								/* 12 bits: address of first AU of cluster */
								/* 12 bits: offset of last 256-byte record in cluster */
};

/*
    WIN FDR record
*/
struct win_fdr
{
	char name[10];          /* file name (10 characters, pad with spaces) */
	UINT16BE xreclen;       /* extended record len: if record len is >= 256, */
								/* reclen is set to 0 and the actual reclen is */
								/* stored here (Myarc HFDC only).  TI reserved */
								/* this  field for data chain pointer extension, */
								/* but this was never implemented. */
	UINT8 flags;            /* file status flags (see enum above) */
	UINT8 recsperphysrec;   /* logical records per physrec */
								/* ignored for variable length record files and */
								/* program files */
	UINT16BE fphysrecs_LSW; /* eldest FDR: file length in physrecs */
								/* Note that the HFDC defines this field as the */
								/* number of allocated physrecs in the cluster */
								/* chain (i.e. rounded on the next AU */
								/* boundary), so level-3 routines should use */
								/* the fixrecs field instead to determine the */
								/* logical length of field.  IIRC, the HFDC */
								/* implementation is regarded as a bug because */
								/* program files do not define the fixrecs field */
								/* field, so program field saved by the HFDC */
								/* DSR may be larger whan they should. */
							/* other sibling FDRs: index of the first file */
								/* physrec in this particular sibling FDR */
	UINT8 eof;              /* EOF offset in last physrec for variable length */
								/* record files and program files (0->256)*/
	UINT8 reclen;           /* logical record size in bytes ([1,255]) */
								/* Maximum allowable record size for variable */
								/* length record files.  Reserved for program */
								/* files (set to 0).  Set to 0 if reclen >=256 */
								/* (HFDC only). */
	UINT16LE fixrecs_LSW;   /* file length in logical records */
								/* For variable length record files, number of */
								/* 256-byte records actually used. */
	ti99_date_time creation;/* date and time of creation */
	ti99_date_time update;  /* date and time of last write to file */

	char id[2];             /* 'FI' */
	UINT16BE prevsibFDR_AU; /* address of the AU where previous sibling FDR is */
								/* (see also xinfo_LSB) */
	UINT16BE nextsibFDR_AU; /* address of the AU where next sibling FDR is */
								/* (see also xinfo_LSB) */
	UINT16BE sibFDR_AUlen;  /* total number of data AUs allocated in this particular sibling FDR */
	UINT16BE parent_FDIR_AU;/* FDIR the file is listed in */
	UINT8 xinfo_MSB;        /* extended information (MSByte) */
								/* bits 0-3: MSN of fphysrecs */
								/* bits 4-7: MSN of fixrecs */
	UINT8 xinfo_LSB;        /* extended information (LSByte) */
								/* bits 8-11: physrec offset within AU for */
									/* previous sibling FDR (see prevsibFDR_AU) */
								/* bits 12-15: physrec offset within AU for */
									/* next sibling FDR (see nextsibFDR_AU) */
	UINT16BE clusters[54][2];/* data cluster table: 0 through 54 entries (4 */
								/* bytes each), one entry for each file cluster. */
								/* 16 bits: address of first AU of cluster */
								/* 16 bits: address of last AU of cluster */
};

/*
    tifile header: stand-alone file
*/
struct tifile_header
{
	char tifiles[8];        /* always '\7TIFILES' */
	UINT16BE fphysrecs;     /* file length in physrecs */
	UINT8 flags;            /* see enum above */
	UINT8 recsperphysrec;   /* records per physrec */
	UINT8 eof;              /* current position of eof in last physrec (0->255)*/
	UINT8 reclen;           /* bytes per record ([1,255] 0->256) */
	UINT16BE fixrecs;       /* file length in records */
	UINT8 res[128-16];      /* reserved */
								/* * variant a: */
									/* 112 chars: 0xCA53 repeated 56 times */
								/* * variant b: */
									/* 4 chars: unknown */
									/* 108 chars: 0xCA53 repeated 54 times */
								/* * variant c: */
									/* 10 chars: original TI file name filed with spaces */
									/* 102 chars: spaces */
								/* * variant d: */
									/* 10 chars: original TI file name filed with spaces */
									/* 2 chars: CR+LF */
									/* 98 chars: spaces */
									/* 2 chars: CR+LF */
								/* * variant e: */
									/* 10 chars: original TI file name */
									/* 4 bytes: unknown */
									/* 4 bytes: time & date of creation */
									/* 4 bytes: time & date of last update */
									/* 90 chars: spaces */
								/* * variant f: */
									/* 6 bytes: 'MYTERM' */
									/* 4 bytes: time & date of creation */
									/* 4 bytes: time & date of last update */
									/* 2 bytes: unknown (always >0000) */
									/* 96 chars: 0xCA53 repeated 56 times */
};

/*
    level-2 file descriptor
*/
struct ti99_lvl2_fileref_dsk
{
	struct ti99_lvl2_imgref *l2_img;
	int fdr_aphysrec;
	dsk_fdr fdr;
};

struct ti99_lvl2_fileref_win
{
	struct ti99_lvl2_imgref *l2_img;
	unsigned fphysrecs;             /* copy of field in the eldest FDR */
	unsigned eldestfdr_aphysrec;    /* aphysrec address of the eldest FDR */
	unsigned curfdr_aphysrec;       /* aphysrec address of the currently open sibling FDR */
	win_fdr curfdr;                 /* buffer with currently open sibling FDR */
};

struct ti99_lvl2_fileref_tifiles
{
	imgtool_stream *file_handle;
	tifile_header hdr;
};

enum l2f_type_t
{
	L2F_DSK,
	L2F_WIN,
	L2F_TIFILES
};

struct ti99_lvl2_fileref
{
	l2f_type_t type;
	union
	{
		ti99_lvl2_fileref_dsk dsk;
		ti99_lvl2_fileref_win win;
		ti99_lvl2_fileref_tifiles tifiles;
	};
};

/*
    Compare two (possibly empty) catalog entry for qsort
*/
static int cat_file_compare_qsort(const void *p1, const void *p2)
{
	const file_entry *entry1 = (const file_entry *)p1;
	const file_entry *entry2 = (const file_entry *)p2;

	if ((entry1->fdr_ptr == 0) && (entry2->fdr_ptr == 0))
		return 0;
	else if (entry1->fdr_ptr == 0)
		return +1;
	else if (entry2->fdr_ptr == 0)
		return -1;
	else
		return memcmp(entry1->name, entry2->name, 10);
}

static int cat_dir_compare_qsort(const void *p1, const void *p2)
{
	const dir_entry *entry1 = (const dir_entry *)p1;
	const dir_entry *entry2 = (const dir_entry *)p2;

	if ((entry1->dir_ptr == 0) && (entry2->dir_ptr == 0))
		return 0;
	else if (entry1->dir_ptr == 0)
		return +1;
	else if (entry2->dir_ptr == 0)
		return -1;
	else
		return memcmp(entry1->name, entry2->name, 10);
}

/*
    Read a directory catalog from disk image

    l2_img: image reference
    aphysrec: physical record address of the FDIR
    dest: pointer to the destination buffer where the catalog should be written

    Return an error code if there was an error, 0 otherwise.
*/
static int dsk_read_catalog(struct ti99_lvl2_imgref *l2_img, int aphysrec, ti99_catalog *dest)
{
	int totphysrecs = l2_img->dsk.totphysrecs;
	UINT16BE fdir_buf[128];
	dsk_fdr fdr;
	int i;
	int reply;


	/* Read FDIR record */
	reply = read_absolute_physrec(& l2_img->l1_img, aphysrec, fdir_buf);
	if (reply)
		return IMGTOOLERR_READERROR;

	/* Copy FDIR info to catalog structure */
	for (i=0; i<128; i++)
		dest->files[i].fdr_ptr = get_UINT16BE(fdir_buf[i]);

	/* Check FDIR pointers and check and extract file names from DDRs */
	for (i=0; i<128; i++)
	{
		if (dest->files[i].fdr_ptr >= totphysrecs)
		{
			return IMGTOOLERR_CORRUPTIMAGE;
		}
		else if (dest->files[i].fdr_ptr)
		{
			reply = read_absolute_physrec(& l2_img->l1_img, dest->files[i].fdr_ptr, &fdr);
			if (reply)
				return IMGTOOLERR_READERROR;

			/* check and copy file name */
			if (check_fname(fdr.name))
				return IMGTOOLERR_CORRUPTIMAGE;
			memcpy(dest->files[i].name, fdr.name, 10);
		}
	}

	/* Check catalog */
	for (i=0; i<127; i++)
	{
		if (((! dest->files[i].fdr_ptr) && dest->files[i+1].fdr_ptr)
			|| ((dest->files[i].fdr_ptr && dest->files[i+1].fdr_ptr) && (memcmp(dest->files[i].name, dest->files[i+1].name, 10) >= 0)))
		{
			/* if the catalog is not sorted, we repair it */
			qsort(dest->files, ARRAY_LENGTH(dest->files), sizeof(dest->files[0]),
					cat_file_compare_qsort);
			break;
		}
	}

	/* Set file count */
	for (i=0; (i<128) && (dest->files[i].fdr_ptr != 0); i++)
		;
	dest->num_files = i;

	/* Set subdir count to 0 (subdirs are loaded elsewhere) */
	dest->num_subdirs = 0;

	return 0;
}

/*
    Read a directory catalog from disk image

    l2_img: image reference
    DDR_AU: AU address of the VIB/DDR
    dest: pointer to the destination buffer where the catalog should be written

    Return an error code if there was an error, 0 otherwise.
*/
static int win_read_catalog(struct ti99_lvl2_imgref *l2_img, int DDR_AU, ti99_catalog *dest)
{
	win_vib_ddr ddr_buf;
	UINT16BE fdir_buf[128];
	win_fdr fdr_buf;
	int i;
	int reply;


	/* Read DDR record */
	reply = read_absolute_physrec(& l2_img->l1_img, DDR_AU*l2_img->AUformat.physrecsperAU, &ddr_buf);
	if (reply)
		return IMGTOOLERR_READERROR;

	/* sanity checks */
	if ((ddr_buf.num_files > 127) || (ddr_buf.num_subdirs > 114) || (get_UINT16BE(ddr_buf.fdir_AU) > l2_img->AUformat.totAUs))
		return IMGTOOLERR_CORRUPTIMAGE;

	/* set file count and subdir count */
	dest->num_files = ddr_buf.num_files;
	dest->num_subdirs = ddr_buf.num_subdirs;

	/* Copy DDR info to catalog structure */
	for (i=0; i<ddr_buf.num_subdirs; i++)
		dest->subdirs[i].dir_ptr = get_UINT16BE(ddr_buf.subdir_AU[i]);

	/* Read FDIR record */
	reply = read_absolute_physrec(& l2_img->l1_img, get_UINT16BE(ddr_buf.fdir_AU)*l2_img->AUformat.physrecsperAU, fdir_buf);
	if (reply)
		return IMGTOOLERR_READERROR;

	/* Copy FDIR info to catalog structure */
	for (i=0; i<dest->num_files; i++)
		dest->files[i].fdr_ptr = get_UINT16BE(fdir_buf[i]);

	/* Check DDR pointers and check and extract file names from FDRs */
	for (i=0; i<dest->num_subdirs; i++)
	{
		if (dest->subdirs[i].dir_ptr >= l2_img->AUformat.totAUs)
		{
			return IMGTOOLERR_CORRUPTIMAGE;
		}
		else if (dest->subdirs[i].dir_ptr)
		{
			reply = read_absolute_physrec(& l2_img->l1_img, dest->subdirs[i].dir_ptr*l2_img->AUformat.physrecsperAU, &ddr_buf);
			if (reply)
				return IMGTOOLERR_READERROR;

			/* check and copy file name */
			if (check_fname(ddr_buf.name))
				return IMGTOOLERR_CORRUPTIMAGE;
			memcpy(dest->subdirs[i].name, ddr_buf.name, 10);
		}
	}

	/* Check FDIR pointers and check and extract file names from FDRs */
	for (i=0; i<dest->num_files; i++)
	{
		if (dest->files[i].fdr_ptr >= l2_img->AUformat.totAUs)
		{
			return IMGTOOLERR_CORRUPTIMAGE;
		}
		else if (dest->files[i].fdr_ptr)
		{
			reply = read_absolute_physrec(& l2_img->l1_img, dest->files[i].fdr_ptr*l2_img->AUformat.physrecsperAU, &fdr_buf);
			if (reply)
				return IMGTOOLERR_READERROR;

			/* check and copy file name */
			if (check_fname(fdr_buf.name))
				return IMGTOOLERR_CORRUPTIMAGE;
			memcpy(dest->files[i].name, fdr_buf.name, 10);
		}
	}

	/* Check catalog */

	/* Check subdir order */
	for (i=0; i<dest->num_subdirs-1; i++)
	{
		if (((! dest->subdirs[i].dir_ptr) && dest->subdirs[i+1].dir_ptr)
			|| ((dest->subdirs[i].dir_ptr && dest->subdirs[i+1].dir_ptr) && (memcmp(dest->subdirs[i].name, dest->subdirs[i+1].name, 10) >= 0)))
		{
			/* if the subdir catalog is not sorted, we repair it */
			qsort(dest->subdirs, dest->num_subdirs, sizeof(dest->subdirs[0]), cat_dir_compare_qsort);
			break;
		}
	}

	/* Fix subdir count */
	while (dest->num_subdirs && (dest->subdirs[dest->num_subdirs-1].dir_ptr == 0))
		dest->num_subdirs--;

	/* Check file order */
	for (i=0; i<dest->num_files-1; i++)
	{
		if (((! dest->files[i].fdr_ptr) && dest->files[i+1].fdr_ptr)
			|| ((dest->files[i].fdr_ptr && dest->files[i+1].fdr_ptr) && (memcmp(dest->files[i].name, dest->files[i+1].name, 10) >= 0)))
		{
			/* if the file catalog is not sorted, we repair it */
			qsort(dest->files, dest->num_files, sizeof(dest->files[0]), cat_file_compare_qsort);
			break;
		}
	}

	/* Fix file count */
	while (dest->num_files && (dest->files[dest->num_files-1].fdr_ptr == 0))
		dest->num_files--;

	return 0;
}

/*
    Search for a file path on a floppy image

    l2_img: image reference
    fpath: path of the file to search
    parent_ref_valid: set to TRUE if either the file was found or the file was
        not found but its parent dir was
    parent_ref: reference to parent dir (0 for root)
    out_is_dir: TRUE if element is a directory
    catalog_index: on output, index of file catalog entry (may be NULL)
*/
static int dsk_find_catalog_entry(struct ti99_lvl2_imgref *l2_img, const char *fpath, int *parent_ref_valid, int *parent_ref, int *out_is_dir, int *catalog_index)
{
	int i;
	const ti99_catalog *cur_catalog;
	const char *element_start, *element_end;
	int element_len;
	char element[10];
	int is_dir = FALSE;


	cur_catalog = & l2_img->dsk.catalogs[0];
	if (parent_ref_valid)
		(* parent_ref_valid) = FALSE;
	if (parent_ref)
		*parent_ref = 0;

	element_start = fpath;
	do
	{
		/* find next path element */
		element_end = strchr(element_start, '.');
		element_len = element_end ? (element_end - element_start) : strlen(element_start);
		if ((element_len > 10) || (element_len == 0))
			return IMGTOOLERR_BADFILENAME;
		/* last path element */
		if ((!element_end) && parent_ref_valid)
			(* parent_ref_valid) = TRUE;

		/* generate file name */
		memcpy(element, element_start, element_len);
		memset(element+element_len, ' ', 10-element_len);

		/* search entry in subdirectories */
		for (i = 0; i<cur_catalog->num_subdirs; i++)
		{
			if (! memcmp(element, cur_catalog->subdirs[i].name, 10))
			{
				is_dir = TRUE;
				break;
			}
		}

		/* if it failed, search entry in files */
		if (i == cur_catalog->num_subdirs)
		{
			for (i = 0; i<cur_catalog->num_files; i++)
			{
				if (! memcmp(element, cur_catalog->files[i].name, 10))
				{
					is_dir = FALSE;
					break;
				}
			}
			/* exit if not found */
			if (i == cur_catalog->num_files)
			{
				return IMGTOOLERR_FILENOTFOUND;
			}
		}

		/* iterate */
		if (element_end)
		{
			element_start = element_end+1;

			if (! is_dir)
				/* this is not a directory */
				return IMGTOOLERR_BADFILENAME;

			/* initialize cur_catalog */
			cur_catalog = & l2_img->dsk.catalogs[i+1];
			if (parent_ref)
				*parent_ref = i+1;
		}
		else
			element_start = NULL;
	}
	while (element_start);

	if (out_is_dir)
		*out_is_dir = is_dir;

	if (catalog_index)
		*catalog_index = i;

	return 0;
}

/*
    Search for a file path on a harddisk image

    l2_img: image reference
    fpath: path of the file to search
    parent_ref_valid: set to TRUE if either the file was found or the file was
        not found but its parent dir was
    parent_ddr_AU: parent DDR AU address (0 for root)
    parent_catalog: catalog of parent dir (cannot be NULL)
    out_is_dir: TRUE if element is a directory
    catalog_index: on output, index of file catalog entry (may be NULL)
*/
static int win_find_catalog_entry(struct ti99_lvl2_imgref *l2_img, const char *fpath,
									int *parent_ref_valid, int *parent_ddr_AU, ti99_catalog *parent_catalog,
									int *out_is_dir, int *catalog_index)
{
	int i;
	const char *element_start, *element_end;
	int element_len;
	char element[10];
	int is_dir = FALSE;
	int errorcode;

	if (parent_ref_valid)
		(* parent_ref_valid) = FALSE;
	if (parent_ddr_AU)
		*parent_ddr_AU = 0;

	errorcode = win_read_catalog(l2_img, 0, parent_catalog);
	if (errorcode)
		return errorcode;

	element_start = fpath;
	do
	{
		/* find next path element */
		element_end = strchr(element_start, '.');
		element_len = element_end ? (element_end - element_start) : strlen(element_start);
		if ((element_len > 10) || (element_len == 0))
			return IMGTOOLERR_BADFILENAME;
		/* last path element */
		if ((!element_end) && parent_ref_valid)
			(* parent_ref_valid) = TRUE;

		/* generate file name */
		memcpy(element, element_start, element_len);
		memset(element+element_len, ' ', 10-element_len);

		/* search entry in subdirectories */
		for (i = 0; i<parent_catalog->num_subdirs; i++)
		{
			if (! memcmp(element, parent_catalog->subdirs[i].name, 10))
			{
				is_dir = TRUE;
				break;
			}
		}

		/* if it failed, search entry in files */
		if (i == parent_catalog->num_subdirs)
		{
			for (i = 0; i<parent_catalog->num_files; i++)
			{
				if (! memcmp(element, parent_catalog->files[i].name, 10))
				{
					is_dir = FALSE;
					break;
				}
			}
			/* exit if not found */
			if (i == parent_catalog->num_files)
			{
				return IMGTOOLERR_FILENOTFOUND;
			}
		}

		/* iterate */
		if (element_end)
		{
			element_start = element_end+1;

			if (! is_dir)
				/* this is not a directory */
				return IMGTOOLERR_BADFILENAME;

			if (parent_ddr_AU)
				*parent_ddr_AU = parent_catalog->subdirs[i].dir_ptr;

			errorcode = win_read_catalog(l2_img, parent_catalog->subdirs[i].dir_ptr, parent_catalog);
			if (errorcode)
				return errorcode;
		}
		else
			element_start = NULL;
	}
	while (element_start);

	if (out_is_dir)
		*out_is_dir = is_dir;

	if (catalog_index)
		*catalog_index = i;

	return 0;
}

/*
    Allocate one AU on disk, for use as a fdr record

    l2_img: image reference
    fdr_AU: on output, address of allocated AU
*/
static int alloc_fdr_AU(struct ti99_lvl2_imgref *l2_img, unsigned *fdr_AU)
{
	int totAUs = l2_img->AUformat.totAUs;
	int i;

	for (i=0; i<totAUs; i++)
	{
		if (! (l2_img->abm[i >> 3] & (1 << (i & 7))))
		{
			*fdr_AU = i;
			l2_img->abm[i >> 3] |= 1 << (i & 7);

			return 0;
		}
	}

	return IMGTOOLERR_NOSPACE;
}

static inline int get_dsk_fdr_cluster_baseAU(struct ti99_lvl2_imgref *l2_img, dsk_fdr *fdr, int cluster_index)
{
	int reply;

	/* read base AU/physrec for this cluster */
	reply = ((fdr->clusters[cluster_index][1] & 0xf) << 8) | fdr->clusters[cluster_index][0];
	/* convert to AU address */
	if (l2_img->AUformat.physrecsperAU <= 2)
		reply /= l2_img->AUformat.physrecsperAU;

	return reply;
}

static inline int get_dsk_fdr_cluster_baseaphysrec(struct ti99_lvl2_imgref *l2_img, dsk_fdr *fdr, int cluster_index)
{
	int reply;

	/* read base AU/physrec for this cluster */
	reply = ((fdr->clusters[cluster_index][1] & 0xf) << 8) | fdr->clusters[cluster_index][0];
	/* convert to physrec address */
	if (l2_img->AUformat.physrecsperAU > 2)
		reply *= l2_img->AUformat.physrecsperAU;

	return reply;
}

static inline int get_dsk_fdr_cluster_lastfphysrec(dsk_fdr *fdr, int cluster_index)
{
	return (fdr->clusters[cluster_index][2] << 4) | (fdr->clusters[cluster_index][1] >> 4);
}

static inline void set_dsk_fdr_cluster_lastfphysrec(dsk_fdr *fdr, int cluster_index, int data)
{
	fdr->clusters[cluster_index][1] = (fdr->clusters[cluster_index][1] & 0x0f) | (data << 4);
	fdr->clusters[cluster_index][2] = data >> 4;
}

static inline void set_dsk_fdr_cluster(struct ti99_lvl2_imgref *l2_img, dsk_fdr *fdr, int cluster_index, int baseAU, int lastfphysrec)
{
	/* convert AU address to FDR value */
	if (l2_img->AUformat.physrecsperAU <= 2)
		baseAU *= l2_img->AUformat.physrecsperAU;

	/* write cluster entry */
	fdr->clusters[cluster_index][0] = baseAU;
	fdr->clusters[cluster_index][1] = ((baseAU >> 8) & 0x0f) | (lastfphysrec << 4);
	fdr->clusters[cluster_index][2] = lastfphysrec >> 4;
}

static inline unsigned get_win_fdr_fphysrecs(win_fdr *fdr)
{
	return (((unsigned) fdr->xinfo_MSB << 12) & 0xf0000) | get_UINT16BE(fdr->fphysrecs_LSW);
}

static inline void set_win_fdr_fphysrecs(win_fdr *fdr, unsigned data)
{
	fdr->xinfo_MSB = (fdr->xinfo_MSB & 0x0f) | ((data >> 12) & 0xf0);
	set_UINT16BE(&fdr->fphysrecs_LSW, data & 0xffff);
}

static inline unsigned get_win_fdr_fixrecs(win_fdr *fdr)
{
	return (((unsigned) fdr->xinfo_MSB << 16) & 0xf0000) | get_UINT16LE(fdr->fixrecs_LSW);
}

static inline void set_win_fdr_fixrecs(win_fdr *fdr, unsigned data)
{
	fdr->xinfo_MSB = (fdr->xinfo_MSB & 0xf0) | ((data >> 16) & 0x0f);
	set_UINT16LE(&fdr->fixrecs_LSW, data & 0xffff);
}

static inline unsigned get_win_fdr_prevsibFDR_aphysrec(struct ti99_lvl2_imgref *l2_img, win_fdr *fdr)
{
	unsigned prevsibFDR_AU = get_UINT16BE(fdr->prevsibFDR_AU);

	return prevsibFDR_AU
			? (prevsibFDR_AU * l2_img->AUformat.physrecsperAU + ((fdr->xinfo_LSB >> 4) & 0xf))
			: 0;
}

static inline unsigned get_win_fdr_nextsibFDR_aphysrec(struct ti99_lvl2_imgref *l2_img, win_fdr *fdr)
{
	unsigned nextsibFDR_AU = get_UINT16BE(fdr->nextsibFDR_AU);

	return nextsibFDR_AU
			? (nextsibFDR_AU * l2_img->AUformat.physrecsperAU + (fdr->xinfo_LSB & 0xf))
			: 0;
}

static inline unsigned get_win_fdr_cursibFDR_basefphysrec(win_fdr *fdr)
{
	return get_UINT16BE(fdr->prevsibFDR_AU) ? get_win_fdr_fphysrecs(fdr) : 0;
}

/*
    Advance to next sibling FDR
*/
static int win_goto_next_sibFDR(ti99_lvl2_fileref_win *win_file)
{
	if (get_UINT16BE(win_file->curfdr.nextsibFDR_AU) == 0)
		return IMGTOOLERR_UNEXPECTED;

	win_file->curfdr_aphysrec = get_win_fdr_nextsibFDR_aphysrec(win_file->l2_img, &win_file->curfdr);
	if (read_absolute_physrec(& win_file->l2_img->l1_img, win_file->curfdr_aphysrec, &win_file->curfdr))
		return IMGTOOLERR_READERROR;

	return 0;
}

/*
    Back to previous sibling FDR
*/
static int win_goto_prev_sibFDR(ti99_lvl2_fileref_win *win_file)
{
	if (get_UINT16BE(win_file->curfdr.prevsibFDR_AU) == 0)
		return IMGTOOLERR_UNEXPECTED;

	win_file->curfdr_aphysrec = get_win_fdr_prevsibFDR_aphysrec(win_file->l2_img, &win_file->curfdr);
	if (read_absolute_physrec(& win_file->l2_img->l1_img, win_file->curfdr_aphysrec, &win_file->curfdr))
		return IMGTOOLERR_READERROR;

	return 0;
}

/*
    Append a new sibling FDR at the end of the sibling FDR list, and open it.

    You must have gone to the end of the list.
*/
static int win_alloc_sibFDR(ti99_lvl2_fileref_win *win_file)
{
	unsigned oldfdr_AU, oldfdr_physrecinAU;
	unsigned newfdr_AU, newfdr_physrecinAU;
	int allocated = FALSE;
	int errorcode;
	unsigned cursibFDR_basefphysrec;

	if (get_UINT16BE(win_file->curfdr.nextsibFDR_AU))
		return IMGTOOLERR_UNEXPECTED;

	oldfdr_AU = win_file->curfdr_aphysrec / win_file->l2_img->AUformat.physrecsperAU;
	oldfdr_physrecinAU = win_file->curfdr_aphysrec % win_file->l2_img->AUformat.physrecsperAU;

	if (oldfdr_physrecinAU != (win_file->l2_img->AUformat.physrecsperAU - 1))
	{   /* current AU is not full */
		newfdr_AU = oldfdr_AU;
		newfdr_physrecinAU = oldfdr_physrecinAU + 1;
	}
	else
	{   /* current AU is full: allocate another */
		errorcode = alloc_fdr_AU(win_file->l2_img, &newfdr_AU);
		if (errorcode)
			return errorcode;
		newfdr_physrecinAU = 0;
		allocated = TRUE;
	}

	set_UINT16BE(&win_file->curfdr.nextsibFDR_AU, newfdr_AU);
	win_file->curfdr.xinfo_LSB = (win_file->curfdr.xinfo_LSB & 0xf0) | newfdr_physrecinAU;

	/* save current sibling FDR */
	if (write_absolute_physrec(& win_file->l2_img->l1_img, win_file->curfdr_aphysrec, &win_file->curfdr))
	{
		/* clear pointer */
		set_UINT16BE(&win_file->curfdr.nextsibFDR_AU, 0);
		win_file->curfdr.xinfo_LSB = win_file->curfdr.xinfo_LSB & 0xf0;
		if (allocated)
			/* free AU */
			win_file->l2_img->abm[newfdr_AU >> 3] |= 1 << (newfdr_AU & 7);
		return IMGTOOLERR_WRITEERROR;
	}

	/* now update in-memory structure to describe new sibling FDR */
	cursibFDR_basefphysrec = get_win_fdr_cursibFDR_basefphysrec(&win_file->curfdr)
								+ get_UINT16BE(win_file->curfdr.sibFDR_AUlen) * win_file->l2_img->AUformat.physrecsperAU;

	set_UINT16BE(&win_file->curfdr.nextsibFDR_AU, 0);
	set_UINT16BE(&win_file->curfdr.prevsibFDR_AU, oldfdr_AU);
	win_file->curfdr.xinfo_LSB = oldfdr_physrecinAU << 4;

	win_file->curfdr_aphysrec = newfdr_AU * win_file->l2_img->AUformat.physrecsperAU + newfdr_physrecinAU;

	set_win_fdr_fphysrecs(&win_file->curfdr, cursibFDR_basefphysrec);
	set_UINT16BE(&win_file->curfdr.sibFDR_AUlen, 0);
	memset(win_file->curfdr.clusters, 0, sizeof(win_file->curfdr.clusters));

	return 0;
}

/*
    Extend a file with nb_alloc_physrecs extra physrecs

    dsk_file: file reference
    nb_alloc_physrecs: number of physical records to allocate
*/
static int dsk_alloc_file_physrecs(ti99_lvl2_fileref_dsk *dsk_file, int nb_alloc_physrecs)
{
	int totAUs = dsk_file->l2_img->AUformat.totAUs;
	int free_physrecs;
	int fphysrecs;
	int i;
	int cluster_index;
	int last_sec, p_last_sec = 0;
	int cur_block_seclen;
	int cluster_baseAU, cluster_AUlen;
	int first_best_block_baseAU = 0, first_best_block_seclen;
	int second_best_block_baseAU = 0, second_best_block_seclen;
	int search_start;

	/* compute free space */
	free_physrecs = 0;
	for (i=0; i<totAUs; i++)
	{
		if (! (dsk_file->l2_img->abm[i >> 3] & (1 << (i & 7))))
			free_physrecs += dsk_file->l2_img->AUformat.physrecsperAU;
	}

	/* check we have enough free space */
	if (free_physrecs < nb_alloc_physrecs)
		return IMGTOOLERR_NOSPACE;

	/* current number of data physrecs in file */
	fphysrecs = get_UINT16BE(dsk_file->fdr.fphysrecs);

	if (fphysrecs == 0)
	{   /* cluster array must be empty */
		cluster_index = 0;
	}
	else
	{   /* try to extend last block */
		last_sec = -1;
		for (cluster_index=0; cluster_index<76; cluster_index++)
		{
			p_last_sec = last_sec;
			last_sec = get_dsk_fdr_cluster_lastfphysrec(&dsk_file->fdr, cluster_index);
			if (last_sec >= (fphysrecs-1))
				break;
		}
		if (cluster_index == 76)
			/* that sucks */
			return IMGTOOLERR_CORRUPTIMAGE;

		if (last_sec > (fphysrecs-1))
		{   /* some extra space has already been allocated */
			cur_block_seclen = last_sec - (fphysrecs-1);
			if (cur_block_seclen > nb_alloc_physrecs)
				cur_block_seclen = nb_alloc_physrecs;

			fphysrecs += cur_block_seclen;
			set_UINT16BE(& dsk_file->fdr.fphysrecs, fphysrecs);
			nb_alloc_physrecs -= cur_block_seclen;
			if (! nb_alloc_physrecs)
				return 0;   /* done */
		}

		/* round up to next AU boundary */
		last_sec = last_sec + dsk_file->l2_img->AUformat.physrecsperAU - (last_sec % dsk_file->l2_img->AUformat.physrecsperAU) - 1;

		if (last_sec > (fphysrecs-1))
		{   /* some extra space has already been allocated */
			cur_block_seclen = last_sec - (fphysrecs-1);
			if (cur_block_seclen > nb_alloc_physrecs)
				cur_block_seclen = nb_alloc_physrecs;

			fphysrecs += cur_block_seclen;
			set_UINT16BE(& dsk_file->fdr.fphysrecs, fphysrecs);
			set_dsk_fdr_cluster_lastfphysrec(&dsk_file->fdr, cluster_index, fphysrecs-1);
			nb_alloc_physrecs -= cur_block_seclen;
			if (! nb_alloc_physrecs)
				return 0;   /* done */
		}

		/* read base AU address for this cluster */
		cluster_baseAU = get_dsk_fdr_cluster_baseAU(dsk_file->l2_img, &dsk_file->fdr, cluster_index);
		/* point past cluster end */
		cluster_baseAU += (last_sec-p_last_sec/*+file->l2_img->AUformat.physrecsperAU-1*/) / dsk_file->l2_img->AUformat.physrecsperAU;
		/* count free physrecs after last block */
		cur_block_seclen = 0;
		for (i=cluster_baseAU; (! (dsk_file->l2_img->abm[i >> 3] & (1 << (i & 7)))) && (cur_block_seclen < nb_alloc_physrecs) && (i < totAUs); i++)
			cur_block_seclen += dsk_file->l2_img->AUformat.physrecsperAU;
		if (cur_block_seclen)
		{   /* extend last block */
			if (cur_block_seclen > nb_alloc_physrecs)
				cur_block_seclen = nb_alloc_physrecs;

			fphysrecs += cur_block_seclen;
			set_UINT16BE(& dsk_file->fdr.fphysrecs, fphysrecs);
			last_sec += cur_block_seclen;
			nb_alloc_physrecs -= cur_block_seclen;
			set_dsk_fdr_cluster_lastfphysrec(&dsk_file->fdr, cluster_index, last_sec);
			cluster_AUlen = (cur_block_seclen + dsk_file->l2_img->AUformat.physrecsperAU - 1) / dsk_file->l2_img->AUformat.physrecsperAU;
			for (i=0; i<cluster_AUlen; i++)
				dsk_file->l2_img->abm[(i+cluster_baseAU) >> 3] |= 1 << ((i+cluster_baseAU) & 7);
			if (! nb_alloc_physrecs)
				return 0;   /* done */
		}
		cluster_index++;
		if (cluster_index == 76)
			/* that sucks */
			return IMGTOOLERR_NOSPACE;
	}

	search_start = dsk_file->l2_img->data_offset;   /* initially, search for free space only in data space */
	while (nb_alloc_physrecs)
	{
		/* find smallest data block at least nb_alloc_physrecs in length, and largest data block less than nb_alloc_physrecs in length */
		first_best_block_seclen = INT_MAX;
		second_best_block_seclen = 0;
		for (i=search_start; i<totAUs; i++)
		{
			if (! (dsk_file->l2_img->abm[i >> 3] & (1 << (i & 7))))
			{   /* found one free block */
				/* compute its length */
				cluster_baseAU = i;
				cur_block_seclen = 0;
				while ((i<totAUs) && (! (dsk_file->l2_img->abm[i >> 3] & (1 << (i & 7)))))
				{
					cur_block_seclen += dsk_file->l2_img->AUformat.physrecsperAU;
					i++;
				}
				/* compare to previous best and second-best blocks */
				if ((cur_block_seclen < first_best_block_seclen) && (cur_block_seclen >= nb_alloc_physrecs))
				{
					first_best_block_baseAU = cluster_baseAU;
					first_best_block_seclen = cur_block_seclen;
					if (cur_block_seclen == nb_alloc_physrecs)
						/* no need to search further */
						break;
				}
				else if ((cur_block_seclen > second_best_block_seclen) && (cur_block_seclen < nb_alloc_physrecs))
				{
					second_best_block_baseAU = cluster_baseAU;
					second_best_block_seclen = cur_block_seclen;
				}
			}
		}

		if (first_best_block_seclen != INT_MAX)
		{   /* found one contiguous block which can hold it all */
			fphysrecs += nb_alloc_physrecs;
			set_UINT16BE(& dsk_file->fdr.fphysrecs, fphysrecs);

			set_dsk_fdr_cluster(dsk_file->l2_img, &dsk_file->fdr, cluster_index, first_best_block_baseAU, fphysrecs-1);
			cluster_AUlen = (nb_alloc_physrecs + dsk_file->l2_img->AUformat.physrecsperAU - 1) / dsk_file->l2_img->AUformat.physrecsperAU;
			for (i=0; i<cluster_AUlen; i++)
				dsk_file->l2_img->abm[(i+first_best_block_baseAU) >> 3] |= 1 << ((i+first_best_block_baseAU) & 7);

			nb_alloc_physrecs = 0;
		}
		else if (second_best_block_seclen != 0)
		{   /* jeez, we need to fragment it.  We use the largest smaller block to limit fragmentation. */
			fphysrecs += second_best_block_seclen;
			set_UINT16BE(& dsk_file->fdr.fphysrecs, fphysrecs);

			set_dsk_fdr_cluster(dsk_file->l2_img, &dsk_file->fdr, cluster_index, second_best_block_baseAU, fphysrecs-1);
			cluster_AUlen = (second_best_block_seclen + dsk_file->l2_img->AUformat.physrecsperAU - 1) / dsk_file->l2_img->AUformat.physrecsperAU;
			for (i=0; i<cluster_AUlen; i++)
				dsk_file->l2_img->abm[(i+second_best_block_baseAU) >> 3] |= 1 << ((i+second_best_block_baseAU) & 7);

			nb_alloc_physrecs -= second_best_block_seclen;

			cluster_index++;
			if (cluster_index == 76)
				/* that sucks */
				return IMGTOOLERR_NOSPACE;
		}
		else if (search_start != 0)
		{   /* we did not find any free physrec in the data section of the disk */
			search_start = 0;   /* time to fall back to fdr space */
		}
		else
			return IMGTOOLERR_NOSPACE;  /* This should never happen, as we pre-check that there is enough free space */
	}

	return 0;
}

/*
    Extend a file with nb_alloc_physrecs extra physrecs

    win_file: file reference
    nb_alloc_physrecs: number of physical records to allocate
*/
static int win_alloc_file_physrecs(ti99_lvl2_fileref_win *win_file, int nb_alloc_physrecs)
{
	int totAUs = win_file->l2_img->AUformat.totAUs;
	int free_physrecs;
	int fphysrecs;
	int i;
	int cluster_index;
	int num_fphysrec;
	int cur_block_seclen;
	int cluster_baseAU, cluster_AUlen;
	int first_best_block_baseAU = 0, first_best_block_seclen;
	int second_best_block_baseAU = 0, second_best_block_seclen;
	int search_start;
	int errorcode;

	/* compute free space */
	free_physrecs = 0;
	for (i=0; i<totAUs; i++)
	{
		if (! (win_file->l2_img->abm[i >> 3] & (1 << (i & 7))))
			free_physrecs += win_file->l2_img->AUformat.physrecsperAU;
	}

	/* check we have enough free space */
	if (free_physrecs < nb_alloc_physrecs)
		return IMGTOOLERR_NOSPACE;

	/* move to last sibling non-empty FDR */
	while ((get_UINT16BE(win_file->curfdr.nextsibFDR_AU) != 0) &&
				(get_UINT16BE(win_file->curfdr.clusters[53][0]) != 0))
	{
		errorcode = win_goto_next_sibFDR(win_file);
		if (errorcode)
			return errorcode;
	}
	if ((get_UINT16BE(win_file->curfdr.clusters[0][0]) == 0) && (get_UINT16BE(win_file->curfdr.prevsibFDR_AU) != 0))
	{   /* this is annoying: we have found a sibling FDR filled with 0s: rewind
        to last non-empty sibling if applicable */
		errorcode = win_goto_prev_sibFDR(win_file);
		if (errorcode)
			return errorcode;
	}

	/* current number of data physrecs in file */
	fphysrecs = win_file->fphysrecs;

	/* current number of allocated physrecs */
	num_fphysrec = get_win_fdr_cursibFDR_basefphysrec(&win_file->curfdr)
						+ get_UINT16BE(win_file->curfdr.sibFDR_AUlen) * win_file->l2_img->AUformat.physrecsperAU;

	if (num_fphysrec > fphysrecs)
	{   /* some extra space has already been allocated */
		cur_block_seclen = num_fphysrec - fphysrecs;
		if (cur_block_seclen > nb_alloc_physrecs)
			cur_block_seclen = nb_alloc_physrecs;

		fphysrecs += cur_block_seclen;
		win_file->fphysrecs = fphysrecs;
		/* TODO: update eldest FDR fphysrecs field */
		/*set_win_fdr_fphysrecs(& win_file->rootfdr.fphysrecs, fphysrecs);*/
		nb_alloc_physrecs -= cur_block_seclen;
		if (! nb_alloc_physrecs)
			return 0;   /* done */
	}

	/* find last non-empty cluster */
	for (cluster_index=0; (cluster_index<54) && (get_UINT16BE(win_file->curfdr.clusters[cluster_index][0]) != 0); cluster_index++)
		;
	/* if we are dealing with an empty file, we will have (cluster_index == 0)... */
	if (cluster_index != 0)
	{
		cluster_index--;
		/* try to extend last cluster */
		/* point past cluster end */
		cluster_baseAU = get_UINT16BE(win_file->curfdr.clusters[cluster_index][1]) + 1;
		/* count free physrecs after last block */
		cur_block_seclen = 0;
		for (i=cluster_baseAU; (! (win_file->l2_img->abm[i >> 3] & (1 << (i & 7)))) && (cur_block_seclen < nb_alloc_physrecs) && (i < totAUs); i++)
			cur_block_seclen += win_file->l2_img->AUformat.physrecsperAU;
		if (cur_block_seclen)
		{   /* extend last block */
			if (cur_block_seclen > nb_alloc_physrecs)
				cur_block_seclen = nb_alloc_physrecs;

			fphysrecs += cur_block_seclen;
			cluster_AUlen = (cur_block_seclen + win_file->l2_img->AUformat.physrecsperAU - 1) / win_file->l2_img->AUformat.physrecsperAU;
			win_file->fphysrecs = fphysrecs;
			/* TODO: update eldest FDR fphysrecs field */
			/*set_win_fdr_fphysrecs(& win_file->rootfdr.fphysrecs, fphysrecs);*/
			set_UINT16BE(&win_file->curfdr.sibFDR_AUlen,
							get_UINT16BE(win_file->curfdr.sibFDR_AUlen)+cluster_AUlen);
			set_UINT16BE(&win_file->curfdr.clusters[cluster_index][1],
							get_UINT16BE(win_file->curfdr.clusters[cluster_index][1])+cluster_AUlen);
			for (i=0; i<cluster_AUlen; i++)
				win_file->l2_img->abm[(i+cluster_baseAU) >> 3] |= 1 << ((i+cluster_baseAU) & 7);
			nb_alloc_physrecs -= cur_block_seclen;
			if (! nb_alloc_physrecs)
				return 0;   /* done */
		}
		/* now point to first free entry in cluster table */
		cluster_index++;
	}

	search_start = win_file->l2_img->data_offset;   /* initially, search for free space only in data space */
	while (nb_alloc_physrecs)
	{
		/* find smallest data block at least nb_alloc_physrecs in length, and largest data block less than nb_alloc_physrecs in length */
		first_best_block_seclen = INT_MAX;
		second_best_block_seclen = 0;
		for (i=search_start; i<totAUs; i++)
		{
			if (! (win_file->l2_img->abm[i >> 3] & (1 << (i & 7))))
			{   /* found one free block */
				/* compute its length */
				cluster_baseAU = i;
				cur_block_seclen = 0;
				while ((i<totAUs) && (! (win_file->l2_img->abm[i >> 3] & (1 << (i & 7)))))
				{
					cur_block_seclen += win_file->l2_img->AUformat.physrecsperAU;
					i++;
				}
				/* compare to previous best and second-best blocks */
				if ((cur_block_seclen < first_best_block_seclen) && (cur_block_seclen >= nb_alloc_physrecs))
				{
					first_best_block_baseAU = cluster_baseAU;
					first_best_block_seclen = cur_block_seclen;
					if (cur_block_seclen == nb_alloc_physrecs)
						/* no need to search further */
						break;
				}
				else if ((cur_block_seclen > second_best_block_seclen) && (cur_block_seclen < nb_alloc_physrecs))
				{
					second_best_block_baseAU = cluster_baseAU;
					second_best_block_seclen = cur_block_seclen;
				}
			}
		}

		if ((first_best_block_seclen != INT_MAX) || (second_best_block_seclen != 0))
		{
			if (cluster_index == 54)
			{
				/* end of cluster list: go to next sibling FDR */
				if (write_absolute_physrec(& win_file->l2_img->l1_img, win_file->curfdr_aphysrec, &win_file->curfdr))
					return IMGTOOLERR_WRITEERROR;
				if (get_UINT16BE(win_file->curfdr.nextsibFDR_AU) != 0)
				{   /* read next sibling FDR */
					errorcode = win_goto_next_sibFDR(win_file);
					if (errorcode)
						return errorcode;
				}
				else
				{   /* allocate new sibling FDR */
					errorcode = win_alloc_sibFDR(win_file);
					if (errorcode)
						return errorcode;
				}
				cluster_index = 0;
			}
		}

		if (first_best_block_seclen != INT_MAX)
		{   /* found one contiguous block which can hold it all */
			fphysrecs += nb_alloc_physrecs;
			cluster_AUlen = (nb_alloc_physrecs + win_file->l2_img->AUformat.physrecsperAU - 1) / win_file->l2_img->AUformat.physrecsperAU;
			win_file->fphysrecs = fphysrecs;
			/* TODO: update eldest FDR fphysrecs field */
			/*set_win_fdr_fphysrecs(& win_file->rootfdr.fphysrecs, fphysrecs);*/
			set_UINT16BE(&win_file->curfdr.sibFDR_AUlen,
								get_UINT16BE(win_file->curfdr.sibFDR_AUlen)+cluster_AUlen);
			set_UINT16BE(&win_file->curfdr.clusters[cluster_index][0], first_best_block_baseAU);
			set_UINT16BE(&win_file->curfdr.clusters[cluster_index][1],
								first_best_block_baseAU+cluster_AUlen-1);

			for (i=0; i<cluster_AUlen; i++)
				win_file->l2_img->abm[(i+first_best_block_baseAU) >> 3] |= 1 << ((i+first_best_block_baseAU) & 7);

			nb_alloc_physrecs = 0;
		}
		else if (second_best_block_seclen != 0)
		{   /* jeez, we need to fragment it.  We use the largest smaller block to limit fragmentation. */
			fphysrecs += second_best_block_seclen;
			cluster_AUlen = (second_best_block_seclen + win_file->l2_img->AUformat.physrecsperAU - 1) / win_file->l2_img->AUformat.physrecsperAU;
			win_file->fphysrecs = fphysrecs;
			/* TODO: update eldest FDR fphysrecs field */
			/*set_win_fdr_fphysrecs(& win_file->rootfdr.fphysrecs, fphysrecs);*/
			set_UINT16BE(&win_file->curfdr.sibFDR_AUlen,
								get_UINT16BE(win_file->curfdr.sibFDR_AUlen)+cluster_AUlen);
			set_UINT16BE(&win_file->curfdr.clusters[cluster_index][0], second_best_block_baseAU);
			set_UINT16BE(&win_file->curfdr.clusters[cluster_index][1],
								second_best_block_baseAU+cluster_AUlen-1);

			for (i=0; i<cluster_AUlen; i++)
				win_file->l2_img->abm[(i+second_best_block_baseAU) >> 3] |= 1 << ((i+second_best_block_baseAU) & 7);

			nb_alloc_physrecs -= second_best_block_seclen;

			/* now point to first free entry in cluster table */
			cluster_index++;
		}
		else if (search_start != 0)
		{   /* we did not find any free physrec in the data section of the disk */
			search_start = 0;   /* time to fall back to fdr space */
		}
		else
			return IMGTOOLERR_NOSPACE;  /* This should never happen, as we pre-check that there is enough free space */
	}

	return 0;
}

/*
    Allocate a new (empty) file
*/
static int new_file_lvl2_dsk(struct ti99_lvl2_imgref *l2_img, int parent_ref, char filename[10], struct ti99_lvl2_fileref *l2_file)
{
	ti99_catalog *catalog = &l2_img->dsk.catalogs[parent_ref];
	unsigned fdr_AU, fdr_aphysrec;
	int catalog_index, i;
	int reply = 0;
	int errorcode;


	if (catalog->num_files >= 127)
		/* if num_files == 128, catalog is full */
		/* if num_files == 127, catalog is not full, but we don't want to write
		a 128th entry for compatibility with some existing DSRs that detect the
		end of the FDIR array with a 0 entry (and do not check that the index
		has not reached 128) */
		return IMGTOOLERR_NOSPACE;

	/* find insertion point in catalog */
	for (i=0; (i < catalog->num_files) && ((reply = memcmp(catalog->files[i].name, filename, 10)) < 0); i++)
		;

	if ((i<catalog->num_files) && (reply == 0))
		/* file already exists */
		return IMGTOOLERR_BADFILENAME;
	else
	{
		/* otherwise insert new entry */
		catalog_index = i;
		errorcode = alloc_fdr_AU(l2_img, &fdr_AU);
		if (errorcode)
			return errorcode;
		fdr_aphysrec = fdr_AU * l2_img->AUformat.physrecsperAU;

		/* shift catalog entries until the insertion point */
		for (i=catalog->num_files; i>catalog_index; i--)
			catalog->files[i] = catalog->files[i-1];

		/* write new catalog entry */
		catalog->files[catalog_index].fdr_ptr = fdr_aphysrec;
		memcpy(catalog->files[catalog_index].name, filename, 10);

		/* update catalog len */
		catalog->num_files++;
	}

	/* set up file handle */
	l2_file->type = L2F_DSK;
	l2_file->dsk.l2_img = l2_img;
	l2_file->dsk.fdr_aphysrec = fdr_aphysrec;
	memset(&l2_file->dsk.fdr, 0, sizeof(l2_file->dsk.fdr));
	memcpy(l2_file->dsk.fdr.name, filename, 10);

	return 0;
}

/*
    Allocate a new (empty) file
*/
static int new_file_lvl2_win(struct ti99_lvl2_imgref *l2_img, ti99_catalog *parent_catalog, char filename[10], struct ti99_lvl2_fileref *l2_file)
{
	unsigned fdr_AU;
	int catalog_index, i;
	int reply = 0;
	int errorcode;


	if (parent_catalog->num_files >= 127)
		/* if num_files == 127, catalog is full */
		return IMGTOOLERR_NOSPACE;

	/* find insertion point in catalog */
	for (i=0; (i < parent_catalog->num_files) && ((reply = memcmp(parent_catalog->files[i].name, filename, 10)) < 0); i++)
		;

	if ((i<parent_catalog->num_files) && (reply == 0))
		/* file already exists */
		return IMGTOOLERR_BADFILENAME;
	else
	{
		/* otherwise insert new entry */
		catalog_index = i;
		errorcode = alloc_fdr_AU(l2_img, &fdr_AU);
		if (errorcode)
			return errorcode;

		/* shift catalog entries until the insertion point */
		for (i=parent_catalog->num_files; i>catalog_index; i--)
			parent_catalog->files[i] = parent_catalog->files[i-1];

		/* write new catalog entry */
		parent_catalog->files[catalog_index].fdr_ptr = fdr_AU;
		memcpy(parent_catalog->files[catalog_index].name, filename, 10);

		/* update catalog len */
		parent_catalog->num_files++;
	}

	/* set up file handle */
	l2_file->type = L2F_WIN;
	l2_file->win.l2_img = l2_img;
	l2_file->win.fphysrecs = 0;
	l2_file->win.eldestfdr_aphysrec = fdr_AU * l2_img->AUformat.physrecsperAU;
	l2_file->win.curfdr_aphysrec = l2_file->win.eldestfdr_aphysrec;
	memset(&l2_file->win.curfdr, 0, sizeof(l2_file->win.curfdr));
	memcpy(l2_file->win.curfdr.name, filename, 10);

	return 0;
}

/*
    Allocate a new (empty) file
*/
static int new_file_lvl2_tifiles(imgtool_stream *file_handle, struct ti99_lvl2_fileref *l2_file)
{
	/* set up file handle */
	l2_file->type = L2F_TIFILES;
	l2_file->tifiles.file_handle = file_handle;
	memset(&l2_file->tifiles.hdr, 0, sizeof(l2_file->tifiles.hdr));
	l2_file->tifiles.hdr.tifiles[0] = '\7';
	l2_file->tifiles.hdr.tifiles[1] = 'T';
	l2_file->tifiles.hdr.tifiles[2] = 'I';
	l2_file->tifiles.hdr.tifiles[3] = 'F';
	l2_file->tifiles.hdr.tifiles[4] = 'I';
	l2_file->tifiles.hdr.tifiles[5] = 'L';
	l2_file->tifiles.hdr.tifiles[6] = 'E';
	l2_file->tifiles.hdr.tifiles[7] = 'S';

	return 0;
}

/*
    Open an existing file on a floppy image

    l2_img: level 2 image the file is located on
    fpath: access path to the file
    file: set up if open is successful
*/
static int open_file_lvl2_dsk(struct ti99_lvl2_imgref *l2_img, const char *fpath, struct ti99_lvl2_fileref *l2_file)
{
	int parent_ref, is_dir, catalog_index;
	int reply;


	reply = dsk_find_catalog_entry(l2_img, fpath, NULL, &parent_ref, &is_dir, &catalog_index);
	if (reply)
		return reply;

	if (is_dir)
		/* this is not a file */
		return IMGTOOLERR_BADFILENAME;

	l2_file->type = L2F_DSK;
	l2_file->dsk.l2_img = l2_img;
	l2_file->dsk.fdr_aphysrec = l2_img->dsk.catalogs[parent_ref].files[catalog_index].fdr_ptr;
	if (read_absolute_physrec(& l2_img->l1_img, l2_file->dsk.fdr_aphysrec, &l2_file->dsk.fdr))
		return IMGTOOLERR_READERROR;

	return 0;
}

/*
    Open an existing file on a harddisk image

    l2_img: level 2 image the file is located on
    fpath: access path to the file
    file: set up if open is successful
*/
static int open_file_lvl2_win(struct ti99_lvl2_imgref *l2_img, const char *fpath, struct ti99_lvl2_fileref *l2_file)
{
	int parent_ref, is_dir, catalog_index;
	ti99_catalog catalog;
	int reply;

	reply = win_find_catalog_entry(l2_img, fpath, NULL, &parent_ref, &catalog, &is_dir, &catalog_index);
	if (reply)
		return reply;

	if (is_dir)
		/* this is not a file */
		return IMGTOOLERR_BADFILENAME;

	l2_file->type = L2F_WIN;
	l2_file->win.l2_img = l2_img;
	l2_file->win.eldestfdr_aphysrec = catalog.files[catalog_index].fdr_ptr * l2_img->AUformat.physrecsperAU;
	l2_file->win.curfdr_aphysrec = l2_file->win.eldestfdr_aphysrec;
	if (read_absolute_physrec(& l2_img->l1_img, l2_file->win.curfdr_aphysrec, &l2_file->win.curfdr))
		return IMGTOOLERR_READERROR;
	l2_file->win.fphysrecs = get_win_fdr_fphysrecs(&l2_file->win.curfdr);

	/* check integrity of FDR sibling chain */
	/* note that as we check that the back chain is consistent with the forward
	chain, we will also detect any cycle in the sibling chain, so we do not
	need to check against them explicitely */
	if (get_UINT16BE(l2_file->win.curfdr.prevsibFDR_AU) != 0)
		return IMGTOOLERR_CORRUPTIMAGE;

	{
		int i, pastendoflist_flag;
		unsigned cur_fphysrec, sibFDR_AUlen;
		win_fdr *cur_fdr, fdr_buf;
		unsigned curfdr_aphysrec, prevfdr_aphysrec;

		cur_fphysrec = 0;
		pastendoflist_flag = 0;
		cur_fdr = &l2_file->win.curfdr;
		curfdr_aphysrec = l2_file->win.eldestfdr_aphysrec;

		while (1)
		{
			sibFDR_AUlen = 0;
			i=0;
			if (! pastendoflist_flag)
			{
				/* compute number of allocated AUs and check number of AUs */
				for (; i<54; i++)
				{
					if (get_UINT16BE(cur_fdr->clusters[i][0]) == 0)
					{
						pastendoflist_flag = TRUE;
						break;
					}
					sibFDR_AUlen += get_UINT16BE(cur_fdr->clusters[i][1])
									- get_UINT16BE(cur_fdr->clusters[i][0])
									+ 1;
				}
			}
			/* clear remainder of cluster table */
			for (; i<54; i++)
			{
#if 0
				set_UINT16BE(&cur_fdr->clusters[i][0], 0);
				set_UINT16BE(&cur_fdr->clusters[i][1], 0);
#endif
				if ((get_UINT16BE(cur_fdr->clusters[i][0]) != 0) || (get_UINT16BE(cur_fdr->clusters[i][1]) != 0))
					return IMGTOOLERR_CORRUPTIMAGE;
			}

			/* check sibFDR_AUlen field */
			if (get_UINT16BE(cur_fdr->sibFDR_AUlen) != sibFDR_AUlen)
				return IMGTOOLERR_CORRUPTIMAGE;

			/* update current file physrec position to point to end of sibling FDR */
			cur_fphysrec += sibFDR_AUlen * l2_file->win.l2_img->AUformat.physrecsperAU;

			/* exit loop if end of sibling chain */
			if (! get_UINT16BE(cur_fdr->nextsibFDR_AU))
				break;

			/* otherwise read next FDR */
			if (get_UINT16BE(cur_fdr->nextsibFDR_AU) >= l2_file->win.l2_img->AUformat.totAUs)
				return IMGTOOLERR_CORRUPTIMAGE;

			prevfdr_aphysrec = curfdr_aphysrec;
			curfdr_aphysrec = get_win_fdr_nextsibFDR_aphysrec(l2_file->win.l2_img, cur_fdr);
			if (read_absolute_physrec(& l2_file->win.l2_img->l1_img, curfdr_aphysrec, &fdr_buf))
				return IMGTOOLERR_READERROR;
			cur_fdr = &fdr_buf;

			/* check that back chaining is consistent with forward chaining */
			if (get_win_fdr_prevsibFDR_aphysrec(l2_file->win.l2_img, &fdr_buf) != prevfdr_aphysrec)
				return IMGTOOLERR_CORRUPTIMAGE;
			/*  check fphysrecs field */
			if (get_win_fdr_fphysrecs(&fdr_buf) != cur_fphysrec)
				return IMGTOOLERR_CORRUPTIMAGE;

			/* check consistency of informative fields (name, record format, flags, etc) */
			if (memcmp(fdr_buf.name, l2_file->win.curfdr.name, 10)
					|| (get_UINT16BE(fdr_buf.xreclen) != get_UINT16BE(l2_file->win.curfdr.xreclen))
					|| (fdr_buf.flags != l2_file->win.curfdr.flags)
					|| (fdr_buf.recsperphysrec != l2_file->win.curfdr.recsperphysrec)
					|| (fdr_buf.eof != l2_file->win.curfdr.eof)
					|| (fdr_buf.reclen != l2_file->win.curfdr.reclen)
					|| (get_UINT16LE(fdr_buf.fixrecs_LSW) != get_UINT16LE(l2_file->win.curfdr.fixrecs_LSW))
					|| memcmp(&fdr_buf.creation, &l2_file->win.curfdr.creation, 4)
					|| memcmp(&fdr_buf.update, &l2_file->win.curfdr.update, 4)
					/*|| memcmp(fdr_buf.id, l2_file->win.curfdr.id, 2)*/
					|| (get_UINT16BE(fdr_buf.parent_FDIR_AU) != get_UINT16BE(l2_file->win.curfdr.parent_FDIR_AU))
					|| ((fdr_buf.xinfo_MSB & 0xf) != (l2_file->win.curfdr.xinfo_MSB & 0xf)))
				return IMGTOOLERR_CORRUPTIMAGE;
		}
		if (cur_fphysrec < l2_file->win.fphysrecs)
			return IMGTOOLERR_CORRUPTIMAGE;
	}

	return 0;
}

/*
    Open an existing file in TIFILES format
*/
static int open_file_lvl2_tifiles(imgtool_stream *file_handle, struct ti99_lvl2_fileref *l2_file)
{
	/* set up file handle */
	l2_file->type = L2F_TIFILES;
	l2_file->tifiles.file_handle = file_handle;

	/* seek to header */
	if (stream_seek(l2_file->tifiles.file_handle, 0, SEEK_SET))
		return IMGTOOLERR_READERROR;
	/* read it */
	if (stream_read(l2_file->tifiles.file_handle, &l2_file->tifiles.hdr, sizeof(l2_file->tifiles.hdr)) != sizeof(l2_file->tifiles.hdr))
		return IMGTOOLERR_READERROR;

	return 0;
}

/*
    compute the aphysrec address for a given file physical record (fphysrec)

    l2_img: image where the file is located
*/
static int dsk_fphysrec_to_aphysrec(ti99_lvl2_fileref_dsk *dsk_file, unsigned fphysrec, unsigned *aphysrec)
{
	int cluster_index;
	int cluster_firstfphysrec, cluster_lastfphysrec;
	int cluster_baseaphysrec;


	/* check parameter */
	if (fphysrec >= get_UINT16BE(dsk_file->fdr.fphysrecs))
		return IMGTOOLERR_UNEXPECTED;


	/* search for the cluster in the data chain pointers array */
	cluster_firstfphysrec = 0;
	for (cluster_index=0; cluster_index<76; cluster_index++)
	{
		/* read curent file block table entry */
		cluster_lastfphysrec = get_dsk_fdr_cluster_lastfphysrec(&dsk_file->fdr, cluster_index);
		if (cluster_lastfphysrec >= fphysrec)
			break;
		cluster_firstfphysrec = cluster_lastfphysrec+1;
	}
	if (cluster_index == 76)
		/* if not found, the file is corrupt */
		return IMGTOOLERR_CORRUPTIMAGE;

	/* read base aphysrec address for this cluster */
	cluster_baseaphysrec = get_dsk_fdr_cluster_baseaphysrec(dsk_file->l2_img, &dsk_file->fdr, cluster_index);
	/* return absolute physrec address */
	*aphysrec = cluster_baseaphysrec + (fphysrec - cluster_firstfphysrec);
	return 0;
}

/*
    compute the aphysrec address for a given file physical record (fphysrec)

    l2_img: image where the file is located
*/
static int win_fphysrec_to_aphysrec(ti99_lvl2_fileref_win *win_file, unsigned fphysrec, unsigned *aphysrec)
{
	int cluster_index;
	int cluster_firstfphysrec, cluster_lastfphysrec;
	int cluster_baseaphysrec;
	int errorcode;


	/* check parameter */
	if (fphysrec >= win_file->fphysrecs)
		return IMGTOOLERR_UNEXPECTED;

	/* look for correct sibling */
	if (fphysrec < get_win_fdr_cursibFDR_basefphysrec(& win_file->curfdr))
	{
		while (fphysrec < get_win_fdr_cursibFDR_basefphysrec(& win_file->curfdr))
		{
			if (get_UINT16BE(win_file->curfdr.prevsibFDR_AU) == 0)
				return IMGTOOLERR_CORRUPTIMAGE;
			errorcode = win_goto_prev_sibFDR(win_file);
			if (errorcode)
				return errorcode;
		}
	}
	else /*if (fphysrec >= get_win_fdr_cursibFDR_basefphysrec(& dsk_file->curfdr))*/
	{
		while (fphysrec >= (get_win_fdr_cursibFDR_basefphysrec(& win_file->curfdr)
								+ get_UINT16BE(win_file->curfdr.sibFDR_AUlen) * win_file->l2_img->AUformat.physrecsperAU))
		{
			if (get_UINT16BE(win_file->curfdr.nextsibFDR_AU) == 0)
				return IMGTOOLERR_CORRUPTIMAGE;
			errorcode = win_goto_next_sibFDR(win_file);
			if (errorcode)
				return errorcode;
		}
	}


	/* search for the cluster in the data chain pointers array */
	cluster_firstfphysrec = get_win_fdr_cursibFDR_basefphysrec(& win_file->curfdr);
	for (cluster_index = 0; cluster_index<54; cluster_index++)
	{
		cluster_lastfphysrec = cluster_firstfphysrec
									+ (get_UINT16BE(win_file->curfdr.clusters[cluster_index][1])
											- get_UINT16BE(win_file->curfdr.clusters[cluster_index][0])
											+ 1)
										* win_file->l2_img->AUformat.physrecsperAU;
		if (fphysrec < cluster_lastfphysrec)
			break;

		cluster_firstfphysrec = cluster_lastfphysrec;
	}

	if (cluster_index == 54)
		return IMGTOOLERR_CORRUPTIMAGE;


	/* read base aphysrec address for this cluster */
	cluster_baseaphysrec = get_UINT16BE(win_file->curfdr.clusters[cluster_index][0]) * win_file->l2_img->AUformat.physrecsperAU;
	/* return absolute physrec address */
	*aphysrec = cluster_baseaphysrec + (fphysrec - cluster_firstfphysrec);
	return 0;
}

/*
    read a 256-byte physical record from a file
*/
static int read_file_physrec(struct ti99_lvl2_fileref *l2_file, unsigned fphysrec, void *dest)
{
	int errorcode;
	unsigned aphysrec;

	switch (l2_file->type)
	{
	case L2F_DSK:
		/* compute absolute physrec address */
		errorcode = dsk_fphysrec_to_aphysrec(&l2_file->dsk, fphysrec, &aphysrec);
		if (errorcode)
			return errorcode;
		/* read physrec */
		if (read_absolute_physrec(& l2_file->dsk.l2_img->l1_img, aphysrec, dest))
			return IMGTOOLERR_READERROR;
		break;

	case L2F_WIN:
		/* compute absolute physrec address */
		errorcode = win_fphysrec_to_aphysrec(&l2_file->win, fphysrec, &aphysrec);
		if (errorcode)
			return errorcode;
		/* read physrec */
		if (read_absolute_physrec(& l2_file->win.l2_img->l1_img, aphysrec, dest))
			return IMGTOOLERR_READERROR;
		break;

	case L2F_TIFILES:
		/* seek to physrec */
		if (stream_seek(l2_file->tifiles.file_handle, 128+256*fphysrec, SEEK_SET))
			return IMGTOOLERR_READERROR;
		/* read it */
		if (stream_read(l2_file->tifiles.file_handle, dest, 256) != 256)
			return IMGTOOLERR_READERROR;
		break;
	}

	return 0;
}

/*
    read a 256-byte physical record from a file
*/
static int write_file_physrec(struct ti99_lvl2_fileref *l2_file, unsigned fphysrec, const void *src)
{
	int errorcode;
	unsigned aphysrec;

	switch (l2_file->type)
	{
	case L2F_DSK:
		/* compute absolute physrec address */
		errorcode = dsk_fphysrec_to_aphysrec(&l2_file->dsk, fphysrec, &aphysrec);
		if (errorcode)
			return errorcode;
		/* write physrec */
		if (write_absolute_physrec(& l2_file->dsk.l2_img->l1_img, aphysrec, src))
			return IMGTOOLERR_WRITEERROR;
		break;

	case L2F_WIN:
		/* compute absolute physrec address */
		errorcode = win_fphysrec_to_aphysrec(&l2_file->win, fphysrec, &aphysrec);
		if (errorcode)
			return errorcode;
		/* write physrec */
		if (write_absolute_physrec(& l2_file->win.l2_img->l1_img, aphysrec, src))
			return IMGTOOLERR_WRITEERROR;
		break;

	case L2F_TIFILES:
		/* seek to physrec */
		if (stream_seek(l2_file->tifiles.file_handle, 128+256*fphysrec, SEEK_SET))
			return IMGTOOLERR_WRITEERROR;
		/* write it */
		if (stream_write(l2_file->tifiles.file_handle, src, 256) != 256)
			return IMGTOOLERR_WRITEERROR;
		break;
	}

	return 0;
}

/*
    Write a field in every fdr record associated to a file
*/
#ifdef UNUSED_FUNCTION
static int set_win_fdr_field(struct ti99_lvl2_fileref *l2_file, size_t offset, size_t size, void *data)
{
	win_fdr fdr_buf;
	unsigned fdr_aphysrec;
	int errorcode = 0;

	for (fdr_aphysrec = l2_file->win.eldestfdr_aphysrec;
			fdr_aphysrec && ((errorcode = (read_absolute_physrec(&l2_file->win.l2_img->l1_img, fdr_aphysrec, &fdr_buf) ? IMGTOOLERR_READERROR : 0)) == 0);
			fdr_aphysrec = get_win_fdr_nextsibFDR_physrec(l2_file->win.l2_img, &fdr_buf))
	{
		memcpy(((UINT8 *) &fdr_buf) + offset, data, size);
		if (write_absolute_physrec(&l2_file->win.l2_img->l1_img, fdr_aphysrec, &fdr_buf))
		{
			errorcode = IMGTOOLERR_WRITEERROR;
			break;
		}
	}

	return errorcode;
}
#endif

static UINT8 get_file_flags(struct ti99_lvl2_fileref *l2_file)
{
	int reply = 0;

	switch (l2_file->type)
	{
	case L2F_DSK:
		reply = l2_file->dsk.fdr.flags;
		break;

	case L2F_WIN:
		reply = l2_file->win.curfdr.flags;
		break;

	case L2F_TIFILES:
		reply = l2_file->tifiles.hdr.flags;
		break;
	}

	return reply;
}

static void set_file_flags(struct ti99_lvl2_fileref *l2_file, UINT8 data)
{
	switch (l2_file->type)
	{
	case L2F_DSK:
		l2_file->dsk.fdr.flags = data;
		break;

	case L2F_WIN:
		l2_file->win.curfdr.flags = data;
		break;

	case L2F_TIFILES:
		l2_file->tifiles.hdr.flags = data;
		break;
	}
}

static UINT8 get_file_recsperphysrec(struct ti99_lvl2_fileref *l2_file)
{
	int reply = 0;

	switch (l2_file->type)
	{
	case L2F_DSK:
		reply = l2_file->dsk.fdr.recsperphysrec;
		break;

	case L2F_WIN:
		reply = l2_file->win.curfdr.recsperphysrec;
		break;

	case L2F_TIFILES:
		reply = l2_file->tifiles.hdr.recsperphysrec;
		break;
	}

	return reply;
}

static void set_file_recsperphysrec(struct ti99_lvl2_fileref *l2_file, UINT8 data)
{
	switch (l2_file->type)
	{
	case L2F_DSK:
		l2_file->dsk.fdr.recsperphysrec = data;
		break;

	case L2F_WIN:
		l2_file->win.curfdr.recsperphysrec = data;
		break;

	case L2F_TIFILES:
		l2_file->tifiles.hdr.recsperphysrec = data;
		break;
	}
}

static unsigned get_file_fphysrecs(struct ti99_lvl2_fileref *l2_file)
{
	int reply = 0;

	switch (l2_file->type)
	{
	case L2F_DSK:
		reply = get_UINT16BE(l2_file->dsk.fdr.fphysrecs);
		break;

	case L2F_WIN:
		reply = l2_file->win.fphysrecs;
		break;

	case L2F_TIFILES:
		reply = get_UINT16BE(l2_file->tifiles.hdr.fphysrecs);
		break;
	}

	return reply;
}

static int set_file_fphysrecs(struct ti99_lvl2_fileref *l2_file, unsigned data)
{
	switch (l2_file->type)
	{
	case L2F_DSK:
		if (data >= 65536)
			return IMGTOOLERR_UNIMPLEMENTED;
		set_UINT16BE(&l2_file->dsk.fdr.fphysrecs, data);
		break;

	case L2F_WIN:
		l2_file->win.fphysrecs = data;
		break;

	case L2F_TIFILES:
		if (data >= 65536)
			return IMGTOOLERR_UNIMPLEMENTED;
		set_UINT16BE(&l2_file->tifiles.hdr.fphysrecs, data);
		break;
	}

	return 0;
}

static UINT8 get_file_eof(struct ti99_lvl2_fileref *l2_file)
{
	int reply = 0;

	switch (l2_file->type)
	{
	case L2F_DSK:
		reply = l2_file->dsk.fdr.eof;
		break;

	case L2F_WIN:
		reply = l2_file->win.curfdr.eof;
		break;

	case L2F_TIFILES:
		reply = l2_file->tifiles.hdr.eof;
		break;
	}

	return reply;
}

static void set_file_eof(struct ti99_lvl2_fileref *l2_file, UINT8 data)
{
	switch (l2_file->type)
	{
	case L2F_DSK:
		l2_file->dsk.fdr.eof = data;
		break;

	case L2F_WIN:
		l2_file->win.curfdr.eof = data;
		break;

	case L2F_TIFILES:
		l2_file->tifiles.hdr.eof = data;
		break;
	}
}

static UINT16 get_file_reclen(struct ti99_lvl2_fileref *l2_file)
{
	int reply = 0;

	switch (l2_file->type)
	{
	case L2F_DSK:
		reply = l2_file->dsk.fdr.reclen;
		if ((reply == 0) && (! (l2_file->dsk.fdr.flags & (fdr99_f_program /*| fdr99_f_var*/))))
			reply = get_UINT16BE(l2_file->dsk.fdr.xreclen);
		break;

	case L2F_WIN:
		reply = l2_file->win.curfdr.reclen;
		if ((reply == 0) && (! (l2_file->win.curfdr.flags & (fdr99_f_program /*| fdr99_f_var*/))))
			reply = get_UINT16BE(l2_file->win.curfdr.xreclen);
		break;

	case L2F_TIFILES:
		reply = l2_file->tifiles.hdr.reclen;
		break;
	}

	return reply;
}

static int set_file_reclen(struct ti99_lvl2_fileref *l2_file, UINT16 data)
{
	switch (l2_file->type)
	{
	case L2F_DSK:
		if (data < 256)
		{
			l2_file->dsk.fdr.reclen = data;
			set_UINT16BE(&l2_file->dsk.fdr.xreclen, 0);
		}
		else
		{
			l2_file->dsk.fdr.reclen = 0;
			set_UINT16BE(&l2_file->dsk.fdr.xreclen, data);
		}
		break;

	case L2F_WIN:
		if (data < 256)
		{
			l2_file->win.curfdr.reclen = data;
			set_UINT16BE(&l2_file->win.curfdr.xreclen, 0);
		}
		else
		{
			l2_file->win.curfdr.reclen = 0;
			set_UINT16BE(&l2_file->win.curfdr.xreclen, data);
		}
		break;

	case L2F_TIFILES:
		if (data >= 256)
			return IMGTOOLERR_UNIMPLEMENTED;
		l2_file->tifiles.hdr.reclen = data;
		break;
	}

	return 0;
}

static unsigned get_file_fixrecs(struct ti99_lvl2_fileref *l2_file)
{
	int reply = 0;

	switch (l2_file->type)
	{
	case L2F_DSK:
		reply = get_UINT16LE(l2_file->dsk.fdr.fixrecs);
		break;

	case L2F_WIN:
		reply = get_win_fdr_fixrecs(&l2_file->win.curfdr);
		break;

	case L2F_TIFILES:
		reply = get_UINT16BE(l2_file->tifiles.hdr.fixrecs);
		break;
	}

	return reply;
}

static int set_file_fixrecs(struct ti99_lvl2_fileref *l2_file, unsigned data)
{
	switch (l2_file->type)
	{
	case L2F_DSK:
		if (data >= 65536)
			return IMGTOOLERR_UNIMPLEMENTED;
		set_UINT16LE(&l2_file->dsk.fdr.fixrecs, data);
		break;

	case L2F_WIN:
		set_win_fdr_fixrecs(&l2_file->win.curfdr, data);
		break;

	case L2F_TIFILES:
		if (data >= 65536)
			return IMGTOOLERR_UNIMPLEMENTED;
		set_UINT16BE(&l2_file->tifiles.hdr.fixrecs, data);
		break;
	}

	return 0;
}

static void get_file_creation_date(struct ti99_lvl2_fileref *l2_file, ti99_date_time *reply)
{
	switch (l2_file->type)
	{
	case L2F_DSK:
		*reply = l2_file->dsk.fdr.creation;
		break;

	case L2F_WIN:
		*reply = l2_file->win.curfdr.creation;
		break;

	case L2F_TIFILES:
		memset(reply, 0, sizeof(*reply));
		break;
	}
}

static void set_file_creation_date(struct ti99_lvl2_fileref *l2_file, ti99_date_time data)
{
	switch (l2_file->type)
	{
	case L2F_DSK:
		l2_file->dsk.fdr.creation = data;
		break;

	case L2F_WIN:
		l2_file->win.curfdr.creation = data;
		break;

	case L2F_TIFILES:
		break;
	}
}

static void get_file_update_date(struct ti99_lvl2_fileref *l2_file, ti99_date_time *reply)
{
	switch (l2_file->type)
	{
	case L2F_DSK:
		*reply = l2_file->dsk.fdr.update;
		break;

	case L2F_WIN:
		*reply = l2_file->win.curfdr.update;
		break;

	case L2F_TIFILES:
		memset(reply, 0, sizeof(*reply));
		break;
	}
}

static void set_file_update_date(struct ti99_lvl2_fileref *l2_file, ti99_date_time data)
{
	switch (l2_file->type)
	{
	case L2F_DSK:
		l2_file->dsk.fdr.update = data;
		break;

	case L2F_WIN:
		l2_file->win.curfdr.update = data;
		break;

	case L2F_TIFILES:
		break;
	}
}

#ifdef UNUSED_FUNCTION
static void current_date_time(ti99_date_time *reply)
{
	/* All these functions should be ANSI */
	time_t cur_time = time(NULL);
	struct tm expanded_time = *localtime(& cur_time);

	reply->time_MSB = (expanded_time.tm_hour << 3) | (expanded_time.tm_min >> 3);
	reply->time_LSB = (expanded_time.tm_min << 5) | (expanded_time.tm_sec >> 1);
	reply->date_MSB = ((expanded_time.tm_year % 100) << 1) | ((expanded_time.tm_mon+1) >> 3);
	reply->date_LSB = ((expanded_time.tm_mon+1) << 5) | expanded_time.tm_mday;
}
#endif

#if 0
#pragma mark -
#pragma mark LEVEL 3 DISK ROUTINES
#endif

/*
    Level 3 implements files as a succession of logical records.

    There are three types of files:
    * program files that are not implemented at level 3 (no logical record)
    * files with fixed-size records (random-access files)
    * files with variable-size records (sequential-access)
*/

struct ti99_lvl3_fileref
{
	ti99_lvl2_fileref l2_file;

	int cur_log_rec;
	int cur_phys_rec;
	int cur_pos_in_phys_rec;
};

#ifdef UNUSED_FUNCTION
/*
    Open a file on level 3.

    To open a file on level 3, you must open (or create) the file on level 2,
    then pass the file reference to open_file_lvl3.
*/
static int open_file_lvl3(ti99_lvl3_fileref *l3_file)
{
	l3_file->cur_log_rec = 0;
	l3_file->cur_phys_rec = 0;
	l3_file->cur_pos_in_phys_rec = 0;

	/*if ()*/

	return 0;
}

/*
    Test a file for EOF
*/
static int is_eof(ti99_lvl3_fileref *l3_file)
{
	int flags = get_file_flags(&l3_file->l2_file);
	int fphysrecs = get_file_fphysrecs(&l3_file->l2_file);
	int fdr_eof = get_file_eof(&l3_file->l2_file);

	if (flags & fdr99_f_var)
	{
		return (l3_file->cur_phys_rec >= fphysrecs);
	}
	else
	{
		return ((l3_file->cur_phys_rec >= fphysrecs)
				|| ((l3_file->cur_phys_rec == (fphysrecs-1))
					&& (l3_file->cur_pos_in_phys_rec >= (fdr_eof ? fdr_eof : 256))));
	}
}

/*
    Read next record from a file
*/
static int read_next_record(ti99_lvl3_fileref *l3_file, void *dest, int *out_reclen)
{
	int errorcode;
	UINT8 physrec_buf[256];
	int reclen;
	int flags = get_file_flags(&l3_file->l2_file);
	int fphysrecs = get_file_fphysrecs(&l3_file->l2_file);
	int fdr_eof = get_file_eof(&l3_file->l2_file);

	if (flags & fdr99_f_program)
	{
		/* program files have no level-3 record */
		return IMGTOOLERR_UNEXPECTED;
	}
	else if (flags & fdr99_f_var)
	{
		/* variable-length records */
		if (is_eof(l3_file))
			return IMGTOOLERR_UNEXPECTED;
		errorcode = read_file_physrec(&l3_file->l2_file, l3_file->cur_phys_rec, physrec_buf);
		if (errorcode)
			return errorcode;
		/* read reclen */
		reclen = physrec_buf[l3_file->cur_pos_in_phys_rec];
		/* check integrity */
		if ((reclen == 0xff) || (reclen > get_file_reclen(&l3_file->l2_file))
				|| ((l3_file->cur_pos_in_phys_rec + 1 + reclen) > 256))
			return IMGTOOLERR_CORRUPTIMAGE;
		/* copy to buffer */
		memcpy(dest, physrec_buf + l3_file->cur_pos_in_phys_rec + 1, reclen);
		l3_file->cur_pos_in_phys_rec += reclen + 1;
		/* skip to next physrec if needed */
		if ((l3_file->cur_pos_in_phys_rec == 256)
				|| (physrec_buf[l3_file->cur_pos_in_phys_rec] == 0xff))
		{
			l3_file->cur_pos_in_phys_rec = 0;
			l3_file->cur_phys_rec++;
		}
		(* out_reclen) = reclen;
	}
	else
	{
		/* fixed len records */
		reclen = get_file_reclen(&l3_file->l2_file);
		if (is_eof(l3_file))
			return IMGTOOLERR_UNEXPECTED;
		if ((l3_file->cur_pos_in_phys_rec + reclen) > 256)
		{
			l3_file->cur_pos_in_phys_rec = 0;
			l3_file->cur_phys_rec++;
		}
		if ((l3_file->cur_phys_rec >= fphysrecs)
				|| ((l3_file->cur_phys_rec == (fphysrecs-1))
					&& ((l3_file->cur_pos_in_phys_rec + reclen) >= (fdr_eof ? fdr_eof : 256))))
			return IMGTOOLERR_CORRUPTIMAGE;
		errorcode = read_file_physrec(&l3_file->l2_file, l3_file->cur_phys_rec, physrec_buf);
		if (errorcode)
			return errorcode;
		memcpy(dest, physrec_buf + l3_file->cur_pos_in_phys_rec, reclen);
		l3_file->cur_pos_in_phys_rec += reclen;
		if (l3_file->cur_pos_in_phys_rec == 256)
		{
			l3_file->cur_pos_in_phys_rec = 0;
			l3_file->cur_phys_rec++;
		}
		(* out_reclen) = reclen;
	}

	return 0;
}
#endif

#if 0
#pragma mark -
#pragma mark IMGTOOL MODULE IMPLEMENTATION
#endif

/*
    ti99 catalog iterator, used when imgtool reads the catalog
*/
struct dsk_iterator
{
	struct ti99_lvl2_imgref *image;
	int level;
	int listing_subdirs;        /* true if we are listing subdirectories at current level */
	int index[2];               /* current index in the disk catalog */
	ti99_catalog *cur_catalog;  /* current catalog */
};

struct win_iterator
{
	struct ti99_lvl2_imgref *image;
	int level;
	int listing_subdirs;        /* true if we are listing subdirectories at current level */
	int index[MAX_DIR_LEVEL];   /* current index in the disk catalog */
	ti99_catalog catalog[MAX_DIR_LEVEL];    /* current catalog */
};


static imgtoolerr_t dsk_image_init_mess(imgtool_image *image, imgtool_stream *f);
static imgtoolerr_t dsk_image_init_v9t9(imgtool_image *image, imgtool_stream *f);
static imgtoolerr_t dsk_image_init_pc99_fm(imgtool_image *image, imgtool_stream *f);
static imgtoolerr_t dsk_image_init_pc99_mfm(imgtool_image *image, imgtool_stream *f);
static imgtoolerr_t win_image_init(imgtool_image *image, imgtool_stream *f);
static void ti99_image_exit(imgtool_image *img);
static void ti99_image_info(imgtool_image *img, char *string, size_t len);
static imgtoolerr_t dsk_image_beginenum(imgtool_directory *enumeration, const char *path);
static imgtoolerr_t dsk_image_nextenum(imgtool_directory *enumeration, imgtool_dirent *ent);
static imgtoolerr_t win_image_beginenum(imgtool_directory *enumeration, const char *path);
static imgtoolerr_t win_image_nextenum(imgtool_directory *enumeration, imgtool_dirent *ent);
static imgtoolerr_t ti99_image_freespace(imgtool_partition *partition, UINT64 *size);
static imgtoolerr_t ti99_image_readfile(imgtool_partition *partition, const char *fpath, const char *fork, imgtool_stream *destf);
static imgtoolerr_t ti99_image_writefile(imgtool_partition *partition, const char *fpath, const char *fork, imgtool_stream *sourcef, option_resolution *writeoptions);
static imgtoolerr_t dsk_image_deletefile(imgtool_partition *partition, const char *fpath);
static imgtoolerr_t win_image_deletefile(imgtool_partition *partition, const char *fpath);
static imgtoolerr_t dsk_image_create_mess(imgtool_image *image, imgtool_stream *f, option_resolution *createoptions);
static imgtoolerr_t dsk_image_create_v9t9(imgtool_image *image, imgtool_stream *f, option_resolution *createoptions);

enum
{
	dsk_createopts_volname = 'A',
	dsk_createopts_sides = 'B',
	dsk_createopts_tracks = 'C',
	dsk_createopts_sectors = 'D',
	dsk_createopts_protection = 'E',
	dsk_createopts_density = 'F'
};

static OPTION_GUIDE_START( dsk_create_optionguide )
	OPTION_STRING(dsk_createopts_volname, "label",  "Volume name" )
	OPTION_INT(dsk_createopts_sides, "sides", "Sides" )
	OPTION_INT(dsk_createopts_tracks, "tracks", "Tracks" )
	OPTION_INT(dsk_createopts_sectors, "sectors",   "Sectors (1->9 for SD, 1->18 for DD, 1->36 for HD)" )
	OPTION_INT(dsk_createopts_protection, "protection", "Protection (0 for normal, 1 for protected)" )
	OPTION_ENUM_START(dsk_createopts_density, "density", "Density" )
		OPTION_ENUM(    0,      "Auto",             "Auto" )
		OPTION_ENUM(    1,      "SD",               "Single Density" )
		OPTION_ENUM(    2,      "DD",               "Double Density" )
		OPTION_ENUM(    3,      "HD",               "High Density" )
	OPTION_ENUM_END
OPTION_GUIDE_END

#define dsk_create_optionspecs "B1-[2];C1-[40]-80;D1-[18]-36;E[0]-1;F[0]-3"

static void ti99_getinfo(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES:             info->i = sizeof(ti99_lvl2_imgref); break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:             info->i = sizeof(dsk_iterator); break;

		case IMGTOOLINFO_STR_EOLN:                          strcpy(info->s = imgtool_temp_str(), "\r"); break;
		case IMGTOOLINFO_PTR_CLOSE:                         info->close = ti99_image_exit; break;
		case IMGTOOLINFO_PTR_INFO:                          info->info = ti99_image_info; break;
		case IMGTOOLINFO_PTR_FREE_SPACE:                    info->free_space = ti99_image_freespace; break;
		case IMGTOOLINFO_PTR_READ_FILE:                     info->read_file = ti99_image_readfile; break;
		case IMGTOOLINFO_PTR_WRITE_FILE:                    info->write_file = ti99_image_writefile; break;
	}
}

static void ti99_dsk_getinfo(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		case IMGTOOLINFO_STR_FILE_EXTENSIONS:               strcpy(info->s = imgtool_temp_str(), "dsk"); break;
		case IMGTOOLINFO_PTR_BEGIN_ENUM:                    info->begin_enum = dsk_image_beginenum; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                     info->next_enum = dsk_image_nextenum; break;
		case IMGTOOLINFO_PTR_DELETE_FILE:                   info->delete_file = dsk_image_deletefile; break;
		default:                                            ti99_getinfo(imgclass, state, info);
	}
}

void ti99_old_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		case IMGTOOLINFO_STR_NAME:                  strcpy(info->s = imgtool_temp_str(), "ti99_old"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:           strcpy(info->s = imgtool_temp_str(), "TI99 Diskette (old MESS format)"); break;
		case IMGTOOLINFO_PTR_OPEN:                  info->open = dsk_image_init_mess; break;
		case IMGTOOLINFO_PTR_CREATE:                info->create = dsk_image_create_mess; break;
		case IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE:  info->createimage_optguide = dsk_create_optionguide; break;
		case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC:   strcpy(info->s = imgtool_temp_str(), dsk_create_optionspecs); break;
		default:                                    ti99_dsk_getinfo(imgclass, state, info); break;
	}
}

void ti99_v9t9_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		case IMGTOOLINFO_STR_NAME:                  strcpy(info->s = imgtool_temp_str(), "v9t9"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:           strcpy(info->s = imgtool_temp_str(), "TI99 Diskette (V9T9 format)"); break;
		case IMGTOOLINFO_PTR_OPEN:                  info->open = dsk_image_init_v9t9; break;
		case IMGTOOLINFO_PTR_CREATE:                info->create = dsk_image_create_v9t9; break;
		case IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE:  info->createimage_optguide = dsk_create_optionguide; break;
		case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC:   strcpy(info->s = imgtool_temp_str(), dsk_create_optionspecs); break;
		default:                                    ti99_dsk_getinfo(imgclass, state, info); break;
	}
}

void ti99_pc99fm_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		case IMGTOOLINFO_STR_NAME:                  strcpy(info->s = imgtool_temp_str(), "pc99fm"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:           strcpy(info->s = imgtool_temp_str(), "TI99 Diskette (PC99 FM format)"); break;
		case IMGTOOLINFO_PTR_OPEN:                  info->open = dsk_image_init_pc99_fm; break;
		case IMGTOOLINFO_PTR_CREATE:                /* info->create = dsk_image_create_pc99fm; */ break;
		default:                                    ti99_dsk_getinfo(imgclass, state, info); break;
	}
}

void ti99_pc99mfm_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		case IMGTOOLINFO_STR_NAME:                  strcpy(info->s = imgtool_temp_str(), "pc99mfm"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:           strcpy(info->s = imgtool_temp_str(), "TI99 Diskette (PC99 MFM format)"); break;
		case IMGTOOLINFO_PTR_OPEN:                  info->open = dsk_image_init_pc99_mfm; break;
		case IMGTOOLINFO_PTR_CREATE:                /* info->create = dsk_image_create_pc99mfm; */ break;
		default:                                    ti99_dsk_getinfo(imgclass, state, info); break;
	}
}

void ti99_ti99hd_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		case IMGTOOLINFO_STR_NAME:                  strcpy(info->s = imgtool_temp_str(), "ti99hd"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:           strcpy(info->s = imgtool_temp_str(), "TI99 Harddisk"); break;
		case IMGTOOLINFO_PTR_OPEN:                  info->open = win_image_init; break;
		case IMGTOOLINFO_PTR_CREATE:                /* info->create = hd_image_create; */ break;

		case IMGTOOLINFO_STR_FILE_EXTENSIONS:       strcpy(info->s = imgtool_temp_str(), "hd"); break;
		case IMGTOOLINFO_PTR_BEGIN_ENUM:            info->begin_enum = win_image_beginenum; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:             info->next_enum = win_image_nextenum; break;
		case IMGTOOLINFO_PTR_DELETE_FILE:           info->delete_file = win_image_deletefile; break;
		default:                                    ti99_getinfo(imgclass, state, info);
	}
}



/*
    Open a file as a ti99_image (common code).
*/
static int dsk_image_init(imgtool_image *img, imgtool_stream *f, ti99_img_format img_format)
{
	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(img);
	dsk_vib vib;
	int reply;
	int totphysrecs;
	unsigned fdir_aphysrec;
	int i;

	/* open disk image at level 1 */
	reply = open_image_lvl1(f, img_format, &image->l1_img, &vib);
	if (reply)
		return reply;

	/* open disk image at level 2 */
	image->type = L2I_DSK;

	/* @BN@ */
	/* Copy in the allocation bit map! */
	memcpy(image->abm, vib.abm, 200);
	/* first compute AU size and number of AUs */
	totphysrecs = get_UINT16BE(vib.totphysrecs);

	image->AUformat.physrecsperAU = (totphysrecs + 1599) / 1600;
	/* round to next larger power of 2 */
	for (i = 1; i < image->AUformat.physrecsperAU; i <<= 1)
		;
	image->AUformat.physrecsperAU = i;
	image->AUformat.totAUs = totphysrecs / image->AUformat.physrecsperAU;

	/* extract number of physrecs */
	image->dsk.totphysrecs = get_UINT16BE(vib.totphysrecs);

	/* read and check main volume catalog */
	reply = dsk_read_catalog(image, 1, &image->dsk.catalogs[0]);
	if (reply)
		return reply;

	image->dsk.fdir_aphysrec[0] = 1;

	/* read and check subdirectory catalogs */
	/* Note that the reserved areas used for HFDC subdirs may be used for other
	purposes by other FDRs, so, if we get any error, we will assume there is no
	subdir after all... */
	image->dsk.catalogs[0].num_subdirs = 0;
	for (i=0; i<3; i++)
	{
		fdir_aphysrec = get_UINT16BE(vib.subdir[i].fdir_aphysrec);
		if ((! memcmp(vib.subdir[i].name, "\0\0\0\0\0\0\0\0\0\0", 10))
				|| (! memcmp(vib.subdir[i].name, "          ", 10)))
		{
			/* name is empty: fine with us unless there is a fdir pointer */
			if (fdir_aphysrec != 0)
			{
				image->dsk.catalogs[0].num_subdirs = 0;
				break;
			}
		}
		else if (check_fname(vib.subdir[i].name))
		{
			/* name is invalid: this is not an HFDC format floppy */
			image->dsk.catalogs[0].num_subdirs = 0;
			break;
		}
		else
		{
			/* there is a non-empty name */
			if ((fdir_aphysrec == 0) || (fdir_aphysrec >= totphysrecs))
			{
				/* error: fdir pointer is invalid or NULL */
				image->dsk.catalogs[0].num_subdirs = 0;
				break;
			}
			/* fill in descriptor fields */
			image->dsk.fdir_aphysrec[image->dsk.catalogs[0].num_subdirs+1] = fdir_aphysrec;
			/*image->dsk.catalogs[0].subdirs[image->dsk.catalogs[0].num_subdirs].dir_ptr = fdir_aphysrec;*/
			memcpy(image->dsk.catalogs[0].subdirs[image->dsk.catalogs[0].num_subdirs].name, vib.subdir[i].name, 10);
			reply = dsk_read_catalog(image, fdir_aphysrec, &image->dsk.catalogs[image->dsk.catalogs[0].num_subdirs+1]);
			if (reply)
			{
				/* error: invalid fdir */
				image->dsk.catalogs[0].num_subdirs = 0;
				break;
			}
			/* found valid subdirectory: increment subdir count */
			image->dsk.catalogs[0].num_subdirs++;
		}
	}

	/* extract volume name */
	memcpy(image->vol_name, vib.name, 10);

	/* initialize default data_offset */
	image->data_offset = 32+2;

	return 0;
}

/*
    Open a file as a ti99_image (MESS format).
*/
static imgtoolerr_t dsk_image_init_mess(imgtool_image *image, imgtool_stream *f)
{
	return (imgtoolerr_t)dsk_image_init(image, f, if_mess);
}

/*
    Open a file as a ti99_image (V9T9 format).
*/
static imgtoolerr_t dsk_image_init_v9t9(imgtool_image *image, imgtool_stream *f)
{
	return (imgtoolerr_t)dsk_image_init(image, f, if_v9t9);
}

/*
    Open a file as a ti99_image (PC99 FM format).
*/
static imgtoolerr_t dsk_image_init_pc99_fm(imgtool_image *image, imgtool_stream *f)
{
	return (imgtoolerr_t)dsk_image_init(image, f, if_pc99_fm);
}

/*
    Open a file as a ti99_image (PC99 MFM format).
*/
static imgtoolerr_t dsk_image_init_pc99_mfm(imgtool_image *image, imgtool_stream *f)
{
	return (imgtoolerr_t)dsk_image_init(image, f, if_pc99_mfm);
}

/*
    Open a file as a ti99_image (harddisk format).
*/
static imgtoolerr_t win_image_init(imgtool_image *img, imgtool_stream *f)
{
	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(img);
	win_vib_ddr vib;
	int reply;
	int i;

	/* open disk image at level 1 */
	reply = open_image_lvl1(f, if_harddisk, & image->l1_img, NULL);
	if (reply)
		return (imgtoolerr_t)reply;

	/* open disk image at level 2 */
	image->type = L2I_WIN;

	/* read VIB */
	reply = read_absolute_physrec(&image->l1_img, 0, &vib);
	if (reply)
		return (imgtoolerr_t)reply;

	/* guess VIB version */
	image->win.vib_version = memcmp(vib.u.vib_v1.id, "WIN", 3) ? win_vib_v2 : win_vib_v1;

	/* extract AU size and number of AUs */
	image->AUformat.physrecsperAU = ((get_UINT16BE(vib.params) >> 12) & 0xf) + 1;
	image->AUformat.totAUs = get_UINT16BE(vib.totAUs);

	/* extract volume name */
	memcpy(image->vol_name, vib.name, 10);

	/* extract data_offset */
	switch (image->win.vib_version)
	{
	case win_vib_v1:
		image->data_offset = 64;
		break;

	case win_vib_v2:
		image->data_offset = vib.u.vib_v2.res_AUs * 64;
		break;
	}

	/* read allocation bitmap (aphysrecs 1 through n, n<=33) */
	for (i=0; i < (image->AUformat.totAUs+2047)/2048; i++)
	{
		reply = read_absolute_physrec(&image->l1_img, i+1, image->abm+i*256);
		if (reply)
			return (imgtoolerr_t)reply;
	}

	return (imgtoolerr_t)0;
}

/*
    close a ti99_image
*/
static void ti99_image_exit(imgtool_image *img)
{
	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(img);

	close_image_lvl1(&image->l1_img);
}

/*
    get basic information on a ti99_image

    Currently returns the volume name
*/
static void ti99_image_info(imgtool_image *img, char *string, size_t len)
{
	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(img);
	char vol_name[11];

	fname_to_str(vol_name, image->vol_name, 11);

	snprintf(string, len, "%s", vol_name);
}

/*
    Open the disk catalog for enumeration
*/
static imgtoolerr_t dsk_image_beginenum(imgtool_directory *enumeration, const char *path)
{
	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(imgtool_directory_image(enumeration));
	dsk_iterator *iter = (dsk_iterator *) imgtool_directory_extrabytes(enumeration);

	iter->image = image;
	iter->level = 0;
	iter->listing_subdirs = 1;
	iter->index[0] = 0;
	iter->cur_catalog = &iter->image->dsk.catalogs[0];

	return (imgtoolerr_t)0;
}

/*
    Enumerate disk catalog next entry
*/
static imgtoolerr_t dsk_image_nextenum(imgtool_directory *enumeration, imgtool_dirent *ent)
{
	dsk_iterator *iter = (dsk_iterator*) imgtool_directory_extrabytes(enumeration);
	dsk_fdr fdr;
	int reply;
	unsigned fdr_aphysrec;


	ent->corrupt = 0;
	ent->eof = 0;

	/* iterate through catalogs to next file or dir entry */
	while ((iter->level >= 0)
			&& (iter->index[iter->level] >= (iter->listing_subdirs
												? iter->cur_catalog->num_subdirs
												: iter->cur_catalog->num_files)))
	{
		if (iter->listing_subdirs)
		{
			iter->listing_subdirs = 0;
			iter->index[iter->level] = 0;
		}
		else
		{
			iter->listing_subdirs = 1;
			if (! iter->level)
				iter->level = -1;
			else
			{
				iter->level = 0;
				iter->index[0]++;
				iter->cur_catalog = &iter->image->dsk.catalogs[0];
			}
		}
	}

	if (iter->level < 0)
	{
		ent->eof = 1;
	}
	else
	{
		if (iter->listing_subdirs)
		{
			fname_to_str(ent->filename, iter->image->dsk.catalogs[0].subdirs[iter->index[iter->level]].name, ARRAY_LENGTH(ent->filename));

			/* set type of DIR */
			snprintf(ent->attr, ARRAY_LENGTH(ent->attr), "DIR");

			/* len in physrecs */
			/* @BN@ return length in bytes */
			/* ent->filesize = 1; */
			ent->filesize = 256;

			/* recurse subdirectory */
			iter->listing_subdirs = 0;  /* no need to list subdirs as only the
                                        root dir has subdirs in DSK format */
			iter->level = 1;
			iter->cur_catalog = &iter->image->dsk.catalogs[iter->index[0]+1];
			iter->index[iter->level] = 0;
		}
		else
		{
			fdr_aphysrec = iter->cur_catalog->files[iter->index[iter->level]].fdr_ptr;
			reply = read_absolute_physrec(& iter->image->l1_img, fdr_aphysrec, & fdr);
			if (reply)
				return IMGTOOLERR_READERROR;
#if 0
			fname_to_str(ent->filename, fdr.name, ARRAY_LENGTH(ent->filename));
#else
			{
				char buf[11];

				ent->filename[0] = '\0';
				if (iter->level)
				{
					fname_to_str(ent->filename, iter->image->dsk.catalogs[0].subdirs[iter->index[0]].name, ARRAY_LENGTH(ent->filename));
					strncat(ent->filename, ".", ARRAY_LENGTH(ent->filename) - 1);
				}
				fname_to_str(buf, fdr.name, 11);
				strncat(ent->filename, buf, ARRAY_LENGTH(ent->filename) - 1);
			}
#endif
			/* parse flags */
			if (fdr.flags & fdr99_f_program)
				snprintf(ent->attr, ARRAY_LENGTH(ent->attr), "PGM%s",
							(fdr.flags & fdr99_f_wp) ? " R/O" : "");
			else
				snprintf(ent->attr, ARRAY_LENGTH(ent->attr), "%c/%c %d%s",
							(fdr.flags & fdr99_f_int) ? 'I' : 'D',
							(fdr.flags & fdr99_f_var) ? 'V' : 'F',
							fdr.reclen,
							(fdr.flags & fdr99_f_wp) ? " R/O" : "");
			/* len in physrecs */
			/* @BN@ return length in bytes */
			/* ent->filesize = get_UINT16BE(fdr.fphysrecs); */
			ent->filesize = (get_UINT16BE(fdr.fphysrecs)+1)*256;

			iter->index[iter->level]++;
		}
	}

	return (imgtoolerr_t)0;
}

/*
    Open the disk catalog for enumeration
*/
static imgtoolerr_t win_image_beginenum(imgtool_directory *enumeration, const char *path)
{
	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(imgtool_directory_image(enumeration));
	win_iterator *iter = (win_iterator *) imgtool_directory_extrabytes(enumeration);
	imgtoolerr_t errorcode;

	iter->image = image;
	iter->level = 0;
	iter->listing_subdirs = 1;
	iter->index[0] = 0;
	errorcode = (imgtoolerr_t)win_read_catalog(image, 0, &iter->catalog[0]);
	if (errorcode)
		return errorcode;

	return (imgtoolerr_t)0;
}

/*
    Enumerate disk catalog next entry
*/
static imgtoolerr_t win_image_nextenum(imgtool_directory *enumeration, imgtool_dirent *ent)
{
	win_iterator *iter = (win_iterator *) imgtool_directory_extrabytes(enumeration);
	unsigned fdr_aphysrec;
	win_fdr fdr;
	int reply;
	int i;


	ent->corrupt = 0;
	ent->eof = 0;

	/* iterate through catalogs to next file or dir entry */
	while ((iter->level >= 0)
			&& (iter->index[iter->level] >= (iter->listing_subdirs
												? iter->catalog[iter->level].num_subdirs
												: iter->catalog[iter->level].num_files)))
	{
		if (iter->listing_subdirs)
		{
			iter->listing_subdirs = 0;
			iter->index[iter->level] = 0;
		}
		else
		{
			iter->listing_subdirs = 1;
			iter->level--;
			iter->index[iter->level]++;
		}
	}

	if (iter->level < 0)
	{
		ent->eof = 1;
	}
	else
	{
		if (iter->listing_subdirs)
		{
#if 0
			fname_to_str(ent->filename, iter->catalog[iter->level].subdirs[iter->index[iter->level]].name, ARRAY_LENGTH(ent->filename));
#else
			{
				char buf[11];

				ent->filename[0] = '\0';
				for (i=0; i<iter->level; i++)
				{
					fname_to_str(buf, iter->catalog[i].subdirs[iter->index[i]].name, 11);
					strncat(ent->filename, buf, ARRAY_LENGTH(ent->filename) - 1);
					strncat(ent->filename, ".", ARRAY_LENGTH(ent->filename) - 1);
				}
				fname_to_str(buf, iter->catalog[iter->level].subdirs[iter->index[iter->level]].name, 11);
				strncat(ent->filename, buf, ARRAY_LENGTH(ent->filename) - 1);
			}
#endif

			/* set type of DIR */
			snprintf(ent->attr, ARRAY_LENGTH(ent->attr), "DIR");

			/* len in physrecs */
			/* @BN@ return length in bytes */
			/* ent->filesize = 2; */
			ent->filesize = 512;

			/* recurse subdirectory */
			/*iter->listing_subdirs = 1;*/
			iter->level++;
			iter->index[iter->level] = 0;
			reply = win_read_catalog(iter->image, iter->catalog[iter->level-1].subdirs[iter->index[iter->level-1]].dir_ptr, &iter->catalog[iter->level]);
			if (reply)
			{
				ent->corrupt = 1;
				return (imgtoolerr_t)reply;
			}
		}
		else
		{
			fdr_aphysrec = iter->catalog[iter->level].files[iter->index[iter->level]].fdr_ptr*iter->image->AUformat.physrecsperAU;
			reply = read_absolute_physrec(& iter->image->l1_img, fdr_aphysrec, & fdr);
			if (reply)
				return IMGTOOLERR_READERROR;
#if 0
			fname_to_str(ent->filename, iter->catalog[iter->level].files[iter->index[iter->level]].name, ARRAY_LENGTH(ent->filename));
#else
			{
				char buf[11];

				ent->filename[0] = '\0';
				for (i=0; i<iter->level; i++)
				{
					fname_to_str(buf, iter->catalog[i].subdirs[iter->index[i]].name, 11);
					strncat(ent->filename, buf, ARRAY_LENGTH(ent->filename) - 1);
					strncat(ent->filename, ".", ARRAY_LENGTH(ent->filename) - 1);
				}
				fname_to_str(buf, iter->catalog[iter->level].files[iter->index[iter->level]].name, 11);
				strncat(ent->filename, buf, ARRAY_LENGTH(ent->filename) - 1);
			}
#endif
			/* parse flags */
			if (fdr.flags & fdr99_f_program)
				snprintf(ent->attr, ARRAY_LENGTH(ent->attr), "PGM%s",
							(fdr.flags & fdr99_f_wp) ? " R/O" : "");
			else
				snprintf(ent->attr, ARRAY_LENGTH(ent->attr), "%c/%c %d%s",
							(fdr.flags & fdr99_f_int) ? 'I' : 'D',
							(fdr.flags & fdr99_f_var) ? 'V' : 'F',
							fdr.reclen,
							(fdr.flags & fdr99_f_wp) ? " R/O" : "");
			/* len in physrecs */
			/* @BN@ return length in bytes */
			/* ent->filesize = get_win_fdr_fphysrecs(&fdr); */
			ent->filesize = (get_win_fdr_fphysrecs(&fdr)+1)*256;

			iter->index[iter->level]++;
		}
	}

	return (imgtoolerr_t)0;
}

/*
    Compute free space on disk image (in AUs)
*/
static imgtoolerr_t ti99_image_freespace(imgtool_partition *partition, UINT64 *size)
{
	imgtool_image *img = imgtool_partition_image(partition);
	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(img);
	size_t freeAUs;
	int i;

	freeAUs = 0;
	for (i=0; i<image->AUformat.totAUs; i++)
	{
		if (! (image->abm[i >> 3] & (1 << (i & 7))))
			freeAUs++;
	}

	/* @BN@ return free space in bytes */
	/*    *size = freeAUs; */
	*size = freeAUs*256;

	return IMGTOOLERR_SUCCESS;
}

/*
    Extract a file from a ti99_image.  The file is saved in tifile format.
*/
static imgtoolerr_t ti99_image_readfile(imgtool_partition *partition, const char *fpath, const char *fork, imgtool_stream *destf)
{
	imgtool_image *img = imgtool_partition_image(partition);
#if 1

	/* extract data as TIFILES */
	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(img);
	ti99_lvl2_fileref src_file;
	ti99_lvl2_fileref dst_file;
	ti99_date_time date_time;
	int fphysrecs;
	int i;
	UINT8 buf[256];
	imgtoolerr_t errorcode;


	if (check_fpath(fpath))
		return IMGTOOLERR_BADFILENAME;

	/* open file on TI image */
	switch (image->type)
	{
	case L2I_DSK:
		errorcode = (imgtoolerr_t)open_file_lvl2_dsk(image, fpath, &src_file);
		break;

	case L2I_WIN:
		errorcode = (imgtoolerr_t)open_file_lvl2_win(image, fpath, &src_file);
		break;

	default:
		errorcode = (imgtoolerr_t)-1;
		break;
	}
	if (errorcode)
		return errorcode;

	errorcode = (imgtoolerr_t)new_file_lvl2_tifiles(destf, &dst_file);
	if (errorcode)
		return errorcode;

	/* write TIFILE header */
	/* set up parameters */
	set_file_flags(&dst_file, get_file_flags(&src_file));
	set_file_recsperphysrec(&dst_file, get_file_recsperphysrec(&src_file));
	errorcode = (imgtoolerr_t)set_file_fphysrecs(&dst_file, get_file_fphysrecs(&src_file));
	if (errorcode)
		return errorcode;
	set_file_eof(&dst_file, get_file_eof(&src_file));
	errorcode = (imgtoolerr_t)set_file_reclen(&dst_file, get_file_reclen(&src_file));
	if (errorcode)
		return errorcode;
	errorcode = (imgtoolerr_t)set_file_fixrecs(&dst_file, get_file_fixrecs(&src_file));
	if (errorcode)
		return errorcode;

	get_file_creation_date(&src_file, &date_time);
#if 0
	if ((date_time.time_MSB == 0) || (date_time.time_LSB == 0)
			|| (date_time.date_MSB == 0) || (date_time.date_LSB == 0))
		current_date_time(&date_time);
#endif
	set_file_creation_date(&dst_file, date_time);

	get_file_update_date(&src_file, &date_time);
#if 0
	if ((date_time.time_MSB == 0) || (date_time.time_LSB == 0)
			|| (date_time.date_MSB == 0) || (date_time.date_LSB == 0))
		current_date_time(&date_time);
#endif
	set_file_update_date(&dst_file, date_time);

	if (stream_write(destf, & dst_file.tifiles.hdr, 128) != 128)
		return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;

	/* copy data to TIFILE */
	fphysrecs = get_file_fphysrecs(&src_file);

	for (i=0; i<fphysrecs; i++)
	{
		errorcode = (imgtoolerr_t)read_file_physrec(& src_file, i, buf);
		if (errorcode)
			return errorcode;

		errorcode = (imgtoolerr_t)write_file_physrec(& dst_file, i, buf);
		if (errorcode)
			return errorcode;
	}

	return (imgtoolerr_t)0;

#else

	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(img);
	ti99_lvl3_fileref src_file;
	UINT8 buf[256];
	int reclen;
	char lineend = '\r';
	int errorcode;


	if (check_fpath(fpath))
		return IMGTOOLERR_BADFILENAME;

	/* open file on TI image */
	switch (image->type)
	{
	case L2I_DSK:
		errorcode = open_file_lvl2_dsk(image, fpath, &src_file.l2_file);
		break;

	case L2I_WIN:
		errorcode = open_file_lvl2_win(image, fpath, &src_file.l2_file);
		break;
	}
	if (errorcode)
		return errorcode;

	errorcode = open_file_lvl3(& src_file);
	if (errorcode)
		return errorcode;

	/* write text data */
	while (! is_eof(& src_file))
	{
		errorcode = read_next_record(& src_file, buf, & reclen);
		if (errorcode)
			return errorcode;
		if (stream_write(destf, buf, reclen) != reclen)
			return IMGTOOLERR_WRITEERROR;
		if (stream_write(destf, &lineend, 1) != 1)
			return IMGTOOLERR_WRITEERROR;
	}

	return 0;

#endif
}

/*
    Add a file to a ti99_image.  The file must be in tifile format.
*/
static imgtoolerr_t ti99_image_writefile(imgtool_partition *partition, const char *fpath, const char *fork, imgtool_stream *sourcef, option_resolution *writeoptions)
{
	imgtool_image *img = imgtool_partition_image(partition);
	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(img);
	const char *filename;
	char ti_fname[10];
	ti99_lvl2_fileref src_file;
	ti99_lvl2_fileref dst_file;
	ti99_date_time date_time;
	int i;
	int fphysrecs;
	UINT8 buf[256];
	imgtoolerr_t errorcode;
	int parent_ref_valid, parent_ref = 0;
	ti99_catalog *catalog, catalog_buf = {0, };


	(void) writeoptions;

	if (check_fpath(fpath))
		return IMGTOOLERR_BADFILENAME;

	switch (image->type)
	{
	case L2I_DSK:
		errorcode = (imgtoolerr_t)dsk_find_catalog_entry(image, fpath, &parent_ref_valid, &parent_ref, NULL, NULL);
		if (errorcode == 0)
			/* file already exists: causes an error for now */
			return (imgtoolerr_t)IMGTOOLERR_UNEXPECTED;
		else if ((! parent_ref_valid) || (errorcode != IMGTOOLERR_FILENOTFOUND))
			return (imgtoolerr_t)errorcode;
		break;

	case L2I_WIN:
		errorcode = (imgtoolerr_t)win_find_catalog_entry(image, fpath, &parent_ref_valid, &parent_ref, &catalog_buf, NULL, NULL);
		if (errorcode == 0)
			/* file already exists: causes an error for now */
			return (imgtoolerr_t)IMGTOOLERR_UNEXPECTED;
		else if ((! parent_ref_valid) || (errorcode != IMGTOOLERR_FILENOTFOUND))
			return (imgtoolerr_t)errorcode;
		break;
	}

	errorcode =(imgtoolerr_t) open_file_lvl2_tifiles(sourcef, & src_file);
	if (errorcode)
		return (imgtoolerr_t)errorcode;

	/* create new file */
	filename = strrchr(fpath, '.');
	if (filename)
		filename++;
	else
		filename = fpath;
	str_to_fname(ti_fname, filename);
	switch (image->type)
	{
	case L2I_DSK:
		errorcode = (imgtoolerr_t)new_file_lvl2_dsk(image, parent_ref, ti_fname, &dst_file);
		if (errorcode)
			return (imgtoolerr_t)errorcode;
		break;

	case L2I_WIN:
		errorcode = (imgtoolerr_t)new_file_lvl2_win(image, &catalog_buf, ti_fname, &dst_file);
		if (errorcode)
			return (imgtoolerr_t)errorcode;
		break;
	}

	/* set up parameters */
	set_file_flags(&dst_file, get_file_flags(&src_file));
	set_file_recsperphysrec(&dst_file, get_file_recsperphysrec(&src_file));
	errorcode = (imgtoolerr_t)set_file_fphysrecs(&dst_file, /*get_file_fphysrecs(&src_file)*/0);
	if (errorcode)
		return (imgtoolerr_t)errorcode;
	set_file_eof(&dst_file, get_file_eof(&src_file));
	errorcode = (imgtoolerr_t)set_file_reclen(&dst_file, get_file_reclen(&src_file));
	if (errorcode)
		return (imgtoolerr_t)errorcode;
	errorcode = (imgtoolerr_t)set_file_fixrecs(&dst_file, get_file_fixrecs(&src_file));
	if (errorcode)
		return (imgtoolerr_t)errorcode;

	get_file_creation_date(&src_file, &date_time);
#if 0
	if ((date_time.time_MSB == 0) || (date_time.time_LSB == 0)
			|| (date_time.date_MSB == 0) || (date_time.date_LSB == 0))
		current_date_time(&date_time);
#endif
	set_file_creation_date(&dst_file, date_time);

	get_file_update_date(&src_file, &date_time);
#if 0
	if ((date_time.time_MSB == 0) || (date_time.time_LSB == 0)
			|| (date_time.date_MSB == 0) || (date_time.date_LSB == 0))
		current_date_time(&date_time);
#endif
	set_file_update_date(&dst_file, date_time);

	/* alloc data physrecs */
	fphysrecs = get_file_fphysrecs(&src_file);
	switch (dst_file.type)
	{
	case L2F_DSK:
		errorcode = (imgtoolerr_t)dsk_alloc_file_physrecs(&dst_file.dsk, fphysrecs);
		if (errorcode)
			return (imgtoolerr_t)errorcode;
		break;

	case L2F_WIN:
		errorcode = (imgtoolerr_t)win_alloc_file_physrecs(&dst_file.win, fphysrecs);
		if (errorcode)
			return (imgtoolerr_t)errorcode;
		break;

	case L2F_TIFILES:
		break;
	}

	/* copy data */
	for (i=0; i<fphysrecs; i++)
	{
		if (stream_read(sourcef, buf, 256) != 256)
			return (imgtoolerr_t)IMGTOOLERR_READERROR;

		errorcode = (imgtoolerr_t)write_file_physrec(& dst_file, i, buf);
		if (errorcode)
			return (imgtoolerr_t)errorcode;
	}

	/* write fdr */
	switch (image->type)
	{
	case L2I_DSK:
		if (write_absolute_physrec(& image->l1_img, dst_file.dsk.fdr_aphysrec, &dst_file.dsk.fdr))
			return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
		break;

	case L2I_WIN:
		/* save fphysrecs field as well */
		if (dst_file.win.curfdr_aphysrec == dst_file.win.eldestfdr_aphysrec)
			set_win_fdr_fphysrecs(&dst_file.win.curfdr, dst_file.win.fphysrecs);
		if (write_absolute_physrec(& image->l1_img, dst_file.win.curfdr_aphysrec, &dst_file.win.curfdr))
			return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
		if (dst_file.win.curfdr_aphysrec != dst_file.win.eldestfdr_aphysrec)
		{
			dst_file.win.curfdr_aphysrec = dst_file.win.eldestfdr_aphysrec;
			if (read_absolute_physrec(& image->l1_img, dst_file.win.curfdr_aphysrec, &dst_file.win.curfdr))
				return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
			set_win_fdr_fphysrecs(&dst_file.win.curfdr, dst_file.win.fphysrecs);
			if (write_absolute_physrec(& image->l1_img, dst_file.win.curfdr_aphysrec, &dst_file.win.curfdr))
				return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
		}
		break;
	}

	/* update catalog */
	switch (image->type)
	{
	case L2I_DSK:
		catalog = &image->dsk.catalogs[parent_ref];
		for (i=0; i<128; i++)
		{
			buf[2*i] = catalog->files[i].fdr_ptr >> 8;
			buf[2*i+1] = catalog->files[i].fdr_ptr & 0xff;
		}
		if (write_absolute_physrec(& image->l1_img, image->dsk.fdir_aphysrec[parent_ref], buf))
			return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
		break;

	case L2I_WIN:
		{
			win_vib_ddr parent_ddr;
			int fdir_AU;

			/* update VIB/DDR and get FDIR AU address */
			if (read_absolute_physrec(& image->l1_img, parent_ref * image->AUformat.physrecsperAU, &parent_ddr))
				return (imgtoolerr_t)IMGTOOLERR_READERROR;
			parent_ddr.num_files = catalog_buf.num_files;
			if (write_absolute_physrec(& image->l1_img, parent_ref * image->AUformat.physrecsperAU, &parent_ddr))
				return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
			fdir_AU = get_UINT16BE(parent_ddr.fdir_AU);

			/* generate FDIR and save it */
			for (i=0; i<127; i++)
			{
				buf[2*i] = catalog_buf.files[i].fdr_ptr >> 8;
				buf[2*i+1] = catalog_buf.files[i].fdr_ptr & 0xff;
			}
			buf[254] = parent_ref >> 8;
			buf[255] = parent_ref & 0xff;
			if (write_absolute_physrec(& image->l1_img, fdir_AU * image->AUformat.physrecsperAU, buf))
				return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
		}
		break;
	}

	/* update bitmap */
	switch (image->type)
	{
	case L2I_DSK:
		{
			dsk_vib vib;

			if (read_absolute_physrec(& image->l1_img, 0, &vib))
				return (imgtoolerr_t)IMGTOOLERR_READERROR;
			memcpy(vib.abm, image->abm, 200);
			if (write_absolute_physrec(& image->l1_img, 0, &vib))
				return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
		}
		break;

	case L2I_WIN:
		/* save allocation bitmap (aphysrecs 1 through n, n<=33) */
		for (i=0; i < (image->AUformat.totAUs+2047)/2048; i++)
			if (write_absolute_physrec(&image->l1_img, i+1, image->abm+i*256))
				return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
		break;
	}

	return (imgtoolerr_t)0;
}

/*
    Delete a file from a ti99_image.
*/
static imgtoolerr_t dsk_image_deletefile(imgtool_partition *partition, const char *fpath)
{
	imgtool_image *img = imgtool_partition_image(partition);
	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(img);
	dsk_fdr fdr;
	int i, cluster_index;
	unsigned cur_AU, cluster_lastfphysrec;
//  int fphysrecs;
	int parent_ref, is_dir, catalog_index;
	imgtoolerr_t errorcode;
	UINT8 buf[256];
	ti99_catalog *catalog;


	if (check_fpath(fpath))
		return IMGTOOLERR_BADFILENAME;

	errorcode = (imgtoolerr_t)dsk_find_catalog_entry(image, fpath, NULL, &parent_ref, &is_dir, &catalog_index);
	if (errorcode)
		return errorcode;

	if (is_dir)
	{
		catalog = &image->dsk.catalogs[catalog_index+1];

		if ((catalog->num_files != 0) || (catalog->num_subdirs != 0))
			return IMGTOOLERR_UNIMPLEMENTED;

		catalog = &image->dsk.catalogs[0];

		/* free fdir AU */
		cur_AU = image->dsk.fdir_aphysrec[catalog_index+1] / image->AUformat.physrecsperAU;
		image->abm[cur_AU >> 3] &= ~ (1 << (cur_AU & 7));

		/* delete catalog entry */
		for (i=catalog_index; i<2; i++)
		{
			catalog->subdirs[i] = catalog->subdirs[i+1];
			image->dsk.fdir_aphysrec[i+1] = image->dsk.fdir_aphysrec[i+2];
		}
		memset(catalog->subdirs[2].name, 0, 10);
		catalog->subdirs[2].dir_ptr = 0;
		image->dsk.fdir_aphysrec[3] = 0;
		catalog->num_subdirs--;

		/* update directory and bitmap in vib */
		{
			dsk_vib vib;

			if (read_absolute_physrec(& image->l1_img, 0, &vib))
				return IMGTOOLERR_READERROR;

			for (i=0; i<3; i++)
			{
				memcpy(vib.subdir[i].name, catalog->subdirs[i].name, 10);
				set_UINT16BE(&vib.subdir[i].fdir_aphysrec, image->dsk.fdir_aphysrec[i+1]);
			}

			memcpy(vib.abm, image->abm, 200);

			if (write_absolute_physrec(& image->l1_img, 0, &vib))
				return IMGTOOLERR_WRITEERROR;
		}
	}
	else
	{
		catalog = &image->dsk.catalogs[parent_ref];

		if (read_absolute_physrec(& image->l1_img, catalog->files[catalog_index].fdr_ptr, &fdr))
			return IMGTOOLERR_READERROR;

		/* free data AUs */
//      fphysrecs =
			get_UINT16BE(fdr.fphysrecs);

		i = 0;
		cluster_index = 0;
		while (cluster_index < 76)
		{
			cur_AU = get_dsk_fdr_cluster_baseAU(image, & fdr, cluster_index);
			if (cur_AU == 0)
				/* end of cluster list */
				break;
			cluster_lastfphysrec = get_dsk_fdr_cluster_lastfphysrec(& fdr, cluster_index);

			while (i<=cluster_lastfphysrec)
			{
				/* the condition below is an error, but it should not prevent
				us from deleting the file */
				if (cur_AU >= image->AUformat.totAUs)
					/*return IMGTOOLERR_CORRUPTIMAGE;*/
					break;

				image->abm[cur_AU >> 3] &= ~ (1 << (cur_AU & 7));

				i += image->AUformat.physrecsperAU;
				cur_AU++;
			}

			cluster_index++;
		}
		/* the condition below is an error, but it should not prevent us from
		deleting the file */
#if 0
		if (i<fphysrecs)
			return IMGTOOLERR_CORRUPTIMAGE;
#endif

		/* free fdr AU */
		cur_AU = catalog->files[catalog_index].fdr_ptr / image->AUformat.physrecsperAU;
		image->abm[cur_AU >> 3] &= ~ (1 << (cur_AU & 7));

		/* delete catalog entry */
		for (i=catalog_index; i<127; i++)
			catalog->files[i] = catalog->files[i+1];
		catalog->files[127].fdr_ptr = 0;
		catalog->num_files--;

		/* update parent fdir */
		for (i=0; i<128; i++)
		{
			buf[2*i] = catalog->files[i].fdr_ptr >> 8;
			buf[2*i+1] = catalog->files[i].fdr_ptr & 0xff;
		}
		if (write_absolute_physrec(& image->l1_img, image->dsk.fdir_aphysrec[parent_ref], buf))
			return IMGTOOLERR_WRITEERROR;

		/* update bitmap */
		{
			dsk_vib vib;

			if (read_absolute_physrec(& image->l1_img, 0, &vib))
				return IMGTOOLERR_READERROR;
			memcpy(vib.abm, image->abm, 200);
			if (write_absolute_physrec(& image->l1_img, 0, &vib))
				return IMGTOOLERR_WRITEERROR;
		}
	}

	return (imgtoolerr_t)0;
}

static imgtoolerr_t win_image_deletefile(imgtool_partition *partition, const char *fpath)
{
	imgtool_image *img = imgtool_partition_image(partition);
	struct ti99_lvl2_imgref *image = (struct ti99_lvl2_imgref *) imgtool_image_extra_bytes(img);
	int parent_ddr_AU, is_dir, catalog_index;
	win_fdr fdr;
	int i;
	unsigned cur_AU, end_AU;
	unsigned curfdr_aphysrec;
	unsigned cursibFDR_index, endsibFDR_index = 0;
	int errorcode;
	UINT8 buf[256];
	ti99_catalog catalog;


	if (check_fpath(fpath))
		return (imgtoolerr_t)IMGTOOLERR_BADFILENAME;

	errorcode = win_find_catalog_entry(image, fpath, NULL, &parent_ddr_AU, &catalog, &is_dir, &catalog_index);
	if (errorcode)
		return (imgtoolerr_t)errorcode;

	if (is_dir)
	{
		unsigned DDR_AU;
		win_vib_ddr ddr_buf;

		/* Read DDR record */
		DDR_AU = catalog.subdirs[catalog_index].dir_ptr;
		if (read_absolute_physrec(& image->l1_img, DDR_AU*image->AUformat.physrecsperAU, &ddr_buf))
			return (imgtoolerr_t)IMGTOOLERR_READERROR;

		/* read FDIR AU pointer */
		cur_AU = get_UINT16BE(ddr_buf.fdir_AU);

		/* sanity checks */
		if ((ddr_buf.num_files > 127) || (ddr_buf.num_subdirs > 114) || (cur_AU > image->AUformat.totAUs))
			return (imgtoolerr_t)IMGTOOLERR_CORRUPTIMAGE;

		if ((ddr_buf.num_files != 0) || (ddr_buf.num_subdirs != 0))
			return (imgtoolerr_t)IMGTOOLERR_UNIMPLEMENTED;

		/* free fdir AU */
		image->abm[cur_AU >> 3] &= ~ (1 << (cur_AU & 7));

		/* free ddr AU */
		image->abm[DDR_AU >> 3] &= ~ (1 << (DDR_AU & 7));

		/* delete parent catalog entry */
		for (i=catalog_index; i<113; i++)
			catalog.subdirs[i] = catalog.subdirs[i+1];
		catalog.subdirs[113].dir_ptr = 0;
		catalog.num_subdirs--;

		/* update parent VIB/DDR */
		if (read_absolute_physrec(& image->l1_img, parent_ddr_AU * image->AUformat.physrecsperAU, &ddr_buf))
			return (imgtoolerr_t)IMGTOOLERR_READERROR;
		ddr_buf.num_subdirs = catalog.num_subdirs;
		for (i=0; i<114; i++)
			set_UINT16BE(&ddr_buf.subdir_AU[i], catalog.subdirs[i].dir_ptr);
		if (write_absolute_physrec(& image->l1_img, parent_ddr_AU * image->AUformat.physrecsperAU, &ddr_buf))
			return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;

		/* save allocation bitmap (aphysrecs 1 through n, n<=33) */
		for (i=0; i < (image->AUformat.totAUs+2047)/2048; i++)
			if (write_absolute_physrec(&image->l1_img, i+1, image->abm+i*256))
				return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
	}
	else
	{
		/* check integrity of FDR sibling chain, and go to last sibling */
		/* note that as we check that the back chain is consistent with the forward
		chain, we will also detect any cycle in the sibling chain, so we do not
		need to check against them explicitely */
		{
			int pastendoflist_flag;
			unsigned prevfdr_aphysrec;


			pastendoflist_flag = 0;
			cursibFDR_index = 0;
			curfdr_aphysrec = catalog.files[catalog_index].fdr_ptr * image->AUformat.physrecsperAU;
			if (read_absolute_physrec(& image->l1_img, curfdr_aphysrec, &fdr))
				return (imgtoolerr_t)IMGTOOLERR_READERROR;

			if (get_UINT16BE(fdr.prevsibFDR_AU) != 0)
				return (imgtoolerr_t)IMGTOOLERR_CORRUPTIMAGE;

			while (1)
			{
				if (! pastendoflist_flag)
				{
					/* look for end of list */
					for (i=0; i<54; i++)
					{
						if (get_UINT16BE(fdr.clusters[i][0]) == 0)
						{
							endsibFDR_index = cursibFDR_index;
							pastendoflist_flag = TRUE;
							break;
						}
					}
				}

				/* exit loop if end of sibling chain */
				if (! get_UINT16BE(fdr.nextsibFDR_AU))
					break;

				/* otherwise read next FDR */
				if (get_UINT16BE(fdr.nextsibFDR_AU) >= image->AUformat.totAUs)
					return (imgtoolerr_t)IMGTOOLERR_CORRUPTIMAGE;

				prevfdr_aphysrec = curfdr_aphysrec;
				curfdr_aphysrec = get_win_fdr_nextsibFDR_aphysrec(image, &fdr);
				if (read_absolute_physrec(& image->l1_img, curfdr_aphysrec, &fdr))
					return (imgtoolerr_t)IMGTOOLERR_READERROR;
				cursibFDR_index++;

				/* check that back chaining is consistent with forward chaining */
				if (get_win_fdr_prevsibFDR_aphysrec(image, &fdr) != prevfdr_aphysrec)
					return (imgtoolerr_t)IMGTOOLERR_CORRUPTIMAGE;
			}
			if (! pastendoflist_flag)
				endsibFDR_index = cursibFDR_index;
		}


		while (1)
		{
			if (cursibFDR_index <= endsibFDR_index)
			{
				for (i = 0; i < 54; i++)
				{
					cur_AU = get_UINT16BE(fdr.clusters[i][0]);
					if (cur_AU == 0)
						/* end of cluster list */
						break;
					end_AU = get_UINT16BE(fdr.clusters[i][1]);

					while (cur_AU<=end_AU)
					{
						/* the condition below is an error, but it should not prevent
						us from deleting the file */
						if (cur_AU >= image->AUformat.totAUs)
							/*return IMGTOOLERR_CORRUPTIMAGE;*/
							break;

						image->abm[cur_AU >> 3] &= ~ (1 << (cur_AU & 7));

						cur_AU++;
					}
				}
			}

			/* free fdr AU if applicable */
			if ((curfdr_aphysrec % image->AUformat.physrecsperAU) == 0)
			{
				cur_AU = curfdr_aphysrec / image->AUformat.physrecsperAU;
				image->abm[cur_AU >> 3] &= ~ (1 << (cur_AU & 7));
			}

			if (cursibFDR_index == 0)
				break;
			curfdr_aphysrec = get_win_fdr_prevsibFDR_aphysrec(image, &fdr);
			if (read_absolute_physrec(& image->l1_img, curfdr_aphysrec, &fdr))
				return (imgtoolerr_t)IMGTOOLERR_READERROR;
			cursibFDR_index--;
		}


		/* delete catalog entry */
		for (i=catalog_index; i<127; i++)
			catalog.files[i] = catalog.files[i+1];
		catalog.files[127].fdr_ptr = 0;
		catalog.num_files--;


		/* update VIB/DDR and get FDIR AU address */
		{
			win_vib_ddr parent_ddr;

			if (read_absolute_physrec(& image->l1_img, parent_ddr_AU * image->AUformat.physrecsperAU, &parent_ddr))
				return (imgtoolerr_t)IMGTOOLERR_READERROR;
			parent_ddr.num_files = catalog.num_files;
			if (write_absolute_physrec(& image->l1_img, parent_ddr_AU * image->AUformat.physrecsperAU, &parent_ddr))
				return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
			cur_AU = get_UINT16BE(parent_ddr.fdir_AU);
		}

		/* generate FDIR and save it */
		for (i=0; i<127; i++)
		{
			buf[2*i] = catalog.files[i].fdr_ptr >> 8;
			buf[2*i+1] = catalog.files[i].fdr_ptr & 0xff;
		}
		buf[254] = parent_ddr_AU >> 8;
		buf[255] = parent_ddr_AU & 0xff;
		if (write_absolute_physrec(& image->l1_img, cur_AU * image->AUformat.physrecsperAU, buf))
			return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;

		/* save allocation bitmap (aphysrecs 1 through n, n<=33) */
		for (i=0; i < (image->AUformat.totAUs+2047)/2048; i++)
			if (write_absolute_physrec(&image->l1_img, i+1, image->abm+i*256))
				return (imgtoolerr_t)IMGTOOLERR_WRITEERROR;
	}

	return (imgtoolerr_t)0;
}

/*
    Create a blank ti99_image (common code).

    Supports MESS and V9T9 formats only
*/
static imgtoolerr_t dsk_image_create(imgtool_image *image, imgtool_stream *f, option_resolution *createoptions, ti99_img_format img_format)
{
	const char *volname;
	int density;
	ti99_lvl1_imgref l1_img;
	int protected_var;

	int totphysrecs, physrecsperAU, totAUs;

	dsk_vib vib;
	UINT8 empty_sec[256];

	int i;

	l1_img.img_format = img_format;
	l1_img.file_handle = f;

	/* read options */
	volname = option_resolution_lookup_string(createoptions, dsk_createopts_volname);
	l1_img.geometry.heads = option_resolution_lookup_int(createoptions, dsk_createopts_sides);
	l1_img.geometry.cylinders = option_resolution_lookup_int(createoptions, dsk_createopts_tracks);
	l1_img.geometry.secspertrack = option_resolution_lookup_int(createoptions, dsk_createopts_sectors);
	protected_var = option_resolution_lookup_int(createoptions, dsk_createopts_protection);
	density = option_resolution_lookup_int(createoptions, dsk_createopts_density);

	/* NPW 03-DEC-2003 - Fixes a crash if volname is NULL */
	if (!volname)
		volname = "";

	totphysrecs = l1_img.geometry.secspertrack * l1_img.geometry.cylinders * l1_img.geometry.heads;
	physrecsperAU = (totphysrecs + 1599) / 1600;
	/* round to next larger power of 2 */
	for (i = 1; i < physrecsperAU; i <<= 1)
		;
	physrecsperAU = i;
	totAUs = totphysrecs / physrecsperAU;

	/* auto-density */
	if (density == 0)
	{
		if (l1_img.geometry.secspertrack <= 9)
			density = 1;
		else if (l1_img.geometry.secspertrack <= 18)
			density = 2;
		else
			density = 3;
	}

	/* check volume name */
	if (strchr(volname, '.') || strchr(volname, ' ') || (strlen(volname) > 10))
		return /*IMGTOOLERR_BADFILENAME*/IMGTOOLERR_PARAMCORRUPT;

	/* FM disks can hold fewer sector per track than MFM disks */
	if ((density == 1) && (l1_img.geometry.secspertrack > 9))
		return IMGTOOLERR_PARAMTOOLARGE;

	/* DD disks can hold fewer sector per track than HD disks */
	if ((density == 2) && (l1_img.geometry.secspertrack > 18))
		return IMGTOOLERR_PARAMTOOLARGE;

	/* check total disk size */
	if (totphysrecs < 2)
		return IMGTOOLERR_PARAMTOOSMALL;

	/* initialize vib in aphysrec 0 */

	/* set volume name */
	str_to_fname(vib.name, volname);

	/* set every header field */
	set_UINT16BE(& vib.totphysrecs, totphysrecs);
	vib.secspertrack = l1_img.geometry.secspertrack;
	vib.id[0] = 'D';
	vib.id[1] = 'S';
	vib.id[2] = 'K';
	vib.protection = protected_var ? 'P' : ' ';
	vib.cylinders = l1_img.geometry.cylinders;
	vib.heads = l1_img.geometry.heads;
	vib.density = density;
	for (i=0; i<3; i++)
	{
		memset(vib.subdir[i].name, '\0', 10);
		set_UINT16BE(& vib.subdir[i].fdir_aphysrec, 0);
	}

	/* clear bitmap */
	for (i=0; i < (totAUs>>3); i++)
		vib.abm[i] = 0;

	if (totAUs & 7)
	{
		vib.abm[i] = 0xff << (totAUs & 7);
		i++;
	}

	for (; i < 200; i++)
		vib.abm[i] = 0xff;

	/* mark first 2 aphysrecs (vib and fdir) as used */
	vib.abm[0] |= (physrecsperAU == 1) ? 3 : 1;

	/* write aphysrec 0 */
	if (write_absolute_physrec(& l1_img, 0, &vib))
		return IMGTOOLERR_WRITEERROR;


	/* now clear every other physrecs, including the FDIR record */
	memset(empty_sec, 0, sizeof(empty_sec));

	for (i=1; i<totphysrecs; i++)
		if (write_absolute_physrec(& l1_img, i, empty_sec))
			return IMGTOOLERR_WRITEERROR;


	return (imgtoolerr_t)dsk_image_init(image, f, img_format);
}

/*
    Create a blank ti99_image (MESS format).
*/
static imgtoolerr_t dsk_image_create_mess(imgtool_image *image, imgtool_stream *f, option_resolution *createoptions)
{
	return dsk_image_create(image, f, createoptions, if_mess);
}

/*
    Create a blank ti99_image (V9T9 format).
*/
static imgtoolerr_t dsk_image_create_v9t9(imgtool_image *image, imgtool_stream *f, option_resolution *createoptions)
{
	return dsk_image_create(image, f, createoptions, if_v9t9);
}
