// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ioprocsvec.h

    Helper for using a vector as an in-memory file

***************************************************************************/
#ifndef MAME_LIB_UTIL_IOPROCSVEC_H
#define MAME_LIB_UTIL_IOPROCSVEC_H

#include "ioprocs.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <system_error>
#include <type_traits>
#include <vector>


namespace util {

template <typename T = std::uint8_t>
class vector_read_write_adapter : public random_read_write
{
public:
	vector_read_write_adapter(std::vector<T> &storage) noexcept : m_storage(storage)
	{
		check_resized();
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
			check_resized();
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
		check_resized();
		if (std::numeric_limits<uint64_t>::max() < m_size)
			return std::errc::file_too_large;

		result = m_size;
		return std::error_condition();
	}

	virtual std::error_condition read(void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		do_read(m_pointer, buffer, length, actual);
		m_pointer += actual;
		return std::error_condition();
	}

	virtual std::error_condition read_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		do_read(offset, buffer, length, actual);
		return std::error_condition();
	}

	virtual std::error_condition finalize() noexcept override
	{
		return std::error_condition();
	}

	virtual std::error_condition flush() noexcept override
	{
		return std::error_condition();
	}

	virtual std::error_condition write(void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		std::error_condition result = do_write(m_pointer, buffer, length, actual);
		m_pointer += actual;
		return result;
	}

	virtual std::error_condition write_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		return do_write(offset, buffer, length, actual);
	}

private:
	using elements_type = typename std::vector<T>::size_type;
	using common_size_type = std::common_type_t<std::uint64_t, std::size_t>;

	void check_resized() noexcept
	{
		if (m_storage.size() != m_elements)
		{
			m_elements = m_storage.size();
			m_size = m_elements * sizeof(T);
		}
	}

	void do_read(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept
	{
		check_resized();
		if ((offset < m_size) && length)
		{
			actual = std::min(std::size_t(m_size - offset), length);
			std::memmove(buffer, reinterpret_cast<std::uint8_t *>(m_storage.data()) + offset, actual);
		}
		else
		{
			actual = 0U;
		}
	}

	std::error_condition do_write(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept
	{
		check_resized();
		std::size_t allocated = m_elements * sizeof(T);
		std::uint64_t const target = ((std::numeric_limits<std::uint64_t>::max() - offset) >= length)
				? (offset + length)
				: std::numeric_limits<std::uint64_t>::max();

		std::error_condition result;
		if (target > allocated)
		{
			elements_type const required = std::min<std::common_type_t<elements_type, std::uint64_t> >(
					std::numeric_limits<elements_type>::max(),
					(target + sizeof(T) - 1) / sizeof(T)); // technically can overflow but unlikely
			try { m_storage.resize(required); } // unpredictable if constructing/assigning T can throw
			catch (...) { result = std::errc::not_enough_memory; }
			m_elements = m_storage.size();
			allocated = m_elements * sizeof(T);
			assert(allocated >= m_size);

			if (offset > m_size)
			{
				std::size_t const new_size = std::size_t(std::min<common_size_type>(allocated, offset));
				std::memset(
						reinterpret_cast<std::uint8_t *>(m_storage.data()) + m_size,
						0x00, // POSIX says unwritten areas of files should read as zero
						new_size - m_size);
				m_size = new_size;
			}
		}

		std::size_t const limit = std::min<common_size_type>(target, allocated);
		if (limit > offset)
		{
			actual = std::size_t(limit - offset);
			std::memmove(reinterpret_cast<std::uint8_t *>(m_storage.data()) + offset, buffer, actual);
			m_size = limit;
		}
		else
		{
			actual = 0U;
		}
		if (!result && (actual < length))
			result = std::errc::no_space_on_device;

		return result;
	}

	std::vector<T> &m_storage;
	std::uint64_t m_pointer = 0U;
	elements_type m_elements = 0U;
	std::size_t m_size = 0U;
};

} // namespace util

#endif // MAME_LIB_UTIL_IOPROCSVEC_H
