// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ioprocs.h

    I/O interfaces

***************************************************************************/
#ifndef MAME_LIB_UTIL_IOPROCS_H
#define MAME_LIB_UTIL_IOPROCS_H

#pragma once

#include "utilfwd.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <system_error>
#include <tuple>
#include <utility>


// FIXME: make a proper place for OSD forward declarations
class osd_file;


namespace util {

/// \defgroup ioprocs Generic I/O interfaces
/// \{

/// \brief Interface to an input stream
///
/// Represents a stream producing a sequence of bytes with no further
/// structure.
/// \sa write_stream read_write_stream random_read
class read_stream
{
public:
	using ptr = std::unique_ptr<read_stream>;

	virtual ~read_stream() = default;

	/// \brief Read from the current position in the stream
	///
	/// Reads up to the specified number of bytes from the stream into
	/// the supplied buffer.  May read less than the requested number of
	/// bytes if the end of the stream is reached or an error occurs.
	/// If the stream supports seeking, reading starts at the current
	/// position in the stream, and the current position is incremented
	/// by the number of bytes read.
	/// \param [out] buffer Destination buffer.  Must be large enough to
	///   hold the requested number of bytes.
	/// \param [in] length Maximum number of bytes to read.
	/// \param [out] actual Number of bytes actually read.  Will always
	///   be less than or equal to the requested length.
	/// \return An error condition if reading stopped due to an error.
	virtual std::error_condition read_some(void *buffer, std::size_t length, std::size_t &actual) noexcept = 0;
};


/// \brief Interface to an output stream
///
/// Represents a stream that accepts a sequence of bytes with no further
/// structure.
/// \sa read_stream read_write_stream random_write
class write_stream
{
public:
	using ptr = std::unique_ptr<write_stream>;

	virtual ~write_stream() = default;

	/// \brief Finish writing data
	///
	/// Performs any tasks necessary to finish writing data to the
	/// stream and guarantee that the written data can be read in its
	/// entirety.  Further writes may not be possible.
	/// \return An error condition if the operation fails.
	virtual std::error_condition finalize() noexcept = 0;

	/// \brief Flush application-side caches
	///
	/// Flushes any caches to the underlying stream.  Success does not
	/// guarantee that data has reached persistent storage.
	/// \return An error condition if flushing caches fails.
	virtual std::error_condition flush() noexcept = 0;

	/// \brief Write at the current position in the stream
	///
	/// Writes up to the specified number of bytes from the supplied
	/// buffer to the stream.  May write less than the requested number
	/// of bytes if an error occurs.  If the stream supports seeking,
	/// writing starts at the current position in the stream, and the
	/// current position is incremented by the number of bytes written.
	/// \param [in] buffer Buffer containing the data to write.  Must
	///   contain at least the specified number of bytes.
	/// \param [in] length Number of bytes to write.
	/// \param [out] actual Number of bytes actually written.  Will
	///   always be less than or equal to the requested length.
	/// \return An error condition if writing stopped due to an error.
	virtual std::error_condition write_some(void const *buffer, std::size_t length, std::size_t &actual) noexcept = 0;
};


/// \brief Interface to an I/O stream
///
/// Represents an object that acts as both a source of and sink for byte
/// sequences.
/// \sa read_stream write_stream random_read_write
class read_write_stream : public virtual read_stream, public virtual write_stream
{
public:
	using ptr = std::unique_ptr<read_write_stream>;
};


/// \brief Interface to a byte sequence that supports seeking
///
/// Provides an interface for controlling the position within a byte
/// sequence that supports seeking.
/// \sa random_read random_write random_read_write
class random_access
{
public:
	virtual ~random_access() = default;

	/// \brief Set the position in the stream
	///
	/// Sets the position for the next read or write operation.  It may
	/// be possible to set the position beyond the end of the stream.
	/// \param [in] offset The offset in bytes, relative to the position
	///   specified by the whence parameter.
	/// \param [in] whence One of SEEK_SET, SEEK_CUR or SEEK_END, to
	///   interpret the offset parameter relative to the beginning of
	///   the stream, the current position in the stream, or the end of
	///   the stream, respectively.
	/// \return An error condition of the operation failed.
	virtual std::error_condition seek(std::int64_t offset, int whence) noexcept = 0;

	/// \brief Get the current position in the stream
	///
	/// Gets the position in the stream for the next read or write
	/// operation.  The position may be beyond the end of the stream.
	/// \param [out] result The position in bytes relative to the
	///   beginning of the stream.  Not valid if the operation fails.
	/// \return An error condition if the operation failed.
	virtual std::error_condition tell(std::uint64_t &result) noexcept = 0;

	/// \brief Get the length of the stream
	///
	/// Gets the current length of the stream.
	/// \param [out] result The length of the stream in bytes.  Not
	///   valid if the operation fails.
	/// \return An error condtion if the operation failed.
	virtual std::error_condition length(std::uint64_t &result) noexcept = 0;
};


/// \brief Interface to a random-access byte input sequence
///
/// Provides an interface for reading from arbitrary positions within a
/// byte sequence.  No further structure is provided.
/// \sa read_stream random_write random_read_write
class random_read : public virtual read_stream, public virtual random_access
{
public:
	using ptr = std::unique_ptr<random_read>;

	/// \brief Read from specified position
	///
	/// Reads up to the specified number of bytes into the supplied
	/// buffer.  If seeking is supported, reading starts at the
	/// specified position and the current position is unaffected.  May
	/// read less than the requested number of bytes if the end of the
	/// stream is encountered or an error occurs.
	/// \param [in] offset The position to start reading from, specified
	///   as a number of bytes from the beginning of the stream.
	/// \param [out] buffer Destination buffer.  Must be large enough to
	///   hold the requested number of bytes.
	/// \param [in] length Maximum number of bytes to read.
	/// \param [out] actual Number of bytes actually read.  Will always
	///   be less than or equal to the requested length.
	/// \return An error condition if seeking failed or reading stopped
	///   due to an error.
	virtual std::error_condition read_some_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept = 0;
};


/// \brief Interface to a random-access byte output sequence
///
/// Provides an interface for writing to arbitrary positions within a
/// byte sequence.  No further structure is provided.
/// \sa write_stream random_read random_read_write
class random_write : public virtual write_stream, public virtual random_access
{
public:
	using ptr = std::unique_ptr<random_write>;

	/// \brief Write at specified position
	///
	/// Writes up to the specified number of bytes from the supplied
	/// buffer.  If seeking is supported, writing starts at the
	/// specified position and the current position is unaffected.   May
	/// write less than the requested number of bytes if an error
	/// occurs.
	/// \param [in] offset The position to start writing at, specified
	///   as a number of bytes from the beginning of the stream.
	/// \param [in] buffer Buffer containing the data to write.  Must
	///   contain at least the specified number of bytes.
	/// \param [in] length Number of bytes to write.
	/// \param [out] actual Number of bytes actually written.  Will
	///   always be less than or equal to the requested length.
	/// \return An error condition if seeking failed or writing stopped
	///   due to an error.
	virtual std::error_condition write_some_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept = 0;
};


/// \brief Interface to a random-access read/write byte sequence
///
/// Provides an interface for reading from and writing to arbitrary
/// positions within a byte sequence.  No further structure is provided.
/// \sa random_read random_write read_write_stream
class random_read_write : public read_write_stream, public virtual random_read, public virtual random_write
{
public:
	using ptr = std::unique_ptr<random_read_write>;
};


/// \brief Read from the current position in the stream
///
/// Reads up to the specified number of bytes from the stream into the
/// supplied buffer, continuing if interrupted by asynchronous signals.
/// May read less than the requested number of bytes if the end of the
/// stream is reached or an error occurs.  If the stream supports
/// seeking, reading starts at the current position in the stream, and
/// the current position is incremented by the number of bytes read.
/// The operation may not be atomic if it is interrupted before the
/// requested number of bytes is read.
/// \param [in] stream The stream to read from.
/// \param [out] buffer Destination buffer.  Must be large enough to
///   hold the requested number of bytes.
/// \param [in] length Maximum number of bytes to read.
/// \return A pair containing an error condition if reading stopped due
///   to an error, and the actual number of bytes read.
std::pair<std::error_condition, std::size_t> read(read_stream &stream, void *buffer, std::size_t length) noexcept;

/// \brief Allocate memory and read from the current position in the
///   stream
///
/// Allocates the specified number of bytes and then reads up to that
/// number of bytes from the stream into the newly allocated buffer,
/// continuing if interrupted by asynchronous signals.  May read less
/// than the requested number of bytes if the end of the stream is
/// reached or an error occurs.  If the stream supports seeking,
/// reading starts at the current position in the stream, and the
/// current position is incremented by the number of bytes read.  The
/// operation may not be atomic if it is interrupted before the
/// requested number of bytes is read.  No data will be read if
/// allocation fails.
/// \param [in] stream The stream to read from.
///   hold the requested number of bytes.
/// \param [in] length Maximum number of bytes to read.
/// \return A tuple containing an error condition if allocation failed
///   or reading stopped due to an error, the allocated buffer, and the
///   actual number of bytes read.
std::tuple<std::error_condition, std::unique_ptr<std::uint8_t []>, std::size_t> read(read_stream &stream, std::size_t length) noexcept;

/// \brief Read from the specified position
///
/// Reads up to the specified number of bytes from the stream into the
/// supplied buffer, continuing if interrupted by asynchronous signals.
/// May read less than the requested number of bytes if the end of the
/// stream is reached or an error occurs.  If seeking is supported,
/// reading starts at the specified position and the current position is
/// unaffected.  The operation may not be atomic if it is interrupted
/// before the requested number of bytes is read.
/// \param [in] stream The stream to read from.
/// \param [in] offset The position to start reading from, specified as
///   a number of bytes from the beginning of the stream.
/// \param [out] buffer Destination buffer.  Must be large enough to
///   hold the requested number of bytes.
/// \param [in] length Maximum number of bytes to read.
/// \return A pair containing an error condition if reading stopped due
///   to an error, and the actual number of bytes read.
std::pair<std::error_condition, std::size_t> read_at(random_read &stream, std::uint64_t offset, void *buffer, std::size_t length) noexcept;

/// \brief Allocate memory and read from the specified position
///
/// Allocates the specified number of bytes and then reads up to that
/// number of bytes from the stream into the newly allocated buffer,
/// continuing if interrupted by asynchronous signals.  May read less
/// than the requested number of bytes if the end of the stream is
/// reached or an error occurs.  If seeking is supported, reading
/// starts at the specified position and the current position is
/// unaffected.  The operation may not be atomic if it is interrupted
/// before the requested number of bytes is read.  No data will be read
/// if allocation fails.
/// \param [in] stream The stream to read from.
/// \param [in] offset The position to start reading from, specified as
///   a number of bytes from the beginning of the stream.
/// \param [out] buffer Destination buffer.  Must be large enough to
///   hold the requested number of bytes.
/// \param [in] length Maximum number of bytes to read.
/// \return A tuple containing an error condition if allocation failed
///   or reading stopped due to an error, the allocated buffer, and the
///   actual number of bytes read.
std::tuple<std::error_condition, std::unique_ptr<std::uint8_t []>, std::size_t> read_at(random_read &stream, std::uint64_t offset, std::size_t length) noexcept;

/// \brief Write at the current position in the stream
///
/// Writes up to the specified number of bytes from the supplied
/// buffer to the stream, continuing if interrupted by asynchronous
/// signals.  May write less than the requested number of bytes if an
/// error occurs.  If the stream supports seeking, writing starts at the
/// current position in the stream, and the current position is
/// incremented by the number of bytes written.  The operation may not
/// be atomic if it is interrupted before the requested number of bytes
/// is written.
/// \param [in] stream The stream to write to.
/// \param [in] buffer Buffer containing the data to write.  Must
///   contain at least the specified number of bytes.
/// \param [in] length Number of bytes to write.
/// \return A pair containing an error condition if writing stopped due
///   to an error, and the actual number of bytes written.
std::pair<std::error_condition, std::size_t> write(write_stream &stream, void const *buffer, std::size_t length) noexcept;

/// \brief Write at specified position
///
/// Writes up to the specified number of bytes from the supplied buffer,
/// continuing if interrupted by asynchronous signals.  If seeking is
/// supported, writing starts at the specified position and the current
/// position is unaffected.   May write less than the requested number
/// of bytes if an error occurs.  The operation may not be atomic if it
/// is interrupted before the requested number of bytes is written.
/// \param [in] stream The stream to write to.
/// \param [in] offset The position to start writing at, specified as a
///   number of bytes from the beginning of the stream.
/// \param [in] buffer Buffer containing the data to write.  Must
///   contain at least the specified number of bytes.
/// \param [in] length Number of bytes to write.
/// \return A pair containing an error condition if writing stopped due
///   to an error, and the actual number of bytes written.
std::pair<std::error_condition, std::size_t> write_at(random_write &stream, std::uint64_t offset, void const *buffer, std::size_t length) noexcept;

/// \}


random_read::ptr ram_read(void const *data, std::size_t size) noexcept;
random_read::ptr ram_read(void const *data, std::size_t size, std::uint8_t filler) noexcept;
random_read::ptr ram_read_copy(void const *data, std::size_t size) noexcept;
random_read::ptr ram_read_copy(void const *data, std::size_t size, std::uint8_t filler) noexcept;

random_read::ptr stdio_read(FILE *file) noexcept;
random_read::ptr stdio_read(FILE *file, std::uint8_t filler) noexcept;
random_read::ptr stdio_read_noclose(FILE *file) noexcept;
random_read::ptr stdio_read_noclose(FILE *file, std::uint8_t filler) noexcept;

random_read_write::ptr stdio_read_write(FILE *file) noexcept;
random_read_write::ptr stdio_read_write(FILE *file, std::uint8_t filler) noexcept;
random_read_write::ptr stdio_read_write_noclose(FILE *file) noexcept;
random_read_write::ptr stdio_read_write_noclose(FILE *file, std::uint8_t filler) noexcept;

random_read::ptr osd_file_read(std::unique_ptr<osd_file> &&file) noexcept;
random_read::ptr osd_file_read(osd_file &file) noexcept;

random_read_write::ptr osd_file_read_write(std::unique_ptr<osd_file> &&file) noexcept;
random_read_write::ptr osd_file_read_write(osd_file &file) noexcept;

} // namespace util

#endif // MAME_LIB_UTIL_IOPROCS_H
