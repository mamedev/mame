/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_READERWRITER_H_HEADER_GUARD
#define BX_READERWRITER_H_HEADER_GUARD

#include <stdarg.h> // va_list
#include <stdio.h>
#include <string.h>

#include "bx.h"
#include "allocator.h"
#include "uint32_t.h"

#if BX_COMPILER_MSVC_COMPATIBLE
#	define fseeko64 _fseeki64
#	define ftello64 _ftelli64
#elif BX_PLATFORM_ANDROID || BX_PLATFORM_BSD || BX_PLATFORM_IOS || BX_PLATFORM_OSX || BX_PLATFORM_QNX
#	define fseeko64 fseeko
#	define ftello64 ftello
#endif // BX_

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
		virtual int32_t read(void* _data, int32_t _size) = 0;
	};

	inline ReaderI::~ReaderI()
	{
	}

	struct BX_NO_VTABLE WriterI
	{
		virtual ~WriterI() = 0;
		virtual int32_t write(const void* _data, int32_t _size) = 0;
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
	inline int32_t read(ReaderI* _reader, void* _data, int32_t _size)
	{
		return _reader->read(_data, _size);
	}

	/// Write value.
	template<typename Ty>
	inline int32_t read(ReaderI* _reader, Ty& _value)
	{
		BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty) );
		return _reader->read(&_value, sizeof(Ty) );
	}

	/// Read value and converts it to host endianess. _fromLittleEndian specifies
	/// underlying stream endianess.
	template<typename Ty>
	inline int32_t readHE(ReaderI* _reader, Ty& _value, bool _fromLittleEndian)
	{
		BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty) );
		Ty value;
		int32_t result = _reader->read(&value, sizeof(Ty) );
		_value = toHostEndian(value, _fromLittleEndian);
		return result;
	}

	/// Write data.
	inline int32_t write(WriterI* _writer, const void* _data, int32_t _size)
	{
		return _writer->write(_data, _size);
	}

	/// Write repeat the same value.
	inline int32_t writeRep(WriterI* _writer, uint8_t _byte, int32_t _size)
	{
		const uint32_t tmp0      = uint32_sels(64   - _size,   64, _size);
		const uint32_t tmp1      = uint32_sels(256  - _size,  256, tmp0);
		const uint32_t blockSize = uint32_sels(1024 - _size, 1024, tmp1);
		uint8_t* temp = (uint8_t*)alloca(blockSize);
		memset(temp, _byte, blockSize);

		int32_t size = 0;
		while (0 < _size)
		{
			int32_t bytes = write(_writer, temp, uint32_min(blockSize, _size) );
			size  += bytes;
			_size -= bytes;
		}

		return size;
	}

	/// Write value.
	template<typename Ty>
	inline int32_t write(WriterI* _writer, const Ty& _value)
	{
		BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty) );
		return _writer->write(&_value, sizeof(Ty) );
	}

	/// Write value as little endian.
	template<typename Ty>
	inline int32_t writeLE(WriterI* _writer, const Ty& _value)
	{
		BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty) );
		Ty value = toLittleEndian(_value);
		int32_t result = _writer->write(&value, sizeof(Ty) );
		return result;
	}

	/// Write value as big endian.
	template<typename Ty>
	inline int32_t writeBE(WriterI* _writer, const Ty& _value)
	{
		BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty) );
		Ty value = toBigEndian(_value);
		int32_t result = _writer->write(&value, sizeof(Ty) );
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

	struct BX_NO_VTABLE WriterSeekerI : public WriterI, public SeekerI
	{
	};

	struct BX_NO_VTABLE FileReaderI : public ReaderSeekerI
	{
		virtual int32_t open(const char* _filePath) = 0;
		virtual int32_t close() = 0;
	};

	struct BX_NO_VTABLE FileWriterI : public WriterSeekerI
	{
		virtual int32_t open(const char* _filePath, bool _append = false) = 0;
		virtual int32_t close() = 0;
	};

	inline int32_t open(FileReaderI* _reader, const char* _filePath)
	{
		return _reader->open(_filePath);
	}

	inline int32_t close(FileReaderI* _reader)
	{
		return _reader->close();
	}

	inline int32_t open(FileWriterI* _writer, const char* _filePath, bool _append = false)
	{
		return _writer->open(_filePath, _append);
	}

	inline int32_t close(FileWriterI* _writer)
	{
		return _writer->close();
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

		virtual int32_t write(const void* /*_data*/, int32_t _size) BX_OVERRIDE
		{
			int32_t morecore = int32_t(m_pos - m_top) + _size;

			if (0 < morecore)
			{
				m_top += morecore;
			}

			int64_t reminder = m_top-m_pos;
			int32_t size = uint32_min(_size, int32_t(reminder > INT32_MAX ? INT32_MAX : reminder) );
			m_pos += size;
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

		virtual int32_t read(void* _data, int32_t _size) BX_OVERRIDE
		{
			int64_t reminder = m_top-m_pos;
			int32_t size = uint32_min(_size, int32_t(reminder > INT32_MAX ? INT32_MAX : reminder) );
			memcpy(_data, &m_data[m_pos], size);
			m_pos += size;
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

		virtual int32_t write(const void* _data, int32_t _size) BX_OVERRIDE
		{
			int32_t morecore = int32_t(m_pos - m_size) + _size;

			if (0 < morecore)
			{
				morecore = BX_ALIGN_MASK(morecore, 0xfff);
				m_data = (uint8_t*)m_memBlock->more(morecore);
				m_size = m_memBlock->getSize();
			}

			int64_t reminder = m_size-m_pos;
			int32_t size = uint32_min(_size, int32_t(reminder > INT32_MAX ? INT32_MAX : reminder) );
			memcpy(&m_data[m_pos], _data, size);
			m_pos += size;
			m_top = int64_max(m_top, m_pos);
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

#if BX_CONFIG_CRT_FILE_READER_WRITER
	class CrtFileReader : public FileReaderI
	{
	public:
		CrtFileReader()
			: m_file(NULL)
		{
		}

		virtual ~CrtFileReader()
		{
		}

		virtual int32_t open(const char* _filePath) BX_OVERRIDE
		{
			m_file = fopen(_filePath, "rb");
			return NULL == m_file;
		}

		virtual int32_t close() BX_OVERRIDE
		{
			fclose(m_file);
			return 0;
		}

		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) BX_OVERRIDE
		{
			fseeko64(m_file, _offset, _whence);
			return ftello64(m_file);
		}

		virtual int32_t read(void* _data, int32_t _size) BX_OVERRIDE
		{
			return (int32_t)fread(_data, 1, _size, m_file);
		}

	private:
		FILE* m_file;
	};

	class CrtFileWriter : public FileWriterI
	{
	public:
		CrtFileWriter()
			: m_file(NULL)
		{
		}

		virtual ~CrtFileWriter()
		{
		}

		virtual int32_t open(const char* _filePath, bool _append = false) BX_OVERRIDE
		{
			if (_append)
			{
				m_file = fopen(_filePath, "ab");
			}
			else
			{
				m_file = fopen(_filePath, "wb");
			}

			return NULL == m_file;
		}

		virtual int32_t close() BX_OVERRIDE
		{
			fclose(m_file);
			return 0;
		}

		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) BX_OVERRIDE
		{
			fseeko64(m_file, _offset, _whence);
			return ftello64(m_file);
		}

		virtual int32_t write(const void* _data, int32_t _size) BX_OVERRIDE
		{
			return (int32_t)fwrite(_data, 1, _size, m_file);
		}

	private:
		FILE* m_file;
	};
#endif // BX_CONFIG_CRT_FILE_READER_WRITER

} // namespace bx

#endif // BX_READERWRITER_H_HEADER_GUARD
