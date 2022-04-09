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

#include <optional>
#include <string_view>

namespace fs {

const coco_rsdos_image COCO_RSDOS;

const char *coco_rsdos_image::name() const
{
	return "coco_rsdos";
}

const char *coco_rsdos_image::description() const
{
	return "CoCo RS-DOS";
}

void coco_rsdos_image::enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const
{
	if (has(form_factor, variants, floppy_image::FF_525, floppy_image::SSDD))
	{
		fe.add(FLOPPY_COCO_RAWDSK_FORMAT, 161280, "coco_rawdsk_rsdos_35", "CoCo Raw Disk RS-DOS single-sided 35 tracks");
		fe.add(FLOPPY_COCO_RAWDSK_FORMAT, 184320, "coco_rawdsk_rsdos_40", "CoCo Raw Disk RS-DOS single-sided 40 tracks");
	}
}

bool coco_rsdos_image::can_format() const
{
	return true;
}

bool coco_rsdos_image::can_read() const
{
	return true;
}

bool coco_rsdos_image::can_write() const
{
	return false;
}

bool coco_rsdos_image::has_rsrc() const
{
	return false;
}

std::vector<meta_description> coco_rsdos_image::file_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_description(meta_name::name, meta_type::string, "", false, [](const meta_value &m) { return validate_filename(m.as_string()); }, "File name, 8.3"));
	results.emplace_back(meta_description(meta_name::file_type, meta_type::number, 0, true, nullptr, "Type of the file"));
	results.emplace_back(meta_description(meta_name::ascii_flag, meta_type::string, 0, true, nullptr, "Ascii or binary flag"));
	results.emplace_back(meta_description(meta_name::size_in_blocks, meta_type::number, 0, true, nullptr, "Number of granules used by the file"));
	results.emplace_back(meta_description(meta_name::length, meta_type::number, 0, true, nullptr, "Size of the file in bytes"));
	return results;
}

std::unique_ptr<filesystem_t> coco_rsdos_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<impl>(blockdev);
}

bool coco_rsdos_image::validate_filename(std::string_view name)
{
	auto pos = name.find('.');
	auto stem_length = pos != std::string::npos ? pos : name.size();
	auto ext_length = pos != std::string::npos ? name.size() - pos - 1 : 0;
	return stem_length > 0 && stem_length <= 8 && ext_length <= 3;
}

coco_rsdos_image::impl::impl(fsblk_t &blockdev)
	: filesystem_t(blockdev, 256)
{
}

meta_data coco_rsdos_image::impl::metadata()
{
	return meta_data();
}

filesystem_t::dir_t coco_rsdos_image::impl::root()
{
	if (!m_root)
		m_root = new root_dir(*this);
	return m_root.strong();
}

void coco_rsdos_image::impl::drop_root_ref()
{
	m_root = nullptr;
}

void coco_rsdos_image::impl::format(const meta_data &meta)
{
	// formatting RS-DOS is easy - just fill everything with 0xFF
	m_blockdev.fill(0xFF);
}

fsblk_t::block_t coco_rsdos_image::impl::read_sector(int track, int sector) const
{
	// the CoCo RS-DOS world thinks in terms of tracks/sectors, but we have a block device
	// abstraction
	return m_blockdev.get(track * 18 + sector - 1);
}

u8 coco_rsdos_image::impl::maximum_granules() const
{
	u32 sector_count = m_blockdev.block_count();
	u32 granule_count = (sector_count / 9) - 2;
	return granule_count <= 0xFF ? u8(granule_count) : 0xFF;
}

std::string coco_rsdos_image::impl::get_filename_from_dirent(const rsdos_dirent &dirent)
{
	std::string_view stem = strtrimrightspace(std::string_view(&dirent.m_filename[0], 8));
	std::string_view ext = strtrimrightspace(std::string_view(&dirent.m_filename[8], 3));
	return util::string_format("%s.%s", stem, ext);
}

void coco_rsdos_image::impl::root_dir::drop_weak_references()
{
	m_fs.drop_root_ref();
}

meta_data coco_rsdos_image::impl::root_dir::metadata()
{
	return meta_data();
}

std::vector<dir_entry> coco_rsdos_image::impl::root_dir::contents()
{
	u64 key = 0;
	std::vector<dir_entry> results;
	for (int dir_sector = 3; dir_sector <= 18; dir_sector++)
	{
		// read this directory sector
		auto dir_block = m_fs.read_sector(17, dir_sector);
		const rsdos_dirent_sector &sector = *reinterpret_cast<const rsdos_dirent_sector *>(dir_block.rodata());

		// and loop through all entries
		for (const auto &ent : sector.m_entries)
		{
			// 0xFF marks the end of the directory
			if (ent.m_dirent.m_filename[0] == '\xFF')
				return results;

			// 0x00 marks a deleted file
			if (ent.m_dirent.m_filename[0] != '\0')
				results.emplace_back(get_filename_from_dirent(ent.m_dirent), dir_entry_type::file, key);

			key++;
		}
	}
	return results;
}

filesystem_t::file_t coco_rsdos_image::impl::root_dir::file_get(u64 key)
{
	auto dir_block = m_fs.read_sector(17, 3 + key / 4);
	const rsdos_dirent_sector &sector = *reinterpret_cast<const rsdos_dirent_sector *>(dir_block.rodata());
	const rsdos_dirent &ent = sector.m_entries[key % 4].m_dirent;
	return file_t(new file(m_fs, rsdos_dirent(ent)));
}

filesystem_t::dir_t coco_rsdos_image::impl::root_dir::dir_get(u64 key)
{
	throw std::logic_error("Directories not supported");
}

coco_rsdos_image::impl::granule_iterator::granule_iterator(impl &fs, const rsdos_dirent &dirent)
	: m_granule_map(fs.read_sector(17, 2))
	, m_current_granule(dirent.m_first_granule)
	, m_maximum_granules(fs.maximum_granules())
	, m_last_sector_bytes((u16(dirent.m_last_sector_bytes_msb) << 8) | dirent.m_last_sector_bytes_lsb)
{
}

bool coco_rsdos_image::impl::granule_iterator::next(u8 &granule, u16 &byte_count)
{
	bool success = false;
	granule = ~0;
	byte_count = 0;

	if (m_current_granule)
	{
		const u8 *granule_map_data = m_granule_map.rodata();
		if (granule_map_data[*m_current_granule] < m_maximum_granules)
		{
			// this entry points to the next granule
			success = true;
			granule = *m_current_granule;
			byte_count = 9 * 256;
			m_current_granule = granule_map_data[*m_current_granule];
		}
		else if (granule_map_data[*m_current_granule] >= 0xC0 && granule_map_data[*m_current_granule] <= 0xC9)
		{
			// this is the last granule in the file
			success = true;
			granule = *m_current_granule;
			u16 sector_count = std::max(granule_map_data[*m_current_granule], u8(0xC1)) - 0xC1;
			byte_count = sector_count * 256 + m_last_sector_bytes;
			m_current_granule = std::nullopt;
		}
		else
		{
			// should not happen; but we'll treat this as an EOF
			m_current_granule = std::nullopt;
		}
	}
	return success;
}

coco_rsdos_image::impl::file::file(impl &fs, rsdos_dirent &&dirent)
	: m_fs(fs)
	, m_dirent(std::move(dirent))
{
}

void coco_rsdos_image::impl::file::drop_weak_references()
{
}

meta_data coco_rsdos_image::impl::file::metadata()
{
	u32 file_size = 0;
	int granule_count = 0;

	// we need to iterate on the file to determine the size and granule/block count
	u8 granule;
	u16 byte_count;
	granule_iterator iter(m_fs, m_dirent);
	while (iter.next(granule, byte_count))
	{
		granule_count++;
		file_size += byte_count;
	}

	// turn the ASCII flag to a single character (this reflects what the user sees when doing a directory listing on a real CoCo)
	char file_type_char = 'B' + m_dirent.m_asciiflag;

	// build the metadata and return it
	meta_data result;
	result.set(meta_name::name,             get_filename_from_dirent(m_dirent));
	result.set(meta_name::file_type,        m_dirent.m_filetype);
	result.set(meta_name::ascii_flag,       std::string(1, file_type_char));
	result.set(meta_name::size_in_blocks,   granule_count);
	result.set(meta_name::length,           file_size);
	return result;
}

std::vector<u8> coco_rsdos_image::impl::file::read_all()
{
	std::vector<u8> result;

	u8 granule;
	u16 byte_count;
	granule_iterator iter(m_fs, m_dirent);
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
			auto block = m_fs.read_sector(track, sector);
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
	return result;
}

} // namespace fs
