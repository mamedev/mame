// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Management of VTech images

#include "fs_vtech.h"
#include "vt_dsk.h"

#include <stdexcept>

namespace fs {

const vtech_image VTECH;

// Floppy only, format is 40 tracks, 1 head, 16 sectors numbered 0-15, 256 bytes/sector.
// Filesystem has no subdirectories.
//
// Track 0 sectors 0-14 have the file names.  16 bytes/entry
//   offset 0  : File type 'T' (basic) or 'B' (binary)
//   offset 1  : 0x3a
//   offset 2-9: File name
//   offset a  : Track number of first file sector
//   offset b  : Sector number of first file sector
//   offset c-d: Start address (little-endian)
//   offset e-f: End address (little-endian)
//
//  Files are stored with 126 bytes/sector, and bytes 126 and 127 are
//  track/sector of the next file data sector.

const char *vtech_image::name() const
{
	return "vtech";
}

const char *vtech_image::description() const
{
	return "VTech (Laser 200/300)";
}

void vtech_image::enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const
{
	if(has(form_factor, variants, floppy_image::FF_525, floppy_image::SSSD))
		fe.add(FLOPPY_VTECH_BIN_FORMAT, 163840, "vtech", "VTech");
}

std::unique_ptr<filesystem_t> vtech_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<impl>(blockdev);
}

bool vtech_image::can_format() const
{
	return true;
}

bool vtech_image::can_read() const
{
	return true;
}

bool vtech_image::can_write() const
{
	return true;
}

bool vtech_image::has_rsrc() const
{
	return false;
}

std::vector<meta_description> vtech_image::volume_meta_description() const
{
	std::vector<meta_description> res;
	return res;
}

meta_data vtech_image::impl::metadata()
{
	meta_data res;
	return res;
}

std::vector<meta_description> vtech_image::file_meta_description() const
{
	std::vector<meta_description> res;
	res.emplace_back(meta_description(meta_name::name, meta_type::string, "", false, [](const meta_value &m) { return m.as_string().size() <= 8; }, "File name, 8 chars"));
	res.emplace_back(meta_description(meta_name::loading_address, meta_type::number, 0x7ae9, false, [](const meta_value &m) { return m.as_number() < 0x10000; }, "Loading address of the file"));
	res.emplace_back(meta_description(meta_name::length, meta_type::number, 0, true, nullptr, "Size of the file in bytes"));
	res.emplace_back(meta_description(meta_name::basic, meta_type::flag, true, true, nullptr, "Basic file"));
	return res;
}


void vtech_image::impl::format(const meta_data &meta)
{
	m_blockdev.fill(0);
}

vtech_image::impl::impl(fsblk_t &blockdev) : filesystem_t(blockdev, 128), m_root(true)
{
}

filesystem_t::dir_t vtech_image::impl::root()
{
	if(!m_root)
		m_root = new root_dir(*this);
	return m_root.strong();
}

void vtech_image::impl::drop_root_ref()
{
	m_root = nullptr;
}

void vtech_image::impl::root_dir::drop_weak_references()
{
	m_fs.drop_root_ref();
}

meta_data vtech_image::impl::root_dir::metadata()
{
	return meta_data();
}

std::vector<dir_entry> vtech_image::impl::root_dir::contents()
{
	std::vector<dir_entry> res;

	u64 id = 0;
	for(int sect = 0; sect != 14; sect++) {
		auto bdir = m_fs.m_blockdev.get(sect);
		for(u32 i = 0; i != 8; i ++) {
			u32 off = i*16;
			u8 type = bdir.r8(off);
			if(type != 'T' && type != 'B')
				continue;
			if(bdir.r8(off+1) != ':')
				continue;
			std::string fname = trim_end_spaces(bdir.rstr(off+2, 8));
			res.emplace_back(dir_entry(fname, dir_entry_type::file, id));
			id++;
		}
	}
	return res;
}

filesystem_t::file_t vtech_image::impl::root_dir::file_get(u64 key)
{
	if(key >= 15*8)
		throw std::out_of_range("Key out of range");

	auto bdir = m_fs.m_blockdev.get(key >> 3);
	int off = (key & 7) << 4;
	return file_t(new file(m_fs, this, bdir.rodata() + off, key));
}

void vtech_image::impl::root_dir::update_file(u16 key, const u8 *entry)
{
	auto bdir = m_fs.m_blockdev.get(key >> 3);
	int off = (key & 7) << 4;
	bdir.copy(off, entry, 16);
}

filesystem_t::dir_t vtech_image::impl::root_dir::dir_get(u64 key)
{
	throw std::logic_error("Directories not supported");
}

vtech_image::impl::file::file(impl &fs, root_dir *dir, const u8 *entry, u16 key) : m_fs(fs), m_dir(dir), m_key(key)
{
	memcpy(m_entry, entry, 16);
}

void vtech_image::impl::file::drop_weak_references()
{
}

meta_data vtech_image::impl::file::metadata()
{
	meta_data res;

	res.set(meta_name::name, trim_end_spaces(rstr(m_entry+2, 8)));
	res.set(meta_name::basic, m_entry[0] == 'T');
	res.set(meta_name::loading_address, r16l(m_entry + 0xc));
	res.set(meta_name::length, ((r16l(m_entry + 0xe) - r16l(m_entry + 0xc) + 1) & 0xffff));

	return res;
}

std::vector<u8> vtech_image::impl::file::read_all()
{
	u8 track = m_entry[0xa];
	u8 sector = m_entry[0xb];
	int len = ((r16l(m_entry + 0xe) - r16l(m_entry + 0xc)) & 0xffff) + 1;

	std::vector<u8> data(len, 0);
	int pos = 0;
	while(pos < len) {
		if(track >= 40 || sector >= 16)
			break;
		auto dblk = m_fs.m_blockdev.get(track*16 + sector);
		int size = len - pos;
		if(size > 126)
			size = 126;
		memcpy(data.data() + pos, dblk.data(), size);
		pos += size;
		track = dblk.r8(126);
		sector = dblk.r8(127);
	}
	return data;
}

vtech_image::impl::file_t vtech_image::impl::root_dir::file_create(const meta_data &info)
{
	// Find the key for the next unused entry
	for(int sect = 0; sect != 14; sect++) {
		auto bdir = m_fs.m_blockdev.get(sect);
		u64 id = 0;
		for(u32 i = 0; i != 16; i ++) {
			u32 off = i*16;
			u8 type = bdir.r8(off);
			if(type != 'T' && type != 'B') {
				std::string fname = info.get_string(meta_name::name, "");
				fname.resize(8, ' ');

				bdir.w8  (off+0x0, info.get_flag(meta_name::basic, true) ? 'T' : 'B');
				bdir.w8  (off+0x1, ':');
				bdir.wstr(off+0x2, fname);
				bdir.w8  (off+0xa, 0x00);
				bdir.w8  (off+0xb, 0x00);
				bdir.w16l(off+0xc, info.get_number(meta_name::loading_address, 0x7ae9));
				bdir.w16l(off+0xe, bdir.r16l(off+0xc) - 1); // Size 0 initially
				return file_t(new file(m_fs, this, bdir.rodata() + off, id));
			}
			id++;
		}
	}
	return nullptr;
}

void vtech_image::impl::root_dir::file_delete(u64 key)
{
}

void vtech_image::impl::file::replace(const std::vector<u8> &data)
{
	u32 cur_len = ((r16l(m_entry + 0xe) - r16l(m_entry + 0xc) + 1) & 0xffff);
	u32 new_len = data.size();
	if(new_len > 65535)
		new_len = 65535;
	u32 cur_ns = (cur_len + 125)/126;
	u32 need_ns = (new_len + 125) / 126;

	// Enough space?
	if(cur_ns < need_ns && m_fs.free_block_count() < need_ns - cur_ns)
		return;

	u8 track = m_entry[0xa];
	u8 sector = m_entry[0xb];
	std::vector<std::pair<u8, u8>> tofree;
	for(u32 i = 0; i != cur_ns; i++) {
		tofree.emplace_back(std::make_pair(track, sector));
		auto dblk = m_fs.m_blockdev.get(track*16 + sector);
		track = dblk.r8(126);
		sector = dblk.r8(127);
	}

	m_fs.free_blocks(tofree);

	std::vector<std::pair<u8, u8>> blocks = m_fs.allocate_blocks(need_ns);
	for(u32 i=0; i != need_ns; i ++) {
		auto dblk = m_fs.m_blockdev.get(blocks[i].first * 16 + blocks[i].second);
		u32 len = new_len - i*126;
		if(len > 126)
			len = 126;
		else if(len < 126)
			dblk.fill(0x00);
		memcpy(dblk.data(), data.data() + 126*i, len);
		if(i < need_ns) {
			dblk.w8(126, blocks[i+1].first);
			dblk.w8(127, blocks[i+1].second);
		} else
			dblk.w16l(126, 0);
	}

	u16 end_address = (r16l(m_entry + 0xc) + data.size() - 1) & 0xffff;
	w16l(m_entry + 0xe, end_address);
	if(need_ns) {
		w8(m_entry + 0xa, blocks[0].first);
		w8(m_entry + 0xb, blocks[0].second);
	} else
		w16l(m_entry + 0xa, 0);

	m_dir->update_file(m_key, m_entry);
}

void vtech_image::impl::root_dir::metadata_change(const meta_data &info)
{
}

void vtech_image::impl::metadata_change(const meta_data &info)
{
}

void vtech_image::impl::file::metadata_change(const meta_data &info)
{
	if(info.has(meta_name::basic))
		w8  (m_entry+0x0, info.get_flag(meta_name::basic) ? 'T' : 'B');
	if(info.has(meta_name::name)) {
		std::string name = info.get_string(meta_name::name);
		name.resize(8, ' ');
		wstr(m_entry+0x2, name);
	}
	if(info.has(meta_name::loading_address)) {
		u16 new_loading = info.get_number(meta_name::loading_address);
		u16 new_end = r16l(m_entry + 0xe) - r16l(m_entry + 0xc) + new_loading;
		w16l(m_entry + 0xc, new_loading);
		w16l(m_entry + 0xe, new_end);
	}
	m_dir->update_file(m_key, m_entry);
}

std::vector<std::pair<u8, u8>> vtech_image::impl::allocate_blocks(u32 count)
{
	std::vector<std::pair<u8, u8>> blocks;
	if(free_block_count() < count)
		return blocks;

	auto fmap = m_blockdev.get(15);
	for(u8 track = 1; track != 40; track++)
		for(u8 sector = 0; sector != 16; sector++) {
			u32 off = (track-1)*2 + (sector / 8);
			u32 bit = 1 << (sector & 7);
			if(!(fmap.r8(off) & bit)) {
				fmap.w8(off, fmap.r8(off) | bit);
				blocks.emplace_back(std::make_pair(track, sector));
				if(blocks.size() == count)
					return blocks;
			}
		}
	abort();
}

void vtech_image::impl::free_blocks(const std::vector<std::pair<u8, u8>> &blocks)
{
	auto fmap = m_blockdev.get(15);
	for(auto ref : blocks) {
		u8 track = ref.first;
		u8 sector = ref.second;
		u32 off = (track-1)*2 + (sector / 8);
		u32 bit = 1 << (sector & 7);
		fmap.w8(off, fmap.r8(off) & ~bit);
	}
}

u32 vtech_image::impl::free_block_count()
{
	auto fmap = m_blockdev.get(15);
	u32 nf = 0;
	for(u32 off = 0; off != (40-1)*2; off++) {
		u8 m = fmap.r8(off);
		// Count 1 bits;
		m = ((m & 0xaa) >> 1) | (m & 0x55);
		m = ((m & 0xcc) >> 2) | (m & 0x33);
		m = ((m & 0xf0) >> 4) | (m & 0x0f);
		nf += 7-m;
	}
	return nf;
}

} // namespace fs
