// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of Apple ProDOS floppy images

#include "emu.h"
#include "fs_prodos.h"
#include "ap_dsk35.h"


const fs_prodos FS_PRODOS;

const char *fs_prodos::name() const
{
	return "prodos";
}

const char *fs_prodos::description() const
{
	return "Apple ProDOS";
}

const u8 fs_prodos::impl::boot[512] = {
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

void fs_prodos::enumerate_f(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	if(has(form_factor, variants, floppy_image::FF_35, floppy_image::DSDD))
		fe.add(FLOPPY_APPLE_GCR_FORMAT, 819200, "prodos_800k", "Apple ProDOS 800K");
	if(has(form_factor, variants, floppy_image::FF_35, floppy_image::SSDD))
		fe.add(FLOPPY_APPLE_GCR_FORMAT, 409600, "prodos_400k", "Apple ProDOS 400K");
}

std::unique_ptr<filesystem_t> fs_prodos::mount(fsblk_t &blockdev) const
{
	return std::make_unique<impl>(blockdev);
}

bool fs_prodos::can_format() const
{
	return true;
}

bool fs_prodos::can_read() const
{
	return true;
}

bool fs_prodos::can_write() const
{
	return false;
}

bool fs_prodos::has_rsrc() const
{
	return true;
}

char fs_prodos::directory_separator() const
{
	return '/';
}

std::vector<fs_meta_description> fs_prodos::volume_meta_description() const
{
	std::vector<fs_meta_description> res;
	res.emplace_back(fs_meta_description(fs_meta_name::name, fs_meta_type::string, "UNTITLED", false, [](const fs_meta &m) { return m.as_string().size() <= 15; }, "Volume name, up to 15 characters"));
	res.emplace_back(fs_meta_description(fs_meta_name::os_version, fs_meta_type::number, 5, false, [](const fs_meta &m) { return m.as_number() <= 255; }, "Creator OS version"));
	res.emplace_back(fs_meta_description(fs_meta_name::os_minimum_version, fs_meta_type::number, 5, false, [](const fs_meta &m) { return m.as_number() <= 255; }, "Minimum OS version"));

	auto now = util::arbitrary_datetime::now();
	res.emplace_back(fs_meta_description(fs_meta_name::creation_date, fs_meta_type::date, now, false, nullptr, "Creation time"));
	res.emplace_back(fs_meta_description(fs_meta_name::modification_date, fs_meta_type::date, now, false, nullptr, "Modification time"));
	return res;
}

std::vector<fs_meta_description> fs_prodos::file_meta_description() const
{
	std::vector<fs_meta_description> res;
	res.emplace_back(fs_meta_description(fs_meta_name::name, fs_meta_type::string, "Empty file", false, [](const fs_meta &m) { return m.as_string().size() <= 15; }, "File name, up to 15 characters"));
	res.emplace_back(fs_meta_description(fs_meta_name::length, fs_meta_type::number, 0, true, nullptr, "Size of the file in bytes"));
	res.emplace_back(fs_meta_description(fs_meta_name::rsrc_length, fs_meta_type::number, 0, true, nullptr, "Size of the resource fork in bytes"));
	res.emplace_back(fs_meta_description(fs_meta_name::os_version, fs_meta_type::number, 5, false, [](const fs_meta &m) { return m.as_number() <= 255; }, "Creator OS version"));
	res.emplace_back(fs_meta_description(fs_meta_name::os_minimum_version, fs_meta_type::number, 5, false, [](const fs_meta &m) { return m.as_number() <= 255; }, "Minimum OS version"));

	auto now = util::arbitrary_datetime::now();
	res.emplace_back(fs_meta_description(fs_meta_name::creation_date, fs_meta_type::date, now, false, nullptr, "Creation time"));
	res.emplace_back(fs_meta_description(fs_meta_name::modification_date, fs_meta_type::date, now, false, nullptr, "Modification time"));
	return res;
}

std::vector<fs_meta_description> fs_prodos::directory_meta_description() const
{
	std::vector<fs_meta_description> res;
	res.emplace_back(fs_meta_description(fs_meta_name::name, fs_meta_type::string, "Empty directory", false, [](const fs_meta &m) { return m.as_string().size() <= 15; }, "Directory name, up to 15 characters"));
	res.emplace_back(fs_meta_description(fs_meta_name::os_version, fs_meta_type::number, 5, false, [](const fs_meta &m) { return m.as_number() <= 255; }, "Creator OS version"));
	res.emplace_back(fs_meta_description(fs_meta_name::os_minimum_version, fs_meta_type::number, 5, false, [](const fs_meta &m) { return m.as_number() <= 255; }, "Minimum OS version"));

	auto now = util::arbitrary_datetime::now();
	res.emplace_back(fs_meta_description(fs_meta_name::creation_date, fs_meta_type::date, now, false, nullptr, "Creation time"));
	res.emplace_back(fs_meta_description(fs_meta_name::modification_date, fs_meta_type::date, now, false, nullptr, "Modification time"));
	return res;
}

void fs_prodos::impl::format(const fs_meta_data &meta)
{
	std::string volume_name = meta.get_string(fs_meta_name::name, "UNTITLED");
	u32 blocks = m_blockdev.block_count();

	// Maximum usable partition size = 32M - 512 bytes (65535 blocks)
	if(blocks >= 0x10000)
		blocks = 0xffff;

	m_blockdev.get(0).copy(0x000, boot, 0x200);               // Standard ProDOS boot sector as written by a 2gs
	m_blockdev.get(1).fill(0x00);                             // No SOS boot sector

	auto kblk1 = m_blockdev.get(2);                           // key block first block
	auto kblk2 = m_blockdev.get(3);                           // key block second block
	auto kblk3 = m_blockdev.get(4);                           // key block third block
	auto kblk4 = m_blockdev.get(5);                           // key block fourth block

	kblk1.w16l(0x00, 0x0000);                                 // Backwards key block pointer (null)
	kblk1.w16l(0x02, 0x0003);                                 // Forwards key block pointer
	kblk1.w8  (0x04, 0xf0 | volume_name.size());              // Block type (f, key block) and name size
	kblk1.wstr(0x05, volume_name);                            // Volume name, up to 15 characters
	kblk1.w32b(0x16, 0x642a250d);                             // ??? date & time
	kblk1.w16b(0x1a, 0x80ff);                                 // ???
	kblk1.w32b(0x1c, 0x642a250d);                             // Creation date & time
	kblk1.w8  (0x20, 0x05);                                   // ProDOS version (2gs)
	kblk1.w8  (0x21, 0x00);                                   // ProDOS minimum version
	kblk1.w8  (0x22, 0xc3);                                   // Allowed access (destroy, rename, !backup, 3x0, write read)
	kblk1.w8  (0x23, 0x27);                                   // Directory entry length (fixed)
	kblk1.w8  (0x24, 0x0d);                                   // Entries per block (fixed)
	kblk1.w16l(0x25, 0x0000);                                 // Number of file entries in the directory
	kblk1.w16l(0x27, 0x0006);                                 // Bitmap block pointer
	kblk1.w16l(0x29, blocks);                                 // Number of blocks

	kblk2.w16l(0x00, 0x0002);                                 // Backwards block pointer of the second volume block
	kblk2.w16l(0x02, 0x0004);                                 // Forwards block pointer of the second volume block
	kblk3.w16l(0x00, 0x0003);                                 // Backwards block pointer of the third volume block
	kblk3.w16l(0x02, 0x0005);                                 // Forwards block pointer of the third volume block
	kblk4.w16l(0x00, 0x0004);                                 // Backwards block pointer of the fourth volume block
	kblk4.w16l(0x02, 0x0000);                                 // Forwards block pointer of the fourth volume block (null)

	u32 fmap_block_count = (blocks + 4095) / 4096;
	u32 first_free_block = 6 + fmap_block_count;

	// Mark blocks from first_free_block to blocks-1 (the last one) as free
	for(u32 i = 0; i != fmap_block_count; i++) {
		auto fmap = m_blockdev.get(6 + i);
		u8 *fdata = fmap.data();
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
}

fs_prodos::impl::impl(fsblk_t &blockdev) : filesystem_t(blockdev, 512), m_root(true)
{
}

util::arbitrary_datetime fs_prodos::impl::prodos_to_dt(u32 date)
{
	util::arbitrary_datetime dt;
	dt.second       = 0;
	dt.minute       = ((date >> 16) & 0x3f);
	dt.hour         = ((date >> 24) & 0x1f);
	dt.day_of_month = ((date >> 0) & 0x1f);
	dt.month        = ((date >> 5) & 0x0f) + 1;
	dt.year         = ((date >> 9) & 0x7f) + 1900;
	if (dt.year <= 1949)
		dt.year += 100;

	return dt;
}

fs_meta_data fs_prodos::impl::metadata()
{
	fs_meta_data res;
	auto bdir = m_blockdev.get(2);
	int len = bdir.r8(0x04) & 0xf;
	res.set(fs_meta_name::name, bdir.rstr(0x05, len));
	res.set(fs_meta_name::os_version, bdir.r8(0x20));
	res.set(fs_meta_name::os_minimum_version, bdir.r8(0x21));
	res.set(fs_meta_name::creation_date, prodos_to_dt(bdir.r32l(0x1c)));
	res.set(fs_meta_name::modification_date, prodos_to_dt(bdir.r32l(0x16)));
	return res;
}

filesystem_t::dir_t fs_prodos::impl::root()
{
	if(!m_root)
		m_root = new root_dir(*this, 2);
	return m_root.strong();
}

void fs_prodos::impl::drop_root_ref()
{
	m_root = nullptr;
}


void fs_prodos::impl::root_dir::drop_weak_references()
{
	if(m_base_block == 2)
		m_fs.drop_root_ref();
}

fs_meta_data fs_prodos::impl::root_dir::metadata()
{
	return fs_meta_data();
}

std::vector<fs_dir_entry> fs_prodos::impl::root_dir::contents()
{
	std::vector<fs_dir_entry> res;

	u16 block = m_base_block;
	u32 off = 39 + 4;
	u32 id = 1;
	do {
		auto blk = m_fs.m_blockdev.get(block);
		while(off < 511) {
			u8 type = blk.r8(off);
			auto name = blk.rstr(off+1, type & 0xf);
			type >>= 4;
			if(type == 0xd)
				res.emplace_back(fs_dir_entry(name, fs_dir_entry_type::dir, id));
			else if(type != 0)
				res.emplace_back(fs_dir_entry(name, fs_dir_entry_type::file, id));
			off += 39;
			id ++;
		}
		block = blk.r16l(2);
		if(block >= m_fs.m_blockdev.block_count())
			break;
		off = 4;
	} while(block);
	return res;
}


std::pair<fsblk_t::block_t, const u8 *> fs_prodos::impl::root_dir::get_entry_ro(uint64_t key)
{
	std::pair<fsblk_t::block_t, const u8 *> res;
	res.first = m_fs.m_blockdev.get(m_base_block);
	while(key >= 13) {
		key -= 13;
		u16 block = res.first.r16l(2);
		if(!block || block >= m_fs.m_blockdev.block_count()) {
			res.first = nullptr;
			res.second = nullptr;
			return res;
		}
		res.first = m_fs.m_blockdev.get(block);
	}
	res.second = res.first.rodata() + 4 + 39 * key;
	return res;
}

std::pair<fsblk_t::block_t, u8 *> fs_prodos::impl::root_dir::get_entry(uint64_t key)
{
	std::pair<fsblk_t::block_t, u8 *> res;
	res.first = m_fs.m_blockdev.get(m_base_block);
	while(key > 13) {
		key -= 13;
		u16 block = res.first.r16l(2);
		if(!block || block >= m_fs.m_blockdev.block_count()) {
			res.first = nullptr;
			res.second = nullptr;
			return res;
		}
		res.first = m_fs.m_blockdev.get(block);
	}
	res.second = res.first.data() + 4 + 39 * key;
	return res;
}

filesystem_t::file_t fs_prodos::impl::root_dir::file_get(uint64_t key)
{
	auto [blk, entry] = get_entry_ro(key);
	if(!blk)
		fatalerror("Out-of-range key on file_get\n");
	u8 type = entry[0] >> 4;
	if(type == 0 || type == 4 || type > 5)
		fatalerror("Unhandled file type %x\n", type);
	return new file(m_fs, entry, key, this);
}

filesystem_t::dir_t fs_prodos::impl::root_dir::dir_get(uint64_t key)
{
	auto [blk, entry] = get_entry_ro(key);
	if(!blk)
		fatalerror("Out-of-range key on dir_get\n");
	u8 type = entry[0] >> 4;
	if(type != 0xd)
		fatalerror("Unhandled directory type %x\n", type);

	return new dir(m_fs, entry, r16l(entry+0x11), key, this);
}

fs_prodos::impl::dir::dir(impl &fs, const u8 *entry, u16 base_block, u16 key, root_dir *parent_dir) : root_dir(fs, base_block), m_parent_dir(parent_dir), m_key(key)
{
	memcpy(m_entry, entry, 39);
	(void)m_key;	
	(void)m_parent_dir;
}

fs_prodos::impl::file::file(impl &fs, const u8 *entry, u16 key, root_dir *parent_dir) : m_fs(fs), m_parent_dir(parent_dir), m_key(key)
{
	memcpy(m_entry, entry, 39);
	(void)m_key;
	(void)m_parent_dir;
}

void fs_prodos::impl::file::drop_weak_references()
{
}

fs_meta_data fs_prodos::impl::file::metadata()
{
	fs_meta_data res;
	u8 type = r8(m_entry);
	std::string name = rstr(m_entry+1, type & 0xf);
	type >>= 4;
	res.set(fs_meta_name::name, name);
	if(type == 5) {
		auto rootblk = m_fs.m_blockdev.get(r16l(m_entry+0x11));
		res.set(fs_meta_name::length, rootblk.r24l(0x005));
		res.set(fs_meta_name::rsrc_length, rootblk.r24l(0x105));

	} else if(type >= 1 && type <= 3)
		res.set(fs_meta_name::length, r24l(m_entry + 0x15));

	else
		fatalerror("fs_prodos::impl::file::metadata: Unhandled file type %d\n", type);

	return res;
}

fs_meta_data fs_prodos::impl::dir::metadata()
{
	fs_meta_data res;
	u8 type = r8(m_entry);
	std::string name = rstr(m_entry+1, type & 0xf);
	res.set(fs_meta_name::name, name);

	return res;
}

std::vector<u8> fs_prodos::impl::file::any_read_all(uint8_t type, u16 block, u32 length)
{
	std::vector<u8> data((length + 511) & ~511);
	u32 nb = data.size()/512;
	if(!nb)
		return data;

	u8 *dst = data.data();
	u8 *end = dst + data.size();
	switch(type) {
	case 1:
		memcpy(dst, m_fs.m_blockdev.get(block).rodata(), 512);
		dst += 512;
		break;

	case 2: {
		auto iblk = m_fs.m_blockdev.get(block);
		for(u32 i=0; i != 256 && dst != end; i++) {
			u16 blk = iblk.r8(i) | (iblk.r8(i | 0x100) << 8);
			memcpy(dst, m_fs.m_blockdev.get(blk).rodata(), 512);
			dst += 512;
		}
		break;
	}

	case 3: {
		auto mblk = m_fs.m_blockdev.get(block);
		for(u32 j=0; dst != end; j += 256) {
			u32 idx = j/256;
			auto iblk = m_fs.m_blockdev.get(mblk.r8(idx) | (mblk.r8(idx | 0x100) << 8));
			for(u32 i=0; i != 256 && dst != end; i++) {
				u16 blk = iblk.r8(i) | (iblk.r8(i | 0x100) << 8);
				memcpy(dst, m_fs.m_blockdev.get(blk).rodata(), 512);
				dst += 512;
			}
		}
		break;
	}

	default:
		fatalerror("fs_prodos::impl::file::get_file_blocks: unknown file type %d\n", type);
	}

	data.resize(length);
	return data;
}

std::vector<u8> fs_prodos::impl::file::read_all()
{
	u8 type = r8(m_entry) >> 4;
	if(type >= 1 && type <= 3)
		return any_read_all(type, r16l(m_entry+0x11), r24l(m_entry + 0x15));

	else if(type == 5) {
		auto kblk = m_fs.m_blockdev.get(r16l(m_entry+0x11));
		return any_read_all(kblk.r8(0x000), kblk.r16l(0x001), kblk.r24l(0x005));

	} else
		fatalerror("fs_prodos::impl::file::read_all: Unhandled file type %d\n", type);
}

std::vector<u8> fs_prodos::impl::file::rsrc_read_all()
{
	u8 type = r8(m_entry) >> 4;

	if(type == 5) {
		auto kblk = m_fs.m_blockdev.get(r16l(m_entry+0x11));
		return any_read_all(kblk.r8(0x100), kblk.r16l(0x101), kblk.r24l(0x105));

	} else
		fatalerror("fs_prodos::impl::file::rsrc_blocks: Unhandled file type %d\n", type);
}
