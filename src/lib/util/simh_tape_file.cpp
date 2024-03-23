// license:BSD-3-Clause
// copyright-holders:Mietek Bak

// best read together with SIMH magtape spec (rev 17 Jan 2022)
// http://simh.trailing-edge.com/docs/simh_magtape.pdf

#include "simh_tape_file.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <fcntl.h>

//////////////////////////////////////////////////////////////////////////////

// constants and helpers

enum class simh_marker : osd::u32 {
	TAPE_MARK   = 0x00000000, // filemark; TODO: SIMH doesn't define setmarks
	ERASE_GAP   = 0xfffffffe,
	EOM         = 0xffffffff
};

inline bool is_simh_marker_half_gap_forward(const simh_marker marker)
{
	// this function is used when we're reading normally (from BOM to EOM); returns true for erase gap markers that have been half overwritten
	return osd::u32(marker) == 0xfffeffff;
}

inline bool is_simh_marker_half_gap_reverse(const simh_marker marker)
{
	// this function is used when we're reading in reverse (from EOM to BOM); returns true for erase gap markers that have been half overwritten
	return osd::u32(marker) >= 0xffff0000 && osd::u32(marker) <= 0xfffffffd;
}

inline bool is_simh_marker_eod_forward(const simh_marker marker)
{
	// this function is used when we're reading normally (from BOM to EOM); returns true for markers that we consider EOD
	return marker == simh_marker::ERASE_GAP
		|| is_simh_marker_half_gap_forward(marker)
		|| marker == simh_marker::EOM; // logical EOM
}

inline bool is_simh_marker_eod_reverse(const simh_marker marker)
{
	// this function is used when we're reading in reverse (from EOM to BOM); returns true for markers that we consider EOD
	return marker == simh_marker::ERASE_GAP
		|| is_simh_marker_half_gap_reverse(marker)
		|| marker == simh_marker::EOM; // logical EOM
}

enum class simh_marker_class : osd::u8 {
	GOOD_DATA_RECORD                = 0x0,
	PRIVATE_DATA_RECORD_1           = 0x1,
	PRIVATE_DATA_RECORD_2           = 0x2,
	PRIVATE_DATA_RECORD_3           = 0x3,
	PRIVATE_DATA_RECORD_4           = 0x4,
	PRIVATE_DATA_RECORD_5           = 0x5,
	PRIVATE_DATA_RECORD_6           = 0x6,
	PRIVATE_MARKER                  = 0x7,
	BAD_DATA_RECORD                 = 0x8,
	RESERVED_DATA_RECORD_9          = 0x9,
	RESERVED_DATA_RECORD_A          = 0xa,
	RESERVED_DATA_RECORD_B          = 0xb,
	RESERVED_DATA_RECORD_C          = 0xc,
	RESERVED_DATA_RECORD_D          = 0xd,
	TAPE_DESCRIPTION_DATA_RECORD    = 0xe,
	RESERVED_MARKER                 = 0xf
};

inline simh_marker_class get_simh_marker_class(const simh_marker marker)
{
	return simh_marker_class(osd::u32(marker) >> 28);
}

inline osd::u32 get_simh_marker_value(const simh_marker marker)
{
	return osd::u32(marker) & 0x0fffffff;
}

//////////////////////////////////////////////////////////////////////////////

// construction

simh_tape_file::simh_tape_file(util::random_read_write &file, osd::u64 file_size, bool read_only, bool create)
	: m_file(file)
	, m_file_size(file_size)
	, m_read_only(read_only)
	, m_pos(0)
{
	if (create) {
		if (read_only) // error: we cannot create read-only file
			throw std::runtime_error("read-only file");

		write_byte_repeat(0, 0xff, m_file_size); // we assume simh_marker::EOM == 0xffffffff
	}
}

simh_tape_file::~simh_tape_file()
{
	m_file.finalize();
}

//////////////////////////////////////////////////////////////////////////////

// internal operations

void simh_tape_file::raw_seek(const osd::u64 pos) const
{
	std::error_condition err = m_file.seek(pos, SEEK_SET);
	if (err) // error: we failed to seek to expected byte offset
		throw std::runtime_error(std::string("failed seek: ") + err.message());
}

void simh_tape_file::raw_read(osd::u8 *const buf, const osd::u32 len) const
{
	auto const [err, actual_len] = read(m_file, buf, len);
	if (err || actual_len != len) // error: we failed to read expected number of bytes
		throw std::runtime_error(std::string("failed read: ") + (err ? err.message() : std::string("unexpected length")));
}

void simh_tape_file::raw_write(const osd::u8 *const buf, const osd::u32 len) const
{
	auto const [err, actual_len] = write(m_file, buf, len);
	if (err || actual_len != len) // error: we failed to write expected number of bytes
		throw std::runtime_error(std::string("failed write: ") + (err ? err.message() : std::string("unexpected length")));
}

void simh_tape_file::read_bytes(const osd::u64 pos, osd::u8 *const buf, const osd::u32 len) const
{
	raw_seek(pos);
	raw_read(buf, len);
}

osd::u32 simh_tape_file::read_word(const osd::u64 pos) const
{
	const osd::u32 tmp_len = 4;
	osd::u8 tmp_buf[tmp_len];
	raw_seek(pos);
	raw_read(tmp_buf, tmp_len);
	return get_u32le(tmp_buf);
}

void simh_tape_file::write_bytes(const osd::u64 pos, const osd::u8 *const buf, const osd::u32 len) const
{
	raw_seek(pos);
	raw_write(buf, len);
}

void simh_tape_file::write_byte_repeat(const osd::u64 pos, const osd::u8 data, const osd::u32 len) const
{
	const osd::u32 tmp_len = 4096;
	osd::u8 tmp_buf[tmp_len];
	memset(tmp_buf, data, std::min(len, tmp_len));
	raw_seek(pos);
	for (osd::u32 i = 0; i < len / tmp_len; i++)
		raw_write(tmp_buf, tmp_len);
	raw_write(tmp_buf, len % tmp_len);
}

void simh_tape_file::write_word(const osd::u64 pos, const osd::u32 data) const
{
	const osd::u32 tmp_len = 4;
	osd::u8 tmp_buf[tmp_len];
	put_u32le(tmp_buf, data);
	raw_seek(pos);
	raw_write(tmp_buf, tmp_len);
}

//////////////////////////////////////////////////////////////////////////////

// position-preserving operations

std::pair<tape_status, osd::u32> simh_tape_file::read_position() const
{
	// this module only keeps track of current tape position, therefore this function scans from BOM to find and return current block address, taking linear time; TODO: this module could be rewritten to also keep track of current block address, therefore enabling this function to only take constant time
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	if (m_pos == 0) // we're at BOM and next block is 0
		return std::pair(tape_status::BOM, 0);

	if (m_pos == m_file_size) // we're at physical EOM and there is no next block
		return std::pair(tape_status::EOM, 0);

	// we need to count how many blocks are between BOM and us
	osd::u32 blocks_num = 0;
	osd::u64 tmp_pos = 0;
	while (tmp_pos < m_pos) {
		if (tmp_pos + 4 > m_file_size) // error: truncated marker
			throw std::runtime_error("truncated marker");

		const simh_marker marker = simh_marker(read_word(tmp_pos));
		if (marker == simh_marker::TAPE_MARK) { // we skip filemarks
			tmp_pos += 4;
			continue;
		}
		if (is_simh_marker_eod_forward(marker)) // error: we reached EOD
			return std::pair(is_ew() ? tape_status::UNKNOWN_EW : tape_status::UNKNOWN, 0);

		const simh_marker_class marker_class = get_simh_marker_class(marker);
		const osd::u32 block_len = get_simh_marker_value(marker);
		const osd::u32 pad_len = block_len % 2; // pad odd-length blocks with 1 byte
		const osd::u32 read_len = 4 + block_len + pad_len + 4;
		switch (marker_class) {
			case simh_marker_class::PRIVATE_MARKER: // we skip other markers
			case simh_marker_class::RESERVED_MARKER:
				tmp_pos += 4;
				break;
			case simh_marker_class::GOOD_DATA_RECORD: // we try to count data blocks
				if (tmp_pos + read_len > m_file_size) // error: truncated block
					throw std::runtime_error("truncated block");

				// we counted another block
				tmp_pos += read_len;
				blocks_num++;
				break;
			default: // we try to skip other blocks
				if (tmp_pos + read_len > m_file_size) // error: truncated block
					throw std::runtime_error("truncated block");

				tmp_pos += read_len;
		}
	}
	if (m_pos < tmp_pos) // we're inside some block
		return std::pair(is_ew() ? tape_status::UNKNOWN_EW : tape_status::UNKNOWN, 0);

	// we're right after some block (at EOD, or at filemark, or at another block)
	assert(tmp_pos == m_pos);
	return std::pair(is_ew() ? tape_status::EW : tape_status::OK, blocks_num);
}

//////////////////////////////////////////////////////////////////////////////

// non-destructive operations

void simh_tape_file::rewind(bool eom)
{
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	m_pos = eom ? m_file_size : 0;
}

tape_status simh_tape_file::locate_block(osd::u32 req_block_addr)
{
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	osd::u32 blocks_num = 0;
	m_pos = 0;
	while (m_pos < m_file_size) {
		if (m_pos + 4 > m_file_size) // error: truncated marker
			throw std::runtime_error("truncated marker");

		const simh_marker marker = simh_marker(read_word(m_pos));
		if (marker == simh_marker::TAPE_MARK) { // we skip filemarks
			m_pos += 4;
			continue;
		}
		if (is_simh_marker_eod_forward(marker)) // error: we reached EOD
			return is_ew() ? tape_status::EOD_EW : tape_status::EOD;

		const simh_marker_class marker_class = get_simh_marker_class(marker);
		const osd::u32 block_len = get_simh_marker_value(marker);
		const osd::u32 pad_len = block_len % 2; // pad odd-length blocks with 1 byte
		const osd::u32 read_len = 4 + block_len + pad_len + 4;
		switch (marker_class) {
			case simh_marker_class::PRIVATE_MARKER: // we skip other markers
			case simh_marker_class::RESERVED_MARKER:
				m_pos += 4;
				break;
			case simh_marker_class::GOOD_DATA_RECORD: // we try to count data blocks
				if (m_pos + read_len > m_file_size) // error: truncated block
					throw std::runtime_error("truncated block");

				if (blocks_num == req_block_addr) // success: we located requested block
					return tape_status::OK;

				m_pos += read_len;
				blocks_num++;
				break;
			default: // we try to skip other blocks
				if (m_pos + read_len > m_file_size) // error: truncated block
					throw std::runtime_error("truncated block");

				m_pos += read_len;
		}
	}
	// error: we reached physical EOM
	assert(m_pos == m_file_size);
	return tape_status::EOM;
}

tape_status simh_tape_file::space_eod()
{
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	while (m_pos < m_file_size) {
		if (m_pos + 4 > m_file_size) // error: truncated marker
			throw std::runtime_error("truncated marker");

		const simh_marker marker = simh_marker(read_word(m_pos));
		if (marker == simh_marker::TAPE_MARK) { // we skip filemarks
			m_pos += 4;
			continue;
		}
		if (is_simh_marker_eod_forward(marker)) // success: we reached EOD
			return tape_status::OK;

		const simh_marker_class marker_class = get_simh_marker_class(marker);
		const osd::u32 block_len = get_simh_marker_value(marker);
		const osd::u32 pad_len = block_len % 2; // pad odd-length blocks with 1 byte
		const osd::u32 read_len = 4 + block_len + pad_len + 4;
		switch (marker_class) {
			case simh_marker_class::PRIVATE_MARKER: // we skip other markers
			case simh_marker_class::RESERVED_MARKER:
				m_pos += 4;
				break;
			case simh_marker_class::GOOD_DATA_RECORD: // we try to skip all blocks
			default:
				if (m_pos + read_len > m_file_size) // error: truncated block
					throw std::runtime_error("truncated block");

				m_pos += read_len;
		}
	}
	// error: we reached physical EOM
	assert(m_pos == m_file_size);
	return tape_status::EOM;
}

std::pair<tape_status, osd::u32> simh_tape_file::space_blocks(osd::u32 req_blocks_num)
{
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	assert(req_blocks_num > 0);
	osd::u32 blocks_num = 0;
	while (m_pos < m_file_size) {
		if (m_pos + 4 > m_file_size) // error: truncated marker
			throw std::runtime_error("truncated marker");

		const simh_marker marker = simh_marker(read_word(m_pos));
		if (marker == simh_marker::TAPE_MARK) { // error: we reached filemark
			m_pos += 4;
			return std::pair(is_ew() ? tape_status::FILEMARK_EW : tape_status::FILEMARK, blocks_num);
		}
		if (is_simh_marker_eod_forward(marker)) // error: we reached EOD
			return std::pair(is_ew() ? tape_status::EOD_EW : tape_status::EOD, blocks_num);

		const simh_marker_class marker_class = get_simh_marker_class(marker);
		const osd::u32 block_len = get_simh_marker_value(marker);
		const osd::u32 pad_len = block_len % 2; // pad odd-length blocks with 1 byte
		const osd::u32 read_len = 4 + block_len + pad_len + 4;
		switch (marker_class) {
			case simh_marker_class::PRIVATE_MARKER: // we skip other markers
			case simh_marker_class::RESERVED_MARKER:
				m_pos += 4;
				break;
			case simh_marker_class::GOOD_DATA_RECORD: // we try to count data blocks
				if (m_pos + read_len > m_file_size) // error: truncated block
					throw std::runtime_error("truncated block");

				m_pos += read_len;
				blocks_num++;
				if (blocks_num == req_blocks_num) // success: we're done
					return std::pair(tape_status::OK, blocks_num);

				break;
			default: // we try to skip other blocks
				if (m_pos + read_len > m_file_size) // error: truncated block
					throw std::runtime_error("truncated block");

				m_pos += read_len;
		}
	}
	// error: we reached physical EOM
	assert(m_pos == m_file_size);
	return std::pair(tape_status::EOM, blocks_num);
}

std::pair<tape_status, osd::u32> simh_tape_file::space_filemarks(osd::u32 req_filemarks_num, bool setmarks, bool sequential)
{
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	assert(req_filemarks_num > 0);
	assert(!setmarks); // TODO: SIMH doesn't define setmarks
	assert(!sequential); // TODO: support spacing over sequential filemarks, once we have good way to test it
	osd::u32 filemarks_num = 0;
	while (m_pos < m_file_size) {
		if (m_pos + 4 > m_file_size) // error: truncated marker
			throw std::runtime_error("truncated marker");

		const simh_marker marker = simh_marker(read_word(m_pos));
		if (marker == simh_marker::TAPE_MARK) { // we count filemarks
			m_pos += 4;
			filemarks_num++;
			if (filemarks_num == req_filemarks_num) // success: we're done
				return std::pair(tape_status::OK, filemarks_num);

			continue;
		}
		if (is_simh_marker_eod_forward(marker)) // error: we reached EOD
			return std::pair(is_ew() ? tape_status::EOD_EW : tape_status::EOD, filemarks_num);

		const simh_marker_class marker_class = get_simh_marker_class(marker);
		const osd::u32 block_len = get_simh_marker_value(marker);
		const osd::u32 pad_len = block_len % 2; // pad odd-length blocks with 1 byte
		const osd::u32 read_len = 4 + block_len + pad_len + 4;
		switch (marker_class) {
			case simh_marker_class::PRIVATE_MARKER: // we skip other markers
			case simh_marker_class::RESERVED_MARKER:
				m_pos += 4;
				break;
			case simh_marker_class::GOOD_DATA_RECORD: // we try to skip all blocks
			default:
				if (m_pos + read_len > m_file_size) // error: truncated block
					throw std::runtime_error("truncated block");

				m_pos += read_len;
		}
	}
	// error: we reached physical EOM
	assert(m_pos == m_file_size);
	return std::pair(tape_status::EOM, filemarks_num);
}

std::pair<tape_status, osd::u32> simh_tape_file::space_blocks_reverse(osd::u32 req_blocks_num)
{
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	assert(req_blocks_num > 0);
	osd::u32 blocks_num = 0;
	while (m_pos > 0) {
		if (m_pos - 4 < 0) // error: truncated marker
			throw std::runtime_error("truncated marker");

		const simh_marker marker = simh_marker(read_word(m_pos - 4));
		if (marker == simh_marker::TAPE_MARK) { // error: we reached filemark
			m_pos -= 4;
			return std::pair(is_ew() ? tape_status::FILEMARK_EW : tape_status::FILEMARK, blocks_num);
		}
		if (is_simh_marker_eod_reverse(marker)) // error: we reached EOD
			return std::pair(is_ew() ? tape_status::EOD_EW : tape_status::EOD, blocks_num);

		const simh_marker_class marker_class = get_simh_marker_class(marker);
		const osd::u32 block_len = get_simh_marker_value(marker);
		const osd::u32 pad_len = block_len % 2; // pad odd-length blocks with 1 byte
		const osd::u32 read_len = 4 + block_len + pad_len + 4;
		switch (marker_class) {
			case simh_marker_class::PRIVATE_MARKER: // we skip other markers
			case simh_marker_class::RESERVED_MARKER:
				m_pos -= 4;
				break;
			case simh_marker_class::GOOD_DATA_RECORD: // we try to count data blocks
				if (m_pos - read_len < 0) // error: truncated block
					throw std::runtime_error("truncated block");

				m_pos -= read_len;
				blocks_num++;
				if (blocks_num == req_blocks_num) // success: we're done
					return std::pair(tape_status::OK, blocks_num);

				break;
			default: // we try to skip other blocks
				if (m_pos - read_len < 0) // error: truncated block
					throw std::runtime_error("truncated block");

				m_pos -= read_len;
		}
	}
	// error: we reached BOM
	assert(m_pos == 0);
	return std::pair(tape_status::BOM, blocks_num);
}

std::pair<tape_status, osd::u32> simh_tape_file::space_filemarks_reverse(osd::u32 req_filemarks_num, bool setmarks, bool sequential)
{
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	assert(req_filemarks_num > 0);
	assert(!setmarks); // TODO: SIMH doesn't define setmarks
	assert(!sequential); // TODO: support spacing over sequential filemarks, once we have good way to test it
	osd::u32 filemarks_num = 0;
	while (m_pos > 0) {
		if (m_pos - 4 < 0) // error: truncated marker
			throw std::runtime_error("truncated marker");

		const simh_marker marker = simh_marker(read_word(m_pos - 4));
		if (marker == simh_marker::TAPE_MARK) { // we count filemarks
			m_pos -= 4;
			filemarks_num++;
			if (filemarks_num == req_filemarks_num) // success: we're done
				return std::pair(tape_status::OK, filemarks_num);

			continue;
		}
		if (is_simh_marker_eod_reverse(marker)) // error: we reached EOD
			return std::pair(is_ew() ? tape_status::EOD_EW : tape_status::EOD, filemarks_num);

		const simh_marker_class marker_class = get_simh_marker_class(marker);
		const osd::u32 block_len = get_simh_marker_value(marker);
		const osd::u32 pad_len = block_len % 2; // pad odd-length blocks with 1 byte
		const osd::u32 read_len = 4 + block_len + pad_len + 4;
		switch (marker_class) {
			case simh_marker_class::PRIVATE_MARKER: // we skip other markers
			case simh_marker_class::RESERVED_MARKER:
				m_pos -= 4;
				break;
			case simh_marker_class::GOOD_DATA_RECORD: // we try to skip all blocks
			default:
				if (m_pos - read_len < 0) // error: truncated block
					throw std::runtime_error("truncated block");

				m_pos -= read_len;
		}
	}
	// error: we reached BOM
	assert(m_pos == 0);
	return std::pair(tape_status::BOM, filemarks_num);
}

std::pair<tape_status, osd::u32> simh_tape_file::read_block(osd::u8 *buf, osd::u32 buf_size)
{
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	while (m_pos < m_file_size) {
		if (m_pos + 4 > m_file_size) // error: truncated marker
			throw std::runtime_error("truncated marker");

		const simh_marker marker = simh_marker(read_word(m_pos));
		if (marker == simh_marker::TAPE_MARK) { // error: we reached filemark
			m_pos += 4;
			return std::pair(is_ew() ? tape_status::FILEMARK_EW : tape_status::FILEMARK, 0);
		}
		if (is_simh_marker_eod_forward(marker)) // error: we reached EOD
			return std::pair(is_ew() ? tape_status::EOD_EW : tape_status::EOD, 0);

		const simh_marker_class marker_class = get_simh_marker_class(marker);
		const osd::u32 block_len = get_simh_marker_value(marker);
		const osd::u32 pad_len = block_len % 2; // pad odd-length blocks with 1 byte
		const osd::u32 read_len = 4 + block_len + pad_len + 4;
		switch (marker_class) {
			case simh_marker_class::PRIVATE_MARKER: // we skip other markers
			case simh_marker_class::RESERVED_MARKER:
				m_pos += 4;
				break;
			case simh_marker_class::GOOD_DATA_RECORD: // we try to read data blocks
				if (m_pos + read_len > m_file_size) // error: truncated block
					throw std::runtime_error("truncated block");

				if (block_len > buf_size) // error: block length too big
					throw std::runtime_error("block length too big");

				// success: we read another block
				read_bytes(m_pos + 4, buf, block_len);
				m_pos += read_len;
				return std::pair(tape_status::OK, block_len);

			default: // we try to skip other blocks
				if (m_pos + read_len > m_file_size) // error: truncated block
					throw std::runtime_error("truncated block");

				m_pos += read_len;
		}
	}
	// error: we reached physical EOM
	assert(m_pos == m_file_size);
	return std::pair(tape_status::EOM, 0);
}

//////////////////////////////////////////////////////////////////////////////

// destructive operations

void simh_tape_file::erase(bool eom)
{
	assert(!m_read_only);
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	const osd::u32 write_len = m_file_size - m_pos; // we always erase entire remainder of tape
	write_byte_repeat(m_pos, 0xff, write_len); // we assume simh_marker::EOM == 0xffffffff
	m_pos += write_len;
}

tape_status simh_tape_file::write_block(const osd::u8 *buf, osd::u32 req_block_len)
{
	assert(!m_read_only);
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	const osd::u32 pad_len = req_block_len % 2; // pad odd-length blocks with 1 byte
	const osd::u32 write_len = 4 + req_block_len + pad_len + 4;
	if (m_pos + write_len >= m_file_size) // error: we reached physical EOM
		return tape_status::EOM;

	write_word(m_pos, req_block_len);
	write_bytes(m_pos + 4, buf, req_block_len);
	write_byte_repeat(m_pos + 4 + req_block_len, 0, pad_len);
	write_word(m_pos + 4 + req_block_len + pad_len, req_block_len);
	m_pos += write_len;
	return is_ew() ? tape_status::EW : tape_status::OK; // success: we wrote another block
}

tape_status simh_tape_file::write_filemarks(osd::u32 req_filemarks_num, bool setmarks)
{
	assert(!m_read_only);
	assert(m_pos <= m_file_size);
	assert(m_pos % 2 == 0);
	assert(!setmarks); // TODO: SIMH doesn't define setmarks
	const osd::u32 write_len = req_filemarks_num * 4;
	if (m_pos + write_len >= m_file_size) // error: we reached physical EOM
		return tape_status::EOM;

	for (osd::u32 i = 0; i < write_len; i += 4)
		write_word(m_pos + i, osd::u32(simh_marker::TAPE_MARK));
	m_pos += write_len;
	return is_ew() ? tape_status::EW : tape_status::OK; // success: we wrote all filemarks
}

//////////////////////////////////////////////////////////////////////////////
