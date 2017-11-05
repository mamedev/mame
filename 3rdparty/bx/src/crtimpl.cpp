/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <bx/crtimpl.h>
#include <stdio.h>

#if BX_CONFIG_ALLOCATOR_CRT
#	include <malloc.h>
#endif // BX_CONFIG_ALLOCATOR_CRT

namespace bx
{
#if BX_CONFIG_ALLOCATOR_CRT
	CrtAllocator::CrtAllocator()
	{
	}

	CrtAllocator::~CrtAllocator()
	{
	}

	void* CrtAllocator::realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line)
	{
		if (0 == _size)
		{
			if (NULL != _ptr)
			{
				if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= _align)
				{
					::free(_ptr);
					return NULL;
				}

#	if BX_COMPILER_MSVC
				BX_UNUSED(_file, _line);
				_aligned_free(_ptr);
#	else
				bx::alignedFree(this, _ptr, _align, _file, _line);
#	endif // BX_
			}

			return NULL;
		}
		else if (NULL == _ptr)
		{
			if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= _align)
			{
				return ::malloc(_size);
			}

#	if BX_COMPILER_MSVC
			BX_UNUSED(_file, _line);
			return _aligned_malloc(_size, _align);
#	else
			return bx::alignedAlloc(this, _size, _align, _file, _line);
#	endif // BX_
		}

		if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= _align)
		{
			return ::realloc(_ptr, _size);
		}

#	if BX_COMPILER_MSVC
		BX_UNUSED(_file, _line);
		return _aligned_realloc(_ptr, _size, _align);
#	else
		return bx::alignedRealloc(this, _ptr, _size, _align, _file, _line);
#	endif // BX_
	}
#endif // BX_CONFIG_ALLOCATOR_CRT

#if BX_CONFIG_CRT_FILE_READER_WRITER

#	if BX_CRT_MSVC
#		define fseeko64 _fseeki64
#		define ftello64 _ftelli64
#	elif 0 \
	  || BX_PLATFORM_ANDROID \
	  || BX_PLATFORM_BSD \
	  || BX_PLATFORM_IOS \
	  || BX_PLATFORM_OSX \
	  || BX_PLATFORM_QNX
#		define fseeko64 fseeko
#		define ftello64 ftello
#	elif BX_PLATFORM_PS4
#		define fseeko64 fseek
#		define ftello64 ftell
#	endif // BX_

	CrtFileReader::CrtFileReader()
		: m_file(NULL)
	{
	}

	CrtFileReader::~CrtFileReader()
	{
	}

	bool CrtFileReader::open(const char* _filePath, Error* _err)
	{
		BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

		if (NULL != m_file)
		{
			BX_ERROR_SET(_err, BX_ERROR_READERWRITER_ALREADY_OPEN, "CrtFileReader: File is already open.");
			return false;
		}

		m_file = fopen(_filePath, "rb");
		if (NULL == m_file)
		{
			BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "CrtFileReader: Failed to open file.");
			return false;
		}

		return true;
	}

	void CrtFileReader::close()
	{
		BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
		FILE* file = (FILE*)m_file;
		fclose(file);
		m_file = NULL;
	}

	int64_t CrtFileReader::seek(int64_t _offset, Whence::Enum _whence)
	{
		BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
		FILE* file = (FILE*)m_file;
		fseeko64(file, _offset, _whence);
		return ftello64(file);
	}

	int32_t CrtFileReader::read(void* _data, int32_t _size, Error* _err)
	{
		BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
		BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

		FILE* file = (FILE*)m_file;
		int32_t size = (int32_t)fread(_data, 1, _size, file);
		if (size != _size)
		{
			if (0 != feof(file) )
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_EOF, "CrtFileReader: EOF.");
			}
			else if (0 != ferror(file) )
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_READ, "CrtFileReader: read error.");
			}

			return size >= 0 ? size : 0;
		}

		return size;
	}

	CrtFileWriter::CrtFileWriter()
		: m_file(NULL)
	{
	}

	CrtFileWriter::~CrtFileWriter()
	{
	}

	bool CrtFileWriter::open(const char* _filePath, bool _append, Error* _err)
	{
		BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

		if (NULL != m_file)
		{
			BX_ERROR_SET(_err, BX_ERROR_READERWRITER_ALREADY_OPEN, "CrtFileReader: File is already open.");
			return false;
		}

		m_file = fopen(_filePath, _append ? "ab" : "wb");

		if (NULL == m_file)
		{
			BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "CrtFileWriter: Failed to open file.");
			return false;
		}

		return true;
	}

	void CrtFileWriter::close()
	{
		BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
		FILE* file = (FILE*)m_file;
		fclose(file);
		m_file = NULL;
	}

	int64_t CrtFileWriter::seek(int64_t _offset, Whence::Enum _whence)
	{
		BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
		FILE* file = (FILE*)m_file;
		fseeko64(file, _offset, _whence);
		return ftello64(file);
	}

	int32_t CrtFileWriter::write(const void* _data, int32_t _size, Error* _err)
	{
		BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
		BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

		FILE* file = (FILE*)m_file;
		int32_t size = (int32_t)fwrite(_data, 1, _size, file);
		if (size != _size)
		{
			BX_ERROR_SET(_err, BX_ERROR_READERWRITER_WRITE, "CrtFileWriter: write failed.");
			return size >= 0 ? size : 0;
		}

		return size;
	}
#endif // BX_CONFIG_CRT_FILE_READER_WRITER

#if BX_CONFIG_CRT_PROCESS

#if BX_CRT_MSVC
#	define popen  _popen
#	define pclose _pclose
#endif // BX_CRT_MSVC

	ProcessReader::ProcessReader()
		: m_file(NULL)
	{
	}

	ProcessReader::~ProcessReader()
	{
		BX_CHECK(NULL == m_file, "Process not closed!");
	}

	bool ProcessReader::open(const char* _command, Error* _err)
	{
		BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

		if (NULL != m_file)
		{
			BX_ERROR_SET(_err, BX_ERROR_READERWRITER_ALREADY_OPEN, "ProcessReader: File is already open.");
			return false;
		}

		m_file = popen(_command, "r");
		if (NULL == m_file)
		{
			BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "ProcessReader: Failed to open process.");
			return false;
		}

		return true;
	}

	void ProcessReader::close()
	{
		BX_CHECK(NULL != m_file, "Process not open!");
		FILE* file = (FILE*)m_file;
		m_exitCode = pclose(file);
		m_file = NULL;
	}

	int32_t ProcessReader::read(void* _data, int32_t _size, Error* _err)
	{
		BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors."); BX_UNUSED(_err);

		FILE* file = (FILE*)m_file;
		int32_t size = (int32_t)fread(_data, 1, _size, file);
		if (size != _size)
		{
			if (0 != feof(file) )
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_EOF, "ProcessReader: EOF.");
			}
			else if (0 != ferror(file) )
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_READ, "ProcessReader: read error.");
			}

			return size >= 0 ? size : 0;
		}

		return size;
	}

	int32_t ProcessReader::getExitCode() const
	{
		return m_exitCode;
	}

	ProcessWriter::ProcessWriter()
		: m_file(NULL)
	{
	}

	ProcessWriter::~ProcessWriter()
	{
		BX_CHECK(NULL == m_file, "Process not closed!");
	}

	bool ProcessWriter::open(const char* _command, bool, Error* _err)
	{
		BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

		if (NULL != m_file)
		{
			BX_ERROR_SET(_err, BX_ERROR_READERWRITER_ALREADY_OPEN, "ProcessWriter: File is already open.");
			return false;
		}

		m_file = popen(_command, "w");
		if (NULL == m_file)
		{
			BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "ProcessWriter: Failed to open process.");
			return false;
		}

		return true;
	}

	void ProcessWriter::close()
	{
		BX_CHECK(NULL != m_file, "Process not open!");
		FILE* file = (FILE*)m_file;
		m_exitCode = pclose(file);
		m_file = NULL;
	}

	int32_t ProcessWriter::write(const void* _data, int32_t _size, Error* _err)
	{
		BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors."); BX_UNUSED(_err);

		FILE* file = (FILE*)m_file;
		int32_t size = (int32_t)fwrite(_data, 1, _size, file);
		if (size != _size)
		{
			if (0 != ferror(file) )
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_WRITE, "ProcessWriter: write error.");
			}

			return size >= 0 ? size : 0;
		}

		return size;
	}

	int32_t ProcessWriter::getExitCode() const
	{
		return m_exitCode;
	}
#endif // BX_CONFIG_CRT_PROCESS

} // namespace bx
