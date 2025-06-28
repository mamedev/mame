// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Management of Oric Jasmin floppy images

#include "fs_oric_jasmin.h"
#include "fsblk.h"
#include "oric_dsk.h"

#include "multibyte.h"

#include <stdexcept>

using namespace fs;

namespace fs { const oric_jasmin_image ORIC_JASMIN; }


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

namespace {

class oric_jasmin_impl : public filesystem_t {
public:
	oric_jasmin_impl(fsblk_t &blockdev);
	virtual ~oric_jasmin_impl() = default;

	virtual meta_data volume_metadata() override;
	virtual std::error_condition volume_metadata_change(const meta_data &info) override;
	virtual std::pair<std::error_condition, meta_data> metadata(const std::vector<std::string> &path) override;
	virtual std::error_condition metadata_change(const std::vector<std::string> &path, const meta_data &meta) override;

	virtual std::pair<std::error_condition, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;
	virtual std::error_condition rename(const std::vector<std::string> &opath, const std::vector<std::string> &npath) override;
	virtual std::error_condition remove(const std::vector<std::string> &path) override;

	virtual std::error_condition file_create(const std::vector<std::string> &path, const meta_data &meta) override;

	virtual std::pair<std::error_condition, std::vector<u8>> file_read(const std::vector<std::string> &path) override;
	virtual std::error_condition file_write(const std::vector<std::string> &path, const std::vector<u8> &data) override;

	virtual std::error_condition format(const meta_data &meta) override;

	static bool validate_filename(std::string name);

private:
	static u32 cs_to_block(u16 ref);
	[[maybe_unused]] static u16 block_to_cs(u32 block);

	bool ref_valid(u16 ref);
	static std::string read_file_name(const u8 *p);

	std::vector<u16> allocate_blocks(u32 count);
	void free_blocks(const std::vector<u16> &blocks);
	u32 free_block_count();

	static std::string file_name_read(const u8 *p);
	static std::string file_name_prepare(std::string name);
	static bool file_is_system(const u8 *entry);
	meta_data file_metadata(const u8 *entry);
	std::tuple<fsblk_t::block_t::ptr, u32, bool> file_find(std::string name);
};
}


const char *oric_jasmin_image::name() const
{
	return "oric_jasmin";
}

const char *oric_jasmin_image::description() const
{
	return "Oric Jasmin";
}

void oric_jasmin_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_ORIC_JASMIN_FORMAT, floppy_image::FF_3, floppy_image::DSDD, 356864, "oric_jasmin_ds", "Oric Jasmin dual-sided");
	fe.add(FLOPPY_ORIC_JASMIN_FORMAT, floppy_image::FF_3, floppy_image::SSDD, 178432, "oric_jasmin_ss", "Oric Jasmin single-sided");
}

std::unique_ptr<filesystem_t> oric_jasmin_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<oric_jasmin_impl>(blockdev);
}

bool oric_jasmin_image::can_format() const
{
	return true;
}

bool oric_jasmin_image::can_read() const
{
	return true;
}

bool oric_jasmin_image::can_write() const
{
	return true;
}

bool oric_jasmin_image::has_rsrc() const
{
	return false;
}

std::vector<meta_description> oric_jasmin_image::volume_meta_description() const
{
	std::vector<meta_description> res;
	res.emplace_back(meta_description(meta_name::name, "UNTITLED", false, [](const meta_value &m) { return m.as_string().size() <= 8; }, "Volume name, up to 8 characters"));

	return res;
}

std::vector<meta_description> oric_jasmin_image::file_meta_description() const
{
	std::vector<meta_description> res;
	res.emplace_back(meta_description(meta_name::name, "", false, [](const meta_value &m) { return oric_jasmin_impl::validate_filename(m.as_string()); }, "File name, 8.3"));
	res.emplace_back(meta_description(meta_name::loading_address, 0x501, false, [](const meta_value &m) { return m.as_number() < 0x10000; }, "Loading address of the file"));
	res.emplace_back(meta_description(meta_name::length, 0, true, nullptr, "Size of the file in bytes"));
	res.emplace_back(meta_description(meta_name::size_in_blocks, 0, true, nullptr, "Number of blocks used by the file"));
	res.emplace_back(meta_description(meta_name::locked, false, false, nullptr, "File locked"));
	res.emplace_back(meta_description(meta_name::sequential, true, false, nullptr, "File sequential"));
	return res;
}



oric_jasmin_impl::oric_jasmin_impl(fsblk_t &blockdev) : filesystem_t(blockdev, 256)
{
}

bool oric_jasmin_impl::ref_valid(u16 ref)
{
	u8 track = ref >> 8;
	u8 sector = ref & 0xff;
	if(sector < 1 || sector > 17)
		return false;
	if(track >= m_blockdev.block_count()/17)
		return false;
	return true;
}

u32 oric_jasmin_impl::cs_to_block(u16 ref)
{
	u8 track = ref >> 8;
	if(track == 0xff)
		abort();
	u8 sector = ref & 0xff;
	return track * 17 + sector - 1;
}

u16 oric_jasmin_impl::block_to_cs(u32 block)
{
	u8 track = block / 17;
	u8 sector = (block % 17) + 1;
	return (track << 8) | sector;
}

bool oric_jasmin_impl::validate_filename(std::string name)
{
	auto pos = name.find('.');
	if(pos != std::string::npos)
		return pos <= 8 && pos > 0 && name.size()-pos-1 <= 3;
	else
		return name.size() > 0 && name.size() <= 8;
}

std::error_condition oric_jasmin_impl::format(const meta_data &meta)
{
	std::string volume_name = meta.get_string(meta_name::name, "UNTITLED");
	u32 blocks = m_blockdev.block_count();

	m_blockdev.fill_all(0x6c);

	u32 bblk = 20*17;
	auto fmap = m_blockdev.get(bblk);
	u32 off = 0;
	for(u32 blk = 0; blk != blocks; blk += 17) {
		if(blk == bblk)
			fmap->w24l(off, 0x07fff);
		else
			fmap->w24l(off, 0x1ffff);
		off += 3;
	}

	for(u32 blk = blocks; blk != 17*42*2; blk += 17) {
		fmap->w24l(off, 0x800000);
		off += 3;
	}

	fmap->w8(0xf6, 0x80);
	fmap->w8(0xf7, 0x80);
	volume_name.resize(8, ' ');
	fmap->wstr(0xf8, volume_name);

	auto bdir = m_blockdev.get(20*17+1);
	bdir->fill(0xff);
	bdir->w16l(0, 0x0000);
	bdir->w16l(2, 0x0000);

	return std::error_condition();
}

meta_data oric_jasmin_impl::volume_metadata()
{
	meta_data res;
	auto bdir = m_blockdev.get(20*17);
	int len = 8;
	while(len > 0 && bdir->rodata()[0xf8 + len - 1] == ' ')
		len--;

	res.set(meta_name::name, bdir->rstr(0xf8, len));
	return res;
}

std::error_condition oric_jasmin_impl::volume_metadata_change(const meta_data &meta)
{
	if(meta.has(meta_name::name)) {
		std::string volume_name = meta.get_string(meta_name::name);
		volume_name.resize(8, ' ');
		m_blockdev.get(20*17)->wstr(0xf8, volume_name);
	}
	return std::error_condition();
}

std::string oric_jasmin_impl::file_name_prepare(std::string fname)
{
	std::string nname;
	size_t i;
	for(i = 0; i != 8 && i != fname.size() && fname[i] != '.'; i++)
		nname += fname[i];
	if(nname.size() != 8)
		nname.insert(nname.end(), 8 - nname.size(), ' ');
	nname += '.';
	while(i != fname.size() && fname[i] != '.')
		i++;
	if(i != fname.size())
		i++;
	while(i != fname.size() && nname.size() != 12)
		nname += fname[i++];
	if(nname.size() != 12)
		nname.insert(nname.end(), 12 - nname.size(), ' ');
	return nname;
}

std::string oric_jasmin_impl::file_name_read(const u8 *p)
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

bool oric_jasmin_impl::file_is_system(const u8 *entry)
{
	u16 ref = get_u16be(entry);
	return ref == 0 && get_u32be(entry+0xb) == 0x2e535953;
}


meta_data oric_jasmin_impl::file_metadata(const u8 *entry)
{
	meta_data res;

	res.set(meta_name::name, file_name_read(entry + 3));
	res.set(meta_name::locked, entry[2] == 'L');
	res.set(meta_name::sequential, entry[0xf] == 'S');
	res.set(meta_name::size_in_blocks, get_u16le(entry + 0x10));

	bool sys = file_is_system(entry);
	if(sys)
		res.set(meta_name::length, 0x3e00);

	else {
		u16 ref = get_u16be(entry);
		auto dblk = m_blockdev.get(cs_to_block(ref));
		res.set(meta_name::loading_address, dblk->r16l(2));
		res.set(meta_name::length, dblk->r16l(4));
	}
	return res;
}

std::tuple<fsblk_t::block_t::ptr, u32, bool> oric_jasmin_impl::file_find(std::string name)
{
	name = file_name_prepare(name);
	auto bdir = m_blockdev.get(20*17+1);
	for(;;) {
		for(u32 i = 0; i != 14; i ++) {
			u32 off = 4 + i*18;
			u16 fref = bdir->r16b(off);
			if(ref_valid(fref) || file_is_system(bdir->rodata()+off)) {
				if(memcmp(bdir->rodata() + off + 3, name.data(), 12)) {
					bool sys = file_is_system(bdir->rodata() + off);
					return std::make_tuple(bdir, off, sys);
				}
			}
		}
		u16 ref = bdir->r16b(2);
		if(!ref || !ref_valid(ref))
			return std::make_tuple(bdir, 0U, false);

		bdir = m_blockdev.get(cs_to_block(ref));
	}
}

std::pair<std::error_condition, meta_data> oric_jasmin_impl::metadata(const std::vector<std::string> &path)
{
	if(path.size() != 1)
		return std::make_pair(error::not_found, meta_data());

	auto [bdir, off, sys] = file_find(path[0]);
	std::ignore = sys;
	if(!off)
		return std::make_pair(error::not_found, meta_data());

	return std::make_pair(std::error_condition(), file_metadata(bdir->rodata() + off));
}

std::error_condition oric_jasmin_impl::metadata_change(const std::vector<std::string> &path, const meta_data &meta)
{
	if(path.size() != 1)
		return error::not_found;

	auto [bdir, off, sys] = file_find(path[0]);
	if(!off)
		return error::not_found;

	u8 *entry = bdir->data() + off;
	if(meta.has(meta_name::locked))
		entry[0x02] = meta.get_flag(meta_name::locked) ? 'L' : 'U';
	if(meta.has(meta_name::name))
		wstr(entry+0x03, file_name_prepare(meta.get_string(meta_name::name)));
	if(meta.has(meta_name::sequential))
		entry[0x0f] = meta.get_flag(meta_name::sequential) ? 'D' : 'S';
	if(!sys && meta.has(meta_name::loading_address))
		m_blockdev.get(cs_to_block(get_u16be(entry)))->w16l(2, meta.get_number(meta_name::loading_address));

	return std::error_condition();
}

std::pair<std::error_condition, std::vector<dir_entry>> oric_jasmin_impl::directory_contents(const std::vector<std::string> &path)
{
	std::pair<std::error_condition, std::vector<dir_entry>> res;

	if(path.size() != 0) {
		res.first = error::not_found;
		return res;
	}

	res.first = std::error_condition();

	auto bdir = m_blockdev.get(20*17+1);
	for(;;) {
		for(u32 i = 0; i != 14; i ++) {
			u32 off = 4 + i*18;
			u16 fref = bdir->r16b(off);
			if(ref_valid(fref) || file_is_system(bdir->rodata()+off)) {
				meta_data meta = file_metadata(bdir->rodata()+off);
				res.second.emplace_back(dir_entry(dir_entry_type::file, meta));
			}
		}
		u16 ref = bdir->r16b(2);
		if(!ref || !ref_valid(ref))
			break;
		bdir = m_blockdev.get(cs_to_block(ref));
	}
	return res;
}

std::error_condition oric_jasmin_impl::rename(const std::vector<std::string> &opath, const std::vector<std::string> &npath)
{
	if(opath.size() != 1 || npath.size() != 1)
		return error::not_found;

	auto [bdir, off, sys] = file_find(opath[0]);
	std::ignore = sys;
	if(!off)
		return error::not_found;

	wstr(bdir->data() + off + 0x03, file_name_prepare(npath[0]));

	return std::error_condition();
}

std::error_condition oric_jasmin_impl::remove(const std::vector<std::string> &path)
{
	return error::unsupported;
}

std::error_condition oric_jasmin_impl::file_create(const std::vector<std::string> &path, const meta_data &meta)
{
	if(path.size() != 0)
		return error::not_found;

	// One block of sector list, one block of data
	u32 nb = 2;

	// Find the key for the next entry, increase nb if needed
	auto bdir = m_blockdev.get(20*17+1);
	u64 id = 0;
	for(;;) {
		for(u32 i = 0; i != 14; i ++) {
			u32 off = 4 + i*18;
			u16 ref = bdir->r16b(off);
			if(!ref_valid(ref))
				goto found;
			id++;
		}
		u16 ref = bdir->r16b(2);
		if(!ref || !ref_valid(ref)) {
			nb ++;
			break;
		}
		bdir = m_blockdev.get(cs_to_block(ref));
	}
 found:
	auto block = allocate_blocks(nb);
	if(block.empty())
		return error::no_space;

	auto sblk = m_blockdev.get(cs_to_block(block[0]));
	sblk->w16b(0, 0xff00);   // Next sector
	sblk->w16l(2, meta.get_number(meta_name::loading_address, 0x500));
	sblk->w16l(4, 0);        // Length
	sblk->w16b(6, block[1]); // Data block

	if(nb == 3) {
		bdir->w16l(0, block[2]);    // Link to the next directory sector
		bdir = m_blockdev.get(cs_to_block(block[2]));
		bdir->fill(0xff);
		bdir->w16l(0, block[2]); // Reference to itself
		bdir->w16l(2, 0xff00);   // No next directory sector
	}

	u32 off = 4 + (id % 14) * 18;
	bdir->w16b(off+0x00, block[0]); // First (and only) sector in the sector list
	bdir->w8  (off+0x02, meta.get_flag(meta_name::locked, false) ? 'L' : 'U');
	bdir->wstr(off+0x03, file_name_prepare(meta.get_string(meta_name::name, "")));
	bdir->w8  (off+0x0f, meta.get_flag(meta_name::sequential, true) ? 'S' : 'D');
	bdir->w16l(off+0x10, 2); // 2 sectors for an empty file

	return std::error_condition();
}

std::pair<std::error_condition, std::vector<u8>> oric_jasmin_impl::file_read(const std::vector<std::string> &path)
{
	std::vector<u8> data;

	if(path.size() != 1)
		return std::make_pair(error::not_found, data);

	auto [bdir, off, sys] = file_find(path[0]);
	if(!off)
		return std::make_pair(error::not_found, data);

	if(sys) {
		data.resize(0x3e00);
		for(u32 i = 0; i != 62; i++) {
			auto dblk = m_blockdev.get(i);
			dblk->read(0, data.data() + 256 * i, 256);
		}

	} else {
		const u8 *entry = bdir->rodata() + off;
		u16 ref = get_u16be(entry);
		auto iblk = m_blockdev.get(cs_to_block(ref));
		u32 length = iblk->r16l(4);
		while(ref_valid(ref)) {
			for(u32 pos = 6; pos != 256 && data.size() < length; pos += 2) {
				u16 dref = iblk->r16b(pos);
				if(!ref_valid(dref))
					goto done;
				auto dblk = m_blockdev.get(cs_to_block(dref));
				u32 dpos = data.size();
				data.resize(dpos + 256);
				dblk->read(0, data.data() + dpos, 256);
				if(data.size() >= length)
					goto done;
			}
			ref = iblk->r16b(2);
			if(!ref_valid(ref))
				break;
			iblk = m_blockdev.get(cs_to_block(ref));
		}
	done:
		data.resize(length);
	}

	return std::make_pair(std::error_condition(), data);
}

std::error_condition oric_jasmin_impl::file_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	if(path.size() != 1)
		return error::not_found;

	auto [bdir, off, sys] = file_find(path[0]);
	if(!off)
		return error::not_found;

	if(sys) {
		if(data.size() != 0x3e00)
			return error::incorrect_size;

		for(u32 i=0; i != 0x3e; i++)
			m_blockdev.get(i)->write(0, data.data() + i * 256, 256);

	} else {
		u8 *entry = bdir->data() + off;
		u32 cur_ns = get_u16le(entry + 0x10);
		// Data sectors first
		u32 need_ns = (data.size() + 255) / 256;
		if(need_ns == 0)
			need_ns = 1;
		// Add the sector list sectors, 125 entries/sector
		need_ns += (need_ns + 124)/125;

		// Enough space?
		if(cur_ns < need_ns && free_block_count() < need_ns - cur_ns)
			return error::no_space;

		u16 load_address = 0;
		std::vector<u16> tofree;
		u16 iref = get_u16be(entry);
		for(u32 i=0; i < cur_ns; i += 125+1) {
			auto iblk = m_blockdev.get(cs_to_block(iref));
			if(!i)
				load_address = iblk->r16l(2);
			tofree.push_back(iref);
			for(u32 j=0; j != 125 && i+j+1 != cur_ns; j++)
				tofree.push_back(iblk->r16b(6+2*j));
			iref = iblk->r16b(2);
		}
		free_blocks(tofree);

		std::vector<u16> blocks = allocate_blocks(need_ns);
		for(u32 i=0; i < need_ns; i += 125+1) {
			auto iblk = m_blockdev.get(cs_to_block(blocks[i]));
			iblk->fill(0xff);
			if(!i) {
				iblk->w16l(2, load_address);
				iblk->w16l(4, data.size());
			}
			if(i + 126 < need_ns)
				iblk->w16b(0, blocks[i+126]);
			else
				iblk->w16b(0, 0xff00);

			for(u32 j=0; j != 125 && i+j+1 != need_ns; j++) {
				u32 dpos = 256 * (j + i/126*125);
				u32 size = data.size() - dpos;
				iblk->w16b(6+j*2, blocks[i+j+1]);
				auto dblk = m_blockdev.get(cs_to_block(blocks[i+j+1]));
				if(size >= 256)
					dblk->write(0, data.data() + dpos, 256);
				else {
					dblk->write(0, data.data() + dpos, size);
					dblk->fill(size, 0x55, 256-size);
				}
			}
		}
		put_u16le(entry + 0x10, need_ns);
		put_u16be(entry + 0x00, blocks[0]);
	}
	return std::error_condition();
}

std::vector<u16> oric_jasmin_impl::allocate_blocks(u32 count)
{
	std::vector<u16> blocks;
	if(free_block_count() < count)
		return blocks;

	auto fmap = m_blockdev.get(20*17);
	u32 nf = 0;
	for(u32 track = 0; track != 2*41 && nf != count; track++) {
		u32 map = fmap->r24l(track*3);
		if(map != 0x800000) {
			for(u32 sect = 1; sect <= 17 && nf != count; sect++)
				if(map & (0x20000 >> sect)) {
					blocks.push_back((track << 8) | sect);
					map &= ~(0x20000 >> sect);
					nf++;
				}
			if(!map)
				map = 0x800000;
			fmap->w24l(track*3, map);
		}
	}
	return blocks;
}

void oric_jasmin_impl::free_blocks(const std::vector<u16> &blocks)
{
	auto fmap = m_blockdev.get(20*17);
	for(u16 ref : blocks) {
		u32 track = ref >> 8;
		u32 sect = ref & 0xff;
		u32 map = fmap->r24l(track*3);
		if(map == 0x800000)
			map = 0;
		map |= 0x20000 >> sect;
		fmap->w24l(track*3, map);
	}
}

u32 oric_jasmin_impl::free_block_count()
{
	auto fmap = m_blockdev.get(20*17);
	u32 nf = 0;
	for(u32 track = 0; track != 2*41; track++) {
		u32 map = fmap->r24l(track*3);
		if(map != 0x800000) {
			for(u32 sect = 1; sect <= 17; sect++)
				if(map & (0x20000 >> sect))
					nf++;
		}
	}
	return nf;
}
