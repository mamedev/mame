// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

    fs_hplif.cpp

    Management of HP LIF images

***************************************************************************/

#include "fs_hplif.h"
#include "fsblk.h"
#include "hp300_dsk.h"

#include "corestr.h"
#include "osdcomm.h"
#include "strformat.h"

#include <array>
#include <optional>
#include <set>
#include <string_view>
#include <tuple>
#include <iostream>

namespace fs {
	const hplif_image HPLIF;
};

using namespace fs;

namespace {

class impl : public filesystem_t {
public:

	struct hplif_time {
		u8 year;
		u8 month;
		u8 day;
		u8 hour;
		u8 minute;
		u8 second;
	};

	struct hplif_dirent
	{
		char        m_file_name[10];
		u16         m_file_type;
		u32         m_starting_sector;
		u32         m_sector_count;
		hplif_time  m_time;
		u16         m_volume_number;
		u32         m_general_purpose;
	};

	class block_iterator
	{
	public:
		block_iterator(const impl &fs, u32 first_sector, u32 sector_count);
		bool next();
		const void *data() const;
		const std::array<hplif_dirent, 8> &dirent_data() const;
		u8 size() const;

	private:
		const impl &        m_fs;
		fsblk_t::block_t    m_block;
		u8                  m_sector;
		u32                 m_sector_count;
	};

	impl(fsblk_t &blockdev);
	virtual ~impl() = default;

	virtual meta_data volume_metadata() override;
	virtual std::pair<err_t, meta_data> metadata(const std::vector<std::string> &path) override;
	virtual std::pair<err_t, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;
	virtual std::pair<err_t, std::vector<u8>> file_read(const std::vector<std::string> &path) override;

private:
	fsblk_t::block_t read_sector(u32 starting_sector) const;
	std::optional<hplif_dirent> dirent_from_path(const std::vector<std::string> &path) const;
	void iterate_directory_entries(const std::function<bool(const hplif_dirent &dirent)> &callback) const;
	util::arbitrary_datetime decode_datetime(const hplif_time *time) const;
	meta_data metadata_from_dirent(const hplif_dirent &dirent) const;
	err_t format(const meta_data &meta) override;
};

// methods
std::string_view strtrimright_hplif(std::string_view str);
template<size_t N> std::string_view strtrimright_hplif(const char (&str)[N]);

} // anonymous namespace


//-------------------------------------------------
//  name
//-------------------------------------------------

const char *fs::hplif_image::name() const
{
	return "hplif";
}


//-------------------------------------------------
//  description
//-------------------------------------------------

const char *fs::hplif_image::description() const
{
	return "HPLIF";
}


//-------------------------------------------------
//  enumerate_f
//-------------------------------------------------

void fs::hplif_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_HP300_FORMAT, floppy_image::FF_35, floppy_image::DSDD,  630784,  "hp_lif_9121_format_1",   "HP 9121 LIF 3.5\" dual-sided double density Format 1");
	fe.add(FLOPPY_HP300_FORMAT, floppy_image::FF_35, floppy_image::DSDD,  709632,  "hp_lif_9121_format_2",   "HP 9121 LIF 3.5\" dual-sided double density Format 2");
	fe.add(FLOPPY_HP300_FORMAT, floppy_image::FF_35, floppy_image::DSDD,  788480,  "hp_lif_9121_format_3",   "HP 9121 LIF 3.5\" dual-sided double density Format 3");
	fe.add(FLOPPY_HP300_FORMAT, floppy_image::FF_35, floppy_image::SSDD,  270336,  "hp_lif_9121_format_4",   "HP 9121 LIF 3.5\" single-sided double density Format 4");
	fe.add(FLOPPY_HP300_FORMAT, floppy_image::FF_35, floppy_image::DSDD,  737280,  "hp_lif_9121_format_16",  "HP 9121 LIF 3.5\" dual-sided double density Format 16");

	fe.add(FLOPPY_HP300_FORMAT, floppy_image::FF_35, floppy_image::DSHD, 1261568, "hp_lif_9122_format_014", "HP 9122 LIF 3.5\" dual-sided high density Format 0, 1, 4");
	fe.add(FLOPPY_HP300_FORMAT, floppy_image::FF_35, floppy_image::DSHD, 1419264, "hp_lif_9122_format_2",   "HP 9122 LIF 3.5\" dual-sided high density Format 2");
	fe.add(FLOPPY_HP300_FORMAT, floppy_image::FF_35, floppy_image::DSHD, 1576960, "hp_lif_9122_format_3",   "HP 9122 LIF 3.5\" dual-sided high density Format 3");
	fe.add(FLOPPY_HP300_FORMAT, floppy_image::FF_35, floppy_image::DSHD, 1474560, "hp_lif_9122_format_16" , "HP 9122 LIF 3.5\" dual-sided high density Format 16");
}


//-------------------------------------------------
//  can_format
//-------------------------------------------------

bool fs::hplif_image::can_format() const
{
	return true;
}


//-------------------------------------------------
//  can_read
//-------------------------------------------------

bool fs::hplif_image::can_read() const
{
	return true;
}


//-------------------------------------------------
//  can_write
//-------------------------------------------------

bool fs::hplif_image::can_write() const
{
	return false;
}


//-------------------------------------------------
//  has_rsrc
//-------------------------------------------------

bool fs::hplif_image::has_rsrc() const
{
	return false;
}


//-------------------------------------------------
//  volume_meta_description
//-------------------------------------------------

std::vector<meta_description> fs::hplif_image::volume_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_name::name, "", false, [](const meta_value &m) { return m.as_string().size() <= 6; }, "Volume name, up to 6 characters");
	results.emplace_back(meta_name::creation_date, 0, true, nullptr, "Time of creation");
	return results;
}


//-------------------------------------------------
//  file_meta_description
//-------------------------------------------------

std::vector<meta_description> fs::hplif_image::file_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_name::name, "", false, [](const meta_value &m) { return m.as_string().size() <= 10; }, "File name, up to 10 characters");
	results.emplace_back(meta_name::file_type, "", true, nullptr, "Type of the file");
	results.emplace_back(meta_name::length, 0, true, nullptr, "Size of the file in bytes");
	results.emplace_back(meta_name::modification_date, 0, true, nullptr, "Time of last modification");
	return results;
}


//-------------------------------------------------
//  mount
//-------------------------------------------------

std::unique_ptr<filesystem_t> fs::hplif_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<impl>(blockdev);
}


//-------------------------------------------------
//  strtrimright_cbm
//-------------------------------------------------

namespace {

std::string_view strtrimright_hplif(std::string_view str)
{
	return strtrimright(str, [](char c) { return c != (char)0x20; });
}


//-------------------------------------------------
//  strtrimright_cbm
//-------------------------------------------------

template<size_t N>
std::string_view strtrimright_hplif(const char (&str)[N])
{
	std::string_view sv(str, std::size(str));
	return strtrimright_hplif(sv);
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
	auto block = read_sector(0);
	std::string_view disk_name = std::string_view((const char *) block.rodata() + 2, 6);

	meta_data results;
	results.set(meta_name::name, strtrimright_hplif(disk_name));
	results.set(meta_name::creation_date, decode_datetime(reinterpret_cast<const hplif_time *>(block.rodata() + 36)));
	return results;
}


//-------------------------------------------------
//  impl::metadata
//-------------------------------------------------

std::pair<err_t, meta_data> impl::metadata(const std::vector<std::string> &path)
{
	std::optional<hplif_dirent> dirent = dirent_from_path(path);
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
	auto callback = [this, &results](const hplif_dirent &ent)
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
	std::optional<hplif_dirent> dirent = dirent_from_path(path);
	if (!dirent)
		return std::make_pair(ERR_NOT_FOUND, std::vector<u8>());

	// and get the data
	u32 sector_count = big_endianize_int32(dirent->m_sector_count);
	hplif_dirent hdr = dirent.value();
	std::vector<u8> result;
	result.insert(
			result.end(),
			reinterpret_cast<const u8 *>(&hdr),
			reinterpret_cast<const u8 *>(&hdr) + 32);
	result.reserve(sector_count * 256 + 32);
	block_iterator iter(*this, big_endianize_int32(dirent->m_starting_sector), sector_count);
	while (iter.next())
		result.insert(result.end(), (const u8 *)iter.data(), (const u8 *)iter.data() + 256);

	return std::make_pair(ERR_OK, std::move(result));
}


//-------------------------------------------------
//  impl::read_sector
//-------------------------------------------------

fsblk_t::block_t impl::read_sector(u32 sector) const
{
	return m_blockdev.get(sector);
}


//-------------------------------------------------
//  impl::dirent_from_path
//-------------------------------------------------

std::optional<impl::hplif_dirent> impl::dirent_from_path(const std::vector<std::string> &path) const
{
	if (path.size() != 1)
		return { };
	std::string_view path_part = path[0];

	std::optional<hplif_dirent> result;
	auto callback = [&result, path_part](const hplif_dirent &dirent)
	{
		bool found = strtrimright_hplif(dirent.m_file_name) == path_part;
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

void impl::iterate_directory_entries(const std::function<bool(const hplif_dirent &dirent)> &callback) const
{
	fsblk_t::block_t block = m_blockdev.get(0);
	block_iterator iter(*this, block.r32b(8), block.r32b(16));

	if (block.r16b(0) != 0x8000)
	{
		return;
	}

	while (iter.next())
	{
		for (const hplif_dirent &ent : iter.dirent_data())
		{
			if (ent.m_file_type == 0xffff)
			{
				return;
			}

			if (ent.m_file_type != 0x0000)
			{
				if (callback(ent))
					return;
			}
		}
	}
}

//-------------------------------------------------
//  impl::decode_datetime
//-------------------------------------------------

util::arbitrary_datetime impl::decode_datetime(const hplif_time *time) const
{
	util::arbitrary_datetime result;
	memset(&result, 0, sizeof(result));

	result.year         = bcd_2_dec(time->year) + 1900;
	result.month        = bcd_2_dec(time->month);
	result.day_of_month = bcd_2_dec(time->day);
	result.hour         = bcd_2_dec(time->hour);
	result.minute       = bcd_2_dec(time->minute);
	result.second       = bcd_2_dec(time->second);
	return result;
}

//-------------------------------------------------
//  impl::metadata_from_dirent
//-------------------------------------------------

meta_data impl::metadata_from_dirent(const hplif_dirent &dirent) const
{
	std::string file_type = util::string_format("0x%04X", big_endianize_int16(dirent.m_file_type));

	// build the metadata and return it
	meta_data result;
	result.set(meta_name::name, strtrimright_hplif(dirent.m_file_name));
	result.set(meta_name::file_type, std::move(file_type));
	result.set(meta_name::length, big_endianize_int32(dirent.m_sector_count) * 256);
	result.set(meta_name::modification_date, decode_datetime(&dirent.m_time));
	return result;
}


//-------------------------------------------------
//  impl::block_iterator ctor
//-------------------------------------------------

impl::block_iterator::block_iterator(const impl &fs, u32 starting_sector, u32 sector_count)
	: m_fs(fs)
	, m_sector(starting_sector)
	, m_sector_count(sector_count)
{
}


//-------------------------------------------------
//  impl::block_iterator::next
//-------------------------------------------------

bool impl::block_iterator::next()
{
	bool result;
	if (m_sector_count != 0x00)
	{
		m_block = m_fs.read_sector(m_sector++);
		m_sector_count--;
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
	return m_block.rodata();
}

//-------------------------------------------------
//  impl::format
//-------------------------------------------------

err_t impl::format(const meta_data &meta)
{
	std::string volume_name = meta.get_string(meta_name::name, "B9826 ");
	fsblk_t::block_t block = m_blockdev.get(0);

	if (volume_name.size() < 6)
		volume_name.insert(volume_name.end(), 6 - volume_name.size(), ' ');
	if (volume_name.size() > 6)
		volume_name.resize(6);

	block.w16b(0, 0x8000);  // LIF magic
	block.wstr(2, volume_name);
	block.w32b(8, 2);       // directory start
	block.w16b(12, 0x1000); // LIF identifier
	block.w32b(16, 14);     // directory size
	block.w16b(20, 1);      // LIF version
	return ERR_OK;
}

//-------------------------------------------------
//  impl::block_iterator::dirent_data
//-------------------------------------------------

const std::array<impl::hplif_dirent, 8> &impl::block_iterator::dirent_data() const
{
	return *reinterpret_cast<const std::array<impl::hplif_dirent, 8> *>(m_block.rodata());
}

} // anonymous namespace
