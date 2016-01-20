// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/****************************************************************************

    mac.c

    Handlers for Classic MacOS images (MFS and HFS formats).

    Raphael Nabet, 2003

    TODO:
    * add support for HFS write

*****************************************************************************

    terminology:
    disk block: 512-byte logical block.  With sectors of 512 bytes, one logical
        block is equivalent to one sector; when the sector size is not 512
        bytes, sectors are split or grouped to make 512-byte disk blocks.
    allocation block: The File Manager always allocates logical disk blocks to
        a file in groups called allocation blocks; an allocation block is
        simply a group of consecutive logical blocks.  The size of a volume's
        allocation blocks depends on the capacity of the volume; there can be
        at most 4094 (MFS) or 65535 (HFS) allocation blocks on a volume.
    MFS (Macintosh File System): File system used by the early Macintosh.  This
        File system does not support folders (you may create folders on a MFS
        disk, but such folders are not implemented on File System level but in
        the Desktop file, and they are just a hint of how programs should list
        files, i.e. you can't have two files with the same name on a volume
        even if they are in two different folders), and it is not adequate for
        large volumes.
    HFS (Hierarchical File System): File system introduced with the HD20
        harddisk, the Macintosh Plus ROMs, and system 3.2 (IIRC).  Contrary to
        MFS, it supports hierarchical folders.  Also, it is suitable for larger
        volumes.
    HFS+ (HFS Plus): New file system introduced with MacOS 8.1.  It has a lot
        in common with HFS, but it supports more allocation blocks (up to 4
        billions IIRC), and many extra features, including longer file names
        (up to 255 UTF-16 Unicode chars).
    tag data: with the GCR encoding, each disk block is associated with a 12
        (3.5" floppies) or 20 (HD20) byte tag record.  This tag record contains
        information on the block allocation status (whether it is allocated
        in a file or free, which file is it belongs to, what offset the block
        has in the file).  This enables to recover all files whose data blocks
        are still on the disk surface even if the disk catalog has been trashed
        completely (though most file properties, like the name, type and
        logical EOF, are not saved in the tag record and cannot be recovered).

    Organization of an MFS volume:

    Logical     Contents                                    Allocation block
    block

    0 - 1:      System startup information
    2 - m:      Master directory block (MDB)
                + allocation block link pointers
    m+1 - n:    Directory file
    n+1 - p-2:  Other files and free space                  0 - ((p-2)-(n+1))/k
    p-1:        Alternate MDB
    p:          Not used
    usually, k = 2, m = 3, n = 16, p = 799 (SSDD 3.5" floppy)
    with DSDD 3.5" floppy, I assume that p = 1599, but I don't know the other
    values


    Master Directory Block:

    Offset  Length  Description
    ------  ------  -----------
         0       2  Volume Signature
         2       4  Creation Date
         6       4  Last Modification Date
        10       2  Volume Attributes
        12       2  Number of Files In Root Directory
        14       2  First Block of Volume Bitmap
       ...

    Links:
        http://developer.apple.com/documentation/mac/Files/Files-99.html
        http://developer.apple.com/documentation/mac/Files/Files-100.html
        http://developer.apple.com/documentation/mac/Files/Files-101.html
        http://developer.apple.com/documentation/mac/Files/Files-102.html
        http://developer.apple.com/documentation/mac/Files/Files-103.html
        http://developer.apple.com/documentation/mac/Files/Files-104.html
        http://developer.apple.com/documentation/mac/Files/Files-105.html
        http://developer.apple.com/documentation/mac/Files/Files-106.html
        http://developer.apple.com/documentation/mac/Devices/Devices-121.html#MARKER-2-169
        http://developer.apple.com/documentation/mac/MoreToolbox/MoreToolbox-99.html

*****************************************************************************/

#include <ctype.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stddef.h>

#include "formats/imageutl.h"
#include "imgtool.h"
#include "macutil.h"
#include "iflopimg.h"
#include "formats/ap_dsk35.h"

/* if 1, check consistency of B-Tree (most of the checks will eventually be
    suppressed when the image is opened as read-only and only enabled when
    the image is opened as read/write) */
#define BTREE_CHECKS 1
/* if 1, check consistency of tag data when we are at risk of corrupting the
    disk (file write and allocation) */
#define TAG_CHECKS 1
/* if 1, check consistency of tag data when reading files (not recommended
    IMHO) */
#define TAG_EXTRA_CHECKS 0

#if 0
#pragma mark MISCELLANEOUS UTILITIES
#endif

struct UINT16BE
{
	UINT8 bytes[2];
};

struct UINT24BE
{
	UINT8 bytes[3];
};

struct UINT32BE
{
	UINT8 bytes[4];
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

#if 0
static inline UINT32 get_UINT24BE(UINT24BE word)
{
	return (word.bytes[0] << 16) | (word.bytes[1] << 8) | word.bytes[2];
}

static inline void set_UINT24BE(UINT24BE *word, UINT32 data)
{
	word->bytes[0] = (data >> 16) & 0xff;
	word->bytes[1] = (data >> 8) & 0xff;
	word->bytes[2] = data & 0xff;
}
#endif

static inline UINT32 get_UINT32BE(UINT32BE word)
{
	return (word.bytes[0] << 24) | (word.bytes[1] << 16) | (word.bytes[2] << 8) | word.bytes[3];
}

static inline void set_UINT32BE(UINT32BE *word, UINT32 data)
{
	word->bytes[0] = (data >> 24) & 0xff;
	word->bytes[1] = (data >> 16) & 0xff;
	word->bytes[2] = (data >> 8) & 0xff;
	word->bytes[3] = data & 0xff;
}

/*
    Macintosh string: first byte is length
*/
typedef UINT8 mac_str27[28];
typedef UINT8 mac_str31[32];
typedef UINT8 mac_str63[64];
typedef UINT8 mac_str255[256];

/*
    Macintosh type/creator code: 4 char value
*/
typedef UINT32BE mac_type;

/*
    point record, with the y and x coordinates
*/
struct mac_point
{
	UINT16BE v;     /* actually signed */
	UINT16BE h;     /* actually signed */
};

/*
    rect record, with the corner coordinates
*/
struct mac_rect
{
	UINT16BE top;   /* actually signed */
	UINT16BE left;  /* actually signed */
	UINT16BE bottom;/* actually signed */
	UINT16BE right; /* actually signed */
};

/*
    FInfo (Finder file info) record
*/
struct mac_FInfo
{
	mac_type  type;         /* file type */
	mac_type  creator;      /* file creator */
	UINT16BE  flags;        /* Finder flags */
	mac_point location;     /* file's location in window */
								/* If set to {0, 0}, and the inited flag is
								clear, the Finder will place the item
								automatically */
	UINT16BE  fldr;         /* MFS: window that contains file */
								/* -3: trash */
								/* -2: desktop */
								/* -1: new folder template?????? */
								/* 0: disk window ("root") */
								/* > 0: specific folder, index of FOBJ resource??? */
								/* The FOBJ resource contains some folder info;
								the name of the resource is the folder name. */
							/* HFS & HFS+:
							    System 7: The window in which the file???s icon appears
							    System 8: reserved (set to 0) */
};

/*
    FXInfo (Finder extended file info) record -- not found in MFS
*/
struct mac_FXInfo
{
	UINT16BE iconID;        /* System 7: An ID number for the file???s icon; the
                                numbers that identify icons are assigned by the
                                Finder */
							/* System 8: Reserved (set to 0) */
	UINT16BE reserved[3];   /* Reserved (set to 0) */
	UINT8    script;        /* System 7: if high-bit is set, the script code
                                for displaying the file name; ignored otherwise */
							/* System 8: Extended flags MSB(?) */
	UINT8    XFlags;        /* Extended flags */
	UINT16BE comment;       /* System 7: Comment ID if high-bit is clear */
							/* System 8: Reserved (set to 0) */
	UINT32BE putAway;       /* Put away folder ID (i.e. if the user moves the
                                file onto the desktop, the directory ID of the
                                folder from which the user moves the file is
                                saved here) */
};

/*
    DInfo (Finder folder info) record -- not found in MFS
*/
struct mac_DInfo
{
	mac_rect rect;          /* Folder's window bounds */
	UINT16BE flags;         /* Finder flags, e.g. kIsInvisible, kNameLocked, etc */
	mac_point location;     /* Location of the folder in parent window */
								/* If set to {0, 0}, and the initied flag is
								clear, the Finder will place the item
								automatically */
	UINT16BE view;          /* System 7: The manner in which folders are
                                displayed */
							/* System 8: reserved (set to 0) */
};

/*
    DXInfo (Finder extended folder info) record -- not found in MFS
*/
struct mac_DXInfo
{
	mac_point scroll;       /* Scroll position */
	UINT32BE openChain;     /* System 7: chain of directory IDs for open folders */
							/* System 8: reserved (set to 0) */
	UINT8    script;        /* System 7: if high-bit is set, the script code
                                for displaying the folder name; ignored otherwise */
							/* System 8: Extended flags MSB(?) */
	UINT8    XFlags;        /* Extended flags */
	UINT16BE comment;       /* System 7: Comment ID if high-bit is clear */
							/* System 8: Reserved (set to 0) */
	UINT32BE putAway;       /* Put away folder ID (i.e. if the user moves the
                                folder onto the desktop, the directory ID of
                                the folder from which the user moves it is
                                saved here) */
};

/*
    defines for FInfo & DInfo flags fields
*/
enum
{
	fif_isOnDesk            = 0x0001,   /* System 6: set if item is located on desktop (files and folders) */
										/* System 7: Unused (set to 0) */
	fif_color               = 0x000E,   /* color coding (files and folders) */
	fif_colorReserved       = 0x0010,   /* System 6: reserved??? */
										/* System 7: Unused (set to 0) */
	fif_requiresSwitchLaunch= 0x0020,   /* System 6: ??? */
										/* System 7: Unused (set to 0) */
	fif_isShared            = 0x0040,   /* Applications files: if set, the */
											/* application can be executed by */
											/* multiple users simultaneously. */
										/* Otherwise, set to 0. */
	fif_hasNoINITs          = 0x0080,   /* Extensions/Control Panels: if set(?), */
											/* this file contains no INIT */
											/* resource */
										/* Otherwise, set to 0. */
	fif_hasBeenInited       = 0x0100,   /* System 6: The Finder has recorded information from
                                            the file???s bundle resource into the desktop
                                            database and given the file or folder a
                                            position on the desktop. */
										/* System 7? 8?: Clear if the file contains desktop database */
											/* resources ('BNDL', 'FREF', 'open', 'kind'...) */
											/* that have not been added yet. Set only by the Finder */
											/* Reserved for folders - make sure this bit is cleared for folders */

	/* bit 0x0200 was at a point (AOCE for system 7.x?) the letter bit for
	AOCE, but was reserved before and it is reserved again in recent MacOS
	releases. */

	fif_hasCustomIcon       = 0x0400,   /* (files and folders) */
	fif_isStationery        = 0x0800,   /* System 7: (files only) */
	fif_nameLocked          = 0x1000,   /* (files and folders) */
	fif_hasBundle           = 0x2000,   /* Files only */
	fif_isInvisible         = 0x4000,   /* (files and folders) */
	fif_isAlias             = 0x8000    /* System 7: (files only) */
};

/*
    mac_to_c_strncpy()

    Encode a macintosh string as a C string.  The NUL character is escaped,
    using the "%00" sequence.  To avoid any ambiguity, '%' is escaped with
    '%25'.

    dst (O): C string
    n (I): length of buffer pointed to by dst
    src (I): macintosh string (first byte is length)
*/
static void mac_to_c_strncpy(char *dst, int n, UINT8 *src)
{
	size_t len = src[0];
	int i, j;

	i = j = 0;
	while ((i < len) && (j < n-1))
	{
		switch (src[i+1])
		{
		case '\0':
			if (j >= n-3)
				goto exit;
			dst[j] = '%';
			dst[j+1] = '0';
			dst[j+2] = '0';
			j += 3;
			break;

		case '%':
			if (j >= n-3)
				goto exit;
			dst[j] = '%';
			dst[j+1] = '2';
			dst[j+2] = '5';
			j += 3;
			break;

		default:
			dst[j] = src[i+1];
			j++;
			break;
		}
		i++;
	}

exit:
	if (n > 0)
		dst[j] = '\0';
}

/*
    c_to_mac_strncpy()

    Decode a C string to a macintosh string.  The NUL character is escaped,
    using the "%00" sequence.  To avoid any ambiguity, '%' is escaped with
    '%25'.

    dst (O): macintosh string (first byte is length)
    src (I): C string
    n (I): length of string pointed to by src
*/
static void c_to_mac_strncpy(mac_str255 dst, const char *src, int n)
{
	size_t len;
	int i;
	char buf[3];


	len = 0;
	i = 0;
	while ((i < n) && (len < 255))
	{
		switch (src[i])
		{
		case '%':
			if (i >= n-2)
				goto exit;
			buf[0] = src[i+1];
			buf[1] = src[i+2];
			buf[2] = '\0';
			dst[len+1] = strtoul(buf, NULL, 16);
			i += 3;
			break;

		default:
			dst[len+1] = src[i];
			i++;
			break;
		}
		len++;
	}

exit:
	dst[0] = len;
}

/*
    mac_strcmp()

    Compare two macintosh strings

    s1 (I): the string to compare
    s2 (I): the comparison string

    Return a zero if s1 and s2 are equal, a negative value if s1 is less than
    s2, and a positive value if s1 is greater than s2.
*/
#ifdef UNUSED_FUNCTION
static int mac_strcmp(const UINT8 *s1, const UINT8 *s2)
{
	size_t common_len;

	common_len = (s1[0] <= s2[0]) ? s1[0] : s2[0];

	return memcmp(s1+1, s2+1, common_len) || ((int)s1[0] - s2[0]);
}
#endif

/*
    mac_stricmp()

    Compare two macintosh strings in a manner compatible with the macintosh HFS
    file system.

    This functions emulates the way HFS (and MFS???) sorts string: this is
    equivalent to the RelString macintosh toolbox call with the caseSensitive
    parameter as false and the diacSensitive parameter as true.

    s1 (I): the string to compare
    s2 (I): the comparison string

    Return a zero if s1 and s2 are equal, a negative value if s1 is less than
    s2, and a positive value if s1 is greater than s2.

Known issues:
    Using this function makes sense with the MacRoman encoding, as it means the
    computer will regard "DeskTop File", "Desktop File", "Desktop file", etc,
    as the same file.  (UNIX users will probably regard this as an heresy, but
    you must consider that, unlike UNIX, the Macintosh was not designed for
    droids, but error-prone human beings that may forget about case.)

    (Also, letters with diatrical signs follow the corresponding letters
    without diacritical signs in the HFS catalog file.  This does not matter,
    though, since the Finder will not display files in the HFS catalog order,
    but it will instead sort files according to whatever order is currently
    selected in the View menu.)

    However, with other text encodings, the behavior will be completely absurd.
    For instance, with the Greek encoding, it will think that the degree symbol
    is the same letter (with different case) as the upper-case Psi, so that if
    a program asks for a file called "90??" on a greek HFS volume, and there is
    a file called "90??" on this volume, file "90??" will be opened.
    Results will probably be even weirder with 2-byte scripts like Japanese or
    Chinese.  Of course, we are not going to fix this issue, since doing so
    would break the compatibility with the original Macintosh OS.
*/

static int mac_stricmp(const UINT8 *s1, const UINT8 *s2)
{
	static const unsigned char mac_char_sort_table[256] =
	{
	/*  \x00    \x01    \x02    \x03    \x04    \x05    \x06    \x07 */
		0x00,   0x01,   0x02,   0x03,   0x04,   0x05,   0x06,   0x07,
	/*  \x08    \x09    \x0a    \x0b    \x0c    \x0d    \x0e    \x0f */
		0x08,   0x09,   0x0a,   0x0b,   0x0c,   0x0d,   0x0e,   0x0f,
	/*  \x10    \x11    \x12    \x13    \x14    \x15    \x16    \x17 */
		0x10,   0x11,   0x12,   0x13,   0x14,   0x15,   0x16,   0x17,
	/*  \x18    \x19    \x1a    \x1b    \x1c    \x1d    \x1e    \x1f */
		0x18,   0x19,   0x1a,   0x1b,   0x1c,   0x1d,   0x1e,   0x1f,
	/*  \x20    \x21    \x22    \x23    \x24    \x25    \x26    \x27 */
		0x20,   0x21,   0x22,   0x27,   0x28,   0x29,   0x2a,   0x2b,
	/*  \x28    \x29    \x2a    \x2b    \x2c    \x2d    \x2e    \x2f */
		0x2e,   0x2f,   0x30,   0x31,   0x32,   0x33,   0x34,   0x35,
	/*  \x30    \x31    \x32    \x33    \x34    \x35    \x36    \x37 */
		0x36,   0x37,   0x38,   0x39,   0x3a,   0x3b,   0x3c,   0x3d,
	/*  \x38    \x39    \x3a    \x3b    \x3c    \x3d    \x3e    \x3f */
		0x3e,   0x3f,   0x40,   0x41,   0x42,   0x43,   0x44,   0x45,
	/*  \x40    \x41    \x42    \x43    \x44    \x45    \x46    \x47 */
		0x46,   0x47,   0x51,   0x52,   0x54,   0x55,   0x5a,   0x5b,
	/*  \x48    \x49    \x4a    \x4b    \x4c    \x4d    \x4e    \x4f */
		0x5c,   0x5d,   0x62,   0x63,   0x64,   0x65,   0x66,   0x68,
	/*  \x50    \x51    \x52    \x53    \x54    \x55    \x56    \x57 */
		0x71,   0x72,   0x73,   0x74,   0x76,   0x77,   0x7c,   0x7d,
	/*  \x58    \x59    \x5a    \x5b    \x5c    \x5d    \x5e    \x5f */
		0x7e,   0x7f,   0x81,   0x82,   0x83,   0x84,   0x85,   0x86,
	/*  \x60    \x61    \x62    \x63    \x64    \x65    \x66    \x67 */
		0x4d,   0x47,   0x51,   0x52,   0x54,   0x55,   0x5a,   0x5b,
	/*  \x68    \x69    \x6a    \x6b    \x6c    \x6d    \x6e    \x6f */
		0x5c,   0x5d,   0x62,   0x63,   0x64,   0x65,   0x66,   0x68,
	/*  \x70    \x71    \x72    \x73    \x74    \x75    \x76    \x77 */
		0x71,   0x72,   0x73,   0x74,   0x76,   0x77,   0x7c,   0x7d,
	/*  \x78    \x79    \x7a    \x7b    \x7c    \x7d    \x7e    \x7f */
		0x7e,   0x7f,   0x81,   0x87,   0x88,   0x89,   0x8a,   0x8b,
	/*  \x80    \x81    \x82    \x83    \x84    \x85    \x86    \x87 */
		0x49,   0x4b,   0x53,   0x56,   0x67,   0x69,   0x78,   0x4e,
	/*  \x88    \x89    \x8a    \x8b    \x8c    \x8d    \x8e    \x8f */
		0x48,   0x4f,   0x49,   0x4a,   0x4b,   0x53,   0x56,   0x57,
	/*  \x90    \x91    \x92    \x93    \x94    \x95    \x96    \x97 */
		0x58,   0x59,   0x5e,   0x5f,   0x60,   0x61,   0x67,   0x6d,
	/*  \x98    \x99    \x9a    \x9b    \x9c    \x9d    \x9e    \x9f */
		0x6e,   0x6f,   0x69,   0x6a,   0x79,   0x7a,   0x7b,   0x78,
	/*  \xa0    \xa1    \xa2    \xa3    \xa4    \xa5    \xa6    \xa7 */
		0x8c,   0x8d,   0x8e,   0x8f,   0x90,   0x91,   0x92,   0x75,
	/*  \xa8    \xa9    \xaa    \xab    \xac    \xad    \xae    \xaf */
		0x93,   0x94,   0x95,   0x96,   0x97,   0x98,   0x4c,   0x6b,
	/*  \xb0    \xb1    \xb2    \xb3    \xb4    \xb5    \xb6    \xb7 */
		0x99,   0x9a,   0x9b,   0x9c,   0x9d,   0x9e,   0x9f,   0xa0,
	/*  \xb8    \xb9    \xba    \xbb    \xbc    \xbd    \xbe    \xbf */
		0xa1,   0xa2,   0xa3,   0x50,   0x70,   0xa4,   0x4c,   0x6b,
	/*  \xc0    \xc1    \xc2    \xc3    \xc4    \xc5    \xc6    \xc7 */
		0xa5,   0xa6,   0xa7,   0xa8,   0xa9,   0xaa,   0xab,   0x25,
	/*  \xc8    \xc9    \xca    \xcb    \xcc    \xcd    \xce    \xcf */
		0x26,   0xac,   0x20,   0x48,   0x4a,   0x6a,   0x6c,   0x6c,
	/*  \xd0    \xd1    \xd2    \xd3    \xd4    \xd5    \xd6    \xd7 */
		0xad,   0xae,   0x23,   0x24,   0x2c,   0x2d,   0xaf,   0xb0,
	/*  \xd8    \xd9    \xda    \xdb    \xdc    \xdd    \xde    \xdf */
		0x80,   0xb1,   0xb2,   0xb3,   0xb4,   0xb5,   0xb6,   0xb7,
	/*  \xe0    \xe1    \xe2    \xe3    \xe4    \xe5    \xe6    \xe7 */
		0xb8,   0xb9,   0xba,   0xbb,   0xbc,   0xbd,   0xbe,   0xbf,
	/*  \xe8    \xe9    \xea    \xeb    \xec    \xed    \xee    \xef */
		0xc0,   0xc1,   0xc2,   0xc3,   0xc4,   0xc5,   0xc6,   0xc7,
	/*  \xf0    \xf1    \xf2    \xf3    \xf4    \xf5    \xf6    \xf7 */
		0xc8,   0xc9,   0xca,   0xcb,   0xcc,   0xcd,   0xce,   0xcf,
	/*  \xf8    \xf9    \xfa    \xfb    \xfc    \xfd    \xfe    \xff */
		0xd0,   0xd1,   0xd2,   0xd3,   0xd4,   0xd5,   0xd6,   0xd7
	};

	size_t common_len;
	int i;
	int c1, c2;

	common_len = (s1[0] <= s2[0]) ? s1[0] : s2[0];

	for (i=0; i<common_len; i++)
	{
		c1 = mac_char_sort_table[s1[i+1]];
		c2 = mac_char_sort_table[s2[i+1]];
		if (c1 != c2)
			return c1 - c2;
	}

	return ((int)s1[0] - s2[0]);
}

/*
    mac_strcpy()

    Copy a macintosh string

    dst (O): dest macintosh string
    src (I): source macintosh string (first byte is length)

    Return a zero if s1 and s2 are equal, a negative value if s1 is less than
    s2, and a positive value if s1 is greater than s2.
*/
static inline void mac_strcpy(UINT8 *dest, const UINT8 *src)
{
	memcpy(dest, src, src[0]+1);
}

#ifdef UNUSED_FUNCTION
/*
    mac_strncpy()

    Copy a macintosh string

    dst (O): dest macintosh string
    n (I): max string length for dest (range 0-255, buffer length + 1)
    src (I): source macintosh string (first byte is length)
*/
static void mac_strncpy(UINT8 *dest, int n, const UINT8 *src)
{
	size_t len;

	len = src[0];
	if (len > n)
		len = n;
	dest[0] = len;
	memcpy(dest+1, src+1, len);
}
#endif

/*
    disk image reference
*/
struct mac_l1_imgref
{
	imgtool_image *image;
	UINT32 heads;
};



static imgtoolerr_t mac_find_block(mac_l1_imgref *image, int block,
	UINT32 *track, UINT32 *head, UINT32 *sector)
{
	*track = 0;
	while(block >= (apple35_sectors_per_track(imgtool_floppy(image->image), *track) * image->heads))
	{
		block -= (apple35_sectors_per_track(imgtool_floppy(image->image), (*track)++) * image->heads);
		if (*track >= 80)
			return IMGTOOLERR_SEEKERROR;
	}

	*head = block / apple35_sectors_per_track(imgtool_floppy(image->image), *track);
	*sector = block % apple35_sectors_per_track(imgtool_floppy(image->image), *track);
	return IMGTOOLERR_SUCCESS;
}



/*
    image_read_block()

    Read one 512-byte block of data from a macintosh disk image

    image (I/O): level-1 image reference
    block (I): address of block to read
    dest (O): buffer where block data should be stored

    Return imgtool error code
*/
static imgtoolerr_t image_read_block(mac_l1_imgref *image, UINT32 block, void *dest)
{
	imgtoolerr_t err;
	floperr_t ferr;
	UINT32 track, head, sector;

	err = mac_find_block(image, block, &track, &head, &sector);
	if (err)
		return err;

	ferr = floppy_read_sector(imgtool_floppy(image->image), head, track, sector, 0, dest, 512);
	if (ferr)
		return imgtool_floppy_error(ferr);

	return IMGTOOLERR_SUCCESS;
}

/*
    image_write_block()

    Read one 512-byte block of data from a macintosh disk image

    image (I/O): level-1 image reference
    block (I): address of block to write
    src (I): buffer with the block data

    Return imgtool error code
*/
static imgtoolerr_t image_write_block(mac_l1_imgref *image, UINT32 block, const void *src)
{
	imgtoolerr_t err;
	floperr_t ferr;
	UINT32 track, head, sector;

	err = mac_find_block(image, block, &track, &head, &sector);
	if (err)
		return err;

	ferr = floppy_write_sector(imgtool_floppy(image->image), head, track, sector, 0, src, 512, 0);  /* TODO: pass ddam argument from imgtool */
	if (ferr)
		return imgtool_floppy_error(ferr);

	return IMGTOOLERR_SUCCESS;
}

/*
    image_get_tag_len()

    Get length of tag data (12 for GCR floppies, 20 for HD20, 0 otherwise)

    image (I/O): level-1 image reference

    Return tag length
*/
static inline int image_get_tag_len(mac_l1_imgref *image)
{
	return 0;
}



/*
    image_read_tag()

    Read a 12- or 20-byte tag record associated with a disk block

    image (I/O): level-1 image reference
    block (I): address of block to read
    dest (O): buffer where tag data should be stored

    Return imgtool error code
*/
static imgtoolerr_t image_read_tag(mac_l1_imgref *image, UINT32 block, void *dest)
{
	return IMGTOOLERR_UNEXPECTED;
}

/*
    image_write_tag()

    Write a 12- or 20-byte tag record associated with a disk block

    image (I/O): level-1 image reference
    block (I): address of block to write
    src (I): buffer with the tag data

    Return imgtool error code
*/
static imgtoolerr_t image_write_tag(mac_l1_imgref *image, UINT32 block, const void *src)
{
	return IMGTOOLERR_UNEXPECTED;
}

#if 0
#pragma mark -
#pragma mark MFS/HFS WRAPPERS
#endif

enum mac_format
{
	L2I_MFS,
	L2I_HFS
};

enum mac_forkID { data_fork = 0x00, rsrc_fork = 0xff };

/*
    MFS image ref
*/
struct mfs_l2_imgref
{
	UINT16 dir_num_files;
	UINT16 dir_start;
	UINT16 dir_blk_len;

	UINT16 ABStart;

	mac_str27 volname;

	unsigned char ABlink_dirty[13]; /* dirty flag for each disk block in the ABlink array */
	UINT8 ABlink[6141];
};

/*
    HFS extent descriptor
*/
struct hfs_extent
{
	UINT16BE stABN;         /* first allocation block */
	UINT16BE numABlks;      /* number of allocation blocks */
};

/*
    HFS likes to group extents by 3 (it is 8 with HFS+), so we create a
    specific type.
*/
typedef hfs_extent hfs_extent_3[3];

/*
    MFS open file ref
*/
struct mfs_fileref
{
	UINT16 stBlk;                   /* first allocation block of file */
};

/*
    HFS open file ref
*/
struct hfs_fileref
{
	hfs_extent_3 extents;           /* first 3 file extents */

	UINT32 parID;                   /* CNID of parent directory (undefined for extent & catalog files) */
	mac_str31 filename;             /* file name (undefined for extent & catalog files) */
};

struct mac_l2_imgref;

/*
    MFS/HFS open file ref
*/
struct mac_fileref
{
	struct mac_l2_imgref *l2_img;   /* image pointer */

	UINT32 fileID;                  /* file ID (a.k.a. CNID in HFS/HFS+) */

	mac_forkID forkType;            /* 0x00 for data, 0xff for resource */

	UINT32 eof;                     /* logical end-of-file */
	UINT32 pLen;                    /* physical end-of-file */

	UINT32 crPs;                    /* current position in file */

	UINT8 reload_buf;
	UINT8 block_buffer[512];        /* buffer with current file block */

	union
	{
		mfs_fileref mfs;
		hfs_fileref hfs;
	};
};

/*
    open BT ref
*/
struct mac_BTref
{
	struct mac_fileref fileref; /* open B-tree file ref */

	UINT16 nodeSize;        /* size of a node, in bytes */
	UINT32 rootNode;        /* node number of root node */
	UINT32 firstLeafNode;   /* node number of first leaf node */
	UINT32 attributes;      /* persistent attributes about the tree */
	UINT16 treeDepth;       /* maximum height (usually leaf nodes) */
	UINT16 maxKeyLength;    /* maximum key length */

	/* function to compare keys during tree searches */
	int (*key_compare_func)(const void *key1, const void *key2);

	void *node_buf;         /* current node buffer */
};

/*
    Constants for BTHeaderRec attributes field
*/
enum
{
	btha_badCloseMask           = 0x00000001,   /* reserved */
	btha_bigKeysMask            = 0x00000002,   /* key length field is 16 bits */
	btha_variableIndexKeysMask  = 0x00000004    /* keys in index nodes are variable length */
};

/*
    HFS image ref
*/
struct hfs_l2_imgref
{
	UINT16 VBM_start;

	UINT16 ABStart;

	mac_str27 volname;

	mac_BTref extents_BT;
	mac_BTref cat_BT;

	UINT8 VBM[8192];
};

/*
    MFS/HFS image ref
*/
struct mac_l2_imgref
{
	mac_l1_imgref l1_img;

	UINT16 numABs;
	UINT16 blocksperAB;

	UINT16 freeABs;

	UINT32 nxtCNID;     /* nxtFNum in MFS, nxtCNID in HFS */

	mac_format format;
	union
	{
		mfs_l2_imgref mfs;
		hfs_l2_imgref hfs;
	} u;
};

/*
    MFS Master Directory Block
*/
struct mfs_mdb
{
	UINT8    sigWord[2];    /* volume signature - always $D2D7 */
	UINT32BE crDate;        /* date and time of volume creation */
	UINT32BE lsMod/*lsBkUp???*/;/* date and time of last modification (backup???) */
	UINT16BE atrb;          /* volume attributes (0x0000) */
								/* bit 15 is set if volume is locked by software */
	UINT16BE nmFls;         /* number of files in directory */
	UINT16BE dirSt;         /* first block of directory */

	UINT16BE blLn;          /* length of directory in blocks (0x000C) */
	UINT16BE nmAlBlks;      /* number of allocation blocks in volume (0x0187) */
	UINT32BE alBlkSiz;      /* size (in bytes) of allocation blocks (0x00000400) */
	UINT32BE clpSiz;        /* default clump size - number of bytes to allocate
                                when a file grows (0x00002000) */
	UINT16BE alBlSt;        /* first allocation block in volume (0x0010) */

	UINT32BE nxtFNum;       /* next unused file number */
	UINT16BE freeABs;       /* number of unused allocation blocks */

	mac_str27 VN;           /* volume name */

	UINT8    ABlink[512-64];/* Link array for file ABs.  Array of nmAlBlks
                            12-bit-long entries, indexed by AB address.  If an
                            AB belongs to no file, the entry is 0; if an AB is
                            the last in any file, the entry is 1; if an AB
                            belongs to a file and is not the last one, the
                            entry is the AB address of the next file AB plus 1.
                            Note that the array extends on as many consecutive
                            disk blocks as needed (usually the MDB block plus
                            the next one).  Incidentally, this array is not
                            saved in the secondary MDB: presumably, the idea
                            was that the disk utility could rely on the tag
                            data to rebuild the link array if it should ever
                            be corrupted. */
};

/*
    HFS Master Directory Block
*/
struct hfs_mdb
{
/* First fields are similar to MFS, though several fields have a different meaning */
	UINT8    sigWord[2];    /* volume signature - always $D2D7 */
	UINT32BE crDate;        /* date and time of volume creation */
	UINT32BE lsMod;         /* date and time of last modification */
	UINT16BE atrb;          /* volume attributes (0x0000) */
								/* bit 15 is set if volume is locked by software */
	UINT16BE nmFls;         /* number of files in root folder */
	UINT16BE VBMSt;         /* first block of volume bitmap */
	UINT16BE allocPtr;      /* start of next allocation search */

	UINT16BE nmAlBlks;      /* number of allocation blocks in volume */
	UINT32BE alBlkSiz;      /* size (in bytes) of allocation blocks */
	UINT32BE clpSiz;        /* default clump size - number of bytes to allocate
                                when a file grows */
	UINT16BE alBlSt;        /* first allocation block in volume (0x0010) */
	UINT32BE nxtCNID;       /* next unused catalog node ID */
	UINT16BE freeABs;       /* number of unused allocation blocks */
	mac_str27 VN;           /* volume name */

/* next fields are HFS-specific */

	UINT32BE volBkUp;       /* date and time of last backup */
	UINT16BE vSeqNum;       /* volume backup sequence number */
	UINT32BE wrCnt;         /* volume write count */
	UINT32BE xtClpSiz;      /* clump size for extents overflow file */
	UINT32BE ctClpSiz;      /* clump size for catalog file */
	UINT16BE nmRtDirs;      /* number of directories in root folder */
	UINT32BE filCnt;        /* number of files in volume */
	UINT32BE dirCnt;        /* number of directories in volume */
	UINT8    fndrInfo[32];  /* information used by the Finder */

	union
	{
		struct
		{
			UINT16BE VCSize;        /* size (in blocks) of volume cache */
			UINT16BE VBMCSize;      /* size (in blocks) of volume bitmap cache */
			UINT16BE ctlCSize;      /* size (in blocks) of common volume cache */
		};
		struct
		{
			UINT16BE embedSigWord;  /* embedded volume signature */
			hfs_extent embedExtent; /* embedded volume location and size */
		} v2;
	} u;

	UINT32BE xtFlSize;      /* size (in bytes) of extents overflow file */
	hfs_extent_3 xtExtRec;  /* extent record for extents overflow file */
	UINT32BE ctFlSize;      /* size (in bytes) of catalog file */
	hfs_extent_3 ctExtRec;  /* extent record for catalog file */
};

/* to save a little stack space, we use the same buffer for MDB and next blocks */
union img_open_buf
{
	struct mfs_mdb mfs_mdb;
	struct hfs_mdb hfs_mdb;
	UINT8 raw[512];
};

/*
    Information extracted from catalog/directory
*/
struct mac_dirent
{
	UINT16 dataRecType;         /* type of data record */

	mac_FInfo flFinderInfo;     /* information used by the Finder */
	mac_FXInfo flXFinderInfo;   /* information used by the Finder */

	UINT8  flags;               /* bit 0=1 if file locked */

	UINT32 fileID;              /* file ID in directory/catalog */

	UINT32 dataLogicalSize;     /* logical EOF of data fork */
	UINT32 dataPhysicalSize;    /* physical EOF of data fork */
	UINT32 rsrcLogicalSize;     /* logical EOF of resource fork */
	UINT32 rsrcPhysicalSize;    /* physical EOF of resource fork */

	UINT32 createDate;          /* date and time of creation */
	UINT32 modifyDate;          /* date and time of last modification */
};

/*
    Tag record for GCR floppies (12 bytes)

    And, no, I don't know the format of the 20-byte tag record of the HD20
*/
struct floppy_tag_record
{
	UINT32BE fileID;            /* a.k.a. CNID */
								/* a value of 1 seems to be the default for non-AB blocks, but this is not consistent */
	UINT8 ftype;                /* bit 1 = 1 if resource fork */
								/* bit 0 = 1 if block is allocated to user file (i.e. it is not
								    in HFS extent & catalog, and not in non-AB blocks such
								    as MDB and MFS directory)??? */
								/* bit 7 seems to be used, but I don't know what it means */
								/* a value of $FF seems to be the default for non-AB blocks, but this is not consistent */
	UINT8 fattr;                /* bit 0 = 1 if locked(?) */
								/* a value of $FF seems to be the default for non-AB blocks, but this is not consistent */
	UINT16BE fblock;            /* relative file block number (enough for any volume up to 32 MBytes in size) */
	UINT32BE wrCnt;             /* MFS: date and time of last write */
								/* HFS: seems related to the wrCnt field in the mdb, i.e.
								    each time a volume is written to, the current value of
								    wrCnt is written in the tag field, then it is incremented */
								/* (DV17 says "disk block number", but it cannot be true) */
};

#ifdef UNUSED_FUNCTION
static void hfs_image_close(struct mac_l2_imgref *l2_img);
#endif
static imgtoolerr_t mfs_file_get_nth_block_address(struct mac_fileref *fileref, UINT32 block_num, UINT32 *block_address);
static imgtoolerr_t hfs_file_get_nth_block_address(struct mac_fileref *fileref, UINT32 block_num, UINT32 *block_address);
static imgtoolerr_t mfs_lookup_path(struct mac_l2_imgref *l2_img, const char *fpath, mac_str255 filename, mac_dirent *cat_info, int create_it);
static imgtoolerr_t hfs_lookup_path(struct mac_l2_imgref *l2_img, const char *fpath, UINT32 *parID, mac_str255 filename, mac_dirent *cat_info);
static imgtoolerr_t mfs_file_open(struct mac_l2_imgref *l2_img, const mac_str255 filename, mac_forkID fork, struct mac_fileref *fileref);
static imgtoolerr_t hfs_file_open(struct mac_l2_imgref *l2_img, UINT32 parID, const mac_str255 filename, mac_forkID fork, struct mac_fileref *fileref);
static imgtoolerr_t mfs_file_setABeof(struct mac_fileref *fileref, UINT32 newABeof);
static imgtoolerr_t mfs_dir_update(struct mac_fileref *fileref);

static struct mac_l2_imgref *get_imgref(imgtool_image *img)
{
	return (struct mac_l2_imgref *) imgtool_floppy_extrabytes(img);
}


#ifdef UNUSED_FUNCTION
/*
    mac_image_close

    Close a macintosh image.

    l2_img (I/O): level-2 image reference
*/
static void mac_image_close(struct mac_l2_imgref *l2_img)
{
	switch (l2_img->format)
	{
	case L2I_MFS:
		break;

	case L2I_HFS:
		hfs_image_close(l2_img);
		break;
	}
}
#endif

/*
    mac_lookup_path

    Resolve a file path, and translate it to a parID + filename pair that enables
    to do an efficient file search on a HFS volume (and an inefficient one on
    MFS, but it is not an issue as MFS volumes typically have a few dozens
    files, vs. possibly thousands with HFS volumes).

    l2_img (I/O): level-2 image reference
    fpath (I): file path (C string)
    parID (O): set to the CNID of the parent directory if the volume is in HFS
        format (reserved for MFS volumes)
    filename (O): set to the actual name of the file, with capitalization matching
        the one on the volume rather than the one in the fpath parameter (Mac
        string)
    cat_info (O): catalog info for this file extracted from the catalog file
        (may be NULL)

    Return imgtool error code
*/
static imgtoolerr_t mac_lookup_path(struct mac_l2_imgref *l2_img, const char *fpath, UINT32 *parID, mac_str255 filename, mac_dirent *cat_info, int create_it)
{
	imgtoolerr_t err = IMGTOOLERR_UNEXPECTED;

	switch (l2_img->format)
	{
		case L2I_MFS:
			*parID = 0;
			err = mfs_lookup_path(l2_img, fpath, filename, cat_info, create_it);
			break;

		case L2I_HFS:
			err = hfs_lookup_path(l2_img, fpath, parID, filename, cat_info);
			break;
	}
	return err;
}

/*
    mac_file_open

    Open a file located on a macintosh image

    l2_img (I/O): level-2 image reference
    parID (I): CNID of the parent directory if the volume is in HFS format
        (reserved for MFS volumes)
    filename (I): name of the file (Mac string)
    mac_forkID (I): tells which fork should be opened
    fileref (O): mac file reference to open

    Return imgtool error code
*/
static imgtoolerr_t mac_file_open(struct mac_l2_imgref *l2_img, UINT32 parID, const mac_str255 filename, mac_forkID fork, struct mac_fileref *fileref)
{
	switch (l2_img->format)
	{
	case L2I_MFS:
		return mfs_file_open(l2_img, filename, fork, fileref);

	case L2I_HFS:
		return hfs_file_open(l2_img, parID, filename, fork, fileref);
	}

	return IMGTOOLERR_UNEXPECTED;
}

/*
    mac_file_read

    Read data from an open mac file, starting at current position in file

    fileref (I/O): mac file reference
    len (I): number of bytes to read
    dest (O): destination buffer

    Return imgtool error code
*/
static imgtoolerr_t mac_file_read(struct mac_fileref *fileref, UINT32 len, void *dest)
{
	UINT32 block = 0;
	floppy_tag_record tag;
	int run_len;
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;

	if ((fileref->crPs + len) > fileref->eof)
		/* EOF */
		return IMGTOOLERR_UNEXPECTED;

	while (len > 0)
	{
		if (fileref->reload_buf)
		{
			switch (fileref->l2_img->format)
			{
			case L2I_MFS:
				err = mfs_file_get_nth_block_address(fileref, fileref->crPs/512, &block);
				break;

			case L2I_HFS:
				err = hfs_file_get_nth_block_address(fileref, fileref->crPs/512, &block);
				break;
			}
			if (err)
				return err;
			err = image_read_block(&fileref->l2_img->l1_img, block, fileref->block_buffer);
			if (err)
				return err;
			fileref->reload_buf = FALSE;

			if (TAG_EXTRA_CHECKS)
			{
				/* optional check */
				if (image_get_tag_len(&fileref->l2_img->l1_img) == 12)
				{
					err = image_read_tag(&fileref->l2_img->l1_img, block, &tag);
					if (err)
						return err;

					if ((get_UINT32BE(tag.fileID) != fileref->fileID)
						|| (((tag.ftype & 2) != 0) != (fileref->forkType == rsrc_fork))
						|| (get_UINT16BE(tag.fblock) != ((fileref->crPs/512) & 0xffff)))
					{
						return IMGTOOLERR_CORRUPTIMAGE;
					}
				}
			}
		}
		run_len = 512 - (fileref->crPs % 512);
		if (run_len > len)
			run_len = len;

		memcpy(dest, fileref->block_buffer+(fileref->crPs % 512), run_len);
		len -= run_len;
		dest = (UINT8 *)dest + run_len;
		fileref->crPs += run_len;
		if ((fileref->crPs % 512) == 0)
			fileref->reload_buf = TRUE;
	}

	return IMGTOOLERR_SUCCESS;
}

/*
    mac_file_write

    Write data to an open mac file, starting at current position in file

    fileref (I/O): mac file reference
    len (I): number of bytes to read
    dest (O): destination buffer

    Return imgtool error code
*/
static imgtoolerr_t mac_file_write(struct mac_fileref *fileref, UINT32 len, const void *src)
{
	UINT32 block = 0;
	floppy_tag_record tag;
	int run_len;
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;

	if ((fileref->crPs + len) > fileref->eof)
		/* EOF */
		return IMGTOOLERR_UNEXPECTED;

	while (len > 0)
	{
		switch (fileref->l2_img->format)
		{
		case L2I_MFS:
			err = mfs_file_get_nth_block_address(fileref, fileref->crPs/512, &block);
			break;

		case L2I_HFS:
			err = hfs_file_get_nth_block_address(fileref, fileref->crPs/512, &block);
			break;
		}
		if (err)
			return err;

		if (TAG_CHECKS)
		{
			/* optional check */
			if (image_get_tag_len(&fileref->l2_img->l1_img) == 12)
			{
				err = image_read_tag(&fileref->l2_img->l1_img, block, &tag);
				if (err)
					return err;

				if ((get_UINT32BE(tag.fileID) != fileref->fileID)
					|| (((tag.ftype & 2) != 0) != (fileref->forkType == rsrc_fork))
					|| (get_UINT16BE(tag.fblock) != ((fileref->crPs/512) & 0xffff)))
				{
					return IMGTOOLERR_CORRUPTIMAGE;
				}
			}
		}

		if (fileref->reload_buf)
		{
			err = image_read_block(&fileref->l2_img->l1_img, block, fileref->block_buffer);
			if (err)
				return err;
			fileref->reload_buf = FALSE;
		}
		run_len = 512 - (fileref->crPs % 512);
		if (run_len > len)
			run_len = len;

		memcpy(fileref->block_buffer+(fileref->crPs % 512), src, run_len);
		err = image_write_block(&fileref->l2_img->l1_img, block, fileref->block_buffer);
		if (err)
			return err;
		/* update tag data */
		if (image_get_tag_len(&fileref->l2_img->l1_img) == 12)
		{
			if (!TAG_CHECKS)
			{
				err = image_read_tag(&fileref->l2_img->l1_img, block, &tag);
				if (err)
					return err;
			}

			switch (fileref->l2_img->format)
			{
			case L2I_MFS:
				set_UINT32BE(&tag.wrCnt, mac_time_now());
				break;

			case L2I_HFS:
				/*set_UINT32BE(&tag.wrCnt, ++fileref->l2_img.u.hfs.wrCnt);*/    /* ***TODO*** */
				break;
			}

			err = image_write_tag(&fileref->l2_img->l1_img, block, &tag);
			if (err)
				return err;
		}
		len -= run_len;
		src = (const UINT8 *)src + run_len;
		fileref->crPs += run_len;
		if ((fileref->crPs % 512) == 0)
			fileref->reload_buf = TRUE;
	}

	return IMGTOOLERR_SUCCESS;
}

#ifdef UNUSED_FUNCTION
/*
    mac_file_tell

    Get current position in an open mac file

    fileref (I/O): mac file reference
    filePos (O): current position in file

    Return imgtool error code
*/
static imgtoolerr_t mac_file_tell(struct mac_fileref *fileref, UINT32 *filePos)
{
	*filePos = fileref->crPs;

	return IMGTOOLERR_SUCCESS;
}
#endif

/*
    mac_file_seek

    Set current position in an open mac file

    fileref (I/O): mac file reference
    filePos (I): new position in file

    Return imgtool error code
*/
static imgtoolerr_t mac_file_seek(struct mac_fileref *fileref, UINT32 filePos)
{
	if ((fileref->crPs / 512) != (filePos / 512))
		fileref->reload_buf = TRUE;

	fileref->crPs = filePos;

	return IMGTOOLERR_SUCCESS;
}

/*
    mac_file_seteof

    Set the position of the EOF in an open mac file

    fileref (I/O): mac file reference
    newEof (I): new position of EOF in file

    Return imgtool error code
*/
static imgtoolerr_t mac_file_seteof(struct mac_fileref *fileref, UINT32 newEof)
{
	UINT32 newABEof;
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;

	newABEof = (newEof + fileref->l2_img->blocksperAB * 512 - 1) / (fileref->l2_img->blocksperAB * 512);

#if 0
	if (fileref->pLen % (fileref->l2_img->blocksperAB * 512))
		return IMGTOOLERR_CORRUPTIMAGE;
#endif

	if (newEof < fileref->eof)
		fileref->eof = newEof;

	switch (fileref->l2_img->format)
	{
	case L2I_MFS:
		err = mfs_file_setABeof(fileref, newABEof);
		break;

	case L2I_HFS:
		err = IMGTOOLERR_UNIMPLEMENTED;
		break;
	}
	if (err)
		return err;

	fileref->eof = newEof;

	err = mfs_dir_update(fileref);
	if (err)
		return err;

	/* update current pos if beyond new EOF */
#if 0
	if (fileref->crPs > newEof)
	{
		if ((fileref->crPs / 512) != (newEof / 512))
			fileref->reload_buf = TRUE;

		fileref->crPs = newEof;
	}
#endif

	return IMGTOOLERR_SUCCESS;
}

#if 0
#pragma mark -
#pragma mark MFS IMPLEMENTATION
#endif

/*
    directory entry for use in the directory file

    Note the structure is variable length.  It is always word-aligned, and
    cannot cross block boundaries.

    Note that the directory does not seem to be sorted: the order in which
    files appear does not match file names, and it does not always match file
    IDs.
*/
struct mfs_dir_entry
{
	UINT8    flags;             /* bit 7=1 if entry used, bit 0=1 if file locked */
								/* 0x00 means end of block: if we are not done
								with reading the directory, the remnants will
								be read from next block */
	UINT8    flVersNum;         /* version number (usually 0x00, but I don't
                                    have the IM volume that describes it) */
	mac_FInfo flFinderInfo;     /* information used by the Finder */

	UINT32BE fileID;            /* file ID */

	UINT16BE dataStartBlock;    /* first allocation block of data fork */
	UINT32BE dataLogicalSize;   /* logical EOF of data fork */
	UINT32BE dataPhysicalSize;  /* physical EOF of data fork */
	UINT16BE rsrcStartBlock;    /* first allocation block of resource fork */
	UINT32BE rsrcLogicalSize;   /* logical EOF of resource fork */
	UINT32BE rsrcPhysicalSize;  /* physical EOF of resource fork */

	UINT32BE createDate;        /* date and time of creation */
	UINT32BE modifyDate;        /* date and time of last modification */

	UINT8    name[1];           /* first char is length of file name */
								/* next chars are file name - 255 chars at most */
								/* IIRC, Finder 7 only supports 31 chars,
								wheareas earlier versions support 63 chars */
};

/*
    FOBJ desktop resource: describes a folder, or the location of the volume
    icon.

    In typical Apple manner, this resource is not documented.  However, I have
    managed to reverse engineer some parts of it.
*/
struct mfs_FOBJ
{
	UINT8 unknown0[2];      /* $00: $0004 for disk, $0008 for folder??? */
	mac_point location;     /* $02: location in parent window */
	UINT8 unknown1[4];      /* $06: ??? */
	UINT8 view;             /* $0A: manner in which folders are displayed??? */
	UINT8 unknown2;         /* $0B: ??? */
	UINT16BE par_fldr;      /* $0C: parent folder ID */
	UINT8 unknown3[10];     /* $0E: ??? */
	UINT16BE unknown4;      /* $18: ??? */
	UINT32BE createDate;    /* $1A: date and time of creation */
	UINT32BE modifyDate;    /* $1E: date and time of last modification */
	UINT16BE unknown5;      /* $22: put-away folder ID?????? */
	UINT8 unknown6[8];      /* $24: ??? */
	mac_rect bounds;        /* $2C: window bounds */
	mac_point scroll;       /* $34: current scroll offset??? */
	union
	{   /* I think there are two versions of the structure */
		struct
		{
			UINT16BE item_count;    /* number of items (folders and files) in
                                        this folder */
			UINT32BE item_descs[1]; /* this variable-length array has
                                        item_count entries - meaning of entry is unknown */
		} v1;
		struct
		{
			UINT16BE zerofill;      /* always 0? */
			UINT16BE unknown0;      /* always 0??? */
			UINT16BE item_count;    /* number of items (folders and files) in
                                        this folder */
			UINT8 unknown1[20];     /* ??? */
			UINT8 name[1];          /* variable-length macintosh string */
		} v2;
	} u;
};

/*
    MFS open dir ref
*/
struct mfs_dirref
{
	struct mac_l2_imgref *l2_img;   /* image pointer */
	UINT16 index;                   /* current file index in the disk directory */
	UINT16 cur_block;               /* current block offset in directory file */
	UINT16 cur_offset;              /* current byte offset in current block of directory file */
	UINT8 block_buffer[512];        /* buffer with current directory block */
};



static imgtoolerr_t mfs_image_create(imgtool_image *image, imgtool_stream *stream, option_resolution *opts)
{
	imgtoolerr_t err;
	UINT8 buffer[512];
	UINT32 heads, tracks, sector_bytes, i;
	UINT32 total_disk_blocks, total_allocation_blocks, allocation_block_size;
	UINT32 free_allocation_blocks;

	heads = option_resolution_lookup_int(opts, 'H');
	tracks = option_resolution_lookup_int(opts, 'T');
	sector_bytes = option_resolution_lookup_int(opts, 'L');

	get_imgref(image)->l1_img.image = image;
	get_imgref(image)->l1_img.heads = heads;

	if (sector_bytes != 512)
		return IMGTOOLERR_UNEXPECTED;

	/* computation */
	allocation_block_size = 1024;
	total_disk_blocks = 0;
	for (i = 0; i < tracks; i++)
		total_disk_blocks += heads * apple35_sectors_per_track(imgtool_floppy(image), i) * sector_bytes / 512;
	total_allocation_blocks = total_disk_blocks / (allocation_block_size / 512);
	free_allocation_blocks = total_allocation_blocks - 3;

	/* write master directory block */
	memset(buffer, 0, sizeof(buffer));
	place_integer_be(buffer,  0, 2, 0xd2d7);                    /* signature */
	place_integer_be(buffer,  2, 4, mac_time_now());            /* creation date */
	place_integer_be(buffer,  6, 4, mac_time_now());            /* last modified date */
	place_integer_be(buffer, 10, 2, 0);                         /* volume attributes */
	place_integer_be(buffer, 12, 2, 0);                         /* number of files in directory */
	place_integer_be(buffer, 14, 2, 4);                         /* first block of directory */
	place_integer_be(buffer, 16, 2, 12);                        /* length of directory in blocks */
	place_integer_be(buffer, 18, 2, total_allocation_blocks);   /* allocation blocks on volume count */
	place_integer_be(buffer, 20, 4, allocation_block_size);     /* allocation block size */
	place_integer_be(buffer, 24, 4, 8192);                      /* default clumping size */
	place_integer_be(buffer, 28, 2, 16);                        /* first allocation block on volume */
	place_integer_be(buffer, 30, 4, 2);                         /* next unused catalog node */
	place_integer_be(buffer, 34, 2, free_allocation_blocks);    /* free allocation block count */
	pascal_from_c_string(&buffer[36], 28, "Untitled");          /* volume title */

	err = image_write_block(&get_imgref(image)->l1_img, 2, buffer);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}



/*
    mfs_image_open

    Open a MFS image.  Image must already be open on level 1.  This function
    should not be called directly: call mac_image_open() instead.

    l2_img (I/O): level-2 image reference to open (l1_img and format fields
        must be initialized)
    img_open_buf (I): buffer with the MDB block

    Return imgtool error code
*/
static imgtoolerr_t mfs_image_open(imgtool_image *image, imgtool_stream *stream)
{
	imgtoolerr_t err;
	struct mac_l2_imgref *l2_img;
	img_open_buf buf_local;
	img_open_buf *buf;

	l2_img = get_imgref(image);
	l2_img->l1_img.image = image;
	l2_img->l1_img.heads = 1;
	l2_img->format = L2I_MFS;

	/* read MDB */
	err = image_read_block(&l2_img->l1_img, 2, &buf_local.raw);
	if (err)
		return err;
	buf = &buf_local;

	/* check signature word */
	if ((buf->mfs_mdb.sigWord[0] != 0xd2) || (buf->mfs_mdb.sigWord[1] != 0xd7)
			|| (buf->mfs_mdb.VN[0] > 27))
		return IMGTOOLERR_CORRUPTIMAGE;

	l2_img->u.mfs.dir_num_files = get_UINT16BE(buf->mfs_mdb.nmFls);
	l2_img->u.mfs.dir_start = get_UINT16BE(buf->mfs_mdb.dirSt);
	l2_img->u.mfs.dir_blk_len = get_UINT16BE(buf->mfs_mdb.blLn);

	l2_img->numABs = get_UINT16BE(buf->mfs_mdb.nmAlBlks);
	if ((get_UINT32BE(buf->mfs_mdb.alBlkSiz) % 512) || (get_UINT32BE(buf->mfs_mdb.alBlkSiz) == 0))
		return IMGTOOLERR_CORRUPTIMAGE;
	l2_img->blocksperAB = get_UINT32BE(buf->mfs_mdb.alBlkSiz) / 512;
	l2_img->u.mfs.ABStart = get_UINT16BE(buf->mfs_mdb.alBlSt);

	l2_img->nxtCNID = get_UINT32BE(buf->mfs_mdb.nxtFNum);

	l2_img->freeABs = get_UINT16BE(buf->mfs_mdb.freeABs);

	mac_strcpy(l2_img->u.mfs.volname, buf->mfs_mdb.VN);

	if (l2_img->numABs > 4094)
		return IMGTOOLERR_CORRUPTIMAGE;

	/* extract link array */
	{
		int byte_len = l2_img->numABs + ((l2_img->numABs + 1) >> 1);
		int cur_byte;
		int cur_block;
		int block_len = sizeof(buf->mfs_mdb.ABlink);

		/* clear dirty flags */
		for (cur_block=0; cur_block<13; cur_block++)
			l2_img->u.mfs.ABlink_dirty[cur_block] = 0;

		/* append the chunk after MDB to link array */
		cur_byte = 0;
		if (block_len > (byte_len - cur_byte))
			block_len = byte_len - cur_byte;
		memcpy(l2_img->u.mfs.ABlink+cur_byte, buf->mfs_mdb.ABlink, block_len);
		cur_byte += block_len;
		cur_block = 2;
		while (cur_byte < byte_len)
		{
			/* read next block */
			cur_block++;
			err = image_read_block(&l2_img->l1_img, cur_block, buf->raw);
			if (err)
				return err;
			block_len = 512;

			/* append this block to link array */
			if (block_len > (byte_len - cur_byte))
				block_len = byte_len - cur_byte;
			memcpy(l2_img->u.mfs.ABlink+cur_byte, buf->raw, block_len);
			cur_byte += block_len;
		}
		/* check that link array and directory don't overlap */
		if (cur_block >= l2_img->u.mfs.dir_start)
			return IMGTOOLERR_CORRUPTIMAGE;
	}

	return IMGTOOLERR_SUCCESS;
}

/*
    mfs_update_mdb

    Update MDB on disk

    l2_img (I/O): level-2 image reference

    Return imgtool error code
*/
static imgtoolerr_t mfs_update_mdb(struct mac_l2_imgref *l2_img)
{
	imgtoolerr_t err;
	union
	{
		struct mfs_mdb mfs_mdb;
		UINT8 raw[512];
	} buf;

	assert(l2_img->format == L2I_MFS);

	/* read MDB */
	err = image_read_block(&l2_img->l1_img, 2, &buf.mfs_mdb);
	if (err)
		return err;

	set_UINT16BE(&buf.mfs_mdb.nmFls, l2_img->u.mfs.dir_num_files);
#if 0   /* these fields are never changed */
	set_UINT16BE(&buf.mfs_mdb.dirSt, l2_img->u.mfs.dir_start);
	set_UINT16BE(&buf.mfs_mdb.blLn, l2_img->u.mfs.dir_blk_len);

	set_UINT16BE(&buf.mfs_mdb.nmAlBlks, l2_img->numABs);
	set_UINT32BE(&buf.mfs_mdb.alBlkSiz, l2_img->blocksperAB*512);
	set_UINT16BE(&buf.mfs_mdb.alBlSt, l2_img->u.mfs.ABStart);
#endif

	set_UINT32BE(&buf.mfs_mdb.nxtFNum, l2_img->nxtCNID);

	set_UINT16BE(&buf.mfs_mdb.freeABs, l2_img->freeABs);

#if 0   /* these fields are never changed */
	mac_strcpy(buf.mfs_mdb.VN, l2_img->u.mfs.volname);
#endif

	/* save link array */
	{
		int byte_len = l2_img->numABs + ((l2_img->numABs + 1) >> 1);
		int cur_byte = 0;
		int cur_block = 2;
		int block_len = sizeof(buf.mfs_mdb.ABlink);

		/* update the chunk of link array after the MDB */
		if (block_len > (byte_len - cur_byte))
			block_len = byte_len - cur_byte;
		memcpy(buf.mfs_mdb.ABlink, l2_img->u.mfs.ABlink+cur_byte, block_len);
		cur_byte += block_len;

		if (block_len < sizeof(buf.mfs_mdb.ABlink))
			memset(buf.mfs_mdb.ABlink+block_len, 0, sizeof(buf.mfs_mdb.ABlink)-block_len);

		l2_img->u.mfs.ABlink_dirty[0] = 0;

		/* write back modified MDB+link */
		err = image_write_block(&l2_img->l1_img, 2, &buf.mfs_mdb);
		if (err)
			return err;

		while (cur_byte < byte_len)
		{
			/* advance to next block */
			cur_block++;
			block_len = 512;

			/* extract the current chunk of link array */
			if (block_len > (byte_len - cur_byte))
				block_len = byte_len - cur_byte;

			if (l2_img->u.mfs.ABlink_dirty[cur_block-2])
			{
				memcpy(buf.raw, l2_img->u.mfs.ABlink+cur_byte, block_len);
				if (block_len < 512)
					memset(buf.raw+block_len, 0, 512-block_len);
				/* write back link array */
				err = image_write_block(&l2_img->l1_img, cur_block, buf.raw);
				if (err)
					return err;
				l2_img->u.mfs.ABlink_dirty[cur_block-2] = 0;
			}

			cur_byte += block_len;
		}
	}

	return IMGTOOLERR_SUCCESS;
}

/*
    mfs_dir_open

    Open the directory file

    l2_img (I/O): level-2 image reference
    dirref (O): open directory file reference

    Return imgtool error code
*/
static imgtoolerr_t mfs_dir_open(struct mac_l2_imgref *l2_img, const char *path, mfs_dirref *dirref)
{
	imgtoolerr_t err;

	assert(l2_img->format == L2I_MFS);

	if (path[0])
		return IMGTOOLERR_PATHNOTFOUND;

	dirref->l2_img = l2_img;
	dirref->index = 0;

	dirref->cur_block = 0;
	dirref->cur_offset = 0;
	err = image_read_block(&l2_img->l1_img, l2_img->u.mfs.dir_start + dirref->cur_block, dirref->block_buffer);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}

/*
    mfs_dir_read

    Read one entry of directory file

    dirref (I/O): open directory file reference
    dir_entry (O): set to point to the entry read: set to NULL if EOF or error

    Return imgtool error code
*/
static imgtoolerr_t mfs_dir_read(mfs_dirref *dirref, mfs_dir_entry **dir_entry)
{
	mfs_dir_entry *cur_dir_entry;
	size_t cur_dir_entry_len;
	imgtoolerr_t err;


	if (dir_entry)
		*dir_entry = NULL;

	if (dirref->index == dirref->l2_img->u.mfs.dir_num_files)
		/* EOF */
		return IMGTOOLERR_SUCCESS;

	/* get cat entry pointer */
	cur_dir_entry = (mfs_dir_entry *) (dirref->block_buffer + dirref->cur_offset);
	while ((dirref->cur_offset == 512) || ! (cur_dir_entry->flags & 0x80))
	{
		dirref->cur_block++;
		dirref->cur_offset = 0;
		if (dirref->cur_block > dirref->l2_img->u.mfs.dir_blk_len)
			/* aargh! */
			return IMGTOOLERR_CORRUPTIMAGE;
		err = image_read_block(&dirref->l2_img->l1_img, dirref->l2_img->u.mfs.dir_start + dirref->cur_block, dirref->block_buffer);
		if (err)
			return err;
		cur_dir_entry = (mfs_dir_entry *) (dirref->block_buffer + dirref->cur_offset);
	}

	cur_dir_entry_len = offsetof(mfs_dir_entry, name) + cur_dir_entry->name[0] + 1;

	if ((dirref->cur_offset + cur_dir_entry_len) > 512)
		/* aargh! */
		return IMGTOOLERR_CORRUPTIMAGE;

	/* entry looks valid: set pointer to entry */
	if (dir_entry)
		*dir_entry = cur_dir_entry;

	/* update offset in block */
	dirref->cur_offset += cur_dir_entry_len;
	/* align to word boundary */
	dirref->cur_offset = (dirref->cur_offset + 1) & ~1;

	/* update file count */
	dirref->index++;

	return IMGTOOLERR_SUCCESS;
}

/*
    mfs_dir_insert

    Add an entry in the directory file

    l2_img (I/O): level-2 image reference
    dirref (I/O): open directory file reference
    filename (I): name of the file for which an entry is created (Mac string)
    dir_entry (O): set to point to the created entry: set to NULL if EOF or
        error

    Return imgtool error code
*/
static imgtoolerr_t mfs_dir_insert(struct mac_l2_imgref *l2_img, mfs_dirref *dirref, const UINT8 *new_fname, mfs_dir_entry **dir_entry)
{
	size_t new_dir_entry_len;
	mfs_dir_entry *cur_dir_entry;
	size_t cur_dir_entry_len;
	UINT32 cur_date;
	imgtoolerr_t err;

	dirref->l2_img = l2_img;
	dirref->index = 0;

	new_dir_entry_len = offsetof(mfs_dir_entry, name) + new_fname[0] + 1;

	for (dirref->cur_block = 0; dirref->cur_block < dirref->l2_img->u.mfs.dir_blk_len; dirref->cur_block++)
	{
		/* read current block */
		err = image_read_block(&dirref->l2_img->l1_img, dirref->l2_img->u.mfs.dir_start + dirref->cur_block, dirref->block_buffer);
		if (err)
			return err;

		/* get free chunk in this block */
		dirref->cur_offset = 0;
		cur_dir_entry = (mfs_dir_entry *) (dirref->block_buffer + dirref->cur_offset);
		while ((dirref->cur_offset < 512) && (cur_dir_entry->flags & 0x80))
		{   /* skip cur_dir_entry */
			cur_dir_entry_len = offsetof(mfs_dir_entry, name) + cur_dir_entry->name[0] + 1;
			/* update offset in block */
			dirref->cur_offset += cur_dir_entry_len;
			/* align to word boundary */
			dirref->cur_offset = (dirref->cur_offset + 1) & ~1;
			/* update entry pointer */
			cur_dir_entry = (mfs_dir_entry *) (dirref->block_buffer + dirref->cur_offset);
			/* update file index (useless, but can't harm) */
			dirref->index++;
		}

		if (new_dir_entry_len <= (/*512*/510 - dirref->cur_offset))
		{
			/*memcpy(cur_dir_entry, new_dir_entry, new_dir_entry_len);*/
			cur_dir_entry->flags = 0x80;
			cur_dir_entry->flVersNum = 0x00;
			memset(&cur_dir_entry->flFinderInfo, 0, sizeof(cur_dir_entry->flFinderInfo));
			set_UINT32BE(&cur_dir_entry->fileID, dirref->l2_img->nxtCNID++);
			set_UINT16BE(&cur_dir_entry->dataStartBlock, 1);
			set_UINT32BE(&cur_dir_entry->dataLogicalSize, 0);
			set_UINT32BE(&cur_dir_entry->dataPhysicalSize, 0);
			set_UINT16BE(&cur_dir_entry->rsrcStartBlock, 1);
			set_UINT32BE(&cur_dir_entry->rsrcLogicalSize, 0);
			set_UINT32BE(&cur_dir_entry->rsrcPhysicalSize, 0);
			cur_date = mac_time_now();
			set_UINT32BE(&cur_dir_entry->createDate, cur_date);
			set_UINT32BE(&cur_dir_entry->modifyDate, cur_date);
			mac_strcpy(cur_dir_entry->name, new_fname);

			/* update offset in block */
			dirref->cur_offset += new_dir_entry_len;
			/* align to word boundary */
			dirref->cur_offset = (dirref->cur_offset + 1) & ~1;
			if (dirref->cur_offset < 512)
				/* mark remaining space as free record */
				dirref->block_buffer[dirref->cur_offset] = 0;
			/* write back directory */
			err = image_write_block(&dirref->l2_img->l1_img, dirref->l2_img->u.mfs.dir_start + dirref->cur_block, dirref->block_buffer);
			if (err)
				return err;
			/* update file count */
			dirref->l2_img->u.mfs.dir_num_files++;

			/* update MDB (nxtCNID & dir_num_files fields) */
			err = mfs_update_mdb(dirref->l2_img);
			if (err)
				return err;

			if (dir_entry)
				*dir_entry = cur_dir_entry;
			return IMGTOOLERR_SUCCESS;
		}
	}

	return IMGTOOLERR_NOSPACE;
}

/*
    mfs_dir_update

    Update one entry of directory file

    fileref (I/O): open file reference

    Return imgtool error code
*/
static imgtoolerr_t mfs_dir_update(struct mac_fileref *fileref)
{
	UINT16 cur_block;
	UINT16 cur_offset;
	UINT8 block_buffer[512];
	mfs_dir_entry *cur_dir_entry;
	size_t cur_dir_entry_len;
	imgtoolerr_t err;

	for (cur_block = 0; cur_block < fileref->l2_img->u.mfs.dir_blk_len; cur_block++)
	{
		/* read current block */
		err = image_read_block(&fileref->l2_img->l1_img, fileref->l2_img->u.mfs.dir_start + cur_block, block_buffer);
		if (err)
			return err;

		/* get free chunk in this block */
		cur_offset = 0;
		cur_dir_entry = (mfs_dir_entry *) (block_buffer + cur_offset);
		while ((cur_offset < 512) && (cur_dir_entry->flags & 0x80))
		{
			if (get_UINT32BE(cur_dir_entry->fileID) == fileref->fileID)
			{   /* found it: update directory entry */
				switch (fileref->forkType)
				{
				case data_fork:
					set_UINT16BE(&cur_dir_entry->dataStartBlock, fileref->mfs.stBlk);
					set_UINT32BE(&cur_dir_entry->dataLogicalSize, fileref->eof);
					set_UINT32BE(&cur_dir_entry->dataPhysicalSize, fileref->pLen);
					break;

				case rsrc_fork:
					set_UINT16BE(&cur_dir_entry->rsrcStartBlock, fileref->mfs.stBlk);
					set_UINT32BE(&cur_dir_entry->rsrcLogicalSize, fileref->eof);
					set_UINT32BE(&cur_dir_entry->rsrcPhysicalSize, fileref->pLen);
					break;
				}
				/* write back directory */
				err = image_write_block(&fileref->l2_img->l1_img, fileref->l2_img->u.mfs.dir_start + cur_block, block_buffer);
				if (err)
					return err;

				return IMGTOOLERR_SUCCESS;
			}
			/* skip cur_dir_entry */
			cur_dir_entry_len = offsetof(mfs_dir_entry, name) + cur_dir_entry->name[0] + 1;
			/* update offset in block */
			cur_offset += cur_dir_entry_len;
			/* align to word boundary */
			cur_offset = (cur_offset + 1) & ~1;
			/* update entry pointer */
			cur_dir_entry = (mfs_dir_entry *) (block_buffer + cur_offset);
		}
	}

	return IMGTOOLERR_UNEXPECTED;
}


/*
    mfs_find_dir_entry

    Find a file in an MFS directory

    dirref (I/O): open directory file reference
    filename (I): file name (Mac string)
    dir_entry (O): set to point to the entry read: set to NULL if EOF or error

    Return imgtool error code
*/
static imgtoolerr_t mfs_find_dir_entry(mfs_dirref *dirref, const mac_str255 filename, mfs_dir_entry **dir_entry)
{
	mfs_dir_entry *cur_dir_entry;
	imgtoolerr_t err;

	if (dir_entry)
		*dir_entry = NULL;

	/*  scan dir for file */
	while (1)
	{
		err = mfs_dir_read(dirref, &cur_dir_entry);
		if (err)
			return err;
		if (!cur_dir_entry)
			/* EOF */
			break;
		if ((! mac_stricmp(filename, cur_dir_entry->name)) && (cur_dir_entry->flVersNum == 0))
		{   /* file found */

			if (dir_entry)
				*dir_entry = cur_dir_entry;

			return IMGTOOLERR_SUCCESS;
		}
	}

	return IMGTOOLERR_FILENOTFOUND;
}

/*
    mfs_lookup_path

    Resolve a file path for MFS volumes.  This function should not be called
    directly: call mac_lookup_path instead.

    l2_img (I/O): level-2 image reference
    fpath (I): file path (C string)
    filename (O): set to the actual name of the file, with capitalization matching
        the one on the volume rather than the one in the fpath parameter (Mac
        string)
    cat_info (I/O): on output, catalog info for this file extracted from the
        catalog file (may be NULL)
        If create_it is TRUE, created info will first be set according to the
        data from cat_info
    create_it (I): TRUE if entry should be created if not found

    Return imgtool error code
*/
static imgtoolerr_t mfs_lookup_path(struct mac_l2_imgref *l2_img, const char *fpath, mac_str255 filename, mac_dirent *cat_info, int create_it)
{
	mfs_dirref dirref;
	mfs_dir_entry *dir_entry;
	imgtoolerr_t err;

	/* rapid check */
	if (strchr(fpath, ':'))
		return IMGTOOLERR_BADFILENAME;

	/* extract file name */
	c_to_mac_strncpy(filename, fpath, strlen(fpath));

	/* open dir */
	mfs_dir_open(l2_img, "", &dirref);

	/* find file */
	err = mfs_find_dir_entry(&dirref, filename, &dir_entry);
	if ((err == IMGTOOLERR_FILENOTFOUND) && create_it)
		err = mfs_dir_insert(l2_img, &dirref, filename, &dir_entry);
	if (err)
		return err;

	mac_strcpy(filename, dir_entry->name);

	if (create_it && cat_info)
	{
		dir_entry->flFinderInfo = cat_info->flFinderInfo;
		dir_entry->flags = (dir_entry->flags & 0x80) | (cat_info->flags & 0x7f);
		set_UINT32BE(&dir_entry->createDate, cat_info->createDate);
		set_UINT32BE(&dir_entry->modifyDate, cat_info->modifyDate);

		/* write current directory block */
		err = image_write_block(&l2_img->l1_img, l2_img->u.mfs.dir_start + dirref.cur_block, dirref.block_buffer);
		if (err)
			return err;
	}

	if (cat_info)
	{
		cat_info->flFinderInfo = dir_entry->flFinderInfo;
		memset(&cat_info->flXFinderInfo, 0, sizeof(cat_info->flXFinderInfo));
		cat_info->flags = dir_entry->flags;
		cat_info->fileID = get_UINT32BE(dir_entry->fileID);
		cat_info->dataLogicalSize = get_UINT32BE(dir_entry->dataLogicalSize);
		cat_info->dataPhysicalSize = get_UINT32BE(dir_entry->dataPhysicalSize);
		cat_info->rsrcLogicalSize = get_UINT32BE(dir_entry->rsrcLogicalSize);
		cat_info->rsrcPhysicalSize = get_UINT32BE(dir_entry->rsrcPhysicalSize);
		cat_info->createDate = get_UINT32BE(dir_entry->createDate);
		cat_info->modifyDate = get_UINT32BE(dir_entry->modifyDate);
		cat_info->dataRecType = 0x200; /* hcrt_File */
	}

	return IMGTOOLERR_SUCCESS;
}

/*
    mfs_file_open_internal

    Open a file fork, given its directory entry.  This function should not be
    called directly: call mfs_file_open instead.

    l2_img (I/O): level-2 image reference
    dir_entry (I): directory entry for the file to open
    mac_forkID (I): tells which fork should be opened
    fileref (O): mac file reference to open

    Return imgtool error code
*/
static imgtoolerr_t mfs_file_open_internal(struct mac_l2_imgref *l2_img, const mfs_dir_entry *dir_entry, mac_forkID fork, struct mac_fileref *fileref)
{
	assert(l2_img->format == L2I_MFS);

	fileref->l2_img = l2_img;

	fileref->fileID = get_UINT32BE(dir_entry->fileID);
	fileref->forkType = fork;

	switch (fork)
	{
	case data_fork:
		fileref->mfs.stBlk = get_UINT16BE(dir_entry->dataStartBlock);
		fileref->eof = get_UINT32BE(dir_entry->dataLogicalSize);
		fileref->pLen = get_UINT32BE(dir_entry->dataPhysicalSize);
		break;

	case rsrc_fork:
		fileref->mfs.stBlk = get_UINT16BE(dir_entry->rsrcStartBlock);
		fileref->eof = get_UINT32BE(dir_entry->rsrcLogicalSize);
		fileref->pLen = get_UINT32BE(dir_entry->rsrcPhysicalSize);
		break;
	}

	fileref->crPs = 0;

	fileref->reload_buf = TRUE;

	return IMGTOOLERR_SUCCESS;
}

/*
    mfs_file_open

    Open a file located on a MFS volume.  This function should not be called
    directly: call mac_file_open instead.

    l2_img (I/O): level-2 image reference
    filename (I): name of the file (Mac string)
    mac_forkID (I): tells which fork should be opened
    fileref (O): mac file reference to open

    Return imgtool error code
*/
static imgtoolerr_t mfs_file_open(struct mac_l2_imgref *l2_img, const mac_str255 filename, mac_forkID fork, struct mac_fileref *fileref)
{
	mfs_dirref dirref;
	mfs_dir_entry *dir_entry;
	imgtoolerr_t err;

	/* open dir */
	mfs_dir_open(l2_img, "", &dirref);

	/* find file */
	err = mfs_find_dir_entry(&dirref, filename, &dir_entry);
	if (err)
		return err;

	/* open it */
	return mfs_file_open_internal(l2_img, dir_entry, fork, fileref);
}

/*
    mfs_get_ABlink

    Read one entry of the Allocation Bitmap link array, on an MFS volume.

    l2_img (I/O): level-2 image reference
    AB_address (I): index in the array, which is an AB address

    Returns the 12-bit value read in array.
*/
static UINT16 mfs_get_ABlink(struct mac_l2_imgref *l2_img, UINT16 AB_address)
{
	UINT16 reply;
	int base;

	assert(l2_img->format == L2I_MFS);

	base = (AB_address >> 1) * 3;

	if (! (AB_address & 1))
		reply = (l2_img->u.mfs.ABlink[base] << 4) | ((l2_img->u.mfs.ABlink[base+1] >> 4) & 0x0f);
	else
		reply = ((l2_img->u.mfs.ABlink[base+1] << 8) & 0xf00) | l2_img->u.mfs.ABlink[base+2];

	return reply;
}

/*
    mfs_set_ABlink

    Set one entry of the Allocation Bitmap link array, on an MFS volume.

    l2_img (I/O): level-2 image reference
    AB_address (I): index in the array, which is an AB address
    data (I): 12-bit value to write in array
*/
static void mfs_set_ABlink(struct mac_l2_imgref *l2_img, UINT16 AB_address, UINT16 data)
{
	int base;

	assert(l2_img->format == L2I_MFS);

	base = (AB_address >> 1) * 3;

	if (! (AB_address & 1))
	{
		l2_img->u.mfs.ABlink[base] = (data >> 4) & 0xff;
		l2_img->u.mfs.ABlink[base+1] = (l2_img->u.mfs.ABlink[base+1] & 0x0f) | ((data << 4) & 0xf0);

		l2_img->u.mfs.ABlink_dirty[(base+64)/512] = 1;
		l2_img->u.mfs.ABlink_dirty[(base+1+64)/512] = 1;
	}
	else
	{
		l2_img->u.mfs.ABlink[base+1] = (l2_img->u.mfs.ABlink[base+1] & 0xf0) | ((data >> 8) & 0x0f);
		l2_img->u.mfs.ABlink[base+2] = data & 0xff;

		l2_img->u.mfs.ABlink_dirty[(base+1+64)/512] = 1;
		l2_img->u.mfs.ABlink_dirty[(base+2+64)/512] = 1;
	}
}

/*
    mfs_file_get_nth_block_address

    Get the disk block address of a given block in an open file on a MFS image.
    Called by macintosh file code.

    fileref (I/O): open mac file reference
    block_num (I): file block index
    block_address (O): disk block address for the file block

    Return imgtool error code
*/
static imgtoolerr_t mfs_file_get_nth_block_address(struct mac_fileref *fileref, UINT32 block_num, UINT32 *block_address)
{
	UINT32 AB_num;
	UINT32 i;
	UINT16 AB_address;

	assert(fileref->l2_img->format == L2I_MFS);

	AB_num = block_num / fileref->l2_img->blocksperAB;

	AB_address = fileref->mfs.stBlk;
	if ((AB_address == 0) || (AB_address >= fileref->l2_img->numABs+2))
		/* 0 -> ??? */
		return IMGTOOLERR_CORRUPTIMAGE;
	if (AB_address == 1)
		/* EOF */
		return IMGTOOLERR_UNEXPECTED;
	AB_address -= 2;
	for (i=0; i<AB_num; i++)
	{
		AB_address = mfs_get_ABlink(fileref->l2_img, AB_address);
		if ((AB_address == 0) || (AB_address >= fileref->l2_img->numABs+2))
			/* 0 -> empty block: there is no way an empty block could make it
			into the link chain!!! */
			return IMGTOOLERR_CORRUPTIMAGE;
		if (AB_address == 1)
			/* EOF */
			return IMGTOOLERR_UNEXPECTED;
		AB_address -= 2;
	}

	*block_address = fileref->l2_img->u.mfs.ABStart + AB_address * fileref->l2_img->blocksperAB
						+ block_num % fileref->l2_img->blocksperAB;

	return IMGTOOLERR_SUCCESS;
}

/*
    mfs_file_allocABs

    Allocate a chunk of ABs

    fileref (I/O): open mac file reference
    lastAB (I): AB address on disk of last file AB (only if
        fileref->mfs.stBlk != 1)
    allocABs (I): number of ABs to allocate in addition to the current file
        allocation
    fblock (I): first file block to allocate (used for tag data)

    Return imgtool error code
*/
static imgtoolerr_t mfs_file_allocABs(struct mac_fileref *fileref, UINT16 lastAB, UINT32 allocABs, UINT32 fblock)
{
	int numABs = fileref->l2_img->numABs;
	int free_ABs;
	int i, j;
	floppy_tag_record tag;
	int extentBaseAB, extentABlen;
	int firstBestExtentBaseAB = 0, firstBestExtentABlen;
	int secondBestExtentBaseAB = 0, secondBestExtentABlen;
	imgtoolerr_t err;

	/* return if done */
	if (! allocABs)
		return IMGTOOLERR_SUCCESS;

	/* compute free space */
	free_ABs = 0;
	for (i=0; i<numABs; i++)
	{
		if (mfs_get_ABlink(fileref->l2_img, i) == 0)
		{
			if (TAG_CHECKS)
			{
				/* optional check */
				if (image_get_tag_len(&fileref->l2_img->l1_img) == 12)
				{
					for (j=0; j<fileref->l2_img->blocksperAB; j++)
					{
						err = image_read_tag(&fileref->l2_img->l1_img, fileref->l2_img->u.mfs.ABStart + i * fileref->l2_img->blocksperAB + j, &tag);
						if (err)
							return err;

						if (get_UINT32BE(tag.fileID) != 0)
						{
							/*return IMGTOOLERR_CORRUPTIMAGE;*/
							goto corrupt_free_block;
						}
					}
				}
			}

			free_ABs++;
		}
corrupt_free_block:
		;
	}

	/* check we have enough free space */
	if (free_ABs < allocABs)
		return IMGTOOLERR_NOSPACE;

	if (fileref->mfs.stBlk != 1)
	{   /* try to extend last file extent */
		/* append free ABs after last AB */
		for (i=lastAB+1; (mfs_get_ABlink(fileref->l2_img, i) == 0) && (allocABs > 0) && (i < numABs); i++)
		{
			if (TAG_CHECKS)
			{
				/* optional check */
				if (image_get_tag_len(&fileref->l2_img->l1_img) == 12)
				{
					for (j=0; j<fileref->l2_img->blocksperAB; j++)
					{
						err = image_read_tag(&fileref->l2_img->l1_img, fileref->l2_img->u.mfs.ABStart + i * fileref->l2_img->blocksperAB + j, &tag);
						if (err)
							return err;

						if (get_UINT32BE(tag.fileID) != 0)
						{
							/*return IMGTOOLERR_CORRUPTIMAGE;*/
							goto corrupt_free_block2;
						}
					}
				}
			}

			mfs_set_ABlink(fileref->l2_img, lastAB, i+2);
			lastAB = i;
			allocABs--;
			free_ABs--;
		}
corrupt_free_block2:
		/* return if done */
		if (! allocABs)
		{
			mfs_set_ABlink(fileref->l2_img, lastAB, 1);
			fileref->l2_img->freeABs = free_ABs;
			return IMGTOOLERR_SUCCESS;  /* done */
		}
	}

	while (allocABs)
	{
		/* find smallest data block at least nb_alloc_physrecs in length, and largest data block less than nb_alloc_physrecs in length */
		firstBestExtentABlen = INT_MAX;
		secondBestExtentABlen = 0;
		for (i=0; i<numABs; i++)
		{
			if (mfs_get_ABlink(fileref->l2_img, i) == 0)
			{   /* found one free block */
				/* compute its length */
				extentBaseAB = i;
				extentABlen = 0;
				while ((i<numABs) && (mfs_get_ABlink(fileref->l2_img, i) == 0))
				{
					if (TAG_CHECKS)
					{
						/* optional check */
						if (image_get_tag_len(&fileref->l2_img->l1_img) == 12)
						{
							for (j=0; j<fileref->l2_img->blocksperAB; j++)
							{
								err = image_read_tag(&fileref->l2_img->l1_img, fileref->l2_img->u.mfs.ABStart + i * fileref->l2_img->blocksperAB + j, &tag);
								if (err)
									return err;

								if (get_UINT32BE(tag.fileID) != 0)
								{
									/*return IMGTOOLERR_CORRUPTIMAGE;*/
									goto corrupt_free_block3;
								}
							}
						}
					}

					extentABlen++;
					i++;
				}
corrupt_free_block3:
				/* compare to previous best and second-best blocks */
				if ((extentABlen < firstBestExtentABlen) && (extentABlen >= allocABs))
				{
					firstBestExtentBaseAB = extentBaseAB;
					firstBestExtentABlen = extentABlen;
					if (extentABlen == allocABs)
						/* no need to search further */
						break;
				}
				else if ((extentABlen > secondBestExtentABlen) && (extentABlen < allocABs))
				{
					secondBestExtentBaseAB = extentBaseAB;
					secondBestExtentABlen = extentABlen;
				}
			}
		}

		if (firstBestExtentABlen != INT_MAX)
		{   /* found one contiguous block which can hold it all */
			extentABlen = allocABs;
			for (i=0; i<allocABs; i++)
			{
				if (fileref->mfs.stBlk != 1)
					mfs_set_ABlink(fileref->l2_img, lastAB, firstBestExtentBaseAB+i+2);
				else
					fileref->mfs.stBlk = firstBestExtentBaseAB+i+2;
				lastAB = firstBestExtentBaseAB+i;
				free_ABs--;
				/* set tag to allocated */
				if (image_get_tag_len(&fileref->l2_img->l1_img) == 12)
				{
					set_UINT32BE(&tag.fileID, fileref->fileID);
					tag.ftype = 1;
					if ((fileref->forkType) == rsrc_fork)
						tag.ftype |= 2;
					tag.fattr = /*fattr*/ 0;        /* ***TODO*** */
					for (j=0; j<fileref->l2_img->blocksperAB; j++)
					{
						set_UINT16BE(&tag.fblock, fblock & 0xffff);
						set_UINT32BE(&tag.wrCnt, mac_time_now());
						err = image_write_tag(&fileref->l2_img->l1_img, fileref->l2_img->u.mfs.ABStart + lastAB * fileref->l2_img->blocksperAB + j, &tag);
						if (err)
						{
							mfs_set_ABlink(fileref->l2_img, lastAB, 1);
							fileref->l2_img->freeABs = free_ABs;
							return err;
						}
						fblock++;
					}
				}
			}
			allocABs = 0;
			mfs_set_ABlink(fileref->l2_img, lastAB, 1);
			fileref->l2_img->freeABs = free_ABs;
			/*return IMGTOOLERR_SUCCESS;*/  /* done */
		}
		else if (secondBestExtentABlen != 0)
		{   /* jeez, we need to fragment it.  We use the largest smaller block to limit fragmentation. */
			for (i=0; i<secondBestExtentABlen; i++)
			{
				if (fileref->mfs.stBlk != 1)
					mfs_set_ABlink(fileref->l2_img, lastAB, secondBestExtentBaseAB+i+2);
				else
					fileref->mfs.stBlk = secondBestExtentBaseAB+i+2;
				lastAB = secondBestExtentBaseAB+i;
				free_ABs--;
				/* set tag to allocated */
				if (image_get_tag_len(&fileref->l2_img->l1_img) == 12)
				{
					set_UINT32BE(&tag.fileID, fileref->fileID);
					tag.ftype = 1;
					if ((fileref->forkType) == rsrc_fork)
						tag.ftype |= 2;
					tag.fattr = /*fattr*/ 0;        /* ***TODO*** */
					for (j=0; j<fileref->l2_img->blocksperAB; j++)
					{
						set_UINT16BE(&tag.fblock, fblock & 0xffff);
						set_UINT32BE(&tag.wrCnt, mac_time_now());
						err = image_write_tag(&fileref->l2_img->l1_img, fileref->l2_img->u.mfs.ABStart + lastAB * fileref->l2_img->blocksperAB + j, &tag);
						if (err)
						{
							mfs_set_ABlink(fileref->l2_img, lastAB, 1);
							fileref->l2_img->freeABs = free_ABs;
							return err;
						}
						fblock++;
					}
				}
			}
			allocABs -= secondBestExtentABlen;
		}
		else
		{
			mfs_set_ABlink(fileref->l2_img, lastAB, 1);
			return IMGTOOLERR_NOSPACE;  /* This should never happen, as we pre-check that there is enough free space */
		}
	}

	return IMGTOOLERR_SUCCESS;
}

/*
    mfs_file_setABeof

    Set physical file EOF in ABs

    fileref (I/O): open mac file reference
    newABeof (I): desired number of allocated ABs for this file

    Return imgtool error code
*/
static imgtoolerr_t mfs_file_setABeof(struct mac_fileref *fileref, UINT32 newABeof)
{
	UINT16 AB_address = 0;
	UINT16 AB_link;
	int i, j;
	floppy_tag_record tag;
	int MDB_dirty = 0;
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;


	assert(fileref->l2_img->format == L2I_MFS);

	/* run through link chain until we reach the old or the new EOF */
	AB_link = fileref->mfs.stBlk;
	if ((AB_link == 0) || (AB_link >= fileref->l2_img->numABs+2))
		/* 0 -> ??? */
		return IMGTOOLERR_CORRUPTIMAGE;
	for (i=0; (i<newABeof) && (AB_link!=1); i++)
	{
		AB_address = AB_link - 2;
		AB_link = mfs_get_ABlink(fileref->l2_img, AB_address);
		if ((AB_link == 0) || (AB_link >= fileref->l2_img->numABs+2))
			/* 0 -> empty block: there is no way an empty block could make it
			into the link chain!!! */
			return IMGTOOLERR_CORRUPTIMAGE;

		if (TAG_CHECKS)
		{
			/* optional check */
			if (image_get_tag_len(&fileref->l2_img->l1_img) == 12)
			{
				for (j=0; j<fileref->l2_img->blocksperAB; j++)
				{
					err = image_read_tag(&fileref->l2_img->l1_img, fileref->l2_img->u.mfs.ABStart + AB_address * fileref->l2_img->blocksperAB + j, &tag);
					if (err)
						return err;

					if ((get_UINT32BE(tag.fileID) != fileref->fileID)
						|| (((tag.ftype & 2) != 0) != (fileref->forkType == rsrc_fork))
						|| (get_UINT16BE(tag.fblock) != ((i * fileref->l2_img->blocksperAB + j) & 0xffff)))
					{
						return IMGTOOLERR_CORRUPTIMAGE;
					}
				}
			}
		}
	}

	if (i == newABeof)
	{   /* new EOF is shorter than old one */
		/* mark new eof */
		if (i==0)
			fileref->mfs.stBlk = 1;
		else
		{
			mfs_set_ABlink(fileref->l2_img, AB_address, 1);
			MDB_dirty = 1;
		}

		/* free all remaining blocks */
		while (AB_link != 1)
		{
			AB_address = AB_link - 2;
			AB_link = mfs_get_ABlink(fileref->l2_img, AB_address);
			if ((AB_link == 0) || (AB_link >= fileref->l2_img->numABs+2))
			{   /* 0 -> empty block: there is no way an empty block could make
                it into the link chain!!! */
				if (MDB_dirty)
				{   /* update MDB (freeABs field) and ABLink array */
					err = mfs_update_mdb(fileref->l2_img);
					if (err)
						return err;
				}
				return IMGTOOLERR_CORRUPTIMAGE;
			}

			if (TAG_CHECKS)
			{
				/* optional check */
				if (image_get_tag_len(&fileref->l2_img->l1_img) == 12)
				{
					for (j=0; j<fileref->l2_img->blocksperAB; j++)
					{
						err = image_read_tag(&fileref->l2_img->l1_img, fileref->l2_img->u.mfs.ABStart + AB_address * fileref->l2_img->blocksperAB + j, &tag);
						if (err)
							return err;

						if ((get_UINT32BE(tag.fileID) != fileref->fileID)
							|| (((tag.ftype & 2) != 0) != (fileref->forkType == rsrc_fork))
							|| (get_UINT16BE(tag.fblock) != ((i * fileref->l2_img->blocksperAB + j) & 0xffff)))
						{
							return IMGTOOLERR_CORRUPTIMAGE;
						}
					}
				}
			}

			mfs_set_ABlink(fileref->l2_img, AB_address, 0);
			fileref->l2_img->freeABs++;
			MDB_dirty = 1;
			/* set tag to free */
			if (image_get_tag_len(&fileref->l2_img->l1_img) == 12)
			{
				memset(&tag, 0, sizeof(tag));
				for (j=0; j<fileref->l2_img->blocksperAB; j++)
				{
					err = image_write_tag(&fileref->l2_img->l1_img, fileref->l2_img->u.mfs.ABStart + AB_address * fileref->l2_img->blocksperAB + j, &tag);
					if (err)
						return err;
				}
			}
			i++;
		}
	}
	else
	{   /* new EOF is larger than old one */
		err = mfs_file_allocABs(fileref, AB_address, newABeof - i, i * fileref->l2_img->blocksperAB);
		if (err)
			return err;
		MDB_dirty = 1;
	}

	if (MDB_dirty)
	{   /* update MDB (freeABs field) and ABLink array */
		err = mfs_update_mdb(fileref->l2_img);
		if (err)
			return err;
	}

	fileref->pLen = newABeof * (fileref->l2_img->blocksperAB * 512);

	return IMGTOOLERR_SUCCESS;
}

#ifdef UNUSED_FUNCTION
/*
    mfs_hashString

    Hash a string: under MFS, this provides the resource ID of the comment
    resource associated with the file whose name is provided (FCMT resource
    type).

    Ripped from Apple technote TB06 (converted from 68k ASM to C)

    string (I): string to hash

    Returns hash value
*/
static int mfs_hashString(const mac_str255 string)
{
	int reply;
	int len;
	int i;

	len = string[0];

	reply = 0;
	for (i=0; i<len; i++)
	{
		reply ^= string[i+1];
		if (reply & 1)
			reply = ((reply >> 1) & 0x7fff) | ~0x7fff;
		else
			reply = ((reply >> 1) & 0x7fff);
		if (! (reply & 0x8000))
			reply = - reply;
	}

	return reply;
}
#endif

#if 0
#pragma mark -
#pragma mark HFS IMPLEMENTATION
#endif

/*
    HFS extents B-tree key
*/
struct hfs_extentKey
{
	UINT8    keyLength;     /* length of key, excluding this field */
	UINT8    forkType;      /* 0 = data fork, FF = resource fork */
	UINT32BE fileID;        /* file ID */
	UINT16BE startBlock;    /* first file allocation block number in this extent */
};
enum
{
	keyLength_hfs_extentKey = sizeof(hfs_extentKey) - sizeof(UINT8)
};

/*
    HFS catalog B-tree key
*/
struct hfs_catKey
{
	UINT8    keyLen;        /* key length */
	UINT8    resrv1;        /* reserved */
	UINT32BE parID;         /* parent directory ID */
	mac_str31 cName;        /* catalog node name */
							/* note that in index nodes, it is a mac_str31, but
							    in leaf keys it's a variable-length string */
};

/*
    HFS catalog data record for a folder - 70 bytes
*/
struct hfs_catFolderData
{
	UINT16BE recordType;        /* record type */
	UINT16BE flags;             /* folder flags */
	UINT16BE valence;           /* folder valence */
	UINT32BE folderID;          /* folder ID */
	UINT32BE createDate;        /* date and time of creation */
	UINT32BE modifyDate;        /* date and time of last modification */
	UINT32BE backupDate;        /* date and time of last backup */
	mac_DInfo userInfo;         /* Finder information */
	mac_DXInfo finderInfo;      /* additional Finder information */
	UINT32BE reserved[4];       /* reserved - set to zero */
};

/*
    HFS catalog data record for a file - 102 bytes
*/
struct hfs_catFileData
{
	UINT16BE recordType;        /* record type */
	UINT8    flags;             /* file flags */
	UINT8    fileType;          /* file type (reserved, always 0?) */
	mac_FInfo userInfo;         /* Finder information */
	UINT32BE fileID;            /* file ID */
	UINT16BE dataStartBlock;    /* not used - set to zero */
	UINT32BE dataLogicalSize;   /* logical EOF of data fork */
	UINT32BE dataPhysicalSize;  /* physical EOF of data fork */
	UINT16BE rsrcStartBlock;    /* not used - set to zero */
	UINT32BE rsrcLogicalSize;   /* logical EOF of resource fork */
	UINT32BE rsrcPhysicalSize;  /* physical EOF of resource fork */
	UINT32BE createDate;        /* date and time of creation */
	UINT32BE modifyDate;        /* date and time of last modification */
	UINT32BE backupDate;        /* date and time of last backup */
	mac_FXInfo finderInfo;      /* additional Finder information */
	UINT16BE clumpSize;         /* file clump size (not used) */
	hfs_extent_3 dataExtents;   /* first data fork extent record */
	hfs_extent_3 rsrcExtents;   /* first resource fork extent record */
	UINT32BE reserved;          /* reserved - set to zero */
};

/*
    HFS catalog data record for a thread - 46 bytes

    The key for a thread record features the CNID of the item and an empty
    name, instead of the CNID of the parent and the item name.
*/
struct hfs_catThreadData
{
	UINT16BE recordType;        /* record type */
	UINT32BE reserved[2];       /* reserved - set to zero */
	UINT32BE parID;             /* parent ID for this catalog node */
	mac_str31 nodeName;         /* name of this catalog node */
};

/*
    union for all types at once
*/
union hfs_catData
{
	UINT16BE dataType;
	hfs_catFolderData folder;
	hfs_catFileData file;
	hfs_catThreadData thread;
};

/*
    HFS catalog record types
*/
enum
{
	hcrt_Folder         = 0x0100,   /* Folder record */
	hcrt_File           = 0x0200,   /* File record */
	hcrt_FolderThread   = 0x0300,   /* Folder thread record */
	hcrt_FileThread     = 0x0400    /* File thread record */
};

/*
    Catalog file record flags

    This is similar to the MFS catalog flag field, but the "thread exists" flag
    (0x02) is specific to HFS/HFS+, whereas the "Record in use" flag (0x80) is
    only used by MFS.
*/
enum
{
	cfrf_fileLocked     = 0x01,     /* file is locked and cannot be written to */
	cfrf_threadExists   = 0x02      /* a file thread record exists for this file */
};

/*
    BT functions used by HFS functions
*/
struct BT_leaf_rec_enumerator
{
	mac_BTref *BTref;
	UINT32 cur_node;
	int cur_rec;
};

static imgtoolerr_t BT_open(mac_BTref *BTref, int (*key_compare_func)(const void *key1, const void *key2), int is_extent);
static void BT_close(mac_BTref *BTref);
static imgtoolerr_t BT_search_leaf_rec(mac_BTref *BTref, const void *search_key,
								UINT32 *node_ID, int *record_ID,
								void **record_ptr, int *record_len,
								int search_exact_match, int *match_found);
static imgtoolerr_t BT_get_keyed_record_data(mac_BTref *BTref, void *rec_ptr, int rec_len, void **data_ptr, int *data_len);
static imgtoolerr_t BT_leaf_rec_enumerator_open(mac_BTref *BTref, BT_leaf_rec_enumerator *enumerator);
static imgtoolerr_t BT_leaf_rec_enumerator_read(BT_leaf_rec_enumerator *enumerator, void **record_ptr, int *rec_len);

struct hfs_cat_enumerator
{
	struct mac_l2_imgref *l2_img;
	BT_leaf_rec_enumerator BT_enumerator;
	UINT32 parID;
};

/*
    hfs_open_extents_file

    Open the file extents B-tree file

    l2_img (I/O): level-2 image reference
    mdb (I): copy of the MDB block
    fileref (O): mac open file reference

    Return imgtool error code
*/
static imgtoolerr_t hfs_open_extents_file(struct mac_l2_imgref *l2_img, const struct hfs_mdb *mdb, struct mac_fileref *fileref)
{
	assert(l2_img->format == L2I_HFS);

	fileref->l2_img = l2_img;

	fileref->fileID = 3;
	fileref->forkType = (mac_forkID)0x00;

	fileref->eof = fileref->pLen = get_UINT32BE(mdb->xtFlSize);
	memcpy(fileref->hfs.extents, mdb->xtExtRec, sizeof(hfs_extent_3));

	fileref->crPs = 0;

	fileref->reload_buf = TRUE;

	return IMGTOOLERR_SUCCESS;
}

/*
    hfs_open_cat_file

    Open the disk catalog B-tree file

    l2_img (I/O): level-2 image reference
    mdb (I): copy of the MDB block
    fileref (O): mac open file reference

    Return imgtool error code
*/
static imgtoolerr_t hfs_open_cat_file(struct mac_l2_imgref *l2_img, const struct hfs_mdb *mdb, struct mac_fileref *fileref)
{
	assert(l2_img->format == L2I_HFS);

	fileref->l2_img = l2_img;

	fileref->fileID = 4;
	fileref->forkType = (mac_forkID)0x00;

	fileref->eof = fileref->pLen = get_UINT32BE(mdb->ctFlSize);
	memcpy(fileref->hfs.extents, mdb->ctExtRec, sizeof(hfs_extent_3));

	fileref->crPs = 0;

	fileref->reload_buf = TRUE;

	return IMGTOOLERR_SUCCESS;
}

/*
    hfs_extentKey_compare

    key compare function for file extents B-tree

    p1 (I): pointer to first key
    p2 (I): pointer to second key

    Return a zero the two keys are equal, a negative value if the key pointed
    to by p1 is less than the key pointed to by p2, and a positive value if the
    key pointed to by p1 is greater than the key pointed to by p2.
*/
static int hfs_extentKey_compare(const void *p1, const void *p2)
{
	const hfs_extentKey *key1 = (const hfs_extentKey*)p1;
	const hfs_extentKey *key2 = (const hfs_extentKey*)p2;

	/* let's keep it simple for now */
	return memcmp(key1, key2, sizeof(hfs_extentKey));
}

/*
    hfs_catKey_compare

    key compare function for disk catalog B-tree

    p1 (I): pointer to first key
    p2 (I): pointer to second key

    Return a zero the two keys are equal, a negative value if the key pointed
    to by p1 is less than the key pointed to by p2, and a positive value if the
    key pointed to by p1 is greater than the key pointed to by p2.
*/
static int hfs_catKey_compare(const void *p1, const void *p2)
{
	const hfs_catKey *key1 = (const hfs_catKey *)p1;
	const hfs_catKey *key2 = (const hfs_catKey *)p2;

	if (get_UINT32BE(key1->parID) != get_UINT32BE(key2->parID))
		return (get_UINT32BE(key1->parID) < get_UINT32BE(key2->parID)) ? -1 : +1;

	return mac_stricmp(key1->cName, key2->cName);
}

/*
    hfs_image_open

    Open a HFS image.  Image must already be open on level 1.

    l2_img (I/O): level-2 image reference to open (l1_img and format fields
        must be initialized)
    img_open_buf (I): buffer with the MDB block

    Return imgtool error code
*/
static imgtoolerr_t hfs_image_open(imgtool_image *image, imgtool_stream *stream)
{
	imgtoolerr_t err;
	struct mac_l2_imgref *l2_img;
	img_open_buf buf_local;
	img_open_buf *buf;

	l2_img = get_imgref(image);
	l2_img->l1_img.image = image;
	l2_img->l1_img.heads = 2;
	l2_img->format = L2I_HFS;

	/* read MDB */
	err = image_read_block(&l2_img->l1_img, 2, &buf_local.raw);
	if (err)
		return err;
	buf = &buf_local;

	/* check signature word */
	if ((buf->hfs_mdb.sigWord[0] != 0x42) || (buf->hfs_mdb.sigWord[1] != 0x44)
			|| (buf->hfs_mdb.VN[0] > 27))
		return IMGTOOLERR_CORRUPTIMAGE;

	l2_img->u.hfs.VBM_start = get_UINT16BE(buf->hfs_mdb.VBMSt);

	l2_img->numABs = get_UINT16BE(buf->hfs_mdb.nmAlBlks);
	if (get_UINT32BE(buf->hfs_mdb.alBlkSiz) % 512)
		return IMGTOOLERR_CORRUPTIMAGE;
	l2_img->blocksperAB = get_UINT32BE(buf->hfs_mdb.alBlkSiz) / 512;
	l2_img->u.hfs.ABStart = get_UINT16BE(buf->hfs_mdb.alBlSt);

	l2_img->nxtCNID = get_UINT32BE(buf->hfs_mdb.nxtCNID);

	l2_img->freeABs = get_UINT16BE(buf->hfs_mdb.freeABs);

	mac_strcpy(l2_img->u.hfs.volname, buf->hfs_mdb.VN);

	/* open extents and catalog BT */
	err = hfs_open_extents_file(l2_img, &buf->hfs_mdb, &l2_img->u.hfs.extents_BT.fileref);
	if (err)
		return err;
	err = BT_open(&l2_img->u.hfs.extents_BT, hfs_extentKey_compare, TRUE);
	if (err)
		return err;
	if ((l2_img->u.hfs.extents_BT.attributes & btha_bigKeysMask)
			/*|| (l2_img->u.hfs.extents_BT.attributes & kBTVariableIndexKeysMask)*/
			|| (l2_img->u.hfs.extents_BT.maxKeyLength != 7))
	{   /* This is not supported by the HFS format */
		/* Variable Index keys are not supported either, but hopefully it will
		not break this imgtool module if it set (though it would probably break
		a real macintosh) */
		BT_close(&l2_img->u.hfs.extents_BT);
		return IMGTOOLERR_CORRUPTIMAGE;
	}
	err = hfs_open_cat_file(l2_img, &buf->hfs_mdb, &l2_img->u.hfs.cat_BT.fileref);
	if (err)
	{
		BT_close(&l2_img->u.hfs.extents_BT);
		return err;
	}
	err = BT_open(&l2_img->u.hfs.cat_BT, hfs_catKey_compare, FALSE);
	if (err)
	{
		return err;
	}
	if ((l2_img->u.hfs.cat_BT.attributes & btha_bigKeysMask)
			/*|| (l2_img->u.hfs.cat_BT.attributes & kBTVariableIndexKeysMask)*/
			|| (l2_img->u.hfs.cat_BT.maxKeyLength != 37))
	{   /* This is not supported by the HFS format */
		/* Variable Index keys are not supported either, but hopefully it will
		not break this imgtool module if it set (though it would probably break
		a real macintosh) */
		BT_close(&l2_img->u.hfs.extents_BT);
		BT_close(&l2_img->u.hfs.cat_BT);
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	/* extract volume bitmap */
	{
		int byte_len = (l2_img->numABs + 7) / 8;
		int cur_byte = 0;
		int cur_block = l2_img->u.hfs.VBM_start;

		while (cur_byte < byte_len)
		{
			/* read next block */
			err = image_read_block(&l2_img->l1_img, cur_block, buf->raw);
			if (err)
				return err;
			cur_block++;

			/* append this block to VBM */
			memcpy(l2_img->u.hfs.VBM+cur_byte, buf->raw, 512);
			cur_byte += 512;
		}
	}

	return IMGTOOLERR_SUCCESS;
}

#ifdef UNUSED_FUNCTION
/*
    hfs_image_close

    Close a HFS image.

    l2_img (I/O): level-2 image reference
*/
static void hfs_image_close(struct mac_l2_imgref *l2_img)
{
	assert(l2_img->format == L2I_HFS);

	BT_close(&l2_img->u.hfs.extents_BT);
	BT_close(&l2_img->u.hfs.cat_BT);
}
#endif

/*
    hfs_get_cat_record_data

    extract data from a catalog B-tree leaf record

    l2_img (I/O): level-2 image reference
    rec_raw (I): pointer to record key and data, as returned by
        BT_node_get_keyed_record
    rec_len (I): total length of record, as returned by
        BT_node_get_keyed_record
    rec_key (O): set to point to record key
    rec_data (O): set to point to record data

    Return imgtool error code
*/
static imgtoolerr_t hfs_get_cat_record_data(struct mac_l2_imgref *l2_img, void *rec_raw, int rec_len, hfs_catKey **rec_key, hfs_catData **rec_data)
{
	hfs_catKey *lrec_key;
	void *rec_data_raw;
	hfs_catData *lrec_data;
	int rec_data_len;
	int min_data_size;
	imgtoolerr_t err;


	assert(l2_img->format == L2I_HFS);

	lrec_key = (hfs_catKey*)rec_raw;
	/* check that key is long enough to hold it all */
	if ((lrec_key->keyLen+1) < (offsetof(hfs_catKey, cName) + lrec_key->cName[0] + 1))
		return IMGTOOLERR_CORRUPTIMAGE;

	/* get pointer to record data */
	err = BT_get_keyed_record_data(&l2_img->u.hfs.cat_BT, rec_raw, rec_len, &rec_data_raw, &rec_data_len);
	if (err)
		return err;
	lrec_data = (hfs_catData*)rec_data_raw;

	/* extract record type */
	if (rec_data_len < 2)
		return IMGTOOLERR_CORRUPTIMAGE;

	/* check that the record is large enough for its type */
	switch (get_UINT16BE(lrec_data->dataType))
	{
	case hcrt_Folder:
		min_data_size = sizeof(hfs_catFolderData);
		break;

	case hcrt_File:
		min_data_size = sizeof(hfs_catFileData);
		break;

	case hcrt_FolderThread:
	case hcrt_FileThread:
		min_data_size = sizeof(hfs_catThreadData);
		break;

	default:
		/* records of unknown type can be safely ignored */
		min_data_size = 0;
		break;
	}

	if (rec_data_len < min_data_size)
		return IMGTOOLERR_CORRUPTIMAGE;

	if (rec_key)
		*rec_key = lrec_key;
	if (rec_data)
		*rec_data = lrec_data;

	return IMGTOOLERR_SUCCESS;
}

/*
    hfs_cat_open

    Open an enumerator on the disk catalog

    l2_img (I/O): level-2 image reference
    enumerator (O): open catalog enumerator reference

    Return imgtool error code
*/
static imgtoolerr_t hfs_cat_open(struct mac_l2_imgref *l2_img, const char *path, hfs_cat_enumerator *enumerator)
{
	imgtoolerr_t err;
	UINT32 parID;
	mac_str255 filename;
	mac_dirent cat_info;

	assert(l2_img->format == L2I_HFS);

	/* resolve path and fetch file info from directory/catalog */
	err = mac_lookup_path(l2_img, path, &parID, filename, &cat_info, FALSE);
	if (err)
		return err;
	if (cat_info.dataRecType != hcrt_Folder)
		return IMGTOOLERR_FILENOTFOUND;

	enumerator->l2_img = l2_img;
	enumerator->parID = parID;

	return BT_leaf_rec_enumerator_open(&l2_img->u.hfs.cat_BT, &enumerator->BT_enumerator);
}

/*
    hfs_cat_read

    Enumerate the disk catalog

    enumerator (I/O): open catalog enumerator reference
    rec_key (O): set to point to record key
    rec_data (O): set to point to record data

    Return imgtool error code
*/
static imgtoolerr_t hfs_cat_read(hfs_cat_enumerator *enumerator, hfs_catKey **rec_key, hfs_catData **rec_data)
{
	void *rec;
	int rec_len = 0;
	imgtoolerr_t err;


	*rec_key = NULL;
	*rec_data = NULL;

	/* read next record */
	err = BT_leaf_rec_enumerator_read(&enumerator->BT_enumerator, &rec, &rec_len);
	if (err)
		return err;

	/* check EOList condition */
	if (rec == NULL)
		return IMGTOOLERR_SUCCESS;

	/* extract record data */
	err = hfs_get_cat_record_data(enumerator->l2_img, rec, rec_len, rec_key, rec_data);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}

/*
    hfs_cat_search

    Search the catalog for a given file

    l2_img (I/O): level-2 image reference
    parID (I): CNID of file parent directory
    cName (I): file name
    rec_key (O): set to point to record key
    rec_data (O): set to point to record data

    Return imgtool error code
*/
static imgtoolerr_t hfs_cat_search(struct mac_l2_imgref *l2_img, UINT32 parID, const mac_str31 cName, hfs_catKey **rec_key, hfs_catData **rec_data)
{
	hfs_catKey search_key;
	void *rec;
	int rec_len;
	imgtoolerr_t err;

	assert(l2_img->format == L2I_HFS);

	if (cName[0] > 31)
		return IMGTOOLERR_UNEXPECTED;

	/* generate search key */
	search_key.keyLen = search_key.resrv1 = 0;  /* these fields do not matter
                                                to the compare function, so we
                                                don't fill them */
	set_UINT32BE(&search_key.parID, parID);
	mac_strcpy(search_key.cName, cName);

	/* search key */
	err = BT_search_leaf_rec(&l2_img->u.hfs.cat_BT, &search_key, NULL, NULL, &rec, &rec_len, TRUE, NULL);
	if (err)
		return err;

	/* extract record data */
	err = hfs_get_cat_record_data(l2_img, rec, rec_len, rec_key, rec_data);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}

/*
    hfs_lookup_path

    Resolve a file path

    l2_img (I/O): level-2 image reference
    fpath (I): file path (C string)
    parID (O): set to the CNID of the file parent directory
    filename (O): set to the actual name of the file, with capitalization matching
        the one on the volume rather than the one in the fpath parameter (Mac
        string)
    cat_info (O): catalog info for this file extracted from the catalog file
        (may be NULL)

    Return imgtool error code
*/
static imgtoolerr_t hfs_lookup_path(struct mac_l2_imgref *l2_img, const char *fpath, UINT32 *parID, mac_str255 filename, mac_dirent *cat_info)
{
	const char *element_start;
	int element_len;
	mac_str255 mac_element_name;
	//int level;
	imgtoolerr_t err;
	hfs_catKey *catrec_key = NULL;
	hfs_catData *catrec_data = NULL;
	UINT16 dataRecType = hcrt_Folder;

	/* iterate each path element */
	element_start = fpath;
	//level = 0;
	*parID = 2; /* root parID is 2 */

	while(*element_start)
	{
		/* find next path element */
		element_len = strlen(element_start);
		/* decode path element name */
		c_to_mac_strncpy(mac_element_name, element_start, element_len);

		err = hfs_cat_search(l2_img, *parID, mac_element_name, &catrec_key, &catrec_data);
		if (err)
			return err;

		dataRecType = get_UINT16BE(catrec_data->dataType);

		/* regular folder/file name */
		if (dataRecType == hcrt_Folder)
			*parID = get_UINT32BE(catrec_data->folder.folderID);
		else if (element_start[element_len + 1])
			return IMGTOOLERR_BADFILENAME;

		/* iterate */
		element_start += element_len + 1;
	}

	if (catrec_key && (dataRecType == hcrt_File))
	{
		/* save ref */
		*parID = get_UINT32BE(catrec_key->parID);
		mac_strcpy(filename, catrec_key->cName);
	}

	if (cat_info)
	{
		if (catrec_data && (dataRecType == hcrt_File))
		{
			cat_info->flFinderInfo = catrec_data->file.userInfo;
			cat_info->flXFinderInfo = catrec_data->file.finderInfo;
			cat_info->flags = catrec_data->file.flags;
			cat_info->fileID = get_UINT32BE(catrec_data->file.fileID);
			cat_info->dataLogicalSize = get_UINT32BE(catrec_data->file.dataLogicalSize);
			cat_info->dataPhysicalSize = get_UINT32BE(catrec_data->file.dataPhysicalSize);
			cat_info->rsrcLogicalSize = get_UINT32BE(catrec_data->file.rsrcLogicalSize);
			cat_info->rsrcPhysicalSize = get_UINT32BE(catrec_data->file.rsrcPhysicalSize);
			cat_info->createDate = get_UINT32BE(catrec_data->file.createDate);
			cat_info->modifyDate = get_UINT32BE(catrec_data->file.modifyDate);
		}
		else
		{
			memset(cat_info, 0, sizeof(*cat_info));
		}
		cat_info->dataRecType = dataRecType;
	}

	return IMGTOOLERR_SUCCESS;
}

/*
    hfs_file_open_internal

    Open a file fork, given its catalog entry.  This function should not be
    called directly: call hfs_file_open instead.

    l2_img (I/O): level-2 image reference
    file_rec (I): catalog entry for the file to open
    mac_forkID (I): tells which fork should be opened
    fileref (O): mac file reference to open

    Return imgtool error code
*/
static imgtoolerr_t hfs_file_open_internal(struct mac_l2_imgref *l2_img, const hfs_catFileData *file_rec, mac_forkID fork, struct mac_fileref *fileref)
{
	assert(l2_img->format == L2I_HFS);

	fileref->l2_img = l2_img;

	fileref->fileID = get_UINT32BE(file_rec->fileID);
	fileref->forkType = fork;

	switch (fork)
	{
	case data_fork:
		fileref->eof = get_UINT32BE(file_rec->dataLogicalSize);
		fileref->pLen = get_UINT32BE(file_rec->dataPhysicalSize);
		memcpy(fileref->hfs.extents, file_rec->dataExtents, sizeof(hfs_extent_3));
		break;

	case rsrc_fork:
		fileref->eof = get_UINT32BE(file_rec->rsrcLogicalSize);
		fileref->pLen = get_UINT32BE(file_rec->rsrcPhysicalSize);
		memcpy(fileref->hfs.extents, file_rec->rsrcExtents, sizeof(hfs_extent_3));
		break;
	}

	fileref->crPs = 0;

	fileref->reload_buf = TRUE;

	return IMGTOOLERR_SUCCESS;
}

/*
    hfs_file_open

    Open a file located on a HFS volume.  This function should not be called
    directly: call mac_file_open instead.

    l2_img (I/O): level-2 image reference
    parID (I): CNID of file parent directory
    filename (I): name of the file (Mac string)
    mac_forkID (I): tells which fork should be opened
    fileref (O): mac file reference to open

    Return imgtool error code
*/
static imgtoolerr_t hfs_file_open(struct mac_l2_imgref *l2_img, UINT32 parID, const mac_str255 filename, mac_forkID fork, struct mac_fileref *fileref)
{
	hfs_catKey *catrec_key;
	hfs_catData *catrec_data;
	UINT16 dataRecType;
	imgtoolerr_t err;

	/* lookup file in catalog */
	err = hfs_cat_search(l2_img, parID, filename, &catrec_key, &catrec_data);
	if (err)
		return err;

	dataRecType = get_UINT16BE(catrec_data->dataType);

	/* file expected */
	if (dataRecType != hcrt_File)
		return IMGTOOLERR_BADFILENAME;

	fileref->hfs.parID = get_UINT32BE(catrec_key->parID);
	mac_strcpy(fileref->hfs.filename, catrec_key->cName);

	/* open it */
	return hfs_file_open_internal(l2_img, &catrec_data->file, fork, fileref);
}

/*
    hfs_file_get_nth_block_address

    Get the disk block address of a given block in an open file on a MFS image.
    Called by macintosh file code.

    fileref (I/O): open mac file reference
    block_num (I): file block index
    block_address (O): disk block address for the file block

    Return imgtool error code
*/
static imgtoolerr_t hfs_file_get_nth_block_address(struct mac_fileref *fileref, UINT32 block_num, UINT32 *block_address)
{
	UINT32 AB_num;
	UINT32 cur_AB;
	UINT32 i;
	void *cur_extents_raw;
	hfs_extent *cur_extents;
	int cur_extents_len;
	void *extents_BT_rec;
	int extents_BT_rec_len;
	imgtoolerr_t err;
	UINT16 AB_address;

	assert(fileref->l2_img->format == L2I_HFS);

	AB_num = block_num / fileref->l2_img->blocksperAB;
	cur_AB = 0;
	cur_extents = fileref->hfs.extents;

	/* first look in catalog tree extents */
	for (i=0; i<3; i++)
	{
		if (AB_num < cur_AB+get_UINT16BE(cur_extents[i].numABlks))
			break;
		cur_AB += get_UINT16BE(cur_extents[i].numABlks);
	}
	if (i == 3)
	{
		/* extent not found: read extents record from extents BT */
		hfs_extentKey search_key;
		hfs_extentKey *found_key;

		search_key.keyLength = keyLength_hfs_extentKey;
		search_key.forkType = fileref->forkType;
		set_UINT32BE(&search_key.fileID, fileref->fileID);
		set_UINT16BE(&search_key.startBlock, AB_num);

		/* search for the record with the largest key lower than or equal to
		search_key.  The keys are constructed in such a way that, if a record
		includes AB_num, it is that one. */
		err = BT_search_leaf_rec(&fileref->l2_img->u.hfs.extents_BT, &search_key,
										NULL, NULL, &extents_BT_rec, &extents_BT_rec_len,
										FALSE, NULL);
		if (err)
			return err;

		if (extents_BT_rec == NULL)
			return IMGTOOLERR_CORRUPTIMAGE;

		found_key = (hfs_extentKey*)extents_BT_rec;
		/* check that this record concerns the correct file */
		if ((found_key->forkType != fileref->forkType)
			|| (get_UINT32BE(found_key->fileID) != fileref->fileID))
			return IMGTOOLERR_CORRUPTIMAGE;

		/* extract start AB */
		cur_AB = get_UINT16BE(found_key->startBlock);
		/* get extents pointer */
		err = BT_get_keyed_record_data(&fileref->l2_img->u.hfs.extents_BT, extents_BT_rec, extents_BT_rec_len, &cur_extents_raw, &cur_extents_len);
		if (err)
			return err;
		if (cur_extents_len < 3*sizeof(hfs_extent))
			return IMGTOOLERR_CORRUPTIMAGE;
		cur_extents = (hfs_extent*)cur_extents_raw;

		/* pick correct extent in record */
		for (i=0; i<3; i++)
		{
			if (AB_num < cur_AB+get_UINT16BE(cur_extents[i].numABlks))
				break;
			cur_AB += get_UINT16BE(cur_extents[i].numABlks);
		}
		if (i == 3)
			/* extent not found */
			return IMGTOOLERR_CORRUPTIMAGE;
	}

	AB_address = get_UINT16BE(cur_extents[i].stABN) + (AB_num-cur_AB);

	if (AB_address >= fileref->l2_img->numABs)
		return IMGTOOLERR_CORRUPTIMAGE;

	*block_address = fileref->l2_img->u.hfs.ABStart + AB_address * fileref->l2_img->blocksperAB
						+ block_num % fileref->l2_img->blocksperAB;

	return IMGTOOLERR_SUCCESS;
}

#if 0
#pragma mark -
#pragma mark B-TREE IMPLEMENTATION
#endif

/*
    B-tree (Balanced search tree) files are used by the HFS and HFS+ file
    systems: the Extents and Catalog files are both B-Tree.

    Note that these B-trees are B+-trees: data is only on the leaf level, and
    nodes located on the same level are also linked sequentially, which allows
    fast sequenctial access to the catalog file.

    These files are normal files, except for the fact that they are not
    referenced from the catalog but the MDB.  They are allocated in fixed-size
    records of 512 bytes (HFS).  (HFS+ supports any power of two from 512
    through 32768, and uses a default of 1024 for Extents, and 4096 for both
    Catalog and Attributes.)

    Nodes can contain any number of records.  The nodes can be of any of four
    types: header node (unique node with b-tree information, pointer to root
    node and start of the node allocation bitmap), map nodes (which are created
    when the node allocation bitmap outgrows the header node), index nodes
    (root and branch node that enable to efficiently search the leaf nodes for
    a specific key value), and leaf nodes (which hold the actual user data
    records with keys and data).  The first node is always a header node.
    Other nodes can be of any of the 3 other type, or they can be free.
*/

/*
    BTNodeHeader

    Header of a node record
*/
struct BTNodeHeader
{
	UINT32BE fLink;         /* (index of) next node at this level */
	UINT32BE bLink;         /* (index of) previous node at this level */
	UINT8    kind;          /* kind of node (leaf, index, header, map) */
	UINT8    height;        /* zero for header, map; 1 for leaf, 2 through
                                treeDepth for index (child is one LESS than
                                parent, whatever IM says) */
	UINT16BE numRecords;    /* number of records in this node */
	UINT16BE reserved;      /* reserved; set to zero */
};

/*
    Constants for BTNodeHeader kind field
*/
enum
{
	btnk_leafNode   = 0xff, /* leaf nodes hold the actual user data records
                                with keys and data */
	btnk_indexNode  = 0,    /* root and branch node that enable to efficiently
                                search the leaf nodes for a specific key value */
	btnk_headerNode = 1,    /* unique node with b-tree information, pointer to
                                root node and start of the node allocation
                                bitmap */
	btnk_mapNode    = 2     /* map nodes are created when the node allocation
                                bitmap outgrows the header node */
};

/*
    BTHeaderRecord: first record of a B-tree header node (second record is
    unused, and third is node allocation bitmap).
*/
struct BTHeaderRecord
{
	UINT16BE treeDepth;     /* maximum height (usually leaf nodes) */
	UINT32BE rootNode;      /* node number of root node */
	UINT32BE leafRecords;   /* number of leaf records in all leaf nodes */
	UINT32BE firstLeafNode; /* node number of first leaf node */
	UINT32BE lastLeafNode;  /* node number of last leaf node */
	UINT16BE nodeSize;      /* size of a node, in bytes */
	UINT16BE maxKeyLength;  /* maximum length of data (index + leaf) record keys;
                                length of all index record keys if
                                btha_variableIndexKeysMask attribute flag is not set */
	UINT32BE totalNodes;    /* total number of nodes in tree */
	UINT32BE freeNodes;     /* number of unused (free) nodes in tree */

	UINT16BE reserved1;     /* unused */
	UINT32BE clumpSize;     /* used in some HFS implementations? (reserved in
                                early HFS implementations, and in HFS Plus) */
	UINT8    btreeType;     /* reserved - set to 0 */
	UINT8    reserved2;     /* reserved */
	UINT32BE attributes;    /* persistent attributes about the tree */
	UINT32BE reserved3[16]; /* reserved */
};

static imgtoolerr_t BT_check(mac_BTref *BTref, int is_extent);

/*
    BT_open

    Open a file as a B-tree.  The file must be already open as a macintosh
    file.

    BTref (I/O): B-tree file handle to open (BTref->fileref must have been
        open previously)
    key_compare_func (I): function that compares two keys
    is_extent (I): TRUE if we are opening the extent B-tree (we want to do
        extra checks in this case because the extent B-Tree may include extent
        records for the extent B-tree itself, and if an extent record for the
        extent B-tree is located in an extent that has not been defined by
        previous extent records, then we can never retreive this extent record)

    Return imgtool error code
*/
static imgtoolerr_t BT_open(mac_BTref *BTref, int (*key_compare_func)(const void *key1, const void *key2), int is_extent)
{
	imgtoolerr_t err;
	BTNodeHeader node_header;
	BTHeaderRecord header_rec;

	/* seek to node 0 */
	err = mac_file_seek(&BTref->fileref, 0);
	if (err)
		return err;

	/* read node header */
	err = mac_file_read(&BTref->fileref, sizeof(node_header), &node_header);
	if (err)
		return err;

	if ((node_header.kind != btnk_headerNode) || (get_UINT16BE(node_header.numRecords) < 3)
			|| (node_header.height != 0))
		return IMGTOOLERR_CORRUPTIMAGE; /* right??? */

	/* CHEESY HACK: we assume that the header record immediately follows the
	node header.  This is because we need to know the node length to know where
	the record pointers are located, but we need to read the header record to
	know the node length. */
	err = mac_file_read(&BTref->fileref, sizeof(header_rec), &header_rec);
	if (err)
		return err;

	BTref->nodeSize = get_UINT16BE(header_rec.nodeSize);
	BTref->rootNode = get_UINT32BE(header_rec.rootNode);
	BTref->firstLeafNode = get_UINT32BE(header_rec.firstLeafNode);
	BTref->attributes = get_UINT32BE(header_rec.attributes);
	BTref->treeDepth = get_UINT16BE(header_rec.treeDepth);
	BTref->maxKeyLength = get_UINT16BE(header_rec.maxKeyLength);

	BTref->key_compare_func = key_compare_func;

	BTref->node_buf = malloc(BTref->nodeSize);
	if (!BTref->node_buf)
		return IMGTOOLERR_OUTOFMEMORY;

	if (BTREE_CHECKS)
	{
		/* optional: check integrity of B-tree */
		err = BT_check(BTref, is_extent);
		if (err)
			return err;
	}

	return IMGTOOLERR_SUCCESS;
}

/*
    BT_close

    Close a B-tree

    BTref (I/O): open B-tree file handle
*/
static void BT_close(mac_BTref *BTref)
{
	free(BTref->node_buf);
}

/*
    BT_read_node

    Read a node from a B-tree

    BTref (I/O): open B-tree file handle
    node_ID (I): index of the node to read
    expected_kind (I): kind of the node to read
    expected_depth (I): depth of the node to read
    dest (O): destination buffer

    Return imgtool error code
*/
static imgtoolerr_t BT_read_node(mac_BTref *BTref, UINT32 node_ID, int expected_kind, int expected_depth, void *dest)
{
	imgtoolerr_t err;

	/* seek to node */
	err = mac_file_seek(&BTref->fileref, node_ID*BTref->nodeSize);
	if (err)
		return err;

	/* read it */
	err = mac_file_read(&BTref->fileref, BTref->nodeSize, dest);
	if (err)
		return err;

	/* check node kind and depth */
	if ((((BTNodeHeader *) dest)->kind != expected_kind)
			|| (((BTNodeHeader *) dest)->height != expected_depth))
		return IMGTOOLERR_CORRUPTIMAGE;

	return IMGTOOLERR_SUCCESS;
}

/*
    BT_node_get_record

    Extract a raw record from a B-tree node

    BTref (I/O): open B-tree file handle
    node_buf (I): buffer with the node the record should be extracted from
    recnum (I): index of record to read
    rec_ptr (O): set to point to start of record (key + data)
    rec_len (O): set to total length of record (key + data)

    Return imgtool error code
*/
static imgtoolerr_t BT_node_get_record(mac_BTref *BTref, void *node_buf, unsigned recnum, void **rec_ptr, int *rec_len)
{
	UINT16 node_numRecords = get_UINT16BE(((BTNodeHeader *) node_buf)->numRecords);
	UINT16 offset;
	UINT16 next_offset;

	if (recnum >= node_numRecords)
		return IMGTOOLERR_UNEXPECTED;

	offset = get_UINT16BE(((UINT16BE *)((UINT8 *) node_buf + BTref->nodeSize))[-recnum-1]);
	next_offset = get_UINT16BE(((UINT16BE *)((UINT8 *) node_buf + BTref->nodeSize))[-recnum-2]);

	if ((offset < sizeof(BTNodeHeader)) || (offset > BTref->nodeSize-2*node_numRecords)
			|| (next_offset < sizeof(BTNodeHeader)) || (next_offset > BTref->nodeSize-2*node_numRecords)
			|| (offset & 1) || (next_offset & 1)
			|| (offset > next_offset))
		return IMGTOOLERR_CORRUPTIMAGE;

	*rec_ptr = (UINT8 *)node_buf + offset;
	*rec_len = next_offset - offset;

	return IMGTOOLERR_SUCCESS;
}

/*
    BT_node_get_keyed_record

    Extract a keyed record from a B-tree node.  Equivalent to
    BT_node_get_record, only we do extra checks.

    BTref (I/O): open B-tree file handle
    node_buf (I): buffer with the node the record should be extracted from
    node_is_index (I): TRUE if node is index node
    recnum (I): index of record to read
    rec_ptr (O): set to point to start of record (key + data)
    rec_len (O): set to total length of record (key + data)

    Return imgtool error code
*/
static imgtoolerr_t BT_node_get_keyed_record(mac_BTref *BTref, void *node_buf, int node_is_index, unsigned recnum, void **rec_ptr, int *rec_len)
{
	imgtoolerr_t err;
	void *lrec_ptr;
	int lrec_len;
	int key_len;

	/* extract record */
	err = BT_node_get_record(BTref, node_buf, recnum, &lrec_ptr, &lrec_len);
	if (err)
		return err;

	/* read key len */
	key_len = (BTref->attributes & btha_bigKeysMask)
				? get_UINT16BE(* (UINT16BE *)lrec_ptr)
				: (* (UINT8 *)lrec_ptr);

	/* check that key fits in record */
	if ((key_len + ((BTref->attributes & btha_bigKeysMask) ? 2 : 1)) > lrec_len)
		/* hurk! */
		return IMGTOOLERR_CORRUPTIMAGE;

	if (key_len > BTref->maxKeyLength)
		return IMGTOOLERR_CORRUPTIMAGE;

	if (node_is_index && (! (BTref->attributes & btha_variableIndexKeysMask)) && (key_len != BTref->maxKeyLength))
		return IMGTOOLERR_CORRUPTIMAGE;

	if (rec_ptr)
		*rec_ptr = lrec_ptr;
	if (rec_len)
		*rec_len = lrec_len;

	return IMGTOOLERR_SUCCESS;
}

/*
    BT_get_keyed_record_data

    extract data from a keyed record

    BTref (I/O): open B-tree file handle
    rec_ptr (I): point to start of record (key + data)
    rec_len (I): total length of record (key + data)
    data_ptr (O): set to point to record data
    data_len (O): set to length of record data

    Return imgtool error code
*/
static imgtoolerr_t BT_get_keyed_record_data(mac_BTref *BTref, void *rec_ptr, int rec_len, void **data_ptr, int *data_len)
{
	int lkey_len;
	int data_offset;

	/* read key len */
	lkey_len = (BTref->attributes & btha_bigKeysMask)
				? get_UINT16BE(* (UINT16BE *)rec_ptr)
				: (* (UINT8 *)rec_ptr);

	/* compute offset to data record */
	data_offset = lkey_len + ((BTref->attributes & btha_bigKeysMask) ? 2 : 1);
	if (data_offset > rec_len)
		/* hurk! */
		return IMGTOOLERR_CORRUPTIMAGE;
	/* fix alignment */
	if (data_offset & 1)
		data_offset++;

	if (data_ptr)
		*data_ptr = (UINT8 *)rec_ptr + data_offset;
	if (data_len)
		*data_len = (rec_len > data_offset) ? rec_len-data_offset : 0;

	return IMGTOOLERR_SUCCESS;
}

/*
    BT_check

    Check integrity of a complete B-tree

    BTref (I/O): open B-tree file handle
    is_extent (I): TRUE if we are opening the extent B-tree (we want to do
        extra checks in this case because the extent B-Tree may include extent
        records for the extent B-tree itself, and if an extent record for the
        extent B-tree is located in an extent that has not been defined by
        previous extent records, then we can never retreive this extent record)

    Return imgtool error code
*/
struct data_nodes_t
{
	void *buf;
	UINT32 node_num;
	UINT32 cur_rec;
	UINT32 num_recs;
};
static imgtoolerr_t BT_check(mac_BTref *BTref, int is_extent)
{
	UINT16 node_numRecords;
	BTHeaderRecord *header_rec;
	UINT8 *bitmap;

	data_nodes_t *data_nodes;
	int i, j;
	UINT32 cur_node, prev_node;
	void *rec1, *rec2;
	int rec1_len, rec2_len;
	void *rec1_data;
	int rec1_data_len;
	UINT32 totalNodes, lastLeafNode;
	UINT32 freeNodes;
	int compare_result;
	UINT32 map_count, map_len;
	UINT32 run_len;
	UINT32 run_bit_len;
	UINT32 actualFreeNodes;
	imgtoolerr_t err;
	UINT32 maxExtentAB = 0, maxExtentNode = 0, extentEOL = 0;   /* if is_extent is TRUE */

	if (is_extent)
	{
		switch (BTref->fileref.l2_img->format)
		{
		case L2I_MFS:
			/* MFS does not feature any extents B-tree! */
			return IMGTOOLERR_UNEXPECTED;

		case L2I_HFS:
			maxExtentAB = 0;
			for (j=0; j<3; j++)
				maxExtentAB += get_UINT16BE(BTref->fileref.hfs.extents[j].numABlks);
			maxExtentNode = (UINT64)maxExtentAB * 512 * BTref->fileref.l2_img->blocksperAB
										/ BTref->nodeSize;
			extentEOL = FALSE;
			break;
		}
	}

	/* read header node */
	if ((! is_extent) || (0 < maxExtentNode))
		err = BT_read_node(BTref, 0, btnk_headerNode, 0, BTref->node_buf);
	else
		err = IMGTOOLERR_CORRUPTIMAGE;
	if (err)
		return err;

	/* check we have enough records */
	node_numRecords = get_UINT16BE(((BTNodeHeader *) BTref->node_buf)->numRecords);
	if (node_numRecords < 3)
		return IMGTOOLERR_CORRUPTIMAGE;

	/* get header record */
	err = BT_node_get_record(BTref, BTref->node_buf, 0, &rec1, &rec1_len);
	if (err)
		return err;
	header_rec = (BTHeaderRecord *)rec1;

	/* check length of header record */
	if (rec1_len < sizeof(BTHeaderRecord))
		return IMGTOOLERR_CORRUPTIMAGE;

	totalNodes = get_UINT32BE(header_rec->totalNodes);
	if (totalNodes == 0)
		/* we need at least one header node */
		return IMGTOOLERR_CORRUPTIMAGE;
	lastLeafNode = get_UINT32BE(header_rec->lastLeafNode);
	freeNodes = get_UINT32BE(header_rec->freeNodes);

	/* check file length */
	if ((BTref->nodeSize * totalNodes) > BTref->fileref.pLen)
		return IMGTOOLERR_CORRUPTIMAGE;

	/* initialize for the function postlog ("bail:" tag) */
	err = IMGTOOLERR_SUCCESS;
	bitmap = NULL;
	data_nodes = NULL;

	/* alloc buffer for reconstructed bitmap */
	map_len = (totalNodes + 7) / 8;
	bitmap = (UINT8*)malloc(map_len);
	if (! bitmap)
		return IMGTOOLERR_OUTOFMEMORY;
	memset(bitmap, 0, map_len);

	/* check B-tree data nodes (i.e. index and leaf nodes) */
	if (BTref->treeDepth == 0)
	{
		/* B-tree is empty */
		if (BTref->rootNode || BTref->firstLeafNode || lastLeafNode)
		{
			err = IMGTOOLERR_OUTOFMEMORY;
			goto bail;
		}
	}
	else
	{
		/* alloc array of buffers for catalog data nodes */
		data_nodes = (data_nodes_t *)malloc(sizeof(data_nodes_t) * BTref->treeDepth);
		if (! data_nodes)
		{
			err = IMGTOOLERR_OUTOFMEMORY;
			goto bail;
		}
		for (i=0; i<BTref->treeDepth; i++)
			data_nodes[i].buf = NULL;   /* required for function postlog to work should next loop fail */
		for (i=0; i<BTref->treeDepth; i++)
		{
			data_nodes[i].buf = malloc(BTref->nodeSize);
			if (!data_nodes[i].buf)
			{
				err = IMGTOOLERR_OUTOFMEMORY;
				goto bail;
			}
		}

		/* read first data nodes */
		cur_node = BTref->rootNode;
		for (i=BTref->treeDepth-1; i>=0; i--)
		{
			/* check node index */
			if (cur_node >= totalNodes)
			{
				err = IMGTOOLERR_CORRUPTIMAGE;
				goto bail;
			}
			/* check that node has not been used for another purpose */
			/* this check is unecessary because the current consistency checks
			that forward and back linking match and that node height is correct
			are enough to detect such errors */
#if 0
			if (bitmap[cur_node >> 3] & (0x80 >> (cur_node & 7)))
			{
				err = IMGTOOLERR_CORRUPTIMAGE;
				goto bail;
			}
#endif
			/* add node in bitmap */
			bitmap[cur_node >> 3] |= (0x80 >> (cur_node & 7));
			/* read node */
			if ((! is_extent) || (cur_node < maxExtentNode))
				err = BT_read_node(BTref, cur_node, i ? btnk_indexNode : btnk_leafNode, i+1, data_nodes[i].buf);
			else
				err = IMGTOOLERR_CORRUPTIMAGE;
			if (err)
				goto bail;
			/* check that it is the first node at this level */
			if (get_UINT32BE(((BTNodeHeader *) data_nodes[i].buf)->bLink))
			{
				err = IMGTOOLERR_CORRUPTIMAGE;
				goto bail;
			}
			/* fill other fields */
			data_nodes[i].node_num = cur_node;
			data_nodes[i].cur_rec = 0;
			data_nodes[i].num_recs = get_UINT16BE(((BTNodeHeader *) data_nodes[i].buf)->numRecords);
			/* check that there is at least one record */
			if (data_nodes[i].num_recs == 0)
			{
				err = IMGTOOLERR_CORRUPTIMAGE;
				goto bail;
			}

			/* iterate to next level if applicable */
			if (i != 0)
			{
				/* extract first record */
				err = BT_node_get_keyed_record(BTref, data_nodes[i].buf, TRUE, 0, &rec1, &rec1_len);
				if (err)
					goto bail;

				/* extract record data ptr */
				err = BT_get_keyed_record_data(BTref, rec1, rec1_len, &rec1_data, &rec1_data_len);
				if (err)
					goto bail;
				if (rec1_data_len < sizeof(UINT32BE))
				{
					err = IMGTOOLERR_CORRUPTIMAGE;
					goto bail;
				}

				/* iterate to next level */
				cur_node = get_UINT32BE(* (UINT32BE *)rec1_data);
			}
		}

		/* check that a) the root node has no successor, and b) that we have really
		read the first leaf node */
		if (get_UINT32BE(((BTNodeHeader *) data_nodes[BTref->treeDepth-1].buf)->fLink)
				|| (cur_node != BTref->firstLeafNode))
		{
			err = IMGTOOLERR_CORRUPTIMAGE;
			goto bail;
		}

		/* check that keys are ordered correctly */
		while (1)
		{
			/* iterate through parent nodes */
			i = 0;
			while ((i<BTref->treeDepth) && ((data_nodes[i].cur_rec == 0) || (data_nodes[i].cur_rec == data_nodes[i].num_recs)))
			{
				/* read next node if necessary */
				if (data_nodes[i].cur_rec == data_nodes[i].num_recs)
				{
					/* get link to next node */
					cur_node = get_UINT32BE(((BTNodeHeader *) data_nodes[i].buf)->fLink);
					if (cur_node == 0)
					{
						if (i == 0)
							/* normal End of List */
							goto end_of_list;
						else
						{
							/* error */
							err = IMGTOOLERR_CORRUPTIMAGE;
							goto bail;
						}
					}
					/* add node in bitmap */
					bitmap[cur_node >> 3] |= (0x80 >> (cur_node & 7));
					/* read node */
					if ((! is_extent) || (cur_node < maxExtentNode))
						err = BT_read_node(BTref, cur_node, i ? btnk_indexNode : btnk_leafNode, i+1, data_nodes[i].buf);
					else
						err = IMGTOOLERR_CORRUPTIMAGE;
					if (err)
						goto bail;
					/* check that backward linking match forward linking */
					if (get_UINT32BE(((BTNodeHeader *) data_nodes[i].buf)->bLink) != data_nodes[i].node_num)
					{
						err = IMGTOOLERR_CORRUPTIMAGE;
						goto bail;
					}
					/* fill other fields */
					data_nodes[i].node_num = cur_node;
					data_nodes[i].cur_rec = 0;
					data_nodes[i].num_recs = get_UINT16BE(((BTNodeHeader *) data_nodes[i].buf)->numRecords);
					/* check that there is at least one record */
					if (data_nodes[i].num_recs == 0)
					{
						err = IMGTOOLERR_CORRUPTIMAGE;
						goto bail;
					}
					/* next test is not necessary because we have checked that
					the root node has no successor */
#if 0
					if (i < BTref->treeDepth-1)
					{
#endif
						data_nodes[i+1].cur_rec++;
#if 0
					}
					else
					{
						err = IMGTOOLERR_CORRUPTIMAGE;
						goto bail;
					}
#endif
				}
				i++;
			}

			if (is_extent && !extentEOL)
			{
				/* extract current leaf record and update maxExtentAB and
				maxExtentNode */
				hfs_extentKey *extentKey;
				hfs_extent *extentData;

				/* extract current leaf record */
				err = BT_node_get_keyed_record(BTref, data_nodes[0].buf, FALSE, data_nodes[0].cur_rec, &rec1, &rec1_len);
				if (err)
					goto bail;

				extentKey = (hfs_extentKey*)rec1;
				if ((extentKey->keyLength < 7) || (extentKey->forkType != 0) || (get_UINT32BE(extentKey->fileID) != 3)
						|| (get_UINT16BE(extentKey->startBlock) != maxExtentAB))
					/* the key is corrupt or does not concern the extent
					B-tree: set the extentEOL flag so that we stop looking for
					further extent records for the extent B-tree */
					extentEOL = TRUE;
				else
				{   /* this key concerns the extent B-tree: update maxExtentAB
                    and maxExtentNode */
					/* extract record data ptr */
					err = BT_get_keyed_record_data(BTref, rec1, rec1_len, &rec1_data, &rec1_data_len);
					if (err)
						goto bail;
					if (rec1_data_len < sizeof(hfs_extent)*3)
						/* the record is corrupt: set the extentEOL flag so
						that we stop looking for further extent records for the
						extent B-tree */
						extentEOL = TRUE;
					else
					{
						extentData = (hfs_extent*)rec1_data;

						for (j=0; j<3; j++)
							maxExtentAB += get_UINT16BE(extentData[j].numABlks);
						maxExtentNode = (UINT64)maxExtentAB * 512 * BTref->fileref.l2_img->blocksperAB
												/ BTref->nodeSize;
					}
				}
				if (extentEOL)
				{
					/* check that the extent B-Tree has been defined entirely */
					if (maxExtentNode < totalNodes)
					{   /* no good */
						err = IMGTOOLERR_CORRUPTIMAGE;
						goto bail;
					}
				}
			}

			if (i<BTref->treeDepth)
			{
				/* extract current record */
				err = BT_node_get_keyed_record(BTref, data_nodes[i].buf, i > 0, data_nodes[i].cur_rec, &rec1, &rec1_len);
				if (err)
					goto bail;

				/* extract previous record */
				err = BT_node_get_keyed_record(BTref, data_nodes[i].buf, i > 0, data_nodes[i].cur_rec-1, &rec2, &rec2_len);
				if (err)
					goto bail;

				/* check that it is sorted correctly */
				compare_result = (*BTref->key_compare_func)(rec1, rec2);
				if (compare_result <= 0)
				{
					err = IMGTOOLERR_CORRUPTIMAGE;
					goto bail;
				}

				i--;
			}
			else
			{
				i--;
				if (i>0)
				{   /* extract first record of root if it is an index node */
					err = BT_node_get_keyed_record(BTref, data_nodes[i].buf, TRUE, data_nodes[i].cur_rec, &rec1, &rec1_len);
					if (err)
						goto bail;
				}
				i--;
			}

			while (i>=0)
			{
				/* extract first record of current level */
				err = BT_node_get_keyed_record(BTref, data_nodes[i].buf, i > 0, data_nodes[i].cur_rec, &rec2, &rec2_len);
				if (err)
					goto bail;

				/* compare key with key of current record of upper level */
				compare_result = (*BTref->key_compare_func)(rec1, rec2);
				if (compare_result != 0)
				{
					err = IMGTOOLERR_CORRUPTIMAGE;
					goto bail;
				}

				/* extract record data ptr */
				err = BT_get_keyed_record_data(BTref, rec1, rec1_len, &rec1_data, &rec1_data_len);
				if (err)
					goto bail;
				if (rec1_data_len < sizeof(UINT32BE))
				{
					err = IMGTOOLERR_CORRUPTIMAGE;
					goto bail;
				}
				cur_node = get_UINT32BE(* (UINT32BE *)rec1_data);

				/* compare node index with data of current record of upper
				level */
				if (cur_node != data_nodes[i].node_num)
				{
					err = IMGTOOLERR_CORRUPTIMAGE;
					goto bail;
				}

				/* iterate to next level */
				rec1 = rec2;
				rec1_len = rec2_len;
				i--;
			}

			/* next leaf record */
			data_nodes[0].cur_rec++;
		}

end_of_list:
		/* check that we are at the end of list for each index level */
		for (i=1; i<BTref->treeDepth; i++)
		{
			if ((data_nodes[i].cur_rec != (data_nodes[i].num_recs-1))
					|| get_UINT32BE(((BTNodeHeader *) data_nodes[i].buf)->fLink))
			{
				err = IMGTOOLERR_CORRUPTIMAGE;
				goto bail;
			}
		}
		/* check that the last leaf node is what it is expected to be */
		if (data_nodes[0].node_num != lastLeafNode)
		{
			err = IMGTOOLERR_CORRUPTIMAGE;
			goto bail;
		}
	}

	/* check map node chain */
	cur_node = 0;   /* node 0 is the header node... */
	bitmap[0] |= 0x80;
	/* check back linking */
	if (get_UINT32BE(((BTNodeHeader *) BTref->node_buf)->bLink))
	{
		err = IMGTOOLERR_CORRUPTIMAGE;
		goto bail;
	}
	/* get pointer to next node */
	cur_node = get_UINT32BE(((BTNodeHeader *) BTref->node_buf)->fLink);
	while (cur_node != 0)
	{
		/* save node address */
		prev_node = cur_node;
		/* check that node has not been used for another purpose */
		/* this check is unecessary because the current consistency checks that
		forward and back linking match and that node height is correct are
		enough to detect such errors */
#if 0
		if (bitmap[cur_node >> 3] & (0x80 >> (cur_node & 7)))
		{
			err = IMGTOOLERR_CORRUPTIMAGE;
			goto bail;
		}
#endif
		/* add node in bitmap */
		bitmap[cur_node >> 3] |= (0x80 >> (cur_node & 7));
		/* read map node */
		if ((! is_extent) || (cur_node < maxExtentNode))
			err = BT_read_node(BTref, cur_node, btnk_mapNode, 0, BTref->node_buf);
		else
			err = IMGTOOLERR_CORRUPTIMAGE;
		if (err)
			goto bail;
		/* check back linking */
		if (get_UINT32BE(((BTNodeHeader *) BTref->node_buf)->bLink) != prev_node)
		{
			err = IMGTOOLERR_CORRUPTIMAGE;
			goto bail;
		}
		/* get pointer to next node */
		cur_node = get_UINT32BE(((BTNodeHeader *) BTref->node_buf)->fLink);
	}

	/* re-read header node */
	err = BT_read_node(BTref, 0, btnk_headerNode, 0, BTref->node_buf);
	if (err)
		goto bail;

	/* get header bitmap record */
	err = BT_node_get_record(BTref, BTref->node_buf, 2, &rec1, &rec1_len);
	if (err)
		goto bail;

	/* check bitmap, iterating map nodes */
	map_count = 0;
	actualFreeNodes = 0;
	while (map_count < map_len)
	{
		/* compute compare len */
		run_len = rec1_len;
		if (run_len > (map_len-map_count))
			run_len = map_len-map_count;
		/* check that all used nodes are marked as such in the B-tree bitmap */
		for (i=0; i<run_len; i++)
			if (bitmap[map_count+i] & ~((UINT8 *)rec1)[i])
			{
				err = IMGTOOLERR_CORRUPTIMAGE;
				goto bail;
			}
		/* count free nodes */
		run_bit_len = rec1_len*8;
		if (run_bit_len > (totalNodes-map_count*8))
			run_bit_len = totalNodes-map_count*8;
		for (i=0; i<run_bit_len; i++)
			if (! (((UINT8 *)rec1)[i>>3] & (0x80 >> (i & 7))))
				actualFreeNodes++;
		map_count += run_len;
		/* read next map node if required */
		if (map_count < map_len)
		{
			/* get pointer to next node */
			cur_node = get_UINT32BE(((BTNodeHeader *) BTref->node_buf)->fLink);
			if (cur_node == 0)
			{
				err = IMGTOOLERR_CORRUPTIMAGE;
				goto bail;
			}
			/* read map node */
			err = BT_read_node(BTref, cur_node, btnk_mapNode, 0, BTref->node_buf);
			if (err)
				goto bail;
			/* get map record */
			err = BT_node_get_record(BTref, BTref->node_buf, 0, &rec1, &rec1_len);
			if (err)
				goto bail;
			header_rec = (BTHeaderRecord *)rec1;
		}
	}

	/* check free node count */
	if (freeNodes != actualFreeNodes)
		return IMGTOOLERR_CORRUPTIMAGE;

bail:
	/* free buffers */
	if (data_nodes)
	{
		for (i=0; i<BTref->treeDepth; i++)
			if (data_nodes[i].buf)
				free(data_nodes[i].buf);
		free(data_nodes);
	}
	if (bitmap)
		free(bitmap);

	return err;
}

/*
    BT_search_leaf_rec

    Search for a given key in a B-Tree.  If exact match found, returns
    corresponding leaf record.  Otherwise, may return the greatest record less
    than the requested key (of course, this will fail if the key is lower than
    all keys in the B-Tree).

    BTref (I/O): open B-tree file handle
    search_key (I): key to search the B-Tree for
    node_ID (O): set to the node ID of the node the record is located in (may
        be NULL)
    record_ID (O): set to the index of the record in the node (may be NULL)
    record_ptr (O): set to point to record in node buffer (may be NULL)
    record_len (O): set to total record len (may be NULL)
    search_exact_match (I): if TRUE, the function will search for a record
        equal to search_key; if FALSE, the function will search for the
        greatest record less than or equal to search_key
    match_found (O): set to TRUE if an exact match for search_key has been
        found (only makes sense if search_exact_match is FALSE) (may be NULL)

    Return imgtool error code
*/
static imgtoolerr_t BT_search_leaf_rec(mac_BTref *BTref, const void *search_key,
										UINT32 *node_ID, int *record_ID,
										void **record_ptr, int *record_len,
										int search_exact_match, int *match_found)
{
	imgtoolerr_t err;
	int i;
	UINT32 cur_node;
	void *cur_rec;
	int cur_rec_len;
	void *last_rec;
	int last_rec_len = 0;
	void *rec_data;
	int rec_data_len;
	int depth;
	UINT16 node_numRecords;
	int compare_result = 0;

	/* start with root node */
	if ((BTref->rootNode == 0) || (BTref->treeDepth == 0))
		/* tree is empty */
		return ((BTref->rootNode == 0) == (BTref->treeDepth == 0))
					? IMGTOOLERR_FILENOTFOUND
					: IMGTOOLERR_CORRUPTIMAGE;

	cur_node = BTref->rootNode;
	depth = BTref->treeDepth;

	while (1)
	{
		/* read current node */
		err = BT_read_node(BTref, cur_node, (depth > 1) ? btnk_indexNode : btnk_leafNode, depth, BTref->node_buf);
		if (err)
			return err;

		/* search for key */
		node_numRecords = get_UINT16BE(((BTNodeHeader *) BTref->node_buf)->numRecords);
		last_rec = cur_rec = NULL;
		for (i=0; i<node_numRecords; i++)
		{
			err = BT_node_get_keyed_record(BTref, BTref->node_buf, depth > 1, i, &cur_rec, &cur_rec_len);
			if (err)
				return err;

			compare_result = (*BTref->key_compare_func)(cur_rec, search_key);
			if (compare_result > 0)
				break;
			last_rec = cur_rec;
			last_rec_len = cur_rec_len;
			if (compare_result == 0)
				break;
		}

		if (! last_rec)
		{   /* all keys are greater than the search key: the search key is
            nowhere in the tree */
			if (search_exact_match)
				return IMGTOOLERR_FILENOTFOUND;

			if (match_found)
				*match_found = FALSE;

			if (node_ID)
				*node_ID = 0;

			if (record_ID)
				*record_ID = -1;

			if (record_ptr)
				*record_ptr = NULL;

			return IMGTOOLERR_SUCCESS;
		}

		if (((BTNodeHeader *) BTref->node_buf)->kind == btnk_leafNode)
			/* leaf node -> end of search */
			break;

		/* extract record data ptr */
		err = BT_get_keyed_record_data(BTref, last_rec, last_rec_len, &rec_data, &rec_data_len);
		if (err)
			return err;
		if (rec_data_len < sizeof(UINT32BE))
			return IMGTOOLERR_CORRUPTIMAGE;

		/* iterate to next level */
		cur_node = get_UINT32BE(* (UINT32BE *)rec_data);
		depth--;
	}

	if (compare_result != 0)
		/* key not found */
		if (search_exact_match)
			return IMGTOOLERR_FILENOTFOUND;

	if (match_found)
		*match_found = (compare_result == 0);

	if (node_ID)
		*node_ID = cur_node;

	if (record_ID)
		*record_ID = i;

	if (record_ptr)
		*record_ptr = last_rec;

	if (record_len)
		*record_len = last_rec_len;

	return IMGTOOLERR_SUCCESS;
}

/*
    BT_leaf_rec_enumerator_open

    Open enumerator for leaf records of a B-Tree

    BTref (I/O): open B-tree file handle
    enumerator (O): B-Tree enumerator to open

    Return imgtool error code
*/
static imgtoolerr_t BT_leaf_rec_enumerator_open(mac_BTref *BTref, BT_leaf_rec_enumerator *enumerator)
{
	enumerator->BTref = BTref;
	enumerator->cur_node = BTref->firstLeafNode;
	enumerator->cur_rec = 0;

	return IMGTOOLERR_SUCCESS;
}

/*
    BT_leaf_rec_enumerator_read

    Read next leaf record of a B-Tree

    enumerator (I/O): open B-Tree enumerator

    Return imgtool error code
*/
static imgtoolerr_t BT_leaf_rec_enumerator_read(BT_leaf_rec_enumerator *enumerator, void **record_ptr, int *rec_len)
{
	UINT16 node_numRecords;
	imgtoolerr_t err;


	*record_ptr = NULL;

	/* check EOList condition */
	if (enumerator->cur_node == 0)
		return IMGTOOLERR_SUCCESS;

	/* read current node */
	err = BT_read_node(enumerator->BTref, enumerator->cur_node, btnk_leafNode, 1, enumerator->BTref->node_buf);
	if (err)
		return err;
	node_numRecords = get_UINT16BE(((BTNodeHeader *) enumerator->BTref->node_buf)->numRecords);

	/* skip nodes until we find a record */
	while ((enumerator->cur_rec >= node_numRecords) && (enumerator->cur_node != 0))
	{
		enumerator->cur_node = get_UINT32BE(((BTNodeHeader *) enumerator->BTref->node_buf)->fLink);
		enumerator->cur_rec = 0;

		/* read node */
		err = BT_read_node(enumerator->BTref, enumerator->cur_node, btnk_leafNode, 1, enumerator->BTref->node_buf);
		if (err)
			return err;
		node_numRecords = get_UINT16BE(((BTNodeHeader *) enumerator->BTref->node_buf)->numRecords);
	}

	/* check EOList condition */
	if (enumerator->cur_node == 0)
		return IMGTOOLERR_SUCCESS;

	/* get current record */
	err = BT_node_get_keyed_record(enumerator->BTref, enumerator->BTref->node_buf, FALSE, enumerator->cur_rec, record_ptr, rec_len);
	if (err)
		return err;

	/* iterate to next record */
	enumerator->cur_rec++;
	if (enumerator->cur_rec >= node_numRecords)
	{   /* iterate to next node if last record (not required, but will improve
        performance on next iteration) */
		enumerator->cur_node = get_UINT32BE(((BTNodeHeader *) enumerator->BTref->node_buf)->fLink);
		enumerator->cur_rec = 0;
	}
	return IMGTOOLERR_SUCCESS;
}

/*
    B-Tree extend EOF algorithm:
    * see if the bitmap will need to be extended
    * extend EOF by min 1 (if bitmap is large engough) or 2 (if bitmap needs
        to be extended) and max ClumpSiz (see extClpSiz and ctClpSiz in MDB)
        ***If we are extending the extent B-Tree, we need to defer the possible
        creation of an additional extent record, or we might enter an endless
        recursion loop***

    Empty node alloc algorithm:

    * see if there is any free node in B-tree bitmap
    * optionally, try to compact the B-tree if file is full
    * if file is still full, extend EOF and try again
    * mark new block as used and return its index


    Empty node delete algorithm:

    * remove node from link list
    * mark node as free in the B-tree bitmap
    * optionally, if more than N% of the B-tree is free, compact the B-tree and
        free some disk space
    * Count nodes on this level; if there is only one left, delete parent index
        node and mark the relaining node as root; if it was the last leaf node,
        update header node with an empty B-tree; in either case, decrement tree
        depth


    Record shifting algorithm:

    For a given node and its first non-empty successor node:

    * compute how much free room there is in the node
    * see if the first record of the first non-empty successor can fit
    * if so, move it (i.e. delete the first record of the later node, and add a
        copy of it to the end of the former)


    Node merging algorithm

    * Consider node and its predecessor.  If there is room, shift all records
        from later to former, then delete empty later node, and delete later
        record from parent index node.


    Node splitting algorithm (non-first)

    * Consider node and its predecessor.  Create new middle node and split
        records in 3 even sets.  Update record for last node and insert record
        for middle node in parent index node.


    Node splitting algorithm (first node)

    * Create new successor node, and split records in 2 1/3 and 2/3 sets.
        Insert record for later node in parent index node.


    Record delete algorithm:

    * remove record from node
    * if record was first in node, test if node is now empty
        * if node is not empty, substitute key of deleted record with key of
            new head record in index tree
        * if node is empty, delete key of deleted record in index tree, then
            delete empty node
    * optionally, look the predecessor node.  Merge the two nodes if possible.


    Record insert algorithm:

    * if there room, just insert new record in node; if new record is in first
        position, update record in parent index node
    * else consider predecessor: see if we can make enough room by shifting
        records.  If so, do shift records, insert new record, update record in
        parent index node
    * else split the nodes and insert record
*/
/*
    Possible additions:

    Node compaction algorithm:

    This algorithm can be executed with a specific start point and max number
    of nodes, or with all nodes on a level.

    * see how many nodes we can save by shifting records left
    * if we will save at least one node, do shift as many records as possible
        (try to leave free space split homogeneously???)
*/

/*static void*/

#if 0
#pragma mark -
#pragma mark RESOURCE IMPLEMENTATION
#endif

/*
    Resource manager

    The resource manager stores arbitrary chunks of data (resource) identified
    by a type/id pair.  The resource type is a 4-char code, which generally
    implies the format of the data (e.g. 'PICT' is for a quickdraw picture,
    'STR ' for a macintosh string, 'CODE' for 68k machine code, etc).  The
    resource id is a signed 16-bit number that uniquely identifies each
    resource of a given type.  Note that, with most resource types, resources
    with id < 128 are system resources that are available to all applications,
    whereas resources with id >= 128 are application resources visible only to
    the application that defines them.

    Each resource can optionally have a resource name, which is a macintosh
    string of 255 chars at most.

    Limits:
    16MBytes of data
    64kbytes of type+reference lists
    64kbytes of resource names

    The Macintosh toolbox can open several resource files simulteanously to
    overcome these restrictions.

    Resources are used virtually everywhere in the Macintosh Toolbox, so it is
    no surprise that file comments and MFS folders are stored in resource files.
*/

/*
    Resource header
*/
struct rsrc_header
{
	UINT32BE data_offs;     /* Offset from beginning of resource fork to resource data */
	UINT32BE map_offs;      /* Offset from beginning of resource fork to resource map */
	UINT32BE data_len;      /* Length of resource data */
	UINT32BE map_len;       /* Length of resource map */
};

/*
    Resource data: each data entry is preceded by its len (UINT32BE)
    Offset to specific data fields are gotten from the resource map
*/

/*
    Resource map:
*/
struct rsrc_map_header
{
	rsrc_header reserved0;  /* Reserved for copy of resource header */
	UINT32BE reserved1;     /* Reserved for handle to next resource map */
	UINT16BE reserved2;     /* Reserved for file reference number */

	UINT16BE attr;          /* Resource fork attributes */
	UINT16BE typelist_offs; /* Offset from beginning of map to resource type list */
	UINT16BE namelist_offs; /* Offset from beginning of map to resource name list */
	UINT16BE type_count;    /* Number of types in the map minus 1 */
							/* This is actually part of the type list, which matters for offsets */
};

/*
    Resource type list entry
*/
struct rsrc_type_entry
{
	UINT32BE type;          /* Resource type */
	UINT16BE ref_count;     /* Number of resources of this type in map minus 1 */
	UINT16BE ref_offs;      /* Offset from beginning of resource type list to reference list for this type */
};

/*
    Resource reference list entry
*/
struct rsrc_ref_entry
{
	UINT16BE id;            /* Resource ID */
	UINT16BE name_offs;     /* Offset from beginning of resource name list to resource name */
							/* (-1 if none) */
	UINT8 attr;             /* Resource attributes */
	UINT24BE data_offs;     /* Offset from beginning of resource data to data for this resource */
	UINT32BE reserved;      /* Reserved for handle to resource */
};

/*
    Resource name list entry: this is just a standard macintosh string
*/

struct mac_resfileref
{
	mac_fileref fileref;    /* open resource fork ref (you may open resources
                                files in data fork, too, if you ever need to,
                                but Classic MacOS never does such a thing
                                (MacOS X often does so, though)) */
	UINT32 data_offs;       /* Offset from beginning of resource file to resource data */
	UINT32 map_offs;        /* Offset from beginning of resource file to resource data */

	UINT16 typelist_offs;   /* Offset from beginning of map to resource type list */
	UINT16 namelist_offs;   /* Offset from beginning of map to resource name list */
	UINT16 type_count;      /* Number of types in the map minus 1 */
							/* This is actually part of the type list, which matters for offsets */
};

#ifdef UNUSED_FUNCTION
/*
    resfile_open

    Open a file as a resource file.  The file must be already open as a
    macintosh file.

    resfileref (I/O): resource file handle to open (resfileref->fileref must
        have been opened previously)

    Return imgtool error code
*/
static imgtoolerr_t resfile_open(mac_resfileref *resfileref)
{
	imgtoolerr_t err;
	rsrc_header header;
	rsrc_map_header map_header;

	/* seek to resource header */
	err = mac_file_seek(&resfileref->fileref, 0);
	if (err)
		return err;

	err = mac_file_read(&resfileref->fileref, sizeof(header), &header);
	if (err)
		return err;

	resfileref->data_offs = get_UINT32BE(header.data_offs);
	resfileref->map_offs = get_UINT32BE(header.map_offs);

	/* seek to resource map header */
	err = mac_file_seek(&resfileref->fileref, resfileref->map_offs);
	if (err)
		return err;

	err = mac_file_read(&resfileref->fileref, sizeof(map_header), &map_header);
	if (err)
		return err;

	resfileref->typelist_offs = get_UINT16BE(map_header.typelist_offs);
	resfileref->namelist_offs = get_UINT16BE(map_header.namelist_offs);
	resfileref->type_count = get_UINT16BE(map_header.type_count);

	return IMGTOOLERR_SUCCESS;
}

/*
    resfile_get_entry

    Get the resource entry in the resource map associated with a given type/id
    pair.

    resfileref (I/O): open resource file handle
    type (I): type of the resource
    id (I): id of the resource
    entry (O): resource entry that has been read

    Return imgtool error code
*/
static imgtoolerr_t resfile_get_entry(mac_resfileref *resfileref, UINT32 type, UINT16 id, rsrc_ref_entry *entry)
{
	imgtoolerr_t err;
	rsrc_type_entry type_entry;
	UINT16 ref_count;
	int i;

	/* seek to resource type list in resource map */
	err = mac_file_seek(&resfileref->fileref, resfileref->map_offs+resfileref->typelist_offs+2);
	if (err)
		return err;

	if (resfileref->type_count == 0xffff)
		/* type list is empty */
		return IMGTOOLERR_FILENOTFOUND;

	for (i=0; i<=resfileref->type_count; i++)
	{
		err = mac_file_read(&resfileref->fileref, sizeof(type_entry), &type_entry);
		if (err)
			return err;
		if (type == get_UINT32BE(type_entry.type))
			break;
	}
	if (i > resfileref->type_count)
		/* type not found in list */
		return IMGTOOLERR_FILENOTFOUND;

	ref_count = get_UINT16BE(type_entry.ref_count);

	/* seek to resource ref list for this type in resource map */
	err = mac_file_seek(&resfileref->fileref, resfileref->map_offs+resfileref->typelist_offs+get_UINT16BE(type_entry.ref_offs));
	if (err)
		return err;

	if (ref_count == 0xffff)
		/* ref list is empty */
		return IMGTOOLERR_FILENOTFOUND;

	for (i=0; i<=ref_count; i++)
	{
		err = mac_file_read(&resfileref->fileref, sizeof(*entry), entry);
		if (err)
			return err;
		if (id == get_UINT16BE(entry->id))
			break;
	}
	if (i > ref_count)
		/* id not found in list */
		return IMGTOOLERR_FILENOTFOUND;

	/* type+id have been found... */
	return IMGTOOLERR_SUCCESS;
}

/*
    resfile_get_resname

    Get the name of a resource.

    resfileref (I/O): open resource file handle
    entry (I): resource entry in the resource map (returned by
        resfile_get_entry)
    string (O): resource name

    Return imgtool error code
*/
static imgtoolerr_t resfile_get_resname(mac_resfileref *resfileref, const rsrc_ref_entry *entry, mac_str255 string)
{
	imgtoolerr_t err;
	UINT16 name_offs;
	UINT8 len;

	name_offs = get_UINT16BE(entry->name_offs);

	if (name_offs == 0xffff)
		/* ref list is empty */
		return IMGTOOLERR_UNEXPECTED;

	/* seek to resource name in name list in resource map */
	err = mac_file_seek(&resfileref->fileref, resfileref->map_offs+name_offs);
	if (err)
		return err;

	/* get string length */
	err = mac_file_read(&resfileref->fileref, 1, &len);
	if (err)
		return err;

	string[0] = len;

	/* get string data */
	err = mac_file_read(&resfileref->fileref, len, string+1);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}

/*
    resfile_get_reslen

    Get the data length for a given resource.

    resfileref (I/O): open resource file handle
    entry (I): resource entry in the resource map (returned by
        resfile_get_entry)
    len (O): resource length

    Return imgtool error code
*/
static imgtoolerr_t resfile_get_reslen(mac_resfileref *resfileref, const rsrc_ref_entry *entry, UINT32 *len)
{
	imgtoolerr_t err;
	UINT32 data_offs;
	UINT32BE llen;

	data_offs = get_UINT24BE(entry->data_offs);

	/* seek to resource data in resource data section */
	err = mac_file_seek(&resfileref->fileref, resfileref->data_offs+data_offs);
	if (err)
		return err;

	/* get data length */
	err = mac_file_read(&resfileref->fileref, sizeof(llen), &llen);
	if (err)
		return err;

	*len = get_UINT32BE(llen);

	return IMGTOOLERR_SUCCESS;
}

/*
    resfile_get_resdata

    Get the data for a given resource.

    resfileref (I/O): open resource file handle
    entry (I): resource entry in the resource map (returned by
        resfile_get_entry)
    offset (I): offset the data should be read from, usually 0
    len (I): length of the data to read, usually the value returned by
        resfile_get_reslen
    dest (O): resource data

    Return imgtool error code
*/
static imgtoolerr_t resfile_get_resdata(mac_resfileref *resfileref, const rsrc_ref_entry *entry, UINT32 offset, UINT32 len, void *dest)
{
	imgtoolerr_t err;
	UINT32 data_offs;
	UINT32BE llen;

	data_offs = get_UINT24BE(entry->data_offs);

	/* seek to resource data in resource data section */
	err = mac_file_seek(&resfileref->fileref, resfileref->data_offs+data_offs);
	if (err)
		return err;

	/* get data length */
	err = mac_file_read(&resfileref->fileref, sizeof(llen), &llen);
	if (err)
		return err;

	/* check that we do not ask to read more data than avalaible */
	if ((offset + len) > get_UINT32BE(llen))
		return IMGTOOLERR_UNEXPECTED;

	if (offset)
	{   /* seek to resource data offset in resource data section */
		err = mac_file_seek(&resfileref->fileref, resfileref->data_offs+data_offs+4+offset);
		if (err)
			return err;
	}

	/* get data */
	err = mac_file_read(&resfileref->fileref, len, dest);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}
#endif

#if 0
#pragma mark -
#pragma mark DESKTOP FILE IMPLEMENTATION
#endif
/*
    All macintosh volumes have information stored in the desktop file or the
    desktop database.

    Such information include file comments, copy of BNDL and FREF resources
    that describes supported file types for each application on the volume,
    copy of icons for each file type registered by each application on the
    volume, etc.  On MFS volumes, the list of folders is stored in the desktop
    file as well.


    There have been two implementations of the desktop metadata:

    * The original desktop file.  The database is stored in the resource fork
        of a (usually invisible) file called "Desktop" (case may change
        according to system versions), located at the root of the volume.  The
        desktop file is used by System 6 and earlier for all volumes (unless
        Appleshare 2 is installed and the volume is shared IIRC), and by System
        7 and later for volumes smaller than 2MBytes (so that floppy disks
        remain fully compatible with earlier versions of system).  The desktop
        file is incompletely documented by Apple technote TB06.

    * The desktop database.  The database is stored in the resource fork is
        stored in the data fork of two (usually invisible) files called
        "Desktop DF" and "Desktop DF".  The desktop database is used for
        volumes shared by Appleshare 2, and for most volumes under System 7 and
        later.  The format of these file is not documented AFAIK.


    The reasons for the introduction of the desktop database were:
    * the macintosh resource manager cannot share resource files, which was
        a problem for Appleshare
    * the macintosh resource manager is pretty limited (+/-16MByte of data and
        2727 resources at most), which was a problem for large hard disks with
        many programs/comments
*/

#ifdef UNUSED_FUNCTION
/*
    get_comment

    Get a comment from the Desktop file

    l2_img (I): macintosh image the data should be read from
    id (I): comment id (from mfs_hashString(), or HFS FXInfo/DXInfo records)
    comment (O): comment that has been read

    Return imgtool error code
*/
static imgtoolerr_t get_comment(struct mac_l2_imgref *l2_img, UINT16 id, mac_str255 comment)
{
	static const UINT8 desktop_fname[] = {'\7','D','e','s','k','t','o','p'};
	#define restype_FCMT (('F' << 24) | ('C' << 16) | ('M' << 8) | 'T')
	mac_resfileref resfileref;
	rsrc_ref_entry resentry;
	UINT32 reslen;
	imgtoolerr_t err;

	/* open rsrc fork of file Desktop in root directory */
	err = mac_file_open(l2_img, 2, desktop_fname, rsrc_fork, &resfileref.fileref);
	if (err)
		return err;

	/* open resource structures */
	err = resfile_open(&resfileref);
	if (err)
		return err;

	/* look for resource FCMT #id */
	err = resfile_get_entry(&resfileref, restype_FCMT, id, &resentry);
	if (err)
		return err;

	/* extract comment len */
	err = resfile_get_reslen(&resfileref, &resentry, &reslen);
	if (err)
		return err;

	/* check comment len */
	if (reslen > 256)
		/* hurk */
		/*return IMGTOOLERR_CORRUPTIMAGE;*/
		/* people willing to extend the MFM comment field (you know, the kind
		of masochists that try to support 20-year-old OSes) might append extra
		fields, so we just truncate the resource */
		reslen = 256;

	/* extract comment data */
	err = resfile_get_resdata(&resfileref, &resentry, 0, reslen, comment);
	if (err)
		return err;

	/* phew, we are done! */
	return IMGTOOLERR_SUCCESS;
}
#endif

#if 0
#pragma mark -
#pragma mark IMGTOOL MODULE IMPLEMENTATION
#endif

#ifdef UNUSED_FUNCTION
static void mac_image_exit(imgtool_image *img);
#endif
static void mac_image_info(imgtool_image *img, char *string, size_t len);
static imgtoolerr_t mac_image_beginenum(imgtool_directory *enumeration, const char *path);
static imgtoolerr_t mac_image_nextenum(imgtool_directory *enumeration, imgtool_dirent *ent);
static imgtoolerr_t mac_image_freespace(imgtool_partition *partition, UINT64 *size);
static imgtoolerr_t mac_image_readfile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *destf);
static imgtoolerr_t mac_image_writefile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *sourcef, option_resolution *writeoptions);

#ifdef UNUSED_FUNCTION
/*
    close a mfs/hfs image
*/
static void mac_image_exit(imgtool_image *img)
{
	struct mac_l2_imgref *image = get_imgref(img);

	mac_image_close(image);
}
#endif

/*
    get basic information on a mfs/hfs image

    Currently returns the volume name
*/
static void mac_image_info(imgtool_image *img, char *string, size_t len)
{
	struct mac_l2_imgref *image = get_imgref(img);

	switch (image->format)
	{
	case L2I_MFS:
		mac_to_c_strncpy(string, len, image->u.mfs.volname);
		break;

	case L2I_HFS:
		mac_to_c_strncpy(string, len, image->u.hfs.volname);
		break;
	}
}

/*
    MFS/HFS catalog iterator, used when imgtool reads the catalog
*/
struct mac_iterator
{
	mac_format format;
	struct mac_l2_imgref *l2_img;
	union
	{
		struct
		{
			mfs_dirref dirref;              /* open directory reference */
		} mfs;
		struct
		{
			hfs_cat_enumerator catref;      /* catalog file enumerator */
		} hfs;
	} u;
};

/*
    Open the disk catalog for enumeration
*/
static imgtoolerr_t mac_image_beginenum(imgtool_directory *enumeration, const char *path)
{
	struct mac_l2_imgref *image = get_imgref(imgtool_directory_image(enumeration));
	mac_iterator *iter = (mac_iterator *) imgtool_directory_extrabytes(enumeration);
	imgtoolerr_t err = IMGTOOLERR_UNEXPECTED;

	iter->format = image->format;
	iter->l2_img = image;

	switch (iter->format)
	{
	case L2I_MFS:
		err = mfs_dir_open(image, path, &iter->u.mfs.dirref);
		break;

	case L2I_HFS:
		err = hfs_cat_open(image, path, &iter->u.hfs.catref);
		break;
	}

	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}

/*
    Enumerate disk catalog next entry (MFS)
*/
static imgtoolerr_t mfs_image_nextenum(mac_iterator *iter, imgtool_dirent *ent)
{
	mfs_dir_entry *cur_dir_entry;
	imgtoolerr_t err;


	assert(iter->format == L2I_MFS);

	ent->corrupt = 0;
	ent->eof = 0;

	err = mfs_dir_read(&iter->u.mfs.dirref, &cur_dir_entry);
	if (err)
	{
		/* error */
		ent->corrupt = 1;
		return err;
	}
	else if (!cur_dir_entry)
	{
		/* EOF */
		ent->eof = 1;
		return IMGTOOLERR_SUCCESS;
	}

	/* copy info */
	mac_to_c_strncpy(ent->filename, ARRAY_LENGTH(ent->filename), cur_dir_entry->name);
	ent->filesize = get_UINT32BE(cur_dir_entry->dataPhysicalSize)
						+ get_UINT32BE(cur_dir_entry->rsrcPhysicalSize);

	return IMGTOOLERR_SUCCESS;
}

#if 0
/*
    Concatenate path elements in the reverse order

    dest (O): destination buffer
    dest_cur_pos (I/O): current position in destination buffer (buffer is
        filled from end to start)
    dest_max_len (I): length of destination buffer (use length minus one if you
        want to preserve a trailing NUL character)
    src (I): source C string
*/
static void concat_fname(char *dest, int *dest_cur_pos, int dest_max_len, const char *src)
{
	static const char ellipsis[] = { '.', '.', '.' };
	int src_len = strlen(src);  /* number of chars from src to insert */

	if (src_len <= *dest_cur_pos)
	{
		*dest_cur_pos -= src_len;
		memcpy(dest + *dest_cur_pos, src, src_len);
	}
	else
	{
		memcpy(dest, src + src_len - *dest_cur_pos, *dest_cur_pos);
		*dest_cur_pos = 0;
		memcpy(dest, ellipsis, (sizeof(ellipsis) <= dest_max_len)
										? sizeof(ellipsis)
										: dest_max_len);
	}
}
#endif

/*
    Enumerate disk catalog next entry (HFS)
*/
static imgtoolerr_t hfs_image_nextenum(mac_iterator *iter, imgtool_dirent *ent)
{
	hfs_catKey *catrec_key;
	hfs_catData *catrec_data;
	UINT16 dataRecType;
	imgtoolerr_t err;
	/* currently, the mac->C conversion transcodes one mac char with at most 3
	C chars */
	int cur_name_head;

	assert(iter->format == L2I_HFS);

	ent->corrupt = 0;
	ent->eof = 0;

	do
	{
		err = hfs_cat_read(&iter->u.hfs.catref, &catrec_key, &catrec_data);
		if (err)
		{
			/* error */
			ent->corrupt = 1;
			return err;
		}
		else if (!catrec_key)
		{
			/* EOF */
			ent->eof = 1;
			return IMGTOOLERR_SUCCESS;
		}
		dataRecType = get_UINT16BE(catrec_data->dataType);
	} while (((dataRecType != hcrt_Folder) && (dataRecType != hcrt_File))
		|| (get_UINT32BE(catrec_key->parID) != iter->u.hfs.catref.parID));

	/* copy info */
	switch (get_UINT16BE(catrec_data->dataType))
	{
		case hcrt_Folder:
			ent->directory = 1;
			ent->filesize = 0;
			break;

		case hcrt_File:
			ent->directory = 0;
			ent->filesize = get_UINT32BE(catrec_data->file.dataPhysicalSize)
				+ get_UINT32BE(catrec_data->file.rsrcPhysicalSize);
			break;
	}

	/* initialize file path buffer */
	cur_name_head = ARRAY_LENGTH(ent->filename);
	if (cur_name_head > 0)
	{
		cur_name_head--;
		ent->filename[cur_name_head] = '\0';
	}

	/* insert folder/file name in buffer */
	mac_to_c_strncpy(ent->filename, ARRAY_LENGTH(ent->filename), catrec_key->cName);
//  concat_fname(ent->filename, &cur_name_head, ARRAY_LENGTH(ent->filename) - 1, buf);

#if 0
	/* extract parent directory ID */
	parID = get_UINT32BE(catrec_key->parID);

	/* looping while (parID != 1) will display the volume name; looping while
	(parID != 2) won't */
	while (parID != /*1*/2)
	{
		/* search catalog for folder thread */
		err = hfs_cat_search(iter->l2_img, parID, mac_empty_str, &catrec_key, &catrec_data);
		if (err)
		{
			/* error */
			concat_fname(ent->filename, &cur_name_head, ARRAY_LENGTH(ent->filename) - 1, ":");
			concat_fname(ent->filename, &cur_name_head, ARRAY_LENGTH(ent->filename) - 1, "???");

			memmove(ent->filename, ent->filename+cur_name_head, ARRAY_LENGTH(ent->filename) - cur_name_head);
			ent->corrupt = 1;
			return err;
		}

		dataRecType = get_UINT16BE(catrec_data->dataType);

		if (dataRecType != hcrt_FolderThread)
		{
			/* error */
			concat_fname(ent->filename, &cur_name_head, ARRAY_LENGTH(ent->filename)-1, ":");
			concat_fname(ent->filename, &cur_name_head, ARRAY_LENGTH(ent->filename)-1, "???");

			memmove(ent->filename, ent->filename+cur_name_head, ARRAY_LENGTH(ent->filename)-cur_name_head);
			ent->corrupt = 1;
			return IMGTOOLERR_CORRUPTIMAGE;
		}

		/* got folder thread record: insert the folder name at the start of
		file path, then iterate */
		mac_to_c_strncpy(buf, sizeof(buf), catrec_data->thread.nodeName);
		concat_fname(ent->filename, &cur_name_head, ARRAY_LENGTH(ent->filename) - 1, ":");
		concat_fname(ent->filename, &cur_name_head, ARRAY_LENGTH(ent->filename) - 1, buf);

		/* extract parent directory ID */
		parID = get_UINT32BE(catrec_data->thread.parID);
	}
	memmove(ent->filename, ent->filename+cur_name_head, ARRAY_LENGTH(ent->filename) -cur_name_head);
#endif
	return IMGTOOLERR_SUCCESS;
}

/*
    Enumerate disk catalog next entry
*/
static imgtoolerr_t mac_image_nextenum(imgtool_directory *enumeration, imgtool_dirent *ent)
{
	imgtoolerr_t err;
	mac_iterator *iter = (mac_iterator *) imgtool_directory_extrabytes(enumeration);

	switch (iter->format)
	{
		case L2I_MFS:
			err = mfs_image_nextenum(iter, ent);
			break;

		case L2I_HFS:
			err = hfs_image_nextenum(iter, ent);
			break;

		default:
			assert(1);
			err = IMGTOOLERR_UNEXPECTED;
			break;
	}
	return err;
}

/*
    Compute free space on disk image in bytes
*/
static imgtoolerr_t mac_image_freespace(imgtool_partition *partition, UINT64 *size)
{
	imgtool_image *image = imgtool_partition_image(partition);
	*size = ((UINT64) get_imgref(image)->freeABs) * 512;
	return IMGTOOLERR_SUCCESS;
}

#ifdef UNUSED_FUNCTION
static imgtoolerr_t mac_get_comment(struct mac_l2_imgref *image, mac_str255 filename, const mac_dirent *cat_info, mac_str255 comment)
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	UINT16 commentID;

	comment[0] = '\0';

	/* get comment from Desktop file */
	switch (image->format)
	{
		case L2I_MFS:
			commentID = mfs_hashString(filename);
			err = get_comment(image, commentID, comment);
			break;

		case L2I_HFS:
			/* This is the way to get Finder comments in system <= 7.  Attached
			comments use another method, and Finder 8 uses yet another one. */
			commentID = get_UINT16BE(cat_info->flXFinderInfo.comment);
			if (commentID)
				err = get_comment(image, commentID, comment);
			break;
	}
	return err;
}
#endif


/*
    Extract a file from a disk image.
*/
static imgtoolerr_t mac_image_readfile(imgtool_partition *partition, const char *fpath, const char *fork, imgtool_stream *destf)
{
	imgtoolerr_t err;
	imgtool_image *img = imgtool_partition_image(partition);
	struct mac_l2_imgref *image = get_imgref(img);
	UINT32 parID;
	mac_str255 filename;
	mac_dirent cat_info;
	mac_fileref fileref;
	UINT8 buf[512];
	UINT32 i, run_len, data_len;
	mac_fork_t fork_num;

	err = mac_identify_fork(fork, &fork_num);
	if (err)
		return err;

	/* resolve path and fetch file info from directory/catalog */
	err = mac_lookup_path(image, fpath, &parID, filename, &cat_info, FALSE);
	if (err)
		return err;
	if (cat_info.dataRecType != hcrt_File)
		return IMGTOOLERR_FILENOTFOUND;

	/* open file */
	err = mac_file_open(image, parID, filename, fork_num ? rsrc_fork : data_fork, &fileref);
	if (err)
		return err;

	data_len = fork_num ? cat_info.rsrcLogicalSize : cat_info.dataLogicalSize;

	/* extract DF */
	i = 0;
	while(i < data_len)
	{
		run_len = MIN(data_len - i, sizeof(buf));

		err = mac_file_read(&fileref, run_len, buf);
		if (err)
			return err;
		if (stream_write(destf, buf, run_len) != run_len)
			return IMGTOOLERR_WRITEERROR;
		i += run_len;
	}

	return IMGTOOLERR_SUCCESS;
}

/*
    Add a file to a disk image.
*/
static imgtoolerr_t mac_image_writefile(imgtool_partition *partition, const char *fpath, const char *fork, imgtool_stream *sourcef, option_resolution *writeoptions)
{
	imgtool_image *img = imgtool_partition_image(partition);
	struct mac_l2_imgref *image = get_imgref(img);
	UINT32 parID;
	mac_str255 filename;
	mac_dirent cat_info;
	mac_fileref fileref;
	UINT32 fork_len;
	/*UINT16 commentID;*/
	/*mac_str255 comment;*/
	UINT8 buf[512];
	UINT32 i, run_len;
	imgtoolerr_t err;
	mac_fork_t fork_num;

	(void) writeoptions;

	if (image->format == L2I_HFS)
		return IMGTOOLERR_UNIMPLEMENTED;

	err = mac_identify_fork(fork, &fork_num);
	if (err)
		return err;

#if 0
	if (header.version_old != 0)
		return IMGTOOLERR_UNIMPLEMENTED;
#endif
	/*mac_strcpy(filename, header.filename);*/
	memset(&cat_info, 0, sizeof(cat_info));
	set_UINT32BE(&cat_info.flFinderInfo.type, 0x3F3F3F3F);
	set_UINT32BE(&cat_info.flFinderInfo.creator, 0x3F3F3F3F);
	fork_len = stream_size(sourcef);
	/*comment[0] = get_UINT16BE(header.comment_len);*/  /* comment length */
	/* Next two fields are set to 0 with MFS volumes.  IIRC, 0 normally
	means system script: I don't think MFS stores the file name script code
	anywhere on disk, so it should be a reasonable approximation. */

	/* create file */
	/* clear inited flag and file location in window */
	set_UINT16BE(&cat_info.flFinderInfo.flags, get_UINT16BE(cat_info.flFinderInfo.flags) & ~fif_hasBeenInited);
	set_UINT16BE(&cat_info.flFinderInfo.location.v, 0);
	set_UINT16BE(&cat_info.flFinderInfo.location.h, 0);

	/* resolve path and create file */
	err = mac_lookup_path(image, fpath, &parID, filename, &cat_info, TRUE);
	if (err)
		return err;

	/* open file fork */
	err = mac_file_open(image, parID, filename, (fork_num ? rsrc_fork : data_fork), &fileref);
	if (err)
		return err;

	err = mac_file_seteof(&fileref, fork_len);
	if (err)
		return err;

	/* extract fork */
	for (i=0; i<fork_len;)
	{
		run_len = fork_len - i;
		if (run_len > 512)
			run_len = 512;
		if (stream_read(sourcef, buf, run_len) != run_len)
			return IMGTOOLERR_READERROR;
		err = mac_file_write(&fileref, run_len, buf);
		if (err)
			return err;
		i += run_len;
	}
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t mac_image_listforks(imgtool_partition *partition, const char *path, imgtool_forkent *ents, size_t len)
{
	imgtoolerr_t err;
	UINT32 parID;
	mac_str255 filename;
	mac_dirent cat_info;
	int fork_num = 0;
	imgtool_image *img = imgtool_partition_image(partition);
	struct mac_l2_imgref *image = get_imgref(img);

	/* resolve path and fetch file info from directory/catalog */
	err = mac_lookup_path(image, path, &parID, filename, &cat_info, FALSE);
	if (err)
		return err;
	if (cat_info.dataRecType != hcrt_File)
		return IMGTOOLERR_FILENOTFOUND;

	/* specify data fork */
	ents[fork_num].type = FORK_DATA;
	ents[fork_num].forkname[0] = '\0';
	ents[fork_num].size = cat_info.dataLogicalSize;
	fork_num++;

	if (cat_info.rsrcLogicalSize > 0)
	{
		/* specify the resource fork */
		ents[fork_num].type = FORK_RESOURCE;
		strcpy(ents[fork_num].forkname, "RESOURCE_FORK");
		ents[fork_num].size = cat_info.rsrcLogicalSize;
		fork_num++;
	}

	ents[fork_num].type = FORK_END;
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t mac_image_getattrs(imgtool_partition *partition, const char *path, const UINT32 *attrs, imgtool_attribute *values)
{
	imgtoolerr_t err;
	imgtool_image *img = imgtool_partition_image(partition);
	UINT32 parID;
	mac_str255 filename;
	mac_dirent cat_info;
	struct mac_l2_imgref *image = get_imgref(img);
	int i;

	/* resolve path and fetch file info from directory/catalog */
	err = mac_lookup_path(image, path, &parID, filename, &cat_info, FALSE);
	if (err)
		return err;
	if (cat_info.dataRecType != hcrt_File)
		return IMGTOOLERR_FILENOTFOUND;

	for (i = 0; attrs[i]; i++)
	{
		switch(attrs[i])
		{
			case IMGTOOLATTR_INT_MAC_TYPE:
				values[i].i = get_UINT32BE(cat_info.flFinderInfo.type);
				break;
			case IMGTOOLATTR_INT_MAC_CREATOR:
				values[i].i = get_UINT32BE(cat_info.flFinderInfo.creator);
				break;
			case IMGTOOLATTR_INT_MAC_FINDERFLAGS:
				values[i].i = get_UINT16BE(cat_info.flFinderInfo.flags);
				break;
			case IMGTOOLATTR_INT_MAC_COORDX:
				values[i].i = get_UINT16BE(cat_info.flFinderInfo.location.h);
				break;
			case IMGTOOLATTR_INT_MAC_COORDY:
				values[i].i = get_UINT16BE(cat_info.flFinderInfo.location.v);
				break;
			case IMGTOOLATTR_INT_MAC_FINDERFOLDER:
				values[i].i = get_UINT16BE(cat_info.flFinderInfo.fldr);
				break;
			case IMGTOOLATTR_INT_MAC_ICONID:
				values[i].i = get_UINT16BE(cat_info.flXFinderInfo.iconID);
				break;
			case IMGTOOLATTR_INT_MAC_SCRIPTCODE:
				values[i].i = cat_info.flXFinderInfo.script;
				break;
			case IMGTOOLATTR_INT_MAC_EXTENDEDFLAGS:
				values[i].i = cat_info.flXFinderInfo.XFlags;
				break;
			case IMGTOOLATTR_INT_MAC_COMMENTID:
				values[i].i = get_UINT16BE(cat_info.flXFinderInfo.comment);
				break;
			case IMGTOOLATTR_INT_MAC_PUTAWAYDIRECTORY:
				values[i].i = get_UINT32BE(cat_info.flXFinderInfo.putAway);
				break;

			case IMGTOOLATTR_TIME_CREATED:
				values[i].t = mac_crack_time(cat_info.createDate);
				break;
			case IMGTOOLATTR_TIME_LASTMODIFIED:
				values[i].t = mac_crack_time(cat_info.modifyDate);
				break;
		}
	}
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t mac_image_setattrs(imgtool_partition *partition, const char *path, const UINT32 *attrs, const imgtool_attribute *values)
{
	imgtoolerr_t err;
	UINT32 parID;
	mac_str255 filename;
	mac_dirent cat_info;
	imgtool_image *img = imgtool_partition_image(partition);
	struct mac_l2_imgref *image = get_imgref(img);
	int i;

	/* resolve path and fetch file info from directory/catalog */
	err = mac_lookup_path(image, path, &parID, filename, &cat_info, FALSE);
	if (err)
		return err;
	if (cat_info.dataRecType != hcrt_File)
		return IMGTOOLERR_FILENOTFOUND;

	for (i = 0; attrs[i]; i++)
	{
		switch(attrs[i])
		{
			case IMGTOOLATTR_INT_MAC_TYPE:
				set_UINT32BE(&cat_info.flFinderInfo.type, values[i].i);
				break;
			case IMGTOOLATTR_INT_MAC_CREATOR:
				set_UINT32BE(&cat_info.flFinderInfo.creator, values[i].i);
				break;
			case IMGTOOLATTR_INT_MAC_FINDERFLAGS:
				set_UINT16BE(&cat_info.flFinderInfo.flags, values[i].i);
				break;
			case IMGTOOLATTR_INT_MAC_COORDX:
				set_UINT16BE(&cat_info.flFinderInfo.location.h, values[i].i);
				break;
			case IMGTOOLATTR_INT_MAC_COORDY:
				set_UINT16BE(&cat_info.flFinderInfo.location.v, values[i].i);
				break;
			case IMGTOOLATTR_INT_MAC_FINDERFOLDER:
				set_UINT16BE(&cat_info.flFinderInfo.fldr, values[i].i);
				break;
			case IMGTOOLATTR_INT_MAC_ICONID:
				set_UINT16BE(&cat_info.flXFinderInfo.iconID, values[i].i);
				break;
			case IMGTOOLATTR_INT_MAC_SCRIPTCODE:
				cat_info.flXFinderInfo.script = values[i].i;
				break;
			case IMGTOOLATTR_INT_MAC_EXTENDEDFLAGS:
				cat_info.flXFinderInfo.XFlags = values[i].i;
				break;
			case IMGTOOLATTR_INT_MAC_COMMENTID:
				set_UINT16BE(&cat_info.flXFinderInfo.comment, values[i].i);
				break;
			case IMGTOOLATTR_INT_MAC_PUTAWAYDIRECTORY:
				set_UINT32BE(&cat_info.flXFinderInfo.putAway, values[i].i);
				break;

			case IMGTOOLATTR_TIME_CREATED:
				cat_info.createDate = mac_setup_time(values[i].t);
				break;
			case IMGTOOLATTR_TIME_LASTMODIFIED:
				cat_info.modifyDate = mac_setup_time(values[i].t);
				break;
		}
	}

	/* resolve path and fetch file info from directory/catalog */
	err = mac_lookup_path(image, path, &parID, filename, &cat_info, TRUE);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}



/*************************************
 *
 *  Our very own resource manager
 *
 *************************************/

static const void *mac_walk_resources(const void *resource_fork, size_t resource_fork_length,
	UINT32 resource_type,
	int (*discriminator)(const void *resource, UINT16 id, UINT32 length, void *param_),
	void *param, UINT16 *resource_id, UINT32 *resource_length)
{
	UINT32 resource_data_offset, resource_data_length;
	UINT32 resource_map_offset, resource_map_length;
	UINT32 resource_typelist_count, resource_typelist_offset;
	UINT32 resource_entry_offset, resource_entry_count;
	UINT32 i, this_resource_type;
	UINT32 this_resource_data, this_resource_length;
	UINT16 this_resource_id;
	const void *this_resource_ptr;

	if (resource_fork_length < 16)
		return NULL;

	/* read resource header; its ok if anything past this point fails */
	resource_data_offset = pick_integer_be(resource_fork,  0, 4);
	resource_map_offset  = pick_integer_be(resource_fork,  4, 4);
	resource_data_length = pick_integer_be(resource_fork,  8, 4);
	resource_map_length  = pick_integer_be(resource_fork, 12, 4);
	if ((resource_data_offset + resource_data_length) > resource_fork_length)
		return NULL;
	if ((resource_map_offset + resource_map_length) > resource_fork_length)
		return NULL;
	if (resource_map_length < 30)
		return NULL;

	/* read resource map and locate the resource type list */
	resource_typelist_offset = pick_integer_be(resource_fork,
		resource_map_offset + 24, 2) + resource_map_offset;
	if ((resource_typelist_offset + 2) > resource_fork_length)
		return NULL;
	resource_typelist_count = pick_integer_be(resource_fork,
		resource_typelist_offset, 2) + 1;
	if ((resource_typelist_offset + resource_typelist_count * 16 + 2) > resource_fork_length)
		return NULL;

	/* scan the resource type list and locate the entries for this type */
	resource_entry_count = 0;
	resource_entry_offset = 0;
	for (i = 0; i < resource_typelist_count; i++)
	{
		this_resource_type = pick_integer_be(resource_fork, resource_typelist_offset
			+ (i * 8) + 2 + 0, 4);
		if (this_resource_type == resource_type)
		{
			resource_entry_count = pick_integer_be(resource_fork, resource_typelist_offset
				+ (i * 8) + 2 + 4, 2) + 1;
			resource_entry_offset = pick_integer_be(resource_fork, resource_typelist_offset
				+ (i * 8) + 2 + 6, 2) + resource_typelist_offset;
			break;
		}
	}

	/* scan the resource entries, and find the correct resource */
	for (i = 0; i < resource_entry_count; i++)
	{
		this_resource_id = pick_integer_be(resource_fork, resource_entry_offset
			+ (i * 12) + 0, 2);
		this_resource_data = pick_integer_be(resource_fork, resource_entry_offset
			+ (i * 12) + 5, 3) + resource_data_offset;
		if ((this_resource_data + 4) > resource_fork_length)
			return NULL;

		/* gauge the length */
		this_resource_length = pick_integer_be(resource_fork, this_resource_data, 4);
		this_resource_data += 4;
		if ((this_resource_data + this_resource_length) > resource_fork_length)
			return NULL;

		this_resource_ptr = ((const UINT8 *) resource_fork) + this_resource_data;
		if (discriminator(this_resource_ptr, this_resource_id,
			this_resource_length, param))
		{
			if (resource_length)
				*resource_length = this_resource_length;
			if (resource_id)
				*resource_id = this_resource_id;
			return this_resource_ptr;
		}
	}

	return NULL;
}



static int get_resource_discriminator(const void *resource, UINT16 id, UINT32 length, void *param)
{
	const UINT16 *id_ptr = (const UINT16 *) param;
	return id == *id_ptr;
}



static const void *mac_get_resource(const void *resource_fork, size_t resource_fork_length,
	UINT32 resource_type, UINT16 resource_id, UINT32 *resource_length)
{
	return mac_walk_resources(resource_fork, resource_fork_length,
		resource_type, get_resource_discriminator, &resource_id, NULL, resource_length);
}



/*************************************
 *
 *  Custom icons
 *
 *************************************/

static int bundle_discriminator(const void *resource, UINT16 id, UINT32 length, void *param)
{
	UINT32 this_creator_code = pick_integer_be(resource, 0, 4);
	UINT32 desired_creator_code = *((UINT32 *) param);
	return this_creator_code == desired_creator_code;
}



static int get_pixel(const UINT8 *src, int width, int height, int bpp,
	int x, int y)
{
	UINT8 byte, mask;
	int bit_position;

	byte = src[(y * width + x) * bpp / 8];
	bit_position = 8 - ((x % (8 / bpp)) + 1) * bpp;
	mask = (1 << bpp) - 1;
	return (byte >> bit_position) & mask;
}



static int load_icon(UINT32 *dest, const void *resource_fork, UINT64 resource_fork_length,
	UINT32 resource_type, UINT16 resource_id, int width, int height, int bpp,
	const UINT32 *palette, int has_mask)
{
	int success = FALSE;
	int y, x, color, is_masked;
	UINT32 pixel;
	const UINT8 *src;
	UINT32 resource_length;
	UINT32 frame_length;
	UINT32 total_length;

	frame_length = width * height * bpp / 8;
	total_length = frame_length * (has_mask ? 2 : 1);

	/* attempt to fetch resource */
	src = (const UINT8*)mac_get_resource(resource_fork, resource_fork_length, resource_type,
		resource_id, &resource_length);
	if (src && (resource_length == total_length))
	{
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x ++)
			{
				/* first check mask bit */
				if (has_mask)
					is_masked = get_pixel(src + frame_length, width, height, bpp, x, y);
				else
					is_masked = dest[y * width + x] >= 0x80000000;

				if (is_masked)
				{
					/* mask is ok; check the actual icon */
					color = get_pixel(src, width, height, bpp, x, y);
					pixel = palette[color] | 0xFF000000;
				}
				else
				{
					/* masked out; nothing */
					pixel = 0;
				}

				dest[y * width + x] = pixel;
			}
		}
		success = TRUE;
	}
	return success;
}



static imgtoolerr_t mac_image_geticoninfo(imgtool_partition *partition, const char *path, imgtool_iconinfo *iconinfo)
{
	static const UINT32 mac_palette_1bpp[2] = { 0xFFFFFF, 0x000000 };

	static const UINT32 mac_palette_4bpp[16] =
	{
		0xFFFFFF, 0xFCF305, 0xFF6402, 0xDD0806, 0xF20884, 0x4600A5,
		0x0000D4, 0x02ABEA, 0x1FB714, 0x006411, 0x562C05, 0x90713A,
		0xC0C0C0, 0x808080, 0x404040, 0x000000
	};

	static const UINT32 mac_palette_8bpp[256] =
	{
		0xFFFFFF, 0xFFFFCC, 0xFFFF99, 0xFFFF66, 0xFFFF33, 0xFFFF00,
		0xFFCCFF, 0xFFCCCC, 0xFFCC99, 0xFFCC66, 0xFFCC33, 0xFFCC00,
		0xFF99FF, 0xFF99CC, 0xFF9999, 0xFF9966, 0xFF9933, 0xFF9900,
		0xFF66FF, 0xFF66CC, 0xFF6699, 0xFF6666, 0xFF6633, 0xFF6600,
		0xFF33FF, 0xFF33CC, 0xFF3399, 0xFF3366, 0xFF3333, 0xFF3300,
		0xFF00FF, 0xFF00CC, 0xFF0099, 0xFF0066, 0xFF0033, 0xFF0000,
		0xCCFFFF, 0xCCFFCC, 0xCCFF99, 0xCCFF66, 0xCCFF33, 0xCCFF00,
		0xCCCCFF, 0xCCCCCC, 0xCCCC99, 0xCCCC66, 0xCCCC33, 0xCCCC00,
		0xCC99FF, 0xCC99CC, 0xCC9999, 0xCC9966, 0xCC9933, 0xCC9900,
		0xCC66FF, 0xCC66CC, 0xCC6699, 0xCC6666, 0xCC6633, 0xCC6600,
		0xCC33FF, 0xCC33CC, 0xCC3399, 0xCC3366, 0xCC3333, 0xCC3300,
		0xCC00FF, 0xCC00CC, 0xCC0099, 0xCC0066, 0xCC0033, 0xCC0000,
		0x99FFFF, 0x99FFCC, 0x99FF99, 0x99FF66, 0x99FF33, 0x99FF00,
		0x99CCFF, 0x99CCCC, 0x99CC99, 0x99CC66, 0x99CC33, 0x99CC00,
		0x9999FF, 0x9999CC, 0x999999, 0x999966, 0x999933, 0x999900,
		0x9966FF, 0x9966CC, 0x996699, 0x996666, 0x996633, 0x996600,
		0x9933FF, 0x9933CC, 0x993399, 0x993366, 0x993333, 0x993300,
		0x9900FF, 0x9900CC, 0x990099, 0x990066, 0x990033, 0x990000,
		0x66FFFF, 0x66FFCC, 0x66FF99, 0x66FF66, 0x66FF33, 0x66FF00,
		0x66CCFF, 0x66CCCC, 0x66CC99, 0x66CC66, 0x66CC33, 0x66CC00,
		0x6699FF, 0x6699CC, 0x669999, 0x669966, 0x669933, 0x669900,
		0x6666FF, 0x6666CC, 0x666699, 0x666666, 0x666633, 0x666600,
		0x6633FF, 0x6633CC, 0x663399, 0x663366, 0x663333, 0x663300,
		0x6600FF, 0x6600CC, 0x660099, 0x660066, 0x660033, 0x660000,
		0x33FFFF, 0x33FFCC, 0x33FF99, 0x33FF66, 0x33FF33, 0x33FF00,
		0x33CCFF, 0x33CCCC, 0x33CC99, 0x33CC66, 0x33CC33, 0x33CC00,
		0x3399FF, 0x3399CC, 0x339999, 0x339966, 0x339933, 0x339900,
		0x3366FF, 0x3366CC, 0x336699, 0x336666, 0x336633, 0x336600,
		0x3333FF, 0x3333CC, 0x333399, 0x333366, 0x333333, 0x333300,
		0x3300FF, 0x3300CC, 0x330099, 0x330066, 0x330033, 0x330000,
		0x00FFFF, 0x00FFCC, 0x00FF99, 0x00FF66, 0x00FF33, 0x00FF00,
		0x00CCFF, 0x00CCCC, 0x00CC99, 0x00CC66, 0x00CC33, 0x00CC00,
		0x0099FF, 0x0099CC, 0x009999, 0x009966, 0x009933, 0x009900,
		0x0066FF, 0x0066CC, 0x006699, 0x006666, 0x006633, 0x006600,
		0x0033FF, 0x0033CC, 0x003399, 0x003366, 0x003333, 0x003300,
		0x0000FF, 0x0000CC, 0x000099, 0x000066, 0x000033, 0xEE0000,
		0xDD0000, 0xBB0000, 0xAA0000, 0x880000, 0x770000, 0x550000,
		0x440000, 0x220000, 0x110000, 0x00EE00, 0x00DD00, 0x00BB00,
		0x00AA00, 0x008800, 0x007700, 0x005500, 0x004400, 0x002200,
		0x001100, 0x0000EE, 0x0000DD, 0x0000BB, 0x0000AA, 0x000088,
		0x000077, 0x000055, 0x000044, 0x000022, 0x000011, 0xEEEEEE,
		0xDDDDDD, 0xBBBBBB, 0xAAAAAA, 0x888888, 0x777777, 0x555555,
		0x444444, 0x222222, 0x111111, 0x000000
	};

	static const UINT32 attrs[4] =
	{
		IMGTOOLATTR_INT_MAC_TYPE,
		IMGTOOLATTR_INT_MAC_CREATOR,
		IMGTOOLATTR_INT_MAC_FINDERFLAGS
	};

	imgtoolerr_t err;
	imgtool_attribute attr_values[3];
	UINT32 type_code, creator_code, finder_flags;
	imgtool_stream *stream = NULL;
	const void *resource_fork;
	UINT64 resource_fork_length;
	const void *bundle;
	UINT32 bundle_length, pos, fref_pos, icn_pos, i;
	UINT16 local_id = 0, resource_id;
	UINT32 fref_bundleentry_length, icn_bundleentry_length;
	const void *fref;
	UINT32 resource_length;

	assert((ARRAY_LENGTH(attrs) - 1)
		== ARRAY_LENGTH(attr_values));

	/* first retrieve type and creator code */
	err = mac_image_getattrs(partition, path, attrs, attr_values);
	if (err)
		goto done;
	type_code = (UINT32) attr_values[0].i;
	creator_code = (UINT32) attr_values[1].i;
	finder_flags = (UINT32) attr_values[2].i;

	/* check the bundle bit; if clear (and the type is not 'APPL'), use the
	 * desktop file */
	if (!(finder_flags & 0x2000) && (type_code != /* APPL */ 0x4150504C))
		path = "Desktop\0";

	stream = stream_open_mem(NULL, 0);
	if (!stream)
	{
		err = IMGTOOLERR_SUCCESS;
		goto done;
	}

	/* read in the resource fork */
	err = mac_image_readfile(partition, path, "RESOURCE_FORK", stream);
	if (err)
		goto done;
	resource_fork = stream_getptr(stream);
	resource_fork_length = stream_size(stream);

	/* attempt to look up the bundle */
	bundle = mac_walk_resources(resource_fork, resource_fork_length, /* BNDL */ 0x424E444C,
		bundle_discriminator, &creator_code, NULL, &bundle_length);
	if (!bundle)
		goto done;

	/* find the FREF and the icon family */
	pos = 8;
	fref_pos = icn_pos = 0;
	fref_bundleentry_length = icn_bundleentry_length = 0;
	while((pos + 10) <= bundle_length)
	{
		UINT32 this_bundleentry_type = pick_integer_be(bundle, pos + 0, 4);
		UINT32 this_bundleentry_length = pick_integer_be(bundle, pos + 4, 2) + 1;

		if (this_bundleentry_type == /* FREF */ 0x46524546)
		{
			fref_pos = pos;
			fref_bundleentry_length = this_bundleentry_length;
		}
		if (this_bundleentry_type == /* ICN# */ 0x49434E23)
		{
			icn_pos = pos;
			icn_bundleentry_length = this_bundleentry_length;
		}
		pos += 6 + this_bundleentry_length * 4;
	}
	if (!fref_pos || !icn_pos)
		goto done;

	/* look up the FREF */
	for (i = 0; i < fref_bundleentry_length; i++)
	{
		local_id = pick_integer_be(bundle, fref_pos + (i * 4) + 6, 2);
		resource_id = pick_integer_be(bundle, fref_pos + (i * 4) + 8, 2);

		fref = mac_get_resource(resource_fork, resource_fork_length,
			/* FREF */ 0x46524546, resource_id, &resource_length);
		if (fref && (resource_length >= 7))
		{
			if (pick_integer_be(fref, 0, 4) == type_code)
				break;
		}
	}
	if (i >= fref_bundleentry_length)
		goto done;

	/* now look up the icon family */
	resource_id = 0;
	for (i = 0; i < icn_bundleentry_length; i++)
	{
		if (pick_integer_be(bundle, icn_pos + (i * 4) + 6, 2) == local_id)
		{
			resource_id = pick_integer_be(bundle, icn_pos + (i * 4) + 8, 2);
			break;
		}
	}
	if (i >= icn_bundleentry_length)
		goto done;

	/* fetch 32x32 icons (ICN#, icl4, icl8) */
	if (load_icon((UINT32 *) iconinfo->icon32x32, resource_fork, resource_fork_length,
		/* ICN# */ 0x49434E23, resource_id, 32, 32, 1, mac_palette_1bpp, TRUE))
	{
		iconinfo->icon32x32_specified = 1;

		load_icon((UINT32 *) iconinfo->icon32x32, resource_fork, resource_fork_length,
			/* icl4 */ 0x69636C34, resource_id, 32, 32, 4, mac_palette_4bpp, FALSE);
		load_icon((UINT32 *) iconinfo->icon32x32, resource_fork, resource_fork_length,
			/* icl8 */ 0x69636C38, resource_id, 32, 32, 8, mac_palette_8bpp, FALSE);
	}

	/* fetch 16x16 icons (ics#, ics4, ics8) */
	if (load_icon((UINT32 *) iconinfo->icon16x16, resource_fork, resource_fork_length,
		/* ics# */ 0x69637323, resource_id, 16, 16, 1, mac_palette_1bpp, TRUE))
	{
		iconinfo->icon16x16_specified = 1;

		load_icon((UINT32 *) iconinfo->icon32x32, resource_fork, resource_fork_length,
			/* ics4 */ 0x69637334, resource_id, 32, 32, 4, mac_palette_4bpp, FALSE);
		load_icon((UINT32 *) iconinfo->icon32x32, resource_fork, resource_fork_length,
			/* ics8 */ 0x69637338, resource_id, 32, 32, 8, mac_palette_8bpp, FALSE);
	}

done:
	if (stream)
		stream_close(stream);
	return err;
}



/*************************************
 *
 *  File transfer suggestions
 *
 *************************************/

static imgtoolerr_t mac_image_suggesttransfer(imgtool_partition *partition, const char *path, imgtool_transfer_suggestion *suggestions, size_t suggestions_length)
{
	imgtoolerr_t err;
	UINT32 parID;
	mac_str255 filename;
	mac_dirent cat_info;
	imgtool_image *img = imgtool_partition_image(partition);
	struct mac_l2_imgref *image = get_imgref(img);
	mac_filecategory_t file_category = MAC_FILECATEGORY_DATA;

	if (path)
	{
		/* resolve path and fetch file info from directory/catalog */
		err = mac_lookup_path(image, path, &parID, filename, &cat_info, FALSE);
		if (err)
			return err;
		if (cat_info.dataRecType != hcrt_File)
			return IMGTOOLERR_FILENOTFOUND;

		file_category = (cat_info.rsrcLogicalSize > 0) ? MAC_FILECATEGORY_FORKED : MAC_FILECATEGORY_DATA;
	}

	mac_suggest_transfer(file_category, suggestions, suggestions_length);
	return IMGTOOLERR_SUCCESS;
}



/*************************************
 *
 *  Module population
 *
 *************************************/

static void generic_mac_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_OPEN_IS_STRICT:                info->i = 1; break;
		case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES:             info->i = sizeof(struct mac_l2_imgref); break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:             info->i = sizeof(struct mac_iterator); break;
		case IMGTOOLINFO_INT_PATH_SEPARATOR:                info->i = ':'; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_FILE:                          strcpy(info->s = imgtool_temp_str(), __FILE__); break;
		case IMGTOOLINFO_STR_EOLN:                          strcpy(info->s = imgtool_temp_str(), "\r"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_MAKE_CLASS:                    info->make_class = imgtool_floppy_make_class; break;
		case IMGTOOLINFO_PTR_CLOSE:                         /* info->close = mac_image_exit */; break;
		case IMGTOOLINFO_PTR_INFO:                          info->info = mac_image_info; break;
		case IMGTOOLINFO_PTR_BEGIN_ENUM:                    info->begin_enum = mac_image_beginenum; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                     info->next_enum = mac_image_nextenum; break;
		case IMGTOOLINFO_PTR_FREE_SPACE:                    info->free_space = mac_image_freespace; break;
		case IMGTOOLINFO_PTR_READ_FILE:                     info->read_file = mac_image_readfile; break;
		case IMGTOOLINFO_PTR_LIST_FORKS:                    info->list_forks = mac_image_listforks; break;
		case IMGTOOLINFO_PTR_GET_ATTRS:                     info->get_attrs = mac_image_getattrs; break;
		case IMGTOOLINFO_PTR_SET_ATTRS:                     info->set_attrs = mac_image_setattrs; break;
		case IMGTOOLINFO_PTR_GET_ICON_INFO:                 info->get_iconinfo = mac_image_geticoninfo; break;
		case IMGTOOLINFO_PTR_SUGGEST_TRANSFER:              info->suggest_transfer = mac_image_suggesttransfer; break;
		case IMGTOOLINFO_PTR_FLOPPY_FORMAT:                 info->p = (void *) floppyoptions_apple35_mac; break;
	}
}



void mac_mfs_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "mac_mfs"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "Mac MFS Floppy"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_FLOPPY_CREATE:                 info->create = mfs_image_create; break;
		case IMGTOOLINFO_PTR_FLOPPY_OPEN:                   info->open = mfs_image_open; break;
		case IMGTOOLINFO_PTR_WRITE_FILE:                    info->write_file = mac_image_writefile; break;

		default: generic_mac_get_info(imgclass, state, info); break;
	}
}



void mac_hfs_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "mac_hfs"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "Mac HFS Floppy"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_FLOPPY_OPEN:                   info->open = hfs_image_open; break;

		default: generic_mac_get_info(imgclass, state, info); break;
	}
}
