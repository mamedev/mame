// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of Apple ProDOS floppy images

#ifndef MAME_FORMATS_FS_PRODOS_H
#define MAME_FORMATS_FS_PRODOS_H

#pragma once

#include "fsmgr.h"

class fs_prodos : public filesystem_manager_t {
public:
	fs_prodos() : filesystem_manager_t() {}

	virtual void enumerate(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual void floppy_instantiate(u32 key, std::vector<u8> &image) const override;

private:
	static const u8 boot[512];
};

extern const filesystem_manager_type FS_PRODOS;

#endif
