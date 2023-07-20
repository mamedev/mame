// license:BSD-3-Clause
// copyright-holders:Mietek Bak

#ifndef MAME_LIB_UTIL_SIMH_TAPE_FILE_H
#define MAME_LIB_UTIL_SIMH_TAPE_FILE_H

#pragma once

#include "tape_file_interface.h"

#include "utilfwd.h"

#include <string>

//////////////////////////////////////////////////////////////////////////////

class simh_tape_file : public tape_file_interface
{
public:
	// construction and destruction
	simh_tape_file(util::random_read_write &file, const u64 file_size, const bool read_only, const bool create = false);
	virtual ~simh_tape_file();

	// position-preserving operations
	virtual const bool is_read_only() const override { return m_read_only; }
	virtual const bool is_ew() const override { return m_pos + 32768 >= m_file_size; } // 32KB from EOM; TODO: ANSI says EW should be 10ft from EOM regardless of density
	virtual const u8 get_density_code() const override { return 0; } // TODO: SIMH doesn't define density
	virtual const std::pair<const tape_status, const u32> read_position() const override;

	// non-destructive operations
	virtual void rewind(const bool eom) override;
	virtual const tape_status locate_block(const u32 req_block_addr) override;
	virtual const tape_status space_eod() override;
	virtual const std::pair<const tape_status, const u32> space_blocks(const u32 req_blocks_num) override;
	virtual const std::pair<const tape_status, const u32> space_blocks_reverse(const u32 req_blocks_num) override;
	virtual const std::pair<const tape_status, const u32> space_filemarks(const u32 req_marks_num, const bool setmarks = false, const bool sequential = false) override;
	virtual const std::pair<const tape_status, const u32> space_filemarks_reverse(const u32 req_marks_num, const bool setmarks = false, const bool sequential = false) override;
	virtual const std::pair<const tape_status, const u32> read_block(u8 *const buf, const u32 buf_size) override;

	// destructive operations
	virtual void erase(const bool eom) override;
	virtual const tape_status write_block(const u8 *const buf, const u32 len) override;
	virtual const tape_status write_filemarks(const u32 req_marks_num, const bool setmarks = false) override;

protected:
	// internal operations
	void raw_seek(const u64 pos) const;
	void raw_read(u8 *const buf, const u32 len) const;
	void raw_write(const u8 *const buf, const u32 len) const;
	void read_bytes(const u64 pos, u8 *const buf, const u32 len) const;
	const u32 read_word(const u64 pos) const;
	void write_bytes(const u64 pos, const u8 *const buf, const u32 len) const;
	void write_byte_repeat(const u64 pos, const u8 data, const u32 len) const;
	void write_word(const u64 pos, const u32 data) const;

	// state
	util::random_read_write &m_file; // tape image file
	u64 m_file_size; // size of tape image file
	bool m_read_only; // should we disallow destructive operations on tape image
	u64 m_pos; // tape position
};

//////////////////////////////////////////////////////////////////////////////

#endif // MAME_LIB_UTIL_SIMH_TAPE_FILE_H
