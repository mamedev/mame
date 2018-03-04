/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/file.h>

#include <stdio.h>
#include <sys/stat.h>

#ifndef BX_CONFIG_CRT_FILE_READER_WRITER
#	define BX_CONFIG_CRT_FILE_READER_WRITER !(0 \
			|| BX_CRT_NONE                      \
			)
#endif // BX_CONFIG_CRT_FILE_READER_WRITER

namespace bx
{
	class NoopWriterImpl : public FileWriterI
	{
	public:
		NoopWriterImpl(void*)
		{
		}

		virtual ~NoopWriterImpl()
		{
			close();
		}

		virtual bool open(const FilePath& _filePath, bool _append, Error* _err) override
		{
			BX_UNUSED(_filePath, _append, _err);
			return false;
		}

		virtual void close() override
		{
		}

		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) override
		{
			BX_UNUSED(_offset, _whence);
			return 0;
		}

		virtual int32_t write(const void* _data, int32_t _size, Error* _err) override
		{
			BX_UNUSED(_data, _size, _err);
			return 0;
		}
	};

#if BX_CONFIG_CRT_FILE_READER_WRITER

#	if BX_CRT_MSVC
#		define fseeko64 _fseeki64
#		define ftello64 _ftelli64
#	elif 0                   \
	  || BX_PLATFORM_ANDROID \
	  || BX_PLATFORM_BSD     \
	  || BX_PLATFORM_IOS     \
	  || BX_PLATFORM_OSX     \
	  || BX_PLATFORM_QNX
#		define fseeko64 fseeko
#		define ftello64 ftello
#	elif BX_PLATFORM_PS4
#		define fseeko64 fseek
#		define ftello64 ftell
#	endif // BX_

	class FileReaderImpl : public FileReaderI
	{
	public:
		FileReaderImpl(FILE* _file)
			: m_file(_file)
			, m_open(false)
		{
		}

		virtual ~FileReaderImpl()
		{
			close();
		}

		virtual bool open(const FilePath& _filePath, Error* _err) override
		{
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			if (NULL != m_file)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_ALREADY_OPEN, "FileReader: File is already open.");
				return false;
			}

			m_file = fopen(_filePath.get(), "rb");
			if (NULL == m_file)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "FileReader: Failed to open file.");
				return false;
			}

			m_open = true;
			return true;
		}

		virtual void close() override
		{
			if (m_open
			&&  NULL != m_file)
			{
				fclose(m_file);
				m_file = NULL;
			}
		}

		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) override
		{
			BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
			fseeko64(m_file, _offset, _whence);
			return ftello64(m_file);
		}

		virtual int32_t read(void* _data, int32_t _size, Error* _err) override
		{
			BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int32_t size = (int32_t)fread(_data, 1, _size, m_file);
			if (size != _size)
			{
				if (0 != feof(m_file) )
				{
					BX_ERROR_SET(_err, BX_ERROR_READERWRITER_EOF, "FileReader: EOF.");
				}
				else if (0 != ferror(m_file) )
				{
					BX_ERROR_SET(_err, BX_ERROR_READERWRITER_READ, "FileReader: read error.");
				}

				return size >= 0 ? size : 0;
			}

			return size;
		}

	private:
		FILE* m_file;
		bool  m_open;
	};

	class FileWriterImpl : public FileWriterI
	{
	public:
		FileWriterImpl(FILE* _file)
			: m_file(_file)
			, m_open(false)
		{
		}

		virtual ~FileWriterImpl()
		{
			close();
		}

		virtual bool open(const FilePath& _filePath, bool _append, Error* _err) override
		{
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			if (NULL != m_file)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_ALREADY_OPEN, "FileReader: File is already open.");
				return false;
			}

			m_file = fopen(_filePath.get(), _append ? "ab" : "wb");

			if (NULL == m_file)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "FileWriter: Failed to open file.");
				return false;
			}

			m_open = true;
			return true;
		}

		virtual void close() override
		{
			if (m_open
			&&  NULL != m_file)
			{
				fclose(m_file);
				m_file = NULL;
			}
		}

		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) override
		{
			BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
			fseeko64(m_file, _offset, _whence);
			return ftello64(m_file);
		}

		virtual int32_t write(const void* _data, int32_t _size, Error* _err) override
		{
			BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int32_t size = (int32_t)fwrite(_data, 1, _size, m_file);
			if (size != _size)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_WRITE, "FileWriter: write failed.");
				return size >= 0 ? size : 0;
			}

			return size;
		}

	private:
		FILE* m_file;
		bool  m_open;
	};

#else

	class FileReaderImpl : public FileReaderI
	{
	public:
		FileReaderImpl(void*)
		{
		}

		virtual ~FileReaderImpl()
		{
			close();
		}

		virtual bool open(const FilePath& _filePath, Error* _err) override
		{
			BX_UNUSED(_filePath, _err);
			return false;
		}

		virtual void close() override
		{
		}

		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) override
		{
			BX_UNUSED(_offset, _whence);
			return 0;
		}

		virtual int32_t read(void* _data, int32_t _size, Error* _err) override
		{
			BX_UNUSED(_data, _size, _err);
			return 0;
		}
	};

	typedef NoopWriterImpl FileWriterImpl;

#endif // BX_CONFIG_CRT_FILE_READER_WRITER

	FileReader::FileReader()
	{
		BX_STATIC_ASSERT(sizeof(FileReaderImpl) <= sizeof(m_internal) );
		BX_PLACEMENT_NEW(m_internal, FileReaderImpl)(NULL);
	}

	FileReader::~FileReader()
	{
		FileReaderImpl* impl = reinterpret_cast<FileReaderImpl*>(m_internal);
		impl->~FileReaderImpl();
	}

	bool FileReader::open(const FilePath& _filePath, Error* _err)
	{
		FileReaderImpl* impl = reinterpret_cast<FileReaderImpl*>(m_internal);
		return impl->open(_filePath, _err);
	}

	void FileReader::close()
	{
		FileReaderImpl* impl = reinterpret_cast<FileReaderImpl*>(m_internal);
		impl->close();
	}

	int64_t FileReader::seek(int64_t _offset, Whence::Enum _whence)
	{
		FileReaderImpl* impl = reinterpret_cast<FileReaderImpl*>(m_internal);
		return impl->seek(_offset, _whence);
	}

	int32_t FileReader::read(void* _data, int32_t _size, Error* _err)
	{
		FileReaderImpl* impl = reinterpret_cast<FileReaderImpl*>(m_internal);
		return impl->read(_data, _size, _err);
	}

	FileWriter::FileWriter()
	{
		BX_STATIC_ASSERT(sizeof(FileWriterImpl) <= sizeof(m_internal) );
		BX_PLACEMENT_NEW(m_internal, FileWriterImpl)(NULL);
	}

	FileWriter::~FileWriter()
	{
		FileWriterImpl* impl = reinterpret_cast<FileWriterImpl*>(m_internal);
		impl->~FileWriterImpl();
	}

	bool FileWriter::open(const FilePath& _filePath, bool _append, Error* _err)
	{
		FileWriterImpl* impl = reinterpret_cast<FileWriterImpl*>(m_internal);
		return impl->open(_filePath, _append, _err);
	}

	void FileWriter::close()
	{
		FileWriterImpl* impl = reinterpret_cast<FileWriterImpl*>(m_internal);
		impl->close();
	}

	int64_t FileWriter::seek(int64_t _offset, Whence::Enum _whence)
	{
		FileWriterImpl* impl = reinterpret_cast<FileWriterImpl*>(m_internal);
		return impl->seek(_offset, _whence);
	}

	int32_t FileWriter::write(const void* _data, int32_t _size, Error* _err)
	{
		FileWriterImpl* impl = reinterpret_cast<FileWriterImpl*>(m_internal);
		return impl->write(_data, _size, _err);
	}

	ReaderI* getStdIn()
	{
		static FileReaderImpl s_stdIn(stdout);
		return &s_stdIn;
	}

	WriterI* getStdOut()
	{
		static FileWriterImpl s_stdOut(stdout);
		return &s_stdOut;
	}

	WriterI* getStdErr()
	{
		static FileWriterImpl s_stdOut(stderr);
		return &s_stdOut;
	}

	WriterI* getNullOut()
	{
		static NoopWriterImpl s_nullOut(NULL);
		return &s_nullOut;
	}

	bool stat(const FilePath& _filePath, FileInfo& _outFileInfo)
	{
		_outFileInfo.m_size = 0;
		_outFileInfo.m_type = FileInfo::Count;

#if BX_COMPILER_MSVC
		struct ::_stat64 st;
		int32_t result = ::_stat64(_filePath.get(), &st);

		if (0 != result)
		{
			return false;
		}

		if (0 != (st.st_mode & _S_IFREG) )
		{
			_outFileInfo.m_type = FileInfo::Regular;
		}
		else if (0 != (st.st_mode & _S_IFDIR) )
		{
			_outFileInfo.m_type = FileInfo::Directory;
		}
#else
		struct ::stat st;
		int32_t result = ::stat(_filePath.get(), &st);
		if (0 != result)
		{
			return false;
		}

		if (0 != (st.st_mode & S_IFREG) )
		{
			_outFileInfo.m_type = FileInfo::Regular;
		}
		else if (0 != (st.st_mode & S_IFDIR) )
		{
			_outFileInfo.m_type = FileInfo::Directory;
		}
#endif // BX_COMPILER_MSVC

		_outFileInfo.m_size = st.st_size;

		return true;
	}

} // namespace bx
