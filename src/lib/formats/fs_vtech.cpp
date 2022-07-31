// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Management of VTech images

#include "fs_vtech.h"
#include "vt_dsk.h"

#include <stdexcept>

using namespace fs;

namespace fs { const vtech_image VTECH; }

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

namespace {

class vtech_impl : public filesystem_t {
public:
	vtech_impl(fsblk_t &blockdev);
	virtual ~vtech_impl() = default;

	virtual meta_data volume_metadata() override;
	virtual err_t volume_metadata_change(const meta_data &info) override;
	virtual std::pair<err_t, meta_data> metadata(const std::vector<std::string> &path) override;
	virtual err_t metadata_change(const std::vector<std::string> &path, const meta_data &meta) override;

	virtual std::pair<err_t, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;
	virtual err_t rename(const std::vector<std::string> &opath, const std::vector<std::string> &npath) override;
	virtual err_t remove(const std::vector<std::string> &path) override;

	virtual err_t file_create(const std::vector<std::string> &path, const meta_data &meta) override;

	virtual std::pair<err_t, std::vector<u8>> file_read(const std::vector<std::string> &path) override;
	virtual err_t file_write(const std::vector<std::string> &path, const std::vector<u8> &data) override;

	virtual err_t format(const meta_data &meta) override;

private:
	meta_data file_metadata(const u8 *entry);
	std::tuple<fsblk_t::block_t, u32> file_find(std::string name);
	std::vector<std::pair<u8, u8>> allocate_blocks(u32 count);
	void free_blocks(const std::vector<std::pair<u8, u8>> &blocks);
	u32 free_block_count();
};
}

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
	return std::make_unique<vtech_impl>(blockdev);
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

std::vector<meta_description> vtech_image::file_meta_description() const
{
	std::vector<meta_description> res;
	res.emplace_back(meta_description(meta_name::name, "", false, [](const meta_value &m) { return m.as_string().size() <= 8; }, "File name, 8 chars"));
	res.emplace_back(meta_description(meta_name::loading_address, 0x7ae9, false, [](const meta_value &m) { return m.as_number() < 0x10000; }, "Loading address of the file"));
	res.emplace_back(meta_description(meta_name::length, 0, true, nullptr, "Size of the file in bytes"));
	res.emplace_back(meta_description(meta_name::basic, true, true, nullptr, "Basic file"));
	return res;
}

vtech_impl::vtech_impl(fsblk_t &blockdev) : filesystem_t(blockdev, 128)
{
}

err_t vtech_impl::format(const meta_data &meta)
{
	m_blockdev.fill(0);
	return ERR_OK;
}

meta_data vtech_impl::volume_metadata()
{
	return meta_data();
}

err_t vtech_impl::volume_metadata_change(const meta_data &meta)
{
	return ERR_OK;
}

meta_data vtech_impl::file_metadata(const u8 *entry)
{
	meta_data res;

	res.set(meta_name::name, trim_end_spaces(rstr(entry+2, 8)));
	res.set(meta_name::basic, entry[0] == 'T');
	res.set(meta_name::loading_address, r16l(entry + 0xc));
	res.set(meta_name::length, ((r16l(entry + 0xe) - r16l(entry + 0xc) + 1) & 0xffff));

	return res;
}

std::tuple<fsblk_t::block_t, u32> vtech_impl::file_find(std::string name)
{
	for(int sect = 0; sect != 14; sect++) {
		auto bdir = m_blockdev.get(sect);
		for(u32 i = 0; i != 8; i ++) {
			u32 off = i*16;
			u8 type = bdir.r8(off);
			if(type != 'T' && type != 'B')
				continue;
			if(bdir.r8(off+1) != ':')
				continue;
			if(trim_end_spaces(bdir.rstr(off+2, 8)) == name) {
				return std::make_tuple(bdir, i);
			}
		}
	}
	return std::make_tuple(fsblk_t::block_t(), 0xffffffff);
}

std::pair<err_t, meta_data> vtech_impl::metadata(const std::vector<std::string> &path)
{
	if(path.size() != 1)
		return std::make_pair(ERR_NOT_FOUND, meta_data());

	auto [bdir, off] = file_find(path[0]);
	if(off == 0xffffffff)
		return std::make_pair(ERR_NOT_FOUND, meta_data());

	return std::make_pair(ERR_OK, file_metadata(bdir.rodata() + off));
}

err_t vtech_impl::metadata_change(const std::vector<std::string> &path, const meta_data &meta)
{
	if(path.size() != 1)
		return ERR_NOT_FOUND;

	auto [bdir, off] = file_find(path[0]);
	if(!off)
		return ERR_NOT_FOUND;

	u8 *entry = bdir.data() + off;
	if(meta.has(meta_name::basic))
		w8  (entry+0x0, meta.get_flag(meta_name::basic) ? 'T' : 'B');
	if(meta.has(meta_name::name)) {
		std::string name = meta.get_string(meta_name::name);
		name.resize(8, ' ');
		wstr(entry+0x2, name);
	}
	if(meta.has(meta_name::loading_address)) {
		u16 new_loading = meta.get_number(meta_name::loading_address);
		u16 new_end = r16l(entry + 0xe) - r16l(entry + 0xc) + new_loading;
		w16l(entry + 0xc, new_loading);
		w16l(entry + 0xe, new_end);
	}

	return ERR_OK;
}

std::pair<err_t, std::vector<dir_entry>> vtech_impl::directory_contents(const std::vector<std::string> &path)
{
	std::pair<err_t, std::vector<dir_entry>> res;

	if(path.size() != 0) {
		res.first = ERR_NOT_FOUND;
		return res;
	}

	res.first = ERR_OK;

	for(int sect = 0; sect != 14; sect++) {
		auto bdir = m_blockdev.get(sect);
		for(u32 i = 0; i != 8; i ++) {
			u32 off = i*16;
			u8 type = bdir.r8(off);
			if(type != 'T' && type != 'B')
				continue;
			if(bdir.r8(off+1) != ':')
				continue;
			meta_data meta = file_metadata(bdir.rodata()+off);
			res.second.emplace_back(dir_entry(dir_entry_type::file, meta));
		}
	}
	return res;
}

err_t vtech_impl::rename(const std::vector<std::string> &opath, const std::vector<std::string> &npath)
{
	if(opath.size() != 1 || npath.size() != 1)
		return ERR_NOT_FOUND;

	auto [bdir, off] = file_find(opath[0]);
	if(!off)
		return ERR_NOT_FOUND;

	std::string name = npath[0];
	name.resize(8, ' ');
	wstr(bdir.data() + off + 2, name);

	return ERR_OK;
}

err_t vtech_impl::remove(const std::vector<std::string> &path)
{
	return ERR_NOT_FOUND;
}


err_t vtech_impl::file_create(const std::vector<std::string> &path, const meta_data &meta)
{
	if(path.size() != 0)
		return ERR_NOT_FOUND;

	// Find the key for the next unused entry
	for(int sect = 0; sect != 14; sect++) {
		auto bdir = m_blockdev.get(sect);
		for(u32 i = 0; i != 16; i ++) {
			u32 off = i*16;
			u8 type = bdir.r8(off);
			if(type != 'T' && type != 'B') {
				std::string fname = meta.get_string(meta_name::name, "");
				fname.resize(8, ' ');

				bdir.w8  (off+0x0, meta.get_flag(meta_name::basic, true) ? 'T' : 'B');
				bdir.w8  (off+0x1, ':');
				bdir.wstr(off+0x2, fname);
				bdir.w8  (off+0xa, 0x00);
				bdir.w8  (off+0xb, 0x00);
				bdir.w16l(off+0xc, meta.get_number(meta_name::loading_address, 0x7ae9));
				bdir.w16l(off+0xe, bdir.r16l(off+0xc) - 1); // Size 0 initially
				return ERR_OK;
			}
		}
	}
	return ERR_NO_SPACE;
}

std::pair<err_t, std::vector<u8>> vtech_impl::file_read(const std::vector<std::string> &path)
{
	std::vector<u8> data;

	if(path.size() != 1)
		return std::make_pair(ERR_NOT_FOUND, data);

	auto [bdir, off] = file_find(path[0]);
	if(off == 0xffffffff)
		return std::make_pair(ERR_NOT_FOUND, data);

	const u8 *entry = bdir.rodata() + off;

	u8 track = entry[0xa];
	u8 sector = entry[0xb];
	int len = ((r16l(entry + 0xe) - r16l(entry + 0xc)) & 0xffff) + 1;

	data.resize(len, 0);
	int pos = 0;
	while(pos < len) {
		if(track >= 40 || sector >= 16)
			break;
		auto dblk = m_blockdev.get(track*16 + sector);
		int size = len - pos;
		if(size > 126)
			size = 126;
		memcpy(data.data() + pos, dblk.data(), size);
		pos += size;
		track = dblk.r8(126);
		sector = dblk.r8(127);
	}
	return std::make_pair(ERR_OK, data);
}

err_t vtech_impl::file_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	if(path.size() != 1)
		return ERR_NOT_FOUND;

	auto [bdir, off] = file_find(path[0]);
	if(off == 0xffffffff)
		return ERR_NOT_FOUND;

	u8 *entry = bdir.data() + off;

	u32 cur_len = ((r16l(entry + 0xe) - r16l(entry + 0xc) + 1) & 0xffff);
	u32 new_len = data.size();
	if(new_len > 65535)
		new_len = 65535;
	u32 cur_ns = (cur_len + 125)/126;
	u32 need_ns = (new_len + 125) / 126;

	// Enough space?
	if(cur_ns < need_ns && free_block_count() < need_ns - cur_ns)
		return ERR_NO_SPACE;

	u8 track = entry[0xa];
	u8 sector = entry[0xb];
	std::vector<std::pair<u8, u8>> tofree;
	for(u32 i = 0; i != cur_ns; i++) {
		tofree.emplace_back(std::make_pair(track, sector));
		auto dblk = m_blockdev.get(track*16 + sector);
		track = dblk.r8(126);
		sector = dblk.r8(127);
	}

	free_blocks(tofree);

	std::vector<std::pair<u8, u8>> blocks = allocate_blocks(need_ns);
	for(u32 i=0; i != need_ns; i ++) {
		auto dblk = m_blockdev.get(blocks[i].first * 16 + blocks[i].second);
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

	u16 end_address = (r16l(entry + 0xc) + data.size() - 1) & 0xffff;
	w16l(entry + 0xe, end_address);
	if(need_ns) {
		w8(entry + 0xa, blocks[0].first);
		w8(entry + 0xb, blocks[0].second);
	} else
		w16l(entry + 0xa, 0);

	return ERR_OK;
}

std::vector<std::pair<u8, u8>> vtech_impl::allocate_blocks(u32 count)
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

void vtech_impl::free_blocks(const std::vector<std::pair<u8, u8>> &blocks)
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

u32 vtech_impl::free_block_count()
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
