/*********************************************************************

    romload.h

    ROM loading functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __ROMLOAD_H__
#define __ROMLOAD_H__

#include "mamecore.h"
#include "chd.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* ----- per-entry constants ----- */
enum
{
	ROMENTRYTYPE_REGION = 1,	/* this entry marks the start of a region */
	ROMENTRYTYPE_END,		/* this entry marks the end of a region */
	ROMENTRYTYPE_RELOAD,		/* this entry reloads the previous ROM */
	ROMENTRYTYPE_CONTINUE,		/* this entry continues loading the previous ROM */
	ROMENTRYTYPE_FILL,		/* this entry fills an area with a constant value */
	ROMENTRYTYPE_COPY,		/* this entry copies data from another region/offset */
	ROMENTRYTYPE_CARTRIDGE,		/* this entry specifies a cartridge (MESS) */
	ROMENTRYTYPE_IGNORE,		/* this entry continues loading the previous ROM but throws the data away */
	ROMENTRYTYPE_SYSTEM_BIOS,	/* this entry specifies a bios */
	ROMENTRYTYPE_COUNT
};

/* ----- per-region constants ----- */
#define ROMREGION_WIDTHMASK			0x00000003			/* native width of region, as power of 2 */
#define		ROMREGION_8BIT			0x00000000			/*    (non-CPU regions only) */
#define		ROMREGION_16BIT			0x00000001
#define		ROMREGION_32BIT			0x00000002
#define		ROMREGION_64BIT			0x00000003

#define ROMREGION_ENDIANMASK		0x00000004			/* endianness of the region */
#define		ROMREGION_LE			0x00000000			/*    (non-CPU regions only) */
#define		ROMREGION_BE			0x00000004

#define ROMREGION_INVERTMASK		0x00000008			/* invert the bits of the region */
#define		ROMREGION_NOINVERT		0x00000000
#define		ROMREGION_INVERT		0x00000008

#define ROMREGION_DISPOSEMASK		0x00000010			/* dispose of the region after init */
#define		ROMREGION_NODISPOSE		0x00000000
#define		ROMREGION_DISPOSE		0x00000010

#define ROMREGION_LOADUPPERMASK		0x00000040			/* load into the upper part of CPU space */
#define		ROMREGION_LOADLOWER		0x00000000			/*     (CPU regions only) */
#define		ROMREGION_LOADUPPER		0x00000040

#define ROMREGION_ERASEMASK			0x00000080			/* erase the region before loading */
#define		ROMREGION_NOERASE		0x00000000
#define		ROMREGION_ERASE			0x00000080

#define ROMREGION_ERASEVALMASK		0x0000ff00			/* value to erase the region to */
#define		ROMREGION_ERASEVAL(x)	((((x) & 0xff) << 8) | ROMREGION_ERASE)
#define		ROMREGION_ERASE00		ROMREGION_ERASEVAL(0)
#define		ROMREGION_ERASEFF		ROMREGION_ERASEVAL(0xff)

#define ROMREGION_DATATYPEMASK		0x00010000			/* inherit all flags from previous definition */
#define		ROMREGION_DATATYPEROM	0x00000000
#define		ROMREGION_DATATYPEDISK	0x00010000


/* ----- per-ROM constants ----- */
#define DISK_READONLYMASK			0x00000400			/* is the disk read-only? */
#define		DISK_READWRITE			0x00000000
#define		DISK_READONLY			0x00000400

#define ROM_OPTIONALMASK			0x00000800			/* optional - won't hurt if it's not there */
#define		ROM_REQUIRED			0x00000000
#define		ROM_OPTIONAL			0x00000800

#define ROM_GROUPMASK				0x0000f000			/* load data in groups of this size + 1 */
#define		ROM_GROUPSIZE(n)		((((n) - 1) & 15) << 12)
#define		ROM_GROUPBYTE			ROM_GROUPSIZE(1)
#define		ROM_GROUPWORD			ROM_GROUPSIZE(2)
#define		ROM_GROUPDWORD			ROM_GROUPSIZE(4)

#define ROM_SKIPMASK				0x000f0000			/* skip this many bytes after each group */
#define		ROM_SKIP(n)				(((n) & 15) << 16)
#define		ROM_NOSKIP				ROM_SKIP(0)

#define ROM_REVERSEMASK				0x00100000			/* reverse the byte order within a group */
#define		ROM_NOREVERSE			0x00000000
#define		ROM_REVERSE				0x00100000

#define ROM_BITWIDTHMASK			0x00e00000			/* width of data in bits */
#define		ROM_BITWIDTH(n)			(((n) & 7) << 21)
#define		ROM_NIBBLE				ROM_BITWIDTH(4)
#define		ROM_FULLBYTE			ROM_BITWIDTH(8)

#define ROM_BITSHIFTMASK			0x07000000			/* left-shift count for the bits */
#define		ROM_BITSHIFT(n)			(((n) & 7) << 24)
#define		ROM_NOSHIFT				ROM_BITSHIFT(0)
#define		ROM_SHIFT_NIBBLE_LO		ROM_BITSHIFT(0)
#define		ROM_SHIFT_NIBBLE_HI		ROM_BITSHIFT(4)

#define ROM_INHERITFLAGSMASK		0x08000000			/* inherit all flags from previous definition */
#define		ROM_INHERITFLAGS		0x08000000

#define ROM_BIOSFLAGSMASK			0xf0000000			/* only loaded if value matches global bios value */
#define 	ROM_BIOS(n)				(((n) & 15) << 28)

#define ROM_INHERITEDFLAGS			(ROM_GROUPMASK | ROM_SKIPMASK | ROM_REVERSEMASK | ROM_BITWIDTHMASK | ROM_BITSHIFTMASK | ROM_BIOSFLAGSMASK)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _rom_entry rom_entry;
struct _rom_entry
{
	const char *	_name;				/* name of the file to load */
	UINT32			_offset;			/* offset to load it to */
	UINT32			_length;			/* length of the file */
	UINT32			_flags;				/* flags */
	const char *	_hashdata;			/* hashing informations (checksums) */
};


/* In mamecore.h: typedef struct _rom_load_data rom_load_data; */
struct _rom_load_data
{
	int				warnings;			/* warning count during processing */
	int				errors;				/* error count during processing */

	int				romsloaded;			/* current ROMs loaded count */
	int				romstotal;			/* total number of ROMs to read */

	mame_file *		file;				/* current file */

	UINT8 *			regionbase;			/* base of current region */
	UINT32			regionlength;		/* length of current region */

	char			errorbuf[4096];		/* accumulated errors */
	UINT8			tempbuf[65536];		/* temporary buffer */
};



/***************************************************************************
    MACROS
***************************************************************************/

/* ----- per-entry macros ----- */
#define ROMENTRY_REGION				((const char *)ROMENTRYTYPE_REGION)
#define ROMENTRY_END				((const char *)ROMENTRYTYPE_END)
#define ROMENTRY_RELOAD				((const char *)ROMENTRYTYPE_RELOAD)
#define ROMENTRY_CONTINUE			((const char *)ROMENTRYTYPE_CONTINUE)
#define ROMENTRY_FILL				((const char *)ROMENTRYTYPE_FILL)
#define ROMENTRY_COPY				((const char *)ROMENTRYTYPE_COPY)
#define ROMENTRY_CARTRIDGE			((const char *)ROMENTRYTYPE_CARTRIDGE)
#define ROMENTRY_IGNORE				((const char *)ROMENTRYTYPE_IGNORE)
#define ROMENTRY_SYSTEM_BIOS		((const char *)ROMENTRYTYPE_SYSTEM_BIOS)

#define ROMENTRY_GETTYPE(r)			((FPTR)(r)->_name)
#define ROMENTRY_ISSPECIAL(r)		(ROMENTRY_GETTYPE(r) < ROMENTRYTYPE_COUNT)
#define ROMENTRY_ISFILE(r)			(!ROMENTRY_ISSPECIAL(r))
#define ROMENTRY_ISREGION(r)		((r)->_name == ROMENTRY_REGION)
#define ROMENTRY_ISEND(r)			((r)->_name == ROMENTRY_END)
#define ROMENTRY_ISRELOAD(r)		((r)->_name == ROMENTRY_RELOAD)
#define ROMENTRY_ISCONTINUE(r)		((r)->_name == ROMENTRY_CONTINUE)
#define ROMENTRY_ISFILL(r)			((r)->_name == ROMENTRY_FILL)
#define ROMENTRY_ISCOPY(r)			((r)->_name == ROMENTRY_COPY)
#define ROMENTRY_ISIGNORE(r)		((r)->_name == ROMENTRY_IGNORE)
#define ROMENTRY_ISSYSTEM_BIOS(r)	((r)->_name == ROMENTRY_SYSTEM_BIOS)
#define ROMENTRY_ISREGIONEND(r)		(ROMENTRY_ISREGION(r) || ROMENTRY_ISEND(r))

/* ----- per-region macros ----- */
#define ROMREGION_GETTYPE(r)		((FPTR)(r)->_hashdata)
#define ROMREGION_GETLENGTH(r)		((r)->_length)
#define ROMREGION_GETFLAGS(r)		((r)->_flags)
#define ROMREGION_GETWIDTH(r)		(8 << (ROMREGION_GETFLAGS(r) & ROMREGION_WIDTHMASK))
#define ROMREGION_ISLITTLEENDIAN(r)	((ROMREGION_GETFLAGS(r) & ROMREGION_ENDIANMASK) == ROMREGION_LE)
#define ROMREGION_ISBIGENDIAN(r)	((ROMREGION_GETFLAGS(r) & ROMREGION_ENDIANMASK) == ROMREGION_BE)
#define ROMREGION_ISINVERTED(r)		((ROMREGION_GETFLAGS(r) & ROMREGION_INVERTMASK) == ROMREGION_INVERT)
#define ROMREGION_ISDISPOSE(r)		((ROMREGION_GETFLAGS(r) & ROMREGION_DISPOSEMASK) == ROMREGION_DISPOSE)
#define ROMREGION_ISLOADUPPER(r)	((ROMREGION_GETFLAGS(r) & ROMREGION_LOADUPPERMASK) == ROMREGION_LOADUPPER)
#define ROMREGION_ISERASE(r)		((ROMREGION_GETFLAGS(r) & ROMREGION_ERASEMASK) == ROMREGION_ERASE)
#define ROMREGION_GETERASEVAL(r)	((ROMREGION_GETFLAGS(r) & ROMREGION_ERASEVALMASK) >> 8)
#define ROMREGION_GETDATATYPE(r)	(ROMREGION_GETFLAGS(r) & ROMREGION_DATATYPEMASK)
#define ROMREGION_ISROMDATA(r)		(ROMREGION_GETDATATYPE(r) == ROMREGION_DATATYPEROM)
#define ROMREGION_ISDISKDATA(r)		(ROMREGION_GETDATATYPE(r) == ROMREGION_DATATYPEDISK)


/* ----- per-ROM macros ----- */
#define ROM_GETNAME(r)				((r)->_name)
#define ROM_SAFEGETNAME(r)			(ROMENTRY_ISFILL(r) ? "fill" : ROMENTRY_ISCOPY(r) ? "copy" : ROM_GETNAME(r))
#define ROM_GETOFFSET(r)			((r)->_offset)
#define ROM_GETLENGTH(r)			((r)->_length)
#define ROM_GETFLAGS(r)				((r)->_flags)
#define ROM_GETHASHDATA(r)          ((r)->_hashdata)
#define ROM_ISOPTIONAL(r)			((ROM_GETFLAGS(r) & ROM_OPTIONALMASK) == ROM_OPTIONAL)
#define ROM_GETGROUPSIZE(r)			(((ROM_GETFLAGS(r) & ROM_GROUPMASK) >> 12) + 1)
#define ROM_GETSKIPCOUNT(r)			((ROM_GETFLAGS(r) & ROM_SKIPMASK) >> 16)
#define ROM_ISREVERSED(r)			((ROM_GETFLAGS(r) & ROM_REVERSEMASK) == ROM_REVERSE)
#define ROM_GETBITWIDTH(r)			(((ROM_GETFLAGS(r) & ROM_BITWIDTHMASK) >> 21) + 8 * ((ROM_GETFLAGS(r) & ROM_BITWIDTHMASK) == 0))
#define ROM_GETBITSHIFT(r)			((ROM_GETFLAGS(r) & ROM_BITSHIFTMASK) >> 24)
#define ROM_INHERITSFLAGS(r)		((ROM_GETFLAGS(r) & ROM_INHERITFLAGSMASK) == ROM_INHERITFLAGS)
#define ROM_GETBIOSFLAGS(r)			((ROM_GETFLAGS(r) & ROM_BIOSFLAGSMASK) >> 28)
#define ROM_NOGOODDUMP(r)			(hash_data_has_info((r)->_hashdata, HASH_INFO_NO_DUMP))


/* ----- per-disk macros ----- */
#define DISK_GETINDEX(r)			((r)->_offset)
#define DISK_ISREADONLY(r)			((ROM_GETFLAGS(r) & DISK_READONLYMASK) == DISK_READONLY)


/* ----- start/stop macros ----- */
#define ROM_START(name)								static const rom_entry rom_##name[] = {
#define ROM_END										{ ROMENTRY_END, 0, 0, 0, NULL } };


/* ----- ROM region macros ----- */
#define ROM_REGION(length,type,flags)				{ ROMENTRY_REGION, 0, length, flags, (const char*)type },
#define ROM_REGION16_LE(length,type,flags)			ROM_REGION(length, type, (flags) | ROMREGION_16BIT | ROMREGION_LE)
#define ROM_REGION16_BE(length,type,flags)			ROM_REGION(length, type, (flags) | ROMREGION_16BIT | ROMREGION_BE)
#define ROM_REGION32_LE(length,type,flags)			ROM_REGION(length, type, (flags) | ROMREGION_32BIT | ROMREGION_LE)
#define ROM_REGION32_BE(length,type,flags)			ROM_REGION(length, type, (flags) | ROMREGION_32BIT | ROMREGION_BE)
#define ROM_REGION64_LE(length,type,flags)			ROM_REGION(length, type, (flags) | ROMREGION_64BIT | ROMREGION_LE)
#define ROM_REGION64_BE(length,type,flags)			ROM_REGION(length, type, (flags) | ROMREGION_64BIT | ROMREGION_BE)


/* ----- core ROM loading macros ----- */
#define ROMMD5_LOAD(name,offset,length,hash,flags)	{ name, offset, length, flags, hash },
#define ROMX_LOAD(name,offset,length,hash,flags)	{ name, offset, length, flags, hash },
#define ROM_LOAD(name,offset,length,hash)			ROMX_LOAD(name, offset, length, hash, 0)
#define ROM_LOAD_OPTIONAL(name,offset,length,hash)	ROMX_LOAD(name, offset, length, hash, ROM_OPTIONAL)
#define ROM_CONTINUE(offset,length)					ROMX_LOAD(ROMENTRY_CONTINUE, offset, length, 0, ROM_INHERITFLAGS)
#define ROM_RELOAD(offset,length)					ROMX_LOAD(ROMENTRY_RELOAD, offset, length, 0, ROM_INHERITFLAGS)
#define ROM_FILL(offset,length,value)				ROM_LOAD(ROMENTRY_FILL, offset, length, (const char*)value)
#define ROM_COPY(rgn,srcoffset,offset,length)		ROMX_LOAD(ROMENTRY_COPY, offset, length, (const char*)srcoffset, (rgn) << 24)
#define ROM_IGNORE(length)							ROMX_LOAD(ROMENTRY_IGNORE, 0, length, 0, ROM_INHERITFLAGS)


/* ----- specialized loading macros ----- */
#define ROM_LOAD_NIB_HIGH(name,offset,length,hash)	ROMX_LOAD(name, offset, length, hash, ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI)
#define ROM_LOAD_NIB_LOW(name,offset,length,hash)	ROMX_LOAD(name, offset, length, hash, ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
#define ROM_LOAD16_BYTE(name,offset,length,hash)	ROMX_LOAD(name, offset, length, hash, ROM_SKIP(1))
#define ROM_LOAD16_WORD(name,offset,length,hash)	ROM_LOAD(name, offset, length, hash)
#define ROM_LOAD16_WORD_SWAP(name,offset,length,hash) ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE)
#define ROM_LOAD32_BYTE(name,offset,length,hash)	ROMX_LOAD(name, offset, length, hash, ROM_SKIP(3))
#define ROM_LOAD32_WORD(name,offset,length,hash)	ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(2))
#define ROM_LOAD32_WORD_SWAP(name,offset,length,hash) ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))
#define ROM_LOAD32_DWORD(name,offset,length,hash)	ROMX_LOAD(name, offset, length, hash, ROM_GROUPDWORD)


/* ----- system BIOS macros ----- */
#define ROM_SYSTEM_BIOS(value,name,description) ROMX_LOAD(ROMENTRY_SYSTEM_BIOS, 0, 0, name "\0" description, ROM_BIOS(value+1))

/* ----- disk loading macros ----- */
#define DISK_REGION(type)							ROM_REGION(1, type, ROMREGION_DATATYPEDISK)
#define DISK_IMAGE(name,idx,hash)					ROMMD5_LOAD(name, idx, 0, hash, DISK_READWRITE)
#define DISK_IMAGE_READONLY(name,idx,hash)			ROMMD5_LOAD(name, idx, 0, hash, DISK_READONLY)


/* ----- hash macros ----- */
#define CRC(x)										"c:" #x "#"
#define SHA1(x)										"s:" #x "#"
#define MD5(x)										"m:" #x "#"
#define NO_DUMP										"$ND$"
#define BAD_DUMP									"$BD$"



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* disk handling */
chd_error open_disk_image(const game_driver *gamedrv, const rom_entry *romp, mame_file **image_file, chd_file **image_chd);
chd_error open_disk_image_options(core_options *options, const game_driver *gamedrv, const rom_entry *romp, mame_file **image_file, chd_file **image_chd);
chd_file *get_disk_handle(int diskindex);

/* ROM processing */
void rom_init(running_machine *machine, const rom_entry *romp);
int rom_load_warnings(void);
const rom_entry *rom_first_region(const game_driver *drv);
const rom_entry *rom_next_region(const rom_entry *romp);
const rom_entry *rom_first_file(const rom_entry *romp);
const rom_entry *rom_next_file(const rom_entry *romp);
const rom_entry *rom_first_chunk(const rom_entry *romp);
const rom_entry *rom_next_chunk(const rom_entry *romp);


#endif	/* __ROMLOAD_H__ */
