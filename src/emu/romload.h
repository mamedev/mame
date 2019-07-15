// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
/*********************************************************************

    romload.h

    ROM loading functions.
*********************************************************************/

#pragma once

#ifndef MAME_EMU_ROMLOAD_H
#define MAME_EMU_ROMLOAD_H

#include "chd.h"

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


namespace romload {

template <typename T>
class const_entry_iterator
{
protected:
	tiny_rom_entry const *m_data;

	constexpr const_entry_iterator() noexcept : m_data(nullptr) { }
	constexpr const_entry_iterator(tiny_rom_entry const *data) noexcept : m_data(data) { }
	constexpr const_entry_iterator(const_entry_iterator const &) noexcept = default;
	const_entry_iterator(const_entry_iterator &&) noexcept = default;
	const_entry_iterator &operator=(const_entry_iterator const &) noexcept = default;
	const_entry_iterator &operator=(const_entry_iterator &&) noexcept = default;

public:
	typedef T value_type;
	typedef value_type const *pointer;
	typedef value_type const &reference;
	typedef std::ptrdiff_t difference_type;
	typedef std::forward_iterator_tag iterator_category;

	reference operator*() const noexcept { return reinterpret_cast<reference>(*m_data); }
	pointer operator->() const noexcept { return reinterpret_cast<pointer>(m_data); }
};


class file final : tiny_rom_entry
{
private:
	file() = default;
	file(file const &) = delete;
	file &operator=(file const &) = delete;

public:
	// ROM
	constexpr char const *get_name()       const { return name; }
	constexpr u32         get_offset()     const { return offset; }
	constexpr u32         get_length()     const { return length; }
	constexpr u32         get_flags()      const { return flags; }
	constexpr char const *get_hashdata()   const { return hashdata; }
	constexpr bool        is_optional()    const { return (flags & ROM_OPTIONALMASK) == ROM_OPTIONAL; }
	constexpr u32         get_groupsize()  const { return ((flags & ROM_GROUPMASK) >> 8) + 1; }
	constexpr u32         get_skipcount()  const { return (flags & ROM_SKIPMASK) >> 12; }
	constexpr bool        is_reversed()    const { return (flags & ROM_REVERSEMASK) == ROM_REVERSE; }
	constexpr u32         get_bitwidth()   const { return (flags & ROM_BITWIDTHMASK) ? ((flags & ROM_BITWIDTHMASK) >> 16) : 8; }
	constexpr u32         get_bitshift()   const { return (flags & ROM_BITSHIFTMASK) >> 20; }
	constexpr bool        inherits_flags() const { return (flags & ROM_INHERITFLAGSMASK) == ROM_INHERITFLAGS; }
	constexpr u32         get_bios_flags() const { return (flags & ROM_BIOSFLAGSMASK) >> 24; }

	// disk
	constexpr u32         get_index()      const { return offset; }
	constexpr bool        is_readonly()    const { return (flags & DISK_READONLYMASK) == DISK_READONLY; }
};

class files
{
private:
	tiny_rom_entry const *m_data;

public:
	class const_iterator : public const_entry_iterator<file>
	{
	private:
		friend class files;

		constexpr const_iterator(tiny_rom_entry const *data) noexcept : const_entry_iterator<file>(data) { }

	public:
		constexpr const_iterator() noexcept = default;
		constexpr const_iterator(const_iterator const &) noexcept = default;
		const_iterator(const_iterator &&) noexcept = default;
		const_iterator &operator=(const_iterator const &) noexcept = default;
		const_iterator &operator=(const_iterator &&) noexcept = default;

		const_iterator &operator++() noexcept
		{
			while (m_data)
			{
				++m_data;
				if (ROMENTRY_ISFILE(m_data))
					break;
				else if (ROMENTRY_ISREGIONEND(m_data))
					m_data = nullptr;
			}
			return *this;
		}

		const_iterator operator++(int) noexcept { const_iterator result(*this); operator++(); return result; }

		constexpr bool operator==(const_iterator const &rhs) const noexcept { return m_data == rhs.m_data; }
		constexpr bool operator!=(const_iterator const &rhs) const noexcept { return m_data != rhs.m_data; }
	};

	files(tiny_rom_entry const *data) : m_data(data)
	{
		while (m_data && !ROMENTRY_ISFILE(m_data))
		{
			if (ROMENTRY_ISREGIONEND(m_data))
				m_data = nullptr;
			else
				++m_data;
		}
	}

	const_iterator begin() const { return const_iterator(m_data); }
	const_iterator cbegin() const { return const_iterator(m_data); }
	const_iterator end() const { return const_iterator(nullptr); }
	const_iterator cend() const { return const_iterator(nullptr); }
};


class region final : tiny_rom_entry
{
private:
	region() = default;
	region(region const &) = delete;
	region &operator=(region const &) = delete;

public:
	constexpr char const *get_tag()         const { return name; }
	constexpr u32         get_length()      const { return length; }
	constexpr u32         get_width()       const { return 8 << ((flags & ROMREGION_WIDTHMASK) >> 8); }
	constexpr bool        is_littleendian() const { return (flags & ROMREGION_ENDIANMASK) == ROMREGION_LE; }
	constexpr bool        is_bigendian()    const { return (flags & ROMREGION_ENDIANMASK) == ROMREGION_BE; }
	constexpr bool        is_inverted()     const { return (flags & ROMREGION_INVERTMASK) == ROMREGION_INVERT; }
	constexpr bool        is_erase()        const { return (flags & ROMREGION_ERASEMASK) == ROMREGION_ERASE; }
	constexpr u32         get_eraseval()    const { return (flags & ROMREGION_ERASEVALMASK) >> 16; }
	constexpr u32         get_datatype()    const { return flags & ROMREGION_DATATYPEMASK; }
	constexpr bool        is_romdata()      const { return get_datatype() == ROMREGION_DATATYPEROM; }
	constexpr bool        is_diskdata()     const { return get_datatype() == ROMREGION_DATATYPEDISK; }

	files get_files() const { return files(static_cast<tiny_rom_entry const *>(this) + 1); }
};

class regions
{
private:
	tiny_rom_entry const *m_data;

public:
	class const_iterator : public const_entry_iterator<region>
	{
	private:
		friend class regions;

		constexpr const_iterator(tiny_rom_entry const *data) noexcept : const_entry_iterator<region>(data) { }

	public:
		constexpr const_iterator() noexcept = default;
		constexpr const_iterator(const_iterator const &) noexcept = default;
		const_iterator(const_iterator &&) noexcept = default;
		const_iterator &operator=(const_iterator const &) noexcept = default;
		const_iterator &operator=(const_iterator &&) noexcept = default;

		const_iterator &operator++() noexcept
		{
			while (m_data)
			{
				++m_data;
				if (ROMENTRY_ISREGION(m_data))
					break;
				else if (ROMENTRY_ISEND(m_data))
					m_data = nullptr;
			}
			return *this;
		}

		const_iterator operator++(int) noexcept { const_iterator result(*this); operator++(); return result; }

		constexpr bool operator==(const_iterator const &rhs) const noexcept { return m_data == rhs.m_data; }
		constexpr bool operator!=(const_iterator const &rhs) const noexcept { return m_data != rhs.m_data; }
	};

	regions(tiny_rom_entry const *data) : m_data(data)
	{
		while (m_data && !ROMENTRY_ISREGION(m_data))
		{
			if (ROMENTRY_ISEND(m_data))
				m_data = nullptr;
			else
				++m_data;
		}
	}

	const_iterator begin() const { return const_iterator(m_data); }
	const_iterator cbegin() const { return const_iterator(m_data); }
	const_iterator end() const { return const_iterator(nullptr); }
	const_iterator cend() const { return const_iterator(nullptr); }
};


class system_bios final : tiny_rom_entry
{
private:
	system_bios() = default;
	system_bios(system_bios const &) = delete;
	system_bios &operator=(system_bios const &) = delete;

public:
	constexpr u32         get_value()       const { return (flags & ROM_BIOSFLAGSMASK) >> 24; }
	constexpr char const *get_name()        const { return name; }
	constexpr char const *get_description() const { return hashdata; }
};

class system_bioses
{
private:
	tiny_rom_entry const *m_data;

public:
	class const_iterator : public const_entry_iterator<system_bios>
	{
	private:
		friend class system_bioses;

		constexpr const_iterator(tiny_rom_entry const *data) noexcept : const_entry_iterator<system_bios>(data) { }

	public:
		constexpr const_iterator() noexcept = default;
		constexpr const_iterator(const_iterator const &) noexcept = default;
		const_iterator(const_iterator &&) noexcept = default;
		const_iterator &operator=(const_iterator const &) noexcept = default;
		const_iterator &operator=(const_iterator &&) noexcept = default;

		const_iterator &operator++() noexcept
		{
			while (m_data)
			{
				++m_data;
				if (ROMENTRY_ISSYSTEM_BIOS(m_data))
					break;
				else if (ROMENTRY_ISEND(m_data))
					m_data = nullptr;
			}
			return *this;
		}

		const_iterator operator++(int) noexcept { const_iterator result(*this); operator++(); return result; }

		constexpr bool operator==(const_iterator const &rhs) const noexcept { return m_data == rhs.m_data; }
		constexpr bool operator!=(const_iterator const &rhs) const noexcept { return m_data != rhs.m_data; }
	};

	system_bioses(tiny_rom_entry const *data) : m_data(data)
	{
		while (m_data && !ROMENTRY_ISSYSTEM_BIOS(m_data))
		{
			if (ROMENTRY_ISEND(m_data))
				m_data = nullptr;
			else
				++m_data;
		}
	}

	const_iterator begin() const { return const_iterator(m_data); }
	const_iterator cbegin() const { return const_iterator(m_data); }
	const_iterator end() const { return const_iterator(nullptr); }
	const_iterator cend() const { return const_iterator(nullptr); }
};


class default_bios final : tiny_rom_entry
{
private:
	default_bios() = default;
	default_bios(default_bios const &) = delete;
	default_bios &operator=(default_bios const &) = delete;

public:
	constexpr char const *get_name() const { return name; }
};


class entries
{
private:
	tiny_rom_entry const *m_data;

public:
	constexpr entries(tiny_rom_entry const *data) : m_data(data) { }

	regions get_regions() const { return regions(m_data); }
	system_bioses get_system_bioses() const { return system_bioses(m_data); }
};

} // namespace romload


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
	void normalize_flags_for_device(const char *rgntag, u8 &width, endianness_t &endian);
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
