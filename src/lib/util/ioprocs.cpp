// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ioprocs.cpp

    I/O interface implementations for RAM, C standard I/O and OSD files

***************************************************************************/

#include "ioprocs.h"

#include "ioprocsfill.h"

#include "osdfile.h"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <iterator>
#include <limits>
#include <new>
#include <type_traits>


namespace util {

namespace {

// helper for holding a block of memory and deallocating it (or not) as necessary

template <typename T, bool Owned>
class ram_adapter_base : public virtual random_access
{
public:
	virtual ~ram_adapter_base()
	{
		if constexpr (Owned)
			std::free(m_data);
	}

	virtual std::error_condition seek(std::int64_t offset, int whence) noexcept override
	{
		switch (whence)
		{
		case SEEK_SET:
			if (0 > offset)
				return std::errc::invalid_argument;
			m_pointer = std::uint64_t(offset);
			return std::error_condition();

		case SEEK_CUR:
			if (0 > offset)
			{
				if (std::uint64_t(-offset) > m_pointer)
					return std::errc::invalid_argument;
			}
			else if ((std::numeric_limits<std::uint64_t>::max() - offset) < m_pointer)
			{
				return std::errc::invalid_argument;
			}
			m_pointer += offset;
			return std::error_condition();

		case SEEK_END:
			if (0 > offset)
			{
				if (std::uint64_t(-offset) > m_size)
					return std::errc::invalid_argument;
			}
			else if ((std::numeric_limits<std::uint64_t>::max() - offset) < m_size)
			{
				return std::errc::invalid_argument;
			}
			m_pointer = std::uint64_t(m_size) + offset;
			return std::error_condition();

		default:
			return std::errc::invalid_argument;
		}
	}

	virtual std::error_condition tell(std::uint64_t &result) noexcept override
	{
		result = m_pointer;
		return std::error_condition();
	}

	virtual std::error_condition length(std::uint64_t &result) noexcept override
	{
		if (std::numeric_limits<uint64_t>::max() < m_size)
			return std::errc::file_too_large;

		result = m_size;
		return std::error_condition();
	}

protected:
	template <typename U>
	ram_adapter_base(U *data, std::size_t size) noexcept : m_data(reinterpret_cast<T>(data)), m_size(size)
	{
		static_assert(sizeof(*m_data) == 1U, "Element type must be byte-sized");
		assert(m_data || !m_size);
	}

	T m_data;
	std::uint64_t m_pointer = 0U;
	std::size_t m_size;
};


// RAM read implementation

template <typename T, bool Owned>
class ram_read_adapter : public ram_adapter_base<T, Owned>, public virtual random_read
{
public:
	template <typename U>
	ram_read_adapter(U *data, std::size_t size) noexcept : ram_adapter_base<T, Owned>(data, size)
	{
	}

	virtual std::error_condition read_some(void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		do_read(this->m_pointer, buffer, length, actual);
		this->m_pointer += actual;
		return std::error_condition();
	}

	virtual std::error_condition read_some_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		do_read(offset, buffer, length, actual);
		return std::error_condition();
	}

private:
	void do_read(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) const noexcept
	{
		if ((offset < this->m_size) && length)
		{
			actual = std::min(std::size_t(this->m_size - offset), length);
			if constexpr (Owned)
				std::memcpy(buffer, this->m_data + offset, actual);
			else
				std::memmove(buffer, this->m_data + offset, actual);
		}
		else
		{
			actual = 0U;
		}
	}
};


// helper for holding a stdio FILE and closing it (or not) as necessary

class stdio_adapter_base : public virtual random_access
{
public:
	virtual ~stdio_adapter_base()
	{
		if (m_close)
			std::fclose(m_file);
	}

	virtual std::error_condition seek(std::int64_t offset, int whence) noexcept override
	{
		if ((std::numeric_limits<long>::max() < offset) || (std::numeric_limits<long>::min() > offset))
			return std::errc::invalid_argument;
		else if (!std::fseek(m_file, long(offset), whence))
			return std::error_condition();
		else
			return std::error_condition(errno, std::generic_category());
	}

	virtual std::error_condition tell(std::uint64_t &result) noexcept override
	{
		long const pos = std::ftell(m_file);
		if (0 > pos)
			return std::error_condition(errno, std::generic_category());
		result = static_cast<unsigned long>(pos);
		return std::error_condition();
	}

	virtual std::error_condition length(std::uint64_t &result) noexcept override
	{
		std::fpos_t oldpos;
		if (std::fgetpos(m_file, &oldpos))
			return std::error_condition(errno, std::generic_category());

		long endpos = -1;
		if (!std::fseek(m_file, 0, SEEK_END))
		{
			m_dangling_read = m_dangling_write = false;
			endpos = std::ftell(m_file);
		}
		std::error_condition err;
		if (0 > endpos)
			err.assign(errno, std::generic_category());
		else if (std::numeric_limits<std::uint64_t>::max() < static_cast<unsigned long>(endpos))
			err = std::errc::file_too_large;
		else
			result = static_cast<unsigned long>(endpos);

		if (!std::fsetpos(m_file, &oldpos))
			m_dangling_read = m_dangling_write = false;
		else if (!err)
			err.assign(errno, std::generic_category());

		return err;
	}

protected:
	stdio_adapter_base(FILE *file, bool close) noexcept : m_file(file), m_close(close)
	{
		assert(m_file);
	}

	FILE *file() noexcept
	{
		return m_file;
	}

private:
	FILE *const m_file;
	bool const m_close;

protected:
	bool m_dangling_read = true, m_dangling_write = true;
};


// stdio read implementation

class stdio_read_adapter : public stdio_adapter_base, public virtual random_read
{
public:
	stdio_read_adapter(FILE *file, bool close) noexcept : stdio_adapter_base(file, close)
	{
	}

	virtual std::error_condition read_some(void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		if (m_dangling_write)
		{
			if (std::fflush(file()))
			{
				std::clearerr(file());
				actual = 0U;
				return std::error_condition(errno, std::generic_category());
			}
			m_dangling_write = false;
		}

		actual = std::fread(buffer, sizeof(std::uint8_t), length, file());
		m_dangling_read = true;
		if (length != actual)
		{
			if (std::ferror(file()))
			{
				std::clearerr(file());
				return std::error_condition(errno, std::generic_category());
			}
			else if (std::feof(file()))
			{
				m_dangling_read = false;
			}
		}

		return std::error_condition();
	}

	virtual std::error_condition read_some_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		actual = 0U;

		if (static_cast<unsigned long>(std::numeric_limits<long>::max()) < offset)
			return std::errc::invalid_argument;

		std::fpos_t oldpos;
		if (std::fgetpos(file(), &oldpos))
			return std::error_condition(errno, std::generic_category());

		std::error_condition err;
		if (std::fseek(file(), long(static_cast<unsigned long>(offset)), SEEK_SET))
		{
			err.assign(errno, std::generic_category());
		}
		else
		{
			m_dangling_write = false;
			actual = std::fread(buffer, sizeof(std::uint8_t), length, file());
			m_dangling_read = true;
			if (length != actual)
			{
				if (std::ferror(file()))
				{
					err.assign(errno, std::generic_category());
					std::clearerr(file());
				}
				else if (std::feof(file()))
				{
					m_dangling_read = false;
				}
			}
		}

		if (!std::fsetpos(file(), &oldpos))
			m_dangling_read = m_dangling_write = false;
		else if (!err)
			err.assign(errno, std::generic_category());

		return err;
	}
};


// stdio read/write implementation

class stdio_read_write_adapter : public stdio_read_adapter, public random_read_write
{
public:
	using stdio_read_adapter::stdio_read_adapter;

	virtual std::error_condition finalize() noexcept override
	{
		return std::error_condition();
	}

	virtual std::error_condition flush() noexcept override
	{
		if (std::fflush(file()))
		{
			std::clearerr(file());
			return std::error_condition(errno, std::generic_category());
		}
		m_dangling_write = false;
		return std::error_condition();
	}

	virtual std::error_condition write_some(void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		if (m_dangling_read)
		{
			if (std::fseek(file(), 0, SEEK_CUR))
			{
				actual = 0U;
				return std::error_condition(errno, std::generic_category());
			}
			m_dangling_read = false;
		}

		actual = std::fwrite(buffer, sizeof(std::uint8_t), length, file());
		m_dangling_write = true;
		if (length != actual)
		{
			std::clearerr(file());
			return std::error_condition(errno, std::generic_category());
		}

		return std::error_condition();
	}

	virtual std::error_condition write_some_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		actual = 0U;

		if (static_cast<unsigned long>(std::numeric_limits<long>::max()) < offset)
			return std::errc::invalid_argument;

		std::fpos_t oldpos;
		if (std::fgetpos(file(), &oldpos))
			return std::error_condition(errno, std::generic_category());

		std::error_condition err;
		if (std::fseek(file(), long(static_cast<unsigned long>(offset)), SEEK_SET))
		{
			err.assign(errno, std::generic_category());
		}
		else
		{
			m_dangling_read = false;
			actual = std::fwrite(buffer, sizeof(std::uint8_t), length, file());
			m_dangling_write = true;
			if (length != actual)
			{
				err.assign(errno, std::generic_category());
				std::clearerr(file());
			}
		}

		if (!std::fsetpos(file(), &oldpos))
			m_dangling_read = m_dangling_write = false;
		else if (!err)
			err.assign(errno, std::generic_category());

		return err;
	}
};


// stdio helper that fills space when writing past the end-of-file

class stdio_read_write_filler : public random_read_fill_wrapper<stdio_read_write_adapter>
{
public:
	stdio_read_write_filler(FILE *file, bool close, std::uint8_t fill) noexcept : random_read_fill_wrapper<stdio_read_write_adapter>(file, close)
	{
		set_filler(fill);
	}

	virtual std::error_condition write_some(void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		actual = 0U;

		long const offset = std::ftell(file());
		if (0 > offset)
			return std::error_condition(errno, std::generic_category());

		if (std::fseek(file(), 0, SEEK_END))
		{
			std::error_condition err(errno, std::generic_category());
			if (!std::fseek(file(), offset, SEEK_SET))
				m_dangling_read = m_dangling_write = false;
			return err;
		}
		m_dangling_read = m_dangling_write = false;
		long endpos = std::ftell(file());
		if (0 > endpos)
		{
			std::error_condition err(errno, std::generic_category());
			std::fseek(file(), offset, SEEK_SET);
			return err;
		}

		if (offset > endpos)
		{
			std::uint8_t block[1024];
			std::fill_n(
					block,
					std::min<std::common_type_t<std::size_t, std::uint64_t, long> >(std::size(block), offset - endpos),
					get_filler());
			do
			{
				std::size_t const chunk = std::min<std::common_type_t<std::size_t, std::uint64_t, long> >(std::size(block), offset - endpos);
				std::size_t const filled = std::fwrite(block, sizeof(block[0]), chunk, file());
				endpos += filled;
				m_dangling_write = true;
				if (chunk != filled)
				{
					std::error_condition err(errno, std::generic_category());
					std::clearerr(file());
					if (!std::fseek(file(), offset, SEEK_SET))
						m_dangling_write = false;
					return err;
				}
			}
			while (static_cast<unsigned long>(endpos) < offset);
		}
		else if ((offset < endpos) && std::fseek(file(), offset, SEEK_SET))
		{
			return std::error_condition(errno, std::generic_category());
		}

		actual = std::fwrite(buffer, sizeof(std::uint8_t), length, file());
		m_dangling_write = true;
		if (length != actual)
		{
			std::clearerr(file());
			return std::error_condition(errno, std::generic_category());
		}

		return std::error_condition();
	}

	virtual std::error_condition write_some_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		actual = 0U;

		if (static_cast<unsigned long>(std::numeric_limits<long>::max()) < offset)
			return std::errc::invalid_argument;

		std::fpos_t oldpos;
		if (std::fgetpos(file(), &oldpos))
			return std::error_condition(errno, std::generic_category());

		std::error_condition err;
		if (std::fseek(file(), 0, SEEK_END))
		{
			err.assign(errno, std::generic_category());
		}
		else
		{
			m_dangling_read = m_dangling_write = false;
			long endpos = std::ftell(file());
			if (0 > endpos)
			{
				err.assign(errno, std::generic_category());
			}
			else if (static_cast<unsigned long>(endpos) > offset)
			{
				if (std::fseek(file(), long(static_cast<unsigned long>(offset)), SEEK_SET))
					err.assign(errno, std::generic_category());
			}
			else if (static_cast<unsigned long>(endpos) < offset)
			{
				std::uint8_t block[1024];
				std::fill_n(
						block,
						std::min<std::common_type_t<std::size_t, std::uint64_t, long> >(std::size(block), offset - endpos),
						get_filler());
				do
				{
					std::size_t const chunk = std::min<std::common_type_t<std::size_t, std::uint64_t, long> >(std::size(block), offset - endpos);
					std::size_t const filled = std::fwrite(block, sizeof(block[0]), chunk, file());
					endpos += filled;
					m_dangling_write = true;
					if (chunk != filled)
					{
						err.assign(errno, std::generic_category());
						std::clearerr(file());
					}
				}
				while (!err && (static_cast<unsigned long>(endpos) < offset));
			}
		}

		if (!err)
		{
			actual = std::fwrite(buffer, sizeof(std::uint8_t), length, file());
			m_dangling_write = true;
			if (length != actual)
			{
				err.assign(errno, std::generic_category());
				std::clearerr(file());
			}
		}

		if (!std::fsetpos(file(), &oldpos))
			m_dangling_read = m_dangling_write = false;
		else if (!err)
			err.assign(errno, std::generic_category());

		return err;
	}
};


// helper class for holding an osd_file and closing it (or not) as necessary

class osd_file_adapter_base : public virtual random_access
{
public:
	virtual ~osd_file_adapter_base()
	{
		if (m_close)
			delete m_file;
	}

	virtual std::error_condition seek(std::int64_t offset, int whence) noexcept override
	{
		switch (whence)
		{
		case SEEK_SET:
			if (0 > offset)
				return std::errc::invalid_argument;
			m_pointer = std::uint64_t(offset);
			return std::error_condition();

		case SEEK_CUR:
			if (0 > offset)
			{
				if (std::uint64_t(-offset) > m_pointer)
					return std::errc::invalid_argument;
			}
			else if ((std::numeric_limits<std::uint64_t>::max() - offset) < m_pointer)
			{
				return std::errc::invalid_argument;
			}
			m_pointer += offset;
			return std::error_condition();

		// TODO: add SEEK_END when osd_file can support it - should it return a different error?
		default:
			return std::errc::invalid_argument;
		}
	}

	virtual std::error_condition tell(std::uint64_t &result) noexcept override
	{
		result = m_pointer;
		return std::error_condition();
	}

	virtual std::error_condition length(std::uint64_t &result) noexcept override
	{
		// not supported by osd_file
		return std::errc::not_supported; // TODO: revisit this error code
	}

protected:
	osd_file_adapter_base(osd_file::ptr &&file) noexcept : m_file(file.release()), m_close(true)
	{
		assert(m_file);
	}

	osd_file_adapter_base(osd_file &file) noexcept : m_file(&file), m_close(false)
	{
	}

	osd_file &file() noexcept
	{
		return *m_file;
	}

	std::uint64_t m_pointer = 0U;

private:
	osd_file *const m_file;
	bool const m_close;
};


// osd_file read implementation

class osd_file_read_adapter : public osd_file_adapter_base, public virtual random_read
{
public:
	osd_file_read_adapter(osd_file::ptr &&file) noexcept : osd_file_adapter_base(std::move(file))
	{
	}

	osd_file_read_adapter(osd_file &file) noexcept : osd_file_adapter_base(file)
	{
	}

	virtual std::error_condition read_some(void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		// actual length not valid on error
		std::uint32_t const chunk = std::min<std::common_type_t<std::uint32_t, std::size_t> >(std::numeric_limits<std::uint32_t>::max(), length);
		std::uint32_t count;
		std::error_condition err = file().read(buffer, m_pointer, chunk, count);
		if (!err)
		{
			m_pointer += count;
			actual = std::size_t(count);
		}
		else
		{
			actual = 0U;
		}
		return err;
	}

	virtual std::error_condition read_some_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		// actual length not valid on error
		std::uint32_t const chunk = std::min<std::common_type_t<std::uint32_t, std::size_t> >(std::numeric_limits<std::uint32_t>::max(), length);
		std::uint32_t count;
		std::error_condition err = file().read(buffer, offset, chunk, count);
		if (!err)
			actual = std::size_t(count);
		else
			actual = 0U;
		return err;
	}
};


// osd_file read/write implementation

class osd_file_read_write_adapter : public osd_file_read_adapter, public random_read_write
{
public:
	using osd_file_read_adapter::osd_file_read_adapter;

	virtual std::error_condition finalize() noexcept override
	{
		return std::error_condition();
	}

	virtual std::error_condition flush() noexcept override
	{
		return file().flush();
	}

	virtual std::error_condition write_some(void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		// actual length not valid on error
		std::uint32_t const chunk = std::min<std::common_type_t<std::uint32_t, std::size_t> >(std::numeric_limits<std::uint32_t>::max(), length);
		std::uint32_t count;
		std::error_condition err = file().write(buffer, m_pointer, chunk, count);
		if (!err)
		{
			actual = std::size_t(count);
			m_pointer += count;
		}
		else
		{
			actual = 0U;
		}
		return err;
	}

	virtual std::error_condition write_some_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		// actual length not valid on error
		std::uint32_t const chunk = std::min<std::common_type_t<std::uint32_t, std::size_t> >(std::numeric_limits<std::uint32_t>::max(), length);
		std::uint32_t count;
		std::error_condition err = file().write(buffer, offset, chunk, count);
		if (!err)
			actual = std::size_t(count);
		else
			actual = 0U;
		return err;
	}
};

} // anonymous namespace


// helper functions for common patterns

std::pair<std::error_condition, std::size_t> read(read_stream &stream, void *buffer, std::size_t length) noexcept
{
	std::size_t actual = 0;
	do
	{
		std::size_t count;
		std::error_condition err = stream.read_some(buffer, length, count);
		actual += count;
		if (!err)
		{
			if (!count)
				break;
		}
		else if (std::errc::interrupted != err)
		{
			return std::make_pair(err, actual);
		}
		buffer = reinterpret_cast<std::uint8_t *>(buffer) + count;
		length -= count;
	}
	while (length);
	return std::make_pair(std::error_condition(), actual);
}

std::tuple<std::error_condition, std::unique_ptr<std::uint8_t []>, std::size_t> read(read_stream &stream, std::size_t length) noexcept
{
	std::unique_ptr<std::uint8_t []> buffer(new (std::nothrow) std::uint8_t [length]);
	if (!buffer)
		return std::make_tuple(std::errc::not_enough_memory, std::move(buffer), std::size_t(0));
	auto [err, actual] = read(stream, buffer.get(), length);
	return std::make_tuple(err, std::move(buffer), actual);
}

std::pair<std::error_condition, std::size_t> read_at(random_read &stream, std::uint64_t offset, void *buffer, std::size_t length) noexcept
{
	std::size_t actual = 0;
	do
	{
		std::size_t count;
		std::error_condition err = stream.read_some_at(offset, buffer, length, count);
		actual += count;
		if (!err)
		{
			if (!count)
				break;
		}
		else if (std::errc::interrupted != err)
		{
			return std::make_pair(err, actual);
		}
		offset += count;
		buffer = reinterpret_cast<std::uint8_t *>(buffer) + count;
		length -= count;
	}
	while (length);
	return std::make_pair(std::error_condition(), actual);
}

std::tuple<std::error_condition, std::unique_ptr<std::uint8_t []>, std::size_t> read_at(random_read &stream, std::uint64_t offset, std::size_t length) noexcept
{
	std::unique_ptr<std::uint8_t []> buffer(new (std::nothrow) std::uint8_t [length]);
	if (!buffer)
		return std::make_tuple(std::errc::not_enough_memory, std::move(buffer), std::size_t(0));
	auto [err, actual] = read_at(stream, offset, buffer.get(), length);
	return std::make_tuple(err, std::move(buffer), actual);
}

std::pair<std::error_condition, std::size_t> write(write_stream &stream, void const *buffer, std::size_t length) noexcept
{
	std::size_t actual = 0;
	do
	{
		std::size_t written;
		std::error_condition err = stream.write_some(buffer, length, written);
		actual += written;
		if ((err || !actual) && (std::errc::interrupted != err))
			return std::make_pair(err, actual);
		buffer = reinterpret_cast<std::uint8_t const *>(buffer) + written;
		length -= written;
	}
	while (length);
	return std::make_pair(std::error_condition(), actual);
}

std::pair<std::error_condition, std::size_t> write_at(random_write &stream, std::uint64_t offset, void const *buffer, std::size_t length) noexcept
{
	std::size_t actual = 0;
	do
	{
		std::size_t written;
		std::error_condition err = stream.write_some_at(offset, buffer, length, written);
		actual += written;
		if (err && (std::errc::interrupted != err))
			return std::make_pair(err, actual);
		offset += written;
		buffer = reinterpret_cast<std::uint8_t const *>(buffer) + written;
		length -= written;
	}
	while (length);
	return std::make_pair(std::error_condition(), actual);
}


// creating RAM read adapters

random_read::ptr ram_read(void const *data, std::size_t size) noexcept
{
	random_read::ptr result;
	if (data || !size)
		result.reset(new (std::nothrow) ram_read_adapter<std::uint8_t const *const, false>(data, size));
	return result;
}

random_read::ptr ram_read(void const *data, std::size_t size, std::uint8_t filler) noexcept
{
	std::unique_ptr<random_read_fill_wrapper<ram_read_adapter<std::uint8_t const *const, false> > > result;
	if (data || !size)
		result.reset(new (std::nothrow) decltype(result)::element_type(data, size));
	if (result)
		result->set_filler(filler);
	return result;
}

random_read::ptr ram_read_copy(void const *data, std::size_t size) noexcept
{
	random_read::ptr result;
	void *const copy = size ? std::malloc(size) : nullptr;
	if (copy)
		std::memcpy(copy, data, size);
	if (copy || !size)
		result.reset(new (std::nothrow) ram_read_adapter<std::uint8_t *const, true>(copy, size));
	if (!result)
		std::free(copy);
	return result;
}

random_read::ptr ram_read_copy(void const *data, std::size_t size, std::uint8_t filler) noexcept
{
	std::unique_ptr<random_read_fill_wrapper<ram_read_adapter<std::uint8_t *const, true> > > result;
	void *const copy = size ? std::malloc(size) : nullptr;
	if (copy)
		std::memcpy(copy, data, size);
	if (copy || !size)
		result.reset(new (std::nothrow) decltype(result)::element_type(copy, size));
	if (!result)
		std::free(copy);
	return result;
}


// creating stdio read adapters

random_read::ptr stdio_read(FILE *file) noexcept
{
	random_read::ptr result;
	if (file)
		result.reset(new (std::nothrow) stdio_read_adapter(file, true));
	return result;
}

random_read::ptr stdio_read(FILE *file, std::uint8_t filler) noexcept
{
	std::unique_ptr<random_read_fill_wrapper<stdio_read_adapter> > result;
	if (file)
		result.reset(new (std::nothrow) decltype(result)::element_type(file, true));
	if (result)
		result->set_filler(filler);
	return result;
}

random_read::ptr stdio_read_noclose(FILE *file) noexcept
{
	random_read::ptr result;
	if (file)
		result.reset(new (std::nothrow) stdio_read_adapter(file, false));
	return result;
}

random_read::ptr stdio_read_noclose(FILE *file, std::uint8_t filler) noexcept
{
	std::unique_ptr<random_read_fill_wrapper<stdio_read_adapter> > result;
	if (file)
		result.reset(new (std::nothrow) decltype(result)::element_type(file, false));
	if (result)
		result->set_filler(filler);
	return result;
}


// creating stdio read/write adapters

random_read_write::ptr stdio_read_write(FILE *file) noexcept
{
	random_read_write::ptr result;
	if (file)
		result.reset(new (std::nothrow) stdio_read_write_adapter(file, true));
	return result;
}

random_read_write::ptr stdio_read_write(FILE *file, std::uint8_t filler) noexcept
{
	random_read_write::ptr result;
	if (file)
		result.reset(new (std::nothrow) stdio_read_write_filler(file, true, filler));
	return result;
}

random_read_write::ptr stdio_read_write_noclose(FILE *file) noexcept
{
	random_read_write::ptr result;
	if (file)
		result.reset(new (std::nothrow) stdio_read_write_adapter(file, false));
	return result;
}

random_read_write::ptr stdio_read_write_noclose(FILE *file, std::uint8_t filler) noexcept
{
	random_read_write::ptr result;
	if (file)
		result.reset(new (std::nothrow) stdio_read_write_filler(file, false, filler));
	return result;
}


// creating osd_file read adapters

random_read::ptr osd_file_read(osd_file::ptr &&file) noexcept
{
	random_read::ptr result;
	if (file)
		result.reset(new (std::nothrow) osd_file_read_adapter(std::move(file)));
	return result;
}

random_read::ptr osd_file_read(osd_file &file) noexcept
{
	return random_read::ptr(new (std::nothrow) osd_file_read_adapter(file));
}


// creating osd_file read/write adapters

random_read_write::ptr osd_file_read_write(osd_file::ptr &&file) noexcept
{
	random_read_write::ptr result;
	if (file)
		result.reset(new (std::nothrow) osd_file_read_write_adapter(std::move(file)));
	return result;
}

random_read_write::ptr osd_file_read_write(osd_file &file) noexcept
{
	return random_read_write::ptr(new (std::nothrow) osd_file_read_write_adapter(file));
}

} // namespace util
