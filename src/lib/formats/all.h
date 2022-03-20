// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Enumerate all the compiled-in formats

#ifndef MAME_FORMATS_ALL_H
#define MAME_FORMATS_ALL_H

#pragma once

#include "cassimg.h"
#include "flopimg.h"
#include "fsmgr.h"

struct mame_formats_enumerator {
	virtual ~mame_formats_enumerator() = default;

	virtual void category(const char *name) = 0;
	virtual void add(const cassette_image::Format *const *formats) = 0;
	virtual void add(floppy_format_type format) = 0;
	virtual void add(const fs::manager_t &fs) = 0;
};

void mame_formats_full_list(mame_formats_enumerator &en);

#endif

