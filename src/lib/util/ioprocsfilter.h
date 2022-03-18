// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ioprocs.h

    I/O filters

***************************************************************************/
#ifndef MAME_LIB_UTIL_IOPROCSFILTER_H
#define MAME_LIB_UTIL_IOPROCSFILTER_H

#include "utilfwd.h"

#include <cstdint>
#include <cstdlib>
#include <memory>


namespace util {

/// \addtogroup ioprocs
/// \{

/// \brief Create a read stream filter that fills unused buffer space
///
/// Creates a sequential read stream filter that fills unused buffer
/// space with a specified byte value.  If a read operation does not
/// produce enough data to fill the supplied buffer, the remaining space
/// in the buffer is filled with the specified filler byte value.  Takes
/// ownership of the underlying input stream.
/// \param [in] stream Underlying stream to read from.
/// \param [in] filler Byte value to fill unused buffer space.
/// \return A pointer to a sequential read stream, or nullptr on error.
/// \sa read_stream
std::unique_ptr<read_stream> read_stream_fill(std::unique_ptr<read_stream> &&stream, std::uint8_t filler) noexcept;

/// \brief Create a random access input filter that fills unused buffer
///   space
///
/// Creates a random access input filter that fills unused buffer space
/// with a specified byte value.  If a read operation does not produce
/// enough data to fill the supplied buffer, the remaining space in the
/// buffer is filled with the specified filler byte value.  Takes
/// ownership of the underlying input sequence.
/// \param [in] stream Underlying input sequence to read from.
/// \param [in] filler Byte value to fill unused buffer space.
/// \return A pointer to a random access input sequence, or nullptr on
///   error.
/// \sa random_read
std::unique_ptr<random_read> random_read_fill(std::unique_ptr<random_read> &&stream, std::uint8_t filler) noexcept;

/// \brief Create a read stream filter that fills unused buffer space
///
/// Creates a sequential read stream filter that fills unused buffer
/// space with a specified byte value.  If a read operation does not
/// produce enough data to fill the supplied buffer, the remaining space
/// in the buffer is filled with the specified filler byte value.  Does
/// not take ownership of the underlying input stream.
/// \param [in] stream Underlying stream to read from.
/// \param [in] filler Byte value to fill unused buffer space.
/// \return A pointer to a sequential read stream, or nullptr on error.
/// \sa read_stream
std::unique_ptr<read_stream> read_stream_fill(read_stream &stream, std::uint8_t filler) noexcept;

/// \brief Create a random access input filter that fills unused buffer
///   space
///
/// Creates a random access input filter that fills unused buffer space
/// with a specified byte value.  If a read operation does not produce
/// enough data to fill the supplied buffer, the remaining space in the
/// buffer is filled with the specified filler byte value.  Does not
/// take ownership of the underlying input sequence.
/// \param [in] stream Underlying input sequence to read from.
/// \param [in] filler Byte value to fill unused buffer space.
/// \return A pointer to a random access input sequence, or nullptr on
///   error.
/// \sa random_read
std::unique_ptr<random_read> random_read_fill(random_read &stream, std::uint8_t filler) noexcept;


/// \brief Create a random access output filter that fills unwritten
///   space
///
/// Creates a random access output filter that fills unwritten space
/// with a specified byte value.  If a write operation starts beyond the
/// end of the output, the space between the end of the output and the
/// start of the written data is filled with the specified filler byte
/// value.  Takes ownership of the underlying output sequence.
/// \param [in] stream Underlying output sequence to write to.
/// \param [in] filler Byte value to fill unwritten space.
/// \return A pointer to a random access output sequence, or nullptr on
///   error.
/// \sa random_write
std::unique_ptr<random_write> random_write_fill(std::unique_ptr<random_write> &&stream, std::uint8_t filler) noexcept;

/// \brief Create a random access output filter that fills unwritten
///   space
///
/// Creates a random access output filter that fills unwritten space
/// with a specified byte value.  If a write operation starts beyond the
/// end of the output, the space between the end of the output and the
/// start of the written data is filled with the specified filler byte
/// value.  Does not take ownership of the underlying output sequence.
/// \param [in] stream Underlying output sequence to write to.
/// \param [in] filler Byte value to fill unwritten space.
/// \return A pointer to a random access output sequence, or nullptr on
///   error.
/// \sa random_write
std::unique_ptr<random_write> random_write_fill(random_write &stream, std::uint8_t filler) noexcept;


/// \brief Create a random access I/O filter that fills unused space
///
/// Creates a random access I/O sequence that fills unused read buffer
/// space and unwritten space with a specified byte value.  If a read
/// operation does not produce enough data to fill the supplied buffer,
/// the remaining space in the buffer is filled with the specified
/// filler byte value.  If a write operation starts beyond the end of
/// the output, the space between the end of the output and the start of
/// the written data is filled with the specified filler byte value.
/// Takes ownership of the underlying I/O sequence.
/// \param [in] stream Underlying I/O sequence to read from and write
///   to.
/// \param [in] filler Byte value to fill unused read buffer space and
///   unwritten space.
/// \return A pointer to a random access I/O sequence, or nullptr on
///   error.
/// \sa random_read_write
std::unique_ptr<random_read_write> random_read_write_fill(std::unique_ptr<random_read_write> &&stream, std::uint8_t filler) noexcept;

/// \brief Create a random access I/O filter that fills unused space
///
/// Creates a random access I/O sequence that fills unused read buffer
/// space and unwritten space with a specified byte value.  If a read
/// operation does not produce enough data to fill the supplied buffer,
/// the remaining space in the buffer is filled with the specified
/// filler byte value.  If a write operation starts beyond the end of
/// the output, the space between the end of the output and the start of
/// the written data is filled with the specified filler byte value.
/// Does not take ownership of the underlying I/O sequence.
/// \param [in] stream Underlying I/O sequence to read from and write
///   to.
/// \param [in] filler Byte value to fill unused read buffer space and
///   unwritten space.
/// \return A pointer to a random access I/O sequence, or nullptr on
///   error.
/// \sa random_read_write
std::unique_ptr<random_read_write> random_read_write_fill(random_read_write &stream, std::uint8_t filler) noexcept;


/// \brief Create an input stream filter that decompresses
///   zlib-compressed data
///
/// Creates a read stream that decompresses zlib-compressed (deflated)
/// data read from the underlying input stream.  A read operation will
/// always stop on reaching an end-of-stream marker in the compressed
/// data.  A subsequent read operation will expect to find the beginning
/// of another block of compressed data.  May read past the end of the
/// compressed data in the underlying input stream.  Takes ownership of
/// the underlying input stream.
/// \param [in] stream Underlying input stream to read from.
/// \param [in] read_chunk Size of buffer for reading compressed data in
///   bytes.
/// \return A pointer to an input stream, or nullptr on error.
/// \sa read_stream
std::unique_ptr<read_stream> zlib_read(std::unique_ptr<read_stream> &&stream, std::size_t read_chunk) noexcept;

/// \brief Create an input stream filter that decompresses
///   zlib-compressed data
///
/// Creates a read stream that decompresses zlib-compressed (deflated)
/// data read from the underlying input sequence.  A read operation will
/// always stop on reaching an end-of-stream marker in the compressed
/// data.  A subsequent read operation will expect to find the beginning
/// of another block of compressed data.  If a read operation reads past
/// an end-of-stream marker in the compressed data, it will seek back so
/// the position for the next read from the underlying input sequence
/// immediately follows the end-of-stream marker.  Takes ownership of
/// the underlying input sequence.
/// \param [in] stream Underlying input sequence to read from.  Must
///   support seeking relative to the current position.
/// \param [in] read_chunk Size of buffer for reading compressed data in
///   bytes.
/// \return A pointer to an input stream, or nullptr on error.
/// \sa read_stream random_read
std::unique_ptr<read_stream> zlib_read(std::unique_ptr<random_read> &&stream, std::size_t read_chunk) noexcept;

/// \brief Create an input stream filter that decompresses
///   zlib-compressed data
///
/// Creates a read stream that decompresses zlib-compressed (deflated)
/// data read from the underlying input stream.  A read operation will
/// always stop on reaching an end-of-stream marker in the compressed
/// data.  A subsequent read operation will expect to find the beginning
/// of another block of compressed data.  May read past the end of the
/// compressed data in the underlying input stream.  Does not take
/// ownership of the underlying input stream.
/// \param [in] stream Underlying input stream to read from.
/// \param [in] read_chunk Size of buffer for reading compressed data in
///   bytes.
/// \return A pointer to an input stream, or nullptr on error.
/// \sa read_stream
std::unique_ptr<read_stream> zlib_read(read_stream &stream, std::size_t read_chunk) noexcept;

/// \brief Create an input stream filter that decompresses
///   zlib-compressed data
///
/// Creates a read stream that decompresses zlib-compressed (deflated)
/// data read from the underlying input sequence.  A read operation will
/// always stop on reaching an end-of-stream marker in the compressed
/// data.  A subsequent read operation will expect to find the beginning
/// of another block of compressed data.  If a read operation reads past
/// an end-of-stream marker in the compressed data, it will seek back so
/// the position for the next read from the underlying input sequence
/// immediately follows the end-of-stream marker.  Does not take
/// ownership of the underlying input sequence.
/// \param [in] stream Underlying input sequence to read from.  Must
///   support seeking relative to the current position.
/// \param [in] read_chunk Size of buffer for reading compressed data in
///   bytes.
/// \return A pointer to an input stream, or nullptr on error.
/// \sa read_stream random_read
std::unique_ptr<read_stream> zlib_read(random_read &stream, std::size_t read_chunk) noexcept;


/// \brief Create an output stream filter that writes zlib-compressed
///   data
///
/// Creates an output stream that compresses data using the zlib deflate
/// algorithm and writes it to the underlying output stream.  Calling
/// the \c finalize member function compresses any buffered input,
/// produces an end-of-stream maker, and writes any buffered compressed
/// data to the underlying output stream.  A subsequent write operation
/// will start a new compressed block.  Calling the \c flush member
/// function writes any buffered compressed data to the underlying
/// output stream and calls the \c flush member function of the
/// underlying output stream; it does not ensure all buffered input data
/// is compressed or force the end of a compressed block.  Takes
/// ownership of the underlying output stream.
/// \param [in] stream Underlying output stream for writing compressed
///   data.
/// \param [in] level Compression level.  Use 0 for no compression, 1
///   for fastest compression, 9 for maximum compression, or -1 for the
///   default compression level as defined by the zlib library.  Larger
///   values between 1 and 9 provide higher compression at the expense
///   of speed.
/// \param [in] buffer_size Size of buffer for compressed data in bytes.
/// \return A pointer to an output stream, or nullptr on error.
/// \sa write_stream
std::unique_ptr<write_stream> zlib_write(std::unique_ptr<write_stream> &&stream, int level, std::size_t buffer_size) noexcept;

/// \brief Create an output stream filter that writes zlib-compressed
///   data
///
/// Creates an output stream that compresses data using the zlib deflate
/// algorithm and writes it to the underlying output stream.  Calling
/// the \c finalize member function compresses any buffered input,
/// produces an end-of-stream maker, and writes any buffered compressed
/// data to the underlying output stream.  A subsequent write operation
/// will start a new compressed block.  Calling the \c flush member
/// function writes any buffered compressed data to the underlying
/// output stream and calls the \c flush member function of the
/// underlying output stream; it does not ensure all buffered input data
/// is compressed or force the end of a compressed block.  Does not take
/// ownership of the underlying output stream.
/// \param [in] stream Underlying output stream for writing compressed
///   data.
/// \param [in] level Compression level.  Use 0 for no compression, 1
///   for fastest compression, 9 for maximum compression, or -1 for the
///   default compression level as defined by the zlib library.  Larger
///   values between 1 and 9 provide higher compression at the expense
///   of speed.
/// \param [in] buffer_size Size of buffer for compressed data in bytes.
/// \return A pointer to an output stream, or nullptr on error.
/// \sa write_stream
std::unique_ptr<write_stream> zlib_write(write_stream &stream, int level, std::size_t buffer_size) noexcept;

/// \}

} // namespace util

#endif // MAME_LIB_UTIL_IOPROCSFILTER_H
