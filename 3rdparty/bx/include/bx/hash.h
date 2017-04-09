/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_HASH_H_HEADER_GUARD
#define BX_HASH_H_HEADER_GUARD

#include "allocator.h" // isAligned
#include "string.h" // StringView

namespace bx
{
	/// MurmurHash2 was written by Austin Appleby, and is placed in the public
	/// domain. The author hereby disclaims copyright to this source code.
	///
	class HashMurmur2A
	{
	public:
		///
		void begin(uint32_t _seed = 0);

		///
		void add(const void* _data, int _len);

		///
		void addAligned(const void* _data, int _len);

		///
		void addUnaligned(const void* _data, int _len);

		///
		template<typename Ty>
		void add(Ty _value);

		///
		uint32_t end();

	private:
		///
		static void readUnaligned(const void* _data, uint32_t& _out);

		///
		void mixTail(const uint8_t*& _data, int& _len);

		uint32_t m_hash;
		uint32_t m_tail;
		uint32_t m_count;
		uint32_t m_size;
	};

	///
	uint32_t hashMurmur2A(const void* _data, uint32_t _size);

	///
	template <typename Ty>
	uint32_t hashMurmur2A(const Ty& _data);

	///
	uint32_t hashMurmur2A(const StringView& _data);

	///
	uint32_t hashMurmur2A(const char* _data);

} // namespace bx

#include "inline/hash.inl"

#endif // BX_HASH_H_HEADER_GUARD
