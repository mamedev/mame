// license:BSD-3-Clause
// copyright-holders:Mietek Bak
#ifndef MAME_UTIL_TAPE_FILE_INTERFACE_H
#define MAME_UTIL_TAPE_FILE_INTERFACE_H

#pragma once

#include <cstdint>
#include <utility>

//////////////////////////////////////////////////////////////////////////////

enum class tape_status : std::uint8_t {
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
	virtual bool is_read_only() const = 0;
	virtual bool is_ew() const = 0;
	virtual std::uint8_t get_density_code() const = 0;
	virtual std::pair<tape_status, std::uint32_t> read_position() const = 0;

	// non-destructive operations
	virtual void rewind(bool eom) = 0;
	virtual tape_status locate_block(std::uint32_t req_block_addr) = 0;
	virtual tape_status space_eod() = 0;
	virtual std::pair<tape_status, std::uint32_t> space_blocks(std::uint32_t req_blocks_num) = 0;
	virtual std::pair<tape_status, std::uint32_t> space_filemarks(std::uint32_t req_marks_num, bool setmarks = false, bool sequential = false) = 0;
	virtual std::pair<tape_status, std::uint32_t> space_blocks_reverse(std::uint32_t req_blocks_num) = 0;
	virtual std::pair<tape_status, std::uint32_t> space_filemarks_reverse(std::uint32_t req_marks_num, bool setmarks = false, bool sequential = false) = 0;
	virtual std::pair<tape_status, std::uint32_t> read_block(std::uint8_t *buf, std::uint32_t buf_size) = 0;

	// destructive operations
	virtual void erase(bool eom) = 0;
	virtual tape_status write_block(const std::uint8_t *buf, std::uint32_t len) = 0;
	virtual tape_status write_filemarks(std::uint32_t req_marks_num, bool setmarks = false) = 0;
};

//////////////////////////////////////////////////////////////////////////////

#endif // MAME_UTIL_TAPE_FILE_INTERFACE_H
