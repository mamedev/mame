// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    fs_coco_os9.cpp

    Management of CoCo OS-9 floppy images

    OS-9 Level 2 Technical Reference, Chapter 5, Random Block File Manager,
    page 2

    https://colorcomputerarchive.com/repo/Documents/Manuals/Operating%20Systems/OS-9%20Level%202%20Manual%20(Tandy).pdf

***************************************************************************/

#include "fs_coco_os9.h"
#include "coco_rawdsk.h"
#include "fsblk.h"

#include "multibyte.h"
#include "strformat.h"

#include <optional>


using namespace fs;

namespace fs { const coco_os9_image COCO_OS9; }

namespace {

// ======================> coco_os9_impl

class coco_os9_impl : public filesystem_t
{
public:
	// ======================> volume_header

	class volume_header
	{
	public:
		volume_header(fsblk_t::block_t &&block);
		volume_header(const volume_header &) = delete;
		volume_header(volume_header &&) = default;

		u32 total_sectors() const { return m_block.r24b(0); }
		u8  track_size_in_sectors() const { return m_block.r8(3); }
		u16 allocation_bitmap_bytes() const { return m_block.r16b(4); }
		u16 cluster_size() const { return m_block.r16b(6); }
		u32 root_dir_lsn() const { return m_block.r24b(8); }
		u16 owner_id() const { return m_block.r16b(11); }
		u16 disk_id() const { return m_block.r16b(14); }
		u8  format_flags() const { return m_block.r8(16); }
		u16 sectors_per_track() const { return m_block.r16b(17); }
		u32 bootstrap_lsn() const { return m_block.r24b(21); }
		u16 bootstrap_size() const { return m_block.r16b(24); }
		util::arbitrary_datetime creation_date() const { return from_os9_date(m_block.r24b(26), m_block.r16b(29)); }
		u16 sector_size() const { u16 result = m_block.r16b(104); return result != 0 ? result : 256; }
		u8 sides() const { return (format_flags() & 0x01) ? 2 : 1; }
		bool double_density() const { return (format_flags() & 0x02) != 0; }
		bool double_track() const { return (format_flags() & 0x04) != 0; }
		bool quad_track_density() const { return (format_flags() & 0x08) != 0; }
		bool octal_track_density() const { return (format_flags() & 0x10) != 0; }

		std::string name() const;

	private:
		fsblk_t::block_t    m_block;
	};


	// ======================> file_header

	class file_header
	{
	public:
		file_header(fsblk_t::block_t &&block, std::string &&filename);
		file_header(const file_header &) = delete;
		file_header(file_header &&) = default;

		file_header &operator=(const file_header &) = delete;
		file_header &operator=(file_header &&) = default;

		u8  attributes() const { return m_block.r8(0); }
		u16 owner_id() const { return m_block.r16b(1); }
		u8  link_count() const { return m_block.r8(8); }
		u32 file_size() const { return m_block.r32b(9); }
		util::arbitrary_datetime creation_date() const;
		bool is_directory() const { return (attributes() & 0x80) != 0; }
		bool is_non_sharable() const { return (attributes() & 0x40) != 0; }
		bool is_public_execute() const { return (attributes() & 0x20) != 0; }
		bool is_public_write() const { return (attributes() & 0x10) != 0; }
		bool is_public_read() const { return (attributes() & 0x08) != 0; }
		bool is_user_execute() const { return (attributes() & 0x04) != 0; }
		bool is_user_write() const { return (attributes() & 0x02) != 0; }
		bool is_user_read() const { return (attributes() & 0x01) != 0; }

		meta_data metadata() const;
		int get_sector_map_entry_count() const;
		void get_sector_map_entry(int entry_number, u32 &start_lsn, u16 &count) const;

	private:
		fsblk_t::block_t    m_block;
		std::string         m_filename;
	};

	// ctor/dtor
	coco_os9_impl(fsblk_t &blockdev, volume_header &&header);
	virtual ~coco_os9_impl() = default;

	virtual meta_data volume_metadata() override;
	virtual std::pair<err_t, meta_data> metadata(const std::vector<std::string> &path) override;
	virtual std::pair<err_t, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;
	virtual std::pair<err_t, std::vector<u8>> file_read(const std::vector<std::string> &path) override;
	virtual err_t format(const meta_data &meta) override;

	std::optional<file_header> find(const std::vector<std::string> &path, std::optional<dir_entry_type> expected_entry_type) const;
	void iterate_directory_entries(const file_header &header, const std::function<bool(std::string &&, u32)> callback) const;
	std::vector<u8> read_file_data(const file_header &header) const;

	static std::string pick_os9_string(std::string_view raw_string);
	static std::string to_os9_string(std::string_view s, size_t length);
	static util::arbitrary_datetime from_os9_date(u32 os9_date, u16 os9_time = 0);
	static std::tuple<u32, u16> to_os9_date(const util::arbitrary_datetime &datetime);
	static bool is_ignored_filename(std::string_view name);
	static bool validate_filename(std::string_view name);

private:
	volume_header   m_volume_header;
};

} // anonymous namespace

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

void coco_os9_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_COCO_RAWDSK_FORMAT, floppy_image::FF_525, floppy_image::SSSD, 161280, "coco_rawdsk_os9_35", "CoCo Raw Disk OS-9 single-sided 35 tracks");
	fe.add(FLOPPY_COCO_RAWDSK_FORMAT, floppy_image::FF_525, floppy_image::SSSD, 184320, "coco_rawdsk_os9_40", "CoCo Raw Disk OS-9 single-sided 40 tracks");
}


//-------------------------------------------------
//  can_format
//-------------------------------------------------

bool coco_os9_image::can_format() const
{
	return true;
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
	results.emplace_back(meta_description(meta_name::name, "UNTITLED", false, [](const meta_value &m) { return m.as_string().size() <= 32; }, "Volume name, up to 32 characters"));
	results.emplace_back(meta_description(meta_name::creation_date, util::arbitrary_datetime::now(), false, nullptr, "Creation time"));
	return results;
}


//-------------------------------------------------
//  file_meta_description
//-------------------------------------------------

std::vector<meta_description> coco_os9_image::file_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_description(meta_name::name, "", false, [](const meta_value &m) { return coco_os9_impl::validate_filename(m.as_string()); }, "File name"));
	results.emplace_back(meta_description(meta_name::creation_date, util::arbitrary_datetime::now(), false, nullptr, "Creation time"));
	results.emplace_back(meta_description(meta_name::owner_id, 0, true, nullptr, "Owner ID"));
	results.emplace_back(meta_description(meta_name::attributes, "", true, nullptr, "File attributes"));
	results.emplace_back(meta_description(meta_name::length, 0, true, nullptr, "Size of the file in bytes"));
	return results;
}


//-------------------------------------------------
//  directory_meta_description
//-------------------------------------------------

std::vector<meta_description> coco_os9_image::directory_meta_description() const
{
	return file_meta_description();
}

//-------------------------------------------------
//  mount
//-------------------------------------------------

std::unique_ptr<filesystem_t> coco_os9_image::mount(fsblk_t &blockdev) const
{
	// read the header block
	blockdev.set_block_size(256);
	coco_os9_impl::volume_header header(blockdev.get(0));

	// sanity checks
	if (header.sectors_per_track() != header.track_size_in_sectors())
		return { };

	// create the implementation
	return std::make_unique<coco_os9_impl>(blockdev, std::move(header));
}


//-------------------------------------------------
//  impl ctor
//-------------------------------------------------

coco_os9_impl::coco_os9_impl(fsblk_t &blockdev, volume_header &&header)
	: filesystem_t(blockdev, 256)
	, m_volume_header(std::move(header))
{
}


//-------------------------------------------------
//  coco_os9_impl::volume_metadata
//-------------------------------------------------

meta_data coco_os9_impl::volume_metadata()
{
	meta_data results;
	results.set(meta_name::name, m_volume_header.name());
	results.set(meta_name::creation_date, m_volume_header.creation_date());
	return results;
}


//-------------------------------------------------
//  coco_os9_impl::metadata
//-------------------------------------------------

std::pair<err_t, meta_data> coco_os9_impl::metadata(const std::vector<std::string> &path)
{
	// look up the path
	std::optional<file_header> header = find(path, { });
	if (!header)
		return std::make_pair(ERR_NOT_FOUND, meta_data());

	return std::make_pair(ERR_OK, header->metadata());
}


//-------------------------------------------------
//  coco_os9_impl::directory_contents
//-------------------------------------------------

std::pair<err_t, std::vector<dir_entry>> coco_os9_impl::directory_contents(const std::vector<std::string> &path)
{
	// look up the path
	std::optional<file_header> header = find(path, dir_entry_type::dir);
	if (!header)
		return std::make_pair(ERR_NOT_FOUND, std::vector<dir_entry>());

	// iterate through the directory
	std::vector<dir_entry> results;
	auto callback = [this, &results](std::string &&filename, u32 lsn)
	{
		file_header header(m_blockdev.get(lsn), std::move(filename));
		dir_entry_type entry_type = header.is_directory()
			? dir_entry_type::dir
			: dir_entry_type::file;
		results.emplace_back(entry_type, header.metadata());
		return false;
	};
	iterate_directory_entries(*header, callback);

	// and we're done
	return std::make_pair(ERR_OK, std::move(results));
}


//-------------------------------------------------
//  coco_os9_impl::file_read
//-------------------------------------------------

std::pair<err_t, std::vector<u8>> coco_os9_impl::file_read(const std::vector<std::string> &path)
{
	// look up the path
	std::optional<file_header> header = find(path, dir_entry_type::file);
	if (!header)
		return std::make_pair(ERR_NOT_FOUND, std::vector<u8>());

	std::vector<u8> data = read_file_data(*header);
	return std::make_pair(ERR_OK, std::move(data));
}


//-------------------------------------------------
//  coco_os9_impl::format
//-------------------------------------------------

err_t coco_os9_impl::format(const meta_data &meta)
{
	// for some reason, the OS-9 world favored filling with 0xE5
	m_blockdev.fill(0xe5);

	// identify geometry info
	u8 sectors = 18;                // TODO - we need a definitive technique to get the floppy geometry
	u8 heads = 1;                   // TODO - we need a definitive technique to get the floppy geometry
	u16 sector_bytes = 256;         // TODO - we need a definitive technique to get the floppy geometry
	bool is_double_density = true;  // TODO - we need a definitive technique to get the floppy geometry
	u32 tracks = m_blockdev.block_count() / sectors / heads;

	// get attributes from metadata
	std::string volume_title = meta.get_string(meta_name::name, "UNTITLED");
	util::arbitrary_datetime creation_datetime = meta.get_date(meta_name::creation_date);
	auto [creation_os9date, creation_os9time] = to_os9_date(creation_datetime);

	u32 lsn_count = m_blockdev.block_count();
	u16 cluster_size = 1;
	u16 owner_id = 1;
	u16 disk_id = 1;
	u8 attributes = 0;
	u32 allocation_bitmap_bits = lsn_count / cluster_size;
	u32 allocation_bitmap_lsns = (allocation_bitmap_bits / 8 + sector_bytes - 1) / sector_bytes;
	u8 format_flags = ((heads > 1) ? 0x01 : 0x00)
		| (is_double_density ? 0x02 : 0x00);

	// volume header
	auto volume_header = m_blockdev.get(0);
	volume_header.fill(0x00);
	volume_header.w24b(0, lsn_count);                               // DD.TOT - total secctors
	volume_header.w8(3, sectors);                                   // DD.TKS - track size in sectors
	volume_header.w16b(4, (allocation_bitmap_bits + 7) / 8);        // DD.MAP - allocation bitmap in bytes
	volume_header.w16b(6, cluster_size);                            // DD.BIT - cluster size
	volume_header.w24b(8, 1 + allocation_bitmap_lsns);              // DD.DIR - root directory LSN
	volume_header.w16b(11, owner_id);                               // DD.OWN - owner ID
	volume_header.w8(13, attributes);                               // DD.ATT - Dattributes
	volume_header.w16b(14, disk_id);                                // DD.DSK - disk ID
	volume_header.w8(16, format_flags);                             // DD.FMT - format flags
	volume_header.w16b(17, sectors);                                // DD.SPT - sectors per track
	volume_header.w24b(26, creation_os9date);                       // DD.DAT - date of creation
	volume_header.w16b(29, creation_os9time);                       // DD.DAT - time of creation
	volume_header.wstr(31, to_os9_string(volume_title, 32));        // DD.NAM - title
	volume_header.w16b(103, sector_bytes / 256);                    // sector bytes

	// path descriptor options
	volume_header.w8(0x3f + 0x00, 1);                               // device class
	volume_header.w8(0x3f + 0x01, 1);                               // drive number
	volume_header.w8(0x3f + 0x03, 0x20);                            // device type
	volume_header.w8(0x3f + 0x04, 1);                               // density capability
	volume_header.w16b(0x3f + 0x05, tracks);                        // number of tracks
	volume_header.w8(0x3f + 0x07, heads);                           // number of sides
	volume_header.w16b(0x3f + 0x09, sectors);                       // sectors per track
	volume_header.w16b(0x3f + 0x0b, sectors);                       // sectors on track zero
	volume_header.w8(0x3f + 0x0d, 3);                               // sector interleave factor
	volume_header.w8(0x3f + 0x0e, 8);                               // default sectors per allocation

	// allocation bitmap
	u32 total_allocated_sectors = 1 + allocation_bitmap_lsns + 1 + 8;
	std::vector<u8> abblk_bytes;
	abblk_bytes.resize(sector_bytes);
	for (u32 i = 0; i < allocation_bitmap_lsns; i++)
	{
		for (u32 j = 0; j < sector_bytes; j++)
		{
			u32 pos = (i * sector_bytes + j) * 8;
			if (pos + 8 < total_allocated_sectors)
				abblk_bytes[j] = 0xff;
			else if (pos >= total_allocated_sectors)
				abblk_bytes[j] = 0x00;
			else
				abblk_bytes[j] = ~((1 << (8 - total_allocated_sectors + pos)) - 1);
		}

		auto abblk = m_blockdev.get(1 + i);
		abblk.copy(0, abblk_bytes.data(), sector_bytes);
	}

	// root directory header
	auto roothdr_blk = m_blockdev.get(1 + allocation_bitmap_lsns);
	roothdr_blk.fill(0x00);
	roothdr_blk.w8(0x00, 0xbf);
	roothdr_blk.w8(0x01, 0x00);
	roothdr_blk.w8(0x02, 0x00);
	roothdr_blk.w24b(0x03, creation_os9date);
	roothdr_blk.w16b(0x06, creation_os9time);
	roothdr_blk.w8(0x08, 0x01);
	roothdr_blk.w8(0x09, 0x00);
	roothdr_blk.w8(0x0a, 0x00);
	roothdr_blk.w8(0x0b, 0x00);
	roothdr_blk.w8(0x0c, 0x40);
	roothdr_blk.w24b(0x0d, creation_os9date);
	roothdr_blk.w24b(0x10, 1 + allocation_bitmap_lsns + 1);
	roothdr_blk.w16b(0x13, 8);

	// root directory data
	auto rootdata_blk = m_blockdev.get(1 + allocation_bitmap_lsns + 1);
	rootdata_blk.fill(0x00);
	rootdata_blk.w8(0x00, 0x2e);
	rootdata_blk.w8(0x01, 0xae);
	rootdata_blk.w8(0x1f, 1 + allocation_bitmap_lsns);
	rootdata_blk.w8(0x20, 0xae);
	rootdata_blk.w8(0x3f, 1 + allocation_bitmap_lsns);
	return ERR_OK;
}


//-------------------------------------------------
//  coco_os9_impl::find
//-------------------------------------------------

std::optional<coco_os9_impl::file_header> coco_os9_impl::find(const std::vector<std::string> &path, std::optional<dir_entry_type> expected_entry_type) const
{
	u32 lsn = m_volume_header.root_dir_lsn();
	file_header current(m_blockdev.get(lsn), "");

	// traverse the directory
	for (const std::string &path_part : path)
	{
		// use iterate_directory_entries() to find this file part
		std::optional<u32> child_lsn;
		auto callback = [&child_lsn, &path_part](std::string &&child_filename, u32 this_child_lsn)
		{
			bool found = child_filename == path_part;
			if (found)
				child_lsn = this_child_lsn;
			return found;
		};
		iterate_directory_entries(current, callback);

		// did we find the child?
		if (!child_lsn)
			return { };
		current = file_header(m_blockdev.get(*child_lsn), std::string(path_part));
	}

	// ensure that we found the entry type we expect
	dir_entry_type current_entry_type = current.is_directory() ? dir_entry_type::dir : dir_entry_type::file;
	return !expected_entry_type || current_entry_type == *expected_entry_type
		? std::move(current)
		: std::optional<file_header>();
}


//-------------------------------------------------
//  coco_os9_impl::iterate_directory_entries
//-------------------------------------------------

void coco_os9_impl::iterate_directory_entries(const file_header &header, const std::function<bool(std::string &&, u32)> callback) const
{
	// read the directory data
	std::vector<u8> directory_data = read_file_data(header);

	// and assemble results
	bool done = false;
	int directory_count = directory_data.size() / 32;
	for (int i = 0; !done && i < directory_count; i++)
	{
		// determine the filename
		std::string_view raw_filename((const char *)&directory_data[i * 32], 29);
		std::string filename = pick_os9_string(raw_filename);
		if (is_ignored_filename(filename))
			continue;

		// set up the child header
		u32 lsn = get_u24be(&directory_data[i * 32] + 29);

		// invoke the callback
		done = callback(std::move(filename), lsn);
	}
}


//-------------------------------------------------
//  coco_os9_impl::read_file_data
//-------------------------------------------------

std::vector<u8> coco_os9_impl::read_file_data(const file_header &header) const
{
	// prep the vector to return the data from
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
//  pick_os9_string
//-------------------------------------------------

std::string coco_os9_impl::pick_os9_string(std::string_view raw_string)
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
		result.append(1, *iter & 0x7f);
	return result;

}


//-------------------------------------------------
//  to_os9_string
//-------------------------------------------------

std::string coco_os9_impl::to_os9_string(std::string_view s, size_t length)
{
	std::string result(length, '\0');
	for (auto i = 0; i < std::min(length, s.size()); i++)
	{
		result[i] = (s[i] & 0x7f)
			| (i == s.size() ? 0x80 : 0x00);
	}
	return result;
}


//-------------------------------------------------
//  from_os9_date
//-------------------------------------------------

util::arbitrary_datetime coco_os9_impl::from_os9_date(u32 os9_date, u16 os9_time)
{
	util::arbitrary_datetime dt;
	memset(&dt, 0, sizeof(dt));
	dt.year = ((os9_date >> 16) & 0xff) + 1900;
	dt.month = (os9_date >> 8) & 0xff;
	dt.day_of_month = (os9_date >> 0) & 0xff;
	dt.hour = (os9_time >> 8) & 0xff;
	dt.minute = (os9_time >> 0) & 0xff;
	return dt;
}


//-------------------------------------------------
//  to_os9_date
//-------------------------------------------------

std::tuple<u32, u16> coco_os9_impl::to_os9_date(const util::arbitrary_datetime &datetime)
{
	u32 os9_date = ((datetime.year - 1900) & 0xff) << 16
		| (datetime.month & 0xff) << 8
		| (datetime.day_of_month & 0xff) << 0;
	u16 os9_time = (datetime.hour & 0xff) << 8
		| (datetime.minute & 0xff) << 0;
	return std::make_tuple(os9_date, os9_time);
}


//-------------------------------------------------
//  validate_filename
//-------------------------------------------------

bool coco_os9_impl::validate_filename(std::string_view name)
{
	return !is_ignored_filename(name)
		&& name.size() <= 29
		&& std::find_if(name.begin(), name.end(), [](const char ch) { return ch == '\0' || ch == '/' || (ch & 0x80); }) == name.end();
}


//-------------------------------------------------
//  is_ignored_filename - should this file name be
//  ignored if it is in the file system?
//-------------------------------------------------

bool coco_os9_impl::is_ignored_filename(std::string_view name)
{
	return name.empty()
		|| name[0] == '\0'
		|| name == "."
		|| name == "..";
}


//-------------------------------------------------
//  volume_header ctor
//-------------------------------------------------

coco_os9_impl::volume_header::volume_header(fsblk_t::block_t &&block)
	: m_block(std::move(block))
{
}


//-------------------------------------------------
//  volume_header::name
//-------------------------------------------------

std::string coco_os9_impl::volume_header::name() const
{
	std::string_view raw_name((const char *)&m_block.rodata()[31], 32);
	return pick_os9_string(raw_name);
}


//-------------------------------------------------
//  file_header ctor
//-------------------------------------------------

coco_os9_impl::file_header::file_header(fsblk_t::block_t &&block, std::string &&filename)
	: m_block(std::move(block))
	, m_filename(std::move(filename))
{
}


//-------------------------------------------------
//  file_header::creation_date
//-------------------------------------------------

util::arbitrary_datetime coco_os9_impl::file_header::creation_date() const
{
	return from_os9_date(m_block.r24b(13));
}


//-------------------------------------------------
//  file_header::metadata
//-------------------------------------------------

meta_data coco_os9_impl::file_header::metadata() const
{
	// format the attributes
	std::string attributes = util::string_format("%c%c%c%c%c%c%c%c",
		is_directory() ? 'd' : '-',
		is_non_sharable() ? 's' : '-',
		is_public_execute() ? 'x' : '-',
		is_public_write() ? 'w' : '-',
		is_public_read() ? 'r' : '-',
		is_user_execute() ? 'x' : '-',
		is_user_write() ? 'w' : '-',
		is_user_read() ? 'r' : '-');

	meta_data result;
	result.set(meta_name::name, m_filename);
	result.set(meta_name::creation_date, creation_date());
	result.set(meta_name::owner_id, owner_id());
	result.set(meta_name::attributes, std::move(attributes));
	result.set(meta_name::length, file_size());
	return result;
}


//-------------------------------------------------
//  file_header::get_sector_map_entry_count
//-------------------------------------------------

int coco_os9_impl::file_header::get_sector_map_entry_count() const
{
	return (m_block.size() - 16) / 5;
}


//-------------------------------------------------
//  file_header::get_sector_map_entry
//-------------------------------------------------

void coco_os9_impl::file_header::get_sector_map_entry(int entry_number, u32 &start_lsn, u16 &count) const
{
	start_lsn = m_block.r24b(16 + (entry_number * 5) + 0);
	count = m_block.r16b(16 + (entry_number * 5) + 3);
}
