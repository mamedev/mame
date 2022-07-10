// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    fs_coco_rsdos.cpp

    Management of CoCo "RS-DOS" floppy images

***************************************************************************/

#include "fs_coco_rsdos.h"
#include "coco_rawdsk.h"
#include "util/corestr.h"
#include "util/strformat.h"

#include <bitset>
#include <optional>
#include <string_view>

using namespace fs;

namespace fs { const coco_rsdos_image COCO_RSDOS; }

namespace {

class coco_rsdos_impl : public filesystem_t {
public:
    coco_rsdos_impl(fsblk_t &blockdev);
    virtual ~coco_rsdos_impl() = default;

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
		} m_entries[4];
	};

	class granule_iterator {
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

	static bool validate_filename(std::string_view name);

private:
	fsblk_t::block_t read_sector(int track, int sector) const;
	u8 maximum_granules() const;
	std::optional<rsdos_dirent> dirent_from_path(const std::vector<std::string> &path);
	void iterate_directory_entries(const std::function<bool(const rsdos_dirent &dirent)> &callback);
	meta_data get_metadata_from_dirent(const rsdos_dirent &dirent);
	static std::string get_filename_from_dirent(const rsdos_dirent &dirent);
};
}


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

void coco_rsdos_image::enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const
{
	if (has(form_factor, variants, floppy_image::FF_525, floppy_image::SSDD))
	{
		fe.add(FLOPPY_COCO_RAWDSK_FORMAT, 161280, "coco_rawdsk_rsdos_35", "CoCo Raw Disk RS-DOS single-sided 35 tracks");
		fe.add(FLOPPY_COCO_RAWDSK_FORMAT, 184320, "coco_rawdsk_rsdos_40", "CoCo Raw Disk RS-DOS single-sided 40 tracks");
	}
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
	return false;
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
	: filesystem_t(blockdev, 256)
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
	auto callback = [this, &results](const rsdos_dirent &dirent)
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
			u16 data_length = std::min(byte_count, u16(256));

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


//-------------------------------------------------
//  fsblk_t::block_t coco_rsdos_impl::read_sector
//-------------------------------------------------

fsblk_t::block_t coco_rsdos_impl::read_sector(int track, int sector) const
{
	// the CoCo RS-DOS world thinks in terms of tracks/sectors, but we have a block device
	// abstraction
	return m_blockdev.get(track * 18 + sector - 1);
}


//-------------------------------------------------
//  coco_rsdos_impl::maximum_granules
//-------------------------------------------------

u8 coco_rsdos_impl::maximum_granules() const
{
	u32 sector_count = m_blockdev.block_count();
	u32 granule_count = (sector_count / 9) - 2;
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
	auto callback = [&result, &target](const rsdos_dirent &dirent)
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

void coco_rsdos_impl::iterate_directory_entries(const std::function<bool(const rsdos_dirent &dirent)> &callback)
{
	bool done = false;
	for (int dir_sector = 3; !done && dir_sector <= 18; dir_sector++)
	{
		// read this directory sector
		auto dir_block = read_sector(17, dir_sector);
		const rsdos_dirent_sector &sector = *reinterpret_cast<const rsdos_dirent_sector *>(dir_block.rodata());

		// and loop through all entries
		for (const auto &ent : sector.m_entries)
		{
			// 0xFF marks the end of the directory
			if (ent.m_dirent.m_filename[0] == '\xFF')
			{
				done = true;
			}
			else
			{
				// 0x00 marks a deleted file
				if (ent.m_dirent.m_filename[0] != '\0')
					done = callback(ent.m_dirent);
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
	std::string_view stem = strtrimrightspace(std::string_view(&dirent.m_filename[0], 8));
	std::string_view ext = strtrimrightspace(std::string_view(&dirent.m_filename[8], 3));
	return util::string_format("%s.%s", stem, ext);
}


//-------------------------------------------------
//  coco_rsdos_impl::granule_iterator ctor
//-------------------------------------------------

coco_rsdos_impl::granule_iterator::granule_iterator(coco_rsdos_impl &fs, const rsdos_dirent &dirent)
	: m_granule_map(fs.read_sector(17, 2))
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
			byte_count = 9 * 256;
			next_granule = granule_map_data[*m_current_granule];

			// check for cycles, which should only happen if the disk is corrupt (or not in RS-DOS format)
			if (m_visited_granules[*next_granule])
				next_granule = std::nullopt;    // this is corrupt!
			else
				m_visited_granules.set(*next_granule);
		}
		else if (granule_map_data[*m_current_granule] >= 0xC0 && granule_map_data[*m_current_granule] <= 0xC9)
		{
			// this is the last granule in the file
			success = true;
			granule = *m_current_granule;
			u16 sector_count = std::max(granule_map_data[*m_current_granule], u8(0xC1)) - 0xC1;
			byte_count = sector_count * 256 + m_last_sector_bytes;
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

