// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ioprocsfill.h

    Wrappers that automatically fill extra space

***************************************************************************/
#ifndef MAME_LIB_UTIL_IOPROCSFILL_H
#define MAME_LIB_UTIL_IOPROCSFILL_H

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <system_error>


namespace util {

template <std::uint8_t DefaultFiller>
class fill_wrapper_base
{
public:
	std::uint8_t get_filler() const noexcept
	{
		return m_filler;
	}

	void set_filler(std::uint8_t filler) noexcept
	{
		m_filler = filler;
	}

private:
	std::uint8_t m_filler = DefaultFiller;
};


template <typename Base, std::uint8_t DefaultFiller = 0U>
class read_stream_fill_wrapper : public Base, public virtual fill_wrapper_base<DefaultFiller>
{
public:
	using Base::Base;

	virtual std::error_condition read(void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		std::error_condition err = Base::read(buffer, length, actual);
		assert(length >= actual);
		std::fill(
				reinterpret_cast<std::uint8_t *>(buffer) + actual,
				reinterpret_cast<std::uint8_t *>(buffer) + length,
				fill_wrapper_base<DefaultFiller>::get_filler());
		return err;
	}
};


template <typename Base, std::uint8_t DefaultFiller = 0U>
class random_read_fill_wrapper : public read_stream_fill_wrapper<Base, DefaultFiller>
{
public:
	using read_stream_fill_wrapper<Base, DefaultFiller>::read_stream_fill_wrapper;

	virtual std::error_condition read_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		std::error_condition err = Base::read_at(offset, buffer, length, actual);
		assert(length >= actual);
		std::fill(
				reinterpret_cast<std::uint8_t *>(buffer) + actual,
				reinterpret_cast<std::uint8_t *>(buffer) + length,
				fill_wrapper_base<DefaultFiller>::get_filler());
		return err;
	}
};


template <typename Base, std::uint8_t DefaultFiller = 0U, std::size_t FillBlock = 1024U>
class random_write_fill_wrapper : public Base, public virtual fill_wrapper_base<DefaultFiller>
{
public:
	using Base::Base;

	virtual std::error_condition write(void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		std::error_condition err;
		actual = 0U;

		std::uint64_t offset;
		std::uint64_t current;
		err = Base::tell(offset);
		if (err)
			return err;
		err = Base::length(current);
		if (err)
			return err;

		if (current < offset)
		{
			std::uint64_t unfilled = offset - current;
			std::uint8_t fill_buffer[FillBlock];
			std::fill(
					fill_buffer,
					fill_buffer + std::min<std::common_type_t<std::size_t, std::uint64_t> >(FillBlock, unfilled),
					fill_wrapper_base<DefaultFiller>::get_filler());
			do
			{
				std::size_t const chunk = std::min<std::common_type_t<std::size_t, std::uint64_t> >(FillBlock, unfilled);
				err = Base::write_at(current, fill_buffer, chunk, actual);
				if (err)
				{
					actual = 0U;
					return err;
				}
				current += chunk;
				unfilled -= chunk;
			}
			while (unfilled);
		}

		return Base::write(buffer, length, actual);
	}

	virtual std::error_condition write_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		std::error_condition err;
		std::uint64_t current;
		err = Base::length(current);
		if (!err && (current < offset))
		{
			std::uint64_t unfilled = offset - current;
			std::uint8_t fill_buffer[FillBlock];
			std::fill(
					fill_buffer,
					fill_buffer + std::min<std::common_type_t<std::size_t, std::uint64_t> >(FillBlock, unfilled),
					fill_wrapper_base<DefaultFiller>::get_filler());
			do
			{
				std::size_t const chunk = std::min<std::common_type_t<std::size_t, std::uint64_t> >(FillBlock, unfilled);
				err = Base::write_at(current, fill_buffer, chunk, actual);
				current += chunk;
				unfilled -= chunk;
			}
			while (unfilled && !err);
		}
		if (err)
		{
			actual = 0U;
			return err;
		}
		return Base::write_at(offset, buffer, length, actual);
	}
};


template <typename Base, std::uint8_t DefaultFiller = 0U, std::size_t FillBlock = 1024U>
using random_read_write_fill_wrapper = random_write_fill_wrapper<random_read_fill_wrapper<Base, DefaultFiller>, DefaultFiller, FillBlock>;

} // namespace util

#endif // MAME_LIB_UTIL_IOPROCSFILL_H
