// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem management code for floppy and hd images

// Currently limited to floppies and creation of preformatted images

#ifndef MAME_FORMATS_FSMGR_H
#define MAME_FORMATS_FSMGR_H

#pragma once

#include "flopimg.h"

class filesystem_manager_t {
public:
	struct floppy_enumerator {
		virtual ~floppy_enumerator() = default;

		virtual void add(const filesystem_manager_t *manager, floppy_format_type type, u32 image_size, const char *name, u32 key, const char *description) = 0;
		virtual void add_raw(const filesystem_manager_t *manager, const char *name, u32 key, const char *description) = 0;
	};

	virtual ~filesystem_manager_t() = default;

	virtual void enumerate(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const;

	// Floppy image initialization
	virtual void floppy_instantiate(u32 key, std::vector<u8> &image) const;

	// Floppy image initialization for add_raw
	virtual void floppy_instantiate_raw(u32 key, floppy_image *image) const;
	
protected:
	filesystem_manager_t() = default;

	static bool has_variant(const std::vector<uint32_t> &variants, uint32_t variant);

	static void copy(std::vector<u8> &image, u32 offset, const u8 *src, u32 size);
	static void fill(std::vector<u8> &image, u32 offset, u8 data, u32 size);
	static void wstr(std::vector<u8> &image, u32 offset, const std::string &str);
	static void w8(std::vector<u8> &image, u32 offset, u8 data);
	static void w16b(std::vector<u8> &image, u32 offset, u16 data);
	static void w32b(std::vector<u8> &image, u32 offset, u32 data);
	static void w16l(std::vector<u8> &image, u32 offset, u16 data);
	static void w32l(std::vector<u8> &image, u32 offset, u32 data);
};


typedef filesystem_manager_t *(*filesystem_manager_type)();

// this template function creates a stub which constructs a filesystem manager
template<class _FormatClass>
filesystem_manager_t *filesystem_manager_creator()
{
	return new _FormatClass();
}

#endif
