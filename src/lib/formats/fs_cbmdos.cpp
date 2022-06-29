// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

	fs_cbmdos.cpp

	Management of CBM (Commodore) DOS disk images

	http://fileformats.archiveteam.org/wiki/CBMFS

***************************************************************************/

#include "fs_cbmdos.h"
#include "d64_dsk.h"
#include "corestr.h"
#include "strformat.h"

#include <array>
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
	struct cbmdos_dirent
	{
		u8		m_next_directory_track;
		u8		m_next_directory_sector;
		u8		m_file_type;
		u8		m_file_first_track;
		u8		m_file_first_sector;
		char	m_file_name[16];
		u8		m_first_side_sector_block_track;
		u8		m_first_side_sector_block_sector;
		u8		m_rel_file_record_length;
		u8		m_unused[6];
		u8		m_sector_count_low;
		u8		m_sector_count_high;
	};

	class block_iterator
	{
	public:
		block_iterator(const impl &fs, u8 first_track, u8 first_sector);
		bool next();
		const void *data() const;
		const std::array<cbmdos_dirent, 4> &dirent_data() const;
		u8 size() const;

	private:
		const impl &					m_fs;
		fsblk_t::block_t				m_block;
		std::set<std::tuple<u8, u8>>	m_visited_set;
		u8								m_track;
		u8								m_sector;
	};

	impl(fsblk_t &blockdev);
	virtual ~impl() = default;

	virtual meta_data volume_metadata() override;
	virtual std::pair<err_t, meta_data> metadata(const std::vector<std::string> &path) override;
	virtual std::pair<err_t, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;
	virtual std::pair<err_t, std::vector<u8>> file_read(const std::vector<std::string> &path) override;

private:
	fsblk_t::block_t read_sector(int track, int sector) const;
	std::optional<cbmdos_dirent> dirent_from_path(const std::vector<std::string> &path) const;
	void iterate_directory_entries(const std::function<bool(const cbmdos_dirent &dirent)> &callback) const;
	meta_data metadata_from_dirent(const cbmdos_dirent &dirent) const;
};

// methods
std::string_view strtrimright_cbm(std::string_view str);
template<size_t N> std::string_view strtrimright_cbm(const char(&str)[N]);

};


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

void fs::cbmdos_image::enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const
{
	if (has(form_factor, variants, floppy_image::FF_525, floppy_image::SSDD))
	{
		fe.add(FLOPPY_D64_FORMAT, 174848, "d64_cbmdos_35", "D64 CBMDOS single-sided 35 tracks");
		fe.add(FLOPPY_D64_FORMAT, 192256, "d64_cbmdos_40", "D64 CBMDOS single-sided 40 tracks");
	}
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
	return false;
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

impl::impl(fsblk_t &blockdev)
	: filesystem_t(blockdev, 256)
{
}


//-------------------------------------------------
//  impl::volume_metadata
//-------------------------------------------------

meta_data impl::volume_metadata()
{
	auto bam_block = read_sector(18, 0);
	std::string_view disk_name = std::string_view((const char *) bam_block.rodata() + 0x90, 16);

	meta_data results;
	results.set(meta_name::name, strtrimright_cbm(disk_name));
	return results;
}


//-------------------------------------------------
//  impl::metadata
//-------------------------------------------------

std::pair<err_t, meta_data> impl::metadata(const std::vector<std::string> &path)
{
	std::optional<cbmdos_dirent> dirent = dirent_from_path(path);
	if (!dirent)
		return std::make_pair(ERR_NOT_FOUND, meta_data());

	return std::make_pair(ERR_OK, metadata_from_dirent(*dirent));
}


//-------------------------------------------------
//  impl::directory_contents
//-------------------------------------------------

std::pair<err_t, std::vector<dir_entry>> impl::directory_contents(const std::vector<std::string> &path)
{
	std::vector<dir_entry> results;
	auto callback = [this, &results](const cbmdos_dirent &ent)
	{
		results.emplace_back(dir_entry_type::file, metadata_from_dirent(ent));
		return false;
	};
	iterate_directory_entries(callback);
	return std::make_pair(ERR_OK, std::move(results));
}


//-------------------------------------------------
//  impl::file_read
//-------------------------------------------------

std::pair<err_t, std::vector<u8>> impl::file_read(const std::vector<std::string> &path)
{
	// find the file
	std::optional<cbmdos_dirent> dirent = dirent_from_path(path);
	if (!dirent)
		return std::make_pair(ERR_NOT_FOUND, std::vector<u8>());

	// and get the data
	std::vector<u8> result;
	block_iterator iter(*this, dirent->m_file_first_track, dirent->m_file_first_sector);
	while (iter.next())
		result.insert(result.end(), (const u8 *)iter.data(), (const u8 *)iter.data() + iter.size());

	return std::make_pair(ERR_OK, std::move(result));
}


//-------------------------------------------------
//  impl::read_sector
//-------------------------------------------------

fsblk_t::block_t impl::read_sector(int track, int sector) const
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
		return { };
	std::string_view path_part = path[0];

	std::optional<cbmdos_dirent> result;
	auto callback = [&result, path_part](const cbmdos_dirent &dirent)
	{
		bool found = strtrimright_cbm(dirent.m_file_name) == path_part;
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

void impl::iterate_directory_entries(const std::function<bool(const cbmdos_dirent &dirent)> &callback) const
{
	block_iterator iter(*this, 18, 1);
	while (iter.next())
	{
		for (const cbmdos_dirent &ent : iter.dirent_data())
		{
			if (ent.m_file_type != 0x00)
			{
				if (callback(ent))
					return;
			}
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
	case 0x80:
		file_type = "DEL";
		break;
	case 0x81:
		file_type = "SEQ";
		break;
	case 0x82:
		file_type = "PRG";
		break;
	case 0x83:
		file_type = "USR";
		break;
	case 0x84:
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
{
}


//-------------------------------------------------
//  impl::block_iterator::next
//-------------------------------------------------

bool impl::block_iterator::next()
{
	bool result;
	if (m_track != 0x00)
	{
		// check for the degenerate scenario where we have a cycle (this should not happen
		// on a non-corrupt disk)
		auto visited_tuple = std::make_tuple(m_track, m_sector);
		if (m_visited_set.find(visited_tuple) != m_visited_set.end())
			return false;
		m_visited_set.insert(visited_tuple);

		// with that out of the way, proceed
		m_block = m_fs.read_sector(m_track, m_sector);
		m_track = m_block.r8(0);
		m_sector = m_block.r8(1);
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
	return m_block.rodata() + 2;
}


//-------------------------------------------------
//  impl::block_iterator::dirent_data
//-------------------------------------------------

const std::array<impl::cbmdos_dirent, 4> &impl::block_iterator::dirent_data() const
{
	return *reinterpret_cast<const std::array<impl::cbmdos_dirent, 4> *>(m_block.rodata());
}


//-------------------------------------------------
//  impl::block_iterator::size
//-------------------------------------------------

u8 impl::block_iterator::size() const
{
	return m_track != 0x00 ? 254 : m_sector - 1;
}

}
