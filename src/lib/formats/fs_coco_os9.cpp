// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    fs_coco_os9.cpp

    Management of CoCo OS-9 floppy images

    OS-9 Level 2 Technical Reference, Chapter 5, Random Block File Manager,
    page 2

    http://www.colorcomputerarchive.com/coco/Documents/Manuals/Operating Systems/OS-9 Level 2 Manual (Tandy).pdf

***************************************************************************/

#include "fs_coco_os9.h"
#include "coco_rawdsk.h"
#include "strformat.h"

namespace fs {

const coco_os9_image COCO_OS9;


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  name
//-------------------------------------------------

const char *coco_os9_image::name() const
{
	return "coco_os9";
}


//-------------------------------------------------
//  description
//-------------------------------------------------

const char *coco_os9_image::description() const
{
	return "CoCo OS-9";
}


//-------------------------------------------------
//  enumerate_f
//-------------------------------------------------

void coco_os9_image::enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const
{
	if (has(form_factor, variants, floppy_image::FF_525, floppy_image::SSDD))
	{
		fe.add(FLOPPY_COCO_RAWDSK_FORMAT, 161280, "coco_rawdsk_os9_35", "CoCo Raw Disk OS-9 single-sided 35 tracks");
		fe.add(FLOPPY_COCO_RAWDSK_FORMAT, 184320, "coco_rawdsk_os9_40", "CoCo Raw Disk OS-9 single-sided 40 tracks");
	}
}


//-------------------------------------------------
//  can_format
//-------------------------------------------------

bool coco_os9_image::can_format() const
{
	return false;
}


//-------------------------------------------------
//  can_read
//-------------------------------------------------

bool coco_os9_image::can_read() const
{
	return true;
}


//-------------------------------------------------
//  can_write
//-------------------------------------------------

bool coco_os9_image::can_write() const
{
	return false;
}


//-------------------------------------------------
//  has_rsrc
//-------------------------------------------------

bool coco_os9_image::has_rsrc() const
{
	return false;
}


//-------------------------------------------------
//  directory_separator
//-------------------------------------------------

char coco_os9_image::directory_separator() const
{
	return '/';
}


//-------------------------------------------------
//  volume_meta_description
//-------------------------------------------------

std::vector<meta_description> coco_os9_image::volume_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_description(meta_name::name, meta_type::string, "UNTITLED", false, [](const meta_value &m) { return m.as_string().size() <= 32; }, "Volume name, up to 32 characters"));
	return results;
}


//-------------------------------------------------
//  file_meta_description
//-------------------------------------------------

std::vector<meta_description> coco_os9_image::file_meta_description() const
{
	return entity_meta_description();
}


//-------------------------------------------------
//  directory_meta_description
//-------------------------------------------------

std::vector<meta_description> coco_os9_image::directory_meta_description() const
{
	return entity_meta_description();
}


//-------------------------------------------------
//  entity_meta_description
//-------------------------------------------------

std::vector<meta_description> coco_os9_image::entity_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_description(meta_name::name, meta_type::string, "", false, [](const meta_value &m) { return validate_filename(m.as_string()); }, "File name"));
	results.emplace_back(meta_description(meta_name::creation_date, meta_type::date, util::arbitrary_datetime::now(), false, nullptr, "Creation time"));
	results.emplace_back(meta_description(meta_name::owner_id, meta_type::number, 0, true, nullptr, "Owner ID"));
	results.emplace_back(meta_description(meta_name::attributes, meta_type::string, 0, true, nullptr, "File attributes"));
	results.emplace_back(meta_description(meta_name::length, meta_type::number, 0, true, nullptr, "Size of the file in bytes"));
	return results;
}


//-------------------------------------------------
//  mount
//-------------------------------------------------

std::unique_ptr<filesystem_t> coco_os9_image::mount(fsblk_t &blockdev) const
{
	// read the header block
	blockdev.set_block_size(256);
	volume_header header(blockdev.get(0));

	// sanity checks
	if (header.sectors_per_track() != header.track_size_in_sectors())
		return { };

	// create the implementation
	return std::make_unique<impl>(blockdev, std::move(header));
}


//-------------------------------------------------
//  pick_os9_string
//-------------------------------------------------

std::string coco_os9_image::pick_os9_string(std::string_view raw_string)
{
	// find the last NUL or high bit character
	auto iter = std::find_if(raw_string.begin(), raw_string.end(), [](char ch)
	{
		return ch == '\0' || ch & 0x80;
	});

	// get the preliminary result
	std::string result(raw_string.begin(), iter);

	// and add the final character if we have to
	if (iter < raw_string.end() && *iter & 0x80)
		result.append(1, *iter & 0x7F);
	return result;

}


//-------------------------------------------------
//  pick_integer_be
//-------------------------------------------------

u32 coco_os9_image::pick_integer_be(const u8 *data, int length)
{
	u32 result = 0;
	for (int i = 0; i < length; i++)
		result |= u32(data[length - i - 1]) << i * 8;
	return result;
}


//-------------------------------------------------
//  validate_filename
//-------------------------------------------------

bool coco_os9_image::validate_filename(std::string_view name)
{
	return !is_ignored_filename(name)
		&& name.size() <= 29
		&& std::find_if(name.begin(), name.end(), [](const char ch) { return ch == '\0' || ch == '/' || ch >= 0x80; }) == name.end();
}


//-------------------------------------------------
//  is_ignored_filename - should this file name be
//  ignored if it is in the file system?
//-------------------------------------------------

bool coco_os9_image::is_ignored_filename(std::string_view name)
{
	return name.empty()
		|| name[0] == '\0'
		|| name == "."
		|| name == "..";
}


//-------------------------------------------------
//  volume_header ctor
//-------------------------------------------------

coco_os9_image::volume_header::volume_header(fsblk_t::block_t &&block)
	: m_block(std::move(block))
{
}


//-------------------------------------------------
//  volume_header::name
//-------------------------------------------------

std::string coco_os9_image::volume_header::name() const
{
	std::string_view raw_name((const char *)&m_block.rodata()[31], 32);
	return pick_os9_string(raw_name);
}


//-------------------------------------------------
//  file_header ctor
//-------------------------------------------------

coco_os9_image::file_header::file_header(fsblk_t::block_t &&block)
	: m_block(std::move(block))
{
}


//-------------------------------------------------
//  file_header::creation_date
//-------------------------------------------------

util::arbitrary_datetime coco_os9_image::file_header::creation_date() const
{
	util::arbitrary_datetime dt;
	memset(&dt, 0, sizeof(dt));
	dt.year         = 1900 + m_block.r8(13);
	dt.month        = m_block.r8(14);
	dt.day_of_month = m_block.r8(15);
	return dt;
}


//-------------------------------------------------
//  file_header::metadata
//-------------------------------------------------

meta_data coco_os9_image::file_header::metadata() const
{
	// format the attributes
	std::string attributes = util::string_format("%c%c%c%c%c%c%c%c",
		is_directory()      ? 'd' : '-',
		is_non_sharable()   ? 's' : '-',
		is_public_execute() ? 'x' : '-',
		is_public_write()   ? 'w' : '-',
		is_public_read()    ? 'r' : '-',
		is_user_execute()   ? 'x' : '-',
		is_user_write()     ? 'w' : '-',
		is_user_read()      ? 'r' : '-');

	meta_data result;
	result.set(meta_name::creation_date,    creation_date());
	result.set(meta_name::owner_id,         owner_id());
	result.set(meta_name::attributes,       std::move(attributes));
	result.set(meta_name::length,           file_size());
	return result;
}


//-------------------------------------------------
//  file_header::get_sector_map_entry_count
//-------------------------------------------------

int coco_os9_image::file_header::get_sector_map_entry_count() const
{
	return (m_block.size() - 16) / 5;
}


//-------------------------------------------------
//  file_header::get_sector_map_entry
//-------------------------------------------------

void coco_os9_image::file_header::get_sector_map_entry(int entry_number, u32 &start_lsn, u16 &count) const
{
	start_lsn   = m_block.r24b(16 + (entry_number * 5) + 0);
	count       = m_block.r16b(16 + (entry_number * 5) + 3);
}


//-------------------------------------------------
//  impl ctor
//-------------------------------------------------

coco_os9_image::impl::impl(fsblk_t &blockdev, volume_header &&header)
	: filesystem_t(blockdev, 256)
	, m_volume_header(std::move(header))
{
}


//-------------------------------------------------
//  impl::metadata
//-------------------------------------------------

meta_data coco_os9_image::impl::metadata()
{
	meta_data results;
	results.set(meta_name::name, m_volume_header.name());
	return results;
}


//-------------------------------------------------
//  impl::root
//-------------------------------------------------

filesystem_t::dir_t coco_os9_image::impl::root()
{
	if (!m_root)
		m_root = open_directory(m_volume_header.root_dir_lsn());
	return m_root.strong();
}


//-------------------------------------------------
//  impl::drop_root_ref
//-------------------------------------------------

void coco_os9_image::impl::drop_root_ref()
{
	m_root = nullptr;
}


//-------------------------------------------------
//  impl::open_directory
//-------------------------------------------------

coco_os9_image::impl::directory *coco_os9_image::impl::open_directory(u32 lsn)
{
	file_header header(m_blockdev.get(lsn));
	return new directory(*this, std::move(header));
}


//-------------------------------------------------
//  impl::read_file_data
//-------------------------------------------------

std::vector<u8> coco_os9_image::impl::read_file_data(const file_header &header) const
{
	std::vector<u8> data;
	data.reserve(header.file_size());
	int entry_count = header.get_sector_map_entry_count();
	for (int i = 0; i < entry_count; i++)
	{
		u32 start_lsn;
		u16 count;
		header.get_sector_map_entry(i, start_lsn, count);

		for (u32 lsn = start_lsn; lsn < start_lsn + count; lsn++)
		{
			auto block = m_blockdev.get(lsn);
			size_t block_size = std::min(std::min(u32(m_volume_header.sector_size()), block.size()), header.file_size() - u32(data.size()));
			for (auto i = 0; i < block_size; i++)
				data.push_back(block.rodata()[i]);
		}
	}
	return data;
}


//-------------------------------------------------
//  file ctor
//-------------------------------------------------

coco_os9_image::impl::file::file(impl &i, file_header &&file_header)
	: m_fs(i)
	, m_file_header(std::move(file_header))
{
}


//-------------------------------------------------
//  file::drop_weak_references
//-------------------------------------------------

void coco_os9_image::impl::file::drop_weak_references()
{
}


//-------------------------------------------------
//  file::metadata
//-------------------------------------------------

meta_data coco_os9_image::impl::file::metadata()
{
	return m_file_header.metadata();
}


//-------------------------------------------------
//  file::read_all
//-------------------------------------------------

std::vector<u8> coco_os9_image::impl::file::read_all()
{
	return m_fs.read_file_data(m_file_header);
}


//-------------------------------------------------
//  directory ctor
//-------------------------------------------------

coco_os9_image::impl::directory::directory(impl &i, file_header &&file_header)
	: m_fs(i)
	, m_file_header(std::move(file_header))
{
}


//-------------------------------------------------
//  directory::drop_weak_references
//-------------------------------------------------

void coco_os9_image::impl::directory::drop_weak_references()
{
}


//-------------------------------------------------
//  directory::metadata
//-------------------------------------------------

meta_data coco_os9_image::impl::directory::metadata()
{
	return m_file_header.metadata();
}


//-------------------------------------------------
//  directory::contents
//-------------------------------------------------

std::vector<dir_entry> coco_os9_image::impl::directory::contents()
{
	// read the directory data
	std::vector<u8> directory_data = m_fs.read_file_data(m_file_header);

	// and assemble results
	std::vector<dir_entry> results;
	int directory_count = directory_data.size() / 32;
	for (int i = 0; i < directory_count; i++)
	{
		// determine the filename
		std::string_view raw_filename((const char *) &directory_data[i * 32], 29);
		std::string filename = pick_os9_string(raw_filename);
		if (is_ignored_filename(filename))
			continue;

		// determine the entry type
		u32 lsn = pick_integer_be(&directory_data[i * 32] + 29, 3);
		file_header file_header(m_fs.m_blockdev.get(lsn));
		dir_entry_type entry_type = file_header.is_directory()
			? dir_entry_type::dir
			: dir_entry_type::file;

		// and return the results
		results.emplace_back(std::move(filename), entry_type, lsn);
	}
	return results;
}


//-------------------------------------------------
//  directory::file_get
//-------------------------------------------------

filesystem_t::file_t coco_os9_image::impl::directory::file_get(u64 key)
{
	file_header header(m_fs.m_blockdev.get(u32(key)));
	return file_t(new file(m_fs, std::move(header)));
}


//-------------------------------------------------
//  directory::dir_get
//-------------------------------------------------

filesystem_t::dir_t coco_os9_image::impl::directory::dir_get(u64 key)
{
	return dir_t(m_fs.open_directory(u32(key)));
}

} // namespace fs
