// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*
    Handlers for ti990 disk images

    Disk images are in MESS format.

    Raphael Nabet, 2003
*/

#include "imgtool.h"

#include "opresolv.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* Max sector length is bytes.  Generally 256, except for a few older disk
units which use 288-byte-long sectors, and SCSI units which generally use
standard 512-byte-long sectors. */
/* I chose a limit of 512.  No need to use more until someone creates CD-ROMs
for TI990. */
#define MAX_SECTOR_SIZE 512
#define MIN_SECTOR_SIZE 256

#define MAX_CYLINDERS 2047
#define MAX_HEADS 31
#define MAX_SECTORS_PER_TRACK 256   /* 255 for 512-byte-long sector, so that the total number of words that can be recorded on a track may fit in one 16-bit word (store registers command) */

/* Max path len in chars: this value is 48 characters in DX10, but since the
path includes at least a leading '.', our value is 47. */
/* Since path generally include a volume name that can be up to 8 character
long, we should display a warning message when the user wants to create a path
that is longer than 39 characters. */
#define MAX_PATH_LEN 47
#define MAX_SAFE_PATH_LEN 39

#define MAX_DIR_LEVEL 25    /* We need to put a recursion limit to avoid endless recursion hazard */


struct UINT16BE
{
	uint8_t bytes[2];
};

struct UINT32BE
{
	uint8_t bytes[4];
};

static inline uint16_t get_UINT16BE(UINT16BE word)
{
	return (word.bytes[0] << 8) | word.bytes[1];
}

static inline void set_UINT16BE(UINT16BE *word, uint16_t data)
{
	word->bytes[0] = (data >> 8) & 0xff;
	word->bytes[1] = data & 0xff;
}

static inline uint32_t get_UINT32BE(UINT32BE word)
{
	return (word.bytes[0] << 24) | (word.bytes[1] << 16) | (word.bytes[2] << 8) | word.bytes[3];
}

static inline void set_UINT32BE(UINT32BE *word, uint32_t data)
{
	word->bytes[0] = (data >> 24) & 0xff;
	word->bytes[1] = (data >> 16) & 0xff;
	word->bytes[2] = (data >> 8) & 0xff;
	word->bytes[3] = data & 0xff;
}

/*
    disk image header
*/
struct disk_image_header
{
	UINT32BE cylinders;         /* number of cylinders on hard disk (big-endian) */
	UINT32BE heads;             /* number of heads on hard disk (big-endian) */
	UINT32BE sectors_per_track; /* number of sectors per track on hard disk (big-endian) */
	UINT32BE bytes_per_sector;  /* number of bytes of data per sector on hard disk (big-endian) */
};

enum
{
	header_len = sizeof(disk_image_header)
};

/*
    Disk structure:

    Track 0 Sector 0: see below
    Track 0 Sector 1: list of bad ADU
    Track 0 Sector 2 through N: disk allocation bitmap
    Track 1 Sector 0 through N-2: optional disk program image loader
    Track 1 Sector N-1: copy of Track 0 Sector 0
    Track 1 Sector N: copy of Track 0 Sector 1
    Last cylinder has diagnostic information (reported as .S$DIAG)
    Remaining sectors are used for fdr and data.
*/

/*
    SC0 record (Disk sector 0)
*/
struct ti990_sc0
{
	char        vnm[8];         /* volume name */
	UINT16BE    tna;            /* total number of ADUs */
	uint8_t       sbm;            /* starting sector of bit maps */
	uint8_t       tbm;            /* total bit maps */
	UINT16BE    rl;             /* track 01 record length */
	UINT16BE    slt;            /* system loader track address */
	uint8_t       fill00[6];      /* * * RESERVED * * */
	UINT16BE    nba;            /* total number of bad ADUs on disk */
	UINT16BE    sle;            /* system loader entry point */
	UINT16BE    sll;            /* system loader length */
	uint8_t       fill01[8];      /* * * RESERVED * * */
	UINT16BE    lt1;            /* system loader track (copy 2) */
	uint8_t       fill02[8];      /* * * RESERVED * * */
	char        pi1[8];         /* primary system file name */
	char        pi2[8];         /* secondary system file name */
	UINT16BE    pif;            /* system selector */
	UINT16BE    vda;            /* volume directory ADU */
	UINT16BE    vpl;            /* vcatalog physical record length */
	UINT16BE    spa;            /* sectors per ADU */
	uint8_t       dcd[4];         /* disk creation date */
	char        pf1[8];         /* primary program file */
	char        pf2[8];         /* secondary program file */
	UINT16BE    pff;            /* program file switch */
	char        of1[8];         /* primary overlay file */
	char        of2[8];         /* secondary overlay file */
	UINT16BE    off;            /* overlay file switch */
	char        il1[8];         /* primary intermediate loader */
	char        il2[8];         /* secondary intermediate loader */
	UINT16BE    ilf;            /* intermediate loader flag */
	char        din[8];         /* diagnostic file name */
	UINT16BE    dif;            /* diagnostic flag */
	UINT16BE    drs;            /* default physical record size "DBUILD DETERMINES DEFAULT PRS" (whatever it means) */
	UINT16BE    bal;            /* starting sector of bad ADU list */
	UINT16BE    spr;            /* track 0 sectors per record */
	char        wf1[8];         /* WCS primary microcode file */
	char        wf2[8];         /* WCS secondary microcode file */
	UINT16BE    wff;            /* WCS flag switch */
	UINT16BE    vif;            /* track 1 select flag (whatever it means) volume information copied flag */
	UINT16BE    sta;            /* state of disk: */
									/*  1 = disk surface has not been tested for defects */
									/*  2 = disk surface has been tested, but no file system has been installed */
									/*  3 = a file system has been installed */
	UINT16BE    dct;            /* disk creation time */
	UINT16BE    fsf;            /* * * RESERVED * * */
	/* SCOSIZ = >AA */
};

/*
    DOR (Directory Overhead Record)
*/
struct ti990_dor
{
	UINT16BE nrc;               /* # records in directory (minus DOR) nrc = nfl + nar (+ tfc???) */
	UINT16BE nfl;               /* # files currently in directory */
	UINT16BE nar;               /* # of available records */
	UINT16BE tfc;               /* number of temporary files */
	char dnm[8];                /* directory file name (VCATALOG for root) */
	UINT16BE lvl;               /* level # of directory (0 for root, 1 for children of root, 2 for grandchildren, etc) */
	char pnm[8];                /* name of parent directory (VCATALOG for root, even though it makes little sense) */
	UINT16BE prs;               /* "default physical record length (used for file creation)" */
	/* DORSIZ = >1C */
};

/*
    file flags found in fdr
*/
enum
{
	ace_flg_rdf = 0x8000,       /* read access flag */
	ace_flg_wrf = 0x4000,       /* write access flag */
	ace_flg_dlf = 0x2000,       /* delete access flag */
	ace_flg_exf = 0x1000,       /* execute access flag */
	ace_flg_ctf = 0x0800,       /* control access flag */

	fdr_fl1_sec = 0x8000,       /* file secured flag */

	fdr_flg_fu  = 0xc000,       /* file usage bits */
	fdr_flg_fu_shift  = 14,
	fdr_flg_fmt = 0x3000,       /* file format bits */
	fdr_flg_fmt_shift  = 12,
	fdr_flg_all = 0x0800,       /* extendable file flag */
	fdr_flg_ft  = 0x0600,       /* file type bits */
	fdr_flg_ft_shift  = 9,
	fdr_flg_wpb = 0x0100,       /* write protect bit */
	fdr_flg_dpb = 0x0080,       /* delete protect bit */
	fdr_flg_tmp = 0x0040,       /* temporary file flag */
	fdr_flg_blb = 0x0020,       /* blocked file flag */
	fdr_flg_ali = 0x0010,       /* alias flag bit */
	fdr_flg_fwt = 0x0008,       /* forced write / partial logging */
	fdr_flg_cdr = 0x0001        /* record is CDR */
};

/*
    ACE subrecord found in FDR
*/
struct ti990_ace
{
	char        agn[8];             /* access group name */
	UINT16BE    flg;                /* flags */
};

/*
    FDR record
*/
struct ti990_fdr
{
	UINT16BE    hkc;            /* hask key count: the number of file descriptor records that are present in the directory that hashed to this record number */
	UINT16BE    hkv;            /* hask key value: the result of the hash algorithm for the file name actually covered in this record */
	char        fnm[8];         /* file name */
	uint8_t     rsv[2];         /* reserved */
	UINT16BE    fl1;            /* flags word 1 */
	UINT16BE    flg;            /* flags word 2 */
	UINT16BE    prs;            /* physical record size */
	UINT16BE    lrs;            /* logical record size */
	UINT16BE    pas;            /* primary allocation size: # of ADUs allocated in primary allocation block? */
	UINT16BE    paa;            /* primary allocation address: first ADU of primary allocation block? */
	UINT16BE    sas;            /* secondary allocation size: used to determinate the # of blocks allocated per secondary allocation */
	UINT16BE    saa;            /* offset of secondary table: ???? "offset into this FDR of the secondary allocation table, if any.  No secondary allocation table is denoted by 0.  Secondary allocations are present only for unbounded files." */
	UINT16BE    rfa;            /* record number of first alias */
	UINT32BE    eom;            /* end of medium record number */
	UINT32BE    bkm;            /* end of medium block number */
	UINT16BE    ofm;            /* end of medium offset / */
								/* prelog number for KIF */
	UINT32BE    fbq;            /* free block queue head */
	UINT16BE    btr;            /* B-tree roots block # */
	UINT32BE    ebq;            /* empty block queue head */
	UINT16BE    kdr;            /* key descriptions record # */
	uint8_t     ud[6];          /* last update date */
	uint8_t     cd[6];          /* creation date */
	uint8_t     apb;            /* ADU's per block */
	uint8_t     bpa;            /* blocks per ADU */
	UINT16BE    mrs;            /* minimum KIF record size */
	uint8_t     sat[64];        /* secondary allocation table: 16 2-word entries.  The first word of an entry contains the size, in ADUs, of the secondary allocation.  The second word contains the starting ADU of the allocation. */

/* bytes >86 to >100 are optional */
	uint8_t     res[10];        /* reserved: seem to be actually meaningful (at least under DX10 3.6.x) */
	char        uid[8];         /* user id of file creator */
	UINT16BE    psa;            /* public security attribute */
	ti990_ace   ace[9];         /* 9 access control entries */
	uint8_t     fil[2];         /* not used */
};

/*
    ADR record: variant of FDR for Aliases

    The fields marked here with *** are in the ADR template to maintain
    compatibility with the FDR template.
*/
struct ti990_adr
{
	UINT16BE    hkc;            /* hask key count */
	UINT16BE    hkv;            /* hask key value */
	char        fnm[8];         /* file name */
	char        psw[4];         /* "password" (whatever it means) */
	UINT16BE    flg;            /* flags (same as fdr.flg) */
	UINT16BE    fill00;         /* *** physical record size */
	UINT16BE    fill01;         /* *** logical record size */
	UINT16BE    fill02;         /* *** primary allocation size */
	UINT16BE    fill03;         /* *** primary allocation address */
	UINT16BE    fill04;         /* *** secondary allocation size */
	UINT16BE    fill05;         /* *** secondary allocation address */
	UINT16BE    rna;            /* record number of next ADR */
	UINT16BE    raf;            /* record # of actual FDR (from 1 through dor.nrc) */
};

/*
    CDR record: variant of FDR for Channel

    The CDR is the permanent record of a channel.  It is carried as an alias
    of the program file in which the channel owner task resides.
*/
struct ti990_cdr
{
	UINT16BE    hkc;            /* hask key count */
	UINT16BE    hkv;            /* hask key value */
	char        fnm[8];         /* file name */
	UINT16BE    fill00;         /* reserved */
	UINT16BE    fill01;         /* reserved */
	UINT16BE    fdf;            /* flags (same as fdr.flg) */
	uint8_t     flg;            /* channel flzgs */
	uint8_t     iid;            /* owner task installed ID */
	uint8_t     typ;            /* default resource type */
	uint8_t     tf;             /* resource type flags */
	UINT16BE    mxl;            /* maximum message length */
	uint8_t     fill04[6];      /* reserved (and, no, I don't know where fill02 and fill03 have gone) */
	UINT16BE    rna;            /* record number of next CDR or ADR */
	UINT16BE    raf;            /* record # of actual FDR */
	uint8_t     fill05[110];    /* reserved */
	char        uid[8];         /* user ID of channel creator */
	UINT16BE    psa;            /* public security attribute */
	uint8_t     scg[94];        /* "SDT with 9 control groups" (whatever it means - and, no, 94 is not dividable by 9) */
	uint8_t     fill06[8];      /* reserved */
};

/*
    Based on contents of the flags field, catalog entries may be either an FDR,
    an ADR or a CDR.

    They may be a KDR, too, but confusion is impossible because the KDR FILL00
    field starts at offset 4, and therefore if we try to interpret a KDR as an
    FDR (or ADR, CDR), we will find fnm[0] and assume the FDR is empty.
*/
union directory_entry
{
	ti990_fdr fdr;
	ti990_adr adr;
	ti990_cdr cdr;
};

#if 0

/*
    tifile header: stand-alone file
*/
struct tifile_header
{
	char        tifiles[8];     /* always '\7TIFILES' */
	uint8_t     secsused_MSB;   /* file length in sectors (big-endian) */
	uint8_t     secsused_LSB;
	uint8_t     flags;          /* see enum above */
	uint8_t     recspersec;     /* records per sector */
	uint8_t     eof;            /* current position of eof in last sector (0->255)*/
	uint8_t     reclen;         /* bytes per record ([1,255] 0->256) */
	uint8_t     fixrecs_MSB;    /* file length in records (big-endian) */
	uint8_t     fixrecs_LSB;
	uint8_t     res[128-16];    /* reserved */
};

/*
    catalog entry (used for in-memory catalog)
*/
struct catalog_entry
{
	uint16_t fdr_secnum;
	char filename[10];
};

#endif

/*
    Disk geometry
*/
struct ti990_geometry
{
	unsigned int cylinders, heads, sectors_per_track, bytes_per_sector;
};

/*
    Physical sector address
*/
struct ti990_phys_sec_address
{
	int cylinder;
	int head;
	int sector;
};

/*
    ti99 disk image descriptor
*/
struct ti990_image
{
	imgtool::stream *file_handle;        /* imgtool file handle */
	ti990_geometry geometry;    /* geometry */
	ti990_sc0 sec0;             /* cached copy of sector 0 */
};

/*
    ti990 catalog iterator, used when imgtool reads the catalog
*/
struct ti990_iterator
{
	ti990_image *image;
	int level;                          /* current recursion level */
	int nrc[MAX_DIR_LEVEL];             /* length of disk catalogs in records */
	int index[MAX_DIR_LEVEL];           /* current index in the disk catalog */
	directory_entry xdr[MAX_DIR_LEVEL]; /* fdr records */
};

static ti990_image *get_ti990_image(imgtool::image &image)
{
	return (ti990_image *)image.extra_bytes();
}


static imgtoolerr_t ti990_image_init(imgtool::image &img, imgtool::stream::ptr &&stream);
static void ti990_image_exit(imgtool::image &img);
static void ti990_image_info(imgtool::image &img, std::ostream &stream);
static imgtoolerr_t ti990_image_beginenum(imgtool::directory &enumeration, const char *path);
static imgtoolerr_t ti990_image_nextenum(imgtool::directory &enumeration, imgtool_dirent &ent);
static void ti990_image_closeenum(imgtool::directory &enumeration);
static imgtoolerr_t ti990_image_freespace(imgtool::partition &partition, uint64_t *size);
[[maybe_unused]] static imgtoolerr_t ti990_image_readfile(imgtool::partition &partition, const char *fpath, imgtool::stream *destf);
[[maybe_unused]] static imgtoolerr_t ti990_image_writefile(imgtool::partition &partition, const char *fpath, imgtool::stream *sourcef, util::option_resolution *writeoptions);
[[maybe_unused]] static imgtoolerr_t ti990_image_deletefile(imgtool::partition &partition, const char *fpath);
static imgtoolerr_t ti990_image_create(imgtool::image &image, imgtool::stream::ptr &&stream, util::option_resolution *createoptions);

enum
{
	/*ti990_createopts_volname = 'A',*/
	ti990_createopts_cylinders = 'B',
	ti990_createopts_heads = 'C',
	ti990_createopts_sectors = 'D',
	ti990_createopts_sectorsize = 'E'
};

OPTION_GUIDE_START( ti990_create_optionguide )
	/*OPTION_STRING(ti990_createopts_volname, "label",  "Volume name" )*/
	OPTION_INT(ti990_createopts_cylinders, "cylinders", "Cylinders" )
	OPTION_INT(ti990_createopts_heads, "heads", "Heads" )
	OPTION_INT(ti990_createopts_sectors, "sectors", "Sectors" )
	OPTION_INT(ti990_createopts_sectorsize, "bytes per sector (typically 256)", "Bytes Per Sector" )
OPTION_GUIDE_END

#define symb2str2(a) #a
#define symb2str(a) symb2str2(a)
#define ti990_create_optionspecs "B1-[145]-" symb2str(MAX_CYLINDERS)";C1-[4]-" symb2str(MAX_HEADS)";D1-[32]-" symb2str(MAX_SECTORS_PER_TRACK)";E" symb2str(MIN_SECTOR_SIZE)"-[256]-" symb2str(MAX_SECTOR_SIZE)";"

void ti990_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch(state)
	{
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "ti990hd"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "TI990 Hard Disk"); break;
		case IMGTOOLINFO_STR_FILE_EXTENSIONS:               strcpy(info->s = imgtool_temp_str(), "hd"); break;
		case IMGTOOLINFO_STR_EOLN:                          /* strcpy(info->s = imgtool_temp_str(), "\r"); */ break;

		case IMGTOOLINFO_PTR_OPEN:                          info->open = ti990_image_init; break;
		case IMGTOOLINFO_PTR_CLOSE:                         info->close = ti990_image_exit; break;
		case IMGTOOLINFO_PTR_INFO:                          info->info = ti990_image_info; break;
		case IMGTOOLINFO_PTR_BEGIN_ENUM:                    info->begin_enum = ti990_image_beginenum; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                     info->next_enum = ti990_image_nextenum; break;
		case IMGTOOLINFO_PTR_CLOSE_ENUM:                    info->close_enum = ti990_image_closeenum; break;
		case IMGTOOLINFO_PTR_FREE_SPACE:                    info->free_space = ti990_image_freespace; break;
		case IMGTOOLINFO_PTR_READ_FILE:                     /* info->read_file = ti990_image_readfile; */ break;
		case IMGTOOLINFO_PTR_WRITE_FILE:                    /* info->write_file = ti990_image_writefile; */ break;
		case IMGTOOLINFO_PTR_DELETE_FILE:                   /* info->delete_file = ti990_image_deletefile; */ break;
		case IMGTOOLINFO_PTR_CREATE:                        info->create = ti990_image_create; break;

		case IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE:          info->createimage_optguide = &ti990_create_optionguide; break;
		case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC:           strcpy(info->s = imgtool_temp_str(), ti990_create_optionspecs); break;
	}
}

/*
    Convert a C string to a 8-character file name (padded with spaces if necessary)
*/
[[maybe_unused]] static void str_to_fname(char dst[8], const char *src)
{
	int i;


	i = 0;

	/* copy 8 characters at most */
	if (src)
		while ((i<8) && (src[i]!='\0'))
		{
			dst[i] = src[i];
			i++;
		}

	/* pad with spaces */
	while (i<8)
	{
		dst[i] = ' ';
		i++;
	}
}

/*
    Convert a 8-character file name to a C string (removing trailing spaces if necessary)
*/
static void fname_to_str(char *dst, const char src[8], int n)
{
	int i;
	int last_nonspace;


	/* copy 8 characters at most */
	if (--n > 8)
		n = 8;

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

#if 0
#pragma mark -
#pragma mark DISK ROUTINES
#endif

/*
    Convert physical sector address to offset
*/
static unsigned phys_address_to_offset(const ti990_phys_sec_address *address, const ti990_geometry *geometry)
{
	unsigned offset;

	/* current MESS format */
	offset = (((address->cylinder*geometry->heads) + address->head)*geometry->sectors_per_track + address->sector)*geometry->bytes_per_sector + header_len;

	return offset;
}

/*
    Read one sector from a disk image

    file_handle: imgtool file handle
    address: physical sector address
    geometry: disk geometry (sectors per track, tracks per side, sides)
    dest: pointer to destination buffer
    len: length of data to read
*/
static int read_sector_physical_len(imgtool::stream &file_handle, const ti990_phys_sec_address *address, const ti990_geometry *geometry, void *dest, int len)
{
	int reply;

	if (len > geometry->bytes_per_sector)
		return 1;

	/* seek to sector */
	reply = file_handle.seek(phys_address_to_offset(address, geometry), SEEK_SET);
	if (reply)
		return 1;
	/* read it */
	reply = file_handle.read(dest, len);
	if (reply != len)
		return 1;

	return 0;
}

/*
    Read one sector from a disk image

    file_handle: imgtool file handle
    address: physical sector address
    geometry: disk geometry (sectors per track, tracks per side, sides)
    dest: pointer to a destination buffer of geometry->bytes_per_sector bytes
*/
[[maybe_unused]] static int read_sector_physical(imgtool::stream &file_handle, const ti990_phys_sec_address *address, const ti990_geometry *geometry, void *dest)
{
	return read_sector_physical_len(file_handle, address, geometry, dest, geometry->bytes_per_sector);
}

/*
    Write one sector to a disk image

    file_handle: imgtool file handle
    address: physical sector address
    geometry: disk geometry (sectors per track, tracks per side, sides)
    src: pointer to source buffer
    len: length of source buffer
*/
static int write_sector_physical_len(imgtool::stream &file_handle, const ti990_phys_sec_address *address, const ti990_geometry *geometry, const void *src, int len)
{
	int reply;

	if (len > geometry->bytes_per_sector)
		return 1;

	/* seek to sector */
	reply = file_handle.seek(phys_address_to_offset(address, geometry), SEEK_SET);
	if (reply)
		return 1;
	/* write it */
	reply = file_handle.write(src, len);
	if (reply != len)
		return 1;
	/* pad with 0s if needed */
	if (len < geometry->bytes_per_sector)
	{
		reply = file_handle.fill(0, geometry->bytes_per_sector - len);

		if (reply != geometry->bytes_per_sector - len)
			return 1;
	}

	return 0;
}

/*
    Write one sector to a disk image

    file_handle: imgtool file handle
    address: physical sector address
    geometry: disk geometry (sectors per track, tracks per side, sides)
    dest: pointer to a source buffer of geometry->bytes_per_sector bytes
*/
[[maybe_unused]] static int write_sector_physical(imgtool::stream &file_handle, const ti990_phys_sec_address *address, const ti990_geometry *geometry, const void *src)
{
	return write_sector_physical_len(file_handle, address, geometry, src, geometry->bytes_per_sector);
}

/*
    Convert logical sector address to physical sector address
*/
static void log_address_to_phys_address(int secnum, const ti990_geometry *geometry, ti990_phys_sec_address *address)
{
	address->sector = secnum % geometry->sectors_per_track;
	secnum /= geometry->sectors_per_track;
	address->head = secnum % geometry->heads;
	address->cylinder = secnum / geometry->heads;
}

/*
    Read one sector from a disk image

    file_handle: imgtool file handle
    secnum: logical sector address
    geometry: disk geometry (sectors per track, tracks per side, sides)
    dest: pointer to destination buffer
    len: length of data to read
*/
static int read_sector_logical_len(imgtool::stream &file_handle, int secnum, const ti990_geometry *geometry, void *dest, int len)
{
	ti990_phys_sec_address address;


	log_address_to_phys_address(secnum, geometry, &address);

	return read_sector_physical_len(file_handle, &address, geometry, dest, len);
}

/*
    Read one sector from a disk image

    file_handle: imgtool file handle
    secnum: logical sector address
    geometry: disk geometry (sectors per track, tracks per side, sides)
    dest: pointer to a destination buffer of geometry->bytes_per_sector bytes
*/
[[maybe_unused]] static int read_sector_logical(imgtool::stream &file_handle, int secnum, const ti990_geometry *geometry, void *dest)
{
	return read_sector_logical_len(file_handle, secnum, geometry, dest, geometry->bytes_per_sector);
}

/*
    Write one sector to a disk image

    file_handle: imgtool file handle
    secnum: logical sector address
    geometry: disk geometry (sectors per track, tracks per side, sides)
    src: pointer to source buffer
    len: length of source buffer
*/
static int write_sector_logical_len(imgtool::stream &file_handle, int secnum, const ti990_geometry *geometry, const void *src, int len)
{
	ti990_phys_sec_address address;


	log_address_to_phys_address(secnum, geometry, &address);

	return write_sector_physical_len(file_handle, &address, geometry, src, len);
}

/*
    Write one sector to a disk image

    file_handle: imgtool file handle
    secnum: logical sector address
    geometry: disk geometry (sectors per track, tracks per side, sides)
    dest: pointer to a source buffer of geometry->bytes_per_sector bytes
*/
static int write_sector_logical(imgtool::stream &file_handle, int secnum, const ti990_geometry *geometry, const void *src)
{
	return write_sector_logical_len(file_handle, secnum, geometry, src, geometry->bytes_per_sector);
}

#if 0
#pragma mark -
#pragma mark CATALOG FILE ROUTINES
#endif

/*
    Find the catalog entry and fdr record associated with a file name

    image: ti990_image image record
    fpath: path of the file to search
    fdr: pointer to buffer where the fdr record should be stored (may be NULL)
    catalog_index: on output, index of file catalog entry (may be NULL)
    out_fdr_secnum: on output, sector address of the fdr (may be NULL)
    out_parent_fdr_secnum: on output, sector offset of the fdr for the parent
        directory fdr (-1 if root) (may be NULL)
*/
static imgtoolerr_t find_fdr(ti990_image *image, const char fpath[MAX_PATH_LEN+1], int *catalog_index, int *out_fdr_secnum, int *out_parent_fdr_secnum)
{
	int fdr_secnum = -1, parent_fdr_secnum;
	int i;
	const char *element_start, *element_end;
	int element_len;
	char element[8];
	ti990_dor dor;
	directory_entry xdr;
	int hash_key;
	int nrc;
	unsigned base;
	int reply;
	int flag;


	base = (unsigned) get_UINT16BE(image->sec0.vda) * get_UINT16BE(image->sec0.spa);

	element_start = fpath;
	do
	{
		/* read directory header */
		reply = read_sector_logical_len(*image->file_handle, base, & image->geometry, &dor, sizeof(dor));
		if (reply)
			return IMGTOOLERR_READERROR;

		nrc = get_UINT16BE(dor.nrc);

		/* find next path element */
		element_end = strchr(element_start, '.');
		element_len = element_end ? (element_end - element_start) : strlen(element_start);
		if ((element_len > 8) || (element_len == 0))
			return IMGTOOLERR_BADFILENAME;
		/* generate file name and hash key */
		hash_key = 1;
		for (i=0; i<element_len; i++)
		{
			element[i] = element_start[i];
			hash_key = ((hash_key * (unsigned char) element_start[i]) % nrc) + 1;
		}
		while (i<8)
		{
			element[i] = ' ';
			i++;
		}

		/* search entry in directory */
		i = hash_key;
		while (1)
		{
			reply = read_sector_logical_len(*image->file_handle, base + i, & image->geometry,
												& xdr, 0x86);
			if (reply)
				return IMGTOOLERR_READERROR;
			if (!memcmp(element, xdr.fdr.fnm, 8))
			{   /* found match !!! */
				parent_fdr_secnum = fdr_secnum;
				fdr_secnum = base + i;
				break;
			}

			/* next record */
			if (i != nrc)
				i++;
			else
				i = 1;

			/* exit if we have tested every record */
			if (i == hash_key)
				return IMGTOOLERR_FILENOTFOUND;
		}

		/* iterate */
		if (element_end)
		{
			element_start = element_end+1;
			/* we need to check that this is a directory */
			/* if it is an alias to a directory, we need to follow it, too. */

			/* parse flags */
			flag = get_UINT16BE(xdr.fdr.flg);

			if (flag & fdr_flg_ali)
			{
				/* read original fdr */
				reply = read_sector_logical_len(*image->file_handle, base + get_UINT16BE(xdr.adr.raf),
													& image->geometry, & xdr, 0x86);

				flag = get_UINT16BE(xdr.fdr.flg);
			}

			if (flag & fdr_flg_cdr)
				/* This is a channel, not a file */
				return IMGTOOLERR_BADFILENAME;
			else if (flag & fdr_flg_ali)
				/* WTH??? */
				return IMGTOOLERR_CORRUPTIMAGE;
			else if (((flag >> fdr_flg_fu_shift) & 3) != 1)
			{
				/* This file is not a directory */
				return IMGTOOLERR_BADFILENAME;
			}

			/* initialize base */
			base = (unsigned) get_UINT16BE(xdr.fdr.paa) * get_UINT16BE(image->sec0.spa);
		}
		else
			element_start = NULL;
	}
	while (element_start);

	if (catalog_index)
		*catalog_index = i;

	if (out_fdr_secnum)
		*out_fdr_secnum = fdr_secnum;

	if (out_parent_fdr_secnum)
		*out_parent_fdr_secnum = parent_fdr_secnum;

	return IMGTOOLERR_SUCCESS;
}

#if 0
/*
    Allocate one sector on disk, for use as a fdr record

    image: ti99_image image record
    fdr_secnum: on output, logical address of a free sector
*/
static int alloc_fdr_sector(ti99_image *image, int *fdr_secnum)
{
	int totsecs = (image->sec0.totsecsMSB << 8) | image->sec0.totsecsLSB;
	int i;


	for (i=0; i<totsecs; i++)
	{
		if (! (image->sec0.abm[i >> 3] & (1 << (i & 7))))
		{
			*fdr_secnum = i;
			image->sec0.abm[i >> 3] |= 1 << (i & 7);

			return 0;
		}
	}

	return IMGTOOLERR_NOSPACE;
}

/*
    Extend a file with nb_alloc_sectors extra sectors

    image: ti99_image image record
    fdr: file fdr record
    nb_alloc_sectors: number of sectors to allocate
*/
static int alloc_file_sectors(ti99_image *image, ti99_fdr *fdr, int nb_alloc_sectors)
{
	int totsecs = (image->sec0.totsecsMSB << 8) | image->sec0.totsecsLSB;
	int free_sectors;
	int secsused;
	int i;
	int lnks_index;
	int cur_sec, last_sec, p_last_sec;
	int cur_block_len, cur_block_start;
	int first_best_block_len, first_best_block_start = 0;
	int second_best_block_len, second_best_block_start = 0;
	int search_start;

	/* compute free space */
	free_sectors = 0;
	for (i=0; i<totsecs; i++)
	{
		if (! (image->sec0.abm[i >> 3] & (1 << (i & 7))))
			free_sectors++;
	}

	/* check we have enough free space */
	if (free_sectors < nb_alloc_sectors)
		return IMGTOOLERR_NOSPACE;

	/* current number of data sectors in file */
	secsused = get_fdr_secsused(fdr);

	if (secsused == 0)
	{   /* links array must be empty */
		lnks_index = 0;
	}
	else
	{   /* try to extend last block */
		last_sec = -1;
		for (lnks_index=0; lnks_index<76; lnks_index++)
		{
			p_last_sec = last_sec;
			last_sec = (fdr->lnks[lnks_index][2] << 4) | (fdr->lnks[lnks_index][1] >> 4);
			if (last_sec >= (secsused-1))
				break;
		}
		if (lnks_index == 76)
			/* that sucks */
			return IMGTOOLERR_CORRUPTIMAGE;

		if (last_sec > (secsused-1))
		{   /* some extra space has already been allocated */
			cur_block_len = last_sec - (secsused-1);
			if (cur_block_len > nb_alloc_sectors)
				cur_block_len = nb_alloc_sectors;

			secsused += cur_block_len;
			set_fdr_secsused(fdr, secsused);
			nb_alloc_sectors -= cur_block_len;
			if (! nb_alloc_sectors)
				return 0;   /* done */
		}

		/* block base */
		cur_sec = ((fdr->lnks[lnks_index][1] & 0xf) << 8) | fdr->lnks[lnks_index][0];
		/* point past block end */
		cur_sec += last_sec-p_last_sec;
		/* count free sectors after last block */
		cur_block_len = 0;
		for (i=cur_sec; (! (image->sec0.abm[i >> 3] & (1 << (i & 7)))) && (cur_block_len < nb_alloc_sectors) && (i < totsecs); i++)
			cur_block_len++;
		if (cur_block_len)
		{   /* extend last block */
			secsused += cur_block_len;
			set_fdr_secsused(fdr, secsused);
			last_sec += cur_block_len;
			nb_alloc_sectors -= cur_block_len;
			fdr->lnks[lnks_index][1] = (fdr->lnks[lnks_index][1] & 0x0f) | (last_sec << 4);
			fdr->lnks[lnks_index][2] = last_sec >> 4;
			for (i=0; i<cur_block_len; i++)
				image->sec0.abm[(i+cur_sec) >> 3] |= 1 << ((i+cur_sec) & 7);
			if (! nb_alloc_sectors)
				return 0;   /* done */
		}
		lnks_index++;
		if (lnks_index == 76)
			/* that sucks */
			return IMGTOOLERR_NOSPACE;
	}

	search_start = image->data_offset;  /* initially, search for free space only in data space */
	while (nb_alloc_sectors)
	{
		/* find smallest data block at least nb_alloc_sectors in length, and largest data block less than nb_alloc_sectors in length */
		first_best_block_len = INT_MAX;
		second_best_block_len = 0;
		for (i=search_start; i<totsecs; i++)
		{
			if (! (image->sec0.abm[i >> 3] & (1 << (i & 7))))
			{   /* found one free block */
				/* compute its length */
				cur_block_start = i;
				cur_block_len = 0;
				while ((i<totsecs) && (! (image->sec0.abm[i >> 3] & (1 << (i & 7)))))
				{
					cur_block_len++;
					i++;
				}
				/* compare to previous best and second-best blocks */
				if ((cur_block_len < first_best_block_len) && (cur_block_len >= nb_alloc_sectors))
				{
					first_best_block_start = cur_block_start;
					first_best_block_len = cur_block_len;
					if (cur_block_len == nb_alloc_sectors)
						/* no need to search further */
						break;
				}
				else if ((cur_block_len > second_best_block_len) && (cur_block_len < nb_alloc_sectors))
				{
					second_best_block_start = cur_block_start;
					second_best_block_len = cur_block_len;
				}
			}
		}

		if (first_best_block_len != INT_MAX)
		{   /* found one contiguous block which can hold it all */
			secsused += nb_alloc_sectors;
			set_fdr_secsused(fdr, secsused);

			fdr->lnks[lnks_index][0] = first_best_block_start;
			fdr->lnks[lnks_index][1] = ((first_best_block_start >> 8) & 0x0f) | ((secsused-1) << 4);
			fdr->lnks[lnks_index][2] = (secsused-1) >> 4;
			for (i=0; i<nb_alloc_sectors; i++)
				image->sec0.abm[(i+first_best_block_start) >> 3] |= 1 << ((i+first_best_block_start) & 7);

			nb_alloc_sectors = 0;
		}
		else if (second_best_block_len != 0)
		{   /* jeez, we need to fragment it.  We use the largest smaller block to limit fragmentation. */
			secsused += second_best_block_len;
			set_fdr_secsused(fdr, secsused);

			fdr->lnks[lnks_index][0] = second_best_block_start;
			fdr->lnks[lnks_index][1] = ((second_best_block_start >> 8) & 0x0f) | ((secsused-1) << 4);
			fdr->lnks[lnks_index][2] = (secsused-1) >> 4;
			for (i=0; i<second_best_block_len; i++)
				image->sec0.abm[(i+second_best_block_start) >> 3] |= 1 << ((i+second_best_block_start) & 7);

			nb_alloc_sectors -= second_best_block_len;

			lnks_index++;
			if (lnks_index == 76)
				/* that sucks */
				return IMGTOOLERR_NOSPACE;
		}
		else if (search_start != 0)
		{   /* we did not find any free sector in the data section of the disk */
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
static int new_file(ti99_image *image, char filename[10], int *out_fdr_secnum/*, ti99_fdr *fdr,*/)
{
	int fdr_secnum;
	int catalog_index, i;
	int reply = 0;


	/* find insertion point in catalog */
	i = 0;
	if ((image->catalog[0].fdr_secnum == 0) && (image->catalog[1].fdr_secnum != 0))
		i = 1;  /* skip empty entry 0 (it must be a non-listable catalog) */

	for (; (i<128) && ((fdr_secnum = image->catalog[i].fdr_secnum) != 0) && ((reply = memcmp(image->catalog[i].filename, filename, 10)) < 0); i++)
		;

	if (i == 128)
		/* catalog is full */
		return IMGTOOLERR_NOSPACE;
	else if (fdr_secnum && (reply == 0))
		/* file already exists */
		return IMGTOOLERR_BADFILENAME;
	else
	{
		catalog_index = i;
		reply = alloc_fdr_sector(image, &fdr_secnum);
		if (reply)
			return reply;

		/* look for first free entry in catalog */
		for (i=catalog_index; image->catalog[i].fdr_secnum != 0; i++)
			;

		if (i == 128)
			/* catalog is full */
				return IMGTOOLERR_NOSPACE;

		/* shift catalog entries until the insertion point */
		for (/*i=127*/; i>catalog_index; i--)
			image->catalog[i] = image->catalog[i-1];

		/* write new catalog entry */
		image->catalog[catalog_index].fdr_secnum = fdr_secnum;
		memcpy(image->catalog[catalog_index].filename, filename, 10);
		if (out_fdr_secnum)
			*out_fdr_secnum = fdr_secnum;
	}

	return 0;
}

/*
    Compare two (possibly empty) catalog entry for qsort
*/
static int qsort_catalog_compare(const void *p1, const void *p2)
{
	const catalog_entry *entry1 = p1;
	const catalog_entry *entry2 = p2;

	if ((entry1->fdr_secnum == 0) && (entry2->fdr_secnum == 0))
		return 0;
	else if (entry1->fdr_secnum == 0)
		return +1;
	else if (entry2->fdr_secnum == 0)
		return -1;
	else
		return memcmp(entry1->filename, entry2->filename, 10);
}
#endif

/*
    Open a file as a ti990_image.
*/
static imgtoolerr_t ti990_image_init(imgtool::image &img, imgtool::stream::ptr &&stream)
{
	ti990_image *image = get_ti990_image(img);
	disk_image_header header;
	int reply;
	unsigned totsecs;

	image->file_handle = stream.get();
	reply = image->file_handle->read(&header, sizeof(header));
	if (reply != sizeof(header))
		return IMGTOOLERR_READERROR;

	image->geometry.cylinders = get_UINT32BE(header.cylinders);
	image->geometry.heads = get_UINT32BE(header.heads);
	image->geometry.sectors_per_track = get_UINT32BE(header.sectors_per_track);
	image->geometry.bytes_per_sector = get_UINT32BE(header.bytes_per_sector);

	totsecs = image->geometry.cylinders*image->geometry.heads*image->geometry.sectors_per_track;

	if ((image->geometry.bytes_per_sector < MIN_SECTOR_SIZE)
		|| (image->geometry.bytes_per_sector > MAX_SECTOR_SIZE)
		|| (image->geometry.sectors_per_track > MAX_SECTORS_PER_TRACK)
		|| (image->geometry.heads > MAX_HEADS)
		|| (image->geometry.cylinders > MAX_CYLINDERS)
		|| (totsecs < 1)
		|| (image->file_handle->size() != header_len + totsecs*image->geometry.bytes_per_sector))
	{
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	{
		ti990_phys_sec_address address = { 0, 0, 0 };
		reply = read_sector_physical_len(*image->file_handle, &address, &image->geometry, & image->sec0, sizeof(image->sec0));
	}
	if (reply)
	{
		return IMGTOOLERR_READERROR;
	}

	if ((get_UINT16BE(image->sec0.tna)*get_UINT16BE(image->sec0.spa) > totsecs)
		|| ((get_UINT16BE(image->sec0.spa) != 1) && (get_UINT16BE(image->sec0.spa) % 3)))
	{
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	if ((get_UINT16BE(image->sec0.vpl) != 0x86) && (get_UINT16BE(image->sec0.vpl) != 0x100))
	{
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	image->file_handle = stream.release();
	return IMGTOOLERR_SUCCESS;
}

/*
    close a ti990_image
*/
static void ti990_image_exit(imgtool::image &img)
{
	ti990_image *image = get_ti990_image(img);
	delete image->file_handle;
}

/*
    get basic information on a ti990_image

    Currently returns the volume name
*/
static void ti990_image_info(imgtool::image &img, std::ostream &stream)
{
	ti990_image *image = get_ti990_image(img);
	char vol_name[9];

	fname_to_str(vol_name, image->sec0.vnm, 9);

	stream << vol_name;
}

/*
    Open the disk catalog for enumeration
*/
static imgtoolerr_t ti990_image_beginenum(imgtool::directory &enumeration, const char *path)
{
	ti990_image *image = (ti990_image *) enumeration.image().extra_bytes();
	ti990_iterator *iter = (ti990_iterator *) enumeration.extra_bytes();
	ti990_dor dor;
	int reply;

	iter->image = image;

	reply = read_sector_logical_len(*iter->image->file_handle,
									(unsigned) get_UINT16BE(iter->image->sec0.vda) * get_UINT16BE(iter->image->sec0.spa),
									& iter->image->geometry, &dor, sizeof(dor));

	if (reply)
		return IMGTOOLERR_READERROR;

	iter->level = 0;
	iter->nrc[0] = get_UINT16BE(dor.nrc);
	iter->index[0] = 0;

	return IMGTOOLERR_SUCCESS;
}

/*
    Enumerate disk catalog next entry
*/
static imgtoolerr_t ti990_image_nextenum(imgtool::directory &enumeration, imgtool_dirent &ent)
{
	ti990_iterator *iter = (ti990_iterator *) enumeration.extra_bytes();
	int flag;
	int reply = 0;


	ent.corrupt = 0;
	ent.eof = 0;

	while ((iter->level >= 0)
			&& (! (reply = read_sector_logical_len(*iter->image->file_handle,
													iter->level ? (unsigned) get_UINT16BE(iter->xdr[iter->level-1].fdr.paa) * get_UINT16BE(iter->image->sec0.spa) + (iter->index[iter->level]+1)
																: (unsigned) get_UINT16BE(iter->image->sec0.vda) * get_UINT16BE(iter->image->sec0.spa) + (iter->index[iter->level]+1),
													& iter->image->geometry, &iter->xdr[iter->level],
													iter->level ? get_UINT16BE(iter->xdr[iter->level-1].fdr.prs)
																: get_UINT16BE(iter->image->sec0.vpl))))
			&& (iter->xdr[iter->level].fdr.fnm[0] == 0))
	{
		iter->index[iter->level]++;
		while (iter->index[iter->level] >= iter->nrc[iter->level])
			iter->level--;
	}

	if (iter->level < 0)
	{
		ent.eof = 1;
	}
	else if (reply)
		return IMGTOOLERR_READERROR;
	else
	{
#if 0
		fname_to_str(ent.filename, iter->xdr[iter->level].fdr.fnm, std::size(ent.filename));
#else
		{
			int i;
			char buf[9];

			ent.filename[0] = '\0';
			for (i=0; i<iter->level; i++)
			{
				fname_to_str(buf, iter->xdr[i].fdr.fnm, 9);
				strncat(ent.filename, buf, std::size(ent.filename) - 1);
				strncat(ent.filename, ".", std::size(ent.filename) - 1);
			}
			fname_to_str(buf, iter->xdr[iter->level].fdr.fnm, 9);
			strncat(ent.filename, buf, std::size(ent.filename) - 1);
		}
#endif

		/* parse flags */
		flag = get_UINT16BE(iter->xdr[iter->level].fdr.flg);
		if (flag & fdr_flg_cdr)
		{
			snprintf(ent.attr, std::size(ent.attr), "CHANNEL");

			ent.filesize = 0;
		}
		else if (flag & fdr_flg_ali)
		{
			ti990_fdr target_fdr;
			char buf[9];

			reply = read_sector_logical_len(*iter->image->file_handle,
												iter->level ? ((unsigned) get_UINT16BE(iter->xdr[iter->level-1].fdr.paa) * get_UINT16BE(iter->image->sec0.spa) + get_UINT16BE(iter->xdr[iter->level].adr.raf))
															: ((unsigned) get_UINT16BE(iter->image->sec0.vda) * get_UINT16BE(iter->image->sec0.spa) + get_UINT16BE(iter->xdr[iter->level].adr.raf)),
												& iter->image->geometry, &target_fdr,
												iter->level ? get_UINT16BE(iter->xdr[iter->level-1].fdr.prs)
															: get_UINT16BE(iter->image->sec0.vpl));

			fname_to_str(buf, target_fdr.fnm, 9);

			snprintf(ent.attr, std::size(ent.attr), "ALIAS OF %s", buf);

			ent.filesize = 0;
		}
		else
		{
			const char *fmt, *type;

			switch ((flag >> fdr_flg_fmt_shift) & 3)
			{
			case 0:
				fmt = "NBS";
				break;
			case 1:
				fmt = "BS ";
				break;
			default:
				fmt = "???";
				break;
			}

			switch ((flag >> fdr_flg_fu_shift) & 3)
			{
			case 1:
				type = "DIR";
				break;
			case 2:
				type = "PRO";
				break;
			case 3:
				type = "IMG";
				break;
			default:
				switch ((flag >> fdr_flg_ft_shift) & 3)
				{
				case 1:
					type = "SEQ";
					break;
				case 2:
					type = "REL";
					break;
				case 3:
					type = "KIF";
					break;
				default:
					type = "???";
					break;
				}
				break;
			}
			snprintf(ent.attr, std::size(ent.attr),
						"%s %c %s%s%s%s%s", fmt, (flag & fdr_flg_all) ? 'N' : 'C', type,
							(flag & fdr_flg_blb) ? "" : " BLK",
							(flag & fdr_flg_tmp) ? " TMP" : "",
							(flag & fdr_flg_wpb) ? " WPT" : "",
							(flag & fdr_flg_dpb) ? " DPT" : "");

			/* len in blocks */
			ent.filesize = get_UINT32BE(iter->xdr[iter->level].fdr.bkm);
			if ((((flag >> fdr_flg_ft_shift) & 3) == 3) || get_UINT16BE(iter->xdr[iter->level].fdr.ofm))
				ent.filesize++;

			/* convert to ADUs */
			if (iter->xdr[iter->level].fdr.apb > 1)
				/* more than one ADU per block */
				ent.filesize *= iter->xdr[iter->level].fdr.apb;
			else if (iter->xdr[iter->level].fdr.bpa > 1)
				/* more than one block per ADU */
				ent.filesize = (ent.filesize + iter->xdr[iter->level].fdr.bpa - 1) / iter->xdr[iter->level].fdr.bpa;
		}
		iter->index[iter->level]++;

		if (get_UINT16BE(iter->xdr[iter->level].fdr.saa) != 0)
			printf("roupoupou");
		else
		{
			int i;
			for (i=0; i<64; i++)
				if (iter->xdr[iter->level].fdr.sat[i])
				{
					printf("roupoupou");
					break;
				}
		}

		/* recurse subdirectory if applicable */
		if ((((flag >> fdr_flg_fu_shift) & 3) == 1)
			&& (! (flag & fdr_flg_ali))
			&& (! (flag & fdr_flg_cdr))
			&& (iter->level < MAX_DIR_LEVEL)
			&& ! ((iter->level == 0) && ! memcmp(iter->xdr[iter->level].fdr.fnm, "VCATALOG", 8)))
		{
			ti990_dor dor;

			if (get_UINT16BE(iter->xdr[iter->level].fdr.saa) != 0)
				printf("ninou");

			read_sector_logical_len(*iter->image->file_handle,
									get_UINT16BE(iter->xdr[iter->level].fdr.paa) * get_UINT16BE(iter->image->sec0.spa),
									& iter->image->geometry, &dor, sizeof(dor));

			iter->level++;
			iter->nrc[iter->level] = get_UINT16BE(dor.nrc)/*get_UINT32BE(iter->fdr[iter->level-1].eom)-1*/;
			iter->index[iter->level] = 0;
#if 0
			if (get_UINT16BE(dor.nrc) != (get_UINT32BE(iter->xdr[iter->level-1].fdr.eom)-1))
				printf("hiha");
#endif
		}

		/* go to upper level if applicable */
		while (iter->index[iter->level] >= iter->nrc[iter->level])
			iter->level--;
	}

	return (imgtoolerr_t)0;
}

/*
    Free enumerator
*/
static void ti990_image_closeenum(imgtool::directory &enumeration)
{
}

/*
    Compute free space on disk image (in ADUs)
*/
static imgtoolerr_t ti990_image_freespace(imgtool::partition &partition, uint64_t *size)
{
	imgtool::image &img(partition.image());
	ti990_image *image = get_ti990_image(img);
	int totadus = get_UINT16BE(image->sec0.tna);
	int adu, record, sub_adu;
	char buf[256];
	size_t freeadus = 0;

	if ((totadus+2031)/2032 != image->sec0.tbm)
		/*return IMGTOOLERR_CORRUPTIMAGE;*/
		return (imgtoolerr_t)0;

	adu = 0;
	record = 0;
	sub_adu = 0;
	while (adu<totadus)
	{
		read_sector_logical_len(*image->file_handle, image->sec0.sbm + record, &image->geometry, buf, sizeof(buf));

		while ((adu < totadus) && (sub_adu < 2032))
		{
			if (! (buf[(sub_adu >> 3) + 2] & (1 << (sub_adu & 7))))
				freeadus++;
			sub_adu++;
			adu++;
		}
		sub_adu = 0;
		record++;
	}

	* size = freeadus;

	return IMGTOOLERR_SUCCESS;
}

/*
    Extract a file from a ti990_image.
*/
static imgtoolerr_t ti990_image_readfile(imgtool::partition &partition, const char *fpath, imgtool::stream *destf)
{
	ti990_image *image = get_ti990_image(partition.image());
	int catalog_index, fdr_secnum, parent_fdr_secnum;
	imgtoolerr_t reply;


	reply = find_fdr(image, fpath, &catalog_index, &fdr_secnum, &parent_fdr_secnum);
	if (reply)
		return reply;

	/* ... */

	return IMGTOOLERR_SUCCESS;

#if 0
	ti99_image *image = (ti99_image*) img;
	int totsecs = (image->sec0.totsecsMSB << 8) | image->sec0.totsecsLSB;
	char ti_fname[10];
	ti99_fdr fdr;
	tifile_header dst_header;
	int i, lnks_index;
	int cur_sec, last_sec;
	int secsused;
	uint8_t buf[256];
	int reply;

	str_to_fname(ti_fname, filename);

	reply = find_fdr(image, ti_fname, &fdr, NULL);
	if (reply)
		return reply;

	dst_header.tifiles[0] = '\7';
	dst_header.tifiles[1] = 'T';
	dst_header.tifiles[2] = 'I';
	dst_header.tifiles[3] = 'F';
	dst_header.tifiles[4] = 'I';
	dst_header.tifiles[5] = 'L';
	dst_header.tifiles[6] = 'E';
	dst_header.tifiles[7] = 'S';
	dst_header.secsused_MSB = fdr.secsused_MSB;
	dst_header.secsused_LSB = fdr.secsused_LSB;
	dst_header.flags = fdr.flags;
	dst_header.recspersec = fdr.recspersec;
	dst_header.eof = fdr.eof;
	dst_header.reclen = fdr.reclen;
	dst_header.fixrecs_MSB = fdr.fixrecs_MSB;
	dst_header.fixrecs_LSB = fdr.fixrecs_LSB;
	for (i=0; i<(128-14); i++)
		dst_header.res[i] = 0;

	if (destf->write(& dst_header, 128) != 128)
		return IMGTOOLERR_WRITEERROR;


	secsused = get_fdr_secsused(&fdr);

	i = 0;          /* file logical sector #0 */
	lnks_index = 0; /* start of file block table */
	while (i<secsused)
	{
		if (lnks_index == 76)
			return IMGTOOLERR_CORRUPTIMAGE;

		/* read curent file block table entry */
		cur_sec = ((fdr.lnks[lnks_index][1] & 0xf) << 8) | fdr.lnks[lnks_index][0];
		last_sec = (fdr.lnks[lnks_index][2] << 4) | (fdr.lnks[lnks_index][1] >> 4);

		/* copy current file block */
		while ((i<=last_sec) && (i<secsused))
		{
			if (cur_sec >= totsecs)
				return IMGTOOLERR_CORRUPTIMAGE;

			if (read_sector_logical(image->file_handle, cur_sec, & image->geometry, buf))
				return IMGTOOLERR_READERROR;

			if (destf->write(buf, 256) != 256)
				return IMGTOOLERR_WRITEERROR;

			i++;
			cur_sec++;
		}

		/* next entry in file block table */
		lnks_index++;
	}

	return IMGTOOLERR_SUCCESS;
#endif
}

/*
    Add a file to a ti990_image.
*/
static imgtoolerr_t ti990_image_writefile(imgtool::partition &partition, const char *fpath, imgtool::stream *sourcef, util::option_resolution *writeoptions)
{
	ti990_image *image = get_ti990_image(partition.image());
	int catalog_index, fdr_secnum, parent_fdr_secnum;
	imgtoolerr_t reply;


	/* check that file does not exist */
	reply = find_fdr(image, fpath, &catalog_index, &fdr_secnum, &parent_fdr_secnum);
	if ((reply) && (reply != IMGTOOLERR_FILENOTFOUND))
		return reply;

	/* ... */

	return IMGTOOLERR_SUCCESS;

#if 0
	ti99_image *image = (ti99_image*) img;
	int totsecs = (image->sec0.totsecsMSB << 8) | image->sec0.totsecsLSB;
	char ti_fname[10];
	ti99_fdr fdr;
	tifile_header src_header;
	int i, lnks_index;
	int cur_sec, last_sec;
	int secsused;
	uint8_t buf[256];
	int reply;
	int fdr_secnum;

	str_to_fname(ti_fname, filename);

	reply = find_fdr(image, ti_fname, &fdr, NULL);
	if (reply == 0)
	{   /* file already exists: causes an error for now */
		return IMGTOOLERR_UNEXPECTED;
	}
	else if (reply != IMGTOOLERR_FILENOTFOUND)
	{
		return reply;
	}

	if (sourcef->read(& src_header, 128) != 128)
		return IMGTOOLERR_READERROR;

	/* create new file */
	reply = new_file(image, ti_fname, &fdr_secnum);
	if (reply)
		return reply;

	memcpy(fdr.name, ti_fname, 10);
	fdr.res10[1] = fdr.res10[0] = 0;
	fdr.flags = src_header.flags;
	fdr.recspersec = src_header.recspersec;
	fdr.secsused_MSB = /*src_header.secsused_MSB*/0;
	fdr.secsused_LSB = /*src_header.secsused_LSB*/0;
	fdr.eof = src_header.eof;
	fdr.reclen = src_header.reclen;
	fdr.fixrecs_LSB = src_header.fixrecs_LSB;
	fdr.fixrecs_MSB = src_header.fixrecs_MSB;
	for (i=0; i<8; i++)
		fdr.res20[i] = 0;
	for (i=0; i<76; i++)
		fdr.lnks[i][2] = fdr.lnks[i][1] = fdr.lnks[i][0] = 0;

	/* alloc data sectors */
	secsused = (src_header.secsused_MSB << 8) | src_header.secsused_LSB;
	reply = alloc_file_sectors(image, &fdr, secsused);
	if (reply)
		return reply;

	/* copy data */
	i = 0;
	lnks_index = 0;
	while (i<secsused)
	{
		if (lnks_index == 76)
			return IMGTOOLERR_CORRUPTIMAGE;

		cur_sec = ((fdr.lnks[lnks_index][1] & 0xf) << 8) | fdr.lnks[lnks_index][0];
		last_sec = (fdr.lnks[lnks_index][2] << 4) | (fdr.lnks[lnks_index][1] >> 4);

		while ((i<secsused) && (i<=last_sec))
		{
			if (cur_sec >= totsecs)
				return IMGTOOLERR_CORRUPTIMAGE;

			if (sourcef->read(buf, 256) != 256)
				return IMGTOOLERR_READERROR;

			if (write_sector_logical(image->file_handle, cur_sec, & image->geometry, buf))
				return IMGTOOLERR_WRITEERROR;

			i++;
			cur_sec++;
		}

		lnks_index++;
	}

	/* write fdr */
	if (write_sector_logical(image->file_handle, fdr_secnum, & image->geometry, &fdr))
		return IMGTOOLERR_WRITEERROR;

	/* update catalog */
	for (i=0; i<128; i++)
	{
		buf[2*i] = image->catalog[i].fdr_secnum >> 8;
		buf[2*i+1] = image->catalog[i].fdr_secnum & 0xff;
	}
	if (write_sector_logical(image->file_handle, 1, & image->geometry, buf))
		return IMGTOOLERR_WRITEERROR;

	/* update bitmap */
	if (write_sector_logical(image->file_handle, 0, & image->geometry, &image->sec0))
		return IMGTOOLERR_WRITEERROR;

	return IMGTOOLERR_SUCCESS;
#endif
}

/*
    Delete a file from a ti990_image.
*/
static imgtoolerr_t ti990_image_deletefile(imgtool::partition &partition, const char *fpath)
{
	ti990_image *image = get_ti990_image(partition.image());
	int catalog_index, fdr_secnum, parent_fdr_secnum;
	imgtoolerr_t reply;


	reply = find_fdr(image, fpath, &catalog_index, &fdr_secnum, &parent_fdr_secnum);
	if (reply)
		return reply;

	/* ... */

	return IMGTOOLERR_SUCCESS;

#if 0
	ti99_image *image = (ti99_image*) img;
	int totsecs = (image->sec0.totsecsMSB << 8) | image->sec0.totsecsLSB;
	char ti_fname[10];
	ti99_fdr fdr;
	int i, lnks_index;
	int cur_sec, last_sec;
	int secsused;
	int catalog_index;
	int reply;
	uint8_t buf[256];

	str_to_fname(ti_fname, filename);

	reply = find_fdr(image, ti_fname, &fdr, &catalog_index);
	if (reply)
		return reply;

	/* free data sectors */
	secsused = get_fdr_secsused(&fdr);

	i = 0;
	lnks_index = 0;
	while (i<secsused)
	{
		if (lnks_index == 76)
			return IMGTOOLERR_CORRUPTIMAGE;

		cur_sec = ((fdr.lnks[lnks_index][1] & 0xf) << 8) | fdr.lnks[lnks_index][0];
		last_sec = (fdr.lnks[lnks_index][2] << 4) | (fdr.lnks[lnks_index][1] >> 4);

		while ((i<secsused) && (i<=last_sec))
		{
			if (cur_sec >= totsecs)
				return IMGTOOLERR_CORRUPTIMAGE;

			image->sec0.abm[cur_sec >> 3] &= ~ (1 << (cur_sec & 7));

			i++;
			cur_sec++;
		}

		lnks_index++;
	}

	/* free fdr sector */
	image->sec0.abm[image->catalog[catalog_index].fdr_secnum >> 3] &= ~ (1 << (image->catalog[catalog_index].fdr_secnum & 7));

	/* delete catalog entry */
	for (i=catalog_index; i<127; i++)
		image->catalog[i] = image->catalog[i+1];
	image->catalog[127].fdr_secnum = 0;

	/* update catalog */
	for (i=0; i<128; i++)
	{
		buf[2*i] = image->catalog[i].fdr_secnum >> 8;
		buf[2*i+1] = image->catalog[i].fdr_secnum & 0xff;
	}
	if (write_sector_logical(image->file_handle, 1, & image->geometry, buf))
		return IMGTOOLERR_WRITEERROR;

	/* update bitmap */
	if (write_sector_logical(image->file_handle, 0, & image->geometry, &image->sec0))
		return IMGTOOLERR_WRITEERROR;

	return IMGTOOLERR_SUCCESS;
#endif
}

/*
    Create a blank ti990_image.
*/
static imgtoolerr_t ti990_image_create(imgtool::image &image, imgtool::stream::ptr &&stream, util::option_resolution *createoptions)
{
	//const char *volname;
	ti990_geometry geometry;
	unsigned totsecs;
	disk_image_header header;
	ti990_sc0 sec0;
	uint8_t empty_sec[MAX_SECTOR_SIZE];
	int reply;
	int i;

	/* read options */
	//volname = createoptions->lookup_string(ti990_createopts_volname);
	geometry.cylinders = createoptions->lookup_int(ti990_createopts_cylinders);
	geometry.heads = createoptions->lookup_int(ti990_createopts_heads);
	geometry.sectors_per_track = createoptions->lookup_int(ti990_createopts_sectors);
	geometry.bytes_per_sector = createoptions->lookup_int(ti990_createopts_sectorsize);

	totsecs = geometry.cylinders * geometry.heads * geometry.sectors_per_track;

	/* write header */
	set_UINT32BE(& header.cylinders, geometry.cylinders);
	set_UINT32BE(& header.heads, geometry.heads);
	set_UINT32BE(& header.sectors_per_track, geometry.sectors_per_track);
	set_UINT32BE(& header.bytes_per_sector, geometry.bytes_per_sector);

	reply = stream->write(&header, sizeof(header));
	if (reply != sizeof(header))
	{
		return IMGTOOLERR_WRITEERROR;
	}


	/* initialize sector 0 */
	/* mark disk as uninitialized */
	set_UINT16BE(& sec0.sta, 2);


	/* write sector 0 */
	if (write_sector_logical(*stream, 0, & geometry, &sec0))
		return IMGTOOLERR_WRITEERROR;


	/* now clear every other sector, including catalog */
	memset(empty_sec, 0, geometry.bytes_per_sector);

	for (i=1; i<totsecs; i++)
		if (write_sector_logical(*stream, i, & geometry, empty_sec))
			return IMGTOOLERR_WRITEERROR;

	return (imgtoolerr_t)0;
}
