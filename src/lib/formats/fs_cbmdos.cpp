// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Wilbert Pol
/***************************************************************************

    fs_cbmdos.cpp

    Management of CBM (Commodore) DOS disk images

    http://fileformats.archiveteam.org/wiki/CBMFS

Current limitations:
- Writing is limited to the first 35 tracks.
- Determine or select the file type. Currently defaulting to PRG; 0 byte files
  will be assigned file type DEL.

***************************************************************************/

#include "fs_cbmdos.h"

#include "d64_dsk.h"
#include "fsblk.h"

#include "corestr.h"
#include "multibyte.h"
#include "strformat.h"

#include <array>
#include <optional>
#include <regex>
#include <set>
#include <string_view>
#include <tuple>


namespace fs {
	const cbmdos_image CBMDOS;
};

using namespace fs;

namespace {

class impl : public filesystem_t {
public:
	static constexpr u8 SECTOR_DIRECTORY_COUNT = 8;

	struct cbmdos_dirent
	{
		u8      m_next_directory_track;
		u8      m_next_directory_sector;
		u8      m_file_type;
		u8      m_file_first_track;
		u8      m_file_first_sector;
		char    m_file_name[16];
		u8      m_first_side_sector_block_track;
		u8      m_first_side_sector_block_sector;
		u8      m_rel_file_record_length;
		u8      m_unused[6];
		u8      m_sector_count_low;
		u8      m_sector_count_high;
	};

	class block_iterator
	{
	public:
		block_iterator(const impl &fs, u8 first_track, u8 first_sector);
		bool next();
		const void *data() const;
		const std::array<cbmdos_dirent, SECTOR_DIRECTORY_COUNT> &dirent_data() const;
		u8 size() const;
		u8 track() const { return m_track; }
		u8 sector() const { return m_sector; }

	private:
		const impl &                    m_fs;
		fsblk_t::block_t::ptr           m_block;
		std::set<std::tuple<u8, u8>>    m_visited_set;
		u8                              m_track;
		u8                              m_sector;
		u8                              m_next_track;
		u8                              m_next_sector;
	};

	impl(fsblk_t &blockdev);
	virtual ~impl() = default;

	virtual meta_data volume_metadata() override;
	virtual std::pair<std::error_condition, meta_data> metadata(const std::vector<std::string> &path) override;
	virtual std::pair<std::error_condition, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;
	virtual std::pair<std::error_condition, std::vector<u8>> file_read(const std::vector<std::string> &path) override;
	virtual std::error_condition file_create(const std::vector<std::string> &path, const meta_data &meta) override;
	virtual std::error_condition file_write(const std::vector<std::string> &path, const std::vector<u8> &data) override;

private:
	static constexpr u32 BLOCK_SIZE = 256;
	static constexpr u8 DIRECTORY_ENTRY_SIZE = 0x20;
	static constexpr u8 FILE_TYPE_DEL = 0x80;
	static constexpr u8 FILE_TYPE_SEQ = 0x81;
	static constexpr u8 FILE_TYPE_PRG = 0x82;
	static constexpr u8 FILE_TYPE_USR = 0x83;
	static constexpr u8 FILE_TYPE_REL = 0x84;
	static constexpr u8 TRACK_VARIANTS = 5;
	static constexpr u8 MAX_SECTORS = 21;
	static constexpr u8 MAX_TRACKS = 40;
	static constexpr u8 DIRECTORY_TRACK = 18;
	static constexpr u8 BAM_SECTOR = 0;
	static constexpr u8 FIRST_DIRECTORY_SECTOR = 1;
	static constexpr u8 SECTOR_DATA_BYTES = 254;
	static constexpr u8 OFFSET_FILE_TYPE = 0x02;
	static constexpr u8 OFFSET_FILE_FIRST_TRACK = 0x03;
	static constexpr u8 OFFSET_FILE_FIRST_SECTOR = 0x04;
	static constexpr u8 OFFSET_FILE_NAME = 0x05;
	static constexpr u8 OFFSET_SECTOR_COUNT = 0x1e;
	static constexpr u8 OFFSET_CHAIN_TRACK = 0x00;
	static constexpr u8 OFFSET_CHAIN_SECTOR = 0x01;
	static constexpr u8 CHAIN_END = 0x00;

	static const struct smap
	{
		u8 first_track;
		u8 last_track;
		int sector[MAX_SECTORS];
	} s_track_sector_map[TRACK_VARIANTS];
	static const u8 s_data_track_order[MAX_TRACKS - 1];
	u8 m_max_track;

	fsblk_t::block_t::ptr read_sector(int track, int sector) const;
	std::optional<cbmdos_dirent> dirent_from_path(const std::vector<std::string> &path) const;
	void iterate_directory_entries(const std::function<bool(u8 track, u8 sector, u8 file_index, const cbmdos_dirent &dirent)> &callback) const;
	void iterate_all_directory_entries(const std::function<bool(u8 track, u8 sector, u8 file_index, const cbmdos_dirent &dirent)> &callback) const;
	meta_data metadata_from_dirent(const cbmdos_dirent &dirent) const;
	bool is_valid_filename(const std::string &filename) const;
	std::pair<std::error_condition, u8> claim_track_sector(u8 track) const;
	std::tuple<std::error_condition, u8, u8> claim_sector() const;
	std::error_condition free_sector(u8 track, u8 sector) const;
	u8 determine_file_type(const std::vector<u8> &data) const;
};

// methods
std::string_view strtrimright_cbm(std::string_view str);
template<size_t N> std::string_view strtrimright_cbm(const char (&str)[N]);

} // anonymous namespace


//-------------------------------------------------
//  name
//-------------------------------------------------

const char *fs::cbmdos_image::name() const
{
	return "cbmdos";
}


//-------------------------------------------------
//  description
//-------------------------------------------------

const char *fs::cbmdos_image::description() const
{
	return "CBMDOS";
}


//-------------------------------------------------
//  enumerate_f
//-------------------------------------------------

void fs::cbmdos_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_D64_FORMAT, floppy_image::FF_525, floppy_image::SSSD, 174848, "d64_cbmdos_35", "D64 CBMDOS single-sided 35 tracks");
	fe.add(FLOPPY_D64_FORMAT, floppy_image::FF_525, floppy_image::SSSD, 192256, "d64_cbmdos_40", "D64 CBMDOS single-sided 40 tracks");
}


//-------------------------------------------------
//  can_format
//-------------------------------------------------

bool fs::cbmdos_image::can_format() const
{
	return false;
}


//-------------------------------------------------
//  can_read
//-------------------------------------------------

bool fs::cbmdos_image::can_read() const
{
	return true;
}


//-------------------------------------------------
//  can_write
//-------------------------------------------------

bool fs::cbmdos_image::can_write() const
{
	return true;
}


//-------------------------------------------------
//  has_rsrc
//-------------------------------------------------

bool fs::cbmdos_image::has_rsrc() const
{
	return false;
}


//-------------------------------------------------
//  volume_meta_description
//-------------------------------------------------

std::vector<meta_description> fs::cbmdos_image::volume_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_name::name, "UNTITLED", false, [](const meta_value &m) { return m.as_string().size() <= 16; }, "Volume name, up to 16 characters");
	return results;
}


//-------------------------------------------------
//  file_meta_description
//-------------------------------------------------

std::vector<meta_description> fs::cbmdos_image::file_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_name::name, "", false, [](const meta_value &m) { return m.as_string().size() <= 16; }, "File name, up to 16 characters");
	results.emplace_back(meta_name::file_type, "", true, nullptr, "Type of the file");
	results.emplace_back(meta_name::length, 0, true, nullptr, "Size of the file in bytes");
	return results;
}


//-------------------------------------------------
//  mount
//-------------------------------------------------

std::unique_ptr<filesystem_t> fs::cbmdos_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<impl>(blockdev);
}


//-------------------------------------------------
//  strtrimright_cbm
//-------------------------------------------------

namespace {

std::string_view strtrimright_cbm(std::string_view str)
{
	return strtrimright(str, [](char c) { return c != (char)0xA0; });
}


//-------------------------------------------------
//  strtrimright_cbm
//-------------------------------------------------

template<size_t N>
std::string_view strtrimright_cbm(const char (&str)[N])
{
	std::string_view sv(str, std::size(str));
	return strtrimright_cbm(sv);
}


//-------------------------------------------------
//  impl ctor
//-------------------------------------------------

const u8 impl::s_data_track_order[MAX_TRACKS - 1] = {
	17, 19, 16, 20, 15, 21, 14, 22, 13, 23, 12, 24, 11, 25, 10, 26, 9, 27, 8, 28, 7, 29, 6, 30, 5, 31, 4, 32, 3, 33, 2, 34, 1, 35, 36, 37, 38, 39, 40
};

const impl::smap impl::s_track_sector_map[TRACK_VARIANTS] = {
	{  1, 17, { 0, 11, 1, 12,  2, 13,  3, 14, 4, 15,  5, 16,  6, 17, 7, 18,  8, 19,  9, 20, 10 } },
	{ 18, 18, { 0,  1, 4,  7, 10, 13, 16,  2, 5,  8, 11, 14, 17,  3, 6,  9, 12, 15, 18, -1, -1 } },
	{ 19, 24, { 0, 10, 1, 11,  2, 12,  3, 13, 4, 14,  5, 15,  6, 16, 7, 17,  8, 18,  9, -1, -1 } },
	{ 25, 30, { 0,  9, 1, 10,  2, 11,  3, 12, 4, 13,  5, 14,  6, 15, 7, 16,  8, 17, -1, -1, -1 } },
	{ 31, 40, { 0,  9, 1, 10,  2, 11,  3, 12, 4, 13,  5, 14,  6, 15, 7, 16,  8, -1, -1, -1, -1 } }
};


impl::impl(fsblk_t &blockdev)
	: filesystem_t(blockdev, BLOCK_SIZE)
	, m_max_track(35)
{
}


//-------------------------------------------------
//  impl::volume_metadata
//-------------------------------------------------

meta_data impl::volume_metadata()
{
	auto bam_block = read_sector(DIRECTORY_TRACK, BAM_SECTOR);
	std::string_view disk_name = bam_block->rstr(0x90, 16);

	meta_data results;
	results.set(meta_name::name, strtrimright_cbm(disk_name));
	return results;
}


//-------------------------------------------------
//  impl::metadata
//-------------------------------------------------

std::pair<std::error_condition, meta_data> impl::metadata(const std::vector<std::string> &path)
{
	std::optional<cbmdos_dirent> dirent = dirent_from_path(path);
	if (!dirent)
		return std::make_pair(error::not_found, meta_data());

	return std::make_pair(std::error_condition(), metadata_from_dirent(*dirent));
}


//-------------------------------------------------
//  impl::directory_contents
//-------------------------------------------------

std::pair<std::error_condition, std::vector<dir_entry>> impl::directory_contents(const std::vector<std::string> &path)
{
	std::vector<dir_entry> results;
	auto callback = [this, &results](u8 track, u8 sector, u8 file_index, const cbmdos_dirent &ent)
	{
		results.emplace_back(dir_entry_type::file, metadata_from_dirent(ent));
		return false;
	};
	iterate_directory_entries(callback);
	return std::make_pair(std::error_condition(), std::move(results));
}


//-------------------------------------------------
//  impl::file_read
//-------------------------------------------------

std::pair<std::error_condition, std::vector<u8>> impl::file_read(const std::vector<std::string> &path)
{
	// find the file
	std::optional<cbmdos_dirent> dirent = dirent_from_path(path);
	if (!dirent)
		return std::make_pair(error::not_found, std::vector<u8>());

	// and get the data
	std::vector<u8> result;
	block_iterator iter(*this, dirent->m_file_first_track, dirent->m_file_first_sector);
	while (iter.next())
		result.insert(result.end(), (const u8 *)iter.data(), (const u8 *)iter.data() + iter.size());

	return std::make_pair(std::error_condition(), std::move(result));
}


std::error_condition impl::file_create(const std::vector<std::string> &path, const meta_data &meta)
{
	std::string filename = meta.get_string(meta_name::name, "");
	if (!is_valid_filename(filename))
		return error::invalid_name;

	std::optional<cbmdos_dirent> result;
	u8 track = 0;
	u8 sector = 0;
	u8 file_index = 0;
	auto callback = [&result, &track, &sector, &file_index](u8 t, u8 s, u8 i, const cbmdos_dirent &dirent)
	{
		bool found = dirent.m_file_type == 0x00;
		if (found)
		{
			result = dirent;
			track = t;
			sector = s;
			file_index = i;
		}
		return found;
	};
	iterate_all_directory_entries(callback);

	if (!result)
	{
		// Claim a next directory sector
		auto const [err, new_sector] = claim_track_sector(DIRECTORY_TRACK);
		if (err)
			return err;
		auto new_block = read_sector(DIRECTORY_TRACK, new_sector);
		for (int i = 2; i < BLOCK_SIZE; i++)
			new_block->w8(i, 0);
		// Find last directory sector
		u8 last_sector = 0;
		block_iterator iter(*this, DIRECTORY_TRACK, FIRST_DIRECTORY_SECTOR);
		while (iter.next())
			last_sector = iter.sector();
		// Update chain on last directory sector
		auto last_dir_block = read_sector(DIRECTORY_TRACK, last_sector);
		last_dir_block->w8(OFFSET_CHAIN_TRACK, DIRECTORY_TRACK);
		last_dir_block->w8(OFFSET_CHAIN_SECTOR, new_sector);
		track = DIRECTORY_TRACK;
		sector = new_sector;
	}

	auto const [err, file_track, file_sector] = claim_sector();
	if (err)
		return err;

	// Create the file
	auto dirblk = read_sector(track, sector);
	u32 offset = file_index * DIRECTORY_ENTRY_SIZE;
	for (int i = 0; i < DIRECTORY_ENTRY_SIZE; i++)
		dirblk->w8(offset + i, (i >= 5 && i < 5 + 16) ? 0xa0 : 0x00);
	dirblk->w8(offset + OFFSET_FILE_TYPE, FILE_TYPE_PRG);
	dirblk->w8(offset + OFFSET_FILE_FIRST_TRACK, file_track);
	dirblk->w8(offset + OFFSET_FILE_FIRST_SECTOR, file_sector);
	dirblk->wstr(offset + OFFSET_FILE_NAME, filename);
	// TODO set first side sector block track (rel file)
	// TODO set first side sector block sector (rel file)
	// TODO set rel file record length
	// sector count will be set while writing the data

	return std::error_condition();
}


std::error_condition impl::file_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	if (path.size() != 1)
		return error::not_found;
	std::string_view path_part = path[0];

	std::optional<cbmdos_dirent> result;
	u8 dir_track = 0;
	u8 dir_sector = 0;
	u8 dir_file_index = 0;
	auto const callback = [&result, &dir_track, &dir_sector, &dir_file_index, path_part] (u8 track, u8 sector, u8 file_index, const cbmdos_dirent &dirent)
	{
		bool found = strtrimright_cbm(dirent.m_file_name) == path_part;
		if (found)
		{
			dir_track = track;
			dir_sector = sector;
			dir_file_index = file_index;
			result = dirent;
		}
		return found;
	};
	iterate_directory_entries(callback);

	if (!result)
		return error::not_found;

	u8 data_track = result->m_file_first_track;
	u8 data_sector = result->m_file_first_sector;

	const size_t data_length = data.size();
	size_t offset = 0;
	u32 sector_count = 0;
	while (offset < data_length)
	{
		auto datablk = read_sector(data_track, data_sector);
		u8 bytes = (data_length - offset) > SECTOR_DATA_BYTES ? SECTOR_DATA_BYTES : data_length - offset;
		datablk->write(2, data.data() + offset, bytes);
		offset += SECTOR_DATA_BYTES;
		sector_count++;
		if (datablk->r8(OFFSET_CHAIN_TRACK) == CHAIN_END)
		{
			if (offset < data_length)
			{
				auto [err, next_track, next_sector] = claim_sector();
				if (err)
					return err;
				datablk->w8(OFFSET_CHAIN_TRACK, next_track);
				datablk->w8(OFFSET_CHAIN_SECTOR, next_sector);
				data_track = next_track;
				data_sector = next_sector;
			}
		}
		else
		{
			if (offset < data_length)
			{
				data_track = datablk->r8(OFFSET_CHAIN_TRACK);
				data_sector = datablk->r8(OFFSET_CHAIN_SECTOR);
			}
			else
			{
				// Free the rest of the chain
				u8 track_to_free = datablk->r8(OFFSET_CHAIN_TRACK);
				u8 sector_to_free = datablk->r8(OFFSET_CHAIN_SECTOR);

				while (track_to_free != CHAIN_END)
				{
					std::error_condition const err = free_sector(track_to_free, sector_to_free);
					if (err)
						return err;
					datablk = read_sector(track_to_free, sector_to_free);
					track_to_free = datablk->r8(OFFSET_CHAIN_TRACK);
					sector_to_free = datablk->r8(OFFSET_CHAIN_SECTOR);
				}
			}
		}
		if (offset >= data_length)
		{
			datablk->w8(OFFSET_CHAIN_TRACK, CHAIN_END);
			datablk->w8(OFFSET_CHAIN_SECTOR, bytes + 1);
		}
	}

	// Write sector count and file type to directory entry
	auto dirblk = read_sector(dir_track, dir_sector);
	u8 file_type = determine_file_type(data);
	if (file_type == FILE_TYPE_DEL)
	{
		// Free sector, update first file sector to 00
		u8 file_track = dirblk->r8(DIRECTORY_ENTRY_SIZE * dir_file_index + OFFSET_FILE_FIRST_TRACK);
		u8 file_sector = dirblk->r8(DIRECTORY_ENTRY_SIZE * dir_file_index + OFFSET_FILE_FIRST_SECTOR);
		std::error_condition err = free_sector(file_track, file_sector);
		if (err)
			return err;
		dirblk->w8(DIRECTORY_ENTRY_SIZE * dir_file_index + OFFSET_FILE_FIRST_TRACK, 0);
		dirblk->w8(DIRECTORY_ENTRY_SIZE * dir_file_index + OFFSET_FILE_FIRST_SECTOR, 0);
		sector_count = 0;
	}
	dirblk->w8(DIRECTORY_ENTRY_SIZE * dir_file_index + OFFSET_FILE_TYPE, file_type);
	dirblk->w16l(DIRECTORY_ENTRY_SIZE * dir_file_index + OFFSET_SECTOR_COUNT, sector_count);

	return std::error_condition();
}


u8 impl::determine_file_type(const std::vector<u8> &data) const
{
	// TODO: Determine/set the actual file type, defaulting to PRG for now.
	const size_t data_length = data.size();
	if (data_length == 0)
		return FILE_TYPE_DEL;
	return FILE_TYPE_PRG;
}


bool impl::is_valid_filename(const std::string &filename) const
{
	// We only support a subset of the character set supported by Commodore for filenames.
	std::regex filename_regex("[A-Z0-9!\"#\\$%&'\\(\\)*+,-./:;<=>?]{1,16}");
	return std::regex_match(filename, filename_regex);
}


std::pair<std::error_condition, u8> impl::claim_track_sector(u8 track) const
{
	if (track == 0 || track > m_max_track)
		return std::make_pair(error::invalid_block, 0);

	u8 map_index;
	for (map_index = 0; map_index < TRACK_VARIANTS && !(s_track_sector_map[map_index].first_track <= track && track <= s_track_sector_map[map_index].last_track) ; map_index++);
	if (map_index >= TRACK_VARIANTS)
		return std::make_pair(error::invalid_block, 0);

	auto bamblk = read_sector(DIRECTORY_TRACK, BAM_SECTOR);
	u8 free_count = bamblk->r8(4 * track);
	u32 free_bitmap = bamblk->r24l(4 * track + 1);

	for (int s = 0; s < MAX_SECTORS; s++)
	{
		int sector = s_track_sector_map[map_index].sector[s];
		if (sector >= 0 && free_count > 0 && util::BIT(free_bitmap, sector))
		{
			free_bitmap &= ~(1 << sector);
			free_count--;
			bamblk->w8(4 * track, free_count);
			bamblk->w24l(4 * track + 1, free_bitmap);
			// Write chain end marker in new sector
			auto claimedlk = read_sector(track, sector);
			claimedlk->w8(OFFSET_CHAIN_TRACK, CHAIN_END);
			claimedlk->w8(OFFSET_CHAIN_SECTOR, 0xff);
			return std::make_pair(std::error_condition(), sector);
		}
	}
	return std::make_pair(error::no_space, 0);
}


std::tuple<std::error_condition, u8, u8> impl::claim_sector() const
{
	for (int track = 0; track < m_max_track - 1; track++)
	{
		auto const [err, sector] = claim_track_sector(s_data_track_order[track]);
		if (!err)
			return std::make_tuple(std::error_condition(), s_data_track_order[track], sector);
		if (err != error::no_space)
			return std::make_tuple(err, 0, 0);
	}
	return std::make_tuple(error::no_space, 0, 0);
}


std::error_condition impl::free_sector(u8 track, u8 sector) const
{
	if (track == 0 || track > m_max_track)
		return error::invalid_block;

	auto bamblk = read_sector(DIRECTORY_TRACK, BAM_SECTOR);
	u8 free_count = bamblk->r8(4 * track);
	u32 free_bitmap = bamblk->r24l(4 * track + 1);
	free_bitmap |= (1 << sector);
	free_count++;
	bamblk->w8(4 * track, free_count);
	bamblk->w24l(4 * track + 1, free_bitmap);
	return std::error_condition();
}


//-------------------------------------------------
//  impl::read_sector
//-------------------------------------------------

fsblk_t::block_t::ptr impl::read_sector(int track, int sector) const
{
	// CBM thinks in terms of tracks/sectors, but we have a block device abstraction
	u32 block = 0;
	block += std::max(std::min(track, 18) -  1, 0) * 21;
	block += std::max(std::min(track, 25) - 18, 0) * 19;
	block += std::max(std::min(track, 31) - 25, 0) * 18;
	block += std::max(std::min(track, 41) - 31, 0) * 17;
	block += sector;
	return m_blockdev.get(block);
}


//-------------------------------------------------
//  impl::dirent_from_path
//-------------------------------------------------

std::optional<impl::cbmdos_dirent> impl::dirent_from_path(const std::vector<std::string> &path) const
{
	if (path.size() != 1)
		return std::nullopt;
	std::string_view path_part = path[0];

	std::optional<cbmdos_dirent> result;
	auto const callback = [&result, path_part] (u8 track, u8 sector, u8 file_index, const cbmdos_dirent &dirent)
	{
		bool const found = strtrimright_cbm(dirent.m_file_name) == path_part;
		if (found)
			result = dirent;
		return found;
	};
	iterate_directory_entries(callback);
	return result;
}


//-------------------------------------------------
//  impl::iterate_directory_entries
//-------------------------------------------------

void impl::iterate_directory_entries(const std::function<bool(u8 track, u8 sector, u8 file_index, const cbmdos_dirent &dirent)> &callback) const
{
	block_iterator iter(*this, DIRECTORY_TRACK, FIRST_DIRECTORY_SECTOR);
	while (iter.next())
	{
		auto entries = iter.dirent_data();

		for (int file_index = 0; file_index < SECTOR_DIRECTORY_COUNT; file_index++)
		{
			if (entries[file_index].m_file_type != 0x00)
			{
				if (callback(iter.track(), iter.sector(), file_index, entries[file_index]))
					return;
			}
		}
	}
}

void impl::iterate_all_directory_entries(const std::function<bool(u8 track, u8 sector, u8 file_index, const cbmdos_dirent &dirent)> &callback) const
{
	block_iterator iter(*this, DIRECTORY_TRACK, FIRST_DIRECTORY_SECTOR);
	while (iter.next())
	{
		auto entries = iter.dirent_data();

		for (int file_index = 0; file_index < SECTOR_DIRECTORY_COUNT; file_index++)
		{
			if (callback(iter.track(), iter.sector(), file_index, entries[file_index]))
				return;
		}
	}
}

//-------------------------------------------------
//  impl::metadata_from_dirent
//-------------------------------------------------

meta_data impl::metadata_from_dirent(const cbmdos_dirent &dirent) const
{
	std::string file_type;
	switch (dirent.m_file_type)
	{
	case FILE_TYPE_DEL:
		file_type = "DEL";
		break;
	case FILE_TYPE_SEQ:
		file_type = "SEQ";
		break;
	case FILE_TYPE_PRG:
		file_type = "PRG";
		break;
	case FILE_TYPE_USR:
		file_type = "USR";
		break;
	case FILE_TYPE_REL:
		file_type = "REL";
		break;
	default:
		file_type = util::string_format("$0x02X", (int)dirent.m_file_type);
		break;
	}

	// compute the file size
	u32 file_size = 0;
	block_iterator iter(*this, dirent.m_file_first_track, dirent.m_file_first_sector);
	while (iter.next())
		file_size += iter.size();

	// build the metadata and return it
	meta_data result;
	result.set(meta_name::name, strtrimright_cbm(dirent.m_file_name));
	result.set(meta_name::file_type, std::move(file_type));
	result.set(meta_name::length, file_size);
	return result;
}


//-------------------------------------------------
//  impl::block_iterator ctor
//-------------------------------------------------

impl::block_iterator::block_iterator(const impl &fs, u8 first_track, u8 first_sector)
	: m_fs(fs)
	, m_track(first_track)
	, m_sector(first_sector)
	, m_next_track(first_track)
	, m_next_sector(first_sector)
{
}


//-------------------------------------------------
//  impl::block_iterator::next
//-------------------------------------------------

bool impl::block_iterator::next()
{
	bool result;
	m_track = m_next_track;
	m_sector = m_next_sector;
	if (m_track != CHAIN_END)
	{
		// check for the degenerate scenario where we have a cycle (this should not happen
		// on a non-corrupt disk)
		auto visited_tuple = std::make_tuple(m_next_track, m_next_sector);
		if (m_visited_set.find(visited_tuple) != m_visited_set.end())
			return false;
		m_visited_set.insert(visited_tuple);

		// with that out of the way, proceed
		m_block = m_fs.read_sector(m_next_track, m_next_sector);
		m_next_track = m_block->r8(OFFSET_CHAIN_TRACK);
		m_next_sector = m_block->r8(OFFSET_CHAIN_SECTOR);
		result = true;
	}
	else
	{
		// the iterator has already completed
		result = false;
	}
	return result;
}


//-------------------------------------------------
//  impl::block_iterator::data
//-------------------------------------------------

const void *impl::block_iterator::data() const
{
	return m_block->rodata() + 2;
}


//-------------------------------------------------
//  impl::block_iterator::dirent_data
//-------------------------------------------------

const std::array<impl::cbmdos_dirent, impl::SECTOR_DIRECTORY_COUNT> &impl::block_iterator::dirent_data() const
{
	return *reinterpret_cast<const std::array<impl::cbmdos_dirent, SECTOR_DIRECTORY_COUNT> *>(m_block->rodata());
}


//-------------------------------------------------
//  impl::block_iterator::size
//-------------------------------------------------

u8 impl::block_iterator::size() const
{
	return (m_track != CHAIN_END) ? SECTOR_DATA_BYTES : (m_sector - 1);
}

} // anonymous namespace
