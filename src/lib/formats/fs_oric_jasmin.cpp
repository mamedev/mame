// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Management of Oric Jasmin floppy images

#include "emu.h"
#include "fs_oric_jasmin.h"
#include "oric_dsk.h"

// Floppy only, format is 41 tracks, 1/2 heads, 17 sectors.
// Filesystem has no subdirectories.
//
// References are pair of bytes (track + head*41, sector)
//
// Track 20 sector 1 has the free bitmap and the volume name
//
//   offset 3*(track + head*41): bitmap from track, 24 bits LSB first,
//                               sector 1 in bit 16, sector 17 in bit 0.
//                               0x01ffff is empty track. Special value
//                               0x800000 marks a full track.
//   offset f6-f7: 0x8080
//   offset f8-ff: volume name, padded with 0x20
//
//
// Track 20 sector 2 has the first directory sector.
// Directory sector format:
//   offset 00-01: reference to the sector, (0, 0) for the first one
//   offset 02-03: reference to the next sector, (0, 0) on the first sector if it's the end, (ff, 00) for the end of another sector
//   offset 04+  : 14 file entries, 18 bytes each
//     offset 00-01: reference to the first sector of the inode, (ff, xx) when no entry
//     offset 02   : U/L for (U)nlocked or (L)ocked file
//     offset 03-0e: filename.ext, space padding between filename and '.'
//     offset 0f   : S/D for (S)equential (normal) or (D)irect-access files
//     offset 10-11: number of sectors used by the file, including inode, little-endian
//
// Inodes:
//     offset 00-01: reference to the next inode sector, (ff, 00) on the last one
//     offset 02-03: loading address for the file on the first sector, ffff otherwise
//     offset 04-05: length of the file in bytes on the first sector, ffff otherwise
//     offset 06+  : reference to data sectors, (ff, ff) when done

void fs_oric_jasmin::enumerate_f(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	if(has(form_factor, variants, floppy_image::FF_3, floppy_image::DSDD))
		fe.add(this, FLOPPY_ORIC_JASMIN_FORMAT, 356864, "oric_jasmin_ds", "Oric Jasmin dual-sided");
	if(has(form_factor, variants, floppy_image::FF_3, floppy_image::SSDD))
		fe.add(this, FLOPPY_ORIC_JASMIN_FORMAT, 178432, "oric_jasmin_ss", "Oric Jasmin single-sided");
}

std::unique_ptr<filesystem_t> fs_oric_jasmin::mount(fsblk_t &blockdev) const
{
	return std::make_unique<impl>(blockdev);
}

bool fs_oric_jasmin::can_format() const
{
	return true;
}

bool fs_oric_jasmin::can_read() const
{
	return true;
}

bool fs_oric_jasmin::can_write() const
{
	return false;
}

bool fs_oric_jasmin::has_rsrc() const
{
	return false;
}

std::vector<fs_meta_description> fs_oric_jasmin::volume_meta_description() const
{
	std::vector<fs_meta_description> res;
	res.emplace_back(fs_meta_description(fs_meta_name::name, fs_meta_type::string, "UNTITLED", false, [](const fs_meta &m) { return m.as_string().size() <= 8; }, "Volume name, up to 8 characters"));

	return res;
}

fs_meta_data fs_oric_jasmin::impl::metadata()
{
	fs_meta_data res;
	auto bdir = m_blockdev.get(20*17);
	int len = 8;
	while(len > 0 && bdir.rodata()[0xf8 + len - 1] == ' ')
		len--;

	res[fs_meta_name::name] = bdir.rstr(0xf8, len);
	return res;	
}

bool fs_oric_jasmin::validate_filename(std::string name)
{
	auto pos = name.find('.');
	if(pos != std::string::npos)
		return pos <= 8 && pos > 0 && name.size()-pos-1 <= 3;
	else
		return name.size() > 0 && name.size() <= 8;
}

std::vector<fs_meta_description> fs_oric_jasmin::file_meta_description() const
{
	std::vector<fs_meta_description> res;
	res.emplace_back(fs_meta_description(fs_meta_name::name, fs_meta_type::string, "", false, [](const fs_meta &m) { return validate_filename(m.as_string()); }, "File name, 8.3"));
	res.emplace_back(fs_meta_description(fs_meta_name::loading_address, fs_meta_type::number, 0x501, false, [](const fs_meta &m) { return m.as_number() < 0x10000; }, "Loading address of the file"));
	res.emplace_back(fs_meta_description(fs_meta_name::length, fs_meta_type::number, 0, true, nullptr, "Size of the file in bytes"));
	res.emplace_back(fs_meta_description(fs_meta_name::size_in_blocks, fs_meta_type::number, 0, true, nullptr, "Number of blocks used by the file"));
	res.emplace_back(fs_meta_description(fs_meta_name::locked, fs_meta_type::flag, false, false, nullptr, "File locked"));
	res.emplace_back(fs_meta_description(fs_meta_name::sequential, fs_meta_type::flag, true, false, nullptr, "File sequential"));
	return res;
}


void fs_oric_jasmin::impl::format(const fs_meta_data &meta)
{
	std::string volume_name = meta.find(fs_meta_name::name)->second.as_string();
	u32 blocks = m_blockdev.block_count();

	m_blockdev.fill(0x6c);

	u32 bblk = 20*17;
	auto fmap = m_blockdev.get(bblk);
	u32 off = 0;
	for(u32 blk = 0; blk != blocks; blk += 17) {
		if(blk == bblk)
			fmap.w24l(off, 0x07fff);
		else
			fmap.w24l(off, 0x1ffff);
		off += 3;
	}

	for(u32 blk = blocks; blk != 17*42*2; blk += 17) {
		fmap.w24l(off, 0x800000);
		off += 3;
	}

	fmap.w8(0xf6, 0x80);
	fmap.w8(0xf7, 0x80);
	fmap.fill(0xf8, 0x20, 8);
	fmap.wstr(0xf8, volume_name);

	auto bdir = m_blockdev.get(20*17+1);
	bdir.fill(0xff);
	bdir.w16l(0, 0x0000);
	bdir.w16l(2, 0x0000);
}

fs_oric_jasmin::impl::impl(fsblk_t &blockdev) : filesystem_t(blockdev, 256), m_root(true)
{	
}

bool fs_oric_jasmin::impl::ref_valid(u16 ref)
{
	u8 track = ref >> 8;
	u8 sector = ref & 0xff;
	if(sector < 1 || sector > 17)
		return false;
	if(track >= m_blockdev.block_count()/17)
		return false;
	return true;
}

u32 fs_oric_jasmin::impl::cs_to_block(u16 ref)
{
	u8 track = ref >> 8;
	u8 sector = ref & 0xff;
	return track * 17 + sector - 1;
}

u16 fs_oric_jasmin::impl::block_to_cs(u32 block)
{
	u8 track = block / 17;
	u8 sector = (block % 17) + 1;
	return (track << 8) | sector;
}

std::string fs_oric_jasmin::impl::read_file_name(const u8 *p)
{
	int main_len;
	for(main_len = 8; main_len > 0; main_len--)
		if(p[main_len - 1] != ' ')
			break;
	int ext_len;
	for(ext_len = 4; ext_len > 0; ext_len--)
		if(p[ext_len + 8 - 1] != ' ')
			break;
	std::string name;
	for(int i=0; i != main_len; i++)
		name += char(p[i]);
	for(int i=0; i != ext_len; i++)
		name += char(p[8 + i]);
	return name;
}


filesystem_t::dir_t fs_oric_jasmin::impl::root()
{
	if(!m_root)
		m_root = new root_dir(*this);
	return m_root.strong();
}

void fs_oric_jasmin::impl::drop_root_ref()
{
	m_root = nullptr;
}

void fs_oric_jasmin::impl::root_dir::drop_weak_references()
{
	m_fs.drop_root_ref();
}

fs_meta_data fs_oric_jasmin::impl::root_dir::metadata()
{
	return fs_meta_data();
}

std::vector<fs_dir_entry> fs_oric_jasmin::impl::root_dir::contents()
{
	std::vector<fs_dir_entry> res;

	auto bdir = m_fs.m_blockdev.get(20*17+1);
	uint64_t id = 0;
	for(;;) {
		for(u32 i = 0; i != 14; i ++) {
			u32 off = 4 + i*18;
			u16 ref = bdir.r16b(off);
			std::string fname = read_file_name(bdir.rodata()+off+3);
			bool system = ref == 0 && id == 0 && bdir.r32b(off+0xb) == 0x2e535953;
			if(system)
				res.emplace_back(fs_dir_entry(fname, fs_dir_entry_type::system_file, 0));

			else if(m_fs.ref_valid(ref))
				res.emplace_back(fs_dir_entry(fname, fs_dir_entry_type::file, id));

			id++;
		}
		u16 ref = bdir.r16b(2);
		if(!ref || !m_fs.ref_valid(ref))
			break;
		bdir = m_fs.m_blockdev.get(cs_to_block(ref));
	}
	return res;
}

std::pair<fsblk_t::block_t, u32> fs_oric_jasmin::impl::root_dir::get_dir_block(uint64_t key)
{
	auto bdir = m_fs.m_blockdev.get(20*17+1);
	while(key >= 14) {
		u16 ref = bdir.r16b(2);
		if(!ref || !m_fs.ref_valid(ref))
			fatalerror("Incorrect file key\n");
		bdir = m_fs.m_blockdev.get(cs_to_block(ref));
		key -= 14;
	}
	return std::pair<fsblk_t::block_t, u32>(bdir, 4 + key * 18);
}

filesystem_t::file_t fs_oric_jasmin::impl::root_dir::file_get(uint64_t key)
{
	uint64_t rkey = key;
	auto [bdir, off] = get_dir_block(rkey);
	u16 ref = bdir.r16b(off);
	bool system = ref == 0 && key == 0 && bdir.r32b(off+0xb) == 0x2e535953;
	if(system)
		return file_t(new system_file(m_fs, bdir.rodata() + off));

	if(!m_fs.ref_valid(ref))
		fatalerror("Key to deleted/non-existent file\n");
	return file_t(new file(m_fs, bdir.rodata() + off, key));
}

filesystem_t::dir_t fs_oric_jasmin::impl::root_dir::dir_get(uint64_t key)
{
	fatalerror("Directories not supported\n");
}

fs_oric_jasmin::impl::file::file(impl &fs, const u8 *entry, u16 key) : m_fs(fs), m_key(key)
{
	memcpy(m_entry, entry, 18);
	(void)m_key;
}

void fs_oric_jasmin::impl::file::drop_weak_references()
{
}

fs_meta_data fs_oric_jasmin::impl::file::metadata()
{
	fs_meta_data res;

	res[fs_meta_name::name] = read_file_name(m_entry + 3);
	res[fs_meta_name::locked] = m_entry[2] == 'L';
	res[fs_meta_name::sequential] = m_entry[0xf] == 'S';
	res[fs_meta_name::size_in_blocks] = r16l(m_entry + 0x10);

	u16 ref = r16b(m_entry);
	auto dblk = m_fs.m_blockdev.get(cs_to_block(ref));
	res[fs_meta_name::loading_address] = uint64_t(dblk.r16l(2));
	res[fs_meta_name::length] = uint64_t(dblk.r16l(4));

	return res;
}

std::vector<u8> fs_oric_jasmin::impl::file::read_all()
{
	auto [sect, length] = build_data_sector_table();
	std::vector<u8> data(length);
	u32 pos = 0;
	for(u16 ref : sect) {
		u32 npos = pos + 256;
		if(npos > length)
			npos = length;
		if(npos > pos) {
			auto dblk = m_fs.m_blockdev.get(cs_to_block(ref));
			memcpy(data.data() + pos, dblk.rodata(), npos - pos);
		} else
			break;
		pos = npos;
	}
	return data;
}

std::vector<u8> fs_oric_jasmin::impl::file::read(u64 start, u64 length)
{
	auto [sect, rlength] = build_data_sector_table();
	length += start;
	if(length > rlength)
		length = rlength;

	if(rlength < start)
		return std::vector<u8>();

	std::vector<u8> data(length - start);
	u32 pos = 0;
	for(u16 ref : sect) {
		u32 npos = pos + 256;
		if(npos > length)
			npos = length;
		if(npos > pos) {
			if(npos > start) {
				u32 off = pos < start ? start & 0xff : 0;
				auto dblk = m_fs.m_blockdev.get(cs_to_block(ref));
				memcpy(data.data() + pos + off - start, dblk.rodata() + off, npos - pos - off);
			}
		} else
			break;
		pos = npos;
	}
	return data;
}

std::pair<std::vector<u16>, u32> fs_oric_jasmin::impl::file::build_data_sector_table()
{
	std::pair<std::vector<u16>, u32> res;
	u16 ref = r16b(m_entry);
	auto iblk = m_fs.m_blockdev.get(cs_to_block(ref));
	u32 length = iblk.r16l(4);
	while(m_fs.ref_valid(ref)) {
		for(u32 pos = 6; pos != 256; pos += 2) {
			u16 dref = iblk.r16b(pos);
			if(!m_fs.ref_valid(dref))
			   goto done;
			res.first.push_back(dref);
		}
		ref = iblk.r16b(2);
		if(!m_fs.ref_valid(ref))
			break;
		iblk = m_fs.m_blockdev.get(cs_to_block(ref));
	}
 done:
	if(length > 256 * res.first.size())
		length = 256 * res.first.size();
	res.second = length;
	return res;	
}

fs_oric_jasmin::impl::system_file::system_file(impl &fs, const u8 *entry) : m_fs(fs)
{
	memcpy(m_entry, entry, 18);
}

void fs_oric_jasmin::impl::system_file::drop_weak_references()
{
}

fs_meta_data fs_oric_jasmin::impl::system_file::metadata()
{
	fs_meta_data res;

	res[fs_meta_name::name] = read_file_name(m_entry + 3);
	res[fs_meta_name::locked] = m_entry[2] == 'L';
	res[fs_meta_name::sequential] = m_entry[0xf] == 'S';
	res[fs_meta_name::size_in_blocks] = r16l(m_entry + 0x10);
	res[fs_meta_name::length] = 0x3e00;

	return res;
}

std::vector<u8> fs_oric_jasmin::impl::system_file::read_all()
{
	std::vector<u8> data(0x3e00);
	for(u32 i = 0; i != 62; i++) {
		auto dblk = m_fs.m_blockdev.get(i);
		memcpy(data.data() + 256 * i, dblk.rodata(), 256);
	}
	return data;
}

std::vector<u8> fs_oric_jasmin::impl::system_file::read(u64 start, u64 length)
{
	if(start >= 0x3e00)
		return std::vector<u8>();
	length += start;
	if(length > 0x3e00)
		length = 0x3e00;
	std::vector<u8> data(length - start);
	
	for(u32 i = start / 256; i <= (length - 1) / 256; i++) {
		u32 pos = i * 256;
		u32 off = i * 256 < start ? start & 0xff : 0;
		u32 len = i * 256 > length ? length & 0xff : 0x100;
		auto dblk = m_fs.m_blockdev.get(i);
		memcpy(data.data() + pos + off - start, dblk.rodata() + off, len);
	}

	return data;
}


const filesystem_manager_type FS_ORIC_JASMIN = &filesystem_manager_creator<fs_oric_jasmin>;;
