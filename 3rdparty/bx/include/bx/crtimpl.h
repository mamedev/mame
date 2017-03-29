/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_CRTIMPL_H_HEADER_GUARD
#define BX_CRTIMPL_H_HEADER_GUARD

#include "bx.h"

#if BX_CONFIG_ALLOCATOR_CRT
#	include "allocator.h"
#endif // BX_CONFIG_ALLOCATOR_CRT

#if BX_CONFIG_CRT_FILE_READER_WRITER
#	include "readerwriter.h"
#endif // BX_CONFIG_CRT_FILE_READER_WRITER

namespace bx
{
#if BX_CONFIG_ALLOCATOR_CRT
	///
	class CrtAllocator : public AllocatorI
	{
	public:
		///
		CrtAllocator();

		///
		virtual ~CrtAllocator();

		///
		virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line) BX_OVERRIDE;
	};
#endif // BX_CONFIG_ALLOCATOR_CRT

#if BX_CONFIG_CRT_FILE_READER_WRITER
	///
	class CrtFileReader : public FileReaderI
	{
	public:
		///
		CrtFileReader();

		///
		virtual ~CrtFileReader();

		///
		virtual bool open(const char* _filePath, Error* _err) BX_OVERRIDE;

		///
		virtual void close() BX_OVERRIDE;

		///
		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) BX_OVERRIDE;

		///
		virtual int32_t read(void* _data, int32_t _size, Error* _err) BX_OVERRIDE;

	private:
		void* m_file;
	};

	///
	class CrtFileWriter : public FileWriterI
	{
	public:
		///
		CrtFileWriter();

		///
		virtual ~CrtFileWriter();

		///
		virtual bool open(const char* _filePath, bool _append, Error* _err) BX_OVERRIDE;

		///
		virtual void close() BX_OVERRIDE;

		///
		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) BX_OVERRIDE;

		///
		virtual int32_t write(const void* _data, int32_t _size, Error* _err) BX_OVERRIDE;

	private:
		void* m_file;
	};
#endif // BX_CONFIG_CRT_FILE_READER_WRITER

#if BX_CONFIG_CRT_PROCESS
	///
	class ProcessReader : public ReaderOpenI, public CloserI, public ReaderI
	{
	public:
		///
		ProcessReader();

		///
		~ProcessReader();

		///
		virtual bool open(const char* _command, Error* _err) BX_OVERRIDE;

		///
		virtual void close() BX_OVERRIDE;

		///
		virtual int32_t read(void* _data, int32_t _size, Error* _err) BX_OVERRIDE;

		///
		int32_t getExitCode() const;

	private:
		void* m_file;
		int32_t m_exitCode;
	};

	///
	class ProcessWriter : public WriterOpenI, public CloserI, public WriterI
	{
	public:
		///
		ProcessWriter();

		///
		~ProcessWriter();

		///
		virtual bool open(const char* _command, bool, Error* _err) BX_OVERRIDE;

		///
		virtual void close() BX_OVERRIDE;

		///
		virtual int32_t write(const void* _data, int32_t _size, Error* _err) BX_OVERRIDE;

		///
		int32_t getExitCode() const;

	private:
		void* m_file;
		int32_t m_exitCode;
	};
#endif // BX_CONFIG_CRT_PROCESS

} // namespace bx

#endif // BX_CRTIMPL_H_HEADER_GUARD
