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

#ifndef MAME_EMU_ROMLOAD_H
#define MAME_EMU_ROMLOAD_H

#include "chd.h"
#include "romentry.h"

#include <type_traits>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


/***************************************************************************
    MACROS
***************************************************************************/

template <typename T> inline std::enable_if_t<!std::is_pointer<T>::value, T const &> ROMENTRY_UNWRAP(T const &r) { return r; }
template <typename T> inline T const &ROMENTRY_UNWRAP(T const *r) { return *r; }

/* ----- per-entry macros ----- */
template <typename T> inline u32  ROMENTRY_GETTYPE(T const &r)         { return ROMENTRY_UNWRAP(r).get_flags() & ROMENTRY_TYPEMASK; }
template <typename T> inline bool ROMENTRY_ISSPECIAL(T const &r)       { return ROMENTRY_GETTYPE(r) != ROMENTRYTYPE_ROM; }
template <typename T> inline bool ROMENTRY_ISFILE(T const &r)          { return ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_ROM; }
template <typename T> inline bool ROMENTRY_ISREGION(T const &r)        { return ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_REGION; }
template <typename T> inline bool ROMENTRY_ISEND(T const &r)           { return ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_END; }
template <typename T> inline bool ROMENTRY_ISRELOAD(T const &r)        { return ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_RELOAD; }
template <typename T> inline bool ROMENTRY_ISCONTINUE(T const &r)      { return ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_CONTINUE; }
template <typename T> inline bool ROMENTRY_ISFILL(T const &r)          { return ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_FILL; }
template <typename T> inline bool ROMENTRY_ISCOPY(T const &r)          { return ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_COPY; }
template <typename T> inline bool ROMENTRY_ISIGNORE(T const &r)        { return ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_IGNORE; }
template <typename T> inline bool ROMENTRY_ISSYSTEM_BIOS(T const &r)   { return ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_SYSTEM_BIOS; }
template <typename T> inline bool ROMENTRY_ISDEFAULT_BIOS(T const &r)  { return ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_DEFAULT_BIOS; }
template <typename T> inline bool ROMENTRY_ISPARAMETER(T const &r)     { return ROMENTRY_GETTYPE(r) == ROMENTRYTYPE_PARAMETER; }
template <typename T> inline bool ROMENTRY_ISREGIONEND(T const &r)     { return ROMENTRY_ISREGION(r) || ROMENTRY_ISPARAMETER(r) || ROMENTRY_ISEND(r); }

/* ----- per-region macros ----- */
#define ROMREGION_GETTAG(r)         ((r)->name().c_str())
template <typename T> inline u32  ROMREGION_GETLENGTH(T const &r)      { return ROMENTRY_UNWRAP(r).get_length(); }
template <typename T> inline u32  ROMREGION_GETFLAGS(T const &r)       { return ROMENTRY_UNWRAP(r).get_flags(); }
template <typename T> inline u32  ROMREGION_GETWIDTH(T const &r)       { return 8 << ((ROMREGION_GETFLAGS(r) & ROMREGION_WIDTHMASK) >> 8); }
template <typename T> inline bool ROMREGION_ISLITTLEENDIAN(T const &r) { return (ROMREGION_GETFLAGS(r) & ROMREGION_ENDIANMASK) == ROMREGION_LE; }
template <typename T> inline bool ROMREGION_ISBIGENDIAN(T const &r)    { return (ROMREGION_GETFLAGS(r) & ROMREGION_ENDIANMASK) == ROMREGION_BE; }
template <typename T> inline bool ROMREGION_ISINVERTED(T const &r)     { return (ROMREGION_GETFLAGS(r) & ROMREGION_INVERTMASK) == ROMREGION_INVERT; }
template <typename T> inline bool ROMREGION_ISERASE(T const &r)        { return (ROMREGION_GETFLAGS(r) & ROMREGION_ERASEMASK) == ROMREGION_ERASE; }
template <typename T> inline u32  ROMREGION_GETERASEVAL(T const &r)    { return (ROMREGION_GETFLAGS(r) & ROMREGION_ERASEVALMASK) >> 16; }
template <typename T> inline u32  ROMREGION_GETDATATYPE(T const &r)    { return ROMREGION_GETFLAGS(r) & ROMREGION_DATATYPEMASK; }
template <typename T> inline bool ROMREGION_ISROMDATA(T const &r)      { return ROMREGION_GETDATATYPE(r) == ROMREGION_DATATYPEROM; }
template <typename T> inline bool ROMREGION_ISDISKDATA(T const &r)     { return ROMREGION_GETDATATYPE(r) == ROMREGION_DATATYPEDISK; }


/* ----- per-ROM macros ----- */
#define ROM_GETNAME(r)              ((r)->name().c_str())
#define ROM_SAFEGETNAME(r)          (ROMENTRY_ISFILL(r) ? "fill" : ROMENTRY_ISCOPY(r) ? "copy" : ROM_GETNAME(r))
template <typename T> inline u32  ROM_GETOFFSET(T const &r)            { return ROMENTRY_UNWRAP(r).get_offset(); }
template <typename T> inline u32  ROM_GETLENGTH(T const &r)            { return ROMENTRY_UNWRAP(r).get_length(); }
template <typename T> inline u32  ROM_GETFLAGS(T const &r)             { return ROMENTRY_UNWRAP(r).get_flags(); }
#define ROM_GETHASHDATA(r)          ((r)->hashdata().c_str())
template <typename T> inline bool ROM_ISOPTIONAL(T const &r)           { return (ROM_GETFLAGS(r) & ROM_OPTIONALMASK) == ROM_OPTIONAL; }
template <typename T> inline u32  ROM_GETGROUPSIZE(T const &r)         { return ((ROM_GETFLAGS(r) & ROM_GROUPMASK) >> 8) + 1; }
template <typename T> inline u32  ROM_GETSKIPCOUNT(T const &r)         { return (ROM_GETFLAGS(r) & ROM_SKIPMASK) >> 12; }
template <typename T> inline bool ROM_ISREVERSED(T const &r)           { return (ROM_GETFLAGS(r) & ROM_REVERSEMASK) == ROM_REVERSE; }
template <typename T> inline u32  ROM_GETBITWIDTH(T const &r)          { return (ROM_GETFLAGS(r) & ROM_BITWIDTHMASK) ? ((ROM_GETFLAGS(r) & ROM_BITWIDTHMASK) >> 16) : 8; }
template <typename T> inline u32  ROM_GETBITSHIFT(T const &r)          { return (ROM_GETFLAGS(r) & ROM_BITSHIFTMASK) >> 20; }
template <typename T> inline bool ROM_INHERITSFLAGS(T const &r)        { return (ROM_GETFLAGS(r) & ROM_INHERITFLAGSMASK) == ROM_INHERITFLAGS; }
template <typename T> inline u32  ROM_GETBIOSFLAGS(T const &r)         { return (ROM_GETFLAGS(r) & ROM_BIOSFLAGSMASK) >> 24; }


/* ----- per-disk macros ----- */
template <typename T> inline u32  DISK_GETINDEX(T const &r)            { return ROMENTRY_UNWRAP(r).get_offset(); }
template <typename T> inline bool DISK_ISREADONLY(T const &r)          { return (ROM_GETFLAGS(r) & DISK_READONLYMASK) == DISK_READONLY; }


/* ----- start/stop macros ----- */
#define ROM_NAME(name)                              rom_##name
#define ROM_START(name)                             static const tiny_rom_entry ROM_NAME(name)[] = {
#define ROM_END                                     { nullptr, nullptr, 0, 0, ROMENTRYTYPE_END } };


/* ----- ROM region macros ----- */
#define ROM_REGION(length,tag,flags)                { tag, nullptr, 0, length, ROMENTRYTYPE_REGION | (flags) },
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
#define ROM_LOAD_NIB_HIGH(name,offset,length,hash)      ROMX_LOAD(name, offset, length, hash, ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI)
#define ROM_LOAD_NIB_LOW(name,offset,length,hash)       ROMX_LOAD(name, offset, length, hash, ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
#define ROM_LOAD16_BYTE(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_SKIP(1))
#define ROM_LOAD16_WORD(name,offset,length,hash)        ROM_LOAD(name, offset, length, hash)
#define ROM_LOAD16_WORD_SWAP(name,offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE)
#define ROM_LOAD32_BYTE(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_SKIP(3))
#define ROM_LOAD32_WORD(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(2))
#define ROM_LOAD32_WORD_SWAP(name,offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))
#define ROM_LOAD32_DWORD(name,offset,length,hash)       ROMX_LOAD(name, offset, length, hash, ROM_GROUPDWORD)
#define ROM_LOAD64_WORD(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(6))
#define ROM_LOAD64_WORD_SWAP(name,offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6))
#define ROM_LOAD64_DWORD_SWAP(name,offset,length,hash)  ROMX_LOAD(name, offset, length, hash, ROM_GROUPDWORD | ROM_REVERSE | ROM_SKIP(4))


/* ----- ROM_RELOAD related macros ----- */
#define ROM_RELOAD(offset,length)                   { nullptr, nullptr, offset, length, ROMENTRYTYPE_RELOAD | ROM_INHERITFLAGS },
#define ROM_RELOAD_PLAIN(offset,length)             { nullptr, nullptr, offset, length, ROMENTRYTYPE_RELOAD },

/* ----- additional ROM-related macros ----- */
#define ROM_CONTINUE(offset,length)                 { nullptr,  nullptr,                 (offset), (length), ROMENTRYTYPE_CONTINUE | ROM_INHERITFLAGS },
#define ROM_IGNORE(length)                          { nullptr,  nullptr,                 0,        (length), ROMENTRYTYPE_IGNORE | ROM_INHERITFLAGS },
#define ROM_FILL(offset,length,value)               { nullptr,  (const char *)(value),   (offset), (length), ROMENTRYTYPE_FILL },
#define ROMX_FILL(offset,length,value,flags)        { nullptr,  (const char *)(value),   (offset), (length), ROMENTRYTYPE_FILL | flags },
#define ROM_COPY(srctag,srcoffs,offset,length)      { (srctag), (const char *)(srcoffs), (offset), (length), ROMENTRYTYPE_COPY },


/* ----- system BIOS macros ----- */
#define ROM_SYSTEM_BIOS(value,name,description)     { name, description, 0, 0, ROMENTRYTYPE_SYSTEM_BIOS | ROM_BIOS(value+1) },
#define ROM_DEFAULT_BIOS(name)                      { name, nullptr,     0, 0, ROMENTRYTYPE_DEFAULT_BIOS },


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
		open_chd(const char *region) : m_region(region) { }

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
	void determine_bios_rom(device_t &device, const char *specbios);
	void count_roms();
	void fill_random(u8 *base, u32 length);
	void handle_missing_file(const rom_entry *romp, std::string tried_file_names, chd_error chderr);
	void dump_wrong_and_correct_checksums(const util::hash_collection &hashes, const util::hash_collection &acthashes);
	void verify_length_and_hash(const char *name, u32 explength, const util::hash_collection &hashes);
	void display_loading_rom_message(const char *name, bool from_list);
	void display_rom_load_results(bool from_list);
	void region_post_process(const char *rgntag, bool invert);
	int open_rom_file(const char *regiontag, const rom_entry *romp, std::string &tried_file_names, bool from_list);
	int rom_fread(u8 *buffer, int length, const rom_entry *parent_region);
	int read_rom_data(const rom_entry *parent_region, const rom_entry *romp);
	void fill_rom_data(const rom_entry *romp);
	void copy_rom_data(const rom_entry *romp);
	void process_rom_entries(const char *regiontag, const rom_entry *parent_region, const rom_entry *romp, device_t *device, bool from_list);
	chd_error open_disk_diff(emu_options &options, const rom_entry *romp, chd_file &source, chd_file &diff_chd);
	void process_disk_entries(const char *regiontag, const rom_entry *parent_region, const rom_entry *romp, const char *locationtag);
	void normalize_flags_for_device(running_machine &machine, const char *rgntag, u8 &width, endianness_t &endian);
	void process_region_list();


	// internal state
	running_machine &   m_machine;            // reference to our machine

	int                 m_warnings;           // warning count during processing
	int                 m_knownbad;           // BAD_DUMP/NO_DUMP count during processing
	int                 m_errors;             // error count during processing

	int                 m_romsloaded;         // current ROMs loaded count
	int                 m_romstotal;          // total number of ROMs to read
	u32                 m_romsloadedsize;     // total size of ROMs loaded so far
	u32                 m_romstotalsize;      // total size of ROMs to read

	std::unique_ptr<emu_file>  m_file;               /* current file */
	std::vector<std::unique_ptr<open_chd>> m_chd_list;     /* disks */

	memory_region *     m_region;             // info about current region

	std::string         m_errorstring;        // error string
	std::string         m_softwarningstring;  // software warning string
};


/* ----- Helpers ----- */

std::unique_ptr<emu_file> common_process_file(emu_options &options, const char *location, bool has_crc, u32 crc, const rom_entry *romp, osd_file::error &filerr);

/* return pointer to the first ROM region within a source */
const rom_entry *rom_first_region(const device_t &device);

/* return pointer to the next ROM region within a source */
const rom_entry *rom_next_region(const rom_entry *romp);

/* return pointer to the first ROM file within a region */
const rom_entry *rom_first_file(const rom_entry *romp);

/* return pointer to the next ROM file within a region */
const rom_entry *rom_next_file(const rom_entry *romp);

/* return the expected size of a file given the ROM description */
u32 rom_file_size(const rom_entry *romp);

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

// builds a rom_entry vector from a tiny_rom_entry array
std::vector<rom_entry> rom_build_entries(const tiny_rom_entry *tinyentries);


/* open a disk image, searching up the parent and loading by checksum */
int open_disk_image(emu_options &options, const game_driver *gamedrv, const rom_entry *romp, chd_file &image_chd, const char *locationtag);

#endif  // MAME_EMU_ROMLOAD_H
