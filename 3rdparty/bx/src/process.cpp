/*
 * Copyright 2010-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include <bx/process.h>

#include <stdio.h>

#ifndef BX_CONFIG_CRT_PROCESS
#	define BX_CONFIG_CRT_PROCESS !(0  \
			|| BX_CRT_NONE            \
			|| BX_PLATFORM_EMSCRIPTEN \
			|| BX_PLATFORM_PS4        \
			|| BX_PLATFORM_WINRT      \
			|| BX_PLATFORM_XBOXONE    \
			)
#endif // BX_CONFIG_CRT_PROCESS

namespace bx
{
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
		BX_ASSERT(NULL == m_file, "Process not closed!");
	}

	bool ProcessReader::open(const FilePath& _filePath, const StringView& _args, Error* _err)
	{
		BX_ASSERT(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

		if (NULL != m_file)
		{
			BX_ERROR_SET(_err, kErrorReaderWriterAlreadyOpen, "ProcessReader: File is already open.");
			return false;
		}

		char tmp[kMaxFilePath*2] = "\"";
		strCat(tmp, BX_COUNTOF(tmp), _filePath);
		strCat(tmp, BX_COUNTOF(tmp), "\" ");
		strCat(tmp, BX_COUNTOF(tmp), _args);

		m_file = popen(tmp, "r");
		if (NULL == m_file)
		{
			BX_ERROR_SET(_err, kErrorReaderWriterOpen, "ProcessReader: Failed to open process.");
			return false;
		}

		return true;
	}

	void ProcessReader::close()
	{
		BX_ASSERT(NULL != m_file, "Process not open!");
		FILE* file = (FILE*)m_file;
		m_exitCode = pclose(file);
		m_file = NULL;
	}

	int32_t ProcessReader::read(void* _data, int32_t _size, Error* _err)
	{
		BX_ASSERT(NULL != _err, "Reader/Writer interface calling functions must handle errors."); BX_UNUSED(_err);

		FILE* file = (FILE*)m_file;
		int32_t size = (int32_t)fread(_data, 1, _size, file);
		if (size != _size)
		{
			if (0 != feof(file) )
			{
				BX_ERROR_SET(_err, kErrorReaderWriterEof, "ProcessReader: EOF.");
			}
			else if (0 != ferror(file) )
			{
				BX_ERROR_SET(_err, kErrorReaderWriterRead, "ProcessReader: read error.");
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
		BX_ASSERT(NULL == m_file, "Process not closed!");
	}

	bool ProcessWriter::open(const FilePath& _filePath, const StringView& _args, Error* _err)
	{
		BX_ASSERT(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

		if (NULL != m_file)
		{
			BX_ERROR_SET(_err, kErrorReaderWriterAlreadyOpen, "ProcessWriter: File is already open.");
			return false;
		}

		char tmp[kMaxFilePath*2] = "\"";
		strCat(tmp, BX_COUNTOF(tmp), _filePath);
		strCat(tmp, BX_COUNTOF(tmp), "\" ");
		strCat(tmp, BX_COUNTOF(tmp), _args);

		m_file = popen(tmp, "w");
		if (NULL == m_file)
		{
			BX_ERROR_SET(_err, kErrorReaderWriterOpen, "ProcessWriter: Failed to open process.");
			return false;
		}

		return true;
	}

	void ProcessWriter::close()
	{
		BX_ASSERT(NULL != m_file, "Process not open!");
		FILE* file = (FILE*)m_file;
		m_exitCode = pclose(file);
		m_file = NULL;
	}

	int32_t ProcessWriter::write(const void* _data, int32_t _size, Error* _err)
	{
		BX_ASSERT(NULL != _err, "Reader/Writer interface calling functions must handle errors."); BX_UNUSED(_err);

		FILE* file = (FILE*)m_file;
		int32_t size = (int32_t)fwrite(_data, 1, _size, file);
		if (size != _size)
		{
			if (0 != ferror(file) )
			{
				BX_ERROR_SET(_err, kErrorReaderWriterWrite, "ProcessWriter: write error.");
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
