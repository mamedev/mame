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
	  19       2  Total sectors (bits 0-15)
	  21       1  Media descriptor
	  22       2  Sectors per FAT
	  24       2  Sectors per track
	  26       2  Number of heads
	  28       4  Hidden sectors
	  32       4  Total sectors (bits 16-47)
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
#include "pc_dsk.h"
#include "strformat.h"
#include "util/corestr.h"
#include "util/strformat.h"

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

	std::string_view raw_stem() const	{ return std::string_view((const char *) &m_block.rodata()[m_offset + 0], 8); }
	std::string_view raw_ext() const	{ return std::string_view((const char *) &m_block.rodata()[m_offset + 8], 3); }
	u8 attributes() const				{ return m_block.r8(m_offset + 11); }
	u32 raw_create_datetime() const		{ return m_block.r32l(m_offset + 14); }
	u32 raw_modified_datetime() const	{ return m_block.r32l(m_offset + 22); }
	u32 start_cluster() const			{ return ((u32)m_block.r16l(m_offset + 20)) << 16 | m_block.r16l(m_offset + 26); }
	u32 file_size() const				{ return m_block.r32l(m_offset + 28); }

	bool is_read_only() const			{ return (attributes() & 0x01) != 0x00; }
	bool is_hidden() const				{ return (attributes() & 0x02) != 0x00; }
	bool is_system() const				{ return (attributes() & 0x04) != 0x00; }
	bool is_volume_label() const		{ return (attributes() & 0x08) != 0x00; }
	bool is_subdirectory() const		{ return (attributes() & 0x10) != 0x00; }
	bool is_archive() const				{ return (attributes() & 0x20) != 0x00; }

	meta_data metadata() const;

private:
	fsblk_t::block_t	m_block;
	u32					m_offset;
};


class impl : public filesystem_t
{
public:
	// ctor/dtor
	impl(fsblk_t &blockdev, fsblk_t::block_t &&boot_sector_block, std::vector<fsblk_t::block_t> &&fat_sectors, u32 starting_sector, u32 sector_count, u16 reserved_sector_count, u8 bits_per_fat_entry);
	virtual ~impl() = default;

	// accessors
	fsblk_t &blockdev() { return m_blockdev; }
	u16 bytes_per_sector() const { return m_bytes_per_sector; }
	u32 dirents_per_sector() const { return bytes_per_sector() / directory_entry::SIZE; }

	// virtuals
	virtual meta_data metadata() override;
	virtual dir_t root() override;

	// methods
	std::vector<u32> get_sectors_from_fat(const directory_entry &dirent) const;
	directory_entry dirent_from_key(u64 key);

private:
	fsblk_t::block_t				m_boot_sector_block;
	std::vector<fsblk_t::block_t>	m_fat_sectors;
	u32								m_starting_sector;
	u32								m_sector_count;
	u16								m_reserved_sector_count;
	u16								m_bytes_per_sector;
	u8								m_bits_per_fat_entry;

	// ======================> file

	class file : public ifile_t
	{
	public:
		file(impl &fs, u64 key);

		virtual void drop_weak_references() override;
		virtual meta_data metadata() override;
		virtual std::vector<u8> read_all() override;

	private:
		impl &			m_fs;
		directory_entry	m_dirent;
	};

	// ======================> directory_base

	class directory_base : public idir_t
	{
	public:
		// ctor/dtor
		directory_base(impl &fs);
		virtual ~directory_base() = default;

		// virtuals
		virtual void drop_weak_references() override;
		virtual std::vector<dir_entry> contents() override;
		virtual file_t file_get(u64 key) override;
		virtual dir_t dir_get(u64 key) override;

	protected:
		virtual std::vector<u32> find_directory_entries() = 0;

		impl &	m_fs;
	};

	// ======================> root_directory

	class root_directory : public directory_base
	{
	public:
		// ctor/dtor
		root_directory(impl &fs, u32 first_sector, u16 directory_entry_count);
		virtual ~root_directory() = default;

		// virtual
		virtual meta_data metadata() override;

	protected:
		virtual std::vector<u32> find_directory_entries() override;

	private:
		u32		m_first_sector;
		u32		m_directory_entry_count;
	};

	// ======================> subdirectory

	class subdirectory : public directory_base
	{
	public:
		subdirectory(impl &fs, u64 key);

		// virtual
		virtual meta_data metadata() override;

	protected:
		virtual std::vector<u32> find_directory_entries() override;

	private:
		directory_entry	m_dirent;
	};
};


}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//	validate_filename
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
//	decode_fat_datetime
//-------------------------------------------------

util::arbitrary_datetime decode_fat_datetime(u32 dt)
{
	util::arbitrary_datetime result;
	memset(&result, 0, sizeof(result));

	result.year			= ((dt >> 25)		& 0x7F) + 1980;
	result.month		= (dt >> 21)		& 0x0F;
	result.day_of_month = (dt >> 16)		& 0x1F;
	result.hour			= (dt >> 11)		& 0x1F;
	result.minute		= (dt >>  5)		& 0x3F;
	result.second		= ((dt >>  0)		& 0x1F) * 2;
	return result;
}


}

//-------------------------------------------------
//	fat_image::can_format
//-------------------------------------------------

bool fs::fat_image::can_format() const
{
	return false;
}


//-------------------------------------------------
//	fat_image::can_read
//-------------------------------------------------

bool fs::fat_image::can_read() const
{
	return true;
}


//-------------------------------------------------
//	fat_image::can_write
//-------------------------------------------------

bool fs::fat_image::can_write() const
{
	return false;
}


//-------------------------------------------------
//	fat_image::has_rsrc
//-------------------------------------------------

bool fs::fat_image::has_rsrc() const
{
	return false;
}


//-------------------------------------------------
//	fat_image::directory_separator
//-------------------------------------------------

char fs::fat_image::directory_separator() const
{
	return '\\';
}


//-------------------------------------------------
//	fat_image::volume_meta_description
//-------------------------------------------------

std::vector<meta_description> fs::fat_image::volume_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_name::name, "UNTITLED",	false, [](const meta_value &m) { return m.as_string().size() <= 11; }, "Volume name, up to 11 characters");
	results.emplace_back(meta_name::oem_name, "",		false, [](const meta_value &m) { return m.as_string().size() <= 8; }, "OEM name, up to 8 characters");
	return results;
}


//-------------------------------------------------
//	fat_image::file_meta_description
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
//	fat_image::directory_meta_description
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
//	fat_image::mount_partition
//-------------------------------------------------

std::unique_ptr<filesystem_t> fs::fat_image::mount_partition(fsblk_t &blockdev, u32 starting_sector, u32 sector_count, u8 bits_per_fat_entry)
{
	// load the boot sector block and get some basic info
	fsblk_t::block_t boot_sector_block = blockdev.get(starting_sector);
	u16 reserved_sector_count = boot_sector_block.r16l(14);

	// load all file allocation table sectors
	u32 fat_count = boot_sector_block.r8(16);
	u32 sectors_per_fat = boot_sector_block.r16l(22);
	std::vector<fsblk_t::block_t> fat_sectors;
	fat_sectors.reserve(fat_count * sectors_per_fat);
	for (auto i = 0; i < fat_count * sectors_per_fat; i++)
	{
		fsblk_t::block_t fatblk = blockdev.get(starting_sector + reserved_sector_count + i);
		fat_sectors.push_back(std::move(fatblk));
	}

	// and return the implementation
	return std::make_unique<impl>(blockdev, std::move(boot_sector_block), std::move(fat_sectors),
		starting_sector, sector_count, reserved_sector_count, bits_per_fat_entry);
}


//-------------------------------------------------
//	directory_entry::metadata
//-------------------------------------------------

meta_data directory_entry::metadata() const
{
	meta_data result;
	result.set(meta_name::creation_date,		decode_fat_datetime(raw_create_datetime()));
	result.set(meta_name::modification_date,	decode_fat_datetime(raw_modified_datetime()));
	result.set(meta_name::length,				file_size());
	return result;
}


//-------------------------------------------------
//	impl ctor
//-------------------------------------------------

impl::impl(fsblk_t &blockdev, fsblk_t::block_t &&boot_sector_block, std::vector<fsblk_t::block_t> &&fat_sectors, u32 starting_sector, u32 sector_count, u16 reserved_sector_count, u8 bits_per_fat_entry)
	: filesystem_t(blockdev, 512)
	, m_boot_sector_block(std::move(boot_sector_block))
	, m_fat_sectors(std::move(fat_sectors))
	, m_starting_sector(starting_sector)
	, m_sector_count(sector_count)
	, m_reserved_sector_count(reserved_sector_count)
	, m_bytes_per_sector(boot_sector_block.r16l(11))
	, m_bits_per_fat_entry(bits_per_fat_entry)
{
}


//-------------------------------------------------
//	impl::metadata
//-------------------------------------------------

meta_data impl::metadata()
{
	meta_data results;
	results.set(meta_name::name,		m_boot_sector_block.rstr(43, 11));
	results.set(meta_name::oem_name,	m_boot_sector_block.rstr(3, 8));
	return results;
}


//-------------------------------------------------
//	impl::root
//-------------------------------------------------

filesystem_t::dir_t impl::root()
{
	u32 first_sector = m_starting_sector + m_reserved_sector_count + (u32)m_fat_sectors.size();
	u16 directory_entry_count = m_boot_sector_block.r16l(17);
	return dir_t(new root_directory(*this, first_sector, directory_entry_count));
}


//-------------------------------------------------
//	impl::get_sectors_from_fat
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
	u32 data_starting_sector = m_starting_sector + m_reserved_sector_count + (u32)m_fat_sectors.size() + root_directory_sector_count;
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
		if (entry_bit_position + m_bits_per_fat_entry <= m_fat_sectors.size() * bytes_per_sector() * 8)
		{
			u32 bit = 0;
			while (bit < m_bits_per_fat_entry)
			{
				u32 fat_sector_index = (entry_bit_position + bit) / 8 / bytes_per_sector();
				u32 offset = (entry_bit_position + bit) / 8 % bytes_per_sector();
				u32 byte = m_fat_sectors[fat_sector_index].r8(offset);
				u32 mask = ((1 << (m_bits_per_fat_entry - bit % 8)) - 1) & 0xFF;
				new_cluster |= (byte & mask) << bit;
				bit += 8 - ((m_bits_per_fat_entry - bit) % 8);
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
//	impl::dirent_from_key
//-------------------------------------------------

directory_entry impl::dirent_from_key(u64 key)
{	
	u32 block = key / dirents_per_sector();
	u32 offset = (key % dirents_per_sector()) * directory_entry::SIZE;
	return directory_entry(m_blockdev.get(block), offset);
}


//-------------------------------------------------
//	file ctor
//-------------------------------------------------

impl::file::file(impl &fs, u64 key)
	: m_fs(fs)
	, m_dirent(fs.dirent_from_key(key))
{
}


//-------------------------------------------------
//	file::drop_weak_references
//-------------------------------------------------

void impl::file::drop_weak_references()
{
}


//-------------------------------------------------
//	file::metadata
//-------------------------------------------------

meta_data impl::file::metadata()
{
	return m_dirent.metadata();
}


//-------------------------------------------------
//	file::read_all
//-------------------------------------------------

std::vector<u8> impl::file::read_all()
{
	std::vector<u32> sectors = m_fs.get_sectors_from_fat(m_dirent);

	// prepare the results
	std::vector<u8> result;
	result.reserve(m_dirent.file_size());

	// and add data from all sectors
	for (u32 sector : sectors)
	{
		fsblk_t::block_t block = m_fs.blockdev().get(sector);
		const u8 *data = block.rodata();
		size_t length = std::min((size_t)m_dirent.file_size() - result.size(), (size_t)block.size());
		result.insert(result.end(), data, data + length);
	}
	return result;
}


//-------------------------------------------------
//	directory_base ctor
//-------------------------------------------------

impl::directory_base::directory_base(impl &fs)
	: m_fs(fs)
{
}


//-------------------------------------------------
//	directory_base::drop_weak_references
//-------------------------------------------------

void impl::directory_base::drop_weak_references()
{
}


//-------------------------------------------------
//	directory_base::contents
//-------------------------------------------------

std::vector<dir_entry> impl::directory_base::contents()
{
	std::vector<u32> sectors = find_directory_entries();

	std::vector<dir_entry> results;
	for (u32 sector : sectors)
	{
		fsblk_t::block_t block = m_fs.blockdev().get(sector);
		for (u32 index = 0; index < m_fs.dirents_per_sector(); index++)
		{
			directory_entry dirent(block, index * 32);
			if (dirent.raw_stem()[0] != 0x00)
			{
				// get the filename
				std::string_view stem = strtrimrightspace(dirent.raw_stem());
				std::string_view ext = strtrimrightspace(dirent.raw_ext());
				if (ext.empty() && (stem == "." || stem == ".."))
					continue;
				std::string filename = !ext.empty()
					? util::string_format("%s.%s", stem, ext)
					: std::string(stem);

				// append the entry
				dir_entry_type entry_type = dirent.is_subdirectory() ? dir_entry_type::dir : dir_entry_type::file;
				u64 key = (u64)sector * m_fs.dirents_per_sector() + index;
				results.emplace_back(std::move(filename), entry_type, key);
			}
		}
	}
	return results;
}


//-------------------------------------------------
//	directory_base::file_get
//-------------------------------------------------

filesystem_t::file_t impl::directory_base::file_get(u64 key)
{
	return file_t(new file(m_fs, key));
}


//-------------------------------------------------
//	directory_base::dir_get
//-------------------------------------------------

filesystem_t::dir_t impl::directory_base::dir_get(u64 key)
{
	return dir_t(new subdirectory(m_fs, key));
}


//-------------------------------------------------
//	root_directory ctor
//-------------------------------------------------

impl::root_directory::root_directory(impl &fs, u32 first_sector, u16 directory_entry_count)
	: directory_base(fs)
	, m_first_sector(first_sector)
	, m_directory_entry_count(directory_entry_count)
{
}


//-------------------------------------------------
//	root_directory::metadata
//-------------------------------------------------

meta_data impl::root_directory::metadata()
{
	return meta_data();
}


//-------------------------------------------------
//	root_directory::find_directory_entries
//-------------------------------------------------
 
std::vector<u32> impl::root_directory::find_directory_entries()
{
	u32 directory_sector_count = (m_directory_entry_count + m_fs.dirents_per_sector() - 1) / m_fs.dirents_per_sector();

	std::vector<u32> result;
	result.reserve(directory_sector_count);
	for (auto i = 0; i < directory_sector_count; i++)
		result.push_back(m_first_sector + i);
	return result;
}


//-------------------------------------------------
//	subdirectory ctor
//-------------------------------------------------

impl::subdirectory::subdirectory(impl &fs, u64 key)
	: directory_base(fs)
	, m_dirent(fs.dirent_from_key(key))
{
}


//-------------------------------------------------
//	subdirectory::metadata
//-------------------------------------------------

meta_data impl::subdirectory::metadata()
{
	return m_dirent.metadata();
}


//-------------------------------------------------
//	subdirectory::find_directory_entries
//-------------------------------------------------

std::vector<u32> impl::subdirectory::find_directory_entries()
{
	return m_fs.get_sectors_from_fat(m_dirent);
}


//**************************************************************************
//  PC FAT SPECIFIC
//**************************************************************************

//-------------------------------------------------
//	pc_fat_image::name
//-------------------------------------------------

const char *fs::pc_fat_image::name() const
{
	return "pc_fat";
}


//-------------------------------------------------
//	pc_fat_image::description
//-------------------------------------------------

const char *fs::pc_fat_image::description() const
{
	return "PC FAT";
}


//-------------------------------------------------
//	pc_fat_image::enumerate_f
//-------------------------------------------------

void pc_fat_image::enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const
{
	if (has(form_factor, variants, floppy_image::FF_35, floppy_image::DSSD))
		fe.add(FLOPPY_PC_FORMAT, 368640, "pc_fat_dssd", "PC FAT 3.5\" dual-sided single density");
	if (has(form_factor, variants, floppy_image::FF_35, floppy_image::DSDD))
		fe.add(FLOPPY_PC_FORMAT, 737280, "pc_fat_dsdd", "PC FAT 3.5\" dual-sided double density");
	if (has(form_factor, variants, floppy_image::FF_35, floppy_image::DSHD))
		fe.add(FLOPPY_PC_FORMAT, 1474560, "pc_fat_dshd", "PC FAT 3.5\" dual-sided high density");
	if (has(form_factor, variants, floppy_image::FF_35, floppy_image::DSED))
		fe.add(FLOPPY_PC_FORMAT, 2949120, "pc_fat_dsed", "PC FAT 3.5\" dual-sided extra density");
}


//-------------------------------------------------
//	pc_fat_image::mount
//-------------------------------------------------

std::unique_ptr<filesystem_t> pc_fat_image::mount(fsblk_t &blockdev) const
{
	blockdev.set_block_size(512);
	return mount_partition(blockdev, 0, blockdev.block_count(), 12);
}
