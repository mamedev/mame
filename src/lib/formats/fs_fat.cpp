// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    fs_fat.cpp

    PC FAT disk images

    Current Limitations:
    - Read only
    - Only supports floppy disks
    - No FAT32 support
    - No Long Filenames Support

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
	static const int SIZE = 32;

	directory_entry(const fsblk_t::block_t &block, u32 offset)
		: m_block(block)
		, m_offset(offset)
	{
	}

	std::string_view raw_stem() const   { return std::string_view((const char *) &m_block.rodata()[m_offset + 0], 8); }
	std::string_view raw_ext() const    { return std::string_view((const char *) &m_block.rodata()[m_offset + 8], 3); }
	u8 attributes() const               { return m_block.r8(m_offset + 11); }
	u32 raw_create_datetime() const     { return m_block.r32l(m_offset + 14); }
	u32 raw_modified_datetime() const   { return m_block.r32l(m_offset + 22); }
	u32 start_cluster() const           { return ((u32)m_block.r16l(m_offset + 20)) << 16 | m_block.r16l(m_offset + 26); }
	u32 file_size() const               { return m_block.r32l(m_offset + 28); }

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

private:
	static constexpr u8 DELETED_FILE_MARKER = 0xe5;

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

	// methods
	std::vector<u32> get_sectors_from_fat(const directory_entry &dirent) const;

private:
	fsblk_t::block_t                m_boot_sector_block;
	std::vector<u8>                 m_file_allocation_table;
	u32                             m_starting_sector;
	u32                             m_sector_count;
	u16                             m_reserved_sector_count;
	u16                             m_bytes_per_sector;
	u8                              m_bits_per_fat_entry;

	// methods
	std::optional<directory_entry> find_entity(const std::vector<std::string> &path) const;
	directory_span::ptr find_directory(std::vector<std::string>::const_iterator path_begin, std::vector<std::string>::const_iterator path_end) const;
	std::optional<directory_entry> find_child(const directory_span &current_dir, std::string_view target) const;
	void iterate_directory_entries(const directory_span &dir, const std::function<bool(const directory_entry &dirent)> &callback) const;
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
	auto is_invalid_filename_char = [](char ch)
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

	result.year         = ((dt >> 25)       & 0x7F) + 1980;
	result.month        = (dt >> 21)        & 0x0F;
	result.day_of_month = (dt >> 16)        & 0x1F;
	result.hour         = (dt >> 11)        & 0x1F;
	result.minute       = (dt >>  5)        & 0x3F;
	result.second       = ((dt >>  0)       & 0x1F) * 2;
	return result;
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
	return false;
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
	results.emplace_back(meta_name::name, "UNTITLED",   false, [](const meta_value &m) { return validate_filename(m.as_string()); }, "Volume name");
	results.emplace_back(meta_name::oem_name, "",       false, [](const meta_value &m) { return m.as_string().size() <= 8; }, "OEM name, up to 8 characters");
	return results;
}


//-------------------------------------------------
//  fat_image::file_meta_description
//-------------------------------------------------

std::vector<meta_description> fs::fat_image::file_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_name::name, "", false, [](const meta_value &m) { return validate_filename(m.as_string()); }, "File name");
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
	results.emplace_back(meta_name::name, "", false, [](const meta_value &m) { return validate_filename(m.as_string()); }, "File name");
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
	u16 reserved_sector_count = boot_sector_block.r16l(14);

	// load all file allocation table sectors
	u32 fat_count = boot_sector_block.r8(16);
	u32 sectors_per_fat = boot_sector_block.r16l(22);
	u16 bytes_per_sector = boot_sector_block.r16l(11);
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
	, m_bytes_per_sector(m_boot_sector_block.r16l(11))
	, m_bits_per_fat_entry(bits_per_fat_entry)
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
	auto callback = [&results](const directory_entry &dirent)
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
	auto callback = [&results](const directory_entry &dirent)
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


//-------------------------------------------------
//  impl::get_sectors_from_fat
//-------------------------------------------------

std::vector<u32> impl::get_sectors_from_fat(const directory_entry &dirent) const
{
	// prepare results
	std::vector<u32> results;
	results.reserve(dirent.file_size() / bytes_per_sector());

	// get critical information
	u8 sectors_per_cluster = m_boot_sector_block.r8(13);
	u16 root_directory_entry_count = m_boot_sector_block.r16l(17);
	u16 root_directory_sector_count = (root_directory_entry_count + 1) / dirents_per_sector();
	u32 fat_sector_count = (u32)(m_file_allocation_table.size() / m_bytes_per_sector);
	u32 data_starting_sector = m_starting_sector + m_reserved_sector_count + fat_sector_count + root_directory_sector_count;
	u32 data_cluster_count = (m_sector_count - data_starting_sector) / sectors_per_cluster;

	// find all clusters
	u32 start_cluster_mask = ((u64)1 << m_bits_per_fat_entry) - 1;
	u32 cluster = dirent.start_cluster() & start_cluster_mask;

	while (cluster >= 2 && cluster < (data_cluster_count + 2))
	{
		// add the sectors for this cluster
		for (auto i = 0; i < sectors_per_cluster; i++)
			results.push_back(data_starting_sector + (cluster - 2) * sectors_per_cluster + i);

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
			if (new_cluster > ((u32)1 << m_bits_per_fat_entry) - 0x10)
				new_cluster |= ~(((u32)1 << m_bits_per_fat_entry) - 1);
		}
		cluster = new_cluster;
	}

	return results;
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
	u16 directory_entry_count = m_boot_sector_block.r16l(17);
	directory_span::ptr current_dir = std::make_unique<root_directory_span>(*this, first_sector, directory_entry_count);

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
	auto callback = [&result, target](const directory_entry &dirent)
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

void impl::iterate_directory_entries(const directory_span &dir, const std::function<bool(const directory_entry &dirent)> &callback) const
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
