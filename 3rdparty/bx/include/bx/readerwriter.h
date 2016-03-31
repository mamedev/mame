/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_READERWRITER_H_HEADER_GUARD
#define BX_READERWRITER_H_HEADER_GUARD

#include <alloca.h>
#include <stdarg.h> // va_list
#include <stdio.h>
#include <string.h>

#include "bx.h"
#include "allocator.h"
#include "error.h"
#include "uint32_t.h"

#if BX_COMPILER_MSVC_COMPATIBLE
#	define fseeko64 _fseeki64
#	define ftello64 _ftelli64
#elif BX_PLATFORM_ANDROID || BX_PLATFORM_BSD || BX_PLATFORM_IOS || BX_PLATFORM_OSX || BX_PLATFORM_QNX
#	define fseeko64 fseeko
#	define ftello64 ftello
#endif // BX_

BX_ERROR_RESULT(BX_ERROR_READERWRITER_OPEN,  BX_MAKEFOURCC('R', 'W', 0, 1) );
BX_ERROR_RESULT(BX_ERROR_READERWRITER_READ,  BX_MAKEFOURCC('R', 'W', 0, 2) );
BX_ERROR_RESULT(BX_ERROR_READERWRITER_WRITE, BX_MAKEFOURCC('R', 'W', 0, 3) );

namespace bx
{
	struct Whence
	{
		enum Enum
		{
			Begin,
			Current,
			End,
		};
	};

	struct BX_NO_VTABLE ReaderI
	{
		virtual ~ReaderI() = 0;
		virtual int32_t read(void* _data, int32_t _size, Error* _err) = 0;
	};

	inline ReaderI::~ReaderI()
	{
	}

	struct BX_NO_VTABLE WriterI
	{
		virtual ~WriterI() = 0;
		virtual int32_t write(const void* _data, int32_t _size, Error* _err) = 0;
	};

	inline WriterI::~WriterI()
	{
	}

	struct BX_NO_VTABLE SeekerI
	{
		virtual ~SeekerI() = 0;
		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) = 0;
	};

	inline SeekerI::~SeekerI()
	{
	}

	/// Read data.
	inline int32_t read(ReaderI* _reader, void* _data, int32_t _size, Error* _err = NULL)
	{
		BX_ERROR_SCOPE(_err);
		return _reader->read(_data, _size, _err);
	}

	/// Read value.
	template<typename Ty>
	inline int32_t read(ReaderI* _reader, Ty& _value, Error* _err = NULL)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty) );
		return _reader->read(&_value, sizeof(Ty), _err);
	}

	/// Read value and converts it to host endianess. _fromLittleEndian specifies
	/// underlying stream endianess.
	template<typename Ty>
	inline int32_t readHE(ReaderI* _reader, Ty& _value, bool _fromLittleEndian, Error* _err = NULL)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty) );
		Ty value;
		int32_t result = _reader->read(&value, sizeof(Ty), _err);
		_value = toHostEndian(value, _fromLittleEndian);
		return result;
	}

	/// Write data.
	inline int32_t write(WriterI* _writer, const void* _data, int32_t _size, Error* _err = NULL)
	{
		BX_ERROR_SCOPE(_err);
		return _writer->write(_data, _size, _err);
	}

	/// Write repeat the same value.
	inline int32_t writeRep(WriterI* _writer, uint8_t _byte, int32_t _size, Error* _err = NULL)
	{
		BX_ERROR_SCOPE(_err);

		const uint32_t tmp0      = uint32_sels(64   - _size,   64, _size);
		const uint32_t tmp1      = uint32_sels(256  - _size,  256, tmp0);
		const uint32_t blockSize = uint32_sels(1024 - _size, 1024, tmp1);
		uint8_t* temp = (uint8_t*)alloca(blockSize);
		memset(temp, _byte, blockSize);

		int32_t size = 0;
		while (0 < _size)
		{
			int32_t bytes = write(_writer, temp, uint32_min(blockSize, _size), _err);
			size  += bytes;
			_size -= bytes;
		}

		return size;
	}

	/// Write value.
	template<typename Ty>
	inline int32_t write(WriterI* _writer, const Ty& _value, Error* _err = NULL)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty) );
		return _writer->write(&_value, sizeof(Ty), _err);
	}

	/// Write value as little endian.
	template<typename Ty>
	inline int32_t writeLE(WriterI* _writer, const Ty& _value, Error* _err = NULL)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty) );
		Ty value = toLittleEndian(_value);
		int32_t result = _writer->write(&value, sizeof(Ty), _err);
		return result;
	}

	/// Write value as big endian.
	template<typename Ty>
	inline int32_t writeBE(WriterI* _writer, const Ty& _value, Error* _err = NULL)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty) );
		Ty value = toBigEndian(_value);
		int32_t result = _writer->write(&value, sizeof(Ty), _err);
		return result;
	}

	/// Write formated string.
	inline int32_t writePrintf(WriterI* _writer, const char* _format, ...)
	{
		va_list argList;
		va_start(argList, _format);

		char temp[2048];
		char* out = temp;
		int32_t max = sizeof(temp);
		int32_t len = vsnprintf(out, max, _format, argList);
		if (len > max)
		{
			out = (char*)alloca(len);
			len = vsnprintf(out, len, _format, argList);
		}

		int32_t size = write(_writer, out, len);

		va_end(argList);

		return size;
	}

	/// Skip _offset bytes forward.
	inline int64_t skip(SeekerI* _seeker, int64_t _offset)
	{
		return _seeker->seek(_offset, Whence::Current);
	}

	/// Seek to any position in file.
	inline int64_t seek(SeekerI* _seeker, int64_t _offset = 0, Whence::Enum _whence = Whence::Current)
	{
		return _seeker->seek(_offset, _whence);
	}

	/// Returns size of file.
	inline int64_t getSize(SeekerI* _seeker)
	{
		int64_t offset = _seeker->seek();
		int64_t size = _seeker->seek(0, Whence::End);
		_seeker->seek(offset, Whence::Begin);
		return size;
	}

	struct BX_NO_VTABLE ReaderSeekerI : public ReaderI, public SeekerI
	{
	};

	/// Peek data.
	inline int32_t peek(ReaderSeekerI* _reader, void* _data, int32_t _size, Error* _err = NULL)
	{
		BX_ERROR_SCOPE(_err);
		int64_t offset = bx::seek(_reader);
		int32_t size = _reader->read(_data, _size, _err);
		bx::seek(_reader, offset, bx::Whence::Begin);
		return size;
	}

	/// Peek value.
	template<typename Ty>
	inline int32_t peek(ReaderSeekerI* _reader, Ty& _value, Error* _err = NULL)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty) );
		return peek(_reader, &_value, sizeof(Ty), _err);
	}

	struct BX_NO_VTABLE WriterSeekerI : public WriterI, public SeekerI
	{
	};

	struct BX_NO_VTABLE ReaderOpenI
	{
		virtual ~ReaderOpenI() = 0;
		virtual bool open(const char* _filePath, Error* _err) = 0;
	};

	inline ReaderOpenI::~ReaderOpenI()
	{
	}

	struct BX_NO_VTABLE WriterOpenI
	{
		virtual ~WriterOpenI() = 0;
		virtual bool open(const char* _filePath, bool _append, Error* _err) = 0;
	};

	inline WriterOpenI::~WriterOpenI()
	{
	}

	struct BX_NO_VTABLE CloserI
	{
		virtual ~CloserI() = 0;
		virtual void close() = 0;
	};

	inline CloserI::~CloserI()
	{
	}

	struct BX_NO_VTABLE FileReaderI : public ReaderOpenI, public CloserI, public ReaderSeekerI
	{
	};

	struct BX_NO_VTABLE FileWriterI : public WriterOpenI, public CloserI, public WriterSeekerI
	{
	};

	inline bool open(ReaderOpenI* _reader, const char* _filePath, Error* _err = NULL)
	{
		BX_ERROR_USE_TEMP_WHEN_NULL(_err);
		return _reader->open(_filePath, _err);
	}

	inline bool open(WriterOpenI* _writer, const char* _filePath, bool _append = false, Error* _err = NULL)
	{
		BX_ERROR_USE_TEMP_WHEN_NULL(_err);
		return _writer->open(_filePath, _append, _err);
	}

	inline void close(CloserI* _reader)
	{
		_reader->close();
	}

	struct BX_NO_VTABLE MemoryBlockI
	{
		virtual void* more(uint32_t _size = 0) = 0;
		virtual uint32_t getSize() = 0;
	};

	class StaticMemoryBlock : public MemoryBlockI
	{
	public:
		StaticMemoryBlock(void* _data, uint32_t _size)
			: m_data(_data)
			, m_size(_size)
		{
		}

		virtual ~StaticMemoryBlock()
		{
		}

		virtual void* more(uint32_t /*_size*/ = 0) BX_OVERRIDE
		{
			return m_data;
		}

		virtual uint32_t getSize() BX_OVERRIDE
		{
			return m_size;
		}

	private:
		void* m_data;
		uint32_t m_size;
	};

	class MemoryBlock : public MemoryBlockI
	{
	public:
		MemoryBlock(AllocatorI* _allocator)
			: m_allocator(_allocator)
			, m_data(NULL)
			, m_size(0)
		{
		}

		virtual ~MemoryBlock()
		{
			BX_FREE(m_allocator, m_data);
		}

		virtual void* more(uint32_t _size = 0) BX_OVERRIDE
		{
			if (0 < _size)
			{
				m_size += _size;
				m_data = BX_REALLOC(m_allocator, m_data, m_size);
			}

			return m_data;
		}

		virtual uint32_t getSize() BX_OVERRIDE
		{
			return m_size;
		}

	private:
		AllocatorI* m_allocator;
		void* m_data;
		uint32_t m_size;
	};

	class SizerWriter : public WriterSeekerI
	{
	public:
		SizerWriter()
			: m_pos(0)
			, m_top(0)
		{
		}

		virtual ~SizerWriter()
		{
		}

		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) BX_OVERRIDE
		{
			switch (_whence)
			{
			case Whence::Begin:
				m_pos = _offset;
				break;

			case Whence::Current:
				m_pos = int64_clamp(m_pos + _offset, 0, m_top);
				break;

			case Whence::End:
				m_pos = int64_clamp(m_top - _offset, 0, m_top);
				break;
			}

			return m_pos;
		}

		virtual int32_t write(const void* /*_data*/, int32_t _size, Error* _err) BX_OVERRIDE
		{
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int32_t morecore = int32_t(m_pos - m_top) + _size;

			if (0 < morecore)
			{
				m_top += morecore;
			}

			int64_t remainder = m_top-m_pos;
			int32_t size = uint32_min(_size, int32_t(remainder > INT32_MAX ? INT32_MAX : remainder) );
			m_pos += size;
			if (size != _size)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_WRITE, "SizerWriter: write truncated.");
			}
			return size;
		}

	private:
		int64_t m_pos;
		int64_t m_top;
	};

	class MemoryReader : public ReaderSeekerI
	{
	public:
		MemoryReader(const void* _data, uint32_t _size)
			: m_data( (const uint8_t*)_data)
			, m_pos(0)
			, m_top(_size)
		{
		}

		virtual ~MemoryReader()
		{
		}

		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) BX_OVERRIDE
		{
			switch (_whence)
			{
				case Whence::Begin:
					m_pos = _offset;
					break;

				case Whence::Current:
					m_pos = int64_clamp(m_pos + _offset, 0, m_top);
					break;

				case Whence::End:
					m_pos = int64_clamp(m_top - _offset, 0, m_top);
					break;
			}

			return m_pos;
		}

		virtual int32_t read(void* _data, int32_t _size, Error* _err) BX_OVERRIDE
		{
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int64_t remainder = m_top-m_pos;
			int32_t size = uint32_min(_size, int32_t(remainder > INT32_MAX ? INT32_MAX : remainder) );
			memcpy(_data, &m_data[m_pos], size);
			m_pos += size;
			if (size != _size)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_READ, "MemoryReader: read truncated.");
			}
			return size;
		}

		const uint8_t* getDataPtr() const
		{
			return &m_data[m_pos];
		}

		int64_t getPos() const
		{
			return m_pos;
		}

		int64_t remaining() const
		{
			return m_top-m_pos;
		}

	private:
		const uint8_t* m_data;
		int64_t m_pos;
		int64_t m_top;
	};

	class MemoryWriter : public WriterSeekerI
	{
	public:
		MemoryWriter(MemoryBlockI* _memBlock)
			: m_memBlock(_memBlock)
			, m_data(NULL)
			, m_pos(0)
			, m_top(0)
			, m_size(0)
		{
		}

		virtual ~MemoryWriter()
		{
		}

		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) BX_OVERRIDE
		{
			switch (_whence)
			{
				case Whence::Begin:
					m_pos = _offset;
					break;

				case Whence::Current:
					m_pos = int64_clamp(m_pos + _offset, 0, m_top);
					break;

				case Whence::End:
					m_pos = int64_clamp(m_top - _offset, 0, m_top);
					break;
			}

			return m_pos;
		}

		virtual int32_t write(const void* _data, int32_t _size, Error* _err) BX_OVERRIDE
		{
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int32_t morecore = int32_t(m_pos - m_size) + _size;

			if (0 < morecore)
			{
				morecore = BX_ALIGN_MASK(morecore, 0xfff);
				m_data = (uint8_t*)m_memBlock->more(morecore);
				m_size = m_memBlock->getSize();
			}

			int64_t remainder = m_size-m_pos;
			int32_t size = uint32_min(_size, int32_t(remainder > INT32_MAX ? INT32_MAX : remainder) );
			memcpy(&m_data[m_pos], _data, size);
			m_pos += size;
			m_top = int64_max(m_top, m_pos);
			if (size != _size)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_WRITE, "MemoryWriter: write truncated.");
			}
			return size;
		}

	private:
		MemoryBlockI* m_memBlock;
		uint8_t* m_data;
		int64_t m_pos;
		int64_t m_top;
		int64_t m_size;
	};

	class StaticMemoryBlockWriter : public MemoryWriter
	{
	public:
		StaticMemoryBlockWriter(void* _data, uint32_t _size)
			: MemoryWriter(&m_smb)
			, m_smb(_data, _size)
		{
		}

		~StaticMemoryBlockWriter()
		{
		}

	private:
		StaticMemoryBlock m_smb;
	};

} // namespace bx

#endif // BX_READERWRITER_H_HEADER_GUARD
