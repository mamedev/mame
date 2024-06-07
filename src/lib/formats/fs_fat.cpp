// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Wilbert Pol
/***************************************************************************

    fs_fat.cpp

    PC FAT disk images

    Current Limitations:
    - Only supports floppy disks
    - No FAT32 support
    - No Long Filenames Support

    Removal of files is untested; floptool does not have a command to delete
    a file.

*****************************************************************************

  Master boot record format:

  Offset  Length  Description
  ------  ------  -----------
       0     446  Boot machine code
     446      16  Partion #1 info
     462      16  Partion #2 info
     478      16  Partion #3 info
     494      16  Partion #4 info
     510       2  Magic bytes (0x55 0xAA)


  Partition info format:

  Offset  Length  Description
  ------  ------  -----------
       0       1  Active byte (0x80=active 0x00=inactive)
       1       1  Starting head
       2       1  Starting sector (bits 5-0) and high bits of starting track (bits 6-5)
       3       1  Low bits of starting track
       4       1  Partition type:
                       0x00     Unused
                       0x?1     FAT12   (0-15 MB)
                       0x?2     XENIX
                       0x?4     FAT16   (16-32 MB)
                       0x?6     FAT16`  (32 MB-2 GB)
                       0x?7     HPFS or NTFS
                       0x?A     Boot Manager
                       0x?B     FAT32   (512 MB-2 TB)
                       0x?C     FAT32   (512 MB-2 TB LBA)
                       0x1?     OS/2 Boot manager/Win95 hidden
                       0xC?     DR-DOS secured partition
                       0xD?     Multiuser DOS secured partition
                       0xE?     SpeedStor extended partition
       5       1  Ending head
       6       1  Ending sector (bits 5-0) and high bits of ending track (bits 6-5)
       7       1  Low bits of ending track
       8       4  Sector index of beginning of partition
      12       4  Total sectors in partition


  Boot sector format:

  Offset  Length  Description
  ------  ------  -----------
       0       3  Jump instruction (to skip over header on boot)
       3       8  OEM Name
      11       2  Bytes per sector
      13       1  Sectors per cluster
      14       2  Reserved sector count (including boot sector)
      16       1  Number of FATs (file allocation tables)
      17       2  Number of root directory entries
      19       2  Total sectors (0 if 0x10000 or more)
      21       1  Media descriptor
      22       2  Sectors per FAT
      24       2  Sectors per track
      26       2  Number of heads
      28       4  Hidden sectors
      32       4  Total sectors (0 if less than 0x10000)
      36       1  Physical drive number
      37       1  Current head
      38       1  Signature
      39       4  ID
      43      11  Volume Label
      54       8  FAT file system type
      62     448  Boot machine code
     510       2  Magic bytes (0x55 0xAA)

  For more information:
    http://support.microsoft.com/kb/q140418/


  Directory Entry Format:

  Offset  Length  Description
  ------  ------  -----------
       0       8  DOS File Name (padded with spaces)
       8       3  DOS File Extension (padded with spaces)
      11       1  File Attributes
      12       2  Unknown
      14       4  Time of Creation
      18       2  Last Access Time
      20       2  EA-Index (OS/2 stuff)
      22       4  Last Modified Time
      26       2  First Cluster
      28       4  File Size


  Dates and times are stored in separate words; when together, the time is
  first and the date is second.

    Time:
        bits 15-11      Hour
        bits 10- 5      Minute
        bits  4- 0      Second / 2

    Date:
        bits 15- 9      Year - 1980
        bits  8- 5      Month
        bits  4- 0      Day

  LFN Entry Format:

  Offset  Length  Description
  ------  ------  -----------
       0       1  Sequence Number (bit 6 is set on highest sequence)
       1      10  Name characters (five UTF-16LE chars)
      11       1  Attributes (always 0x0F)
      12       1  Reserved (always 0x00)
      13       1  Checksum of short filename entry
      14      12  Name characters (six UTF-16LE chars)
      26       2  Entry Cluster (always 0x00)
      28       4  Name characters (two UTF-16LE chars)

  Valid characters in DOS file names:
    - Upper case letters A-Z
    - Numbers 0-9
    - Space (though there is no way to identify a trailing space)
    - ! # $ % & ( ) - @ ^ _ ` { } ~
    - Characters 128-255 (though the code page is indeterminate)

  For more information:
    http://en.wikipedia.org/wiki/File_Allocation_Table

****************************************************************************/

#include "fs_fat.h"
#include "fsblk.h"
#include "pc_dsk.h"

#include "strformat.h"

#include <optional>
#include <regex>
#include <string_view>


using namespace fs;

const fs::pc_fat_image fs::PC_FAT;

//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

namespace {

// ======================> directory_entry

class directory_entry
{
public:
	static constexpr int SIZE = 32;
	static constexpr int OFFSET_FNAME = 0;
	static constexpr int FNAME_LENGTH = 11;
	static constexpr int OFFSET_ATTRIBUTES = 11;
	static constexpr int OFFSET_CREATE_DATETIME = 14;
	static constexpr int OFFSET_START_CLUSTER_HI = 20;
	static constexpr int OFFSET_MODIFIED_DATETIME = 22;
	static constexpr int OFFSET_START_CLUSTER = 26;
	static constexpr int OFFSET_FILE_SIZE = 28;
	static constexpr u8 DELETED_FILE_MARKER = 0xe5;
	static constexpr u8 ATTR_READ_ONLY = 0x01;
	static constexpr u8 ATTR_HIDDEN = 0x02;
	static constexpr u8 ATTR_SYSTEM = 0x04;
	static constexpr u8 ATTR_VOLUME_LABEL = 0x08;
	static constexpr u8 ATTR_DIRECTORY = 0x10;
	static constexpr u8 ATTR_ARCHIVE = 0x20;

	directory_entry(const fsblk_t::block_t &block, u32 offset)
		: m_block(block)
		, m_offset(offset)
	{
	}

	std::string_view raw_stem() const   { return std::string_view((const char *) &m_block.rodata()[m_offset + OFFSET_FNAME], 8); }
	std::string_view raw_ext() const    { return std::string_view((const char *) &m_block.rodata()[m_offset + OFFSET_FNAME + 8], 3); }
	u8 attributes() const               { return m_block.r8(m_offset + OFFSET_ATTRIBUTES); }
	u32 raw_create_datetime() const     { return m_block.r32l(m_offset + OFFSET_CREATE_DATETIME); }
	u32 raw_modified_datetime() const   { return m_block.r32l(m_offset + OFFSET_MODIFIED_DATETIME); }
	u32 start_cluster() const           { return ((u32)m_block.r16l(m_offset + OFFSET_START_CLUSTER_HI)) << 16 | m_block.r16l(m_offset + OFFSET_START_CLUSTER); }
	u32 file_size() const               { return m_block.r32l(m_offset + OFFSET_FILE_SIZE); }

	bool is_read_only() const           { return (attributes() & 0x01) != 0x00; }
	bool is_hidden() const              { return (attributes() & 0x02) != 0x00; }
	bool is_system() const              { return (attributes() & 0x04) != 0x00; }
	bool is_volume_label() const        { return (attributes() & 0x08) != 0x00; }
	bool is_long_file_name() const      { return attributes() == 0x0f; }
	bool is_subdirectory() const        { return (attributes() & 0x10) != 0x00; }
	bool is_archive() const             { return (attributes() & 0x20) != 0x00; }
	bool is_deleted() const             { return m_block.r8(m_offset) == DELETED_FILE_MARKER; }

	std::string name() const;
	meta_data metadata() const;

	void set_file_size(u32 file_size)   { m_block.w32l(m_offset + OFFSET_FILE_SIZE, file_size); }
	void set_raw_modified_datetime(u32 datetime) { m_block.w32l(m_offset + OFFSET_MODIFIED_DATETIME, datetime); }
	void mark_deleted()                 { m_block.w8(m_offset + OFFSET_FNAME, DELETED_FILE_MARKER); }

private:
	fsblk_t::block_t    m_block;
	u32                 m_offset;
};

// ======================> directory_span

class directory_span
{
public:
	typedef std::unique_ptr<directory_span> ptr;

	directory_span() = default;
	virtual ~directory_span() = default;

	virtual std::vector<u32> get_directory_sectors() const = 0;
};


// ======================> directory_entry

class impl : public filesystem_t
{
public:
	// ctor/dtor
	impl(fsblk_t &blockdev, fsblk_t::block_t &&boot_sector_block, std::vector<u8> &&file_allocation_table, u32 starting_sector, u32 sector_count, u16 reserved_sector_count, u8 bits_per_fat_entry);
	virtual ~impl() = default;

	// accessors
	fsblk_t &blockdev() { return m_blockdev; }
	u16 bytes_per_sector() const { return m_bytes_per_sector; }
	u32 dirents_per_sector() const { return bytes_per_sector() / directory_entry::SIZE; }

	// virtuals
	virtual meta_data volume_metadata() override;
	virtual std::pair<err_t, meta_data> metadata(const std::vector<std::string> &path) override;
	virtual std::pair<err_t, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;
	virtual std::pair<err_t, std::vector<u8>> file_read(const std::vector<std::string> &path) override;
	virtual err_t file_create(const std::vector<std::string> &path, const meta_data &meta) override;
	virtual err_t file_write(const std::vector<std::string> &path, const std::vector<u8> &data) override;
	virtual err_t remove(const std::vector<std::string> &path) override;

	// methods
	std::vector<u32> get_sectors_from_fat(const directory_entry &dirent) const;

	// Boot sector settings
	static constexpr u32 OFFSET_BYTES_PER_SECTOR = 0x0b;
	static constexpr u32 OFFSET_CLUSTER_SECTOR_COUNT = 0x0d;
	static constexpr u32 OFFSET_RESERVED_SECTOR_COUNT = 0x0e;
	static constexpr u32 OFFSET_FAT_COUNT = 0x10;
	static constexpr u32 OFFSET_DIRECTORY_ENTRY_COUNT = 0x11;
	static constexpr u32 OFFSET_FAT_SECTOR_COUNT = 0x16;

private:
	static constexpr u32 FIRST_VALID_CLUSTER = 2;

	fsblk_t::block_t                m_boot_sector_block;
	std::vector<u8>                 m_file_allocation_table;
	u32                             m_starting_sector;
	u32                             m_sector_count;
	u16                             m_reserved_sector_count;
	u16                             m_bytes_per_sector;
	u16                             m_root_directory_size;
	u16                             m_sectors_per_cluster;
	u8                              m_fat_count;
	u16                             m_fat_sector_count;
	u8                              m_bits_per_fat_entry;
	u32                             m_last_cluster_indicator;
	u32                             m_last_valid_cluster;

	// methods
	std::optional<directory_entry> find_entity(const std::vector<std::string> &path) const;
	directory_span::ptr find_directory(std::vector<std::string>::const_iterator path_begin, std::vector<std::string>::const_iterator path_end) const;
	std::optional<directory_entry> find_child(const directory_span &current_dir, std::string_view target) const;
	template <typename T> void iterate_directory_entries(const directory_span &dir, T &&callback) const;
	bool is_valid_short_filename(std::string const &filename);
	err_t build_direntry_filename(std::string const &filename, std::string &fname);
	err_t file_create_root(std::string &fname, u8 attributes = 0);
	err_t file_create_directory(directory_entry &dirent, std::string &fname, u8 attributes = 0);
	err_t file_create_sector(u32 sector, std::string &fname, u8 attributes);
	err_t initialize_directory(u32 directory_cluster, u32 parent_cluster);
	err_t initialize_directory_entry(fsblk_t::block_t &dirblk, u32 offset, const std::string_view &fname, u8 attributes, u32 start_cluster);
	err_t free_clusters(u32 start_cluster);
	void clear_cluster_sectors(u32 cluster, u8 fill_byte);
	u32 first_cluster_sector(u32 cluster);
	u32 get_next_cluster(u32 cluster);
	void set_next_cluster(u32 cluster, u32 next_cluster);
	u32 find_free_cluster();

};


// ======================> root_directory_span

class root_directory_span : public directory_span
{
public:
	root_directory_span(const impl &fs, u32 first_sector, u16 directory_entry_count);

	virtual std::vector<u32> get_directory_sectors() const override;

private:
	const impl &    m_fs;
	u32             m_first_sector;
	u16             m_directory_entry_count;
};


// ======================> subdirectory_span

class subdirectory_span : public directory_span
{
public:
	subdirectory_span(const impl &fs, directory_entry &&dirent);

	virtual std::vector<u32> get_directory_sectors() const override;

private:
	const impl &    m_fs;
	directory_entry m_dirent;
};


}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  validate_filename
//-------------------------------------------------

namespace {
bool validate_filename(std::string_view name)
{
	auto const is_invalid_filename_char = [] (char ch)
	{
		return ch == '\0' || strchr("\\/:*?\"<>|", ch);
	};

	return !name.empty()
		&& std::find_if(name.begin(), name.end(), is_invalid_filename_char) == name.end();
}


//-------------------------------------------------
//  decode_fat_datetime
//-------------------------------------------------

util::arbitrary_datetime decode_fat_datetime(u32 dt)
{
	util::arbitrary_datetime result;
	memset(&result, 0, sizeof(result));

	result.year         = ((dt >> 25)       & 0x7f) + 1980;
	result.month        = (dt >> 21)        & 0x0f;
	result.day_of_month = (dt >> 16)        & 0x1f;
	result.hour         = (dt >> 11)        & 0x1f;
	result.minute       = (dt >>  5)        & 0x3f;
	result.second       = ((dt >>  0)       & 0x1f) * 2;
	return result;
}

u32 encode_now_fat_datetime()
{
	auto now = util::arbitrary_datetime::now();

	return u32((((now.year - 1980) & 0x7f) << 25) |
		((now.month & 0x0f) << 21) |
		((now.day_of_month & 0x1f) << 16) |
		((now.hour & 0x1f) << 11) |
		((now.minute & 0x3f) << 5) |
		((now.second >> 1) & 0x1f));
}


}

//-------------------------------------------------
//  fat_image::can_format
//-------------------------------------------------

bool fs::fat_image::can_format() const
{
	return false;
}


//-------------------------------------------------
//  fat_image::can_read
//-------------------------------------------------

bool fs::fat_image::can_read() const
{
	return true;
}


//-------------------------------------------------
//  fat_image::can_write
//-------------------------------------------------

bool fs::fat_image::can_write() const
{
	return true;
}


//-------------------------------------------------
//  fat_image::has_rsrc
//-------------------------------------------------

bool fs::fat_image::has_rsrc() const
{
	return false;
}


//-------------------------------------------------
//  fat_image::directory_separator
//-------------------------------------------------

char fs::fat_image::directory_separator() const
{
	return '\\';
}


//-------------------------------------------------
//  fat_image::volume_meta_description
//-------------------------------------------------

std::vector<meta_description> fs::fat_image::volume_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_name::name, "UNTITLED",   false, [] (const meta_value &m) { return validate_filename(m.as_string()); }, "Volume name");
	results.emplace_back(meta_name::oem_name, "",       false, [] (const meta_value &m) { return m.as_string().size() <= 8; }, "OEM name, up to 8 characters");
	return results;
}


//-------------------------------------------------
//  fat_image::file_meta_description
//-------------------------------------------------

std::vector<meta_description> fs::fat_image::file_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_name::name, "", false, [] (const meta_value &m) { return validate_filename(m.as_string()); }, "File name");
	results.emplace_back(meta_name::creation_date, util::arbitrary_datetime::now(), false, nullptr, "Creation time");
	results.emplace_back(meta_name::modification_date, util::arbitrary_datetime::now(), false, nullptr, "Modification time");
	results.emplace_back(meta_name::length, 0, true, nullptr, "Size of the file in bytes");
	return results;
}


//-------------------------------------------------
//  fat_image::directory_meta_description
//-------------------------------------------------

std::vector<meta_description> fs::fat_image::directory_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_name::name, "", false, [] (const meta_value &m) { return validate_filename(m.as_string()); }, "File name");
	results.emplace_back(meta_name::creation_date, util::arbitrary_datetime::now(), false, nullptr, "Creation time");
	results.emplace_back(meta_name::modification_date, util::arbitrary_datetime::now(), false, nullptr, "Modification time");
	return results;
}


//-------------------------------------------------
//  fat_image::mount_partition
//-------------------------------------------------

std::unique_ptr<filesystem_t> fs::fat_image::mount_partition(fsblk_t &blockdev, u32 starting_sector, u32 sector_count, u8 bits_per_fat_entry)
{
	// load the boot sector block and get some basic info
	fsblk_t::block_t boot_sector_block = blockdev.get(starting_sector);
	u16 reserved_sector_count = boot_sector_block.r16l(impl::OFFSET_RESERVED_SECTOR_COUNT);

	// load all file allocation table sectors
	u32 fat_count = boot_sector_block.r8(impl::OFFSET_FAT_COUNT);
	u32 sectors_per_fat = boot_sector_block.r16l(impl::OFFSET_FAT_SECTOR_COUNT);
	u16 bytes_per_sector = boot_sector_block.r16l(impl::OFFSET_BYTES_PER_SECTOR);
	std::vector<u8> file_allocation_table;
	file_allocation_table.reserve(fat_count * sectors_per_fat * bytes_per_sector);
	for (auto i = 0; i < fat_count * sectors_per_fat; i++)
	{
		fsblk_t::block_t fatblk = blockdev.get(starting_sector + reserved_sector_count + i);
		file_allocation_table.insert(file_allocation_table.end(), fatblk.rodata(), fatblk.rodata() + bytes_per_sector);
	}

	// and return the implementation
	return std::make_unique<impl>(blockdev, std::move(boot_sector_block), std::move(file_allocation_table),
		starting_sector, sector_count, reserved_sector_count, bits_per_fat_entry);
}


//-------------------------------------------------
//  directory_entry::name
//-------------------------------------------------

std::string directory_entry::name() const
{
	std::string_view stem = filesystem_t::trim_end_spaces(raw_stem());
	std::string_view ext = filesystem_t::trim_end_spaces(raw_ext());
	return !ext.empty()
		? util::string_format("%s.%s", stem, ext)
		: std::string(stem);
}


//-------------------------------------------------
//  directory_entry::metadata
//-------------------------------------------------

meta_data directory_entry::metadata() const
{
	meta_data result;
	result.set(meta_name::name,                 name());
	result.set(meta_name::creation_date,        decode_fat_datetime(raw_create_datetime()));
	result.set(meta_name::modification_date,    decode_fat_datetime(raw_modified_datetime()));
	result.set(meta_name::length,               file_size());
	return result;
}


//-------------------------------------------------
//  impl ctor
//-------------------------------------------------

impl::impl(fsblk_t &blockdev, fsblk_t::block_t &&boot_sector_block, std::vector<u8> &&file_allocation_table, u32 starting_sector, u32 sector_count, u16 reserved_sector_count, u8 bits_per_fat_entry)
	: filesystem_t(blockdev, 512)
	, m_boot_sector_block(std::move(boot_sector_block))
	, m_file_allocation_table(std::move(file_allocation_table))
	, m_starting_sector(starting_sector)
	, m_sector_count(sector_count)
	, m_reserved_sector_count(reserved_sector_count)
	, m_bytes_per_sector(m_boot_sector_block.r16l(OFFSET_BYTES_PER_SECTOR))
	, m_root_directory_size(m_boot_sector_block.r16l(OFFSET_DIRECTORY_ENTRY_COUNT))
	, m_sectors_per_cluster(m_boot_sector_block.r8(OFFSET_CLUSTER_SECTOR_COUNT))
	, m_fat_count(m_boot_sector_block.r8(OFFSET_FAT_COUNT))
	, m_fat_sector_count(m_boot_sector_block.r16l(OFFSET_FAT_SECTOR_COUNT))
	, m_bits_per_fat_entry(bits_per_fat_entry)
	, m_last_cluster_indicator(((u64)1 << bits_per_fat_entry) - 1)
	, m_last_valid_cluster(m_last_cluster_indicator - 0x10)
{
}


//-------------------------------------------------
//  impl::volume_metadata
//-------------------------------------------------

meta_data impl::volume_metadata()
{
	std::vector<std::string> root_path;
	directory_span::ptr root_dir = find_directory(root_path.begin(), root_path.end());
	assert(root_dir);

	// Get the volume label from the root directory, not the extended BPB (whose name field may not be kept up-to-date even when it exists)
	meta_data results;
	auto const callback = [&results] (const directory_entry &dirent)
	{
		if (dirent.is_volume_label())
		{
			results.set(meta_name::name, dirent.name());
			return true;
		}
		return false;
	};
	iterate_directory_entries(*root_dir, callback);
	if (!results.has(meta_name::name))
		results.set(meta_name::name, "UNTITLED");

	results.set(meta_name::oem_name, m_boot_sector_block.rstr(3, 8));
	return results;
}


//-------------------------------------------------
//  impl::metadata
//-------------------------------------------------

std::pair<err_t, meta_data> impl::metadata(const std::vector<std::string> &path)
{
	std::optional<directory_entry> dirent = find_entity(path);
	if (!dirent)
		return std::make_pair(ERR_NOT_FOUND, meta_data());

	return std::make_pair(ERR_OK, dirent->metadata());
}


//-------------------------------------------------
//  impl::directory_contents
//-------------------------------------------------

std::pair<err_t, std::vector<dir_entry>> impl::directory_contents(const std::vector<std::string> &path)
{
	directory_span::ptr dir = find_directory(path.begin(), path.end());
	if (!dir)
		return std::make_pair(ERR_NOT_FOUND, std::vector<dir_entry>());

	std::vector<dir_entry> results;
	auto const callback = [&results] (const directory_entry &dirent)
	{
		if (!dirent.is_volume_label())
		{
			dir_entry_type entry_type = dirent.is_subdirectory() ? dir_entry_type::dir : dir_entry_type::file;
			results.emplace_back(entry_type, dirent.metadata());
		}
		return false;
	};
	iterate_directory_entries(*dir, callback);
	return std::make_pair(ERR_OK, std::move(results));
}


//-------------------------------------------------
//  impl::file_read
//-------------------------------------------------

std::pair<err_t, std::vector<u8>> impl::file_read(const std::vector<std::string> &path)
{
	// find the file
	std::optional<directory_entry> dirent = find_entity(path);
	if (!dirent || dirent->is_subdirectory())
		return std::make_pair(ERR_NOT_FOUND, std::vector<u8>());

	// get the list of sectors for this file
	std::vector<u32> sectors = get_sectors_from_fat(*dirent);

	// prepare the results
	std::vector<u8> result;
	result.reserve(dirent->file_size());

	// and add data from all sectors
	for (u32 sector : sectors)
	{
		fsblk_t::block_t block = m_blockdev.get(sector);
		const u8 *data = block.rodata();
		size_t length = std::min((size_t)dirent->file_size() - result.size(), (size_t)block.size());
		result.insert(result.end(), data, data + length);
	}
	return std::make_pair(ERR_OK, std::move(result));
}


bool impl::is_valid_short_filename(std::string const &filename)
{
	/*
	    Valid characters in DOS file names:
	    - Upper case letters A-Z
	    - Numbers 0-9
	    - Space (though there is no way to identify a trailing space)
	    - ! # $ % & ( ) - @ ^ _ ` { } ~
	    - Characters 128-255, except e5 (though the code page is indeterminate)
	    We currently do not check for characters 128-255.
	*/
	std::regex filename_regex("([A-Z0-9!#\\$%&\\(\\)\\-@^_`\\{\\}~]{0,8})(\\.([A-Z0-9!#\\$%&\\(\\)\\-@^_`\\{\\}~]{0,3}))?");
	return std::regex_match(filename, filename_regex);
}


err_t impl::build_direntry_filename(std::string const &filename, std::string &fname)
{
	std::regex filename_regex("([A-Z0-9!#\\$%&\\(\\)\\-@^_`\\{\\}~]{0,8})(\\.([A-Z0-9!#\\$%&\\(\\)\\-@^_`\\{\\}~]{0,3}))?");
	std::smatch smatch;
	if (!std::regex_match(filename, smatch, filename_regex))
		return ERR_INVALID;
	if (smatch.size() != 4)
		return ERR_INVALID;

	fname.resize(directory_entry::FNAME_LENGTH, ' ');

	for (int i = 0; i < 8 && i < smatch.str(1).size(); i++)
		fname[i] = smatch.str(1)[i];

	for (int j = 0; j < 3 && j < smatch.str(3).size(); j++)
		fname[8 + j] = smatch.str(3)[j];

	return ERR_OK;
}


err_t impl::file_create(const std::vector<std::string> &path, const meta_data &meta)
{
	std::string filename = meta.get_string(meta_name::name, "");
	std::string fname;
	err_t err = build_direntry_filename(filename, fname);
	if (err != ERR_OK)
		return err;

	if (path.empty())
	{
		return file_create_root(fname);
	}
	else
	{
		// Make sure that all parts of the path exist, creating the path parts as needed.
		std::optional<directory_entry> dirent = find_entity(path);
		if (!dirent)
		{
			std::vector<std::string> partial_path;
			std::optional<directory_entry> parent_entry;
			for (auto const &path_part : path)
			{
				partial_path.emplace_back(path_part);
				std::optional<directory_entry> dir_entry = find_entity(partial_path);
				if (!dir_entry)
				{
					if (!is_valid_short_filename(path_part))
						return ERR_INVALID;

					std::string part_fname;
					err_t err = build_direntry_filename(path_part, part_fname);
					if (err != ERR_OK)
						return err;
					err = !parent_entry ?
							file_create_root(part_fname, directory_entry::ATTR_DIRECTORY) :
							file_create_directory(parent_entry.value(), part_fname, directory_entry::ATTR_DIRECTORY);
					if (err != ERR_OK)
						return err;

					dir_entry = find_entity(partial_path);
					if (!dir_entry)
						return ERR_INVALID;

					err = initialize_directory(dir_entry->start_cluster(), parent_entry ? parent_entry->start_cluster() : 0);
					if (err != ERR_OK)
						return err;
				}
				else
				{
					if (!dir_entry->is_subdirectory())
						return ERR_INVALID;
				}
				parent_entry = dir_entry;
			}

			dirent = find_entity(path);
			if (!dirent)
				return ERR_INVALID;
		}

		return file_create_directory(*dirent, fname);
	}
}


err_t impl::initialize_directory(u32 directory_cluster, u32 parent_cluster)
{
	clear_cluster_sectors(directory_cluster, 0x00);

	auto dirblk = m_blockdev.get(first_cluster_sector(directory_cluster));

	// Add special directory entries for . and ..
	std::string dir_fname;
	dir_fname.resize(directory_entry::FNAME_LENGTH, ' ');
	dir_fname[0] = '.';
	err_t err = initialize_directory_entry(dirblk, 0, dir_fname, directory_entry::ATTR_DIRECTORY, directory_cluster);
	if (err != ERR_OK)
		return err;

	dir_fname[1] = '.';
	err = initialize_directory_entry(dirblk, directory_entry::SIZE, dir_fname, directory_entry::ATTR_DIRECTORY, parent_cluster);
	if (err != ERR_OK)
		return err;

	return ERR_OK;
}


err_t impl::initialize_directory_entry(fsblk_t::block_t &dirblk, u32 offset, const std::string_view &fname, u8 attributes, u32 start_cluster)
{
	if (fname.size() != directory_entry::FNAME_LENGTH)
		return ERR_INVALID;

	for (int i = 0; i < directory_entry::SIZE; i += 4)
		dirblk.w32l(offset + i, 0);

	dirblk.wstr(offset + directory_entry::OFFSET_FNAME, fname);
	dirblk.w8(offset + directory_entry::OFFSET_ATTRIBUTES, attributes);
	dirblk.w32l(offset + directory_entry::OFFSET_CREATE_DATETIME, encode_now_fat_datetime());
	dirblk.w32l(offset + directory_entry::OFFSET_MODIFIED_DATETIME, encode_now_fat_datetime());
	dirblk.w16l(offset + directory_entry::OFFSET_START_CLUSTER_HI, u16(start_cluster >> 16));
	dirblk.w16l(offset + directory_entry::OFFSET_START_CLUSTER, u16(start_cluster & 0xffff));

	return ERR_OK;
}


err_t impl::file_create_root(std::string &fname, u8 attributes)
{
	const u32 first_directory_sector = m_starting_sector + m_reserved_sector_count + ((u32)m_file_allocation_table.size() / m_bytes_per_sector);
	const u32 directory_sector_count = (m_root_directory_size * directory_entry::SIZE) / m_bytes_per_sector;
	for (u32 sector = first_directory_sector; sector < first_directory_sector + directory_sector_count; sector++)
	{
		err_t err = file_create_sector(sector, fname, attributes);
		if (err != ERR_NOT_FOUND)
			return err;
	}
	return ERR_NO_SPACE;
}


err_t impl::file_create_directory(directory_entry &dirent, std::string &fname, u8 attributes)
{
	u32 current_cluster = dirent.start_cluster();
	do {
		const u32 first_sector = first_cluster_sector(current_cluster);
		for (int i = 0; i < m_sectors_per_cluster; i++)
		{
			err_t err = file_create_sector(first_sector + i, fname, attributes);
			if (err != ERR_NOT_FOUND)
				return err;
		}

		// File could not be created yet. Move to next cluster, allocating a new cluster when needed.
		u32 next_cluster = get_next_cluster(current_cluster);
		if (next_cluster >= m_last_valid_cluster)
		{
			next_cluster = find_free_cluster();
			if (next_cluster == 0)
				return ERR_NO_SPACE;

			set_next_cluster(current_cluster, next_cluster);
			set_next_cluster(next_cluster, m_last_cluster_indicator);

			clear_cluster_sectors(next_cluster, 0x00);
		}
		current_cluster = next_cluster;
	} while (current_cluster > FIRST_VALID_CLUSTER && current_cluster < m_last_valid_cluster);
	return ERR_NO_SPACE;
}


u32 impl::first_cluster_sector(u32 cluster)
{
	return m_starting_sector + m_reserved_sector_count +
		((u32)m_file_allocation_table.size() / m_bytes_per_sector) +
		((m_root_directory_size + 1) / dirents_per_sector()) +
		((cluster - FIRST_VALID_CLUSTER) * m_sectors_per_cluster);
}


void impl::clear_cluster_sectors(u32 cluster, u8 fill_byte)
{
	const u32 sector = first_cluster_sector(cluster);
	for (int i = 0; i < m_sectors_per_cluster; i++)
	{
		auto dirblk = m_blockdev.get(sector + i);
		for (int offset = 0; offset < m_bytes_per_sector; offset++)
			dirblk.w8(offset, fill_byte);
	}
}


// Returns ERR_NOT_FOUND when no room could be found to create the file in the sector.
err_t impl::file_create_sector(u32 sector, std::string &fname, u8 attributes)
{
	auto dirblk = m_blockdev.get(sector);
	for (u32 blkoffset = 0; blkoffset < m_bytes_per_sector; blkoffset += directory_entry::SIZE)
	{
		u8 first_byte = dirblk.r8(blkoffset);
		if (first_byte == 0x00 || first_byte == directory_entry::DELETED_FILE_MARKER)
		{
			u32 start_cluster = find_free_cluster();
			if (start_cluster == 0)
				return ERR_NO_SPACE;
			set_next_cluster(start_cluster, m_last_cluster_indicator);

			err_t err = initialize_directory_entry(dirblk, blkoffset, fname, attributes, start_cluster);
			if (err != ERR_OK)
				return err;

			return ERR_OK;
		}
	}
	return ERR_NOT_FOUND;
}


err_t impl::file_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	std::optional<directory_entry> dirent = find_entity(path);
	if (!dirent)
		return ERR_NOT_FOUND;

	if (dirent->is_subdirectory())
		return ERR_INVALID;

	u32 current_length = dirent->file_size();
	const size_t data_length = data.size();
	const u32 bytes_per_cluster = m_sectors_per_cluster * bytes_per_sector();
	const u32 current_clusters = (current_length + bytes_per_cluster - 1) / bytes_per_cluster;
	const u32 required_clusters = (data_length + bytes_per_cluster - 1) / bytes_per_cluster;

	if (required_clusters > current_clusters)
	{
		u32 current_cluster = dirent->start_cluster();
		u32 next_cluster = 0;
		do {
			next_cluster = get_next_cluster(current_cluster);
			if (next_cluster < FIRST_VALID_CLUSTER)
				return ERR_INVALID;
		} while (next_cluster < m_last_valid_cluster);
		for (int i = current_clusters; i < required_clusters; i++)
		{
			u32 free_cluster = find_free_cluster();
			if (free_cluster < FIRST_VALID_CLUSTER)
				return ERR_NO_SPACE;

			set_next_cluster(current_cluster, free_cluster);
			set_next_cluster(free_cluster, m_last_cluster_indicator);
			current_cluster = free_cluster;
		}
	}
	if (required_clusters < current_clusters)
	{
		u32 current_cluster = dirent->start_cluster();
		for (int i = 0; i < required_clusters; i++)
		{
			current_cluster = get_next_cluster(current_cluster);
		}
		u32 next_cluster = get_next_cluster(current_cluster);
		set_next_cluster(current_cluster, m_last_cluster_indicator);

		err_t err = free_clusters(next_cluster);
		if (err != ERR_OK)
			return err;
	}

	auto sectors = get_sectors_from_fat(*dirent);
	size_t offset = 0;
	for (auto sector : sectors)
	{
		if (offset < data_length)
		{
			auto datablk = m_blockdev.get(sector);
			u32 bytes = (data_length - offset > m_bytes_per_sector) ? m_bytes_per_sector : data_length - offset;
			memcpy(datablk.data(), data.data() + offset, bytes);
			offset += m_bytes_per_sector;
		}
	}

	dirent->set_raw_modified_datetime(encode_now_fat_datetime());
	dirent->set_file_size(data_length);

	return ERR_OK;
}


err_t impl::remove(const std::vector<std::string> &path)
{
	if (path.size() != 0)
		return ERR_UNSUPPORTED;

	std::optional<directory_entry> dirent = find_entity(path);
	if (!dirent)
		return ERR_OK;

	// Removing directories is not supported yet
	if (dirent->is_subdirectory())
		return ERR_UNSUPPORTED;

	dirent->mark_deleted();

	return ERR_OK;
}


err_t impl::free_clusters(u32 start_cluster)
{
	while (start_cluster < m_last_valid_cluster)
	{
		if (start_cluster < FIRST_VALID_CLUSTER)
			return ERR_INVALID;

		u32 next_cluster = get_next_cluster(start_cluster);
		set_next_cluster(start_cluster, 0);
		start_cluster = next_cluster;
	}
	return ERR_OK;
}

//-------------------------------------------------
//  impl::get_sectors_from_fat
//-------------------------------------------------

std::vector<u32> impl::get_sectors_from_fat(const directory_entry &dirent) const
{
	// prepare results
	std::vector<u32> results;
	results.reserve(dirent.file_size() / bytes_per_sector());

	// get critical information
	u16 root_directory_sector_count = (m_root_directory_size + 1) / dirents_per_sector();
	u32 fat_sector_count = m_fat_count * m_fat_sector_count;
	u32 data_starting_sector = m_starting_sector + m_reserved_sector_count + fat_sector_count + root_directory_sector_count;
	u32 data_cluster_count = (m_sector_count - data_starting_sector) / m_sectors_per_cluster;

	// find all clusters
	u32 start_cluster_mask = ((u64)1 << m_bits_per_fat_entry) - 1;
	u32 cluster = dirent.start_cluster() & start_cluster_mask;

	while (cluster >= FIRST_VALID_CLUSTER && cluster < (data_cluster_count + 2))
	{
		// add the sectors for this cluster
		for (auto i = 0; i < m_sectors_per_cluster; i++)
			results.push_back(data_starting_sector + (cluster - FIRST_VALID_CLUSTER) * m_sectors_per_cluster + i);

		// determine the bit position of this entry
		u32 entry_bit_position = cluster * m_bits_per_fat_entry;

		// sanity check; check for overflows
		u32 new_cluster = 0;
		if (entry_bit_position + m_bits_per_fat_entry <= m_file_allocation_table.size() * 8)
		{
			// this awkward logic is here because we cannot rely on FAT entries all being in one
			// sector (thank you FAT12)
			u32 current_bit = 0;
			while (current_bit < m_bits_per_fat_entry)
			{
				u32 pos = entry_bit_position + current_bit;
				u32 shift = pos % 8;
				u32 bit_count = std::min(8 - shift, m_bits_per_fat_entry - current_bit);
				u32 bits = (m_file_allocation_table[pos / 8] >> shift) & ((1 << bit_count) - 1);

				new_cluster |= (bits << current_bit);
				current_bit += bit_count;
			}

			// normalize special cluster IDs
			if (new_cluster > m_last_valid_cluster)
				new_cluster |= ~m_last_cluster_indicator;
		}
		cluster = new_cluster;
	}

	return results;
}


u32 impl::get_next_cluster(u32 cluster)
{
	u32 entry_bit_position = cluster * m_bits_per_fat_entry;
	u32 new_cluster = 0;
	if (entry_bit_position + m_bits_per_fat_entry <= m_file_allocation_table.size() * 8)
	{
		u32 current_bit = 0;
		while (current_bit < m_bits_per_fat_entry)
		{
			u32 pos = entry_bit_position + current_bit;
			u32 shift = pos % 8;
			u32 bit_count = std::min(8 - shift, m_bits_per_fat_entry - current_bit);
			u32 bits = (m_file_allocation_table[pos / 8] >> shift) & ((1 << bit_count) - 1);

			new_cluster |= (bits << current_bit);
			current_bit += bit_count;
		}
	}
	return new_cluster;
}


void impl::set_next_cluster(u32 cluster, u32 next_cluster)
{
	const u32 m_fat_start_sector = m_starting_sector + m_reserved_sector_count;
	const u32 entry_bit_position = cluster * m_bits_per_fat_entry;
	if (entry_bit_position + m_bits_per_fat_entry <= m_file_allocation_table.size() * 8)
	{
		u32 current_bit = 0;
		while (current_bit < m_bits_per_fat_entry)
		{
			u32 pos = entry_bit_position + current_bit;
			u32 shift = pos % 8;
			u32 bit_count = std::min(8 - shift, m_bits_per_fat_entry - current_bit);
			u32 byte_pos = pos / 8;
			u32 mask = ((1 << bit_count) - 1);
			m_file_allocation_table[byte_pos] = (m_file_allocation_table[byte_pos] & ~(mask << shift)) | ((next_cluster & mask) << shift);
			next_cluster = next_cluster >> bit_count;
			current_bit += bit_count;
			// Write back to backing blocks
			for (int i = 0; i < m_fat_count; i++)
			{
				u32 fat_sector = m_fat_start_sector + (i * m_fat_sector_count) + (byte_pos / m_bytes_per_sector);
				auto fatblk = m_blockdev.get(fat_sector);
				fatblk.w8(byte_pos % m_bytes_per_sector, m_file_allocation_table[byte_pos]);
			}
		}
	}
}


// Returns 0 if no free cluster could be found
u32 impl::find_free_cluster()
{
	u16 root_directory_sector_count = (m_root_directory_size + 1) / dirents_per_sector();
	u32 fat_sector_count = m_fat_count * m_fat_sector_count;
	u32 data_starting_sector = m_starting_sector + m_reserved_sector_count + fat_sector_count + root_directory_sector_count;
	u32 data_cluster_count = (m_sector_count - data_starting_sector) / m_sectors_per_cluster;

	for (u32 cluster = FIRST_VALID_CLUSTER; cluster < (data_cluster_count + 2); cluster++)
	{
		u32 entry_bit_position = cluster * m_bits_per_fat_entry;

		if (entry_bit_position + m_bits_per_fat_entry <= m_file_allocation_table.size() * 8)
		{
			u32 new_cluster = 0;
			u32 current_bit = 0;
			while (current_bit < m_bits_per_fat_entry)
			{
				u32 pos = entry_bit_position + current_bit;
				u32 shift = pos % 8;
				u32 bit_count = std::min(8 - shift, m_bits_per_fat_entry - current_bit);
				u32 bits = (m_file_allocation_table[pos / 8] >> shift) & ((1 << bit_count) - 1);

				new_cluster |= (bits << current_bit);
				current_bit += bit_count;
			}

			if (new_cluster == 0)
				return cluster;
		}
	}
	return 0;
}

//-------------------------------------------------
//  impl::find_entity
//-------------------------------------------------

std::optional<directory_entry> impl::find_entity(const std::vector<std::string> &path) const
{
	// special case; reject empty paths
	if (path.empty())
		return { };

	// find the containing directory
	directory_span::ptr dir = find_directory(path.begin(), path.end() - 1);
	if (!dir)
		return { };

	// find the last child
	return find_child(*dir, path[path.size() - 1]);
}


//-------------------------------------------------
//  impl::find_directory
//-------------------------------------------------

directory_span::ptr impl::find_directory(std::vector<std::string>::const_iterator path_begin, std::vector<std::string>::const_iterator path_end) const
{
	// the root directory is treated differently
	u32 first_sector = m_starting_sector + m_reserved_sector_count + (u32)m_file_allocation_table.size() / m_bytes_per_sector;
	directory_span::ptr current_dir = std::make_unique<root_directory_span>(*this, first_sector, m_root_directory_size);

	// traverse the directory
	for (auto iter = path_begin; iter != path_end; iter++)
	{
		// find the child file
		std::optional<directory_entry> child_directory = find_child(*current_dir, *iter);
		if (!child_directory)
			return { };

		// advance into the child directory
		current_dir = std::make_unique<subdirectory_span>(*this, std::move(*child_directory));
	}

	return current_dir;
}


//-------------------------------------------------
//  impl::find_child
//-------------------------------------------------

std::optional<directory_entry> impl::find_child(const directory_span &current_dir, std::string_view target) const
{
	std::optional<directory_entry> result;
	auto const callback = [&result, target] (const directory_entry &dirent)
	{
		bool found = dirent.name() == target;
		if (found)
			result = dirent;
		return found;
	};
	iterate_directory_entries(current_dir, callback);
	return result;
}


//-------------------------------------------------
//  impl::iterate_directory_entries
//-------------------------------------------------

template <typename T>
void impl::iterate_directory_entries(const directory_span &dir, T &&callback) const
{
	std::vector<u32> sectors = dir.get_directory_sectors();
	for (u32 sector : sectors)
	{
		bool done = false;
		fsblk_t::block_t block = m_blockdev.get(sector);
		for (u32 index = 0; !done && (index < dirents_per_sector()); index++)
		{
			directory_entry dirent(block, index * 32);
			if (dirent.raw_stem()[0] != 0x00 && !dirent.is_deleted() && !dirent.is_long_file_name())
			{
				// get the filename
				std::string_view stem = trim_end_spaces(dirent.raw_stem());
				std::string_view ext = trim_end_spaces(dirent.raw_ext());
				if (ext.empty() && (stem == "." || stem == ".."))
					continue;

				// invoke the callback
				done = callback(dirent);
			}
		}

		if (done)
			break;
	}
}


//-------------------------------------------------
//  root_directory_span ctor
//-------------------------------------------------

root_directory_span::root_directory_span(const impl &fs, u32 first_sector, u16 directory_entry_count)
	: m_fs(fs)
	, m_first_sector(first_sector)
	, m_directory_entry_count(directory_entry_count)
{
}


//-------------------------------------------------
//  root_directory_span::get_directory_sectors
//-------------------------------------------------

std::vector<u32> root_directory_span::get_directory_sectors() const
{
	u32 directory_sector_count = (m_directory_entry_count + m_fs.dirents_per_sector() - 1) / m_fs.dirents_per_sector();

	std::vector<u32> result;
	result.reserve(directory_sector_count);
	for (auto i = 0; i < directory_sector_count; i++)
		result.push_back(m_first_sector + i);
	return result;
}


//-------------------------------------------------
//  subdirectory_span ctor
//-------------------------------------------------

subdirectory_span::subdirectory_span(const impl &fs, directory_entry &&dirent)
	: m_fs(fs)
	, m_dirent(std::move(dirent))
{
}


//-------------------------------------------------
//  subdirectory_span::get_directory_sectors
//-------------------------------------------------

std::vector<u32> subdirectory_span::get_directory_sectors() const
{
	return m_fs.get_sectors_from_fat(m_dirent);
}


//**************************************************************************
//  PC FAT SPECIFIC
//**************************************************************************

//-------------------------------------------------
//  pc_fat_image::name
//-------------------------------------------------

const char *fs::pc_fat_image::name() const
{
	return "pc_fat";
}


//-------------------------------------------------
//  pc_fat_image::description
//-------------------------------------------------

const char *fs::pc_fat_image::description() const
{
	return "PC FAT";
}


//-------------------------------------------------
//  pc_fat_image::enumerate_f
//-------------------------------------------------

void pc_fat_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_PC_FORMAT, floppy_image::FF_35, floppy_image::DSSD, 368640, "pc_fat_dssd", "PC FAT 3.5\" dual-sided single density");
	fe.add(FLOPPY_PC_FORMAT, floppy_image::FF_35, floppy_image::DSDD, 737280, "pc_fat_dsdd", "PC FAT 3.5\" dual-sided double density");
	fe.add(FLOPPY_PC_FORMAT, floppy_image::FF_35, floppy_image::DSHD, 1474560, "pc_fat_dshd", "PC FAT 3.5\" dual-sided high density");
	fe.add(FLOPPY_PC_FORMAT, floppy_image::FF_35, floppy_image::DSED, 2949120, "pc_fat_dsed", "PC FAT 3.5\" dual-sided extra density");
	fe.add(FLOPPY_PC_FORMAT, floppy_image::FF_525, floppy_image::SSDD, 163840, "pc_fat_525ssdd_8", "PC FAT 5.25\" single-sided double density, 8 sectors/track");
	fe.add(FLOPPY_PC_FORMAT, floppy_image::FF_525, floppy_image::SSDD, 184320, "pc_fat_525ssdd", "PC FAT 5.25\" single-sided double density, 9 sectors/track");
	fe.add(FLOPPY_PC_FORMAT, floppy_image::FF_525, floppy_image::DSDD, 327680, "pc_fat_525dsdd_8", "PC FAT 5.25\" dual-sided double density, 8 sectors/track");
	fe.add(FLOPPY_PC_FORMAT, floppy_image::FF_525, floppy_image::DSDD, 368640, "pc_fat_525dsdd", "PC FAT 5.25\" dual-sided double density, 9 sectors/track");
	fe.add(FLOPPY_PC_FORMAT, floppy_image::FF_525, floppy_image::DSQD, 737280, "pc_fat_525dsqd", "PC FAT 5.25\" dual-sided quad density");
	fe.add(FLOPPY_PC_FORMAT, floppy_image::FF_525, floppy_image::DSHD, 1228800, "pc_fat_525dshd", "PC FAT 5.25\" dual-sided high density");
}


//-------------------------------------------------
//  pc_fat_image::mount
//-------------------------------------------------

std::unique_ptr<filesystem_t> pc_fat_image::mount(fsblk_t &blockdev) const
{
	blockdev.set_block_size(512);
	return mount_partition(blockdev, 0, blockdev.block_count(), 12);
}
