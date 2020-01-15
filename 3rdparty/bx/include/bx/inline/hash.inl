/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_HASH_H_HEADER_GUARD
#	error "Must be included from bx/hash.h!"
#endif // BX_HASH_H_HEADER_GUARD

namespace bx
{
#define MURMUR_M 0x5bd1e995
#define MURMUR_R 24
#define mmix(_h, _k) { _k *= MURMUR_M; _k ^= _k >> MURMUR_R; _k *= MURMUR_M; _h *= MURMUR_M; _h ^= _k; }

	inline void HashMurmur2A::begin(uint32_t _seed)
	{
		m_hash = _seed;
		m_tail = 0;
		m_count = 0;
		m_size = 0;
	}

	inline void HashMurmur2A::add(const void* _data, int _len)
	{
		if (BX_UNLIKELY(!isAligned(_data, 4) ) )
		{
			addUnaligned(_data, _len);
			return;
		}

		addAligned(_data, _len);
	}

	inline void HashMurmur2A::addAligned(const void* _data, int _len)
	{
		const uint8_t* data = (const uint8_t*)_data;
		m_size += _len;

		mixTail(data, _len);

		while(_len >= 4)
		{
			uint32_t kk = *(uint32_t*)data;

			mmix(m_hash, kk);

			data += 4;
			_len -= 4;
		}

		mixTail(data, _len);
	}

	inline void HashMurmur2A::addUnaligned(const void* _data, int _len)
	{
		const uint8_t* data = (const uint8_t*)_data;
		m_size += _len;

		mixTail(data, _len);

		while(_len >= 4)
		{
			uint32_t kk;
			readUnaligned(data, kk);

			mmix(m_hash, kk);

			data += 4;
			_len -= 4;
		}

		mixTail(data, _len);
	}

	template<typename Ty>
	inline void HashMurmur2A::add(Ty _value)
	{
		add(&_value, sizeof(Ty) );
	}

	inline uint32_t HashMurmur2A::end()
	{
		mmix(m_hash, m_tail);
		mmix(m_hash, m_size);

		m_hash ^= m_hash >> 13;
		m_hash *= MURMUR_M;
		m_hash ^= m_hash >> 15;

		return m_hash;
	}

	inline void HashMurmur2A::readUnaligned(const void* _data, uint32_t& _out)
	{
		const uint8_t* data = (const uint8_t*)_data;
		if (BX_ENABLED(BX_CPU_ENDIAN_BIG) )
		{
			_out = 0
				| data[0]<<24
				| data[1]<<16
				| data[2]<<8
				| data[3]
				;
		}
		else
		{
			_out = 0
				| data[0]
				| data[1]<<8
				| data[2]<<16
				| data[3]<<24
				;
		}
	}

	inline void HashMurmur2A::mixTail(const uint8_t*& _data, int& _len)
	{
		while( _len && ((_len<4) || m_count) )
		{
			m_tail |= (*_data++) << (m_count * 8);

			m_count++;
			_len--;

			if(m_count == 4)
			{
				mmix(m_hash, m_tail);
				m_tail = 0;
				m_count = 0;
			}
		}
	}

#undef MURMUR_M
#undef MURMUR_R
#undef mmix

	inline void HashAdler32::begin()
	{
		m_a = 1;
		m_b = 0;
	}

	inline void HashAdler32::add(const void* _data, int _len)
	{
		const uint32_t kModAdler = 65521;
		const uint8_t* data = (const uint8_t*)_data;
		for (; _len != 0; --_len)
		{
			m_a = (m_a + *data++) % kModAdler;
			m_b = (m_b + m_a    ) % kModAdler;
		}
	}

	template<typename Ty>
	inline void HashAdler32::add(Ty _value)
	{
		add(&_value, sizeof(Ty) );
	}

	inline uint32_t HashAdler32::end()
	{
		return m_a | (m_b<<16);
	}

	template<typename Ty>
	inline void HashCrc32::add(Ty _value)
	{
		add(&_value, sizeof(Ty) );
	}

	inline uint32_t HashCrc32::end()
	{
		m_hash ^= UINT32_MAX;
		return m_hash;
	}

	template<typename HashT>
	inline uint32_t hash(const void* _data, uint32_t _size)
	{
		HashT hh;
		hh.begin();
		hh.add(_data, (int)_size);
		return hh.end();
	}

	template<typename HashT, typename Ty>
	inline uint32_t hash(const Ty& _data)
	{
		BX_STATIC_ASSERT(isTriviallyCopyable<Ty>() );
		return hash<HashT>(&_data, sizeof(Ty) );
	}

	template<typename HashT>
	inline uint32_t hash(const StringView& _data)
	{
		return hash<HashT>(_data.getPtr(), _data.getLength() );
	}

	template<typename HashT>
	inline uint32_t hash(const char* _data)
	{
		return hash<HashT>(StringView(_data) );
	}

} // namespace bx
