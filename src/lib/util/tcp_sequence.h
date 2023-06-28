// license:BSD-3-Clause
/***************************************************************************

    tcp_sequence.h

    RFC 9293 TCP sequence comparisons

***************************************************************************/
#ifndef MAME_UTIL_TCP_SEQUENCE_H
#define MAME_UTIL_TCP_SEQUENCE_H

#pragma once

#include <cstdint>
namespace util {
	
class tcp_sequence
{
	public:

	tcp_sequence() = default;
	constexpr tcp_sequence(const tcp_sequence &) = default;
	constexpr tcp_sequence(uint32_t data) : m_data(data)
	{}

	tcp_sequence &operator=(const tcp_sequence &) = default;
	tcp_sequence &operator=(const uint32_t rhs)
	{
		m_data = rhs;
		return *this;
	}

	tcp_sequence &operator+=(const uint32_t rhs)
	{
		m_data += rhs;
		return *this;
	}

	tcp_sequence &operator-=(const uint32_t rhs)
	{
		m_data -= rhs;
		return *this;
	}

	tcp_sequence& operator++()
	{
		++m_data;
		return *this;
	}

	tcp_sequence& operator--()
	{
		--m_data;
		return *this;
	}

	constexpr explicit operator uint32_t() const
	{
		return m_data;
	}

	private:
	uint32_t m_data;

	friend constexpr bool operator==(const tcp_sequence &, const tcp_sequence &);
	friend constexpr bool operator!=(const tcp_sequence &, const tcp_sequence &);
	friend constexpr bool operator<(const tcp_sequence &, const tcp_sequence &);
	friend constexpr bool operator<=(const tcp_sequence &, const tcp_sequence &);
	friend constexpr bool operator>(const tcp_sequence &, const tcp_sequence &);
	friend constexpr bool operator>=(const tcp_sequence &, const tcp_sequence &);

	friend constexpr tcp_sequence operator+(const tcp_sequence &, const uint32_t);
	friend constexpr tcp_sequence operator-(const tcp_sequence &, const uint32_t);
	friend constexpr uint32_t operator-(const tcp_sequence &, const tcp_sequence &);
};

constexpr bool operator==(const tcp_sequence &lhs, const tcp_sequence &rhs)
{
	return lhs.m_data == rhs.m_data;
}

constexpr bool operator!=(const tcp_sequence &lhs, const tcp_sequence &rhs)
{
	return lhs.m_data != rhs.m_data; 
}

constexpr bool operator<(const tcp_sequence &lhs, const tcp_sequence &rhs)
{
	return static_cast<int32_t>(lhs.m_data - rhs.m_data) < 0;
}

constexpr bool operator<=(const tcp_sequence &lhs, const tcp_sequence &rhs)
{
	return static_cast<int32_t>(lhs.m_data - rhs.m_data) <= 0;
}

constexpr bool operator>(const tcp_sequence &lhs, const tcp_sequence &rhs)
{
	return static_cast<int32_t>(lhs.m_data - rhs.m_data) > 0;
}

constexpr bool operator>=(const tcp_sequence &lhs, const tcp_sequence &rhs)
{
	return static_cast<int32_t>(lhs.m_data - rhs.m_data) >= 0;
}

constexpr tcp_sequence operator+(const tcp_sequence &lhs, const uint32_t rhs)
{
	return tcp_sequence(lhs.m_data + rhs);
}

constexpr tcp_sequence operator-(const tcp_sequence &lhs, const uint32_t rhs)
{
	return tcp_sequence(lhs.m_data - rhs);
}

constexpr uint32_t operator-(const tcp_sequence &lhs, const tcp_sequence &rhs)
{
	return lhs.m_data - rhs.m_data;
}

} // namespace util

#endif // MAME_UTIL_TCP_SEQUENCE_H