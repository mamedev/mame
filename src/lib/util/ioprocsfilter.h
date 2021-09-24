// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ioprocs.h

    I/O filters

***************************************************************************/
#ifndef MAME_LIB_UTIL_IOPROCSFILTER_H
#define MAME_LIB_UTIL_IOPROCSFILTER_H

#include "utilfwd.h"

#include <cstdlib>
#include <memory>


namespace util {

std::unique_ptr<read_stream> zlib_read(std::unique_ptr<read_stream> &&stream, std::size_t read_chunk) noexcept;
std::unique_ptr<read_stream> zlib_read(std::unique_ptr<random_read> &&stream, std::size_t read_chunk) noexcept;
std::unique_ptr<read_stream> zlib_read(read_stream &stream, std::size_t read_chunk) noexcept;
std::unique_ptr<read_stream> zlib_read(random_read &stream, std::size_t read_chunk) noexcept;

std::unique_ptr<write_stream> zlib_write(std::unique_ptr<write_stream> &&stream, int level, std::size_t buffer_size) noexcept;
std::unique_ptr<write_stream> zlib_write(write_stream &stream, int level, std::size_t buffer_size) noexcept;

} // namespace util

#endif // MAME_LIB_UTIL_IOPROCSFILTER_H
