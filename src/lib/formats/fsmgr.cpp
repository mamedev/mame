// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem management code for floppy and hd images

// Currently limited to floppies and creation of preformatted images

#include "emu.h"
#include "fsmgr.h"

void filesystem_manager_t::enumerate(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
}

void filesystem_manager_t::floppy_instantiate(u32 key, std::vector<u8> &image) const
{
	fprintf(stderr, "filesystem_manager_t::floppy_instantiate called while unsupported\n");
	abort();
}

void filesystem_manager_t::floppy_instantiate_raw(u32 key, floppy_image *image) const
{
	fprintf(stderr, "filesystem_manager_t::floppy_instantiate_raw called while unsupported\n");
	abort();
}

bool filesystem_manager_t::has_variant(const std::vector<uint32_t> &variants, uint32_t variant)
{
	for(uint32_t v : variants)
		if(variant == v)
			return true;
	return false;
}

void filesystem_manager_t::copy(std::vector<u8> &image, u32 offset, const u8 *src, u32 size)
{
	memcpy(image.data() + offset, src, size);
}

void filesystem_manager_t::fill(std::vector<u8> &image, u32 offset, u8 data, u32 size)
{
	memset(image.data() + offset, data, size);
}

void filesystem_manager_t::wstr(std::vector<u8> &image, u32 offset, const std::string &str)
{
	memcpy(image.data() + offset, str.data(), str.size());
}

void filesystem_manager_t::w8(std::vector<u8> &image, u32 offset, u8 data)
{
	image[offset] = data;
}

void filesystem_manager_t::w16b(std::vector<u8> &image, u32 offset, u16 data)
{
	image[offset  ] = data >> 8;
	image[offset+1] = data;
}

void filesystem_manager_t::w32b(std::vector<u8> &image, u32 offset, u32 data)
{
	image[offset  ] = data >> 24;
	image[offset+1] = data >> 16;
	image[offset+2] = data >> 8;
	image[offset+3] = data;
}

void filesystem_manager_t::w16l(std::vector<u8> &image, u32 offset, u16 data)
{
	image[offset  ] = data;
	image[offset+1] = data >> 8;
}

void filesystem_manager_t::w32l(std::vector<u8> &image, u32 offset, u32 data)
{
	image[offset  ] = data;
	image[offset+1] = data >> 8;
	image[offset+2] = data >> 16;
	image[offset+3] = data >> 24;
}
