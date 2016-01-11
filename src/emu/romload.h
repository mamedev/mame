// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
/*********************************************************************

    romload.h

    ROM loading functions.
*********************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __ROMLOAD_H__
#define __ROMLOAD_H__

#include "chd.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* ----- type constants ----- */
#define ROMENTRY_TYPEMASK           0x0000000f          /* type of entry */
enum
{
	ROMENTRYTYPE_ROM = 0,       /* this entry is an actual ROM definition */
	ROMENTRYTYPE_REGION,        /* this entry marks the start of a region */
	ROMENTRYTYPE_END,           /* this entry marks the end of a region */
	ROMENTRYTYPE_RELOAD,        /* this entry reloads the previous ROM */
	ROMENTRYTYPE_CONTINUE,      /* this entry continues loading the previous ROM */
	ROMENTRYTYPE_FILL,          /* this entry fills an area with a constant value */
	ROMENTRYTYPE_COPY,          /* this entry copies data from another region/offset */
	ROMENTRYTYPE_CARTRIDGE,     /* this entry specifies a cartridge (MESS) */
	ROMENTRYTYPE_IGNORE,        /* this entry continues loading the previous ROM but throws the data away */
	ROMENTRYTYPE_SYSTEM_BIOS,   /* this entry specifies a bios */
	ROMENTRYTYPE_DEFAULT_BIOS,  /* this entry specifies a default bios */
	ROMENTRYTYPE_PARAMETER,     /* this entry specifies a per-game parameter */
	ROMENTRYTYPE_COUNT
};

/* ----- per-region constants ----- */
#define ROMREGION_WIDTHMASK         0x00000300          /* native width of region, as power of 2 */
#define     ROMREGION_8BIT          0x00000000          /*    (non-CPU regions only) */
#define     ROMREGION_16BIT         0x00000100
#define     ROMREGION_32BIT         0x00000200
#define     ROMREGION_64BIT         0x00000300

#define ROMREGION_ENDIANMASK        0x00000400          /* endianness of the region */
#define     ROMREGION_LE            0x00000000          /*    (non-CPU regions only) */
#define     ROMREGION_BE            0x00000400

#define ROMREGION_INVERTMASK        0x00000800          /* invert the bits of the region */
#define     ROMREGION_NOINVERT      0x00000000
#define     ROMREGION_INVERT        0x00000800

#define ROMREGION_ERASEMASK         0x00002000          /* erase the region before loading */
#define     ROMREGION_NOERASE       0x00000000
#define     ROMREGION_ERASE         0x00002000

#define ROMREGION_DATATYPEMASK      0x00004000          /* type of region (ROM versus disk) */
#define     ROMREGION_DATATYPEROM   0x00000000
#define     ROMREGION_DATATYPEDISK  0x00004000

#define ROMREGION_ERASEVALMASK      0x00ff0000          /* value to erase the region to */
#define     ROMREGION_ERASEVAL(x)   ((((x) & 0xff) << 16) | ROMREGION_ERASE)
#define     ROMREGION_ERASE00       ROMREGION_ERASEVAL(0)
#define     ROMREGION_ERASEFF       ROMREGION_ERASEVAL(0xff)


/* ----- per-ROM constants ----- */
#define DISK_READONLYMASK           0x00000010          /* is the disk read-only? */
#define     DISK_READWRITE          0x00000000
#define     DISK_READONLY           0x00000010

#define ROM_OPTIONALMASK            0x00000020          /* optional - won't hurt if it's not there */
#define     ROM_REQUIRED            0x00000000
#define     ROM_OPTIONAL            0x00000020

#define ROM_REVERSEMASK             0x00000040          /* reverse the byte order within a group */
#define     ROM_NOREVERSE           0x00000000
#define     ROM_REVERSE             0x00000040

#define ROM_INHERITFLAGSMASK        0x00000080          /* inherit all flags from previous definition */
#define     ROM_INHERITFLAGS        0x00000080

#define ROM_GROUPMASK               0x00000f00          /* load data in groups of this size + 1 */
#define     ROM_GROUPSIZE(n)        ((((n) - 1) & 15) << 8)
#define     ROM_GROUPBYTE           ROM_GROUPSIZE(1)
#define     ROM_GROUPWORD           ROM_GROUPSIZE(2)
#define     ROM_GROUPDWORD          ROM_GROUPSIZE(4)

#define ROM_SKIPMASK                0x0000f000          /* skip this many bytes after each group */
#define     ROM_SKIP(n)             (((n) & 15) << 12)
#define     ROM_NOSKIP              ROM_SKIP(0)

#define ROM_BITWIDTHMASK            0x000f0000          /* width of data in bits */
#define     ROM_BITWIDTH(n)         (((n) & 15) << 16)
#define     ROM_NIBBLE              ROM_BITWIDTH(4)
#define     ROM_FULLBYTE            ROM_BITWIDTH(8)

#define ROM_BITSHIFTMASK            0x00f00000          /* left-shift count for the bits */
#define     ROM_BITSHIFT(n)         (((n) & 15) << 20)
#define     ROM_NOSHIFT             ROM_BITSHIFT(0)
#define     ROM_SHIFT_NIBBLE_LO     ROM_BITSHIFT(0)
#define     ROM_SHIFT_NIBBLE_HI     ROM_BITSHIFT(4)

#define ROM_BIOSFLAGSMASK           0xff000000          /* only loaded if value matches global bios value */
#define     ROM_BIOS(n)             (((n) & 255) << 24)

#define ROM_INHERITEDFLAGS          (ROM_GROUPMASK | ROM_SKIPMASK | ROM_REVERSEMASK | ROM_BITWIDTHMASK | ROM_BITSHIFTMASK | ROM_BIOSFLAGSMASK)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class machine_config;
class emu_options;
class chd_file;
class software_list_device;

struct rom_entry
{
	const char *    _name;              /* name of the file to load */
	const char *    _hashdata;          /* hashing informations (checksums) */
	UINT32          _offset;            /* offset to load it to */
	UINT32          _length;            /* length of the file */
	UINT32          _flags;             /* flags */
};



/***************************************************************************
    MACROS
***************************************************************************/

/* ----- per-entry macros ----- */
#define ROMENTRY_GETTYPE(r)         ((r)->_flags & ROMENTRY_TYPEMASK)
#define ROMENTRY_ISSPECIAL(r)       (ROMENTRY_GETTYPE(r) != ROMENTRYTYPE_ROM)
#define ROMENTRY_ISFILE(r)          (ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_ROM)
#define ROMENTRY_ISREGION(r)        (ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_REGION)
#define ROMENTRY_ISEND(r)           (ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_END)
#define ROMENTRY_ISRELOAD(r)        (ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_RELOAD)
#define ROMENTRY_ISCONTINUE(r)      (ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_CONTINUE)
#define ROMENTRY_ISFILL(r)          (ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_FILL)
#define ROMENTRY_ISCOPY(r)          (ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_COPY)
#define ROMENTRY_ISIGNORE(r)        (ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_IGNORE)
#define ROMENTRY_ISSYSTEM_BIOS(r)   (ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_SYSTEM_BIOS)
#define ROMENTRY_ISDEFAULT_BIOS(r)  (ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_DEFAULT_BIOS)
#define ROMENTRY_ISPARAMETER(r)     (ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_PARAMETER)
#define ROMENTRY_ISREGIONEND(r)     (ROMENTRY_ISREGION(r) || ROMENTRY_ISPARAMETER(r) || ROMENTRY_ISEND(r))

/* ----- per-region macros ----- */
#define ROMREGION_GETTAG(r)         ((r)->_name)
#define ROMREGION_GETLENGTH(r)      ((r)->_length)
#define ROMREGION_GETFLAGS(r)       ((r)->_flags)
#define ROMREGION_GETWIDTH(r)       (8 << ((ROMREGION_GETFLAGS(r) & ROMREGION_WIDTHMASK) >> 8))
#define ROMREGION_ISLITTLEENDIAN(r) ((ROMREGION_GETFLAGS(r) & ROMREGION_ENDIANMASK) == ROMREGION_LE)
#define ROMREGION_ISBIGENDIAN(r)    ((ROMREGION_GETFLAGS(r) & ROMREGION_ENDIANMASK) == ROMREGION_BE)
#define ROMREGION_ISINVERTED(r)     ((ROMREGION_GETFLAGS(r) & ROMREGION_INVERTMASK) == ROMREGION_INVERT)
#define ROMREGION_ISERASE(r)        ((ROMREGION_GETFLAGS(r) & ROMREGION_ERASEMASK) == ROMREGION_ERASE)
#define ROMREGION_GETERASEVAL(r)    ((ROMREGION_GETFLAGS(r) & ROMREGION_ERASEVALMASK) >> 16)
#define ROMREGION_GETDATATYPE(r)    (ROMREGION_GETFLAGS(r) & ROMREGION_DATATYPEMASK)
#define ROMREGION_ISROMDATA(r)      (ROMREGION_GETDATATYPE(r) == ROMREGION_DATATYPEROM)
#define ROMREGION_ISDISKDATA(r)     (ROMREGION_GETDATATYPE(r) == ROMREGION_DATATYPEDISK)


/* ----- per-ROM macros ----- */
#define ROM_GETNAME(r)              ((r)->_name)
#define ROM_SAFEGETNAME(r)          (ROMENTRY_ISFILL(r) ? "fill" : ROMENTRY_ISCOPY(r) ? "copy" : ROM_GETNAME(r))
#define ROM_GETOFFSET(r)            ((r)->_offset)
#define ROM_GETLENGTH(r)            ((r)->_length)
#define ROM_GETFLAGS(r)             ((r)->_flags)
#define ROM_GETHASHDATA(r)          ((r)->_hashdata)
#define ROM_ISOPTIONAL(r)           ((ROM_GETFLAGS(r) & ROM_OPTIONALMASK) == ROM_OPTIONAL)
#define ROM_GETGROUPSIZE(r)         (((ROM_GETFLAGS(r) & ROM_GROUPMASK) >> 8) + 1)
#define ROM_GETSKIPCOUNT(r)         ((ROM_GETFLAGS(r) & ROM_SKIPMASK) >> 12)
#define ROM_ISREVERSED(r)           ((ROM_GETFLAGS(r) & ROM_REVERSEMASK) == ROM_REVERSE)
#define ROM_GETBITWIDTH(r)          (((ROM_GETFLAGS(r) & ROM_BITWIDTHMASK) >> 16) + 8 * ((ROM_GETFLAGS(r) & ROM_BITWIDTHMASK) == 0))
#define ROM_GETBITSHIFT(r)          ((ROM_GETFLAGS(r) & ROM_BITSHIFTMASK) >> 20)
#define ROM_INHERITSFLAGS(r)        ((ROM_GETFLAGS(r) & ROM_INHERITFLAGSMASK) == ROM_INHERITFLAGS)
#define ROM_GETBIOSFLAGS(r)         ((ROM_GETFLAGS(r) & ROM_BIOSFLAGSMASK) >> 24)


/* ----- per-disk macros ----- */
#define DISK_GETINDEX(r)            ((r)->_offset)
#define DISK_ISREADONLY(r)          ((ROM_GETFLAGS(r) & DISK_READONLYMASK) == DISK_READONLY)


/* ----- start/stop macros ----- */
#define ROM_NAME(name)                              rom_##name
#define ROM_START(name)                             static const rom_entry ROM_NAME(name)[] = {
#define ROM_END                                     { NULL, NULL, 0, 0, ROMENTRYTYPE_END } };


/* ----- ROM region macros ----- */
#define ROM_REGION(length,tag,flags)                { tag, NULL, 0, length, ROMENTRYTYPE_REGION | (flags) },
#define ROM_REGION16_LE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_16BIT | ROMREGION_LE)
#define ROM_REGION16_BE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_16BIT | ROMREGION_BE)
#define ROM_REGION32_LE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_32BIT | ROMREGION_LE)
#define ROM_REGION32_BE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_32BIT | ROMREGION_BE)
#define ROM_REGION64_LE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_64BIT | ROMREGION_LE)
#define ROM_REGION64_BE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_64BIT | ROMREGION_BE)


/* ----- core ROM loading macros ----- */
#define ROMX_LOAD(name,offset,length,hash,flags)    { name, hash, offset, length, ROMENTRYTYPE_ROM | (flags) },
#define ROM_LOAD(name,offset,length,hash)           ROMX_LOAD(name, offset, length, hash, 0)
#define ROM_LOAD_OPTIONAL(name,offset,length,hash)  ROMX_LOAD(name, offset, length, hash, ROM_OPTIONAL)


/* ----- specialized loading macros ----- */
#define ROM_LOAD_NIB_HIGH(name,offset,length,hash)  ROMX_LOAD(name, offset, length, hash, ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI)
#define ROM_LOAD_NIB_LOW(name,offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
#define ROM_LOAD16_BYTE(name,offset,length,hash)    ROMX_LOAD(name, offset, length, hash, ROM_SKIP(1))
#define ROM_LOAD16_WORD(name,offset,length,hash)    ROM_LOAD(name, offset, length, hash)
#define ROM_LOAD16_WORD_SWAP(name,offset,length,hash) ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE)
#define ROM_LOAD32_BYTE(name,offset,length,hash)    ROMX_LOAD(name, offset, length, hash, ROM_SKIP(3))
#define ROM_LOAD32_WORD(name,offset,length,hash)    ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(2))
#define ROM_LOAD32_WORD_SWAP(name,offset,length,hash) ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))
#define ROM_LOAD32_DWORD(name,offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_GROUPDWORD)
#define ROM_LOAD64_WORD(name,offset,length,hash)    ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(6))
#define ROM_LOAD64_WORD_SWAP(name,offset,length,hash) ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6))
#define ROM_LOAD64_DWORD_SWAP(name,offset,length,hash) ROMX_LOAD(name, offset, length, hash, ROM_GROUPDWORD | ROM_REVERSE | ROM_SKIP(4))


/* ----- ROM_RELOAD related macros ----- */
#define ROM_RELOAD(offset,length)                   { NULL, NULL, offset, length, ROMENTRYTYPE_RELOAD | ROM_INHERITFLAGS },
#define ROM_RELOAD_PLAIN(offset,length)                 { NULL, NULL, offset, length, ROMENTRYTYPE_RELOAD },

/* ----- additional ROM-related macros ----- */
#define ROM_CONTINUE(offset,length)                 { NULL, NULL, offset, length, ROMENTRYTYPE_CONTINUE | ROM_INHERITFLAGS },
#define ROM_IGNORE(length)                          { NULL, NULL, 0,      length, ROMENTRYTYPE_IGNORE | ROM_INHERITFLAGS },
#define ROM_FILL(offset,length,value)               { NULL, (const char *)value, offset, length, ROMENTRYTYPE_FILL },
#define ROM_COPY(srctag,srcoffs,offset,length)      { srctag, (const char *)srcoffs, offset, length, ROMENTRYTYPE_COPY },


/* ----- system BIOS macros ----- */
#define ROM_SYSTEM_BIOS(value,name,description)     { name, description, 0, 0, ROMENTRYTYPE_SYSTEM_BIOS | ROM_BIOS(value+1) },
#define ROM_DEFAULT_BIOS(name)                      { name, NULL, 0, 0, ROMENTRYTYPE_DEFAULT_BIOS },


/* ----- game parameter macro ----- */
#define ROM_PARAMETER(tag, value)                   { tag, value, 0, 0, ROMENTRYTYPE_PARAMETER },

/* ----- disk loading macros ----- */
#define DISK_REGION(tag)                            ROM_REGION(1, tag, ROMREGION_DATATYPEDISK)
#define DISK_IMAGE(name,idx,hash)                   ROMX_LOAD(name, idx, 0, hash, DISK_READWRITE)
#define DISK_IMAGE_READONLY(name,idx,hash)          ROMX_LOAD(name, idx, 0, hash, DISK_READONLY)
#define DISK_IMAGE_READONLY_OPTIONAL(name,idx,hash) ROMX_LOAD(name, idx, 0, hash, DISK_READONLY | ROM_OPTIONAL)


/***************************************************************************
TYPE DEFINITIONS
***************************************************************************/

// ======================> rom_load_manager

class rom_load_manager
{
	class open_chd
	{
	public:
		open_chd(const char *region)
			: m_region(region) { }

		const char *region() const { return m_region.c_str(); }
		chd_file &chd() { return m_diffchd.opened() ? m_diffchd : m_origchd; }
		chd_file &orig_chd() { return m_origchd; }
		chd_file &diff_chd() { return m_diffchd; }

	private:
		std::string         m_region;               /* disk region we came from */
		chd_file            m_origchd;              /* handle to the original CHD */
		chd_file            m_diffchd;              /* handle to the diff CHD */
	};

public:
	// construction/destruction
	rom_load_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }

	/* return the number of warnings we generated */
	int warnings() const { return m_warnings; }

	std::string& software_load_warnings_message() { return m_softwarningstring; }

	/* return the number of BAD_DUMP/NO_DUMP warnings we generated */
	int knownbad() const { return m_knownbad; }

	/* ----- disk handling ----- */

	/* return a pointer to the CHD file associated with the given region */
	chd_file *get_disk_handle(const char *region);

	/* set a pointer to the CHD file associated with the given region */
	int set_disk_handle(const char *region, const char *fullpath);

	void load_software_part_region(device_t &device, software_list_device &swlist, const char *swname, const rom_entry *start_region);

private:
	void determine_bios_rom(device_t *device, const char *specbios);
	void count_roms();
	void fill_random(UINT8 *base, UINT32 length);
	void handle_missing_file(const rom_entry *romp, std::string tried_file_names, chd_error chderr);
	void dump_wrong_and_correct_checksums(const hash_collection &hashes, const hash_collection &acthashes);
	void verify_length_and_hash(const char *name, UINT32 explength, const hash_collection &hashes);
	void display_loading_rom_message(const char *name, bool from_list);
	void display_rom_load_results(bool from_list);
	void region_post_process(const char *rgntag, bool invert);
	int open_rom_file(const char *regiontag, const rom_entry *romp, std::string &tried_file_names, bool from_list);
	int rom_fread(UINT8 *buffer, int length, const rom_entry *parent_region);
	int read_rom_data(const rom_entry *parent_region, const rom_entry *romp);
	void fill_rom_data(const rom_entry *romp);
	void copy_rom_data(const rom_entry *romp);
	void process_rom_entries(const char *regiontag, const rom_entry *parent_region, const rom_entry *romp, device_t *device, bool from_list);
	chd_error open_disk_diff(emu_options &options, const rom_entry *romp, chd_file &source, chd_file &diff_chd);
	void process_disk_entries(const char *regiontag, const rom_entry *parent_region, const rom_entry *romp, const char *locationtag);
	void normalize_flags_for_device(running_machine &machine, const char *rgntag, UINT8 &width, endianness_t &endian);
	void process_region_list();


	// internal state
	running_machine &   m_machine;                  // reference to our machine

	int             m_warnings;           /* warning count during processing */
	int             m_knownbad;           /* BAD_DUMP/NO_DUMP count during processing */
	int             m_errors;             /* error count during processing */

	int             m_romsloaded;         /* current ROMs loaded count */
	int             m_romstotal;          /* total number of ROMs to read */
	UINT32          m_romsloadedsize;     /* total size of ROMs loaded so far */
	UINT32          m_romstotalsize;      /* total size of ROMs to read */

	emu_file *      m_file;               /* current file */
	std::vector<std::unique_ptr<open_chd>> m_chd_list;     /* disks */

	memory_region * m_region;             /* info about current region */

	std::string     m_errorstring;        /* error string */
	std::string     m_softwarningstring;  /* software warning string */
};


/* ----- Helpers ----- */

file_error common_process_file(emu_options &options, const char *location, const char *ext, const rom_entry *romp, emu_file **image_file);
file_error common_process_file(emu_options &options, const char *location, bool has_crc, UINT32 crc, const rom_entry *romp, emu_file **image_file);

/* return pointer to the first ROM region within a source */
const rom_entry *rom_first_region(const device_t &device);

/* return pointer to the next ROM region within a source */
const rom_entry *rom_next_region(const rom_entry *romp);

/* return pointer to the first ROM file within a region */
const rom_entry *rom_first_file(const rom_entry *romp);

/* return pointer to the next ROM file within a region */
const rom_entry *rom_next_file(const rom_entry *romp);

/* return the expected size of a file given the ROM description */
UINT32 rom_file_size(const rom_entry *romp);

/* return the appropriate name for a rom region */
std::string rom_region_name(const device_t &device, const rom_entry *romp);

/* return pointer to the first per-game parameter */
const rom_entry *rom_first_parameter(const device_t &device);

/* return pointer to the next per-game parameter */
const rom_entry *rom_next_parameter(const rom_entry *romp);

/* return the appropriate name for a per-game parameter */
std::string rom_parameter_name(const device_t &device, const rom_entry *romp);

/* return the value for a per-game parameter */
std::string rom_parameter_value(const rom_entry *romp);


/* open a disk image, searching up the parent and loading by checksum */
int open_disk_image(emu_options &options, const game_driver *gamedrv, const rom_entry *romp, chd_file &image_chd, const char *locationtag);

#endif  /* __ROMLOAD_H__ */
