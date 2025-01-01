/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_HASH_H_HEADER_GUARD
#define BX_HASH_H_HEADER_GUARD

#include "allocator.h" // isAligned
#include "string.h" // StringView

namespace bx
{
	/// 32-bit Adler checksum hash.
	class HashAdler32
	{
	public:
		///
		void begin();

		///
		void add(const void* _data, int32_t _len);

		///
		void add(const char* _data);

		///
		void add(const StringView& _data);

		///
		template<typename Ty>
		void add(const Ty& _data);

		///
		uint32_t end();

	private:
		uint32_t m_a;
		uint32_t m_b;
	};

	/// 32-bit cyclic redundancy checksum hash.
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
		void add(const void* _data, int32_t _len);

		///
		void add(const char* _data);

		///
		void add(const StringView& _data);

		///
		template<typename Ty>
		void add(const Ty& _data);

		///
		uint32_t end();

	private:
		const uint32_t* m_table;
		uint32_t m_hash;
	};

	/// 32-bit non-cryptographic multiply and rotate hash.
	class HashMurmur2A
	{
	public:
		///
		void begin(uint32_t _seed = 0);

		///
		void add(const void* _data, int32_t _len);

		///
		void add(const char* _data);

		///
		void add(const StringView& _data);

		///
		template<typename Ty>
		void add(const Ty& _data);

		///
		uint32_t end();

	private:
		uint32_t m_hash;
		uint32_t m_size;
		uint8_t  m_tail[4];
		uint8_t  m_count;
	};

	/// 32-bit non-cryptographic multiply and rotate hash.
	class HashMurmur3
	{
	public:
		///
		void begin(uint32_t _seed = 0);

		///
		void add(const void* _data, int32_t _len);

		///
		void add(const char* _data);

		///
		void add(const StringView& _data);

		///
		template<typename Ty>
		void add(const Ty& _data);

		///
		uint32_t end();

	private:
		uint32_t m_hash;
		uint32_t m_size;
		uint8_t  m_tail[4];
		uint8_t  m_count;
	};

	///
	template<typename HashT>
	uint32_t hash(const void* _data, uint32_t _size);

	///
	template<typename HashT>
	uint32_t hash(const char* _data);

	///
	template<typename HashT>
	uint32_t hash(const StringView& _data);

	///
	template<typename HashT, typename Ty>
	uint32_t hash(const Ty& _data);

} // namespace bx

#include "inline/hash.inl"

#endif // BX_HASH_H_HEADER_GUARD
