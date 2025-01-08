// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Wilbert Pol
/***************************************************************************

    fs_coco_rsdos.cpp

    Management of CoCo "RS-DOS" floppy images

Limitation:
- The determination of the file type is very limited.

***************************************************************************/

#include "fs_coco_rsdos.h"
#include "coco_rawdsk.h"
#include "fsblk.h"

#include "util/strformat.h"

#include <bitset>
#include <optional>
#include <regex>
#include <string_view>

using namespace fs;

namespace fs { const coco_rsdos_image COCO_RSDOS; }

namespace {

class coco_rsdos_impl : public filesystem_t
{
public:
	coco_rsdos_impl(fsblk_t &blockdev);
	virtual ~coco_rsdos_impl() = default;

	static constexpr int SECTOR_DIRECTORY_ENTRY_COUNT = 8;

	struct rsdos_dirent
	{
		char    m_filename[11];
		u8      m_filetype;
		u8      m_asciiflag;
		u8      m_first_granule;
		u8      m_last_sector_bytes_msb;
		u8      m_last_sector_bytes_lsb;
	};

	struct rsdos_dirent_sector
	{
		struct
		{
			rsdos_dirent    m_dirent;
			u8              m_unused[16];
		} m_entries[SECTOR_DIRECTORY_ENTRY_COUNT];
	};

	class granule_iterator
	{
	public:
		granule_iterator(coco_rsdos_impl &fs, const rsdos_dirent &dirent);
		bool next(u8 &granule, u16 &byte_count);

	private:
		fsblk_t::block_t    m_granule_map;
		std::optional<u8>   m_current_granule;
		u8                  m_maximum_granules;
		u16                 m_last_sector_bytes;
		std::bitset<256>    m_visited_granules;
	};

	virtual std::pair<err_t, meta_data> metadata(const std::vector<std::string> &path) override;
	virtual std::pair<err_t, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;
	virtual std::pair<err_t, std::vector<u8>> file_read(const std::vector<std::string> &path) override;
	virtual err_t format(const meta_data &meta) override;
	virtual err_t file_create(const std::vector<std::string> &path, const meta_data &meta) override;
	virtual err_t file_write(const std::vector<std::string> &path, const std::vector<u8> &data) override;

	static bool validate_filename(std::string_view name);

private:
	static constexpr u8 TRACK_GRANULE_COUNT = 2;
	static constexpr u8 GRANULE_SECTOR_COUNT = 9;
	static constexpr u8 TRACK_SECTOR_COUNT = TRACK_GRANULE_COUNT * GRANULE_SECTOR_COUNT;
	static constexpr u8 DIRECTORY_TRACK = 17;
	static constexpr u8 DIRECTORY_ENTRY_SIZE = 0x20;
	static constexpr u8 FNAME_LENGTH = 11;
	static constexpr u16 SECTOR_SIZE = 0x100;
	static constexpr u8 OFFSET_FILE_TYPE = 0x0b;
	static constexpr u8 OFFSET_ASCII_FLAG = 0x0c;
	static constexpr u8 OFFSET_FIRST_GRANULE = 0x0d;
	static constexpr u8 OFFSET_LAST_SECTOR_BYTES = 0x0e;
	static constexpr u8 FILE_TYPE_BASIC = 0x00;
	static constexpr u8 FILE_TYPE_BASIC_DATA = 0x01;
	static constexpr u8 FILE_TYPE_MACHINE_CODE = 0x02;
	static constexpr u8 FILE_TYPE_TEXT_EDITOR = 0x03;
	static constexpr u8 FILE_LAST_GRANULE_INDICATOR = 0xc0;

	fsblk_t::block_t read_sector(int track, int sector) const;
	fsblk_t::block_t read_granule_sector(u8 granule, u8 sector) const;
	u8 maximum_granules() const;
	std::optional<rsdos_dirent> dirent_from_path(const std::vector<std::string> &path);
	template <typename T>
	void iterate_directory_entries(T &&callback);
	meta_data get_metadata_from_dirent(const rsdos_dirent &dirent);
	static std::string get_filename_from_dirent(const rsdos_dirent &dirent);
	std::pair<err_t, std::string> build_direntry_filename(const std::string &filename);
	std::pair<err_t, u8> claim_granule();
	void write_granule_map(u8 granule, u8 map_data);
	u8 read_granule_map(u8 granule) const;
	bool is_ascii(const std::vector<u8> &data) const;
	u8 determine_file_type(const std::vector<u8> &data) const;
};

} // anonymous namespace


//-------------------------------------------------
//  name
//-------------------------------------------------

const char *coco_rsdos_image::name() const
{
	return "coco_rsdos";
}


//-------------------------------------------------
//  description
//-------------------------------------------------

const char *coco_rsdos_image::description() const
{
	return "CoCo RS-DOS";
}


//-------------------------------------------------
//  enumerate_f
//-------------------------------------------------

void coco_rsdos_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_COCO_RAWDSK_FORMAT, floppy_image::FF_525, floppy_image::SSSD, 161280, "coco_rawdsk_rsdos_35", "CoCo Raw Disk RS-DOS single-sided 35 tracks");
	fe.add(FLOPPY_COCO_RAWDSK_FORMAT, floppy_image::FF_525, floppy_image::SSSD, 184320, "coco_rawdsk_rsdos_40", "CoCo Raw Disk RS-DOS single-sided 40 tracks");
}


//-------------------------------------------------
//  can_format
//-------------------------------------------------

bool coco_rsdos_image::can_format() const
{
	return true;
}


//-------------------------------------------------
//  can_read
//-------------------------------------------------

bool coco_rsdos_image::can_read() const
{
	return true;
}


//-------------------------------------------------
//  can_write
//-------------------------------------------------

bool coco_rsdos_image::can_write() const
{
	return true;
}


//-------------------------------------------------
//  has_rsrc
//-------------------------------------------------

bool coco_rsdos_image::has_rsrc() const
{
	return false;
}


//-------------------------------------------------
//  file_meta_description
//-------------------------------------------------

std::vector<meta_description> coco_rsdos_image::file_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_description(meta_name::name, "", false, [](const meta_value &m) { return coco_rsdos_impl::validate_filename(m.as_string()); }, "File name, 8.3"));
	results.emplace_back(meta_description(meta_name::file_type, 0, true, nullptr, "Type of the file"));
	results.emplace_back(meta_description(meta_name::ascii_flag, "B", true, nullptr, "Ascii or binary flag"));
	results.emplace_back(meta_description(meta_name::size_in_blocks, 0, true, nullptr, "Number of granules used by the file"));
	results.emplace_back(meta_description(meta_name::length, 0, true, nullptr, "Size of the file in bytes"));
	return results;
}


//-------------------------------------------------
//  mount
//-------------------------------------------------

std::unique_ptr<filesystem_t> coco_rsdos_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<coco_rsdos_impl>(blockdev);
}


//-------------------------------------------------
//  validate_filename
//-------------------------------------------------

bool coco_rsdos_impl::validate_filename(std::string_view name)
{
	auto pos = name.find('.');
	auto stem_length = pos != std::string::npos ? pos : name.size();
	auto ext_length = pos != std::string::npos ? name.size() - pos - 1 : 0;
	return stem_length > 0 && stem_length <= 8 && ext_length <= 3;
}


//-------------------------------------------------
//  coco_rsdos_impl ctor
//-------------------------------------------------

coco_rsdos_impl::coco_rsdos_impl(fsblk_t &blockdev)
	: filesystem_t(blockdev, SECTOR_SIZE)
{
}


//-------------------------------------------------
//  coco_rsdos_impl::metadata
//-------------------------------------------------

std::pair<err_t, meta_data> coco_rsdos_impl::metadata(const std::vector<std::string> &path)
{
	// attempt to find the file
	const std::optional<rsdos_dirent> dirent = dirent_from_path(path);
	if (!dirent)
		return std::make_pair(ERR_NOT_FOUND, meta_data());

	return std::make_pair(ERR_OK, get_metadata_from_dirent(*dirent));
}


//-------------------------------------------------
//  coco_rsdos_impl::directory_contents
//-------------------------------------------------

std::pair<err_t, std::vector<dir_entry>> coco_rsdos_impl::directory_contents(const std::vector<std::string> &path)
{
	std::vector<dir_entry> results;
	auto const callback = [this, &results](u8 s, u8 i, const rsdos_dirent &dirent)
	{
		results.emplace_back(dir_entry_type::file, get_metadata_from_dirent(dirent));
		return false;
	};
	iterate_directory_entries(callback);
	return std::make_pair(ERR_OK, std::move(results));
}


//-------------------------------------------------
//  coco_rsdos_impl::file_read
//-------------------------------------------------

std::pair<err_t, std::vector<u8>> coco_rsdos_impl::file_read(const std::vector<std::string> &path)
{
	// attempt to find the file
	const std::optional<rsdos_dirent> dirent = dirent_from_path(path);
	if (!dirent)
		return std::make_pair(ERR_NOT_FOUND, std::vector<u8>());

	std::vector<u8> result;
	u8 granule;
	u16 byte_count;
	granule_iterator iter(*this, *dirent);
	while (iter.next(granule, byte_count))
	{
		// resize the results
		size_t current_size = result.size();
		result.resize(current_size + byte_count);

		// determine which track and sector this granule starts at
		int track = granule / 2 + (granule >= 34 ? 1 : 0);
		int sector = granule % 2 * 9 + 1;

		// and read all the sectors
		while (byte_count > 0)
		{
			// read this sector
			auto block = read_sector(track, sector);
			const u8 *data = block.rodata();
			u16 data_length = std::min(byte_count, u16(SECTOR_SIZE));

			// and append it to the results
			memcpy(result.data() + current_size, data, data_length);

			// and advance
			current_size += data_length;
			byte_count -= data_length;
			sector++;
		}
	}
	return std::make_pair(ERR_OK, std::move(result));
}


//-------------------------------------------------
//  coco_rsdos_impl::format
//-------------------------------------------------

err_t coco_rsdos_impl::format(const meta_data &meta)
{
	// formatting RS-DOS is easy - just fill everything with 0xFF
	m_blockdev.fill(0xFF);
	return ERR_OK;
}


std::pair<err_t, std::string> coco_rsdos_impl::build_direntry_filename(const std::string &filename)
{
	// The manual does not say anything about valid characters for a file name.
	const std::regex filename_regex("([^.]{0,8})(\\.([^.]{0,3}))?");
	std::smatch smatch;
	if (!std::regex_match(filename, smatch, filename_regex))
		return std::make_pair(ERR_INVALID, std::string());
	if (smatch.size() != 4)
		return std::make_pair(ERR_INVALID, std::string());

	std::string fname;
	fname.resize(FNAME_LENGTH, ' ');

	for (int i = 0; i < 8 && i < smatch.str(1).size(); i++)
		fname[i] = smatch.str(1)[i];

	for (int j = 0; j < 3 && j < smatch.str(3).size(); j++)
		fname[8 + j] = smatch.str(3)[j];

	return std::make_pair(ERR_OK, std::move(fname));
}


err_t coco_rsdos_impl::file_create(const std::vector<std::string> &path, const meta_data &meta)
{
	if (!path.empty())
		return ERR_UNSUPPORTED;

	const std::string filename = meta.get_string(meta_name::name, "");
	auto [err, fname] = build_direntry_filename(filename);
	if (err != ERR_OK)
		return err;

	bool found_entry = false;
	u8 dir_sector = 0;
	u8 file_index = 0;
	for (dir_sector = 3; !found_entry && dir_sector <= TRACK_SECTOR_COUNT; dir_sector++)
	{
		auto dir_block = read_sector(DIRECTORY_TRACK, dir_sector);

		for (file_index = 0; !found_entry && file_index < SECTOR_DIRECTORY_ENTRY_COUNT; file_index++)
		{
			const u8 first_byte = dir_block.r8(file_index * DIRECTORY_ENTRY_SIZE);
			// 0xff marks the end of the directory, 0x00 marks a deleted file
			found_entry = (first_byte == 0xff || first_byte == 0x00);
			if (found_entry)
			{
				for (int i = 0; i < DIRECTORY_ENTRY_SIZE; i++)
					dir_block.w8(file_index * DIRECTORY_ENTRY_SIZE + i, 0);

				auto [cerr, granule] = claim_granule();
				if (cerr != ERR_OK)
					return cerr;

				dir_block.wstr(file_index * DIRECTORY_ENTRY_SIZE + 0, fname);
				dir_block.w8(file_index * DIRECTORY_ENTRY_SIZE + OFFSET_FIRST_GRANULE, granule);
				// The file type, ASCII flag, and number of bytes in last sector of file will be set during writing.
			}
		}
	}
	if (!found_entry)
		return ERR_NO_SPACE;

	return ERR_OK;
}


err_t coco_rsdos_impl::file_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	if (path.size() != 1)
		return ERR_NOT_FOUND;
	const std::string &target = path[0];

	std::optional<rsdos_dirent> result;
	u8 dir_sector = 0;
	u8 dir_file_index = 0;
	auto const callback = [&result, &target, &dir_sector, &dir_file_index](u8 s, u8 i, const rsdos_dirent &dirent)
	{
		const bool found = get_filename_from_dirent(dirent) == target;
		if (found)
		{
			result = dirent;
			dir_sector = s;
			dir_file_index = i;
		}
		return found;
	};
	iterate_directory_entries(callback);

	if (!result)
		return ERR_NOT_FOUND;

	const size_t data_length = data.size();
	const u8 max_granule = maximum_granules();
	u8 granule = result->m_first_granule;
	u8 granule_sector = 1;
	size_t offset = 0;
	u16 bytes_in_last_sector = 0;
	u8 linked_granule = read_granule_map(granule);

	while (offset < data_length)
	{
		auto data_block = read_granule_sector(granule, granule_sector);
		bytes_in_last_sector = (data_length - offset) > SECTOR_SIZE ? SECTOR_SIZE : data_length - offset;
		memcpy(data_block.data(), data.data() + offset, bytes_in_last_sector);
		offset += SECTOR_SIZE;
		write_granule_map(granule, FILE_LAST_GRANULE_INDICATOR + granule_sector);
		if (offset < data_length)
		{
			granule_sector++;
			if (granule_sector > GRANULE_SECTOR_COUNT)
			{
				// Re-use linked granule or claim a new granule
				if (linked_granule < max_granule)
				{
					granule = linked_granule;
					linked_granule = read_granule_map(linked_granule);
				}
				else
				{
					auto [err, next_granule] = claim_granule();
					if (err != ERR_OK)
						return err;
					write_granule_map(granule, next_granule);
					granule = next_granule;
				}
				granule_sector = 1;
			}
		}
	}

	// Free unused space
	while (linked_granule < max_granule)
	{
		const u8 granule_to_free = linked_granule;
		linked_granule = read_granule_map(granule_to_free);
		write_granule_map(granule_to_free, 0xff);
	}

	// Update directory entry
	auto dir_block = read_sector(DIRECTORY_TRACK, dir_sector);
	dir_block.w8(dir_file_index * DIRECTORY_ENTRY_SIZE + OFFSET_FILE_TYPE, determine_file_type(data));
	dir_block.w8(dir_file_index * DIRECTORY_ENTRY_SIZE + OFFSET_ASCII_FLAG, is_ascii(data) ? 0xff : 0x00);
	dir_block.w16b(dir_file_index * DIRECTORY_ENTRY_SIZE + OFFSET_LAST_SECTOR_BYTES, bytes_in_last_sector);

	return ERR_OK;
}


bool coco_rsdos_impl::is_ascii(const std::vector<u8> &data) const
{
	const size_t data_length = data.size();

	if (data_length == 0)
		return false;

	for (int i = 0; i < data_length; i++)
	{
		if (!(data[i] == 0x0d || data[i] == 0x0a || (data[i] >= 0x20 && data[i] < 0x60)))
			return false;
	}

	return true;
}


u8 coco_rsdos_impl::determine_file_type(const std::vector<u8> &data) const
{
	if (is_ascii(data))
	{
		// TODO: Distinguish between BASIC code and text editor data
		return FILE_TYPE_BASIC;
	}

	const size_t data_length = data.size();
	// Binary BASIC code seems to begin with ff <16bit size> 26
	if (data_length > 4 && data[0] == 0xff && data[3] == 0x26 && ((data[1] << 8) | data[2]) == data_length - 3)
		return FILE_TYPE_BASIC;

	// TODO: Distinguish between Machine Code and Basic Data

	return FILE_TYPE_MACHINE_CODE;
}


std::pair<err_t, u8> coco_rsdos_impl::claim_granule()
{
	// Granules are likely not assigned in this order on hardware.
	auto granule_block = read_sector(DIRECTORY_TRACK, 2);
	for (int g = 0; g < maximum_granules(); g++)
	{
		if (granule_block.r8(g) == 0xff)
		{
			granule_block.w8(g, FILE_LAST_GRANULE_INDICATOR);
			return std::make_pair(ERR_OK, g);
		}
	}
	return std::make_pair(ERR_NO_SPACE, 0);
}


void coco_rsdos_impl::write_granule_map(u8 granule, u8 map_data)
{
	auto granule_block = read_sector(DIRECTORY_TRACK, 2);
	granule_block.w8(granule, map_data);
}


u8 coco_rsdos_impl::read_granule_map(u8 granule) const
{
	auto granule_block = read_sector(DIRECTORY_TRACK, 2);
	return granule_block.r8(granule);
}


//-------------------------------------------------
//  fsblk_t::block_t coco_rsdos_impl::read_sector
//-------------------------------------------------

fsblk_t::block_t coco_rsdos_impl::read_sector(int track, int sector) const
{
	// the CoCo RS-DOS world thinks in terms of tracks/sectors, but we have a block device
	// abstraction
	return m_blockdev.get(track * TRACK_SECTOR_COUNT + sector - 1);
}


fsblk_t::block_t coco_rsdos_impl::read_granule_sector(u8 granule, u8 sector) const
{
	// Track 17 does not hold granules
	if (granule < (DIRECTORY_TRACK * TRACK_GRANULE_COUNT))
		return m_blockdev.get(granule * GRANULE_SECTOR_COUNT + sector - 1);
	else
		return m_blockdev.get((granule + TRACK_GRANULE_COUNT) * GRANULE_SECTOR_COUNT + sector - 1);
}


//-------------------------------------------------
//  coco_rsdos_impl::maximum_granules
//-------------------------------------------------

u8 coco_rsdos_impl::maximum_granules() const
{
	u32 sector_count = m_blockdev.block_count();
	u32 granule_count = (sector_count / GRANULE_SECTOR_COUNT) - 2;
	return granule_count <= 0xFF ? u8(granule_count) : 0xFF;
}


//-------------------------------------------------
//  coco_rsdos_impl::dirent_from_path
//-------------------------------------------------

std::optional<coco_rsdos_impl::rsdos_dirent> coco_rsdos_impl::dirent_from_path(const std::vector<std::string> &path)
{
	if (path.size() != 1)
		return { };
	const std::string &target = path[0];

	std::optional<rsdos_dirent> result;
	auto const callback = [&result, &target](u8 s, u8 i, const rsdos_dirent &dirent)
	{
		bool found = get_filename_from_dirent(dirent) == target;
		if (found)
			result = dirent;
		return found;
	};
	iterate_directory_entries(callback);
	return result;
}


//-------------------------------------------------
//  coco_rsdos_impl::iterate_directory_entries
//-------------------------------------------------

template <typename T>
void coco_rsdos_impl::iterate_directory_entries(T &&callback)
{
	bool done = false;
	for (int dir_sector = 3; !done && dir_sector <= 18; dir_sector++)
	{
		// read this directory sector
		auto dir_block = read_sector(DIRECTORY_TRACK, dir_sector);
		const rsdos_dirent_sector &sector = *reinterpret_cast<const rsdos_dirent_sector *>(dir_block.rodata());

		// and loop through all entries
		for (int file_index = 0; file_index < 8; file_index++)
		{
			// 0xFF marks the end of the directory
			if (sector.m_entries[file_index].m_dirent.m_filename[0] == '\xFF')
			{
				done = true;
			}
			else
			{
				// 0x00 marks a deleted file
				if (sector.m_entries[file_index].m_dirent.m_filename[0] != '\0')
					done = callback(dir_sector, 0, sector.m_entries[file_index].m_dirent);
			}

			if (done)
				break;
		}
	}
}


//-------------------------------------------------
//  coco_rsdos_impl::get_metadata_from_dirent
//-------------------------------------------------

meta_data coco_rsdos_impl::get_metadata_from_dirent(const rsdos_dirent &dirent)
{
	u32 file_size = 0;
	int granule_count = 0;

	// we need to iterate on the file to determine the size and granule/block count
	u8 granule;
	u16 byte_count;
	granule_iterator iter(*this, dirent);
	while (iter.next(granule, byte_count))
	{
		granule_count++;
		file_size += byte_count;
	}

	// determine the file name
	std::string name = get_filename_from_dirent(dirent);

	// turn the ASCII flag to a single character (this reflects what the user sees when doing a directory listing on a real CoCo)
	char file_type_char = 'B' + dirent.m_asciiflag;

	// build the metadata and return it
	meta_data result;
	result.set(meta_name::name, std::move(name));
	result.set(meta_name::file_type, dirent.m_filetype);
	result.set(meta_name::ascii_flag, std::string(1, file_type_char));
	result.set(meta_name::size_in_blocks, granule_count);
	result.set(meta_name::length, file_size);
	return result;
}


//-------------------------------------------------
//  coco_rsdos_impl::get_filename_from_dirent
//-------------------------------------------------

std::string coco_rsdos_impl::get_filename_from_dirent(const rsdos_dirent &dirent)
{
	std::string_view stem = trim_end_spaces(std::string_view(&dirent.m_filename[0], 8));
	std::string_view ext = trim_end_spaces(std::string_view(&dirent.m_filename[8], 3));
	return util::string_format("%s.%s", stem, ext);
}


//-------------------------------------------------
//  coco_rsdos_impl::granule_iterator ctor
//-------------------------------------------------

coco_rsdos_impl::granule_iterator::granule_iterator(coco_rsdos_impl &fs, const rsdos_dirent &dirent)
	: m_granule_map(fs.read_sector(DIRECTORY_TRACK, 2))
	, m_current_granule(dirent.m_first_granule)
	, m_maximum_granules(fs.maximum_granules())
	, m_last_sector_bytes((u16(dirent.m_last_sector_bytes_msb) << 8) | dirent.m_last_sector_bytes_lsb)
{
	// visit the first granule
	m_visited_granules.set(dirent.m_first_granule);
}


//-------------------------------------------------
//  coco_rsdos_impl::granule_iterator::next
//-------------------------------------------------

bool coco_rsdos_impl::granule_iterator::next(u8 &granule, u16 &byte_count)
{
	bool success = false;
	granule = ~0;
	byte_count = 0;

	if (m_current_granule)
	{
		std::optional<u8> next_granule;
		const u8 *granule_map_data = m_granule_map.rodata();
		if (granule_map_data[*m_current_granule] < m_maximum_granules)
		{
			// this entry points to the next granule
			success = true;
			granule = *m_current_granule;
			byte_count = GRANULE_SECTOR_COUNT * SECTOR_SIZE;
			next_granule = granule_map_data[*m_current_granule];

			// check for cycles, which should only happen if the disk is corrupt (or not in RS-DOS format)
			if (m_visited_granules[*next_granule])
				next_granule = std::nullopt;    // this is corrupt!
			else
				m_visited_granules.set(*next_granule);
		}
		else if (granule_map_data[*m_current_granule] >= FILE_LAST_GRANULE_INDICATOR && granule_map_data[*m_current_granule] <= FILE_LAST_GRANULE_INDICATOR + GRANULE_SECTOR_COUNT)
		{
			// this is the last granule in the file
			success = true;
			granule = *m_current_granule;
			u16 sector_count = std::max(granule_map_data[*m_current_granule], u8(0xC1)) - 0xC1;
			byte_count = sector_count * SECTOR_SIZE + m_last_sector_bytes;
			next_granule = std::nullopt;
		}
		else
		{
			// should not happen; but we'll treat this as an EOF
			next_granule = std::nullopt;
		}
		m_current_granule = next_granule;
	}
	return success;
}
