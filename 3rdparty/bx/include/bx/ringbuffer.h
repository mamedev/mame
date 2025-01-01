/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_RINGBUFFER_H_HEADER_GUARD
#define BX_RINGBUFFER_H_HEADER_GUARD

#include "bx.h"
#include "cpu.h"
#include "uint32_t.h"

namespace bx
{
	///
	class RingBufferControl
	{
		BX_CLASS(RingBufferControl
			, NO_DEFAULT_CTOR
			, NO_COPY
			);

	public:
		///
		RingBufferControl(uint32_t _size);

		///
		~RingBufferControl();

		///
		uint32_t available() const;

		///
		uint32_t consume(uint32_t _size); // consumer only

		///
		uint32_t reserve(uint32_t _size, bool _mustSucceed = false); // producer only

		///
		uint32_t commit(uint32_t _size); // producer only

		///
		uint32_t distance(uint32_t _from, uint32_t _to) const; // both

		///
		void reset();

		const uint32_t m_size;
		uint32_t m_current;
		uint32_t m_write;
		uint32_t m_read;
	};

	///
	class SpScRingBufferControl
	{
		BX_CLASS(SpScRingBufferControl
			, NO_DEFAULT_CTOR
			, NO_COPY
			);

	public:
		///
		SpScRingBufferControl(uint32_t _size);

		///
		~SpScRingBufferControl();

		///
		uint32_t available() const;

		///
		uint32_t consume(uint32_t _size); // consumer only

		///
		uint32_t reserve(uint32_t _size); // producer only

		///
		uint32_t commit(uint32_t _size); // producer only

		///
		uint32_t distance(uint32_t _from, uint32_t _to) const; // both

		///
		void reset();

		const uint32_t m_size;
		uint32_t m_current;
		uint32_t m_write;
		uint32_t m_read;
	};

	///
	template <typename ControlT>
	class ReadRingBufferT
	{
		BX_CLASS(ReadRingBufferT
			, NO_DEFAULT_CTOR
			, NO_COPY
			);

	public:
		///
		ReadRingBufferT(ControlT& _control, const char* _buffer, uint32_t _size);

		///
		~ReadRingBufferT();

		///
		void end();

		///
		void read(char* _data, uint32_t _len);

		///
		void skip(uint32_t _len);

	private:
		template <typename Ty>
		friend class WriteRingBufferT;

		ControlT& m_control;
		uint32_t m_read;
		uint32_t m_end;
		const uint32_t m_size;
		const char* m_buffer;
	};

	///
	typedef ReadRingBufferT<RingBufferControl> ReadRingBuffer;

	///
	typedef ReadRingBufferT<SpScRingBufferControl> SpScReadRingBuffer;

	///
	template <typename ControlT>
	class WriteRingBufferT
	{
		BX_CLASS(WriteRingBufferT
			, NO_DEFAULT_CTOR
			, NO_COPY
			);

	public:
		///
		WriteRingBufferT(ControlT& _control, char* _buffer, uint32_t _size);

		///
		~WriteRingBufferT();

		///
		void end();

		///
		void write(const char* _data, uint32_t _len);

		///
		void write(ReadRingBufferT<ControlT>& _read, uint32_t _len);

		///
		void skip(uint32_t _len);

	private:
		ControlT& m_control;
		uint32_t m_write;
		uint32_t m_end;
		const uint32_t m_size;
		char* m_buffer;
	};

	///
	typedef WriteRingBufferT<RingBufferControl> WriteRingBuffer;

	///
	typedef WriteRingBufferT<SpScRingBufferControl> SpScWriteRingBuffer;

} // namespace bx

#include "inline/ringbuffer.inl"

#endif // BX_RINGBUFFER_H_HEADER_GUARD
