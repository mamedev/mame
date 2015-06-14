// license:GPL-2.0+
// copyright-holders:Dirk Best
/****************************************************************************

    amiga.c

    Amiga floppies

****************************************************************************/


/*****************************************************************************
 Includes
*****************************************************************************/


#include <time.h>
#include <string.h>
#include <ctype.h>

#include "imgtool.h"
#include "iflopimg.h"
#include "formats/imageutl.h"



/*****************************************************************************
 Data structures
*****************************************************************************/


#define BSIZE (512) /* Block size is always 512 bytes for floppies */
#define TSIZE ((BSIZE/4) - 56) /* Size of data tables */
#define MSIZE ((BSIZE/4) - 1) /* Size of bitmaps */


enum disk_type
{
	DT_UNKNOWN       = -1,
	DT_OFS           =  0,
	DT_FFS           =  1,
	DT_OFS_INTL      =  2,
	DT_FFS_INTL      =  3,
	DT_OFS_INTL_DIRC =  4,
	DT_FFS_INTL_DIRC =  5
};


enum
{
	T_INVALID  =  0,
	T_HEADER   =  2,
	T_DATA     =  8,
	T_LIST     = 16,
	T_DIRCACHE = 33
};


enum sec_type
{
	ST_INVALID  =  0,
	ST_ROOT     =  1,
	ST_USERDIR  =  2,
	ST_FILE     = -3,
	ST_LINKFILE = -4,
	ST_LINKDIR  =  4,
	ST_SOFTLINK =  3
};


struct amiga_date
{
	UINT32 days;  /* days since 1 jan 78 */
	UINT32 mins;  /* minutes past midnight */
	UINT32 ticks; /* ticks (1/50 sec) past last minute */
};


struct root_block
{
	UINT32 ht_size;      /* Hash table size in long */
	UINT32 chksum;       /* Rootblock checksum */
	UINT32 ht[TSIZE];    /* Hash table (entry block number) */
	UINT32 bm_flag;      /* bitmap flag, -1 means VALID */
	UINT32 bm_pages[25]; /* bitmap blocks pointers (first one at bm_pages[0]) */
	amiga_date r;        /* last root alteration date */
	UINT8 name_len;      /* volume name length */
	UINT8 diskname[30];  /* volume name */
	amiga_date v;        /* last disk alteration date */
	amiga_date c;        /* filesystem creation date */
	UINT32 extension;    /* FFS: first directory cache block, 0 otherwise */
};


struct bitmap_block
{
	UINT32 chksum;      /* checksum, normal algorithm */
	UINT32 map[MSIZE];  /* bitmap */
};


struct bitmap_ext_block
{
	UINT32 map[MSIZE];  /* bitmap */
	UINT32 next;        /* next extension block */
};


struct file_block
{
	UINT32 header_key;   /* self pointer (to this block) */
	UINT32 high_seq;     /* number of data block ptr stored here */
	UINT32 first_data;   /* first data block ptr */
	UINT32 chksum;       /* same algorithm as rootblock */
	UINT32 data_blocks[TSIZE]; /* data blk ptr */
	UINT16 uid;          /* UserID */
	UINT16 gid;          /* GroupID */
	UINT32 protect;      /* protection flags (0 by default) */
	UINT32 byte_size;    /* file size in bytes */
	UINT8  comm_len;     /* file comment length */
	UINT8  comment[79];  /* comment (max. 79 chars permitted) */
	amiga_date date;     /* last change date */
	UINT8  name_len;     /* filename length */
	UINT8  filename[30]; /* filename (max. 30 chars permitted) */
	UINT32 real_entry;   /* FFS: unused, set to 0 */
	UINT32 next_link;    /* FFS: hardlinks chained list (first == newest */
	UINT32 hash_chain;   /* next entry ptr with same hash */
	UINT32 parent;       /* parent directory */
	UINT32 extension;    /* pointer to 1st file extension block */
};


struct file_ext_block
{
	UINT32 header_key;   /* self pointer (to this block) */
	UINT32 high_seq;     /* number of data block ptr stored here */
	UINT32 chksum;       /* same algorithm as rootblock */
	UINT32 data_blocks[TSIZE]; /* data blk ptr */
	UINT32 parent;       /* file header block */
	UINT32 extension;    /* pointer to next file extension block */
};


struct data_block
{
	UINT32 header_key;     /* self pointer (to this block) */
	UINT32 seq_num;        /* file data block number */
	UINT32 data_size;      /* data size */
	UINT32 next_data;      /* next data block ptr */
	UINT32 chksum;         /* checksum, rootblock algorithm */
	UINT8  data[BSIZE-24]; /* file data */
};


struct dir_block
{
	UINT32 header_key;   /* self pointer (to this block) */
	UINT32 chksum;       /* same algorithm as rootblock */
	UINT32 ht[TSIZE];    /* hash table (entry block number) */
	UINT8  uid;          /* UserID */
	UINT8  gid;          /* GroupID */
	UINT32 protect;      /* protection flags (0 by default) */
	UINT8  comm_len;     /* file comment length */
	UINT8  comment[79];  /* comment (max. 79 chars permitted) */
	amiga_date date;     /* last access date */
	UINT8  name_len;     /* directory name length */
	UINT8  dirname[30];  /* directory name (max. 30 chars permitted) */
	UINT32 next_link;    /* FFS: hardlinks chained list (first == newest */
	UINT32 hash_chain;   /* next entry ptr with same hash */
	UINT32 parent;       /* parent directory */
	UINT32 extension;    /* FFS: first directory cache block */
};


struct hardlink_block
{
	UINT32 header_key;   /* self pointer (to this block) */
	UINT32 chksum;       /* same algorithm as rootblock */
	UINT32 protect;      /* protection flags (0 by default) */
	UINT8  comm_len;     /* file comment length */
	UINT8  comment[79];  /* comment (max. 79 chars permitted) */
	amiga_date date;     /* last access date */
	UINT8  name_len;     /* hard link name length */
	UINT8  hlname[30];   /* hard link name (max. 30 chars permitted) */
	UINT32 real_entry;   /* FFS: pointer to "real" file or directory */
	UINT32 next_link;    /* FFS: hardlinks chained list (first == newest */
	UINT32 hash_chain;   /* next entry ptr with same hash */
	UINT32 parent;       /* parent directory */
	UINT32 sec_type;     /* secondary type, ST_LINKFILE/ST_LINKDIR */
};


struct softlink_block
{
	UINT32 header_key;   /* self pointer (to this block) */
	UINT32 chksum;       /* same algorithm as rootblock */
	UINT8  symbolic_name[BSIZE-224]; /* path name to referenced object */
	UINT32 protect;      /* protection flags (0 by default) */
	UINT8  comm_len;     /* file comment length */
	UINT8  comment[79];  /* comment (max. 79 chars permitted) */
	amiga_date date;     /* last access date */
	UINT8  name_len;     /* soft link name length */
	UINT8  slname[30];   /* soft link name (max. 30 chars permitted) */
	UINT32 hash_chain;   /* next entry ptr with same hash */
	UINT32 parent;       /* parent directory */
};


/* Basic Amiga floppy disk image info */
struct amiga_floppy
{
	imgtool_stream *stream;
	UINT8 sectors;
};


/* iterator used to walk through directory entries */
struct amiga_iterator
{
	unsigned int index;    /* current file index */
	int block;             /* block number we are iterating */
	UINT32 next_block;     /* next block in hash chain */
	int ht_index;          /* current index in the hash table */
	unsigned int eof : 1;  /* end of file listing reached? */
};


/*****************************************************************************
 Prototypes
*****************************************************************************/


static imgtoolerr_t amiga_image_read_sector(imgtool_image* img,
	UINT32 track, UINT32 head, UINT32 sector, void *buf, size_t len);
static imgtoolerr_t amiga_image_write_sector(imgtool_image* img,
	UINT32 track, UINT32 head, UINT32 sector, const void *buf, size_t len, int ddam);



/*****************************************************************************
 Utility functions
*****************************************************************************/


/* Amiga version of the toupper function */
static int intl_toupper(int c)
{
	return (c>='a' && c<='z') || (c>=224 && c<=254 && c!=247) ? c - ('a'-'A') : c ;
}


/* Amiga filename case insensitive string compare */
static int intl_stricmp(const char *s1, const char *s2)
{
	for (;;)
	{
		int c1 = intl_toupper(*s1++);
		int c2 = intl_toupper(*s2++);

		if (c1 == 0 || c1 != c2)
			return c1 - c2;
	}
}


/* Calculate the hash value for a filename */
static int hash_name(const char *name, int intl)
{
	int i, l = strlen(name);
	UINT32 hash = l;

	for(i = 0; i < l; i++)
	{
		hash *= 13;
		hash += (UINT8) (intl ? intl_toupper(name[i]) : toupper(name[i]));
		hash &= 0x7ff;
	}

	return hash % TSIZE; /* 0 < hash < 71 in case of BSIZE=512 */
}


/* Returns TRUE if year is a leap year */
static int is_leap(int year)
{
	return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}


/* Convert amiga time to standard time */
static time_t amiga_crack_time(amiga_date *date)
{
	int month_days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int year = 1978, month = 1, year_days = 365; /* base date */
	int day = date->days;
	struct tm t;

	/* initialize struct */
	memset(&t, 0, sizeof(t));

	/* first calculate the year */
	while (day >= year_days)
	{
		day -= year_days;
		year_days = is_leap(++year) ? 366 : 365;
	}

	/* then the month */
	while(day >= month_days[month-1])
	{
		day -= month_days[month-1];
		if (month == 2 && is_leap(year))
			day -= 1;
		month++;
	}

	/* fill the struct with our calculated values */
	t.tm_year = year - 1900;
	t.tm_mon  = month - 1;
	t.tm_mday = day + 1;
	t.tm_hour = date->mins / 60;
	t.tm_min  = date->mins % 60;
	t.tm_sec  = date->ticks / 50;

	return mktime(&t);
}


/* convert standard time to amiga time */
static void amiga_setup_time(time_t time, amiga_date *dest)
{
	struct tm t = *localtime(&time);
	int year;

	dest->days = 0;

	for (year = 1978; year < t.tm_year + 1900; year++)
	{
		dest->days += is_leap(year) ? 366 : 365;
	}

	dest->days += t.tm_yday;
	dest->mins = t.tm_hour * 60 + t.tm_min;
	dest->ticks = t.tm_sec * 50;
}


/* convert flags to human readable form */
static void amiga_decode_flags(UINT32 flags, char *dest)
{
	/* test for flags */
	dest[0] = (flags & 0x80) ? 'h' : '-';
	dest[1] = (flags & 0x40) ? 's' : '-';
	dest[2] = (flags & 0x20) ? 'p' : '-';
	dest[3] = (flags & 0x10) ? 'a' : '-';
	dest[4] = (flags & 0x08) ? '-' : 'r';
	dest[5] = (flags & 0x04) ? '-' : 'w';
	dest[6] = (flags & 0x02) ? '-' : 'e';
	dest[7] = (flags & 0x01) ? '-' : 'd';
	dest[8] = '\0';
}


static void copy_integer_array_be(UINT32 *dest, const UINT32 *source, int size)
{
	int i;

	for (i = 0; i < size; i++)
	{
		dest[i] = BIG_ENDIANIZE_INT32(source[i]);
	}
}


/* This function converts an array of UINT32s to an amiga_date */
static void copy_date_be(amiga_date *dest, const UINT32 *source)
{
	dest->days  = BIG_ENDIANIZE_INT32(source[0]);
	dest->mins  = BIG_ENDIANIZE_INT32(source[1]);
	dest->ticks = BIG_ENDIANIZE_INT32(source[2]);
}


/* Calculate the block checksum of a byte array */
static UINT32 block_checksum(UINT8 *buffer, int length)
{
	UINT32 chksum = 0;
	int i;

	for (i = 0; i < length/4; i++)
	{
		chksum += pick_integer_be(buffer, i*4, 4);
	}

	return -chksum;
}


/* Returns the total number of blocks in the image */
static int get_total_blocks(imgtool_image *img)
{
	amiga_floppy *f = (amiga_floppy *)imgtool_image_extra_bytes(img);

	return 2 * 80 * f->sectors;
}


/* Returns track, head and sector for a block */
static void find_block(amiga_floppy *f, int block, int *track,
	int *head, int *sector)
{
	*track = block / f->sectors;
	*head = (block - *track * f->sectors) / f->sectors;
	*sector = (block - *track * f->sectors) % f->sectors;
}


/* Generic read block */
static imgtoolerr_t read_block(imgtool_image *img, int block, UINT8 *buffer)
{
	imgtoolerr_t ret;
	int track, head, sector;

	find_block((amiga_floppy *) imgtool_image_extra_bytes(img), block, &track, &head, &sector);

	/* get block from image */
	ret = amiga_image_read_sector(img, track, head, sector, buffer, BSIZE);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


/* Generic write block */
static imgtoolerr_t write_block(imgtool_image *img, int block, const UINT8 *buffer)
{
	imgtoolerr_t ret;
	int track, head, sector;

	find_block((amiga_floppy *) imgtool_image_extra_bytes(img), block, &track, &head, &sector);

	/* write block to image */
	ret = amiga_image_write_sector(img, track, head, sector, buffer, BSIZE, 0);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


/* Return the type a block */
static sec_type get_block_type(imgtool_image *img, int block)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* get data */
	ret = read_block(img, block, buffer);
	if (ret) return ST_INVALID;

	/* return type */
	switch ((INT32) pick_integer_be(buffer, BSIZE-4, 4))
	{
	case  1: return ST_ROOT;
	case  2: return ST_USERDIR;
	case -3: return ST_FILE;
	case -4: return ST_LINKFILE;
	case  4: return ST_LINKDIR;
	case  3: return ST_SOFTLINK;
	default: return ST_INVALID;
	}
}


/* Read a bitmap block */
static imgtoolerr_t read_bitmap_block(imgtool_image *img, int block, bitmap_block *bm)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* read block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* fill in data */
	bm->chksum = pick_integer_be(buffer, 0, 4);
	copy_integer_array_be(bm->map, (UINT32 *) &buffer[4], MSIZE);

	return IMGTOOLERR_SUCCESS;
}


/* Write a bitmap block */
static imgtoolerr_t write_bitmap_block(imgtool_image *img, int block, const bitmap_block *bm)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* Setup buffer */
	place_integer_be(buffer, 0, 4, bm->chksum);
	copy_integer_array_be((UINT32 *) &buffer[4], bm->map, MSIZE);

	/* write block */
	ret = write_block(img, block, buffer);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


#ifdef UNUSED_FUNCTION
/* Read a bitmap extended block */
static imgtoolerr_t read_bitmap_ext_block(imgtool_image *img, int block, bitmap_ext_block *bm)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* read block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* fill in data */
	copy_integer_array_be(bm->map, (UINT32 *) &buffer, MSIZE);
	bm->next = pick_integer_be(buffer, BSIZE-4, 4);

	return IMGTOOLERR_SUCCESS;
}
#endif


/* Read the root block */
static imgtoolerr_t read_root_block(imgtool_image *img, root_block *root)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* get raw root block from image */
	ret = read_block(img, get_total_blocks(img)/2, buffer);
	if (ret) return ret;

	/* copy data to root_block */
	memset(root, 0, sizeof(root_block));

	root->ht_size = pick_integer_be(buffer, 12, 4);
	root->chksum = pick_integer_be(buffer, 20, 4);
	copy_integer_array_be(root->ht, (UINT32 *) &buffer[24], TSIZE);
	root->bm_flag = pick_integer_be(buffer, BSIZE-200, 4);
	copy_integer_array_be(root->bm_pages, (UINT32 *) &buffer[BSIZE-196], 25);
	copy_date_be(&root->r, (UINT32 *) &buffer[BSIZE-92]);
	root->name_len = pick_integer_be(buffer, BSIZE-80, 1);
	memcpy(root->diskname, &buffer[BSIZE-79], 30);
	copy_date_be(&root->v, (UINT32 *) &buffer[BSIZE-40]);
	copy_date_be(&root->c, (UINT32 *) &buffer[BSIZE-28]);
	root->extension = pick_integer_be(buffer, BSIZE-8, 4);

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t write_root_block(imgtool_image *img, const root_block *root)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* Setup buffer */
	memset(buffer, 0, BSIZE);

	place_integer_be(buffer, 0, 4, T_HEADER);
	place_integer_be(buffer, 12, 4, root->ht_size);
	place_integer_be(buffer, 20, 4, root->chksum);
	copy_integer_array_be((UINT32 *) &buffer[24], root->ht, TSIZE);
	place_integer_be(buffer, BSIZE-200, 4, root->bm_flag);
	copy_integer_array_be((UINT32 *) &buffer[BSIZE-196], root->bm_pages, 25);
	place_integer_be(buffer, BSIZE-92, 4, root->r.days);
	place_integer_be(buffer, BSIZE-88, 4, root->r.mins);
	place_integer_be(buffer, BSIZE-84, 4, root->r.ticks);
	place_integer_be(buffer, BSIZE-80, 1, root->name_len);
	memcpy(&buffer[BSIZE-79], root->diskname, root->name_len);
	place_integer_be(buffer, BSIZE-40, 4, root->v.days);
	place_integer_be(buffer, BSIZE-36, 4, root->v.mins);
	place_integer_be(buffer, BSIZE-32, 4, root->v.ticks);
	place_integer_be(buffer, BSIZE-28, 4, root->c.days);
	place_integer_be(buffer, BSIZE-24, 4, root->c.mins);
	place_integer_be(buffer, BSIZE-20, 4, root->c.ticks);
	place_integer_be(buffer, BSIZE-8, 4, root->extension);
	place_integer_be(buffer, BSIZE-4, 4, ST_ROOT);

	/* write root block to image */
	ret = write_block(img, get_total_blocks(img)/2, buffer);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


/* Read a file block */
static imgtoolerr_t read_file_block(imgtool_image *img, int block, file_block *fb)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* read block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* fill in data */
	fb->header_key = pick_integer_be(buffer, 4, 4);
	fb->high_seq = pick_integer_be(buffer, 8, 4);
	fb->first_data = pick_integer_be(buffer, 16, 4);
	fb->chksum = pick_integer_be(buffer, 20, 4);
	copy_integer_array_be(fb->data_blocks, (UINT32 *) &buffer[24], TSIZE);
	fb->uid = pick_integer_be(buffer, BSIZE-196, 2);
	fb->gid = pick_integer_be(buffer, BSIZE-194, 2);
	fb->protect = pick_integer_be(buffer, BSIZE-192, 4);
	fb->byte_size = pick_integer_be(buffer, BSIZE-188, 4);
	fb->comm_len = pick_integer_be(buffer, BSIZE-184, 1);
	memcpy(fb->comment, &buffer[BSIZE-183], 79);
	copy_date_be(&fb->date, (UINT32 *) &buffer[BSIZE-92]);
	fb->name_len = pick_integer_be(buffer, BSIZE-80, 1);
	memcpy(fb->filename, (UINT32 *) &buffer[BSIZE-79], 30);
	fb->real_entry = pick_integer_be(buffer, BSIZE-44, 4);
	fb->next_link = pick_integer_be(buffer, BSIZE-40, 4);
	fb->hash_chain = pick_integer_be(buffer, BSIZE-16, 4);
	fb->parent = pick_integer_be(buffer, BSIZE-12, 4);
	fb->extension = pick_integer_be(buffer, BSIZE-8, 4);

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t read_file_ext_block(imgtool_image *img, int block, file_ext_block *fe)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* read block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* fill in data */
	fe->header_key = pick_integer_be(buffer, 4, 4);
	fe->high_seq = pick_integer_be(buffer, 8, 4);
	fe->chksum = pick_integer_be(buffer, 20, 4);
	copy_integer_array_be(fe->data_blocks, (UINT32 *) &buffer[24], TSIZE);
	fe->parent = pick_integer_be(buffer, BSIZE-12, 4);
	fe->extension = pick_integer_be(buffer, BSIZE-8, 4);

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t read_data_block(imgtool_image *img, int block, data_block *d)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* read block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* fill in data */
	d->header_key = pick_integer_be(buffer, 4, 4);
	d->seq_num = pick_integer_be(buffer, 8, 4);
	d->data_size = pick_integer_be(buffer, 12, 4);
	d->next_data = pick_integer_be(buffer, 16, 4);
	d->chksum = pick_integer_be(buffer, 20, 4);
	memcpy(d->data, &buffer[24], BSIZE-24);

	return IMGTOOLERR_SUCCESS;
}


/* Read a directory block */
static imgtoolerr_t read_dir_block(imgtool_image *img, int block, dir_block *db)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* read block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* fill in data */
	db->header_key = pick_integer_be(buffer, 4, 4);
	db->chksum = pick_integer_be(buffer, 20, 4);
	copy_integer_array_be(db->ht, (UINT32 *) &buffer[24], TSIZE);
	db->uid = pick_integer_be(buffer, BSIZE-196, 2);
	db->gid = pick_integer_be(buffer, BSIZE-194, 2);
	db->protect = pick_integer_be(buffer, BSIZE-192, 4);
	db->comm_len = pick_integer_be(buffer, BSIZE-184, 1);
	memcpy(db->comment, &buffer[BSIZE-183], 79);
	copy_date_be(&db->date, (UINT32 *) &buffer[BSIZE-92]);
	db->name_len = pick_integer_be(buffer, BSIZE-80, 1);
	memcpy(db->dirname, (UINT32 *) &buffer[BSIZE-79], 30);
	db->next_link = pick_integer_be(buffer, BSIZE-40, 4);
	db->hash_chain = pick_integer_be(buffer, BSIZE-16, 4);
	db->parent = pick_integer_be(buffer, BSIZE-12, 4);
	db->extension = pick_integer_be(buffer, BSIZE-8, 4);

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t write_dir_block(imgtool_image *img, int block, const dir_block *db)
{
	UINT8 buffer[BSIZE];

	/* Setup buffer */
	memset(buffer, 0, BSIZE);

	/* Copy data */
	place_integer_be(buffer, 0, 4, T_HEADER);
	place_integer_be(buffer, 4, 4, db->header_key);
	place_integer_be(buffer, 20, 4, db->chksum);
	copy_integer_array_be((UINT32 *) &buffer[24], db->ht, TSIZE);
	place_integer_be(buffer, BSIZE-196, 2, db->uid);
	place_integer_be(buffer, BSIZE-194, 2, db->gid);
	place_integer_be(buffer, BSIZE-192, 4, db->protect);
	place_integer_be(buffer, BSIZE-184, 1, db->comm_len);
	memcpy((UINT32 *) &buffer[BSIZE-183], db->comment, db->comm_len);
	place_integer_be(buffer, BSIZE-92, 4, db->date.days);
	place_integer_be(buffer, BSIZE-88, 4, db->date.mins);
	place_integer_be(buffer, BSIZE-84, 4, db->date.ticks);
	place_integer_be(buffer, BSIZE-80, 1, db->name_len);
	memcpy((UINT32 *) &buffer[BSIZE-79], db->dirname, db->name_len);
	place_integer_be(buffer, BSIZE-40, 4, db->next_link);
	place_integer_be(buffer, BSIZE-16, 4, db->hash_chain);
	place_integer_be(buffer, BSIZE-12, 4, db->parent);
	place_integer_be(buffer, BSIZE-8, 4, db->extension);
	place_integer_be(buffer, BSIZE-4, 4, ST_USERDIR);

	/* Write block to disk */
	return write_block(img, block, buffer);
}


static imgtoolerr_t read_hardlink_block(imgtool_image *img, int block, hardlink_block *hl)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* read block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* fill in data */
	hl->header_key = pick_integer_be(buffer, 4, 4);
	hl->chksum = pick_integer_be(buffer, 20, 4);
	hl->protect = pick_integer_be(buffer, BSIZE-192, 4);
	hl->comm_len = pick_integer_be(buffer, BSIZE-184, 1);
	memcpy(hl->comment, &buffer[BSIZE-183], 79);
	copy_date_be(&hl->date, (UINT32 *) &buffer[BSIZE-92]);
	hl->name_len = pick_integer_be(buffer, BSIZE-80, 1);
	memcpy(hl->hlname, (UINT32 *) &buffer[BSIZE-79], 30);
	hl->real_entry = pick_integer_be(buffer, BSIZE-44, 4);
	hl->next_link = pick_integer_be(buffer, BSIZE-40, 4);
	hl->hash_chain = pick_integer_be(buffer, BSIZE-16, 4);
	hl->parent = pick_integer_be(buffer, BSIZE-12, 4);
	hl->sec_type = pick_integer_be(buffer, BSIZE-4, 4);

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t read_softlink_block(imgtool_image *img, int block, softlink_block *sl)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* read block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* fill in data */
	sl->header_key = pick_integer_be(buffer, 4, 4);
	sl->chksum = pick_integer_be(buffer, 20, 4);
	memcpy(sl->symbolic_name, &buffer[24], BSIZE-224);
	sl->protect = pick_integer_be(buffer, BSIZE-192, 4);
	sl->comm_len = pick_integer_be(buffer, BSIZE-184, 1);
	memcpy(sl->comment, &buffer[BSIZE-183], 79);
	copy_date_be(&sl->date, (UINT32 *) &buffer[BSIZE-92]);
	sl->name_len = pick_integer_be(buffer, BSIZE-80, 1);
	memcpy(sl->slname, (UINT32 *) &buffer[BSIZE-79], 30);
	sl->hash_chain = pick_integer_be(buffer, BSIZE-16, 4);
	sl->parent = pick_integer_be(buffer, BSIZE-12, 4);

	return IMGTOOLERR_SUCCESS;
}


/* Returns the disk type */
static disk_type get_disk_type(imgtool_image *img)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	ret = read_block(img, 0, buffer);
	if (ret) return DT_UNKNOWN;

	switch(buffer[3])
	{
	case  0: return DT_OFS;
	case  1: return DT_FFS;
	case  2: return DT_OFS_INTL;
	case  3: return DT_FFS_INTL;
	case  4: return DT_OFS_INTL_DIRC;
	case  5: return DT_FFS_INTL_DIRC;
	default: return DT_UNKNOWN;
	}
}


/* Returns TRUE if the disk is formatted with the FastFileSystem */
static int is_ffs(imgtool_image *img)
{
	disk_type t = get_disk_type(img);

	return ((t == DT_FFS ||
				t == DT_FFS_INTL ||
				t == DT_FFS_INTL_DIRC) ? TRUE : FALSE);
}


/* Returns TRUE if the disk uses the international mode */
static int is_intl(imgtool_image *img)
{
	disk_type t = get_disk_type(img);

	return ((t == DT_OFS_INTL ||
				t == DT_FFS_INTL ||
				t == DT_OFS_INTL_DIRC ||
				t == DT_FFS_INTL_DIRC) ? TRUE : FALSE);
}

#ifdef UNUSED_FUNCTION
/* Returns TRUE if the disk uses the directory cache mode */
static int is_dirc(imgtool_image *img)
{
	disk_type t = get_disk_type(img);

	return ((t == DT_OFS_INTL_DIRC ||
				t == DT_FFS_INTL_DIRC) ? TRUE : FALSE);
}
#endif

static imgtoolerr_t get_hash_table(imgtool_image *img, int block, UINT32 *ht)
{
	imgtoolerr_t ret;

	switch (get_block_type(img, block))
	{
	case ST_USERDIR:
	{
		dir_block dir;

		/* get the directory block */
		ret = read_dir_block(img, block, &dir);
		if (ret) return ret;

		/* copy data */
		memcpy(ht, &dir.ht, TSIZE*4);

		return IMGTOOLERR_SUCCESS;
	}

	case ST_ROOT:
	{
		root_block root;

		/* get the root block */
		ret = read_root_block(img, &root);
		if (ret) return ret;

		/* copy data */
		memcpy(ht, &root.ht, TSIZE*4);

		return IMGTOOLERR_SUCCESS;
	}

	default:
		return IMGTOOLERR_UNEXPECTED;
	}
}


static imgtoolerr_t set_hash_table(imgtool_image *img, int block, const UINT32 *ht)
{
	UINT8 buffer[BSIZE];
	imgtoolerr_t ret;

	/* Read block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* Copy new hash table into it */
	copy_integer_array_be((UINT32 *) &buffer[24], ht, TSIZE);

	/* Write it back again */
	ret = write_block(img, block, buffer);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}

#ifdef UNUSED_FUNCTION
static imgtoolerr_t get_root_hash_table(imgtool_image *img, UINT32 *ht)
{
	return get_hash_table(img, get_total_blocks(img)/2, ht);
}
#endif

static imgtoolerr_t get_blockname(imgtool_image *img, int block, char *dest)
{
	UINT8 buffer[BSIZE];
	imgtoolerr_t ret;

	/* Read the block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* Copy filename out of the buffer */
	memset(dest, 0, 31);
	memcpy(dest, &buffer[BSIZE-79], buffer[BSIZE-80]);

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t get_hash_chain(imgtool_image *img, int block, UINT32 *chain)
{
	UINT8 buffer[BSIZE];
	imgtoolerr_t ret;

	/* Read block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* Get chain value */
	*chain = pick_integer_be(buffer, BSIZE-16, 4);

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t set_hash_chain(imgtool_image *img, int block, UINT32 chain)
{
	UINT8 buffer[BSIZE];
	imgtoolerr_t ret;

	/* Read block */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* Copy new hash chain value into it */
	place_integer_be(buffer, BSIZE-16, 4, chain);

	/* Write it back again */
	ret = write_block(img, block, buffer);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t walk_hash_chain(imgtool_image *img, const char *path, int start_block, int *prev_block, int *block)
{
	imgtoolerr_t err;
	UINT32 hash_chain;
	char name[31];

	/* choose compare function depending on intl mode */
	int (*cmp)(const char *, const char *) = is_intl(img) ? &intl_stricmp : &core_stricmp;

	/* initialize filenames */
	memset(name, 0, sizeof(name));

	switch (get_block_type(img, start_block))
	{
	case ST_USERDIR:
	{
		dir_block dir;

		/* read block */
		err = read_dir_block(img, start_block, &dir);
		if (err) return err;

		/* copy filename string and next hash */
		memcpy(name, dir.dirname, dir.name_len);
		hash_chain = dir.hash_chain;

		break;
	}

	case ST_FILE:
	{
		file_block file;

		/* read block */
		err = read_file_block(img, start_block, &file);
		if (err) return err;

		/* copy filename string and next hash */
		memcpy(name, file.filename, file.name_len);
		hash_chain = file.hash_chain;

		break;
	}

	case ST_SOFTLINK:
	{
		softlink_block sl;

		/* read block */
		err = read_softlink_block(img, start_block, &sl);
		if (err) return err;

		/* copy filename string and next hash */
		memcpy(name, sl.slname, sl.name_len);
		hash_chain = sl.hash_chain;

		break;
	}

	case ST_LINKDIR:
	case ST_LINKFILE:
	{
		hardlink_block hl;

		/* read block */
		err = read_hardlink_block(img, start_block, &hl);
		if (err) return err;

		/* copy filename string and next hash */
		memcpy(name, hl.hlname, hl.name_len);
		hash_chain = hl.hash_chain;
		break;
	}

	default:
		return IMGTOOLERR_UNEXPECTED;

	}

	/* if we haven't found the right filename but there are linked entries,
	 * walk up the chain */
	if ((*cmp)(name, path) != 0 && hash_chain != 0)
	{
		*prev_block = start_block;
		return walk_hash_chain(img, path, hash_chain, prev_block, block);
	}

	/* found the correct block, return */
	if ((*cmp)(name, path) == 0)
	{
		*block = start_block;
		return IMGTOOLERR_SUCCESS;
	}

	/* we should never get here */
	return IMGTOOLERR_UNEXPECTED;
}


/* Returns the block number for a dir/file/link entry given as NUL delimited
 * list of path parts, for example "dir1\0dir2\0dir3" returns the block number
 * for directory "dir3" */
static imgtoolerr_t find_entry(imgtool_image *img, const char *path, int start_block, int *block)
{
	imgtoolerr_t ret;
	const char *next_path;
	int current_block, prev;
	UINT32 ht[TSIZE];

	/* get the hash table */
	ret = get_hash_table(img, start_block, ht);
	if (ret) return ret;

	/* calculate hash and get block for initial entry */
	current_block = ht[hash_name(path, is_intl(img))];

	/* check if there was a match in the hash table */
	if (current_block == 0)
	{
		return IMGTOOLERR_PATHNOTFOUND;
	}

	/* walk the linked hash list */
	ret = walk_hash_chain(img, path, current_block, &prev, block);
	if (ret) return ret;

	/* follow links */
	switch (get_block_type(img, *block))
	{
	case ST_SOFTLINK:

		/* TODO: Softlink support */
		return IMGTOOLERR_UNIMPLEMENTED;

	case ST_LINKDIR:
	case ST_LINKFILE:
	{
		hardlink_block hl;

		ret = read_hardlink_block(img, *block, &hl);
		if (ret) return ret;

		*block = hl.real_entry;

		break;
	}

	default:
		break;
	}

	/* get next path part */
	next_path = path + strlen(path) + 1;

	/* if there are more path parts, search the next block */
	if (next_path[0])
	{
		return find_entry(img, next_path, *block, block);
	}

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t get_block_chksum(imgtool_image *img, int block, UINT32 *chksum, int bitmap)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* get block data */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* clear old checksum */
	if (bitmap)
	{
		memset(buffer, 0, 4);
	}
	else
	{
		memset(&buffer[20], 0, 4);
	}

	/* calulate checksum */
	*chksum = block_checksum(buffer, BSIZE);

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t fix_chksum(imgtool_image *img, int block, int bitmap)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];
	UINT32 chksum;

	/* calculate block checksum */
	ret = get_block_chksum(img, block, &chksum, bitmap);
	if (ret) return ret;

	/* read block data */
	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* update checksum */
	if (bitmap)
	{
		place_integer_be(buffer, 0, 4, chksum);
	}
	else
	{
		place_integer_be(buffer, 20, 4, chksum);
	}

	/* write back new block data */
	ret = write_block(img, block, buffer);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t fix_block_chksum(imgtool_image *img, int block)
{
	return fix_chksum(img, block, FALSE);
}


static imgtoolerr_t fix_bitmap_chksum(imgtool_image *img, int block)
{
	return fix_chksum(img, block, TRUE);
}


/* Set a block as used */
static imgtoolerr_t bitmap_mark(imgtool_image *img, int block, int used)
{
	imgtoolerr_t ret;
	bitmap_block bm;
	root_block root;
	int page;

	block -= 2; /* subtract boot block sectors, 2 only for floppies! */

	ret = read_root_block(img, &root);
	if (ret) return ret;

	/* figure out bitmap block location */
	page = root.bm_pages[block / (MSIZE * 32)];

	/* get bitmap */
	ret = read_bitmap_block(img, page, &bm);
	if (ret) return ret;

	/* subtract pages we skip */
	block -= MSIZE * 32 * (block / (MSIZE * 32));

	/* mark as used or free */
	if (used)
	{
		bm.map[block/32] &= ~(1 << block % 32);
	}
	else
	{
		bm.map[block/32] |= (1 << block % 32);
	}

	/* write changed bitmap block back to disk */
	ret = write_bitmap_block(img, page, &bm);
	if (ret) return ret;

	/* update checksum */
	ret = fix_bitmap_chksum(img, page);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t bitmap_mark_used(imgtool_image *img, int block)
{
	return bitmap_mark(img, block, TRUE);
}


static imgtoolerr_t bitmap_mark_free(imgtool_image *img, int block)
{
	return bitmap_mark(img, block, FALSE);
}


static imgtoolerr_t update_block_modified_date(imgtool_image *img, int block)
{
	UINT8 buffer[BSIZE];
	imgtoolerr_t ret;
	amiga_date date;
	time_t now;

	ret = read_block(img, block, buffer);
	if (ret) return ret;

	/* Set new time */
	time(&now);
	amiga_setup_time(now, &date);

	/* Write new time into block */
	place_integer_be(buffer, BSIZE-92, 4, date.days);
	place_integer_be(buffer, BSIZE-88, 4, date.mins);
	place_integer_be(buffer, BSIZE-84, 4, date.ticks);

	/* Write block back to disk */
	ret = write_block(img, block, buffer);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t clear_hash_table_entry(imgtool_image *img, int parent, char *filename)
{
	imgtoolerr_t ret;
	UINT32 ht[TSIZE], chain;
	int index, entry, prev, block;

	ret = get_hash_table(img, parent, ht);
	if (ret) return ret;

	/* Calculate hash and get block for initial entry */
	index = hash_name(filename, is_intl(img));
	entry = ht[index];

	/* Walk the hash chain to get the real entry */
	ret = walk_hash_chain(img, filename, entry, &prev, &block);
	if (ret) return ret;

	/* Get chained value from block */
	ret = get_hash_chain(img, block, &chain);
	if (ret) return ret;

	/* Check if we need to change the hash table */
	if (entry == block)
	{
		/* Set new value (might be 0 if there were no linked entries) */
		ht[index] = chain;

		/* Save changed hash table */
		ret = set_hash_table(img, parent, ht);
		if (ret) return ret;

		/* Update last modified date */
		ret = update_block_modified_date(img, parent);
		if (ret) return ret;

		/* Calculate new checksum */
		ret = fix_block_chksum(img, parent);
		if (ret) return ret;
	}
	else
	{
		/* Save chained value to previous chain element */
		ret = set_hash_chain(img, prev, chain);
		if (ret) return ret;

		/* Calculate new checksum */
		ret = fix_block_chksum(img, prev);
		if (ret) return ret;
	}

	/* Mark our block as free */
	ret = bitmap_mark_free(img, block);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


/* Returns the number of the first bit that is set in the array */
static int get_first_bit(UINT32 *array, int size)
{
	int i;

	for (i = 0; i < size; i++)
	{
		if (array[i] != 0)
		{
			UINT32 v = array[i];
			int c;

			/* get first bit that is set */
			for (c = 0; (v & 1) == 0; c++) v >>= 1;

			/* return bit number */
			return i * 32 + c;
		}
	}

	return -1;
}


#ifdef UNUSED_FUNCTION
static imgtoolerr_t walk_bitmap_ext_blocks(imgtool_image *img, int start, int *block)
{
	imgtoolerr_t ret;
	bitmap_ext_block bm_ext;
	int bit;

	/* if we don't have a valid block, bail out */
	if (start == 0)
	{
		return IMGTOOLERR_NOSPACE;
	}

	/* get the extended bitmap block */
	ret = read_bitmap_ext_block(img, start, &bm_ext);
	if (ret) return ret;

	/* get the first bit that is set in the map */
	bit = get_first_bit(bm_ext.map, MSIZE);

	/* if we found one, return */
	if (bit != -1)
	{
		*block += bit;
		return IMGTOOLERR_SUCCESS;
	}

	/* increase on each call */
	*block += MSIZE * 32;

	/* else continue walking the list */
	return walk_bitmap_ext_blocks(img, bm_ext.next, block);
}
#endif


/* Searches for a block marked as free
 * TODO: bm_ext support for HDs */
static imgtoolerr_t find_free_block(imgtool_image *img, int *block)
{
	imgtoolerr_t ret;
	root_block root;
	int i;

	/* get root block */
	ret = read_root_block(img, &root);
	if (ret) return ret;

	/* iterate bitmap pointers */
	for (i = 0; i < 25; i++)
	{
		bitmap_block bm;

		/* get bitmap block pointed to */
		ret = read_bitmap_block(img, root.bm_pages[i], &bm);
		if (ret) return ret;

		*block = i * 32 * MSIZE + get_first_bit(bm.map, MSIZE);

		if (*block != -1)
		{
			*block += 2;
			return IMGTOOLERR_SUCCESS;
		}
	}

	/* if we get here we haven't found a free block */
	return IMGTOOLERR_NOSPACE;
}


static imgtoolerr_t add_entry(imgtool_image *img, int parent, int block)
{
	imgtoolerr_t ret;
	UINT32 ht[TSIZE];
	char name[31];
	int hash;

	ret = get_blockname(img, block, name);
	if (ret) return ret;

	ret = get_hash_table(img, parent, ht);
	if (ret) return ret;

	hash = hash_name(name, is_intl(img));

	/* Check if there is already an entry with that name */
	if (ht[hash] != 0)
	{
		/* Save the old value to our hash chain */
		ret = set_hash_chain(img, block, ht[hash]);
		if (ret) return ret;
	}

	/* Write our block number into the table */
	ht[hash] = block;

	/* Write table back to disk */
	ret = set_hash_table(img, parent, ht);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


/* Recursively create new directory entries */
static imgtoolerr_t makedir(imgtool_image *img, const char *path, int parent)
{
	imgtoolerr_t ret;
	dir_block dir;
	time_t now;
	int block;

	if (!path)
	{
		return IMGTOOLERR_PARAMNEEDED;
	}

	if (!path[0])
	{
		return IMGTOOLERR_SUCCESS;
	}

	/* Get a free block */
	ret = find_free_block(img, &block);
	if (ret) return ret;

	/* Initialize entry */
	memset(&dir, 0, sizeof(dir_block));

	/* Copy data */
	time(&now);
	amiga_setup_time(now, &dir.date);
	dir.name_len = strlen(path);
	memcpy(dir.dirname, path, dir.name_len);
	dir.header_key = block;
	dir.parent = parent;

	/* Write block */
	ret = write_dir_block(img, block, &dir);
	if (ret) return ret;

	/* Fix checksum */
	ret = fix_block_chksum(img, block);
	if (ret) return ret;

	/* Link the new entry in the parent */
	ret = add_entry(img, parent, block);
	if (ret) return ret;

	/* Mark it as used */
	ret = bitmap_mark_used(img, block);
	if (ret) return ret;

	/* Create the next entry */
	return makedir(img, path + strlen(path) + 1, block);
}


/* Recursively checks the path parts and creates directories for them */
static imgtoolerr_t checkdir(imgtool_image *img, const char *path, int parent)
{
	imgtoolerr_t ret;
	int block;
	char first_part[31];

	memset(first_part, 0, sizeof(first_part));
	strcpy(first_part, path);

	/* Directories all the way down, bail out */
	if (!path[0])
	{
		return IMGTOOLERR_CANNOTUSEPATH;
	}

	/* Search for the entry */
	ret = find_entry(img, first_part, parent, &block);

	switch (ret)
	{
	case IMGTOOLERR_PATHNOTFOUND:

		/* There is no entry with this name yet, so we can just create them */
		return makedir(img, path, parent);

	case IMGTOOLERR_SUCCESS:

		if (get_block_type(img, block) == ST_USERDIR)
		{
			/* Go down one level */
			return checkdir(img, path + strlen(path) + 1, block);
		}
		else
		{
			/* There is an entry but it's not a directory, create it */
			return makedir(img, path, parent);
		}

	default: return ret;
	}
}


/* Writes the file data from the specified block into the stream */
static imgtoolerr_t write_file_block_data(imgtool_image *img, int block, int size, imgtool_stream *destf)
{
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];

	/* Check if we even need to write something */
	if (size == 0)
	{
		return IMGTOOLERR_SUCCESS;
	}

	if (is_ffs(img))
	{
		/* Get block and read directly into buffer */
		ret = read_block(img, block, buffer);
		if (ret) return ret;
	}
	else
	{
		data_block db;
		UINT32 chksum;

		ret = read_data_block(img, block, &db);
		if (ret) return ret;

		/* Verify data checksum */
		ret = get_block_chksum(img, block, &chksum, FALSE);
		if (ret) return ret;

		if (db.chksum != chksum)
		{
			return IMGTOOLERR_CORRUPTFILE;
		}

		/* Copy data to buffer */
		memcpy(buffer, db.data, size);
	}

	/* Write data to stream */
	if (stream_write (destf, buffer, size) != size)
	{
		return IMGTOOLERR_WRITEERROR;
	}

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t walk_data_block_ptr(imgtool_image *img, UINT32 *ptr, int *filesize, imgtool_stream *destf, int write)
{
	int i, blocksize = is_ffs(img) ? BSIZE : BSIZE-24;
	imgtoolerr_t ret;

	for (i = TSIZE-1; i >= 0; i--)
	{
		/* We write either blocksize bytes or whats remaining */
		int bytes_left = (*filesize >= blocksize) ? blocksize : *filesize;

		if (write)
		{
			ret = write_file_block_data(img, ptr[i], bytes_left, destf);
			if (ret) return ret;
		}
		else
		{
			ret = bitmap_mark_free(img, ptr[i]);
			if (ret) return ret;
		}

		*filesize -= bytes_left;

		/* Check if we are finished early */
		if (*filesize == 0) break;
	}

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t write_data_block_ptr(imgtool_image *img, UINT32 *ptr, int *filesize, imgtool_stream *destf)
{
	return walk_data_block_ptr(img, ptr, filesize, destf, TRUE);
}


/* Marks all blocks pointed to by the data block pointers as free */
static imgtoolerr_t clear_data_block_ptr(imgtool_image *img, UINT32 *ptr, int *filesize)
{
	return walk_data_block_ptr(img, ptr, filesize, NULL, FALSE);
}


static imgtoolerr_t walk_file_ext_data(imgtool_image *img, int block, int *filesize, imgtool_stream *destf, int write)
{
	file_ext_block file_ext;
	imgtoolerr_t ret;

	/* Get file extension block */
	ret = read_file_ext_block(img, block, &file_ext);
	if (ret) return ret;

	/* Write all data pointers in the table */
	ret = walk_data_block_ptr(img, file_ext.data_blocks, filesize, destf, write);
	if (ret) return ret;

	/* Check if we are finished */
	if (*filesize != 0)
	{
		if (file_ext.extension == 0)
		{
			/* We are not finished, but there are no more extension blocks */
			return IMGTOOLERR_CORRUPTFILE;
		}
		else
		{
			/* Write the next file extension block */
			return walk_file_ext_data(img, file_ext.extension, filesize, destf, write);
		}
	}

	/* Mark ourself as free if we not writing */
	if (!write)
	{
		ret = bitmap_mark_free(img, block);
		if (ret) return ret;
	}

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t write_file_ext_data(imgtool_image *img, int block, int *filesize, imgtool_stream *destf)
{
	return walk_file_ext_data(img, block, filesize, destf, TRUE);
}


static imgtoolerr_t clear_file_ext_data(imgtool_image *img, int block, int *filesize)
{
	return walk_file_ext_data(img, block, filesize, NULL, FALSE);
}


/* Updates the disk alteration date stored in the root block */
static imgtoolerr_t update_disk_alteration_date(imgtool_image *img)
{
	imgtoolerr_t ret;
	root_block root;
	time_t now;

	/* Get root block */
	ret = read_root_block(img, &root);
	if (ret) return ret;

	/* Get current time */
	time(&now);
	amiga_setup_time(now, &root.v);

	/* Write back new root block */
	ret = write_root_block(img, &root);
	if (ret) return ret;

	/* And update its checksum */
	ret = fix_block_chksum(img, get_total_blocks(img)/2);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


/*****************************************************************************
 Imgtool functions
*****************************************************************************/


static imgtoolerr_t amiga_image_open(imgtool_image *img, imgtool_stream *stream)
{
	amiga_floppy *f = (amiga_floppy *) imgtool_image_extra_bytes(img);
	UINT64 size = stream_size(stream);

	f->stream = stream;
	f->sectors = size/BSIZE/80/2;

	if (f->sectors != 11 && f->sectors != 22)
	{
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	return IMGTOOLERR_SUCCESS;
}


static void amiga_image_exit(imgtool_image *img)
{
}


static void amiga_image_info(imgtool_image *img, char *string, size_t len)
{
	imgtoolerr_t ret;
	root_block root;
	char info[255];
	time_t t_c, t_v, t_r;
	char c[19], v[19], r[19];

	ret = read_root_block(img, &root);
	if (ret) return;

	t_c = amiga_crack_time(&root.c);
	t_v = amiga_crack_time(&root.v);
	t_r = amiga_crack_time(&root.r);

	strftime(c, sizeof(c), "%d-%b-%y %H:%M:%S", localtime(&t_c));
	strftime(v, sizeof(v), "%d-%b-%y %H:%M:%S", localtime(&t_v));
	strftime(r, sizeof(r), "%d-%b-%y %H:%M:%S", localtime(&t_r));

	strcpy(info, "Volume     name: ");
	strncat(info, (char *)root.diskname, root.name_len);
	strcat(info, "\nVolume  created: ");
	strcat(info, c);
	strcat(info, "\nVolume modified: ");
	strcat(info, v);
	strcat(info, "\n  Root modified: ");
	strcat(info, r);

	strncpy(string, info, len);
}


static imgtoolerr_t amiga_image_read_sector(imgtool_image* img, UINT32 track, UINT32 head, UINT32 sector, void *buf, size_t len)
{
	amiga_floppy *f = (amiga_floppy *) imgtool_image_extra_bytes(img);

	/* skip ahead to the area we want to read */
	stream_seek(f->stream, track * (head+1) * f->sectors * BSIZE + sector * BSIZE, SEEK_CUR);

	if (stream_read(f->stream, buf, len) != len)
	{
		return IMGTOOLERR_READERROR;
	}

	/* reset stream */
	stream_seek(f->stream, 0, 0);

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t amiga_image_write_sector(imgtool_image* img, UINT32 track, UINT32 head, UINT32 sector, const void *buf, size_t len, int ddam)
{
	amiga_floppy *f = (amiga_floppy *) imgtool_image_extra_bytes(img);

	/* skip ahead to the area we want to write */
	stream_seek(f->stream, track * (head+1) * f->sectors * BSIZE + sector * BSIZE, SEEK_CUR);

	/* write data */
	if (stream_write(f->stream, buf, len) != len)
	{
		return IMGTOOLERR_WRITEERROR;
	}

	/* reset stream */
	stream_seek(f->stream, 0, 0);

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t amiga_image_get_sector_size(imgtool_image* img, UINT32 track, UINT32 head, UINT32 sector, UINT32 *sector_size)
{
	*sector_size = BSIZE;
	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t amiga_image_beginenum(imgtool_directory *enumeration, const char *path)
{
	int blocks = get_total_blocks(imgtool_directory_image(enumeration));
	imgtoolerr_t ret;
	amiga_iterator *iter;

	iter = (amiga_iterator *) imgtool_directory_extrabytes(enumeration);
	if (!iter) return IMGTOOLERR_OUTOFMEMORY;

	iter->index = 1;
	iter->ht_index = 0;
	iter->eof = 0;

	if (path[0])
	{
		/* search for the directory block, start with the root block */
		ret = find_entry(imgtool_directory_image(enumeration), path, blocks/2, &iter->block);
		if (ret) return ret;
	}
	else
	{
		/* we didn't get a path, use the root directory */
		iter->block = blocks / 2;
	}

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t amiga_image_nextenum(imgtool_directory *enumeration, imgtool_dirent *ent)
{
	amiga_iterator *iter = (amiga_iterator *) imgtool_directory_extrabytes(enumeration);
	imgtoolerr_t ret;
	UINT32 ht[TSIZE];
	int block;

	/* finished listing all entries? */
	if (iter->eof == 1 || iter->ht_index == TSIZE)
	{
		ent->eof = 1;
		return IMGTOOLERR_SUCCESS;
	}

	/* get hash table */
	ret = get_hash_table(imgtool_directory_image(enumeration), iter->block, ht);
	if (ret) return ret;

	/* skip empty hash table entries */
	while (ht[iter->ht_index] == 0)
	{
		iter->ht_index++;
		/* check if we are already at the end */
		if (iter->ht_index == TSIZE)
		{
			ent->eof = 1;
			return IMGTOOLERR_SUCCESS;
		}
	}

	/* get block number */
	block = (iter->next_block == 0) ? ht[iter->ht_index] : iter->next_block;

	switch (get_block_type(imgtool_directory_image(enumeration), block))
	{
	case ST_FILE:
	{
		file_block file;

		/* get block */
		ret = read_file_block(imgtool_directory_image(enumeration), block, &file);
		if (ret) return ret;

		/* fill directory entry */
		strncpyz(ent->filename, (char *)file.filename, file.name_len + 1);
		ent->filesize = file.byte_size;
		ent->lastmodified_time = amiga_crack_time(&file.date);
		amiga_decode_flags(file.protect, ent->attr);
		strncpyz(ent->comment, (char *)file.comment, file.comm_len + 1);

		iter->next_block = file.hash_chain;

		break;
	}

	case ST_USERDIR:
	{
		dir_block dir;

		/* get block */
		ret = read_dir_block(imgtool_directory_image(enumeration), block, &dir);
		if (ret) return ret;

		/* fill directory entry */
		strncpyz(ent->filename, (char *)dir.dirname, dir.name_len + 1);
		ent->lastmodified_time = amiga_crack_time(&dir.date);
		amiga_decode_flags(dir.protect, ent->attr);
		strncpyz(ent->comment, (char *)dir.comment, dir.comm_len + 1);
		ent->directory = 1;

		iter->next_block = dir.hash_chain;

		break;
	}

	case ST_SOFTLINK:
	{
		softlink_block sl;

		/* get block */
		ret = read_softlink_block(imgtool_directory_image(enumeration), block, &sl);
		if (ret) return ret;

		/* fill directory entry */
		strncpyz(ent->filename, (char *)sl.slname, sl.name_len + 1);
		ent->lastmodified_time = amiga_crack_time(&sl.date);
		amiga_decode_flags(sl.protect, ent->attr);
		strncpyz(ent->comment, (char *)sl.comment, sl.comm_len + 1);
		strcpy(ent->softlink, (char *)sl.symbolic_name);

		iter->next_block = sl.hash_chain;

		break;
	}

	case ST_LINKDIR:

		ent->directory = 1;

	case ST_LINKFILE:
	{
		hardlink_block hl;

		/* get block */
		ret = read_hardlink_block(imgtool_directory_image(enumeration), block, &hl);
		if (ret) return ret;

		/* get filesize from linked file */
		if (!ent->directory)
		{
			file_block file;
			ret = read_file_block(imgtool_directory_image(enumeration), hl.real_entry, &file);
			if (ret) return ret;
			ent->filesize = file.byte_size;
		}

		/* fill directory entry */
		strncpyz(ent->filename, (char *)hl.hlname, hl.name_len + 1);
		ent->lastmodified_time = amiga_crack_time(&hl.date);
		amiga_decode_flags(hl.protect, ent->attr);
		strncpyz(ent->comment, (char *)hl.comment, hl.comm_len + 1);
		ent->hardlink = 1;

		iter->next_block = hl.hash_chain;

		break;
	}

	default:
		return IMGTOOLERR_UNIMPLEMENTED;
	}

	/* if there are no linked entries, go to the next hash table entry */
	if (iter->next_block == 0)
	{
		iter->ht_index++;
	}

	/* jump to next index */
	iter->index++;

	return IMGTOOLERR_SUCCESS;
}


static void amiga_image_closeenum(imgtool_directory *enumeration)
{
	free(enumeration);
}


static imgtoolerr_t amiga_image_freespace(imgtool_partition *partition, UINT64 *size)
{
	imgtoolerr_t ret;
	imgtool_image *image = imgtool_partition_image(partition);
	const int data_size = is_ffs(image) ? BSIZE : BSIZE-24;
	root_block root;
	bitmap_block bm;
	int blocks, blocks_processed = 0, pages, i, c;
	UINT32 v;

	/* initialize size */
	*size = 0;

	/* get root block */
	ret = read_root_block(image, &root);
	if (ret) return ret;

	/* get total number of blocks in the image */
	blocks = get_total_blocks(image);

	/* subtract the two bootblock blocks (only for floppies!) */
	blocks -= 2;

	/* iterate all bitmap pages */
	for (pages = 0; pages < 25; pages++)
	{
		ret = read_bitmap_block(image, root.bm_pages[pages], &bm);
		if (ret) return ret;

		for (i = 0; i < MSIZE; i++)
		{
			v = bm.map[i];

			/* clear half used value */
			if ((blocks_processed + 32) > blocks)
				v &= ~(~0 << (blocks - blocks_processed));

			/* count bits */
			for (c = 0; v; c++)
				v &= v - 1;

			*size += c * data_size;

			blocks_processed += 32;

			if (blocks_processed >= blocks)
				return IMGTOOLERR_SUCCESS;
		}
	}

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t amiga_image_readfile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *destf)
{
	imgtool_image *img = imgtool_partition_image(partition);
	imgtoolerr_t ret;
	file_block file;
	int filesize, block;

	/* Search for the block number */
	ret = find_entry(img, filename, get_total_blocks(img)/2, &block);
	if (ret == IMGTOOLERR_PATHNOTFOUND) return IMGTOOLERR_FILENOTFOUND;
	if (ret) return ret;  /* Other errors */

	/* Phase 1: Follow data pointers */
	ret = read_file_block(img, block, &file);
	if (ret) return ret;

	filesize = file.byte_size;

	/* Write out file data pointed to by data block pointers */
	ret = write_data_block_ptr(img, file.data_blocks, &filesize, destf);
	if (ret) return ret;

	/* Check if we are done already */
	if (filesize == 0) return IMGTOOLERR_SUCCESS;

	/* Phase 2: Follow file extension blocks */
	ret = write_file_ext_data(img, file.extension, &filesize, destf);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


/* When a file is deleted, only its File header block number is cleared from
 * the Directory block (or from the same-hash-value list) and the bitmap is
 * updated. File header block, Data blocks and File extension blocks are not
 * cleared, but the bitmap blocks are updated. */
static imgtoolerr_t amiga_image_deletefile(imgtool_partition *partition, const char *fname)
{
	imgtool_image *img = imgtool_partition_image(partition);
	imgtoolerr_t ret;
	int parent, block;
	char filename[31];

	/* Initialize filename */
	memset(filename, 0, sizeof(filename));

	/* Search for the block number */
	ret = find_entry(img, fname, get_total_blocks(img)/2, &block);
	if (ret == IMGTOOLERR_PATHNOTFOUND) return IMGTOOLERR_FILENOTFOUND;
	if (ret) return ret;

	/* Get the parent block, where we need to clear the hash */
	switch (get_block_type(img, block))
	{
	case ST_FILE:
	{
		file_block file;
		int filesize;

		ret = read_file_block(img, block, &file);
		if (ret) return ret;

		filesize = file.byte_size;
		parent = file.parent;
		memcpy(filename, file.filename, file.name_len);

		/* Clear all linked data sectors */
		ret = clear_data_block_ptr(img, file.data_blocks, &filesize);
		if (ret) return ret;

		/* Clear extended file data sectors */
		if (filesize != 0)
		{
			ret = clear_file_ext_data(img, file.extension, &filesize);
			if (ret) return ret;
		}

		break;
	}

	case ST_LINKFILE:
	{
		softlink_block link;

		ret = read_softlink_block(img, block, &link);
		if (ret) return ret;

		parent = link.parent;
		memcpy(filename, link.slname, link.name_len);

		break;
	}

	default:
		return IMGTOOLERR_UNEXPECTED;
	}

	/* Clear hash table entry */
	ret = clear_hash_table_entry(img, parent, filename);
	if (ret) return ret;

	/* Update disk alteration date */
	ret = update_disk_alteration_date(img);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t amiga_image_writefile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *sourcef, option_resolution *opts)
{
	return IMGTOOLERR_UNIMPLEMENTED;
}


static imgtoolerr_t amiga_image_create(imgtool_image *img, imgtool_stream *stream, option_resolution *opts)
{
	amiga_floppy *f = (amiga_floppy *) imgtool_image_extra_bytes(img);
	const char *dskname = option_resolution_lookup_string(opts, 'N');
	imgtoolerr_t ret;
	UINT8 buffer[BSIZE];
	root_block root;
	bitmap_block bm;
	time_t now;
	int blocks;

	f->stream = stream;

	switch (option_resolution_lookup_int(opts, 'S'))
	{
	case 0: f->sectors = 11; break;
	case 1: f->sectors = 22; break;
	default: return IMGTOOLERR_PARAMCORRUPT;
	}

	/* initialize with zeros */
	memset(buffer, 0, BSIZE);

	/* add DOS magic string and flags */
	buffer[0] = 'D';
	buffer[1] = 'O';
	buffer[2] = 'S';
	buffer[3] = 0;

	/* File system */
	buffer[3] += (option_resolution_lookup_int(opts, 'F'));

	/* File system mode */
	buffer[3] += (option_resolution_lookup_int(opts, 'M'));

	/* write first bootblock sector */
	ret = write_block(img, 0, buffer);
	if (ret) return ret;

	/* reset with zeros */
	memset(buffer, 0, BSIZE);

	/* write second bootblock sector */
	ret = write_block(img, 1, buffer);
	if (ret) return ret;

	/* rootblock */
	memset(&root, 0, sizeof(root_block));

	blocks = get_total_blocks(img);

	root.chksum      = 0;
	root.ht_size     = TSIZE;
	root.bm_flag     = -1;
	root.bm_pages[0] = blocks/2 + 1;  /* generally it's located here */

	time(&now);
	amiga_setup_time(now, &root.r);
	amiga_setup_time(now, &root.v);
	amiga_setup_time(now, &root.c);

	/* volume name */
	if (dskname)
	{
		root.name_len = strlen(dskname);
		memcpy(&root.diskname, dskname, root.name_len);
	}
	else
	{
		root.name_len = strlen("Empty");
		memcpy(&root.diskname, "Empty", root.name_len);
	}

	/* write root block to disk */
	ret = write_root_block(img, &root);
	if (ret) return ret;

	/* calculate block checksum */
	ret = fix_block_chksum(img, blocks/2);
	if (ret) return ret;

	/* bitmap block */
	memset(&bm, 0xff, sizeof(bitmap_block));

	/* write bitmap block to disk */
	ret = write_bitmap_block(img, root.bm_pages[0], &bm);
	if (ret) return ret;

	/* set root and bitmap block as used */
	ret = bitmap_mark_used(img, blocks/2);
	if (ret) return ret;

	ret = bitmap_mark_used(img, root.bm_pages[0]);
	if (ret) return ret;

	/* write empty last block so that we don't get a truncated image */
	memset(buffer, 0, BSIZE);

	ret = write_block(img, blocks - 1, buffer);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t amiga_image_createdir(imgtool_partition *partition, const char *path)
{
	imgtool_image *img = imgtool_partition_image(partition);
	imgtoolerr_t ret;

	/* Create directories */
	ret = checkdir(img, path, get_total_blocks(img)/2);
	if (ret) return ret;

	/* Update disk alteration date */
	ret = update_disk_alteration_date(img);
	if (ret) return ret;

	return IMGTOOLERR_SUCCESS;
}


static imgtoolerr_t amiga_image_getattrs(imgtool_partition *partition, const char *path, const UINT32 *attrs, imgtool_attribute *values)
{
	return IMGTOOLERR_UNIMPLEMENTED;
}


static imgtoolerr_t amiga_image_setattrs(imgtool_partition *partition, const char *path, const UINT32 *attrs, const imgtool_attribute *values)
{
	return IMGTOOLERR_UNIMPLEMENTED;
}


static imgtoolerr_t amiga_image_geticoninfo(imgtool_partition *partition, const char *path, imgtool_iconinfo *iconinfo)
{
	return IMGTOOLERR_UNIMPLEMENTED;
}


static imgtoolerr_t amiga_image_suggesttransfer(imgtool_partition *partition, const char *fname, imgtool_transfer_suggestion *suggestions, size_t suggestions_length)
{
	return IMGTOOLERR_UNIMPLEMENTED;
}



/*****************************************************************************
 Create image options
*****************************************************************************/


static OPTION_GUIDE_START(amiga_createimage_optionguide)
	OPTION_STRING( 'N', "name", "Volume name" )
	OPTION_ENUM_START( 'S', "density", "Density" )
		OPTION_ENUM( 0, "dd", "Double Density" )
		OPTION_ENUM( 1, "hd", "High Density" )
	OPTION_ENUM_END
	OPTION_ENUM_START( 'F', "filesystem", "File system" )
		OPTION_ENUM( 0, "ofs", "OldFileSystem" )
		OPTION_ENUM( 1, "ffs", "FastFileSystem" )
	OPTION_ENUM_END
	OPTION_ENUM_START( 'M', "mode", "File system options" )
		OPTION_ENUM( 0, "none",  "None" )
		OPTION_ENUM( 2, "intl", "International Mode" )
		OPTION_ENUM( 4, "dirc", "International Mode with Directory Cache" )
	OPTION_ENUM_END
OPTION_GUIDE_END



/*****************************************************************************
 Imgtool module declaration
*****************************************************************************/


/* Amiga floppy disk attributes */
void amiga_floppy_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES:          info->i = sizeof(amiga_floppy); break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:      info->i = sizeof(amiga_iterator); break;
		case IMGTOOLINFO_INT_SUPPORTS_LASTMODIFIED_TIME: info->i = 1; break;
		case IMGTOOLINFO_INT_PATH_SEPARATOR:             info->i = '/'; break;
		case IMGTOOLINFO_INT_CHARSET:                    info->i = IMGTOOL_CHARSET_ISO_8859_1; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                       strcpy(info->s = imgtool_temp_str(), "amiga_floppy"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                strcpy(info->s = imgtool_temp_str(), "Amiga floppy disk image (OFS/FFS format)"); break;
		case IMGTOOLINFO_STR_FILE_EXTENSIONS:            strcpy(info->s = imgtool_temp_str(), "adf"); break;
		case IMGTOOLINFO_STR_FILE:                       strcpy(info->s = imgtool_temp_str(), __FILE__); break;
		case IMGTOOLINFO_STR_EOLN:                       strcpy(info->s = imgtool_temp_str(), EOLN_LF); break;
		case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC:        strcpy(info->s = imgtool_temp_str(), "S[0]-1;F[0]-1;M[0]/2/4"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_OPEN:                       info->open = amiga_image_open; break;
		case IMGTOOLINFO_PTR_CLOSE:                      info->close = amiga_image_exit; break;
		case IMGTOOLINFO_PTR_READ_SECTOR:                info->read_sector = amiga_image_read_sector; break;
		case IMGTOOLINFO_PTR_WRITE_SECTOR:               info->write_sector = amiga_image_write_sector; break;
		case IMGTOOLINFO_PTR_GET_SECTOR_SIZE:            info->get_sector_size = amiga_image_get_sector_size; break;
		case IMGTOOLINFO_PTR_CREATE:                     info->create = amiga_image_create; break;
		case IMGTOOLINFO_PTR_INFO:                       info->info = amiga_image_info; break;
		case IMGTOOLINFO_PTR_BEGIN_ENUM:                 info->begin_enum = amiga_image_beginenum; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                  info->next_enum = amiga_image_nextenum; break;
		case IMGTOOLINFO_PTR_CLOSE_ENUM:                 info->close_enum = amiga_image_closeenum; break;
		case IMGTOOLINFO_PTR_FREE_SPACE:                 info->free_space = amiga_image_freespace; break;
		case IMGTOOLINFO_PTR_READ_FILE:                  info->read_file = amiga_image_readfile; break;
		case IMGTOOLINFO_PTR_WRITE_FILE:                 info->write_file = amiga_image_writefile; break;
		case IMGTOOLINFO_PTR_DELETE_FILE:                info->delete_file = amiga_image_deletefile; break;
		case IMGTOOLINFO_PTR_CREATE_DIR:                 info->create_dir = amiga_image_createdir; break;
		case IMGTOOLINFO_PTR_GET_ATTRS:                  info->get_attrs = amiga_image_getattrs; break;
		case IMGTOOLINFO_PTR_SET_ATTRS:                  info->set_attrs = amiga_image_setattrs; break;
		case IMGTOOLINFO_PTR_GET_ICON_INFO:              info->get_iconinfo = amiga_image_geticoninfo; break;
		case IMGTOOLINFO_PTR_SUGGEST_TRANSFER:           info->suggest_transfer = amiga_image_suggesttransfer; break;
		case IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE:       info->createimage_optguide = amiga_createimage_optionguide; break;
	}
}
