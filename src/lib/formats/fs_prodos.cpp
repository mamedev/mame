// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of Apple ProDOS floppy images

#include "fs_prodos.h"
#include "ap_dsk35.h"
#include "ap2_dsk.h"
#include "fsblk.h"

#include "corestr.h"
#include "multibyte.h"
#include "strformat.h"

#include <map>
#include <stdexcept>

using namespace fs;

namespace fs { const prodos_image PRODOS; }

namespace {
class prodos_impl : public filesystem_t {
public:
	prodos_impl(fsblk_t &blockdev);
	virtual ~prodos_impl() = default;

	virtual meta_data volume_metadata() override;
	virtual std::pair<std::error_condition, meta_data> metadata(const std::vector<std::string> &path) override;

	virtual std::pair<std::error_condition, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;

	virtual std::pair<std::error_condition, std::vector<u8>> file_read(const std::vector<std::string> &path) override;
	virtual std::pair<std::error_condition, std::vector<u8>> file_rsrc_read(const std::vector<std::string> &path) override;

	virtual std::error_condition format(const meta_data &meta) override;

private:
	static const u8 boot[512];
	static util::arbitrary_datetime prodos_to_dt(u32 date);

	std::tuple<fsblk_t::block_t::ptr, u32> path_find_step(const std::string &name, u16 block);
	std::tuple<fsblk_t::block_t::ptr, u32, bool> path_find(const std::vector<std::string> &path);
	std::pair<std::error_condition, std::vector<u8>> any_read(u8 type, u16 block, u32 length);
};
}


const char *prodos_image::name() const
{
	return "prodos";
}

const char *prodos_image::description() const
{
	return "Apple ProDOS";
}

const u8 prodos_impl::boot[512] = {
	0x01, 0x38, 0xb0, 0x03, 0x4c, 0x1c, 0x09, 0x78, 0x86, 0x43, 0xc9, 0x03, 0x08, 0x8a, 0x29, 0x70,
	0x4a, 0x4a, 0x4a, 0x4a, 0x09, 0xc0, 0x85, 0x49, 0xa0, 0xff, 0x84, 0x48, 0x28, 0xc8, 0xb1, 0x48,
	0xd0, 0x3a, 0xb0, 0x0e, 0xa9, 0x03, 0x8d, 0x00, 0x08, 0xe6, 0x3d, 0xa5, 0x49, 0x48, 0xa9, 0x5b,
	0x48, 0x60, 0x85, 0x40, 0x85, 0x48, 0xa0, 0x5e, 0xb1, 0x48, 0x99, 0x94, 0x09, 0xc8, 0xc0, 0xeb,
	0xd0, 0xf6, 0xa2, 0x06, 0xbc, 0x32, 0x09, 0xbd, 0x39, 0x09, 0x99, 0xf2, 0x09, 0xbd, 0x40, 0x09,
	0x9d, 0x7f, 0x0a, 0xca, 0x10, 0xee, 0xa9, 0x09, 0x85, 0x49, 0xa9, 0x86, 0xa0, 0x00, 0xc9, 0xf9,
	0xb0, 0x2f, 0x85, 0x48, 0x84, 0x60, 0x84, 0x4a, 0x84, 0x4c, 0x84, 0x4e, 0x84, 0x47, 0xc8, 0x84,
	0x42, 0xc8, 0x84, 0x46, 0xa9, 0x0c, 0x85, 0x61, 0x85, 0x4b, 0x20, 0x27, 0x09, 0xb0, 0x66, 0xe6,
	0x61, 0xe6, 0x61, 0xe6, 0x46, 0xa5, 0x46, 0xc9, 0x06, 0x90, 0xef, 0xad, 0x00, 0x0c, 0x0d, 0x01,
	0x0c, 0xd0, 0x52, 0xa9, 0x04, 0xd0, 0x02, 0xa5, 0x4a, 0x18, 0x6d, 0x23, 0x0c, 0xa8, 0x90, 0x0d,
	0xe6, 0x4b, 0xa5, 0x4b, 0x4a, 0xb0, 0x06, 0xc9, 0x0a, 0xf0, 0x71, 0xa0, 0x04, 0x84, 0x4a, 0xad,
	0x20, 0x09, 0x29, 0x0f, 0xa8, 0xb1, 0x4a, 0xd9, 0x20, 0x09, 0xd0, 0xdb, 0x88, 0x10, 0xf6, 0xa0,
	0x16, 0xb1, 0x4a, 0x4a, 0x6d, 0x1f, 0x09, 0x8d, 0x1f, 0x09, 0xa0, 0x11, 0xb1, 0x4a, 0x85, 0x46,
	0xc8, 0xb1, 0x4a, 0x85, 0x47, 0xa9, 0x00, 0x85, 0x4a, 0xa0, 0x1e, 0x84, 0x4b, 0x84, 0x61, 0xc8,
	0x84, 0x4d, 0x20, 0x27, 0x09, 0xb0, 0x35, 0xe6, 0x61, 0xe6, 0x61, 0xa4, 0x4e, 0xe6, 0x4e, 0xb1,
	0x4a, 0x85, 0x46, 0xb1, 0x4c, 0x85, 0x47, 0x11, 0x4a, 0xd0, 0x18, 0xa2, 0x01, 0xa9, 0x00, 0xa8,
	0x91, 0x60, 0xc8, 0xd0, 0xfb, 0xe6, 0x61, 0xea, 0xea, 0xca, 0x10, 0xf4, 0xce, 0x1f, 0x09, 0xf0,
	0x07, 0xd0, 0xd8, 0xce, 0x1f, 0x09, 0xd0, 0xca, 0x58, 0x4c, 0x00, 0x20, 0x4c, 0x47, 0x09, 0x02,
	0x26, 0x50, 0x52, 0x4f, 0x44, 0x4f, 0x53, 0xa5, 0x60, 0x85, 0x44, 0xa5, 0x61, 0x85, 0x45, 0x6c,
	0x48, 0x00, 0x08, 0x1e, 0x24, 0x3f, 0x45, 0x47, 0x76, 0xf4, 0xd7, 0xd1, 0xb6, 0x4b, 0xb4, 0xac,
	0xa6, 0x2b, 0x18, 0x60, 0x4c, 0xbc, 0x09, 0x20, 0x58, 0xfc, 0xa0, 0x14, 0xb9, 0x58, 0x09, 0x99,
	0xb1, 0x05, 0x88, 0x10, 0xf7, 0x4c, 0x55, 0x09, 0xd5, 0xce, 0xc1, 0xc2, 0xcc, 0xc5, 0xa0, 0xd4,
	0xcf, 0xa0, 0xcc, 0xcf, 0xc1, 0xc4, 0xa0, 0xd0, 0xd2, 0xcf, 0xc4, 0xcf, 0xd3, 0xa5, 0x53, 0x29,
	0x03, 0x2a, 0x05, 0x2b, 0xaa, 0xbd, 0x80, 0xc0, 0xa9, 0x2c, 0xa2, 0x11, 0xca, 0xd0, 0xfd, 0xe9,
	0x01, 0xd0, 0xf7, 0xa6, 0x2b, 0x60, 0xa5, 0x46, 0x29, 0x07, 0xc9, 0x04, 0x29, 0x03, 0x08, 0x0a,
	0x28, 0x2a, 0x85, 0x3d, 0xa5, 0x47, 0x4a, 0xa5, 0x46, 0x6a, 0x4a, 0x4a, 0x85, 0x41, 0x0a, 0x85,
	0x51, 0xa5, 0x45, 0x85, 0x27, 0xa6, 0x2b, 0xbd, 0x89, 0xc0, 0x20, 0xbc, 0x09, 0xe6, 0x27, 0xe6,
	0x3d, 0xe6, 0x3d, 0xb0, 0x03, 0x20, 0xbc, 0x09, 0xbc, 0x88, 0xc0, 0x60, 0xa5, 0x40, 0x0a, 0x85,
	0x53, 0xa9, 0x00, 0x85, 0x54, 0xa5, 0x53, 0x85, 0x50, 0x38, 0xe5, 0x51, 0xf0, 0x14, 0xb0, 0x04,
	0xe6, 0x53, 0x90, 0x02, 0xc6, 0x53, 0x38, 0x20, 0x6d, 0x09, 0xa5, 0x50, 0x18, 0x20, 0x6f, 0x09,
	0xd0, 0xe3, 0xa0, 0x7f, 0x84, 0x52, 0x08, 0x28, 0x38, 0xc6, 0x52, 0xf0, 0xce, 0x18, 0x08, 0x88,
	0xf0, 0xf5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void prodos_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_APPLE_GCR_FORMAT, floppy_image::FF_35, floppy_image::DSDD, 819200, "prodos_800k", "Apple ProDOS 3.5\" 800K");
	fe.add(FLOPPY_APPLE_GCR_FORMAT, floppy_image::FF_35, floppy_image::SSDD, 409600, "prodos_400k", "Apple ProDOS 3.5\" 400K");
	fe.add(FLOPPY_A216S_PRODOS_FORMAT, floppy_image::FF_525, floppy_image::SSSD, 143360, "prodos_140k", "Apple ProDOS 5.25\" 140K");
}

std::unique_ptr<filesystem_t> prodos_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<prodos_impl>(blockdev);
}

bool prodos_image::can_format() const
{
	return true;
}

bool prodos_image::can_read() const
{
	return true;
}

bool prodos_image::can_write() const
{
	return false;
}

bool prodos_image::has_rsrc() const
{
	return true;
}

char prodos_image::directory_separator() const
{
	return '/';
}

// TODO: this list is incomplete
static const std::map<u8, const char *> s_file_types =
{
	{ 0x00, "UNK" },
	{ 0x01, "BAD" },
	{ 0x04, "TXT" },
	{ 0x06, "BIN" },
	{ 0x0f, "DIR" },
	{ 0x19, "ADB" },
	{ 0x1a, "AWP" },
	{ 0x1b, "ASP" },
	{ 0xc9, "FND" },
	{ 0xca, "ICN" },
	{ 0xef, "PAS" },
	{ 0xf0, "CMD" },
	{ 0xfa, "INT" },
	{ 0xfb, "IVR" },
	{ 0xfc, "BAS" },
	{ 0xfd, "VAR" },
	{ 0xfe, "REL" },
	{ 0xff, "SYS" }
};

static std::string file_type_to_string(uint8_t type)
{
	auto res = s_file_types.find(type);
	if (res != s_file_types.end())
		return res->second;
	else
		return util::string_format("0x%02x", type);
}

static bool validate_file_type(std::string str)
{
	if ((str.length() == 3 || str.length() == 4) && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
		return str.find_first_not_of("0123456789ABCDEFabcdef", 2) == std::string::npos;
	else
		return std::any_of(s_file_types.begin(), s_file_types.end(), [&str](const auto &pair) { return util::strequpper(str, pair.second); });
}

std::vector<meta_description> prodos_image::volume_meta_description() const
{
	std::vector<meta_description> res;
	res.emplace_back(meta_description(meta_name::name, "UNTITLED", false, [](const meta_value &m) { return m.as_string().size() <= 15; }, "Volume name, up to 15 characters"));
	res.emplace_back(meta_description(meta_name::os_version, 5, false, [](const meta_value &m) { return m.as_number() <= 255; }, "Creator OS version"));
	res.emplace_back(meta_description(meta_name::os_minimum_version, 5, false, [](const meta_value &m) { return m.as_number() <= 255; }, "Minimum OS version"));

	auto now = util::arbitrary_datetime::now();
	res.emplace_back(meta_description(meta_name::creation_date, now, false, nullptr, "Creation time"));
	res.emplace_back(meta_description(meta_name::modification_date, now, false, nullptr, "Modification time"));
	return res;
}

std::vector<meta_description> prodos_image::file_meta_description() const
{
	std::vector<meta_description> res;
	res.emplace_back(meta_description(meta_name::name, "Empty file", false, [](const meta_value &m) { return m.as_string().size() <= 15; }, "File name, up to 15 characters"));
	res.emplace_back(meta_description(meta_name::length, 0, true, nullptr, "Size of the file in bytes"));
	res.emplace_back(meta_description(meta_name::rsrc_length, 0, true, nullptr, "Size of the resource fork in bytes"));
	res.emplace_back(meta_description(meta_name::file_type, "UNK", false, [](const meta_value &m) { return validate_file_type(m.as_string()); }, "File type, 3 letters or hex code preceded by 0x"));
	res.emplace_back(meta_description(meta_name::os_version, 5, false, [](const meta_value &m) { return m.as_number() <= 255; }, "Creator OS version"));
	res.emplace_back(meta_description(meta_name::os_minimum_version, 5, false, [](const meta_value &m) { return m.as_number() <= 255; }, "Minimum OS version"));

	auto now = util::arbitrary_datetime::now();
	res.emplace_back(meta_description(meta_name::creation_date, now, false, nullptr, "Creation time"));
	res.emplace_back(meta_description(meta_name::modification_date, now, false, nullptr, "Modification time"));
	return res;
}

std::vector<meta_description> prodos_image::directory_meta_description() const
{
	std::vector<meta_description> res;
	res.emplace_back(meta_description(meta_name::name, "Empty directory", false, [](const meta_value &m) { return m.as_string().size() <= 15; }, "Directory name, up to 15 characters"));
	res.emplace_back(meta_description(meta_name::os_version, 5, false, [](const meta_value &m) { return m.as_number() <= 255; }, "Creator OS version"));
	res.emplace_back(meta_description(meta_name::os_minimum_version, 5, false, [](const meta_value &m) { return m.as_number() <= 255; }, "Minimum OS version"));

	auto now = util::arbitrary_datetime::now();
	res.emplace_back(meta_description(meta_name::creation_date, now, false, nullptr, "Creation time"));
	res.emplace_back(meta_description(meta_name::modification_date, now, false, nullptr, "Modification time"));
	return res;
}

std::error_condition prodos_impl::format(const meta_data &meta)
{
	std::string volume_name = meta.get_string(meta_name::name, "UNTITLED");
	u32 blocks = m_blockdev.block_count();

	// Maximum usable partition size = 32M - 512 bytes (65535 blocks)
	if(blocks >= 0x10000)
		blocks = 0xffff;

	m_blockdev.get(0)->write(0x000, boot, 0x200);             // Standard ProDOS boot sector as written by a 2gs
	m_blockdev.get(1)->fill(0x00);                            // No SOS boot sector

	auto kblk1 = m_blockdev.get(2);                           // key block first block
	auto kblk2 = m_blockdev.get(3);                           // key block second block
	auto kblk3 = m_blockdev.get(4);                           // key block third block
	auto kblk4 = m_blockdev.get(5);                           // key block fourth block

	kblk1->w16l(0x00, 0x0000);                                // Backwards key block pointer (null)
	kblk1->w16l(0x02, 0x0003);                                // Forwards key block pointer
	kblk1->w8  (0x04, 0xf0 | volume_name.size());             // Block type (f, key block) and name size
	kblk1->wstr(0x05, volume_name);                           // Volume name, up to 15 characters
	kblk1->w32b(0x16, 0x642a250d);                            // ??? date & time
	kblk1->w16b(0x1a, 0x80ff);                                // ???
	kblk1->w32b(0x1c, 0x642a250d);                            // Creation date & time
	kblk1->w8  (0x20, 0x05);                                  // ProDOS version (2gs)
	kblk1->w8  (0x21, 0x00);                                  // ProDOS minimum version
	kblk1->w8  (0x22, 0xc3);                                  // Allowed access (destroy, rename, !backup, 3x0, write read)
	kblk1->w8  (0x23, 0x27);                                  // Directory entry length (fixed)
	kblk1->w8  (0x24, 0x0d);                                  // Entries per block (fixed)
	kblk1->w16l(0x25, 0x0000);                                // Number of file entries in the directory
	kblk1->w16l(0x27, 0x0006);                                // Bitmap block pointer
	kblk1->w16l(0x29, blocks);                                // Number of blocks

	kblk2->w16l(0x00, 0x0002);                                // Backwards block pointer of the second volume block
	kblk2->w16l(0x02, 0x0004);                                // Forwards block pointer of the second volume block
	kblk3->w16l(0x00, 0x0003);                                // Backwards block pointer of the third volume block
	kblk3->w16l(0x02, 0x0005);                                // Forwards block pointer of the third volume block
	kblk4->w16l(0x00, 0x0004);                                // Backwards block pointer of the fourth volume block
	kblk4->w16l(0x02, 0x0000);                                // Forwards block pointer of the fourth volume block (null)

	u32 fmap_block_count = (blocks + 4095) / 4096;
	u32 first_free_block = 6 + fmap_block_count;

	// Mark blocks from first_free_block to blocks-1 (the last one) as free
	for(u32 i = 0; i != fmap_block_count; i++) {
		auto fmap = m_blockdev.get(6 + i);
		u8 *fdata = fmap->data();
		u32 start = i ? 0 : first_free_block;
		u32 end = i != fmap_block_count - 1 ? 4095 : (blocks - 1) & 4095;
		end += 1;
		u32 sb = start >> 3;
		u32 si = start & 7;
		u32 eb = end >> 3;
		u32 ei = end & 7;
		if(sb == eb)
			fdata[sb] = (0xff >> si) & ~(0xff >> ei);
		else {
			fdata[sb] = 0xff >> si;
			if(eb != 512)
				fdata[eb] = ~(0xff >> ei);
			if(eb - sb > 1)
				memset(fdata+sb, 0xff, eb-sb-1);
		}
	}
	return std::error_condition();
}

prodos_impl::prodos_impl(fsblk_t &blockdev) : filesystem_t(blockdev, 512)
{
}

util::arbitrary_datetime prodos_impl::prodos_to_dt(u32 date)
{
	util::arbitrary_datetime dt;
	dt.second       = 0;
	dt.minute       = ((date >> 16) & 0x3f);
	dt.hour         = ((date >> 24) & 0x1f);
	dt.day_of_month = ((date >> 0) & 0x1f); // 1-based
	dt.month        = ((date >> 5) & 0x0f); // 1-based
	dt.year         = ((date >> 9) & 0x7f) + 1900;
	if (dt.year <= 1949)
		dt.year += 100;

	return dt;
}

meta_data prodos_impl::volume_metadata()
{
	meta_data res;
	auto bdir = m_blockdev.get(2);
	int len = bdir->r8(0x04) & 0xf;
	res.set(meta_name::name, bdir->rstr(0x05, len));
	res.set(meta_name::os_version, bdir->r8(0x20));
	res.set(meta_name::os_minimum_version, bdir->r8(0x21));
	res.set(meta_name::creation_date, prodos_to_dt(bdir->r32l(0x1c)));
	res.set(meta_name::modification_date, prodos_to_dt(bdir->r32l(0x16)));
	return res;
}

std::pair<std::error_condition, std::vector<dir_entry>> prodos_impl::directory_contents(const std::vector<std::string> &path)
{
	u16 block;
	if(path.empty())
		block = 2;

	else {
		auto [blk, off, dir] = path_find(path);
		if(!off || !dir)
			return std::make_pair(error::not_found, std::vector<dir_entry>());
		block = blk->r16l(off+0x11);
	}

	std::vector<dir_entry> res;

	do {
		auto blk = m_blockdev.get(block);
		for(u32 off = 4; off < 511; off += 39) {
			u8 type = blk->r8(off);
			// skip inactive entries and subroutine/volume headers
			if(type != 0 && type < 0xe0) {
				meta_data meta;
				meta.set(meta_name::name, blk->rstr(off+1, type & 0xf));
				type >>= 4;

				if(type == 5) {
					auto rootblk = m_blockdev.get(blk->r16l(off+0x11));
					meta.set(meta_name::length, rootblk->r24l(0x005));
					meta.set(meta_name::rsrc_length, rootblk->r24l(0x105));

				} else if(type >= 1 && type <= 3) {
					meta.set(meta_name::length, blk->r24l(off + 0x15));
					meta.set(meta_name::file_type, file_type_to_string(blk->r8(off + 0x10)));
				}

				meta.set(meta_name::os_version, blk->r8(off + 0x1c));
				meta.set(meta_name::os_minimum_version, blk->r8(off + 0x1d));
				meta.set(meta_name::creation_date, prodos_to_dt(blk->r32l(off + 0x18)));
				meta.set(meta_name::modification_date, prodos_to_dt(blk->r32l(off + 0x21)));

				res.emplace_back(dir_entry(type == 0xd ? dir_entry_type::dir : dir_entry_type::file, meta));
			}
		}
		block = blk->r16l(2);
		if(block >= m_blockdev.block_count())
			break;
	} while(block);
	return std::make_pair(std::error_condition(), res);
}

std::tuple<fsblk_t::block_t::ptr, u32> prodos_impl::path_find_step(const std::string &name, u16 block)
{
	for(;;) {
		auto blk = m_blockdev.get(block);
		for(u32 off = 4; off < 511; off += 39) {
			u8 type = blk->r8(off);
			if(type != 0 && type < 0xe0 && name == blk->rstr(off+1, type & 0xf))
				return std::make_tuple(blk, off);
		}
		block = blk->r16l(2);
		if(!block || block >= m_blockdev.block_count())
			return std::make_tuple(blk, 0U);
	}
}

std::tuple<fsblk_t::block_t::ptr, u32, bool> prodos_impl::path_find(const std::vector<std::string> &path)
{
	if(path.size() == 0)
		return std::tuple<fsblk_t::block_t::ptr, u32, bool>(fsblk_t::block_t::ptr(), 0, false);

	u16 block = 2;
	for(u32 pathc = 0;; pathc++) {
		auto [blk, off] = path_find_step(path[pathc], block);
		if(!off)
			return std::make_tuple(blk, off, false);

		if(pathc + 1 == path.size())
			return std::make_tuple(blk, off, (blk->r8(off) & 0xf0) == 0xd0);

		if((blk->r8(off) & 0xf0) != 0xd0)
			return std::make_tuple(blk, 0U, false);

		block = blk->r16l(off + 0x11);
	}
}


std::pair<std::error_condition, meta_data> prodos_impl::metadata(const std::vector<std::string> &path)
{
	if(path.size() == 0)
		return std::make_pair(std::error_condition(), meta_data());

	auto [blk, off, dir] = path_find(path);

	if(!off)
		return std::make_pair(error::not_found, meta_data());

	const u8 *entry = blk->rodata() + off;

	meta_data res;
	if(dir) {
		u8 type = entry[0];
		std::string_view name = rstr(entry+1, type & 0xf);
		res.set(meta_name::name, name);
	} else {
		u8 type = entry[0];
		std::string_view name = rstr(entry+1, type & 0xf);
		type >>= 4;
		res.set(meta_name::name, name);
		if(type == 5) {
			auto rootblk = m_blockdev.get(get_u16le(entry+0x11));
			res.set(meta_name::length, rootblk->r24l(0x005));
			res.set(meta_name::rsrc_length, rootblk->r24l(0x105));

		} else if(type >= 1 && type <= 3)
			res.set(meta_name::length, get_u24le(entry + 0x15));

		else
			return std::make_pair(error::unsupported, meta_data());
	}

	return std::make_pair(std::error_condition(), res);
}

std::pair<std::error_condition, std::vector<u8>> prodos_impl::any_read(u8 type, u16 block, u32 length)
{
	std::pair<std::error_condition, std::vector<u8>> data;
	data.first = std::error_condition();
	data.second.resize((length + 511) & ~511);
	u32 nb = data.second.size()/512;
	if(!nb)
		return data;

	u8 *dst = data.second.data();
	u8 *end = dst + data.second.size();
	switch(type) {
	case 1:
		m_blockdev.get(block)->read(0, dst, 512);
		dst += 512;
		break;

	case 2: {
		auto iblk = m_blockdev.get(block);
		for(u32 i=0; i != 256 && dst != end; i++) {
			u16 blk = iblk->r8(i) | (iblk->r8(i | 0x100) << 8);
			m_blockdev.get(blk)->read(0, dst, 512);
			dst += 512;
		}
		break;
	}

	case 3: {
		auto mblk = m_blockdev.get(block);
		for(u32 j=0; dst != end; j += 256) {
			u32 idx = j/256;
			auto iblk = m_blockdev.get(mblk->r8(idx) | (mblk->r8(idx | 0x100) << 8));
			for(u32 i=0; i != 256 && dst != end; i++) {
				u16 blk = iblk->r8(i) | (iblk->r8(i | 0x100) << 8);
				m_blockdev.get(blk)->read(0, dst, 512);
				dst += 512;
			}
		}
		break;
	}

	default:
		data.first = error::unsupported;
		data.second.clear();
		return data;
	}

	data.second.resize(length);
	return data;
}

std::pair<std::error_condition, std::vector<u8>> prodos_impl::file_read(const std::vector<std::string> &path)
{
	auto [blk, off, dir] = path_find(path);
	if(!off || dir)
		return std::make_pair(error::not_found, std::vector<u8>());

	const u8 *entry = blk->rodata() + off;
	u8 type = entry[0] >> 4;

	if(type >= 1 && type <= 3)
		return any_read(type, get_u16le(entry+0x11), get_u24le(entry + 0x15));

	else if(type == 5) {
		auto kblk = m_blockdev.get(get_u16le(entry+0x11));
		return any_read(kblk->r8(0x000), kblk->r16l(0x001), kblk->r24l(0x005));

	} else
		return std::make_pair(error::unsupported, std::vector<u8>());
}

std::pair<std::error_condition, std::vector<u8>> prodos_impl::file_rsrc_read(const std::vector<std::string> &path)
{
	auto [blk, off, dir] = path_find(path);
	if(!off || dir)
		return std::make_pair(error::not_found, std::vector<u8>());

	const u8 *entry = blk->rodata() + off;
	u8 type = entry[0] >> 4;

	if(type == 5) {
		auto kblk = m_blockdev.get(get_u16le(entry+0x11));
		return any_read(kblk->r8(0x100), kblk->r16l(0x101), kblk->r24l(0x105));

	} else
		return std::make_pair(error::unsupported, std::vector<u8>());
}
