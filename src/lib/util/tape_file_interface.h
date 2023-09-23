// license:BSD-3-Clause
// copyright-holders:Mietek Bak

#ifndef MAME_LIB_UTIL_TAPE_FILE_INTERFACE_H
#define MAME_LIB_UTIL_TAPE_FILE_INTERFACE_H

#pragma once

#include <utility>

//////////////////////////////////////////////////////////////////////////////

enum class tape_status : u8 {
	OK, // oll korrekt
	BOM, // beginning of medium
	EW, // early warning
	FILEMARK, // filemark
	FILEMARK_EW, // filemark and early warning
	SETMARK, // setmark
	SETMARK_EW, // setmark and early warning
	EOD, // end of data
	EOD_EW, // end of data and early warning
	UNKNOWN, // unknown
	UNKNOWN_EW, // unknown and early warning
	EOM // end of medium
};

class tape_file_interface
{
public:
	// construction
	virtual ~tape_file_interface() {}

	// position-preserving operations
	virtual const bool is_read_only() const = 0;
	virtual const bool is_ew() const = 0;
	virtual const u8 get_density_code() const = 0;
	virtual const std::pair<const tape_status, const u32> read_position() const = 0;

	// non-destructive operations
	virtual void rewind(const bool eom) = 0;
	virtual const tape_status locate_block(const u32 req_block_addr) = 0;
	virtual const tape_status space_eod() = 0;
	virtual const std::pair<const tape_status, const u32> space_blocks(const u32 req_blocks_num) = 0;
	virtual const std::pair<const tape_status, const u32> space_filemarks(const u32 req_marks_num, const bool setmarks = false, const bool sequential = false) = 0;
	virtual const std::pair<const tape_status, const u32> space_blocks_reverse(const u32 req_blocks_num) = 0;
	virtual const std::pair<const tape_status, const u32> space_filemarks_reverse(const u32 req_marks_num, const bool setmarks = false, const bool sequential = false) = 0;
	virtual const std::pair<const tape_status, const u32> read_block(u8 *const buf, const u32 buf_size) = 0;

	// destructive operations
	virtual void erase(const bool eom) = 0;
	virtual const tape_status write_block(const u8 *const buf, const u32 len) = 0;
	virtual const tape_status write_filemarks(const u32 req_marks_num, const bool setmarks = false) = 0;
};

//////////////////////////////////////////////////////////////////////////////

#endif // MAME_LIB_UTIL_TAPE_FILE_INTERFACE_H
