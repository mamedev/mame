/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_RINGBUFFER_H_HEADER_GUARD
#	error "Must be included from bx/ringbuffer.h!"
#endif // BX_RINGBUFFER_H_HEADER_GUARD

namespace bx
{
	inline RingBufferControl::RingBufferControl(uint32_t _size)
		: m_size(_size)
		, m_current(0)
		, m_write(0)
		, m_read(0)
	{
	}

	inline RingBufferControl::~RingBufferControl()
	{
	}

	inline uint32_t RingBufferControl::available() const
	{
		return distance(m_read, m_current);
	}

	inline uint32_t RingBufferControl::consume(uint32_t _size)
	{
		const uint32_t maxSize    = distance(m_read, m_current);
		const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
		const uint32_t test       = uint32_sub(sizeNoSign, maxSize);
		const uint32_t size       = uint32_sels(test, _size, maxSize);
		const uint32_t advance    = uint32_add(m_read, size);
		const uint32_t read       = uint32_mod(advance, m_size);
		m_read = read;
		return size;
	}

	inline uint32_t RingBufferControl::reserve(uint32_t _size, bool _mustSucceed)
	{
		const uint32_t dist       = distance(m_write, m_read)-1;
		const uint32_t maxSize    = uint32_sels(dist, m_size-1, dist);
		const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
		const uint32_t test       = uint32_sub(sizeNoSign, maxSize);
		const uint32_t size       = uint32_sels(test, _size, _mustSucceed ? 0 : maxSize);
		const uint32_t advance    = uint32_add(m_write, size);
		const uint32_t write      = uint32_mod(advance, m_size);
		m_write = write;
		return size;
	}

	inline uint32_t RingBufferControl::commit(uint32_t _size)
	{
		const uint32_t maxSize    = distance(m_current, m_write);
		const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
		const uint32_t test       = uint32_sub(sizeNoSign, maxSize);
		const uint32_t size       = uint32_sels(test, _size, maxSize);
		const uint32_t advance    = uint32_add(m_current, size);
		const uint32_t current    = uint32_mod(advance, m_size);
		m_current = current;
		return size;
	}

	inline uint32_t RingBufferControl::distance(uint32_t _from, uint32_t _to) const
	{
		const uint32_t diff   = uint32_sub(_to, _from);
		const uint32_t le     = uint32_add(m_size, diff);
		const uint32_t result = uint32_sels(diff, le, diff);

		return result;
	}

	inline void RingBufferControl::reset()
	{
		m_current = 0;
		m_write   = 0;
		m_read    = 0;
	}

	inline SpScRingBufferControl::SpScRingBufferControl(uint32_t _size)
		: m_size(_size)
		, m_current(0)
		, m_write(0)
		, m_read(0)
	{
	}

	inline SpScRingBufferControl::~SpScRingBufferControl()
	{
	}

	inline uint32_t SpScRingBufferControl::available() const
	{
		return distance(m_read, m_current);
	}

	inline uint32_t SpScRingBufferControl::consume(uint32_t _size)
	{
		const uint32_t maxSize    = distance(m_read, m_current);
		const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
		const uint32_t test       = uint32_sub(sizeNoSign, maxSize);
		const uint32_t size       = uint32_sels(test, _size, maxSize);
		const uint32_t advance    = uint32_add(m_read, size);
		const uint32_t read       = uint32_mod(advance, m_size);
		m_read = read;
		return size;
	}

	inline uint32_t SpScRingBufferControl::reserve(uint32_t _size)
	{
		const uint32_t dist       = distance(m_write, m_read)-1;
		const uint32_t maxSize    = uint32_sels(dist, m_size-1, dist);
		const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
		const uint32_t test       = uint32_sub(sizeNoSign, maxSize);
		const uint32_t size       = uint32_sels(test, _size, maxSize);
		const uint32_t advance    = uint32_add(m_write, size);
		const uint32_t write      = uint32_mod(advance, m_size);
		m_write = write;
		return size;
	}

	inline uint32_t SpScRingBufferControl::commit(uint32_t _size)
	{
		const uint32_t maxSize    = distance(m_current, m_write);
		const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
		const uint32_t test       = uint32_sub(sizeNoSign, maxSize);
		const uint32_t size       = uint32_sels(test, _size, maxSize);
		const uint32_t advance    = uint32_add(m_current, size);
		const uint32_t current    = uint32_mod(advance, m_size);

		// must commit all memory writes before moving m_current pointer
		// once m_current pointer moves data is used by consumer thread
		memoryBarrier();
		m_current = current;
		return size;
	}

	inline uint32_t SpScRingBufferControl::distance(uint32_t _from, uint32_t _to) const
	{
		const uint32_t diff   = uint32_sub(_to, _from);
		const uint32_t le     = uint32_add(m_size, diff);
		const uint32_t result = uint32_sels(diff, le, diff);

		return result;
	}

	inline void SpScRingBufferControl::reset()
	{
		m_current = 0;
		m_write   = 0;
		m_read    = 0;
	}

	template <typename ControlT>
	inline ReadRingBufferT<ControlT>::ReadRingBufferT(ControlT& _control, const char* _buffer, uint32_t _size)
		: m_control(_control)
		, m_read(_control.m_read)
		, m_end(m_read+_size)
		, m_size(_size)
		, m_buffer(_buffer)
	{
		BX_CHECK(_control.available() >= _size, "%d >= %d", _control.available(), _size);
	}

	template <typename ControlT>
	inline ReadRingBufferT<ControlT>::~ReadRingBufferT()
	{
	}

	template <typename ControlT>
	inline void ReadRingBufferT<ControlT>::end()
	{
		m_control.consume(m_size);
	}

	template <typename ControlT>
	inline void ReadRingBufferT<ControlT>::read(char* _data, uint32_t _len)
	{
		const uint32_t eof = (m_read + _len) % m_control.m_size;
		uint32_t wrap = 0;
		const char* from = &m_buffer[m_read];

		if (eof < m_read)
		{
			wrap = m_control.m_size - m_read;
			memCopy(_data, from, wrap);
			_data += wrap;
			from = (const char*)&m_buffer[0];
		}

		memCopy(_data, from, _len-wrap);

		m_read = eof;
	}

	template <typename ControlT>
	inline void ReadRingBufferT<ControlT>::skip(uint32_t _len)
	{
		m_read += _len;
		m_read %= m_control.m_size;
	}

	template <typename ControlT>
	inline WriteRingBufferT<ControlT>::WriteRingBufferT(ControlT& _control, char* _buffer, uint32_t _size)
		: m_control(_control)
		, m_size(_size)
		, m_buffer(_buffer)
	{
		uint32_t size = m_control.reserve(_size);
		BX_UNUSED(size);
		BX_CHECK(size == _size, "%d == %d", size, _size);
		m_write = m_control.m_current;
		m_end = m_write+_size;
	}

	template <typename ControlT>
	inline WriteRingBufferT<ControlT>::~WriteRingBufferT()
	{
	}

	template <typename ControlT>
	inline void WriteRingBufferT<ControlT>::end()
	{
		m_control.commit(m_size);
	}

	template <typename ControlT>
	inline void WriteRingBufferT<ControlT>::write(const char* _data, uint32_t _len)
	{
		const uint32_t eof = (m_write + _len) % m_control.m_size;
		uint32_t wrap = 0;
		char* to = &m_buffer[m_write];

		if (eof < m_write)
		{
			wrap = m_control.m_size - m_write;
			memCopy(to, _data, wrap);
			_data += wrap;
			to = (char*)&m_buffer[0];
		}

		memCopy(to, _data, _len-wrap);

		m_write = eof;
	}

	template <typename ControlT>
	inline void WriteRingBufferT<ControlT>::write(ReadRingBufferT<ControlT>& _read, uint32_t _len)
	{
		const uint32_t eof = (_read.m_read + _len) % _read.m_control.m_size;
		uint32_t wrap = 0;
		const char* from = &_read.m_buffer[_read.m_read];

		if (eof < _read.m_read)
		{
			wrap = _read.m_control.m_size - _read.m_read;
			write(from, wrap);
			from = (const char*)&_read.m_buffer[0];
		}

		write(from, _len-wrap);

		_read.m_read = eof;
	}

	template <typename ControlT>
	inline void WriteRingBufferT<ControlT>::skip(uint32_t _len)
	{
		m_write += _len;
		m_write %= m_control.m_size;
	}

} // namespace bx
