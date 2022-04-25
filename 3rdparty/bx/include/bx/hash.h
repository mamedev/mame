/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
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
		template<typename Ty>
		void add(Ty _value);

		///
		uint32_t end();

	private:
		///
		void addAligned(const void* _data, int _len);

		///
		void addUnaligned(const void* _data, int _len);

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
	class HashAdler32
	{
	public:
		///
		void begin();

		///
		void add(const void* _data, int _len);

		///
		template<typename Ty>
		void add(Ty _value);

		///
		uint32_t end();

	private:
		uint32_t m_a;
		uint32_t m_b;
	};

	///
	class HashCrc32
	{
	public:
		enum Enum
		{
			Ieee,       //!< 0xedb88320
			Castagnoli, //!< 0x82f63b78
			Koopman,    //!< 0xeb31d82e

			Count
		};

		///
		void begin(Enum _type = Ieee);

		///
		void add(const void* _data, int _len);

		///
		template<typename Ty>
		void add(Ty _value);

		///
		uint32_t end();

	private:
		const uint32_t* m_table;
		uint32_t m_hash;
	};

	///
	template<typename HashT>
	uint32_t hash(const void* _data, uint32_t _size);

	///
	template<typename HashT, typename Ty>
	uint32_t hash(const Ty& _data);

	///
	template<typename HashT>
	uint32_t hash(const StringView& _data);

	///
	template<typename HashT>
	uint32_t hash(const char* _data);

} // namespace bx

#include "inline/hash.inl"

#endif // BX_HASH_H_HEADER_GUARD
