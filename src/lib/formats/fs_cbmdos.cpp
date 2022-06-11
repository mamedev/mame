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
	class root_dir : public idir_t {
	public:
		root_dir(impl &i) : m_fs(i) {}
		virtual ~root_dir() = default;

		virtual void drop_weak_references() override;
		virtual meta_data metadata() override;
		virtual std::vector<dir_entry> contents() override;
		virtual file_t file_get(u64 key) override;
		virtual dir_t dir_get(u64 key) override;

	private:
		impl &m_fs;
	};

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
		block_iterator(impl &fs, u8 first_track, u8 first_sector);
		bool next();
		const void *data() const;
		u8 size() const;

	private:
		impl &				m_fs;
		fsblk_t::block_t	m_block;
		u8					m_track;
		u8					m_sector;
	};

	class file : public ifile_t
	{
	public:
		file(impl &fs, cbmdos_dirent &&dirent);
		virtual ~file() = default;

		virtual void drop_weak_references() override;

		virtual meta_data metadata() override;
		virtual std::vector<u8> read_all() override;

	private:
		impl &			m_fs;
		cbmdos_dirent	m_dirent;
	};

	impl(fsblk_t &blockdev);
	virtual ~impl() = default;

	virtual meta_data metadata() override;
	virtual dir_t root() override;

private:
	dir_t m_root;

	void drop_root_ref();
	fsblk_t::block_t read_sector(int track, int sector) const;
	static const std::array<cbmdos_dirent, 4> &dir_entries(const fsblk_t::block_t &block);
};

// methods
bool validate_filename(std::string_view name);
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
//  validate_filename
//-------------------------------------------------

namespace {

bool validate_filename(std::string_view name)
{
	return true;
}


//-------------------------------------------------
//  strtrimright_cbm
//-------------------------------------------------

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
//  impl::metadata
//-------------------------------------------------

meta_data impl::metadata()
{
	auto bam_block = read_sector(18, 0);
	std::string_view disk_name = std::string_view((const char *) bam_block.rodata() + 0x90, 16);

	meta_data results;
	results.set(meta_name::name, strtrimright_cbm(disk_name));
	return results;
}


//-------------------------------------------------
//  impl::root
//-------------------------------------------------

filesystem_t::dir_t impl::root()
{
	if (!m_root)
		m_root = new root_dir(*this);
	return m_root.strong();
}


//-------------------------------------------------
//  impl::drop_root_ref
//-------------------------------------------------

void impl::drop_root_ref()
{
	m_root = nullptr;
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
//  impl::dir_entries
//-------------------------------------------------

const std::array<impl::cbmdos_dirent, 4> &impl::dir_entries(const fsblk_t::block_t &block)
{
	return *reinterpret_cast<const std::array<impl::cbmdos_dirent, 4> *>(block.rodata());
}


//-------------------------------------------------
//  impl::root_dir::drop_weak_references
//-------------------------------------------------

void impl::root_dir::drop_weak_references()
{
	m_fs.drop_root_ref();
}


//-------------------------------------------------
//  impl::root_dir::metadata
//-------------------------------------------------

meta_data impl::root_dir::metadata()
{
	return meta_data();
}


//-------------------------------------------------
//  impl::root_dir::contents
//-------------------------------------------------

std::vector<dir_entry> impl::root_dir::contents()
{
	std::vector<dir_entry> results;
	std::set<std::tuple<u8, u8>> visited_set;

	u8 dir_track = 18;
	u8 dir_sector = 1;
	while (dir_track != 0)
	{
		// this logic checks for a degenerate scenario where the directory structure contains
		// a cyle
		auto visited_tuple = std::make_tuple(dir_track, dir_sector);
		if (visited_set.find(visited_tuple) != visited_set.end())
			break;
		visited_set.insert(visited_tuple);

		// process directory entries in this sector
		auto dir_block = m_fs.read_sector(dir_track, dir_sector);
		auto &entries = dir_entries(dir_block);
		u64 key = ((u64)dir_track) << 16 | ((u64)dir_sector) << 8;
		for (const cbmdos_dirent &ent : entries)
		{
			if (ent.m_file_type != 0x00)
			{
				std::string_view file_name = strtrimright_cbm(ent.m_file_name);
				results.emplace_back(std::string(file_name), dir_entry_type::file, key);
			}
			key++;
		}

		// advance to the next directory sector
		dir_track = entries[0].m_next_directory_track;
		dir_sector = entries[0].m_next_directory_sector;
	}
	return results;
}


//-------------------------------------------------
//  impl::root_dir::file_get
//-------------------------------------------------

filesystem_t::file_t impl::root_dir::file_get(u64 key)
{
	u8 track = (u8)(key >> 16);
	u8 sector = (u8)(key >> 8);
	auto dir_block = m_fs.read_sector(track, sector);
	const cbmdos_dirent &ent = dir_entries(dir_block)[(u8)key];
	return file_t(new file(m_fs, cbmdos_dirent(ent)));
}


//-------------------------------------------------
//  impl::root_dir::dir_get
//-------------------------------------------------

filesystem_t::dir_t impl::root_dir::dir_get(u64 key)
{
	throw std::logic_error("Directories not supported");
}


//-------------------------------------------------
//  impl::block_iterator ctor
//-------------------------------------------------

impl::block_iterator::block_iterator(impl &fs, u8 first_track, u8 first_sector)
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
		m_block = m_fs.read_sector(m_track, m_sector);
		m_track = m_block.r8(0);
		m_sector = m_block.r8(1);
		result = true;
	}
	else
	{
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
//  impl::block_iterator::size
//-------------------------------------------------

u8 impl::block_iterator::size() const
{
	return m_track != 0x00 ? 254 : m_sector - 1;
}


//-------------------------------------------------
//  impl::file ctor
//-------------------------------------------------

impl::file::file(impl &fs, cbmdos_dirent &&dirent)
	: m_fs(fs)
	, m_dirent(std::move(dirent))
{
}


//-------------------------------------------------
//  impl::file::drop_weak_references
//-------------------------------------------------

void impl::file::drop_weak_references()
{
}


//-------------------------------------------------
//  impl::file::metadata
//-------------------------------------------------

meta_data impl::file::metadata()
{
	std::string file_type;
	switch (m_dirent.m_file_type)
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
		file_type = util::string_format("$0x02X", (int)m_dirent.m_file_type);
		break;
	}

	// compute the file size
	u32 file_size = 0;
	block_iterator iter(m_fs, m_dirent.m_file_first_track, m_dirent.m_file_first_sector);
	while (iter.next())
		file_size += iter.size();

	// build the metadata and return it
	meta_data result;
	result.set(meta_name::name, strtrimright_cbm(m_dirent.m_file_name));
	result.set(meta_name::file_type, std::move(file_type));
	result.set(meta_name::length, file_size);
	return result;
}


//-------------------------------------------------
//  impl::file::read_all
//-------------------------------------------------

std::vector<u8> impl::file::read_all()
{
	std::vector<u8> result;

	block_iterator iter(m_fs, m_dirent.m_file_first_track, m_dirent.m_file_first_sector);
	while (iter.next())
		result.insert(result.end(), (const u8 *)iter.data(), (const u8 *)iter.data() + iter.size());

	return result;
}

}
