// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem management code for floppy and hd images

// Currently limited to floppies and creation of preformatted images

#include "emu.h"
#include "fsmgr.h"

void fs_refcounted_inner::ref()
{
	m_ref ++;
}

void fs_refcounted_inner::ref_weak()
{
	m_weak_ref ++;
}

bool fs_refcounted_inner::unref()
{
	m_ref --;
	if(m_ref == 0) {
		if(m_weak_ref)
			drop_weak_references();
		else
			delete this;
		return true;
	}
	return false;
}

bool fs_refcounted_inner::unref_weak()
{
	m_weak_ref --;
	if(m_weak_ref == 0 && m_ref == 0) {
		delete this;
		return true;
	}
	return false;
}



void filesystem_manager_t::enumerate_f(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
}

void filesystem_manager_t::enumerate_h(hd_enumerator &he) const
{
}

void filesystem_manager_t::enumerate_c(cdrom_enumerator &ce) const
{
}

bool filesystem_manager_t::has_variant(const std::vector<uint32_t> &variants, uint32_t variant)
{
	for(uint32_t v : variants)
		if(variant == v)
			return true;
	return false;
}

bool filesystem_manager_t::has(uint32_t form_factor, const std::vector<uint32_t> &variants, uint32_t ff, uint32_t variant)
{
	if(form_factor == floppy_image::FF_UNKNOWN)
		return true;
	if(form_factor != ff)
		return false;
	for(uint32_t v : variants)
		if(variant == v)
			return true;
	return false;
}

std::vector<fs_meta_description> filesystem_manager_t::volume_meta_description() const
{
	std::vector<fs_meta_description> res;
	return res;
}

std::vector<fs_meta_description> filesystem_manager_t::file_meta_description() const
{
	std::vector<fs_meta_description> res;
	return res;
}

std::vector<fs_meta_description> filesystem_manager_t::directory_meta_description() const
{
	std::vector<fs_meta_description> res;
	return res;
}

char filesystem_manager_t::directory_separator() const
{
	return 0; // Subdirectories not supported by default
}

void filesystem_t::format(const fs_meta_data &meta)
{
	fatalerror("format called on a filesystem not supporting it.\n");
}

filesystem_t::dir_t filesystem_t::root()
{
	fatalerror("root called on a filesystem not supporting it.\n");
}

fs_meta_data filesystem_t::metadata()
{
	fatalerror("filesystem_t::metadata called on a filesystem not supporting it.\n");
}

void fsblk_t::set_block_size(uint32_t block_size)
{
	m_block_size = block_size;
}


uint8_t *fsblk_t::iblock_t::offset(const char *function, uint32_t off, uint32_t size)
{
	if(off + size > m_size)
		fatalerror("block_t::%s out-of-block access, offset=%d, size=%d, block size=%d\n", function, off, size, m_size);
	return data() + off;
}

const uint8_t *fsblk_t::iblock_t::rooffset(const char *function, uint32_t off, uint32_t size)
{
	if(off + size > m_size)
		fatalerror("block_t::%s out-of-block read access, offset=%d, size=%d, block size=%d\n", function, off, size, m_size);
	return rodata() + off;
}

void fsblk_t::block_t::copy(u32 offset, const uint8_t *src, u32 size)
{
	uint8_t *blk = m_object->offset("copy", offset, size);
	memcpy(blk, src, size);
}

void fsblk_t::block_t::fill(u32 offset, uint8_t data, u32 size)
{
	uint8_t *blk = m_object->offset("fill", offset, size);
	memset(blk, data, size);
}

void fsblk_t::block_t::fill(uint8_t data)
{
	uint8_t *blk = m_object->data();
	memset(blk, data, m_object->size());
}

void fsblk_t::block_t::wstr(u32 offset, const std::string &str)
{
	uint8_t *blk = m_object->offset("wstr", offset, str.size());
	memcpy(blk, str.data(), str.size());
}

void fsblk_t::block_t::w8(u32 offset, uint8_t data)
{
	uint8_t *blk = m_object->offset("w8", offset, 1);
	blk[0] = data;
}

void fsblk_t::block_t::w16b(u32 offset, u16 data)
{
	uint8_t *blk = m_object->offset("w16b", offset, 2);
	blk[0] = data >> 8;
	blk[1] = data;
}

void fsblk_t::block_t::w24b(u32 offset, u32 data)
{
	uint8_t *blk = m_object->offset("w24b", offset, 3);
	blk[0] = data >> 16;
	blk[1] = data >> 8;
	blk[2] = data;
}

void fsblk_t::block_t::w32b(u32 offset, u32 data)
{
	uint8_t *blk = m_object->offset("w32b", offset, 4);
	blk[0] = data >> 24;
	blk[1] = data >> 16;
	blk[2] = data >> 8;
	blk[3] = data;
}

void fsblk_t::block_t::w16l(u32 offset, u16 data)
{
	uint8_t *blk = m_object->offset("w16l", offset, 2);
	blk[0] = data;
	blk[1] = data >> 8;
}

void fsblk_t::block_t::w24l(u32 offset, u32 data)
{
	uint8_t *blk = m_object->offset("w24l", offset, 3);
	blk[0] = data;
	blk[1] = data >> 8;
	blk[2] = data >> 16;
}

void fsblk_t::block_t::w32l(u32 offset, u32 data)
{
	uint8_t *blk = m_object->offset("w32l", offset, 4);
	blk[0] = data;
	blk[1] = data >> 8;
	blk[2] = data >> 16;
	blk[3] = data >> 24;
}

std::string fsblk_t::block_t::rstr(u32 offset, u32 size)
{
	const u8 *d = m_object->rooffset("rstr", offset, size);
	std::string res;
	for(u32 i=0; i != size; i++)
		res += char(*d++);
	return res;
}

uint8_t fsblk_t::block_t::r8(u32 offset)
{
	const uint8_t *blk = m_object->offset("r8", offset, 1);
	return blk[0];
}

uint16_t fsblk_t::block_t::r16b(u32 offset)
{
	const uint8_t *blk = m_object->offset("r16b", offset, 2);
	return (blk[0] << 8) | blk[1];
}

uint32_t fsblk_t::block_t::r24b(u32 offset)
{
	const uint8_t *blk = m_object->offset("r24b", offset, 3);
	return (blk[0] << 16) | (blk[1] << 8) | blk[2];
}

uint32_t fsblk_t::block_t::r32b(u32 offset)
{
	const uint8_t *blk = m_object->offset("r32b", offset, 4);
	return (blk[0] << 24) | (blk[1] << 16) | (blk[2] << 8) | blk[3];
}

uint16_t fsblk_t::block_t::r16l(u32 offset)
{
	const uint8_t *blk = m_object->offset("r16l", offset, 2);
	return blk[0] | (blk[1] << 8);
}

uint32_t fsblk_t::block_t::r24l(u32 offset)
{
	const uint8_t *blk = m_object->offset("r24l", offset, 3);
	return blk[0] | (blk[1] << 8) | (blk[2] << 16);
}

uint32_t fsblk_t::block_t::r32l(u32 offset)
{
	const uint8_t *blk = m_object->offset("r32l", offset, 4);
	return blk[0] | (blk[1] << 8) | (blk[2] << 16) | (blk[3] << 24);
}



void filesystem_t::copy(uint8_t *p, const uint8_t *src, u32 size)
{
	memcpy(p, src, size);
}

void filesystem_t::fill(uint8_t *p, uint8_t data, u32 size)
{
	memset(p, data, size);
}

void filesystem_t::wstr(uint8_t *p, const std::string &str)
{
	memcpy(p, str.data(), str.size());
}

void filesystem_t::w8(uint8_t *p, uint8_t data)
{
	p[0] = data;
}

void filesystem_t::w16b(uint8_t *p, u16 data)
{
	p[0] = data >> 8;
	p[1] = data;
}

void filesystem_t::w24b(uint8_t *p, u32 data)
{
	p[0] = data >> 16;
	p[1] = data >> 8;
	p[2] = data;
}

void filesystem_t::w32b(uint8_t *p, u32 data)
{
	p[0] = data >> 24;
	p[1] = data >> 16;
	p[2] = data >> 8;
	p[3] = data;
}

void filesystem_t::w16l(uint8_t *p, u16 data)
{
	p[0] = data;
	p[1] = data >> 8;
}

void filesystem_t::w24l(uint8_t *p, u32 data)
{
	p[0] = data;
	p[1] = data >> 8;
	p[2] = data >> 16;
}

void filesystem_t::w32l(uint8_t *p, u32 data)
{
	p[0] = data;
	p[1] = data >> 8;
	p[2] = data >> 16;
	p[3] = data >> 24;
}

std::string filesystem_t::rstr(const uint8_t *p, u32 size)
{
	std::string res;
	for(u32 i=0; i != size; i++)
		res += char(*p++);
	return res;
}

uint8_t filesystem_t::r8(const uint8_t *p)
{
	return p[0];
}

uint16_t filesystem_t::r16b(const uint8_t *p)
{
	return (p[0] << 8) | p[1];
}

uint32_t filesystem_t::r24b(const uint8_t *p)
{
	return (p[0] << 16) | (p[1] << 8) | p[2];
}

uint32_t filesystem_t::r32b(const uint8_t *p)
{
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

uint16_t filesystem_t::r16l(const uint8_t *p)
{
	return p[0] | (p[1] << 8);
}

uint32_t filesystem_t::r24l(const uint8_t *p)
{
	return p[0] | (p[1] << 8) | (p[2] << 16);
}

uint32_t filesystem_t::r32l(const uint8_t *p)
{
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

std::string filesystem_t::trim_end_spaces(const std::string &str)
{
	const auto i = str.find_last_not_of(' ');
	return str.substr(0, (std::string::npos != i) ? (i + 1) : 0);
}

filesystem_t::file_t filesystem_t::idir_t::file_create(const fs_meta_data &info)
{
	fatalerror("file_create called on a filesystem not supporting write\n");
}

void filesystem_t::idir_t::file_delete(uint64_t key)
{
	fatalerror("file_delete called on a filesystem not supporting write\n");
}


void filesystem_t::ifile_t::replace(const std::vector<u8> &data)
{
	fatalerror("replace called on a filesystem not supporting write \n");
}

void filesystem_t::ifile_t::rsrc_replace(const std::vector<u8> &data)
{
	fatalerror("rsrc_replace called on a filesystem not supporting write or resource forks \n");
}

void filesystem_t::ifile_t::metadata_change(const fs_meta_data &info)
{
	fatalerror("metadata_change called on a filesystem not supporting write \n");
}

void filesystem_t::idir_t::metadata_change(const fs_meta_data &info)
{
	fatalerror("metadata_change called on a filesystem not supporting write \n");
}

void filesystem_t::metadata_change(const fs_meta_data &info)
{
	fatalerror("metadata_change called on a filesystem not supporting write \n");
}

std::vector<u8> filesystem_t::ifile_t::rsrc_read_all()
{
	fatalerror("rsrc_read_all called on a filesystem without resource forks\n");
}

const char *fs_meta_data::entry_name(fs_meta_name name)
{
	switch(name) {
	case fs_meta_name::basic: return "basic";
	case fs_meta_name::creation_date: return "creation_date";
	case fs_meta_name::length: return "length";
	case fs_meta_name::loading_address: return "loading_address";
	case fs_meta_name::locked: return "locked";
	case fs_meta_name::modification_date: return "modification_date";
	case fs_meta_name::name: return "name";
	case fs_meta_name::os_minimum_version: return "os_minimum_version";
	case fs_meta_name::os_version: return "os_version";
	case fs_meta_name::rsrc_length: return "rsrc_length";
	case fs_meta_name::sequential: return "sequential";
	case fs_meta_name::size_in_blocks: return "size_in_blocks";
	}
	return "";
}

std::string fs_meta::to_string(fs_meta_type type, const fs_meta &m)
{
	switch(type) {
	case fs_meta_type::string: return m.as_string();
	case fs_meta_type::number: return util::string_format("0x%x", m.as_number());
	case fs_meta_type::flag:   return m.as_flag() ? "t" : "f";
	case fs_meta_type::date:   {
		auto dt = m.as_date();
		return util::string_format("%04d-%02d-%02d %02d:%02d:%02d",
								   dt.year, dt.month, dt.day_of_month,
								   dt.hour, dt.minute, dt.second);
	}
	}
	return std::string("");
}

