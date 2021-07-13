/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_READERWRITER_H_HEADER_GUARD
#	error "Must be included from bx/readerwriter!"
#endif // BX_READERWRITER_H_HEADER_GUARD

namespace bx
{
	inline ReaderI::~ReaderI()
	{
	}

	inline WriterI::~WriterI()
	{
	}

	inline SeekerI::~SeekerI()
	{
	}

	inline ReaderOpenI::~ReaderOpenI()
	{
	}

	inline WriterOpenI::~WriterOpenI()
	{
	}

	inline ProcessOpenI::~ProcessOpenI()
	{
	}

	inline CloserI::~CloserI()
	{
	}

	inline StaticMemoryBlock::StaticMemoryBlock(void* _data, uint32_t _size)
		: m_data(_data)
		, m_size(_size)
	{
	}

	inline StaticMemoryBlock::~StaticMemoryBlock()
	{
	}

	inline void* StaticMemoryBlock::more(uint32_t _size)
	{
		BX_UNUSED(_size);
		return m_data;
	}

	inline uint32_t StaticMemoryBlock::getSize()
	{
		return m_size;
	}

	inline MemoryBlock::MemoryBlock(AllocatorI* _allocator)
		: m_allocator(_allocator)
		, m_data(NULL)
		, m_size(0)
	{
	}

	inline MemoryBlock::~MemoryBlock()
	{
		BX_FREE(m_allocator, m_data);
	}

	inline void* MemoryBlock::more(uint32_t _size)
	{
		if (0 < _size)
		{
			m_size += _size;
			m_data = BX_REALLOC(m_allocator, m_data, m_size);
		}

		return m_data;
	}

	inline uint32_t MemoryBlock::getSize()
	{
		return m_size;
	}

	inline SizerWriter::SizerWriter()
		: m_pos(0)
		, m_top(0)
	{
	}

	inline SizerWriter::~SizerWriter()
	{
	}

	inline int64_t SizerWriter::seek(int64_t _offset, Whence::Enum _whence)
	{
		switch (_whence)
		{
			case Whence::Begin:
				m_pos = clamp<int64_t>(_offset, 0, m_top);
				break;

			case Whence::Current:
				m_pos = clamp<int64_t>(m_pos + _offset, 0, m_top);
				break;

			case Whence::End:
				m_pos = clamp<int64_t>(m_top - _offset, 0, m_top);
				break;
		}

		return m_pos;
	}

	inline int32_t SizerWriter::write(const void* /*_data*/, int32_t _size, Error* _err)
	{
		BX_ASSERT(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

		int32_t morecore = int32_t(m_pos - m_top) + _size;

		if (0 < morecore)
		{
			m_top += morecore;
		}

		int64_t remainder = m_top-m_pos;
		int32_t size = uint32_min(_size, uint32_t(min<int64_t>(remainder, INT32_MAX) ) );
		m_pos += size;
		if (size != _size)
		{
			BX_ERROR_SET(_err, kErrorReaderWriterWrite, "SizerWriter: write truncated.");
		}
		return size;
	}

	inline MemoryReader::MemoryReader(const void* _data, uint32_t _size)
		: m_data( (const uint8_t*)_data)
		, m_pos(0)
		, m_top(_size)
	{
	}

	inline MemoryReader::~MemoryReader()
	{
	}

	inline int64_t MemoryReader::seek(int64_t _offset, Whence::Enum _whence)
	{
		switch (_whence)
		{
			case Whence::Begin:
				m_pos = clamp<int64_t>(_offset, 0, m_top);
				break;

			case Whence::Current:
				m_pos = clamp<int64_t>(m_pos + _offset, 0, m_top);
				break;

			case Whence::End:
				m_pos = clamp<int64_t>(m_top - _offset, 0, m_top);
				break;
		}

		return m_pos;
	}

	inline int32_t MemoryReader::read(void* _data, int32_t _size, Error* _err)
	{
		BX_ASSERT(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

		int64_t remainder = m_top-m_pos;
		int32_t size = uint32_min(_size, uint32_t(min<int64_t>(remainder, INT32_MAX) ) );
		memCopy(_data, &m_data[m_pos], size);
		m_pos += size;
		if (size != _size)
		{
			BX_ERROR_SET(_err, kErrorReaderWriterRead, "MemoryReader: read truncated.");
		}
		return size;
	}

	inline const uint8_t* MemoryReader::getDataPtr() const
	{
		return &m_data[m_pos];
	}

	inline int64_t MemoryReader::getPos() const
	{
		return m_pos;
	}

	inline int64_t MemoryReader::remaining() const
	{
		return m_top-m_pos;
	}

	inline MemoryWriter::MemoryWriter(MemoryBlockI* _memBlock)
		: m_memBlock(_memBlock)
		, m_data(NULL)
		, m_pos(0)
		, m_top(0)
		, m_size(0)
	{
	}

	inline MemoryWriter::~MemoryWriter()
	{
	}

	inline int64_t MemoryWriter::seek(int64_t _offset, Whence::Enum _whence)
	{
		switch (_whence)
		{
			case Whence::Begin:
				m_pos = clamp<int64_t>(_offset, 0, m_top);
				break;

			case Whence::Current:
				m_pos = clamp<int64_t>(m_pos + _offset, 0, m_top);
				break;

			case Whence::End:
				m_pos = clamp<int64_t>(m_top - _offset, 0, m_top);
				break;
		}

		return m_pos;
	}

	inline int32_t MemoryWriter::write(const void* _data, int32_t _size, Error* _err)
	{
		BX_ASSERT(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

		int32_t morecore = int32_t(m_pos - m_size) + _size;

		if (0 < morecore)
		{
			morecore = alignUp(morecore, 0x1000);
			m_data = (uint8_t*)m_memBlock->more(morecore);
			m_size = m_memBlock->getSize();
		}

		int64_t remainder = m_size-m_pos;
		int32_t size = uint32_min(_size, uint32_t(min<int64_t>(remainder, INT32_MAX) ) );
		memCopy(&m_data[m_pos], _data, size);
		m_pos += size;
		m_top = max(m_top, m_pos);
		if (size != _size)
		{
			BX_ERROR_SET(_err, kErrorReaderWriterWrite, "MemoryWriter: write truncated.");
		}
		return size;
	}

	inline StaticMemoryBlockWriter::StaticMemoryBlockWriter(void* _data, uint32_t _size)
		: MemoryWriter(&m_smb)
		, m_smb(_data, _size)
	{
	}

	inline StaticMemoryBlockWriter::~StaticMemoryBlockWriter()
	{
	}

	inline int32_t read(ReaderI* _reader, void* _data, int32_t _size, Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		return _reader->read(_data, _size, _err);
	}

	template<typename Ty>
	inline int32_t read(ReaderI* _reader, Ty& _value, Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(isTriviallyCopyable<Ty>() );
		return _reader->read(&_value, sizeof(Ty), _err);
	}

	template<typename Ty>
	inline int32_t readHE(ReaderI* _reader, Ty& _value, bool _fromLittleEndian, Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(isTriviallyCopyable<Ty>() );
		Ty value;
		int32_t result = _reader->read(&value, sizeof(Ty), _err);
		_value = toHostEndian(value, _fromLittleEndian);
		return result;
	}

	inline int32_t write(WriterI* _writer, const void* _data, int32_t _size, Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		return _writer->write(_data, _size, _err);
	}

	inline int32_t write(WriterI* _writer, const char* _str, Error* _err)
	{
		return write(_writer, _str, strLen(_str), _err);
	}

	inline int32_t write(WriterI* _writer, const StringView& _str, Error* _err)
	{
		return write(_writer, _str.getPtr(), _str.getLength(), _err);
	}

	inline int32_t writeRep(WriterI* _writer, uint8_t _byte, int32_t _size, Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		const uint32_t tmp0      = uint32_sels(64   - _size,   64, _size);
		const uint32_t tmp1      = uint32_sels(256  - _size,  256, tmp0);
		const uint32_t blockSize = uint32_sels(1024 - _size, 1024, tmp1);
		uint8_t* temp = (uint8_t*)alloca(blockSize);
		memSet(temp, _byte, blockSize);

		int32_t size = 0;
		while (0 < _size)
		{
			int32_t bytes = write(_writer, temp, uint32_min(blockSize, _size), _err);
			size  += bytes;
			_size -= bytes;
		}

		return size;
	}

	template<typename Ty>
	inline int32_t write(WriterI* _writer, const Ty& _value, Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(isTriviallyCopyable<Ty>() );
		return _writer->write(&_value, sizeof(Ty), _err);
	}

	template<typename Ty>
	inline int32_t writeLE(WriterI* _writer, const Ty& _value, Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(isTriviallyCopyable<Ty>() );
		Ty value = toLittleEndian(_value);
		int32_t result = _writer->write(&value, sizeof(Ty), _err);
		return result;
	}

	template<>
	inline int32_t writeLE(WriterI* _writer, const float& _value, Error* _err)
	{
		return writeLE(_writer, floatToBits(_value), _err);
	}

	template<typename Ty>
	inline int32_t writeBE(WriterI* _writer, const Ty& _value, Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(isTriviallyCopyable<Ty>() );
		Ty value = toBigEndian(_value);
		int32_t result = _writer->write(&value, sizeof(Ty), _err);
		return result;
	}

	template<>
	inline int32_t writeBE(WriterI* _writer, const float& _value, Error* _err)
	{
		return writeBE(_writer, floatToBits(_value), _err);
	}

	inline int64_t skip(SeekerI* _seeker, int64_t _offset)
	{
		return _seeker->seek(_offset, Whence::Current);
	}

	inline int64_t seek(SeekerI* _seeker, int64_t _offset, Whence::Enum _whence)
	{
		return _seeker->seek(_offset, _whence);
	}

	inline int64_t getSize(SeekerI* _seeker)
	{
		int64_t offset = _seeker->seek();
		int64_t size   = _seeker->seek(0, Whence::End);
		_seeker->seek(offset, Whence::Begin);
		return size;
	}

	inline int64_t getRemain(SeekerI* _seeker)
	{
		int64_t offset = _seeker->seek();
		int64_t size   = _seeker->seek(0, Whence::End);
		_seeker->seek(offset, Whence::Begin);
		return size-offset;
	}

	inline int32_t peek(ReaderSeekerI* _reader, void* _data, int32_t _size, Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		int64_t offset = bx::seek(_reader);
		int32_t size = _reader->read(_data, _size, _err);
		bx::seek(_reader, offset, bx::Whence::Begin);
		return size;
	}

	template<typename Ty>
	inline int32_t peek(ReaderSeekerI* _reader, Ty& _value, Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		BX_STATIC_ASSERT(isTriviallyCopyable<Ty>() );
		return peek(_reader, &_value, sizeof(Ty), _err);
	}

	inline int32_t align(ReaderSeekerI* _reader, uint32_t _alignment, Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		const int64_t current = bx::seek(_reader);
		const int64_t aligned = ( (current + _alignment-1)/_alignment) * _alignment;
		const int32_t size    = int32_t(aligned - current);
		if (0 != size)
		{
			const int64_t offset  = bx::seek(_reader, size);
			if (offset != aligned)
			{
				BX_ERROR_SET(_err, kErrorReaderWriterWrite, "Align: read truncated.");
			}
			return int32_t(offset - current);
		}

		return 0;
	}

	inline int32_t align(WriterSeekerI* _writer, uint32_t _alignment, Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		const int64_t current = bx::seek(_writer);
		const int64_t aligned = ( (current + _alignment-1)/_alignment) * _alignment;
		const int32_t size    = int32_t(aligned - current);
		if (0 != size)
		{
			return writeRep(_writer, 0, size, _err);
		}

		return 0;
	}

	inline bool open(ReaderOpenI* _reader, const FilePath& _filePath, Error* _err)
	{
		BX_ERROR_USE_TEMP_WHEN_NULL(_err);
		return _reader->open(_filePath, _err);
	}

	inline bool open(WriterOpenI* _writer, const FilePath& _filePath, bool _append, Error* _err)
	{
		BX_ERROR_USE_TEMP_WHEN_NULL(_err);
		return _writer->open(_filePath, _append, _err);
	}

	inline bool open(ProcessOpenI* _process, const FilePath& _filePath, const StringView& _args, Error* _err)
	{
		BX_ERROR_USE_TEMP_WHEN_NULL(_err);
		return _process->open(_filePath, _args, _err);
	}

	inline void close(CloserI* _reader)
	{
		_reader->close();
	}

} // namespace bx
