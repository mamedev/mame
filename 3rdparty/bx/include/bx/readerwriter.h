/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_READERWRITER_H_HEADER_GUARD
#define BX_READERWRITER_H_HEADER_GUARD

#include "allocator.h"
#include "endian.h"
#include "error.h"
#include "filepath.h"
#include "math.h"
#include "string.h"
#include "uint32_t.h"

namespace bx
{
	BX_ERROR_RESULT(kErrorReaderWriterOpen,        BX_MAKEFOURCC('b', 'x', 2, 1) );
	BX_ERROR_RESULT(kErrorReaderWriterRead,        BX_MAKEFOURCC('b', 'x', 2, 2) );
	BX_ERROR_RESULT(kErrorReaderWriterWrite,       BX_MAKEFOURCC('b', 'x', 2, 3) );
	BX_ERROR_RESULT(kErrorReaderWriterEof,         BX_MAKEFOURCC('b', 'x', 2, 4) );
	BX_ERROR_RESULT(kErrorReaderWriterAlreadyOpen, BX_MAKEFOURCC('b', 'x', 2, 5) );

	/// The position from where offset is added.
	struct Whence
	{
		/// Whence values:
		enum Enum
		{
			Begin,   //!< From beginning of file.
			Current, //!< From current position of file.
			End,     //!< From end of file.
		};
	};

	/// Reader interface.
	struct BX_NO_VTABLE ReaderI
	{
		///
		virtual ~ReaderI() = 0;

		///
		virtual int32_t read(void* _data, int32_t _size, Error* _err) = 0;
	};

	/// Writer interface.
	struct BX_NO_VTABLE WriterI
	{
		///
		virtual ~WriterI() = 0;

		///
		virtual int32_t write(const void* _data, int32_t _size, Error* _err) = 0;
	};

	/// Seeker interface.
	struct BX_NO_VTABLE SeekerI
	{
		///
		virtual ~SeekerI() = 0;

		///
		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) = 0;
	};

	/// Reader seeker interface.
	struct BX_NO_VTABLE ReaderSeekerI : public ReaderI, public SeekerI
	{
	};

	/// Writer seeker interface.
	struct BX_NO_VTABLE WriterSeekerI : public WriterI, public SeekerI
	{
	};

	/// Open for reading interface.
	struct BX_NO_VTABLE ReaderOpenI
	{
		///
		virtual ~ReaderOpenI() = 0;

		///
		virtual bool open(const FilePath& _filePath, Error* _err) = 0;
	};

	/// Open for writing interface.
	struct BX_NO_VTABLE WriterOpenI
	{
		///
		virtual ~WriterOpenI() = 0;

		///
		virtual bool open(const FilePath& _filePath, bool _append, Error* _err) = 0;
	};

	/// Open process interface.
	struct BX_NO_VTABLE ProcessOpenI
	{
		///
		virtual ~ProcessOpenI() = 0;

		///
		virtual bool open(const FilePath& _filePath, const StringView& _args, Error* _err) = 0;
	};

	/// Closer interface.
	struct BX_NO_VTABLE CloserI
	{
		///
		virtual ~CloserI() = 0;

		///
		virtual void close() = 0;
	};

	/// File reader interface.
	struct BX_NO_VTABLE FileReaderI : public ReaderOpenI, public CloserI, public ReaderSeekerI
	{
	};

	/// File writer interface.
	struct BX_NO_VTABLE FileWriterI : public WriterOpenI, public CloserI, public WriterSeekerI
	{
	};

	/// Memory block interface.
	struct BX_NO_VTABLE MemoryBlockI
	{
		virtual void* more(uint32_t _size = 0) = 0;
		virtual uint32_t getSize() = 0;
	};

	/// Static memory block interface.
	class StaticMemoryBlock : public MemoryBlockI
	{
	public:
		///
		StaticMemoryBlock(void* _data, uint32_t _size);

		///
		virtual ~StaticMemoryBlock();

		///
		virtual void* more(uint32_t _size = 0) override;

		///
		virtual uint32_t getSize() override;

	private:
		void*    m_data;
		uint32_t m_size;
	};

	/// Memory block.
	class MemoryBlock : public MemoryBlockI
	{
	public:
		///
		MemoryBlock(AllocatorI* _allocator);

		///
		virtual ~MemoryBlock();

		///
		virtual void* more(uint32_t _size = 0) override;

		///
		virtual uint32_t getSize() override;

	private:
		AllocatorI* m_allocator;
		void*       m_data;
		uint32_t    m_size;
	};

	/// Sizer writer. Dummy writter that only counts number of bytes written into it.
	class SizerWriter : public WriterSeekerI
	{
	public:
		///
		SizerWriter();

		///
		virtual ~SizerWriter();

		///
		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) override;

		///
		virtual int32_t write(const void* /*_data*/, int32_t _size, Error* _err) override;

	private:
		int64_t m_pos;
		int64_t m_top;
	};

	/// Memory reader.
	class MemoryReader : public ReaderSeekerI
	{
	public:
		///
		MemoryReader(const void* _data, uint32_t _size);

		///
		virtual ~MemoryReader();

		///
		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) override;

		///
		virtual int32_t read(void* _data, int32_t _size, Error* _err) override;

		///
		const uint8_t* getDataPtr() const;

		///
		int64_t getPos() const;

		///
		int64_t remaining() const;

	private:
		const uint8_t* m_data;
		int64_t m_pos;
		int64_t m_top;
	};

	/// Memory writer.
	class MemoryWriter : public WriterSeekerI
	{
	public:
		///
		MemoryWriter(MemoryBlockI* _memBlock);

		///
		virtual ~MemoryWriter();

		///
		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) override;

		///
		virtual int32_t write(const void* _data, int32_t _size, Error* _err) override;

	private:
		MemoryBlockI* m_memBlock;
		uint8_t* m_data;
		int64_t  m_pos;
		int64_t  m_top;
		int64_t  m_size;
	};

	/// Static (fixed size) memory block writer.
	class StaticMemoryBlockWriter : public MemoryWriter
	{
	public:
		///
		StaticMemoryBlockWriter(void* _data, uint32_t _size);

		///
		virtual ~StaticMemoryBlockWriter();

	private:
		StaticMemoryBlock m_smb;
	};

	/// Read data.
	int32_t read(ReaderI* _reader, void* _data, int32_t _size, Error* _err = NULL);

	/// Read value.
	template<typename Ty>
	int32_t read(ReaderI* _reader, Ty& _value, Error* _err = NULL);

	/// Read value and converts it to host endianess. _fromLittleEndian specifies
	/// underlying stream endianess.
	template<typename Ty>
	int32_t readHE(ReaderI* _reader, Ty& _value, bool _fromLittleEndian, Error* _err = NULL);

	/// Write data.
	int32_t write(WriterI* _writer, const void* _data, int32_t _size, Error* _err = NULL);

	/// Write C string.
	int32_t write(WriterI* _writer, const char* _str, Error* _err = NULL);

	/// Write string view.
	int32_t write(WriterI* _writer, const StringView& _str, Error* _err = NULL);

	/// Write formatted string.
	int32_t write(WriterI* _writer, const StringView& _format, va_list _argList, Error* _err);

	/// Write formatted string.
	int32_t write(WriterI* _writer, Error* _err, const StringView* _format, ...);

	/// Write formatted string.
	int32_t write(WriterI* _writer, Error* _err, const char* _format, ...);

	/// Write repeat the same value.
	int32_t writeRep(WriterI* _writer, uint8_t _byte, int32_t _size, Error* _err = NULL);

	/// Write value.
	template<typename Ty>
	int32_t write(WriterI* _writer, const Ty& _value, Error* _err = NULL);

	/// Write value as little endian.
	template<typename Ty>
	int32_t writeLE(WriterI* _writer, const Ty& _value, Error* _err = NULL);

	/// Write value as big endian.
	template<typename Ty>
	int32_t writeBE(WriterI* _writer, const Ty& _value, Error* _err = NULL);

	/// Skip _offset bytes forward.
	int64_t skip(SeekerI* _seeker, int64_t _offset);

	/// Seek to any position in file.
	int64_t seek(SeekerI* _seeker, int64_t _offset = 0, Whence::Enum _whence = Whence::Current);

	/// Returns size of file.
	int64_t getSize(SeekerI* _seeker);

	/// Returns remaining size from current offset of file.
	int64_t getRemain(SeekerI* _seeker);

	/// Peek data.
	int32_t peek(ReaderSeekerI* _reader, void* _data, int32_t _size, Error* _err = NULL);

	/// Peek value.
	template<typename Ty>
	int32_t peek(ReaderSeekerI* _reader, Ty& _value, Error* _err = NULL);

	/// Align reader stream.
	int32_t align(ReaderSeekerI* _reader, uint32_t _alignment, Error* _err = NULL);

	/// Align writer stream (pads stream with zeros).
	int32_t align(WriterSeekerI* _writer, uint32_t _alignment, Error* _err = NULL);

	/// Open for read.
	bool open(ReaderOpenI* _reader, const FilePath& _filePath, Error* _err = NULL);

	/// Open for write.
	bool open(WriterOpenI* _writer, const FilePath& _filePath, bool _append = false, Error* _err = NULL);

	/// Open process.
	bool open(ProcessOpenI* _process, const FilePath& _filePath, const StringView& _args, Error* _err = NULL);

	/// Close.
	void close(CloserI* _reader);

} // namespace bx

#include "inline/readerwriter.inl"

#endif // BX_READERWRITER_H_HEADER_GUARD
