/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_HASH_H_HEADER_GUARD
#	error "Must be included from bx/hash.h!"
#endif // BX_HASH_H_HEADER_GUARD

namespace bx
{
	inline void HashAdler32::begin()
	{
		m_a = 1;
		m_b = 0;
	}

	inline void HashAdler32::add(const void* _data, int32_t _len)
	{
		constexpr uint32_t kModAdler = 65521;

		const uint8_t* data = (const uint8_t*)_data;
		for (; _len != 0; --_len)
		{
			m_a = (m_a + *data++) % kModAdler;
			m_b = (m_b + m_a    ) % kModAdler;
		}
	}

	inline void HashAdler32::add(const char* _data)
	{
		return add(StringView(_data) );
	}

	inline void HashAdler32::add(const StringView& _data)
	{
		return add(_data.getPtr(), _data.getLength() );
	}

	template<typename Ty>
	inline void HashAdler32::add(const Ty& _data)
	{
		add(&_data, sizeof(Ty) );
	}

	inline uint32_t HashAdler32::end()
	{
		return m_a | (m_b<<16);
	}

	inline void HashCrc32::add(const char* _data)
	{
		return add(StringView(_data) );
	}

	inline void HashCrc32::add(const StringView& _data)
	{
		return add(_data.getPtr(), _data.getLength() );
	}

	template<typename Ty>
	inline void HashCrc32::add(const Ty& _data)
	{
		add(&_data, sizeof(Ty) );
	}

	inline uint32_t HashCrc32::end()
	{
		m_hash ^= UINT32_MAX;
		return m_hash;
	}

	inline void HashMurmur2A::begin(uint32_t _seed)
	{
		BX_UNUSED(m_tail);
		m_hash  = _seed;
		m_size  = 0;
		m_count = 0;
	}

	inline void HashMurmur2A::add(const char* _data)
	{
		return add(StringView(_data) );
	}

	inline void HashMurmur2A::add(const StringView& _data)
	{
		return add(_data.getPtr(), _data.getLength() );
	}

	template<typename Ty>
	inline void HashMurmur2A::add(const Ty& _data)
	{
		add(&_data, sizeof(Ty) );
	}

	inline void HashMurmur3::begin(uint32_t _seed)
	{
		BX_UNUSED(m_tail);
		m_hash  = _seed;
		m_size  = 0;
		m_count = 0;
	}

	inline void HashMurmur3::add(const char* _data)
	{
		return add(StringView(_data) );
	}

	inline void HashMurmur3::add(const StringView& _data)
	{
		return add(_data.getPtr(), _data.getLength() );
	}

	template<typename Ty>
	inline void HashMurmur3::add(const Ty& _data)
	{
		add(&_data, sizeof(Ty) );
	}

	template<typename HashT>
	inline uint32_t hash(const void* _data, uint32_t _size)
	{
		HashT hh;
		hh.begin();
		hh.add(_data, (int32_t)_size);
		return hh.end();
	}

	template<typename HashT>
	inline uint32_t hash(const char* _data)
	{
		return hash<HashT>(StringView(_data) );
	}

	template<typename HashT>
	inline uint32_t hash(const StringView& _data)
	{
		return hash<HashT>(_data.getPtr(), _data.getLength() );
	}

	template<typename HashT, typename Ty>
	inline uint32_t hash(const Ty& _data)
	{
		BX_STATIC_ASSERT(isTriviallyCopyable<Ty>() );
		return hash<HashT>(&_data, sizeof(Ty) );
	}

} // namespace bx
